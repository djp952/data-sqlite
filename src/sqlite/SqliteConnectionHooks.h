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

#ifndef __SQLITECONNECTIONHOOKS_H_
#define __SQLITECONNECTIONHOOKS_H_
#pragma once

#include "SqliteConnectionHook.h"			// Include SqliteConnectionHook decls
#include "SqliteDelegates.h"				// Include Sqlite delegate decls
#include "SqliteEnumerations.h"			// Include Sqlite enumeration decls
#include "SqliteEventArgs.h"				// Include Sqlite eventarg declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

using namespace System;
using namespace System::ComponentModel;
using namespace System::Data;
using namespace System::Data::Common;

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// Forward Class Declarations
//---------------------------------------------------------------------------

ref class SqliteConnection;				// SqliteConnection.h

//---------------------------------------------------------------------------
// Class SqliteConnectionAuthorizationHook (internal)
//
// Specializes SqliteConnectionHook<T> to implement the authorization hook.
//
// Please review the notes in the base class description for some implementation
// details and explanations about the connection hooks.
//---------------------------------------------------------------------------

ref class SqliteConnectionAuthorizationHook sealed : 
	public SqliteConnectionHook<SqliteAuthorizeEventHandler^, SqliteAuthorizeEventArgs^>
{
public:

	//-----------------------------------------------------------------------
	// Constructors

	SqliteConnectionAuthorizationHook(SqliteConnection^ conn) : 
	  SqliteConnectionHook<SqliteAuthorizeEventHandler^, SqliteAuthorizeEventArgs^>(conn) {}

protected:

	//-----------------------------------------------------------------------
	// Protected Member Functions

	// InstallHook (SqliteConnectionHook)
	//
	// Installs the underlying SQLite hook
	virtual void InstallHook(DatabaseHandle* pDatabase, void* context) override;

	// InternalRaise (SqliteConnectionHook)
	//
	// Raises the event to all registered delegate handlers
	virtual void InternalRaise(SqliteAuthorizeEventHandler^ handler, Object^ sender, 
		SqliteAuthorizeEventArgs^ args) override;

	// RemoveHook (SqliteConnectionHook)
	//
	// Uninstalls the underlying SQLite hook
	virtual void RemoveHook(DatabaseHandle* pDatabase) override;
};

//---------------------------------------------------------------------------
// Class SqliteConnectionCollationNeededHook (internal)
//
// Specializes SqliteConnectionHook<T> to implement the collation needed hook.
//
// Please review the notes in the base class description for some implementation
// details and explanations about the connection hooks.
//---------------------------------------------------------------------------

ref class SqliteConnectionCollationNeededHook sealed : 
	public SqliteConnectionHook<SqliteCollationNeededEventHandler^, SqliteCollationNeededEventArgs^>
{
public:

	//-----------------------------------------------------------------------
	// Constructors

	SqliteConnectionCollationNeededHook(SqliteConnection^ conn) : 
	  SqliteConnectionHook<SqliteCollationNeededEventHandler^, SqliteCollationNeededEventArgs^>(conn) {}

protected:

	//-----------------------------------------------------------------------
	// Protected Member Functions

	// InstallHook (SqliteConnectionHook)
	//
	// Installs the underlying SQLite hook
	virtual void InstallHook(DatabaseHandle* pDatabase, void* context) override;

	// InternalRaise (SqliteConnectionHook)
	//
	// Raises the event to all registered delegate handlers
	virtual void InternalRaise(SqliteCollationNeededEventHandler^ handler, Object^ sender, 
		SqliteCollationNeededEventArgs^ args) override;

	// RemoveHook (SqliteConnectionHook)
	//
	// Uninstalls the underlying SQLite hook
	virtual void RemoveHook(DatabaseHandle* pDatabase) override;
};

//---------------------------------------------------------------------------
// Class SqliteConnectionCommitHook (internal)
//
// Specializes SqliteConnectionHook<T> to implement the commit hook
//
// Please review the notes in the base class description for some implementation
// details and explanations about the connection hooks.
//---------------------------------------------------------------------------

ref class SqliteConnectionCommitHook sealed : 
	public SqliteConnectionHook<CancelEventHandler^, CancelEventArgs^>
{
public:

	//-----------------------------------------------------------------------
	// Constructors

	SqliteConnectionCommitHook(SqliteConnection^ conn) : 
	  SqliteConnectionHook<CancelEventHandler^, CancelEventArgs^>(conn) {}

protected:

	//-----------------------------------------------------------------------
	// Protected Member Functions

	// InstallHook (SqliteConnectionHook)
	//
	// Installs the underlying SQLite hook
	virtual void InstallHook(DatabaseHandle* pDatabase, void* context) override;

	// InternalRaise (SqliteConnectionHook)
	//
	// Raises the event to all registered delegate handlers
	virtual void InternalRaise(CancelEventHandler^ handler, Object^ sender, 
		CancelEventArgs^ args) override;

	// RemoveHook (SqliteConnectionHook)
	//
	// Uninstalls the underlying SQLite hook
	virtual void RemoveHook(DatabaseHandle* pDatabase) override;

};

//---------------------------------------------------------------------------
// Class SqliteConnectionProgressHook (internal)
//
// Specializes SqliteConnectionHook<T> to implement the progress hook. Note that
// this class is slightly different than the others because you can configure
// the number of opcodes executed between invocations.  It has to keep track
// of if the handler is installed or not on it's own.
//
// Please review the notes in the base class description for some implementation
// details and explanations about the connection hooks.
//---------------------------------------------------------------------------

ref class SqliteConnectionProgressHook sealed : 
	public SqliteConnectionHook<CancelEventHandler^, CancelEventArgs^>
{
public:

	//-----------------------------------------------------------------------
	// Constructors

	SqliteConnectionProgressHook(SqliteConnection^ conn) : 
		SqliteConnectionHook<CancelEventHandler^, CancelEventArgs^>(conn), 
		m_freq(DEFAULT_FREQUENCY) {}

	//-----------------------------------------------------------------------
	// Properties

	// Frequency
	//
	// Determines the frequency at which the progress callback event is fired,
	// based on the number of virtual machine opcodes that are executed.  Lower
	// numbers indicate a more frequent callback
	property int Frequency
	{
		int get(void) { return m_freq; }
		void set(int value);
	}

protected:

	//-----------------------------------------------------------------------
	// Protected Member Functions

	// InstallHook (SqliteConnectionHook)
	//
	// Installs the underlying SQLite hook
	virtual void InstallHook(DatabaseHandle* pDatabase, void* context) override;

	// InternalRaise (SqliteConnectionHook)
	//
	// Raises the event to all registered delegate handlers
	virtual void InternalRaise(CancelEventHandler^ handler, Object^ sender, 
		CancelEventArgs^ args) override;

	// RemoveHook (SqliteConnectionHook)
	//
	// Uninstalls the underlying SQLite hook
	virtual void RemoveHook(DatabaseHandle* pDatabase) override;

private:

	//-----------------------------------------------------------------------
	// Private Constants

	// DEFAULT_FREQUENCY
	//
	// The default frequency at which the progress handler will be invoked.
	// TODO: This value is a wild guess and currently has no bearing on reality
	literal int DEFAULT_FREQUENCY = 35;

	//-----------------------------------------------------------------------
	// Member Variables

	sqlite3*				m_hDatabase;		// Cached database handle
	void*					m_context;			// Cached context pointer
	int						m_freq;				// Callback frequency
};

//---------------------------------------------------------------------------
// Class SqliteConnectionRollbackHook (internal)
//
// Specializes SqliteConnectionHook<T> to implement the rollback hook
//
// Please review the notes in the base class description for some implementation
// details and explanations about the connection hooks.
//---------------------------------------------------------------------------

ref class SqliteConnectionRollbackHook sealed : 
	public SqliteConnectionHook<EventHandler^, EventArgs^>
{
public:

	//-----------------------------------------------------------------------
	// Constructors

	SqliteConnectionRollbackHook(SqliteConnection^ conn) : 
	  SqliteConnectionHook<EventHandler^, EventArgs^>(conn) {}

protected:

	//-----------------------------------------------------------------------
	// Protected Member Functions

	// InstallHook (SqliteConnectionHook)
	//
	// Installs the underlying SQLite hook
	virtual void InstallHook(DatabaseHandle* pDatabase, void* context) override;

	// InternalRaise (SqliteConnectionHook)
	//
	// Raises the event to all registered delegate handlers
	virtual void InternalRaise(EventHandler^ handler, Object^ sender, 
		EventArgs^ args) override;

	// RemoveHook (SqliteConnectionHook)
	//
	// Uninstalls the underlying SQLite hook
	virtual void RemoveHook(DatabaseHandle* pDatabase) override;
};

//---------------------------------------------------------------------------
// Class SqliteConnectionProfileHook (internal)
//
// Specializes SqliteConnectionHook<T> to implement the statement trace hook
//
// Please review the notes in the base class description for some implementation
// details and explanations about the connection hooks.
//---------------------------------------------------------------------------

ref class SqliteConnectionProfileHook sealed : 
	public SqliteConnectionHook<SqliteProfileEventHandler^, SqliteProfileEventArgs^>
{
public:

	//-----------------------------------------------------------------------
	// Constructors

	SqliteConnectionProfileHook(SqliteConnection^ conn) : 
		SqliteConnectionHook<SqliteProfileEventHandler^, SqliteProfileEventArgs^>(conn) {}

protected:

	//-----------------------------------------------------------------------
	// Protected Member Functions

	// InstallHook (SqliteConnectionHook)
	//
	// Installs the underlying SQLite hook
	virtual void InstallHook(DatabaseHandle* pDatabase, void* context) override;

	// InternalRaise (SqliteConnectionHook)
	//
	// Raises the event to all registered delegate handlers
	virtual void InternalRaise(SqliteProfileEventHandler^ handler, Object^ sender, \
		SqliteProfileEventArgs^ args) override;

	// RemoveHook (SqliteConnectionHook)
	//
	// Uninstalls the underlying SQLite hook
	virtual void RemoveHook(DatabaseHandle* pDatabase) override;
};

//---------------------------------------------------------------------------
// Class SqliteConnectionTraceHook (internal)
//
// Specializes SqliteConnectionHook<T> to implement the statement trace hook
//
// Please review the notes in the base class description for some implementation
// details and explanations about the connection hooks.
//---------------------------------------------------------------------------

ref class SqliteConnectionTraceHook sealed : 
	public SqliteConnectionHook<SqliteTraceEventHandler^, SqliteTraceEventArgs^>
{
public:

	//-----------------------------------------------------------------------
	// Constructors

	SqliteConnectionTraceHook(SqliteConnection^ conn) : 
		SqliteConnectionHook<SqliteTraceEventHandler^, SqliteTraceEventArgs^>(conn) {}

protected:

	//-----------------------------------------------------------------------
	// Protected Member Functions

	// InstallHook (SqliteConnectionHook)
	//
	// Installs the underlying SQLite hook
	virtual void InstallHook(DatabaseHandle* pDatabase, void* context) override;

	// InternalRaise (SqliteConnectionHook)
	//
	// Raises the event to all registered delegate handlers
	virtual void InternalRaise(SqliteTraceEventHandler^ handler, Object^ sender, \
		SqliteTraceEventArgs^ args) override;

	// RemoveHook (SqliteConnectionHook)
	//
	// Uninstalls the underlying SQLite hook
	virtual void RemoveHook(DatabaseHandle* pDatabase) override;
};

//---------------------------------------------------------------------------
// Class SqliteConnectionUpdateHook (internal)
//
// Specializes SqliteConnectionHook<T> to implement the row update hook
//
// Please review the notes in the base class description for some implementation
// details and explanations about the connection hooks.
//---------------------------------------------------------------------------

ref class SqliteConnectionUpdateHook sealed : 
	public SqliteConnectionHook<SqliteRowChangedEventHandler^, SqliteRowChangedEventArgs^>
{
public:

	//-----------------------------------------------------------------------
	// Constructors

	SqliteConnectionUpdateHook(SqliteConnection^ conn) : 
		SqliteConnectionHook<SqliteRowChangedEventHandler^, SqliteRowChangedEventArgs^>(conn) {}

protected:

	//-----------------------------------------------------------------------
	// Protected Member Functions

	// InstallHook (SqliteConnectionHook)
	//
	// Installs the underlying SQLite hook
	virtual void InstallHook(DatabaseHandle* pDatabase, void* context) override;

	// InternalRaise (SqliteConnectionHook)
	//
	// Raises the event to all registered delegate handlers
	virtual void InternalRaise(SqliteRowChangedEventHandler^ handler, Object^ sender, 
		SqliteRowChangedEventArgs^ args) override;

	// RemoveHook (SqliteConnectionHook)
	//
	// Uninstalls the underlying SQLite hook
	virtual void RemoveHook(DatabaseHandle* pDatabase) override;
};

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)

#endif		// __SQLITECONNECTIONHOOKS_H_
