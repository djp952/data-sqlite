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

#ifndef __ZDBCONNECTIONHOOKS_H_
#define __ZDBCONNECTIONHOOKS_H_
#pragma once

#include "zDBConnectionHook.h"			// Include zDBConnectionHook decls
#include "zDBDelegates.h"				// Include zDB delegate decls
#include "zDBEnumerations.h"			// Include zDB enumeration decls
#include "zDBEventArgs.h"				// Include zDB eventarg declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

using namespace System;
using namespace System::ComponentModel;
using namespace System::Data;
using namespace System::Data::Common;

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// Forward Class Declarations
//---------------------------------------------------------------------------

ref class zDBConnection;				// zDBConnection.h

//---------------------------------------------------------------------------
// Class zDBConnectionAuthorizationHook (internal)
//
// Specializes zDBConnectionHook<T> to implement the authorization hook.
//
// Please review the notes in the base class description for some implementation
// details and explanations about the connection hooks.
//---------------------------------------------------------------------------

ref class zDBConnectionAuthorizationHook sealed : 
	public zDBConnectionHook<zDBAuthorizeEventHandler^, zDBAuthorizeEventArgs^>
{
public:

	//-----------------------------------------------------------------------
	// Constructors

	zDBConnectionAuthorizationHook(zDBConnection^ conn) : 
	  zDBConnectionHook<zDBAuthorizeEventHandler^, zDBAuthorizeEventArgs^>(conn) {}

protected:

	//-----------------------------------------------------------------------
	// Protected Member Functions

	// InstallHook (zDBConnectionHook)
	//
	// Installs the underlying SQLite hook
	virtual void InstallHook(DatabaseHandle* pDatabase, void* context) override;

	// InternalRaise (zDBConnectionHook)
	//
	// Raises the event to all registered delegate handlers
	virtual void InternalRaise(zDBAuthorizeEventHandler^ handler, Object^ sender, 
		zDBAuthorizeEventArgs^ args) override;

	// RemoveHook (zDBConnectionHook)
	//
	// Uninstalls the underlying SQLite hook
	virtual void RemoveHook(DatabaseHandle* pDatabase) override;
};

//---------------------------------------------------------------------------
// Class zDBConnectionCollationNeededHook (internal)
//
// Specializes zDBConnectionHook<T> to implement the collation needed hook.
//
// Please review the notes in the base class description for some implementation
// details and explanations about the connection hooks.
//---------------------------------------------------------------------------

ref class zDBConnectionCollationNeededHook sealed : 
	public zDBConnectionHook<zDBCollationNeededEventHandler^, zDBCollationNeededEventArgs^>
{
public:

	//-----------------------------------------------------------------------
	// Constructors

	zDBConnectionCollationNeededHook(zDBConnection^ conn) : 
	  zDBConnectionHook<zDBCollationNeededEventHandler^, zDBCollationNeededEventArgs^>(conn) {}

protected:

	//-----------------------------------------------------------------------
	// Protected Member Functions

	// InstallHook (zDBConnectionHook)
	//
	// Installs the underlying SQLite hook
	virtual void InstallHook(DatabaseHandle* pDatabase, void* context) override;

	// InternalRaise (zDBConnectionHook)
	//
	// Raises the event to all registered delegate handlers
	virtual void InternalRaise(zDBCollationNeededEventHandler^ handler, Object^ sender, 
		zDBCollationNeededEventArgs^ args) override;

	// RemoveHook (zDBConnectionHook)
	//
	// Uninstalls the underlying SQLite hook
	virtual void RemoveHook(DatabaseHandle* pDatabase) override;
};

//---------------------------------------------------------------------------
// Class zDBConnectionCommitHook (internal)
//
// Specializes zDBConnectionHook<T> to implement the commit hook
//
// Please review the notes in the base class description for some implementation
// details and explanations about the connection hooks.
//---------------------------------------------------------------------------

ref class zDBConnectionCommitHook sealed : 
	public zDBConnectionHook<CancelEventHandler^, CancelEventArgs^>
{
public:

	//-----------------------------------------------------------------------
	// Constructors

	zDBConnectionCommitHook(zDBConnection^ conn) : 
	  zDBConnectionHook<CancelEventHandler^, CancelEventArgs^>(conn) {}

protected:

	//-----------------------------------------------------------------------
	// Protected Member Functions

	// InstallHook (zDBConnectionHook)
	//
	// Installs the underlying SQLite hook
	virtual void InstallHook(DatabaseHandle* pDatabase, void* context) override;

	// InternalRaise (zDBConnectionHook)
	//
	// Raises the event to all registered delegate handlers
	virtual void InternalRaise(CancelEventHandler^ handler, Object^ sender, 
		CancelEventArgs^ args) override;

	// RemoveHook (zDBConnectionHook)
	//
	// Uninstalls the underlying SQLite hook
	virtual void RemoveHook(DatabaseHandle* pDatabase) override;

};

//---------------------------------------------------------------------------
// Class zDBConnectionProgressHook (internal)
//
// Specializes zDBConnectionHook<T> to implement the progress hook. Note that
// this class is slightly different than the others because you can configure
// the number of opcodes executed between invocations.  It has to keep track
// of if the handler is installed or not on it's own.
//
// Please review the notes in the base class description for some implementation
// details and explanations about the connection hooks.
//---------------------------------------------------------------------------

ref class zDBConnectionProgressHook sealed : 
	public zDBConnectionHook<CancelEventHandler^, CancelEventArgs^>
{
public:

	//-----------------------------------------------------------------------
	// Constructors

	zDBConnectionProgressHook(zDBConnection^ conn) : 
		zDBConnectionHook<CancelEventHandler^, CancelEventArgs^>(conn), 
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

	// InstallHook (zDBConnectionHook)
	//
	// Installs the underlying SQLite hook
	virtual void InstallHook(DatabaseHandle* pDatabase, void* context) override;

	// InternalRaise (zDBConnectionHook)
	//
	// Raises the event to all registered delegate handlers
	virtual void InternalRaise(CancelEventHandler^ handler, Object^ sender, 
		CancelEventArgs^ args) override;

	// RemoveHook (zDBConnectionHook)
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
// Class zDBConnectionRollbackHook (internal)
//
// Specializes zDBConnectionHook<T> to implement the rollback hook
//
// Please review the notes in the base class description for some implementation
// details and explanations about the connection hooks.
//---------------------------------------------------------------------------

ref class zDBConnectionRollbackHook sealed : 
	public zDBConnectionHook<EventHandler^, EventArgs^>
{
public:

	//-----------------------------------------------------------------------
	// Constructors

	zDBConnectionRollbackHook(zDBConnection^ conn) : 
	  zDBConnectionHook<EventHandler^, EventArgs^>(conn) {}

protected:

	//-----------------------------------------------------------------------
	// Protected Member Functions

	// InstallHook (zDBConnectionHook)
	//
	// Installs the underlying SQLite hook
	virtual void InstallHook(DatabaseHandle* pDatabase, void* context) override;

	// InternalRaise (zDBConnectionHook)
	//
	// Raises the event to all registered delegate handlers
	virtual void InternalRaise(EventHandler^ handler, Object^ sender, 
		EventArgs^ args) override;

	// RemoveHook (zDBConnectionHook)
	//
	// Uninstalls the underlying SQLite hook
	virtual void RemoveHook(DatabaseHandle* pDatabase) override;
};

//---------------------------------------------------------------------------
// Class zDBConnectionProfileHook (internal)
//
// Specializes zDBConnectionHook<T> to implement the statement trace hook
//
// Please review the notes in the base class description for some implementation
// details and explanations about the connection hooks.
//---------------------------------------------------------------------------

ref class zDBConnectionProfileHook sealed : 
	public zDBConnectionHook<zDBProfileEventHandler^, zDBProfileEventArgs^>
{
public:

	//-----------------------------------------------------------------------
	// Constructors

	zDBConnectionProfileHook(zDBConnection^ conn) : 
		zDBConnectionHook<zDBProfileEventHandler^, zDBProfileEventArgs^>(conn) {}

protected:

	//-----------------------------------------------------------------------
	// Protected Member Functions

	// InstallHook (zDBConnectionHook)
	//
	// Installs the underlying SQLite hook
	virtual void InstallHook(DatabaseHandle* pDatabase, void* context) override;

	// InternalRaise (zDBConnectionHook)
	//
	// Raises the event to all registered delegate handlers
	virtual void InternalRaise(zDBProfileEventHandler^ handler, Object^ sender, \
		zDBProfileEventArgs^ args) override;

	// RemoveHook (zDBConnectionHook)
	//
	// Uninstalls the underlying SQLite hook
	virtual void RemoveHook(DatabaseHandle* pDatabase) override;
};

//---------------------------------------------------------------------------
// Class zDBConnectionTraceHook (internal)
//
// Specializes zDBConnectionHook<T> to implement the statement trace hook
//
// Please review the notes in the base class description for some implementation
// details and explanations about the connection hooks.
//---------------------------------------------------------------------------

ref class zDBConnectionTraceHook sealed : 
	public zDBConnectionHook<zDBTraceEventHandler^, zDBTraceEventArgs^>
{
public:

	//-----------------------------------------------------------------------
	// Constructors

	zDBConnectionTraceHook(zDBConnection^ conn) : 
		zDBConnectionHook<zDBTraceEventHandler^, zDBTraceEventArgs^>(conn) {}

protected:

	//-----------------------------------------------------------------------
	// Protected Member Functions

	// InstallHook (zDBConnectionHook)
	//
	// Installs the underlying SQLite hook
	virtual void InstallHook(DatabaseHandle* pDatabase, void* context) override;

	// InternalRaise (zDBConnectionHook)
	//
	// Raises the event to all registered delegate handlers
	virtual void InternalRaise(zDBTraceEventHandler^ handler, Object^ sender, \
		zDBTraceEventArgs^ args) override;

	// RemoveHook (zDBConnectionHook)
	//
	// Uninstalls the underlying SQLite hook
	virtual void RemoveHook(DatabaseHandle* pDatabase) override;
};

//---------------------------------------------------------------------------
// Class zDBConnectionUpdateHook (internal)
//
// Specializes zDBConnectionHook<T> to implement the row update hook
//
// Please review the notes in the base class description for some implementation
// details and explanations about the connection hooks.
//---------------------------------------------------------------------------

ref class zDBConnectionUpdateHook sealed : 
	public zDBConnectionHook<zDBRowChangedEventHandler^, zDBRowChangedEventArgs^>
{
public:

	//-----------------------------------------------------------------------
	// Constructors

	zDBConnectionUpdateHook(zDBConnection^ conn) : 
		zDBConnectionHook<zDBRowChangedEventHandler^, zDBRowChangedEventArgs^>(conn) {}

protected:

	//-----------------------------------------------------------------------
	// Protected Member Functions

	// InstallHook (zDBConnectionHook)
	//
	// Installs the underlying SQLite hook
	virtual void InstallHook(DatabaseHandle* pDatabase, void* context) override;

	// InternalRaise (zDBConnectionHook)
	//
	// Raises the event to all registered delegate handlers
	virtual void InternalRaise(zDBRowChangedEventHandler^ handler, Object^ sender, 
		zDBRowChangedEventArgs^ args) override;

	// RemoveHook (zDBConnectionHook)
	//
	// Uninstalls the underlying SQLite hook
	virtual void RemoveHook(DatabaseHandle* pDatabase) override;
};

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)

#endif		// __ZDBCONNECTIONHOOKS_H_
