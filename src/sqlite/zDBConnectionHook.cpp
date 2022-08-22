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

#include "stdafx.h"					// Include project pre-compiled headers
#include "zDBConnectionHook.h"		// Include zDBConnectionHook declarations
#include "zDBConnection.h"			// Include zDBConnection declarations

#pragma warning(push, 4)			// Enable maximum compiler warnings

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// zDBConnectionHook Constructor
//
// Arguments:
//
//	conn		- Reference to the parent zDBConnection object

generic<typename _handler, typename _args>
zDBConnectionHook<_handler, _args>::zDBConnectionHook(zDBConnection^ conn)
{
	m_weakthis = GCHandle::Alloc(this, GCHandleType::Weak);
	m_weakconn = GCHandle::Alloc(conn, GCHandleType::Weak);
}

//---------------------------------------------------------------------------
// zDBConnectionHook Finalizer

generic<typename _handler, typename _args>
void zDBConnectionHook<_handler, _args>::!zDBConnectionHook()
{
	m_weakconn.Free();							// Release the GCHandle
	m_weakthis.Free();							// Release the GCHandle

	if(m_pDatabase) m_pDatabase->Release(this);
	m_pDatabase = NULL;
}

//---------------------------------------------------------------------------
// zDBConnectionHook::Add
//
// Adds a new handler to the contained delegate.  If this is the first handler
// and we have a connection to work with, will also install the hook
//
// Arguments:
//
//	handler		- The new handler to register against the delegate

generic<typename _handler, typename _args>
void zDBConnectionHook<_handler, _args>::Add(_handler handler)
{
	_handler			newDelegate;			// The new delegate object
	int					ref;					// The new delegate ref count

	CHECK_DISPOSED(m_disposed);

	// Combine the existing delegate with the new handler and store it 
	// locally so we can detect if anything about the delegate changed

	newDelegate = safe_cast<_handler>(Delegate::Combine(safe_cast<Delegate^>(m_delegate), 
		safe_cast<Delegate^>(handler)));

	if(newDelegate == m_delegate) return;		// No changes to delegate
	else m_delegate = newDelegate;				// Replace existing delegate

	ref = Interlocked::Increment(m_cRef);		// Increment the reference count

	// If this is the FIRST reference added to the delegate, and the database
	// handle has been set already, install the underlying database hook

	if((ref == 1) && (m_pDatabase)) 
		InstallHook(m_pDatabase, GCHandle::ToIntPtr(m_weakthis).ToPointer());
}

//---------------------------------------------------------------------------
// zDBConnectionHook::GetConnection (private)
//
// Gets a reference to the parent zDBConnection object, or nullptr if the
// connection has been disposed of or finalized already

generic<typename _handler, typename _args>
zDBConnection^ zDBConnectionHook<_handler, _args>::GetConnection(void)
{
	CHECK_DISPOSED(m_disposed);
	
	if(!m_weakconn.IsAllocated) return nullptr;
	else return safe_cast<zDBConnection^>(m_weakconn.Target);
}

//---------------------------------------------------------------------------
// zDBConnectionHook::OnCloseConnection
//
// Removes the hook from the database to prevent any more events from firing,
// but does not remove the event handlers themselves
//
// Arguments:
//
//	NONE

generic<typename _handler, typename _args>
void zDBConnectionHook<_handler, _args>::OnCloseConnection(void)
{
	CHECK_DISPOSED(m_disposed);

	// If there are any outstanding handlers hooked into the delegate,
	// and there was an active database connection, remove the hook

	if((m_cRef > 0) && (m_pDatabase)) RemoveHook(m_pDatabase);

	if(m_pDatabase) m_pDatabase->Release(this);
	m_pDatabase = NULL;
}

//---------------------------------------------------------------------------
// zDBConnectionHook::OnOpenConnection
//
// Automatically installs the hook if there are event handlers already
// registered against the contained delegate
//
// Arguments:
//
//	pDatabase		- Pointer to the new DatabaseHandle to use

generic<typename _handler, typename _args>
void zDBConnectionHook<_handler, _args>::OnOpenConnection(DatabaseHandle* pDatabase)
{
	CHECK_DISPOSED(m_disposed);
	if(!pDatabase) throw gcnew ArgumentNullException();

	Debug::Assert(m_pDatabase == NULL);			// Should always be ...
	if(m_pDatabase) OnCloseConnection();		// ... but just in case

	m_pDatabase = pDatabase;					// Save instance pointer
	m_pDatabase->AddRef(this);					// We're keeping this

	// If there are already references against the delegate, install the hook.
	// Otherwise this will wait for a handler to actually be set in Add()

	if(m_cRef > 0) InstallHook(m_pDatabase, GCHandle::ToIntPtr(m_weakthis).ToPointer());
}

//---------------------------------------------------------------------------
// zDBConnectionHook::Raise
//
// Raises the event to all registered delegate handlers
//
// Arguments:
//
//	args		- Hook-specific event arguments

generic<typename _handler, typename _args>
void zDBConnectionHook<_handler, _args>::Raise(_args args)
{
	zDBConnection^			sender;			// Reference to the parent connection

	CHECK_DISPOSED(m_disposed);
	
	Debug::Assert(m_delegate != nullptr);		// Should never happen
	if(m_delegate == nullptr) return;			// Can't raise a NULL event

	sender = GetConnection();					// Cast back out the connection
	InternalRaise(m_delegate, sender, args);	// Raise the event
	
	GC::KeepAlive(sender);						// Keep the object alive
}

//---------------------------------------------------------------------------
// zDBConnectionHook::Remove
//
// Removes an existing handler from the contained delegate.  If this is the
// last handler and we have a connection to work with, will also remove the hook
//
// Arguments:
//
//	handler		- The existing handler to remove from the delegate

generic<typename _handler, typename _args>
void zDBConnectionHook<_handler, _args>::Remove(_handler handler)
{
	_handler			newDelegate;			// The new delegate object
	int					ref;					// The new delegate ref count

	CHECK_DISPOSED(m_disposed);

	// Combine the existing delegate with the new handler and store it 
	// locally so we can detect if anything about the delegate changed

	newDelegate = safe_cast<_handler>(Delegate::Remove(safe_cast<Delegate^>(m_delegate),
		safe_cast<Delegate^>(handler)));

	if(newDelegate == m_delegate) return;		// No changes to delegate
	else m_delegate = newDelegate;				// Replace existing delegate

	ref = Interlocked::Decrement(m_cRef);		// Increment the reference count
	Debug::Assert(ref >= 0);					// Should never go below zero

	// If this is the LAST reference to the delegate, and the database handle
	// is still set, remove the underlying database hook

	if((ref == 0) && (m_pDatabase)) RemoveHook(m_pDatabase);
}

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)
