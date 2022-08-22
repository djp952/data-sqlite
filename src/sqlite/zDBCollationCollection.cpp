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
#include "zDBCollationCollection.h"	// Include zDBCollationCollection decls

#pragma warning(push, 4)			// Enable maximum compiler warnings

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// zdb_collation_func
//
// Provides the implementation for xCompare that calls through a managed
// zDBCollation delegate to perform the requested operation.
//
// Arguments:
//
//	context			- Context pointer passed into create_collation()
//	cbLeft			- Length of the left-hand argument, in bytes
//	pvLeft			- Pointer to the left-hand argument data
//	cbRight			- Length of the right-hand argument, in bytes
//	pvRight			- Pointer to the right-hand argument data

int zdb_collation_func(void* context, int cbLeft, const void* pvLeft, int cbRight, const void* pvRight)
{
	GCHandleRef<zDBCollationWrapper^> collation(context);	// Unwrap the GCHandle

	// The invocation logic is really contained in the zDBCollationWrapper, so
	// all we do in here is call that.  There is no way to convert exceptions into
	// errors when working with collations, so the best we can do is return zero

	try { return collation->Invoke(pvLeft, cbLeft, pvRight, cbRight); }
	catch(Exception^) { return 0; }
}

//---------------------------------------------------------------------------
// zDBCollationCollection Constructor
//
// Arguments:
//
//	NONE

zDBCollationCollection::zDBCollationCollection()
{
	m_pCol = new FunctionMap();
	if(!m_pCol) throw gcnew OutOfMemoryException();
}

//---------------------------------------------------------------------------
// zDBCollationCollection Finalizer

zDBCollationCollection::!zDBCollationCollection()
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
// zDBCollationCollection::Add
//
// Adds a new standard UTF16 string collation to the collection.  If the 
// connection to the database is currently open, the function will be immediately 
// installed.  Otherwise it will be installed as soon as the parent connection 
// does get opened
//
// Arguments:
//
//	name		- Collation name to register
//	collation	- Delegate representing the collation

void zDBCollationCollection::Add(String^ name, zDBCollation^ collation)
{
	PinnedStringPtr				pinName;		// Pinned name string
	GCHandle					gchandle;		// Delegate GCHandle structure
	intptr_t					pthandle;		// Serialized GCHandle structure

	CHECK_DISPOSED(m_disposed);

	if(name == nullptr) throw gcnew ArgumentNullException();
	if(collation == nullptr) throw gcnew ArgumentNullException();

	Remove(name, zDBCollationEncoding::UTF16);	// Remove existing collation

	// Generate the collection key, which is based on the name and arg count

	pinName = PtrToStringChars(name);
	FunctionMapKey key = FunctionMapKey(pinName, static_cast<int>(zDBCollationEncoding::UTF16));

	// Create a new zDBCollationWrapper as well as a STRONG GCHandle against it
	// so we can keep it alive without the garbage collector screwing us up

	gchandle = GCHandle::Alloc(gcnew zDBCollationWrapper(collation));
	pthandle = reinterpret_cast<intptr_t>(GCHandle::ToIntPtr(gchandle).ToPointer());

	try {

		// If the database is open already, try to install the collation now.
		// Regardless, insert the new entry into the collection for safe keeping

		if(m_pDatabase) InstallCollation(m_pDatabase->Handle, key.Name, 
			static_cast<int>(zDBCollationEncoding::UTF16), pthandle);
		m_pCol->insert(std::make_pair(key, pthandle));
	}

	catch(Exception^) { gchandle.Free(); throw; }	// <-- Release GCHandle
}

//---------------------------------------------------------------------------
// zDBCollationCollection::Add
//
// Adds a new collation to the collection.  If the connection to the database
// is currently open, the function will be immediately installed.  Otherwise
// it will be installed as soon as the parent connection does get opened
//
// Arguments:
//
//	name		- Collation name to register
//	encoding	- Encoding to use when SQLite invokes the collation
//	collation	- Delegate representing the collation

void zDBCollationCollection::Add(String^ name, zDBCollationEncoding encoding, 
	zDBBinaryCollation^ collation)
{
	PinnedStringPtr				pinName;		// Pinned name string
	GCHandle					gchandle;		// Delegate GCHandle structure
	intptr_t					pthandle;		// Serialized GCHandle structure

	CHECK_DISPOSED(m_disposed);

	if(name == nullptr) throw gcnew ArgumentNullException();
	if(collation == nullptr) throw gcnew ArgumentNullException();

	Remove(name, encoding);						// Remove existing collation

	// Generate the collection key, which is based on the name and arg count

	pinName = PtrToStringChars(name);
	FunctionMapKey key = FunctionMapKey(pinName, static_cast<int>(encoding));

	// Create a new zDBCollationWrapper as well as a STRONG GCHandle against it
	// so we can keep it alive without the garbage collector screwing us up

	gchandle = GCHandle::Alloc(gcnew zDBCollationWrapper(collation));
	pthandle = reinterpret_cast<intptr_t>(GCHandle::ToIntPtr(gchandle).ToPointer());

	try {

		// If the database is open already, try to install the collation now.
		// Regardless, insert the new entry into the collection for safe keeping

		if(m_pDatabase) InstallCollation(m_pDatabase->Handle, key.Name, 
			static_cast<int>(encoding), pthandle);
		m_pCol->insert(std::make_pair(key, pthandle));
	}

	catch(Exception^) { gchandle.Free(); throw; }	// <-- Release GCHandle
}

//---------------------------------------------------------------------------
// zDBCollationCollection::Clear
//
// Removes all collations from the collection.  If the connection is currently
// open, they will be removed from the database immediately.  This has no
// effect on collations not registered from this provider.
//
// Arguments:
//
//	NONE

void zDBCollationCollection::Clear(void)
{
	CHECK_DISPOSED(m_disposed);

	// Iterate over the collection, and remove/destroy all functions one
	// at a time.  Do not call Remove(), as it will invalidate the iterator

	for(FunctionMapIterator it = m_pCol->begin(); it != m_pCol->end(); it++) {

		try { if(m_pDatabase) RemoveCollation(m_pDatabase->Handle, it->first.Name, 
			it->first.Argument, it->second); }
		finally { GCHandle::FromIntPtr(IntPtr(it->second)).Free(); }
	}

	m_pCol->clear();							// Clear out the collection
}

//---------------------------------------------------------------------------
// zDBCollationCollection::InstallCollation (private, static)
//
// Installs a collation into the specified database connection
//
// Arguments:
//
//	hDatabase		- Database connection handle to install into
//	name			- Name of the function to be installed
//	encoding		- Encoding mode for the collation sequence
//	funcwrapper		- Serialized GCHandle of the zDBCollationWrapper instance

void zDBCollationCollection::InstallCollation(sqlite3* hDatabase, std::wstring name,
	int encoding, intptr_t funcwrapper)
{
	GCHandleRef<zDBCollationWrapper^>	func(funcwrapper);	// Unwrapped GCHandle
	int									nResult;			// Result from function call

	if(!hDatabase) throw gcnew ArgumentNullException();

	// Ask SQLite to create the user defined collation against this database.  Note
	// that the declaration of sqlite_create_collation16 is screwed and expects a char*
	ENGINE_ISSUE(3.3.8, "sqlite3_create_collation16 expects const char* instead of const void*");

	nResult = sqlite3_create_collation16(hDatabase, reinterpret_cast<const char*>(name.c_str()),
		encoding, reinterpret_cast<void*>(funcwrapper), zdb_collation_func);
	if(nResult != SQLITE_OK) throw gcnew zDBException(hDatabase, nResult);

	func->DatabaseHandle = hDatabase;					// Hook up to the database

#ifdef ZDB_TRACE_FUNCTIONS
	Debug::WriteLine(String::Format("zDBCollationCollection: installed collation {0} "
		"(encoding = {1})", gcnew String(name.c_str()), 
		static_cast<zDBCollationEncoding>(encoding).ToString()));
#endif
}

//---------------------------------------------------------------------------
// zDBCollationCollection::InternalDispose (internal)
//
// Operates a a pseudo-disposal mechanism to prevent applications from being
// able to invoke it prematurely on us
//
// Arguments:
//
//	NONE

void zDBCollationCollection::InternalDispose(void)
{
	if(m_disposed) return;		// Class has already been disposed of

	Clear();								// Remove all functions

	this->!zDBCollationCollection();		// Invoke the finalizer
	GC::SuppressFinalize(this);				// Suppress finalization
	m_disposed = true;						// Object is now disposed
}

//---------------------------------------------------------------------------
// zDBCollationCollection::OnCloseConnection (internal)
//
// Invoked when the parent connection is closed in order to uninstall all
// collations, but not remove them from the member collection
//
// Arguments:
//
// NONE

void zDBCollationCollection::OnCloseConnection(void)
{
	CHECK_DISPOSED(m_disposed);

	Debug::Assert(m_pDatabase != NULL);		// Should always be open here
	if(!m_pDatabase) return;				// Not open -- nothing to do

	// Iterate over every entry in the member collection, and attempt
	// to uninstall all of the functions from the database.  Do not release
	// the GCHandles or remove the collection items ...

	try {

		for(FunctionMapIterator it = m_pCol->begin(); it != m_pCol->end(); it++)
			RemoveCollation(m_pDatabase->Handle, it->first.Name, it->first.Argument, it->second);
	}

	finally {

		// Even if something catastrophic happened up there, we always want to
		// close out our instance of the database handle.

		if(m_pDatabase) m_pDatabase->Release(this);
		m_pDatabase = NULL;
	}
}

//---------------------------------------------------------------------------
// zDBCollationCollection::OnOpenConnection (internal)
//
// Invoked when the parent database connection has been opened.  Any collations
// that were already added to the collection will be automatically installed
// to the database at this time
//
// Arguments:
//
//	pDatabase		- Pointer to the new database handle wrapper

void zDBCollationCollection::OnOpenConnection(DatabaseHandle *pDatabase)
{
	CHECK_DISPOSED(m_disposed);
	if(!pDatabase) throw gcnew ArgumentNullException();

	Debug::Assert(m_pDatabase == NULL);			// Should always be ...
	if(m_pDatabase) OnCloseConnection();		// ... but just in case

	m_pDatabase = pDatabase;					// Save instance pointer
	m_pDatabase->AddRef(this);					// We're keeping this

	// Iterate over the member collection and install any collations that have
	// already been added to this connection object ...

	for(FunctionMapIterator it = m_pCol->begin(); it != m_pCol->end(); it++)
		InstallCollation(m_pDatabase->Handle, it->first.Name, it->first.Argument, it->second);
}

//---------------------------------------------------------------------------
// zDBCollationCollection::Remove
//
// Removes all collations with the specified name from the collection.  If the
// connection is currently open, they will be removed from the database now.
// This does not affect collations registered outside of this provider.
//
// Arguments:
//
//	name		- Name of the function(s) to be removed

bool zDBCollationCollection::Remove(String^ name)
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

		// We have a match.  If the database is open, remove the collation first, but
		// no matter what happens, free the GCHandle and erase the item from the
		// collection before allowing the exception to be raised

		try { 
			
			if(m_pDatabase) RemoveCollation(m_pDatabase->Handle, it->first.Name, 
				it->first.Argument, it->second); 
		}

		finally { GCHandle::FromIntPtr(IntPtr(it->second)).Free(); it = m_pCol->erase(it); } 
		
		result = true;							// Removed at least one item
	}

	return result;								// Return result code
}

//---------------------------------------------------------------------------
// zDBCollationCollection::Remove
//
// Removes the collation with the specified name and encoding count from the 
// collection.  If the connection is currently open, they will be removed 
// from the database immediately.  This has no effect on collations registered
// outside of this provider.
//
// Arguments:
//
//	name		- Name of the function(s) to be removed
//	encoding	- Encoding mode of the collation

bool zDBCollationCollection::Remove(String^ name, zDBCollationEncoding encoding)
{
	PinnedStringPtr				pinName;		// Pinned name string
	FunctionMapIterator			it;				// Collection iterator

	CHECK_DISPOSED(m_disposed);

	if(name == nullptr) throw gcnew ArgumentNullException();

	// Generate the collection key, which is based on the name and arg count

	pinName = PtrToStringChars(name);
	FunctionMapKey key = FunctionMapKey(pinName, static_cast<int>(encoding));

	// Attempt to locate the item in the collection. If not there, we're done

	it = m_pCol->find(key);
	if(it == m_pCol->end()) return false;

	// Try to uninstall the function from SQLite as necessary, and make sure we nuke the
	// collection item and release the GCHandle even if that operation fails miserably

	try { 
		
		if(m_pDatabase) RemoveCollation(m_pDatabase->Handle, it->first.Name, 
			it->first.Argument, it->second); 
	}

	finally { GCHandle::FromIntPtr(IntPtr(it->second)).Free(); m_pCol->erase(it); }

	return true;						// Function was successfully removed
}

//---------------------------------------------------------------------------
// zDBCollationCollection::RemoveCollation (static, private)
//
// Uninstalls a collation from the specified database connection
//
// Arguments:
//
//	hDatabase		- Database connection handle to remove function from
//	name			- Name of the function to be removed
//	encoding		- Encoding value for the collation sequence
//	funcwrapper		- Serialized GCHandle of the zDBCollationWrapper instance

void zDBCollationCollection::RemoveCollation(sqlite3* hDatabase, std::wstring name, 
	int encoding, intptr_t funcwrapper)
{
	GCHandleRef<zDBCollationWrapper^>	func(funcwrapper);		// Unwrapped GCHandle
	int									nResult;				// Result from function call

	if(!hDatabase) throw gcnew ArgumentNullException();

	func->DatabaseHandle = NULL;				// Unhook instance from connection

	// Ask SQLite to remove the user defined collation from this database.  Note that
	// the declaration for sqlite3_create_collation16 is screwed up and expects a char*
	ENGINE_ISSUE(3.3.8, "sqlite3_create_collation16 expects const char* instead of const void*");

	nResult = sqlite3_create_collation16(hDatabase, reinterpret_cast<const char*>(name.c_str()),
		encoding, NULL, NULL);
	if(nResult != SQLITE_OK) throw gcnew zDBException(hDatabase, nResult);

#ifdef ZDB_TRACE_FUNCTIONS
	Debug::WriteLine(String::Format("zDBCollationCollection: removed collation {0} "
		"(encoding = {1})", gcnew String(name.c_str()), 
		static_cast<zDBCollationEncoding>(encoding).ToString()));
#endif
}

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)
