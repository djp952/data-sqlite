//---------------------------------------------------------------------------
// Copyright (c) 2008-2022 Michael G. Brehm
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//---------------------------------------------------------------------------

#ifndef __SQLITEROWEVENTARGS_H_
#define __SQLITEROWEVENTARGS_H_
#pragma once

#include "SqliteCommand.h"					// Include SqliteCommand declarations
#include "SqliteEnumerations.h"			// Include Sqlite enumeration decls
#include "SqliteUtil.h"					// Include SqliteUtil declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

using namespace System;
using namespace System::Collections::ObjectModel;
using namespace System::Data;
using namespace System::Data::Common;
using namespace System::Runtime::InteropServices;

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// Class SqliteAuthorizeEventArgs
//
// Used as the event argument class for the SqliteConnection::Authorize event
// and of course all of it's plumbing.  SQLite is a little inconsistent (from
// my perspective) as to the order of the arguments passed into it's hander,
// so instead of trying to come up with single property names that would be
// meaningless, I've opted for a read-only collection of strings instead.
//---------------------------------------------------------------------------

public ref class SqliteAuthorizeEventArgs : public EventArgs
{
public:

	//-----------------------------------------------------------------------
	// Properties

	// Action
	//
	// Exposes the database action that has caused the authorizer to be called
	property SqliteAuthorizeAction Action
	{
		SqliteAuthorizeAction get(void) { return m_action; }
	}

	// Arguments
	//
	// A read-only collection of the three authorizer argument strings
	property ReadOnlyCollection<String^>^ Arguments
	{
		ReadOnlyCollection<String^>^ get(void) { return m_args; }
	}

	// IndirectCaller
	//
	// If this is an indirect authorization request, provides the name of
	// the innermost trigger or view that is responsible for it
	property String^ IndirectCaller
	{
		String^ get(void) { return m_indirect; }
	}

	// IsIndirect
	//
	// Flag indicating if this authorization request is indirect, meaning
	// that it came from a trigger or a view and not directly from user SQL
	property bool IsIndirect
	{
		bool get(void) { return (m_indirect->Length > 0); }
	}

	// Response
	//
	// Sets the response to send back to SQLite to allow, deny or ignore
	// the action being attempted
	property SqliteAuthorizeResponse Response
	{
		SqliteAuthorizeResponse get(void) { return m_response; }
		void set(SqliteAuthorizeResponse value) { m_response = value; }
	}

internal:

	// INTERNAL CONSTRUCTOR
	SqliteAuthorizeEventArgs(int action, const char* arg0, const char* arg1, const char* arg2,
		const char* indirectCaller);

private:

	//-----------------------------------------------------------------------
	// Member Variables

	SqliteAuthorizeAction				m_action;		// Reason Authorize() was invoked
	ReadOnlyCollection<String^>^	m_args;			// Read-only argument collection
	String^							m_indirect;		// Indirect caller name if present
	SqliteAuthorizeResponse			m_response;		// Response from authorization
};

//---------------------------------------------------------------------------
// Class SqliteCollationNeededEventArgs
//
// Used as the event argument class for SqliteConnection::CollationNeeded
//---------------------------------------------------------------------------

public ref class SqliteCollationNeededEventArgs : public EventArgs
{
public:

	//-----------------------------------------------------------------------
	// Properties

	// Encoding
	//
	// Gets the optimal encoding for the required collation
	property SqliteCollationEncoding Encoding
	{
		SqliteCollationEncoding get(void) { return m_encoding; }
	}

	// Name
	//
	// Returns the required name for the needed collation
	property String^ Name
	{
		String^ get(void) { return m_name; }
	}

internal:

	// INTERNAL CONSTRUCTOR
	SqliteCollationNeededEventArgs(int encoding, const void* name);

private:

	//-----------------------------------------------------------------------
	// Member Variables

	SqliteCollationEncoding			m_encoding;		// Requested encoding mode
	String^							m_name;			// Requested collation name
};

//---------------------------------------------------------------------------
// Class SqliteProfileEventArgs
//
// Used as the event argument class for SqliteConnection::StatementProfile
//---------------------------------------------------------------------------

public ref class SqliteProfileEventArgs : public EventArgs
{
public:

	//-----------------------------------------------------------------------
	// Properties

	// ElapsedTime
	//
	// Exposes the elapsed time of the SQL statement.  Note that this includes
	// all the time the application spends processing the statement, so be sure
	// to document that property and provide a meaningful example
	property TimeSpan ElapsedTime { TimeSpan get(void) { return m_elapsed; } }

	// Statement
	//
	// Exposes the SQL statement being profiled
	property String^ Statement { String^ get(void) { return m_statement; } }

internal:

	// INTERNAL CONSTRUCTOR
	SqliteProfileEventArgs(const char* statement, sqlite_uint64 ticks)
	{
		// NOTE: Do not use SqliteUtil::FastPtrToStringAnsi here since that
		// uses the stack, and this message can potentially be enormous.

		m_statement = Marshal::PtrToStringAnsi(IntPtr(const_cast<char*>(statement)));
		m_elapsed = TimeSpan::FromTicks(ticks / 100);		// NS -> 100NS TICKS
	}

private:

	//-----------------------------------------------------------------------
	// Member Variables

	String^					m_statement;		// Statement text
	TimeSpan				m_elapsed;			// Elapsed statement time
};

//---------------------------------------------------------------------------
// Class SqliteRowChangedEventArgs
//
// Used as the event argument class for the SqliteConnection hook events that
// expose when a row has been DELETEd, INSERTed or UPDATEd
//---------------------------------------------------------------------------

public ref class SqliteRowChangedEventArgs : public EventArgs
{
public:

	//-----------------------------------------------------------------------
	// Properties

	// ChangeType
	//
	// Exposes the type of change that incurred the event
	property SqliteRowChangeType ChangeType { SqliteRowChangeType get(void) { return m_reason; } }

	// DatabaseName
	//
	// Name of the database that had a row changed
	property String^ DatabaseName { String^ get(void) { return m_database; } }

	// RowID
	//
	// The ROWID of the modified row in DatabaseName.TableName
	property __int64 RowID { __int64 get(void) { return m_rowid; } }

	// TableName
	//
	// Name of the table that had a row changed
	property String^ TableName { String^ get(void) { return m_table; } } 

internal:

	// INTERNAL CONSTRUCTOR
	SqliteRowChangedEventArgs(int reason, const char* database, const char* table, __int64 rowid) :
		m_reason(static_cast<SqliteRowChangeType>(reason)), m_database(SqliteUtil::FastPtrToStringAnsi(database)), 
		m_table(SqliteUtil::FastPtrToStringAnsi(table)), m_rowid(rowid) {}

private:

	//-----------------------------------------------------------------------
	// Member Variables

	SqliteRowChangeType		m_reason;			// Reason row was changed
	String^					m_database;			// The database name
	String^					m_table;			// The table name
	__int64					m_rowid;			// The ROWID value
};

//---------------------------------------------------------------------------
// Class SqliteRowUpdatedEventArgs
//
// Provides a strongly-typed version of the Data::Common::RowUpdatedEventArgs
// class, as is required to implement the SqliteDataAdapter properly
//---------------------------------------------------------------------------

public ref class SqliteRowUpdatedEventArgs : public RowUpdatedEventArgs
{
public:

	//-----------------------------------------------------------------------
	// Properties

	// Command
	//
	// Casts the weakly typed DbCommand property into a SqliteCommand object
	property SqliteCommand^ Command 
	{ 
		SqliteCommand^ get(void) new { return safe_cast<SqliteCommand^>(__super::Command); }
	}

internal:

	// INTERNAL CONSTRUCTOR
	SqliteRowUpdatedEventArgs(DataRow^ row, IDbCommand^ cmd, Data::StatementType type, 
		DataTableMapping^ mapping) : RowUpdatedEventArgs(row, cmd, type, mapping) {}
};

//---------------------------------------------------------------------------
// Class SqliteRowUpdatingEventArgs
//
// Provides a strongly-typed version of the Data::Common::RowUpdatingEventArgs
// class, as is required to implement the SqliteDataAdapter properly
//---------------------------------------------------------------------------

public ref struct SqliteRowUpdatingEventArgs : public RowUpdatingEventArgs
{
	SqliteRowUpdatingEventArgs(DataRow^ row, IDbCommand^ cmd, Data::StatementType type, 
		DataTableMapping^ mapping) : RowUpdatingEventArgs(row, cmd, type, mapping) {}

	property SqliteCommand^ Command
	{
		SqliteCommand^ get(void) new { return safe_cast<SqliteCommand^>(__super::Command); }
		void set(SqliteCommand^ value) { __super::Command = value; }
	}
};

//---------------------------------------------------------------------------
// Class SqliteTraceEventArgs
//
// Used as the event argument class for SqliteConnection::StatementTrace
//---------------------------------------------------------------------------

public ref class SqliteTraceEventArgs : public EventArgs
{
public:

	//-----------------------------------------------------------------------
	// Properties

	// Message
	//
	// Exposes the trace message sent from the SQLite engine
	property String^ Message { String^ get(void) { return m_message; } }

internal:

	// INTERNAL CONSTRUCTOR
	SqliteTraceEventArgs(const char* message)
	{
		// NOTE: Do not use SqliteUtil::FastPtrToStringAnsi here since that
		// uses the stack, and this message can potentially be enormous

		m_message = Marshal::PtrToStringAnsi(IntPtr(const_cast<char*>(message)));
	}

private:

	//-----------------------------------------------------------------------
	// Member Variables

	String^					m_message;			// Trace message from SQLite
};

//---------------------------------------------------------------------------


} // zuki::data::sqlite

#pragma warning(pop)

#endif		// __SQLITEROWEVENTARGS_H_
