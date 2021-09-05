//---------------------------------------------------------------------------
// Copyright (c) 2008-2021 Michael G. Brehm
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

#ifndef __ZDBCONNECTIONHOOK_H_
#define __ZDBCONNECTIONHOOK_H_
#pragma once

#include "DatabaseHandle.h"				// Include DatabaseHandle declarations
#include "ITrackableObject.h"			// Include ITrackableObject declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings
#pragma warning(disable:4100)			// "unreferenced formal parameter"

using namespace System;
using namespace System::Data;
using namespace System::Runtime::InteropServices;
using namespace System::Threading;

BEGIN_NAMESPACE(zuki)
BEGIN_NAMESPACE(data)
BEGIN_NAMESPACE(dbms)

//---------------------------------------------------------------------------
// Forward Class Declarations
//---------------------------------------------------------------------------

ref class zDBConnection;				// zDBConnection.h

//---------------------------------------------------------------------------
// Class zDBConnectionHook (internal)
//
// zDBConnectionHook implements the base class that the specific hook classes
// derive from.  There is simply too much code in the main zDBConnection.cpp
// file, and breaking all the hooks out into separate classes makes it more
// managable, even if it's a little more convoluted.
//
// Note: The Connection property was added for convenience so it can be the
// "sender" argument when raising events.  It's not used for anything important,
// although it was at one point.  I was too concerned with having to add all
// the proper GC::KeepAlive() calls and whatnot since it's based on a weak
// reference.  Using the DatabaseHandle is much more deterministic.
//---------------------------------------------------------------------------

generic<typename _handler, typename _args>
where _handler : Delegate
ref class zDBConnectionHook abstract : public ITrackableObject
{
public:

	//-----------------------------------------------------------------------
	// Constructors

	zDBConnectionHook(zDBConnection^ conn);

	//-----------------------------------------------------------------------
	// Member Functions

	// Add
	//
	// Adds a new handler to the delegate, and installs the hook if this is
	// the first handler to be registered
	void Add(_handler handler);

	// IsDisposed (ITrackableObject)
	//
	// Exposes this object's disposal state
	virtual bool IsDisposed(void) { return m_disposed; }

	// OnCloseConnection
	//
	// Called when the connection is closed to uninstall the hook from the
	// underlying SQLite database, if handlers are registered
	void OnCloseConnection(void);

	// OnOpenConnection
	//
	// Called when the connection is opened to install the hook into the
	// underlying SQLite database, if handlers are registered
	void OnOpenConnection(DatabaseHandle* pDatabase);

	// Raise
	//
	// Raises the event to all registered delegate handlers
	void Raise(_args args);

	// Remove
	//
	// Removes an existing handler from the delegate, and uninstalls the
	// hook if that was the final handler registered
	void Remove(_handler handler);

protected:

	//-----------------------------------------------------------------------
	// Protected Member Functions

	// InstallHook
	//
	// Must be implemnented in the derived class to install the hook
	virtual void InstallHook(DatabaseHandle* pDatabase, void* context) abstract;

	// InternalRaise
	//
	// Must be implemented in the derived class to raise the event, since we
	// can't do it without the ever-so-slow DynamicInvoke in the base class
	virtual void InternalRaise(_handler handler, Object^ sender, _args args) abstract;

	// RemoveHook
	//
	// Must be implemented in the derived class to remove the hook
	virtual void RemoveHook(DatabaseHandle* pDatabase) abstract;

private:

	// DESTRUCTOR / FINALIZER
	~zDBConnectionHook() { this->!zDBConnectionHook(); m_disposed = true; }
	!zDBConnectionHook();

	//-----------------------------------------------------------------------
	// Private Member Functions

	// GetConnection
	//
	// Casts the weak GCHandle back into a zDBConnection to send along as
	// the sender of the event
	zDBConnection^ GetConnection(void);

	//-----------------------------------------------------------------------
	// Member Variables

	bool				m_disposed;			// Object disposal flag
	_handler			m_delegate;			// Contained delegate object
	GCHandle			m_weakconn;			// WEAK reference to the connection
	GCHandle			m_weakthis;			// WEAK reference to ourselves
	DatabaseHandle*		m_pDatabase;		// Underlying database handle
	int					m_cRef;				// Delegate reference count
};

//---------------------------------------------------------------------------

END_NAMESPACE(dbms)
END_NAMESPACE(data)
END_NAMESPACE(zuki)

#pragma warning(pop)

#endif		// __ZDBCONNECTIONHOOK_H_
