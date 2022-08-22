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

#ifndef __ZDBROWEVENTARGS_H_
#define __ZDBROWEVENTARGS_H_
#pragma once

#include "zDBCommand.h"					// Include zDBCommand declarations
#include "zDBEnumerations.h"			// Include zDB enumeration decls
#include "zDBUtil.h"					// Include zDBUtil declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

using namespace System;
using namespace System::Collections::ObjectModel;
using namespace System::Data;
using namespace System::Data::Common;
using namespace System::Runtime::InteropServices;

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// Class zDBAuthorizeEventArgs
//
// Used as the event argument class for the zDBConnection::Authorize event
// and of course all of it's plumbing.  SQLite is a little inconsistent (from
// my perspective) as to the order of the arguments passed into it's hander,
// so instead of trying to come up with single property names that would be
// meaningless, I've opted for a read-only collection of strings instead.
//---------------------------------------------------------------------------

public ref class zDBAuthorizeEventArgs : public EventArgs
{
public:

	//-----------------------------------------------------------------------
	// Properties

	// Action
	//
	// Exposes the database action that has caused the authorizer to be called
	property zDBAuthorizeAction Action
	{
		zDBAuthorizeAction get(void) { return m_action; }
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
	property zDBAuthorizeResponse Response
	{
		zDBAuthorizeResponse get(void) { return m_response; }
		void set(zDBAuthorizeResponse value) { m_response = value; }
	}

internal:

	// INTERNAL CONSTRUCTOR
	zDBAuthorizeEventArgs(int action, const char* arg0, const char* arg1, const char* arg2,
		const char* indirectCaller);

private:

	//-----------------------------------------------------------------------
	// Member Variables

	zDBAuthorizeAction				m_action;		// Reason Authorize() was invoked
	ReadOnlyCollection<String^>^	m_args;			// Read-only argument collection
	String^							m_indirect;		// Indirect caller name if present
	zDBAuthorizeResponse			m_response;		// Response from authorization
};

//---------------------------------------------------------------------------
// Class zDBCollationNeededEventArgs
//
// Used as the event argument class for zDBConnection::CollationNeeded
//---------------------------------------------------------------------------

public ref class zDBCollationNeededEventArgs : public EventArgs
{
public:

	//-----------------------------------------------------------------------
	// Properties

	// Encoding
	//
	// Gets the optimal encoding for the required collation
	property zDBCollationEncoding Encoding
	{
		zDBCollationEncoding get(void) { return m_encoding; }
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
	zDBCollationNeededEventArgs(int encoding, const void* name);

private:

	//-----------------------------------------------------------------------
	// Member Variables

	zDBCollationEncoding			m_encoding;		// Requested encoding mode
	String^							m_name;			// Requested collation name
};

//---------------------------------------------------------------------------
// Class zDBProfileEventArgs
//
// Used as the event argument class for zDBConnection::StatementProfile
//---------------------------------------------------------------------------

public ref class zDBProfileEventArgs : public EventArgs
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
	zDBProfileEventArgs(const char* statement, sqlite_uint64 ticks)
	{
		// NOTE: Do not use zDBUtil::FastPtrToStringAnsi here since that
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
// Class zDBRowChangedEventArgs
//
// Used as the event argument class for the zDBConnection hook events that
// expose when a row has been DELETEd, INSERTed or UPDATEd
//---------------------------------------------------------------------------

public ref class zDBRowChangedEventArgs : public EventArgs
{
public:

	//-----------------------------------------------------------------------
	// Properties

	// ChangeType
	//
	// Exposes the type of change that incurred the event
	property zDBRowChangeType ChangeType { zDBRowChangeType get(void) { return m_reason; } }

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
	zDBRowChangedEventArgs(int reason, const char* database, const char* table, __int64 rowid) :
		m_reason(static_cast<zDBRowChangeType>(reason)), m_database(zDBUtil::FastPtrToStringAnsi(database)), 
		m_table(zDBUtil::FastPtrToStringAnsi(table)), m_rowid(rowid) {}

private:

	//-----------------------------------------------------------------------
	// Member Variables

	zDBRowChangeType		m_reason;			// Reason row was changed
	String^					m_database;			// The database name
	String^					m_table;			// The table name
	__int64					m_rowid;			// The ROWID value
};

//---------------------------------------------------------------------------
// Class zDBRowUpdatedEventArgs
//
// Provides a strongly-typed version of the Data::Common::RowUpdatedEventArgs
// class, as is required to implement the zDBDataAdapter properly
//---------------------------------------------------------------------------

public ref class zDBRowUpdatedEventArgs : public RowUpdatedEventArgs
{
public:

	//-----------------------------------------------------------------------
	// Properties

	// Command
	//
	// Casts the weakly typed DbCommand property into a zDBCommand object
	property zDBCommand^ Command 
	{ 
		zDBCommand^ get(void) new { return safe_cast<zDBCommand^>(__super::Command); }
	}

internal:

	// INTERNAL CONSTRUCTOR
	zDBRowUpdatedEventArgs(DataRow^ row, IDbCommand^ cmd, Data::StatementType type, 
		DataTableMapping^ mapping) : RowUpdatedEventArgs(row, cmd, type, mapping) {}
};

//---------------------------------------------------------------------------
// Class zDBRowUpdatingEventArgs
//
// Provides a strongly-typed version of the Data::Common::RowUpdatingEventArgs
// class, as is required to implement the zDBDataAdapter properly
//---------------------------------------------------------------------------

public ref struct zDBRowUpdatingEventArgs : public RowUpdatingEventArgs
{
	zDBRowUpdatingEventArgs(DataRow^ row, IDbCommand^ cmd, Data::StatementType type, 
		DataTableMapping^ mapping) : RowUpdatingEventArgs(row, cmd, type, mapping) {}

	property zDBCommand^ Command
	{
		zDBCommand^ get(void) new { return safe_cast<zDBCommand^>(__super::Command); }
		void set(zDBCommand^ value) { __super::Command = value; }
	}
};

//---------------------------------------------------------------------------
// Class zDBTraceEventArgs
//
// Used as the event argument class for zDBConnection::StatementTrace
//---------------------------------------------------------------------------

public ref class zDBTraceEventArgs : public EventArgs
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
	zDBTraceEventArgs(const char* message)
	{
		// NOTE: Do not use zDBUtil::FastPtrToStringAnsi here since that
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

#endif		// __ZDBROWEVENTARGS_H_
