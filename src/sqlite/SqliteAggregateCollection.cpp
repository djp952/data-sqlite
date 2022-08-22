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
#include "SqliteAggregateCollection.h"	// Include SqliteAggregateCollection decls

#pragma warning(push, 4)			// Enable maximum compiler warnings

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// sqlite_aggregate_step
//
// Provides the implementation for xStep that calls through a managed
// SqliteAggregate-based class to perform the requested accumulation
//
// Arguments:
//
//	context			- SQLite aggregate context data
//	argc			- Number of aggregate arguments
//	argv			- Aggregate arguments

void sqlite_aggregate_step(sqlite3_context* context, int argc, sqlite3_value** argv)
{
	intptr_t*				pStateHandle;		// Serialized instance GCHandle		

	GCHandleRef<SqliteAggregateWrapper^> wrapper(sqlite3_user_data(context));

	// Grab the aggregate context object from SQLite.  The first time xStep is 
	// called, the value will be zero, which indicates to us that we need to
	// create and initialize a new aggregate instance.

	pStateHandle = reinterpret_cast<intptr_t*>(sqlite3_aggregate_context(context, sizeof(intptr_t)));
	if(*pStateHandle == 0) {

		Object^ instance = Activator::CreateInstance(wrapper->AggregateType);
		*pStateHandle = reinterpret_cast<intptr_t>(GCHandle::ToIntPtr(GCHandle::Alloc(instance)).ToPointer());
	}

	GCHandleRef<SqliteAggregate^> agg(*pStateHandle);		// Get the object instance
	gcroot<SqliteArgumentCollection^> args;

	args = gcnew SqliteArgumentCollection(argc, argv);

	try { agg->Accumulate(args); }
	catch(Exception^ ex) { sqlite3_result_error(context, AutoAnsiString(ex->Message), -1); }
	finally { delete args; }
}

//---------------------------------------------------------------------------
// sqlite_aggregate_final
//
// Provides the implementation for xFinal that calls through a managed
// SqliteAggregate-based class to perform the requested operation
//
// Arguments:
//
//	context			- SQLite aggregate context data

void sqlite_aggregate_final(sqlite3_context* context)
{
	intptr_t*				pStateHandle;		// Serialized instance GCHandle		

	GCHandleRef<SqliteAggregateWrapper^> wrapper(sqlite3_user_data(context));

	pStateHandle = reinterpret_cast<intptr_t*>(sqlite3_aggregate_context(context, sizeof(intptr_t)));
	if(*pStateHandle == 0) { /* ERROR */ }

	GCHandleRef<SqliteAggregate^> agg(*pStateHandle);		// Get the object instance
	gcroot<SqliteResult^> result = gcnew SqliteResult(context);

	try { agg->GetResult(result); }
	catch(Exception^ ex) { sqlite3_result_error(context, AutoAnsiString(ex->Message), -1); }

	finally { 

		delete result;							// Dispose of the result object
		delete static_cast<Object^>(agg);		// Dispose of the aggregate instance
	
		GCHandle::FromIntPtr(IntPtr(*pStateHandle)).Free();		// Release GCHandle
		*pStateHandle = 0;										// Reset state data
	}
}

//---------------------------------------------------------------------------
// SqliteAggregateCollection Constructor
//
// Arguments:
//
//	NONE

SqliteAggregateCollection::SqliteAggregateCollection()
{
	m_pCol = new FunctionMap();
	if(!m_pCol) throw gcnew OutOfMemoryException();
}

//---------------------------------------------------------------------------
// SqliteAggregateCollection Finalizer

SqliteAggregateCollection::!SqliteAggregateCollection()
{
	// If there is still an outstanding reference to the database, release it

	if(m_pDatabase) m_pDatabase->Release(this);
	m_pDatabase = NULL;

	// We use an unmanaged collection to hold the GCHandles so we can ensure
	// that they get freed.  Managed collections can cause lifetime issues
	// when used to store GCHandles (trust me -- it's a bag of worms)

	for(FunctionMapIterator it = m_pCol->begin(); it != m_pCol->end(); it++)
		GCHandle::FromIntPtr(IntPtr(it->second)).Free();

	delete m_pCol;						// Release unmanaged object
	m_pCol = NULL;						// Reset pointer to null
}

//---------------------------------------------------------------------------
// SqliteAggregateCollection::Add
//
// Adds a new aggregate to the collection.  If the connection to the database
// is currently open, the aggregate will be immediately installed.  Otherwise
// it will be installed as soon as the parent connection does get opened
//
// Arguments:
//
//	name			- Aggregate name to register
//	argCount		- Number of arguments the aggregate will accept (-1 = dynamic)
//	aggregateType	- Type of the SqliteAggregate-derived class

void SqliteAggregateCollection::Add(String^ name, int argCount, Type^ aggregateType)
{
	PinnedStringPtr				pinName;		// Pinned name string
	GCHandle					gchandle;		// Delegate GCHandle structure
	intptr_t					pthandle;		// Serialized GCHandle structure

	CHECK_DISPOSED(m_disposed);

	if(name == nullptr) throw gcnew ArgumentNullException();
	if(aggregateType == nullptr) throw gcnew ArgumentNullException();

	// The provided type must be a class that derives from SqliteAggregate in
	// order to be added into this collection (a rather obvious conclusion)

	if(!IsValidAggregateType(aggregateType)) 
		throw gcnew SqliteExceptions::InvalidAggregateException(aggregateType);

	Remove(name, argCount);						// Remove existing function

	// Generate the collection key, which is based on the name and arg count

	pinName = PtrToStringChars(name);
	FunctionMapKey key = FunctionMapKey(pinName, argCount);

	// Create a new SqliteAggregateWrapper as well as a STRONG GCHandle against it
	// so we can keep it alive without the garbage collector screwing us up

	gchandle = GCHandle::Alloc(gcnew SqliteAggregateWrapper(aggregateType));
	pthandle = reinterpret_cast<intptr_t>(GCHandle::ToIntPtr(gchandle).ToPointer());

	try {

		// If the database is open already, try to install the aggregate now.
		// Regardless, insert the new entry into the collection for safe keeping

		if(m_pDatabase) InstallAggregate(m_pDatabase->Handle, key.Name, argCount, pthandle);
		m_pCol->insert(std::make_pair(key, pthandle));
	}

	catch(Exception^) { gchandle.Free(); throw; }	// <-- Release GCHandle
}

//---------------------------------------------------------------------------
// SqliteAggregateCollection::Clear
//
// Removes all aggregates from the collection.  If the connection is currently
// open, they will be removed from the database immediately.  This has no
// effect on aggregates not registered from this provider.
//
// Arguments:
//
//	NONE

void SqliteAggregateCollection::Clear(void)
{
	CHECK_DISPOSED(m_disposed);

	// Iterate over the collection, and remove/destroy all functions one
	// at a time.  Do not call Remove(), as it will invalidate the iterator

	for(FunctionMapIterator it = m_pCol->begin(); it != m_pCol->end(); it++) {

		try { 
			
			if(m_pDatabase) RemoveAggregate(m_pDatabase->Handle, it->first.Name, 
				it->first.Argument, it->second); 
		}

		finally { GCHandle::FromIntPtr(IntPtr(it->second)).Free(); }
	}

	m_pCol->clear();							// Clear out the collection
}

//---------------------------------------------------------------------------
// SqliteAggregateCollection::InstallAggregate (private, static)
//
// Installs an aggregate into the specified database connection
//
// Arguments:
//
//	hDatabase		- Database connection handle to install into
//	name			- Name of the function to be installed
//	argCount		- Argument count of the function to be installed
//	aggwrapper		- Serialized GCHandle of the SqliteAggregateWrapper

void SqliteAggregateCollection::InstallAggregate(sqlite3* hDatabase, std::wstring name,
	int argCount, intptr_t aggwrapper)
{
	GCHandleRef<SqliteAggregateWrapper^>	agg(aggwrapper);	// Unwrapped GCHandle
	int									nResult;			// Result from function call

	if(!hDatabase) throw gcnew ArgumentNullException();

	// Ask SQLite to create the user defined function against this database

	nResult = sqlite3_create_function16(hDatabase, name.c_str(), argCount, 
		SQLITE_ANY, reinterpret_cast<void*>(aggwrapper), NULL, sqlite_aggregate_step, 
		sqlite_aggregate_final);
	if(nResult != SQLITE_OK) throw gcnew SqliteException(hDatabase, nResult);

	agg->DatabaseHandle = hDatabase;				// Hook up to the database

#ifdef sqlite_TRACE_FUNCTIONS
	Debug::WriteLine(String::Format("SqliteAggregateCollection: installed "
		"aggregate function {0} (argc = {1})", gcnew String(name.c_str()), argCount));
#endif
}

//---------------------------------------------------------------------------
// SqliteAggregateCollection::InternalDispose (internal)
//
// Operates a a pseudo-disposal mechanism to prevent applications from being
// able to invoke it prematurely on us
//
// Arguments:
//
//	NONE

void SqliteAggregateCollection::InternalDispose(void)
{
	if(m_disposed) return;		// Class has already been disposed of

	Clear();								// Remove all functions

	this->!SqliteAggregateCollection();		// Invoke the finalizer
	GC::SuppressFinalize(this);				// Suppress finalization
	m_disposed = true;						// Object is now disposed
}

//---------------------------------------------------------------------------
// SqliteAggregateCollection::IsValidAggregateType (private, static)
//
// Determines if the specified type represents a valid aggregate class
//
// Arguments:
//
//	aggregateType		- Type to be tested for aggregate compliance

bool SqliteAggregateCollection::IsValidAggregateType(Type^ aggregateType)
{
	try { return (aggregateType->BaseType == SqliteAggregate::typeid); }
	catch(Exception^) { return false; }
}

//---------------------------------------------------------------------------
// SqliteAggregateCollection::OnCloseConnection (internal)
//
// Invoked when the parent connection is closed in order to uninstall all
// aggregates, but not remove them from the member collection
//
// Arguments:
//
// NONE

void SqliteAggregateCollection::OnCloseConnection(void)
{
	CHECK_DISPOSED(m_disposed);

	Debug::Assert(m_pDatabase != NULL);		// Should always be open here
	if(!m_pDatabase) return;				// Not open -- nothing to do

	// Iterate over every entry in the member collection, and attempt
	// to uninstall all of the aggregates from the database.  Do not release
	// the GCHandles or remove the collection items ...

	try {

		for(FunctionMapIterator it = m_pCol->begin(); it != m_pCol->end(); it++)
			RemoveAggregate(m_pDatabase->Handle, it->first.Name, it->first.Argument, it->second);
	}

	finally {

		// Even if something catastrophic happened up there, we always want to
		// close out our instance of the database handle.

		if(m_pDatabase) m_pDatabase->Release(this);
		m_pDatabase = NULL;
	}
}

//---------------------------------------------------------------------------
// SqliteAggregateCollection::OnOpenConnection (internal)
//
// Invoked when the parent database connection has been opened.  Any aggregates
// that were already added to the collection will be automatically installed
// to the database at this time
//
// Arguments:
//
//	pDatabase		- Pointer to the new database handle wrapper

void SqliteAggregateCollection::OnOpenConnection(DatabaseHandle *pDatabase)
{
	CHECK_DISPOSED(m_disposed);
	if(!pDatabase) throw gcnew ArgumentNullException();

	Debug::Assert(m_pDatabase == NULL);			// Should always be ...
	if(m_pDatabase) OnCloseConnection();		// ... but just in case

	m_pDatabase = pDatabase;					// Save instance pointer
	m_pDatabase->AddRef(this);					// We're keeping this

	// Iterate over the member collection and install any functions that have
	// already been added to this connection object ...

	for(FunctionMapIterator it = m_pCol->begin(); it != m_pCol->end(); it++)
		InstallAggregate(m_pDatabase->Handle, it->first.Name, it->first.Argument, it->second);
}

//---------------------------------------------------------------------------
// SqliteAggregateCollection::Remove
//
// Removes all aggregates with the specified name from the collection.  If the
// connection is currently open, they will be removed from the database now.
// This does not affect aggregates registered outside of this provider.
//
// Arguments:
//
//	name		- Name of the aggregate(s) to be removed

bool SqliteAggregateCollection::Remove(String^ name)
{
	PinnedStringPtr				pinName;			// Pinned name string
	FunctionMapIterator			it;					// Collection iterator
	bool						result = false;		// Result from this function

	CHECK_DISPOSED(m_disposed);

	if(name == nullptr) throw gcnew ArgumentNullException();

	pinName = PtrToStringChars(name);				// Pin down the name

	// Scan the contents of the collection, looking for items that match
	// the specified function name.  Use a case-insensitive comparison.

	it = m_pCol->begin();
	while(it != m_pCol->end()) {

		// If the current entry's name does not match, just increment and keep going

		if(_wcsicmp(pinName, it->first.Name.c_str()) == 0) { it++; continue; }

		// We have a match.  If the database is open, remove the aggregate first, but
		// no matter what happens, free the GCHandle and erase the item from the
		// collection before allowing the exception to be raised

		try { 
			
			if(m_pDatabase) RemoveAggregate(m_pDatabase->Handle, it->first.Name, 
				it->first.Argument, it->second); 
		}

		finally { GCHandle::FromIntPtr(IntPtr(it->second)).Free(); it = m_pCol->erase(it); } 
		
		result = true;							// Removed at least one item
	}

	return result;								// Return result code
}

//---------------------------------------------------------------------------
// SqliteAggregateCollection::Remove
//
// Removes the aggregate with the specified name and argument count from the 
// collection.  If the connection is currently open, they will be removed 
// from the database immediately.  This has no effect on aggregates registered
// outside of this provider.
//
// Arguments:
//
//	name		- Name of the aggregate to be removed
//	argCount	- Number of arguments accepted by the function

bool SqliteAggregateCollection::Remove(String^ name, int argCount)
{
	PinnedStringPtr				pinName;		// Pinned name string
	FunctionMapIterator			it;				// Collection iterator

	CHECK_DISPOSED(m_disposed);

	if(name == nullptr) throw gcnew ArgumentNullException();

	// Generate the collection key, which is based on the name and arg count

	pinName = PtrToStringChars(name);
	FunctionMapKey key = FunctionMapKey(pinName, argCount);

	// Attempt to locate the item in the collection. If not there, we're done

	it = m_pCol->find(key);
	if(it == m_pCol->end()) return false;

	// Try to uninstall the function from SQLite as necessary, and make sure we nuke the
	// collection item and release the GCHandle even if that operation fails miserably

	try { 
		
		if(m_pDatabase) RemoveAggregate(m_pDatabase->Handle, it->first.Name, 
			it->first.Argument, it->second); 
	}

	finally { GCHandle::FromIntPtr(IntPtr(it->second)).Free(); m_pCol->erase(it); }

	return true;						// Function was successfully removed
}

//---------------------------------------------------------------------------
// SqliteAggregateCollection::RemoveAggregate (static, private)
//
// Uninstalls an aggregate from the specified database connection
//
// Arguments:
//
//	hDatabase		- Database connection handle to remove function from
//	name			- Name of the function to be removed
//	argCount		- Argument count of the function to be removed
//	aggwrapper		- Serialized GCHandle of the SqliteAggregateWrapper instance

void SqliteAggregateCollection::RemoveAggregate(sqlite3* hDatabase, std::wstring name, 
	int argCount, intptr_t aggwrapper)
{
	GCHandleRef<SqliteAggregateWrapper^>	agg(aggwrapper);	// Unwrapped GCHandle
	int									nResult;			// Result from function call

	if(!hDatabase) throw gcnew ArgumentNullException();

	agg->DatabaseHandle = NULL;				// Unhook instance from connection

	// Ask SQLite to remove the user defined aggregate from this database

	nResult = sqlite3_create_function16(hDatabase, name.c_str(), argCount, 
		SQLITE_ANY, NULL, NULL, NULL, NULL);
	if(nResult != SQLITE_OK) throw gcnew SqliteException(hDatabase, nResult);

#ifdef sqlite_TRACE_FUNCTIONS
	Debug::WriteLine(String::Format("SqliteAggregateCollection: removed "
		"aggregate function {0} (argc = {1})", gcnew String(name.c_str()), argCount));
#endif
}

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)
