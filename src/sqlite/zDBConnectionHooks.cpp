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
#include "zDBConnectionHooks.h"		// Include zDB connection hooks

#pragma warning(push, 4)			// Enable maximum compiler warnings
#pragma warning(disable:4100)		// "unreferenced formal parameter"

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// ZDBCONNECTIONAUTHORIZERHOOK IMPLEMENTATION
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// zdbconnection_authorizer_hook
//
// Unmanaged callback function for the sqlite3_set_authorizer handler, which
// ends up getting translated into a zDBConnection event in managed code
//
// Arguments:
//
//	context		- Context pointer passed into sqlite3_set_authorizer
//	action		- Action that needs to be authorized
//	resource1	- First argument into the authorizer
//	resource2	- Second argument into the authorizer
//	resource3	- Third argument into the authorizer
//	indirect	- Name of trigger or view indirectly accessing something

static int zdbconnection_authorizer_hook(void* context, int action, const char* resource1,
	const char* resource2, const char* resource3, const char* indirect)
{
	GCHandleRef<zDBConnectionAuthorizationHook^>	hook(context);	// Hook instance
	gcroot<zDBAuthorizeEventArgs^>					args;			// Hook arguments

	// Don't even try to touch anything if the GCHandle is no longer allocated,
	// the object reference is NULL, or the connection has been disposed of

	Debug::Assert(hook != nullptr);
	if((!hook.IsAllocated) || (hook == nullptr) || (hook->IsDisposed())) return SQLITE_DENY;

	args = gcnew zDBAuthorizeEventArgs(action, resource1, resource2, resource3, indirect);

	// Convert the unmanaged ANSI string and raise the event into .NET land, and
	// just deny the operation to occur if an exception happens.  When dealing with
	// security, I think it's better to fail than silently succeed.

	try { hook->Raise(args); }
	catch(Exception^) { return SQLITE_DENY; }

	return static_cast<int>(args->Response);
}

//---------------------------------------------------------------------------
// zDBConnectionAuthorizationHook::InstallHook (protected)
//
// Installs the underlying SQLite database hook
//
// Arguments:
//
//	pDatabase		- SQLite database handle wrapper instance
//	context			- Context pointer to register with the hook

void zDBConnectionAuthorizationHook::InstallHook(DatabaseHandle* pDatabase, void* context)
{
	if(!pDatabase) throw gcnew ArgumentNullException();

	sqlite3_set_authorizer(pDatabase->Handle, zdbconnection_authorizer_hook, context);

#ifdef ZDB_TRACE_CONNECTIONHOOKS
	Debug::WriteLine(String::Format("zDBConnectionAuthorizationHook 0x{0:X} installed.",
		IntPtr(pDatabase)));
#endif
}

//---------------------------------------------------------------------------
// zDBConnectionAuthorizationHook::InternalRaise (protected)
//
// Raises the database hook as a managed event
//
// Arguments:
//
//	handler		- The delegate handler
//	sender		- Sender to specify for the event
//	args		- Event arguments

void zDBConnectionAuthorizationHook::InternalRaise(zDBAuthorizeEventHandler^ handler, 
	Object^ sender, zDBAuthorizeEventArgs^ args)
{
	if(handler != nullptr) handler(sender, args);
}

//---------------------------------------------------------------------------
// zDBConnectionAuthorizationHook::RemoveHook (protected)
//
// Removes the underlying SQLite database hook
//
// Arguments:
//
//	pDatabase		- SQLite database handle wrapper instance

void zDBConnectionAuthorizationHook::RemoveHook(DatabaseHandle* pDatabase)
{
	if(!pDatabase) throw gcnew ArgumentNullException();

	sqlite3_set_authorizer(pDatabase->Handle, NULL, NULL);

#ifdef ZDB_TRACE_CONNECTIONHOOKS
	Debug::WriteLine(String::Format("zDBConnectionAuthorizationHook 0x{0:X} removed.",
		IntPtr(pDatabase)));
#endif
}

//---------------------------------------------------------------------------
// ZDBCONNECTIONCOLLATIONNEEDEDHOOK IMPLEMENTATION
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// zdbconnection_collation_hook
//
// Unmanaged callback function for the sqlite3_collation_needed handler, which
// ends up getting translated into a zDBConnection event in managed code
//
// Arguments:
//
//	context		- Context pointer passed into sqlite3_set_authorizer
//	hDatabase	- Database handle for the current connection
//	encoding	- Requested encoding of the collation
//	name		- Required name of the collation

static void zdbconnection_collation_hook(void* context, sqlite3* hDatabase, int encoding,
	const void* name)
{
	GCHandleRef<zDBConnectionCollationNeededHook^>	hook(context);	// Hook instance
	gcroot<zDBCollationNeededEventArgs^>			args;			// Hook arguments

	// Don't even try to touch anything if the GCHandle is no longer allocated,
	// the object reference is NULL, or the connection has been disposed of

	Debug::Assert(hook != nullptr);
	if((!hook.IsAllocated) || (hook == nullptr) || (hook->IsDisposed())) return;

	args = gcnew zDBCollationNeededEventArgs(encoding, name);

	// Raise the event into the managed universe, and if it fails, well, there
	// is no mechanism for reporting that so just hide it.  Bummer, I know.

	try { hook->Raise(args); }
	catch(Exception^) { /* DO NOTHING */ }
}

//---------------------------------------------------------------------------
// zDBConnectionCollationNeededHook::InstallHook (protected)
//
// Installs the underlying SQLite database hook
//
// Arguments:
//
//	pDatabase		- SQLite database handle wrapper instance
//	context			- Context pointer to register with the hook

void zDBConnectionCollationNeededHook::InstallHook(DatabaseHandle* pDatabase, void* context)
{
	if(!pDatabase) throw gcnew ArgumentNullException();

	sqlite3_collation_needed16(pDatabase->Handle, context, zdbconnection_collation_hook);

#ifdef ZDB_TRACE_CONNECTIONHOOKS
	Debug::WriteLine(String::Format("zDBConnectionCollationNeededHook 0x{0:X} installed.",
		IntPtr(pDatabase)));
#endif
}

//---------------------------------------------------------------------------
// zDBConnectionCollationNeededHook::InternalRaise (protected)
//
// Raises the database hook as a managed event
//
// Arguments:
//
//	handler		- The delegate handler
//	sender		- Sender to specify for the event
//	args		- Event arguments

void zDBConnectionCollationNeededHook::InternalRaise(zDBCollationNeededEventHandler^ handler, 
	Object^ sender, zDBCollationNeededEventArgs^ args)
{
	if(handler != nullptr) handler(sender, args);
}

//---------------------------------------------------------------------------
// zDBConnectionCollationNeededHook::RemoveHook (protected)
//
// Removes the underlying SQLite database hook
//
// Arguments:
//
//	pDatabase		- SQLite database handle wrapper instance

void zDBConnectionCollationNeededHook::RemoveHook(DatabaseHandle* pDatabase)
{
	if(!pDatabase) throw gcnew ArgumentNullException();
	
	sqlite3_collation_needed16(pDatabase->Handle, NULL, NULL);

#ifdef ZDB_TRACE_CONNECTIONHOOKS
	Debug::WriteLine(String::Format("zDBConnectionCollationNeededHook 0x{0:X} removed.",
		IntPtr(pDatabase)));
#endif
}

//---------------------------------------------------------------------------
// ZDBCONNECTIONCOMMITHOOK IMPLEMENTATION
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// zdbconnection_commit_hook
//
// Unmanaged callback function for the sqlite3_commit_hook handler, which
// ends up getting translated into a zDBConneciton event in managed code
//
// Arguments:
//
//	context		- Context pointer passed into sqlite3_commit_hook()

static int zdbconnection_commit_hook(void* context)
{
	GCHandleRef<zDBConnectionCommitHook^>	hook(context);	// Hook object instance
	gcroot<CancelEventArgs^>				args;			// Arguments for event

	// Don't even try to touch anything if the GCHandle is no longer allocated,
	// the object reference is NULL, or the connection has been disposed of

	Debug::Assert(hook != nullptr);
	if((!hook.IsAllocated) || (hook == nullptr) || (hook->IsDisposed())) return 0;

	args = gcnew CancelEventArgs();			// Construct the argument class

	// Attempt to raise the event back up to the application, and just allow
	// the commit to take place in the event of an exception

	try { hook->Raise(args); }
	catch(Exception^) { return 0; }

	return (args->Cancel) ? 1 : 0;			// Non-zero to rollback instead
}

//---------------------------------------------------------------------------
// zDBConnectionCommitHook::InstallHook (protected)
//
// Installs the underlying SQLite database hook
//
// Arguments:
//
//	pDatabase		- SQLite database handle wrapper instance
//	context			- Context pointer to register with the hook

void zDBConnectionCommitHook::InstallHook(DatabaseHandle* pDatabase, void* context)
{
	if(!pDatabase) throw gcnew ArgumentNullException();

	// sqlite3_commit_hook is currently experimental
	ENGINE_ISSUE(3.3.8, "Commit callback is listed as experimental");

	sqlite3_commit_hook(pDatabase->Handle, zdbconnection_commit_hook, context);

#ifdef ZDB_TRACE_CONNECTIONHOOKS
	Debug::WriteLine(String::Format("zDBConnectionCommitHook 0x{0:X} installed.",
		IntPtr(pDatabase)));
#endif
}

//---------------------------------------------------------------------------
// zDBConnectionCommitHook::InternalRaise (protected)
//
// Raises the database hook as a managed event
//
// Arguments:
//
//	handler		- The delegate handler
//	sender		- Sender to specify for the event
//	args		- Event arguments

void zDBConnectionCommitHook::InternalRaise(CancelEventHandler^ handler, 
	Object^ sender, CancelEventArgs^ args)
{
	if(handler != nullptr) handler(sender, args);
}

//---------------------------------------------------------------------------
// zDBConnectionCommitHook::RemoveHook (protected)
//
// Removes the underlying SQLite database hook
//
// Arguments:
//
//	pDatabase		- SQLite database handle wrapper instance

void zDBConnectionCommitHook::RemoveHook(DatabaseHandle* pDatabase)
{
	if(!pDatabase) throw gcnew ArgumentNullException();

	sqlite3_commit_hook(pDatabase->Handle, NULL, NULL);

#ifdef ZDB_TRACE_CONNECTIONHOOKS
	Debug::WriteLine(String::Format("zDBConnectionCommitHook 0x{0:X} removed.",
		IntPtr(pDatabase)));
#endif
}

//---------------------------------------------------------------------------
// ZDBCONNECTIONPROGRESSHOOK IMPLEMENTATION
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// zdbconnection_progress_hook
//
// Unmanaged callback function for the sqlite3_progress_handler, which
// ends up getting translated into a zDBConneciton event in managed code
//
// Arguments:
//
//	context		- Context pointer passed into sqlite3_progress_handler()

static int zdbconnection_progress_hook(void* context)
{
	GCHandleRef<zDBConnectionProgressHook^>	hook(context);	// Hook object instance
	gcroot<CancelEventArgs^>				args;			// Arguments for event

	// Don't even try to touch anything if the GCHandle is no longer allocated,
	// the object reference is NULL, or the connection has been disposed of

	Debug::Assert(hook != nullptr);
	if((!hook.IsAllocated) || (hook == nullptr) || (hook->IsDisposed())) return 0;

	args = gcnew CancelEventArgs();					// Construct the arguments

	// Raise the event, and just ignore any exceptions that occur, since there
	// really isn't anything useful we can do about it here other than terminate
	// the statement, which is probably not a good fallback position to have

	try { hook->Raise(args); }
	catch(Exception^) { return 0; }

	return (args->Cancel) ? 1 : 0;			// Non-zero to rollback instead
}

//---------------------------------------------------------------------------
// zDBConnectionProgressHook::Frequency::set
//
// Changes the frequency at which the progress callback will be invoked.
// Lower numbers indicate a more frequent callback

void zDBConnectionProgressHook::Frequency::set(int value)
{
	if(value <= 0) throw gcnew ArgumentOutOfRangeException();

	m_freq = value;						// Change the contained value

	// If there is a cached database handle, the hook is currently installed.
	// We need to replace the handler with a new one in order to update the
	// frequency in which the callback will occur

	if(m_hDatabase) sqlite3_progress_handler(m_hDatabase, m_freq, 
		zdbconnection_progress_hook, m_context);
}

//---------------------------------------------------------------------------
// zDBConnectionProgressHook::InstallHook (protected)
//
// Installs the underlying SQLite database hook
//
// Arguments:
//
//	pDatabase		- SQLite database handle wrapper instance
//	context			- Context pointer to register with the hook

void zDBConnectionProgressHook::InstallHook(DatabaseHandle* pDatabase, void* context)
{
	if(!pDatabase) throw gcnew ArgumentNullException();

	// sqlite3_progress_handler is currently experimental
	ENGINE_ISSUE(3.3.8, "Progress callback is listed as experimental");

	// Install the progress handler hook and save the database handle so we can change
	// it later if need be in the .Frequency property

	sqlite3_progress_handler(pDatabase->Handle, m_freq, zdbconnection_progress_hook, context);
	m_hDatabase = pDatabase->Handle;
	m_context = context;

#ifdef ZDB_TRACE_CONNECTIONHOOKS
	Debug::WriteLine(String::Format("zDBConnectionProgressHook 0x{0:X} installed.",
		IntPtr(pDatabase)));
#endif
}

//---------------------------------------------------------------------------
// zDBConnectionProgressHook::InternalRaise (protected)
//
// Raises the database hook as a managed event
//
// Arguments:
//
//	handler		- The delegate handler
//	sender		- Sender to specify for the event
//	args		- Event arguments

void zDBConnectionProgressHook::InternalRaise(CancelEventHandler^ handler, 
	Object^ sender, CancelEventArgs^ args)
{
	if(handler != nullptr) handler(sender, args);
}

//---------------------------------------------------------------------------
// zDBConnectionProgressHook::RemoveHook (protected)
//
// Removes the underlying SQLite database hook
//
// Arguments:
//
//	pDatabase		- SQLite database handle wrapper instance

void zDBConnectionProgressHook::RemoveHook(DatabaseHandle* pDatabase)
{
	if(!pDatabase) throw gcnew ArgumentNullException();

	sqlite3_progress_handler(pDatabase->Handle, 0, NULL, NULL);
	m_hDatabase = NULL;
	m_context = NULL;

#ifdef ZDB_TRACE_CONNECTIONHOOKS
	Debug::WriteLine(String::Format("zDBConnectionProgressHook 0x{0:X} removed.",
		IntPtr(pDatabase)));
#endif
}

//---------------------------------------------------------------------------
// ZDBCONNECTIONROLLBACKHOOK IMPLEMENTATION
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// zdbconnection_rollback_hook
//
// Unmanaged callback function for the sqlite3_rollback_hook handler, which
// ends up getting translated into a zDBConneciton event in managed code
//
// Arguments:
//
//	context		- Context pointer passed into sqlite3_rollback_hook()

static void zdbconnection_rollback_hook(void* context)
{
	GCHandleRef<zDBConnectionRollbackHook^>	hook(context);	// Hook object instance

	// Don't even try to touch anything if the GCHandle is no longer allocated,
	// the object reference is NULL, or the connection has been disposed of

	Debug::Assert(hook != nullptr);
	if((!hook.IsAllocated) || (hook == nullptr) || (hook->IsDisposed())) return;

	// Try to raise the event, and once again there is really nothing useful we
	// can do in the event of an exception in the handler.

	try { hook->Raise(EventArgs::Empty); }
	catch(Exception^) { /* DO NOTHING */ }
}

//---------------------------------------------------------------------------
// zDBConnectionRollbackHook::InstallHook (protected)
//
// Installs the underlying SQLite database hook
//
// Arguments:
//
//	pDatabase		- SQLite database handle wrapper instance
//	context			- Context pointer to register with the hook

void zDBConnectionRollbackHook::InstallHook(DatabaseHandle* pDatabase, void* context)
{
	if(!pDatabase) throw gcnew ArgumentNullException();

	sqlite3_rollback_hook(pDatabase->Handle, zdbconnection_rollback_hook, context);

#ifdef ZDB_TRACE_CONNECTIONHOOKS
	Debug::WriteLine(String::Format("zDBConnectionRollbackHook 0x{0:X} installed.",
		IntPtr(pDatabase)));
#endif
}

//---------------------------------------------------------------------------
// zDBConnectionRollbackHook::InternalRaise (protected)
//
// Raises the database hook as a managed event
//
// Arguments:
//
//	handler		- The delegate handler
//	sender		- Sender to specify for the event
//	args		- Event arguments

void zDBConnectionRollbackHook::InternalRaise(EventHandler^ handler, Object^ sender, 
	EventArgs^ args)
{
	if(handler != nullptr) handler(sender, args);
}

//---------------------------------------------------------------------------
// zDBConnectionRollbackHook::RemoveHook (protected)
//
// Removes the underlying SQLite database hook
//
// Arguments:
//
//	pDatabase		- SQLite database handle wrapper instance

void zDBConnectionRollbackHook::RemoveHook(DatabaseHandle* pDatabase)
{
	if(!pDatabase) throw gcnew ArgumentNullException();

	sqlite3_rollback_hook(pDatabase->Handle, NULL, NULL);

#ifdef ZDB_TRACE_CONNECTIONHOOKS
	Debug::WriteLine(String::Format("zDBConnectionRollbackHook 0x{0:X} removed",
		IntPtr(pDatabase)));
#endif
}

//---------------------------------------------------------------------------
// ZDBCONNECTIONPROFILEHOOK IMPLEMENTATION
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// zdbconnection_profile_hook
//
// Unmanaged callback function for the sqlite3_profile handler
//
// Arguments:
//
//	context		- Context pointer passed into sqlite3_trace()
//	statement	- Statement being profiled
//	elapsed		- Elapsed statement time

static void zdbconnection_profile_hook(void* context, const char* statement, sqlite_uint64 elapsed)
{
	GCHandleRef<zDBConnectionProfileHook^>	hook(context);	// Hook object instance
	gcroot<String^>							msg;			// Event argument data

	// Don't even try to touch anything if the GCHandle is no longer allocated,
	// the object reference is NULL, or the connection has been disposed of

	Debug::Assert(hook != nullptr);
	if((!hook.IsAllocated) || (hook == nullptr) || (hook->IsDisposed())) return;

	// Convert the unmanaged ANSI string and raise the event into .NET land.
	// Note that we're NOT using the zDBUtil::FastPtrToStringAnsi here, since
	// the message can be HUGE and that uses the stack rather than the heap
	// when converting the string.  A potentially bad thing.

	try { hook->Raise(gcnew zDBProfileEventArgs(statement, elapsed)); }
	catch(Exception^) { /* DO NOTHING */ }
}

//---------------------------------------------------------------------------
// zDBConnectionProfileHook::InstallHook (protected)
//
// Installs the underlying SQLite database hook
//
// Arguments:
//
//	hDatabase		- SQLite database handle
//	context			- Context pointer to register with the hook

void zDBConnectionProfileHook::InstallHook(DatabaseHandle* pDatabase, void* context)
{
	if(!pDatabase) throw gcnew ArgumentNullException();

	sqlite3_profile(pDatabase->Handle, zdbconnection_profile_hook, context);

#ifdef ZDB_TRACE_CONNECTIONHOOKS
	Debug::WriteLine(String::Format("zDBConnectionProfileHook 0x{0:X} installed.",
		IntPtr(pDatabase)));
#endif
}

//---------------------------------------------------------------------------
// zDBConnectionProfileHook::InternalRaise (protected)
//
// Raises the database hook as a managed event
//
// Arguments:
//
//	handler		- The delegate handler
//	sender		- Sender to specify for the event
//	args		- Event arguments

void zDBConnectionProfileHook::InternalRaise(zDBProfileEventHandler^ handler, 
	Object^ sender, zDBProfileEventArgs^ args)
{
	if(handler != nullptr) handler(sender, args);
}

//---------------------------------------------------------------------------
// zDBConnectionProfileHook::RemoveHook (protected)
//
// Removes the underlying SQLite database hook
//
// Arguments:
//
//	pDatabase		- SQLite database handle wrapper instance

void zDBConnectionProfileHook::RemoveHook(DatabaseHandle* pDatabase)
{
	if(!pDatabase) throw gcnew ArgumentNullException();

	sqlite3_profile(pDatabase->Handle, NULL, NULL);

#ifdef ZDB_TRACE_CONNECTIONHOOKS
	Debug::WriteLine(String::Format("zDBConnectionProfileHook 0x{0:X} removed",
		IntPtr(pDatabase)));
#endif
}

//---------------------------------------------------------------------------
// ZDBCONNECTIONTRACEHOOK IMPLEMENTATION
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// zdbconnection_trace_hook
//
// Unmanaged callback function for the sqlite3_trace handler
//
// Arguments:
//
//	context		- Context pointer passed into sqlite3_trace()
//	message		- Trace message from the database engine

static void zdbconnection_trace_hook(void* context, const char* message)
{
	GCHandleRef<zDBConnectionTraceHook^>	hook(context);	// Hook object instance
	gcroot<String^>							msg;			// Event argument data

	// Don't even try to touch anything if the GCHandle is no longer allocated,
	// the object reference is NULL, or the connection has been disposed of

	Debug::Assert(hook != nullptr);
	if((!hook.IsAllocated) || (hook == nullptr) || (hook->IsDisposed())) return;

	// Convert the unmanaged ANSI string and raise the event into .NET land.
	// Note that we're NOT using the zDBUtil::FastPtrToStringAnsi here, since
	// the message can be HUGE and that uses the stack rather than the heap
	// when converting the string.  A potentially bad thing.

	try { hook->Raise(gcnew zDBTraceEventArgs(message)); }
	catch(Exception^) { /* DO NOTHING */ }
}

//---------------------------------------------------------------------------
// zDBConnectionTraceHook::InstallHook (protected)
//
// Installs the underlying SQLite database hook
//
// Arguments:
//
//	hDatabase		- SQLite database handle
//	context			- Context pointer to register with the hook

void zDBConnectionTraceHook::InstallHook(DatabaseHandle* pDatabase, void* context)
{
	if(!pDatabase) throw gcnew ArgumentNullException();

	sqlite3_trace(pDatabase->Handle, zdbconnection_trace_hook, context);

#ifdef ZDB_TRACE_CONNECTIONHOOKS
	Debug::WriteLine(String::Format("zDBConnectionTraceHook 0x{0:X} installed.",
		IntPtr(pDatabase)));
#endif
}

//---------------------------------------------------------------------------
// zDBConnectionTraceHook::InternalRaise (protected)
//
// Raises the database hook as a managed event
//
// Arguments:
//
//	handler		- The delegate handler
//	sender		- Sender to specify for the event
//	args		- Event arguments

void zDBConnectionTraceHook::InternalRaise(zDBTraceEventHandler^ handler, 
	Object^ sender, zDBTraceEventArgs^ args)
{
	if(handler != nullptr) handler(sender, args);
}

//---------------------------------------------------------------------------
// zDBConnectionTraceHook::RemoveHook (protected)
//
// Removes the underlying SQLite database hook
//
// Arguments:
//
//	pDatabase		- SQLite database handle wrapper instance

void zDBConnectionTraceHook::RemoveHook(DatabaseHandle* pDatabase)
{
	if(!pDatabase) throw gcnew ArgumentNullException();

	sqlite3_trace(pDatabase->Handle, NULL, NULL);

#ifdef ZDB_TRACE_CONNECTIONHOOKS
	Debug::WriteLine(String::Format("zDBConnectionTraceHook 0x{0:X} removed",
		IntPtr(pDatabase)));
#endif
}

//---------------------------------------------------------------------------
// ZDBCONNECTIONUPDATEHOOK IMPLEMENTATION
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// zdbconnection_update_hook
//
// Unmanaged callback function for the sqlite3_update_hook handler, which
// ends up getting translated into a zDBConneciton event in managed code
//
// Arguments:
//
//	context		- Context pointer passed into sqlite3_xxx_hook()
//	reason		- Reason that the handler is being invoked
//	database	- Name of the database being modified
//	table		- Name of the table being modified
//	rowid		- ROWID of the row affected by the change

static void zdbconnection_update_hook(void* context, int reason, const char* database,
	const char* table, __int64 rowid)
{
	GCHandleRef<zDBConnectionUpdateHook^>	hook(context);	// Hook object instance
	gcroot<zDBRowChangedEventArgs^>			args;			// Event argument construct

	// Don't even try to touch anything if the GCHandle is no longer allocated,
	// the object reference is NULL, or the connection has been disposed of

	Debug::Assert(hook != nullptr);
	if((!hook.IsAllocated) || (hook == nullptr) || (hook->IsDisposed())) return;

	args = gcnew zDBRowChangedEventArgs(reason, database, table, rowid);

	// Pretty much the same model over and over in here. Raise the event, ignore
	// anything bad that happens...

	try { hook->Raise(args); }
	catch(Exception^) { /* DO NOTHING */ }
}

//---------------------------------------------------------------------------
// zDBConnectionUpdateHook::InstallHook (protected)
//
// Installs the underlying SQLite database hook
//
// Arguments:
//
//	hDatabase		- SQLite database handle
//	context			- Context pointer to register with the hook

void zDBConnectionUpdateHook::InstallHook(DatabaseHandle* pDatabase, void* context)
{
	if(!pDatabase) throw gcnew ArgumentNullException();

	sqlite3_update_hook(pDatabase->Handle, zdbconnection_update_hook, context);

#ifdef ZDB_TRACE_CONNECTIONHOOKS
	Debug::WriteLine(String::Format("zDBConnectionUpdateHook 0x{0:X} installed.",
		IntPtr(pDatabase)));
#endif
}

//---------------------------------------------------------------------------
// zDBConnectionUpdateHook::InternalRaise (protected)
//
// Raises the database hook as a managed event
//
// Arguments:
//
//	handler		- The delegate handler
//	sender		- Sender to specify for the event
//	args		- Event arguments

void zDBConnectionUpdateHook::InternalRaise(zDBRowChangedEventHandler^ handler, 
	Object^ sender, zDBRowChangedEventArgs^ args)
{
	if(handler != nullptr) handler(sender, args);
}

//---------------------------------------------------------------------------
// zDBConnectionUpdateHook::RemoveHook (protected)
//
// Removes the underlying SQLite database hook
//
// Arguments:
//
//	pDatabase		- SQLite database handle wrapper instance

void zDBConnectionUpdateHook::RemoveHook(DatabaseHandle* pDatabase)
{
	if(!pDatabase) throw gcnew ArgumentNullException();

	sqlite3_update_hook(pDatabase->Handle, NULL, NULL);

#ifdef ZDB_TRACE_CONNECTIONHOOKS
	Debug::WriteLine(String::Format("zDBConnectionUpdateHook 0x{0:X} removed",
		IntPtr(pDatabase)));
#endif
}

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)
