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
#include "zDBVirtualTable.h"		// Include zDBVirtualTable declarations
#include "zDBConnection.h"			// Include zDBConnection declarations

#pragma warning(push, 4)			// Enable maximum compiler warnings
#pragma warning(disable:4100)		// "unreferenced formal parameter"

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// zDBVirtualTable Constructor (protected)
//
// Arguments:
//
//	NONE

generic<class _cursor>
zDBVirtualTable<_cursor>::zDBVirtualTable()
{
	// Originally, this class required the zDBVirtualTableConstructorArgs to
	// be passed into it.  However, it was kind of evil to make the derived
	// class do that, especially since it can only be detected at runtime as
	// a problem if it didn't.   Now there is a hokey but quite effective
	// static collection in zDBVirtualTableConstructorArgs to use that provides
	// the object instance to us.  Ugly on the inside to be pretty outside

	m_args = zDBVirtualTableConstructorArgs::Pop();
	m_pFuncs = new FunctionMap();
}

//---------------------------------------------------------------------------
// zDBVirtualTable Finalizer

generic<class _cursor>
zDBVirtualTable<_cursor>::!zDBVirtualTable()
{
	// On finalization, it's critical to release all of the GCHandles that
	// have been allocated for overloaded functions

	for(FunctionMapIterator it = m_pFuncs->begin(); it != m_pFuncs->end(); it++)
		GCHandle::FromIntPtr(IntPtr(it->second)).Free();

	delete m_pFuncs;				// Destroy the collection
	m_pFuncs = NULL;				// Reset pointer to NULL
}

//---------------------------------------------------------------------------
// zDBVirtualTable::Arguments::get (protected)
//
// Exposes a collection of arguments passed into CREATE VIRTUAL TABLE

generic<class _cursor>
ReadOnlyCollection<String^>^ zDBVirtualTable<_cursor>::Arguments::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return m_args->Arguments;
}

//---------------------------------------------------------------------------
// zDBVirtualTable::Connection::get (protected)
//
// Exposes the zDBConnection instance this virtual table was registered with

generic<class _cursor>
zDBConnection^ zDBVirtualTable<_cursor>::Connection::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return m_args->Connection;
}

//---------------------------------------------------------------------------
// zDBVirtualTable::DatabaseName::get (protected)
//
// Exposes the name of the database instance that owns this virtual table

generic<class _cursor>
String^ zDBVirtualTable<_cursor>::DatabaseName::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return m_args->DatabaseName;
}

//---------------------------------------------------------------------------
// zDBVirtualTable::DataTableToSchema (private, static)
//
// Converts the schema of a DataTable into a CREATE TABLE statement that
// satisfies the requirements of the base interface.  Kinda silly to have
// a static function like this in a generic class, but that's OK for now.
//
// Arguments:
//
//	name	- Table name (purely academic)
//	dt		- DataTable with a schema to be converted

generic<class _cursor>
String^ zDBVirtualTable<_cursor>::DataTableToSchema(String^ name, DataTable^ dt)
{
	StringBuilder^	sb = gcnew StringBuilder();		// CREATE TABLE builder

	if(dt == nullptr) throw gcnew ArgumentNullException();

	// Start out with a generic CREATE TABLE statement; the table name itself
	// is ignored by SQLite when generating the schema information

	sb->Append("CREATE TABLE [" + name + "](");
	
	// Append the name and data type of each column in the DataTable to
	// the CREATE TABLE statement, in the order they come back to us

	//
	// TODO: Use the type code instead of the type name to prevent funky things.
	// don't forget to manually check for byte[], char[] and Guid since they are
	// valid but don't have corresponding type codes.
	//

	for each(DataColumn^ dc in dt->Columns)
		sb->Append(String::Format("[{0}] {1}, ", dc->ColumnName, dc->DataType->Name));

	// Remove the trailing comma and space, tack on a closing parenthesis,
	// and that should be that ..

	return sb->ToString()->TrimEnd(gcnew array<Char>{',', ' '}) + ")";
}

//---------------------------------------------------------------------------
// zDBVirtualTable::FindFunctionInternal (private)
//
// Implements zDBVirtualTableBase::FindFunction.  The local collection of
// overriden functions is examined, and if a decent match is located, it is
// returned as a GCHandle back into the unmanaged code portion
//
// Arguments:
//
//	name			- Name of the function to find
//	argc			- Number of arguments to the function
//	funcwrapper		- Set to an instance of zDBFunctionWrapper on TRUE

generic<class _cursor>
bool zDBVirtualTable<_cursor>::FindFunctionInternal(String^ name, int argc, GCHandle% funcwrapper)
{
	PinnedStringPtr				pinName;			// Pinned function name string
	FunctionMapIterator			it;					// Collection iterator

	CHECK_DISPOSED(m_disposed);
	if(name == nullptr) throw gcnew ArgumentNullException();

	pinName = PtrToStringChars(name);
	FunctionMapKey key = FunctionMapKey(pinName, argc);

	// Attempt to locate an exact match for the function in the collection.  If
	// we don't have one of those, look for one that accepts any number of arguments.
	// If we don't find one of those ... we're done.

	it = m_pFuncs->find(key);
	if(it == m_pFuncs->end()) { key.Argument = -1; it = m_pFuncs->find(key); }
	if(it == m_pFuncs->end()) return false;

	// We found a function that meets the requirements. Return the GCHandle
	// for the zDBFunctionWrapper back to the caller.  That becomes the context
	// pointer passed into the unmanaged function callback.

	funcwrapper = GCHandle::FromIntPtr(IntPtr(it->second));
	return true;
}

//---------------------------------------------------------------------------
// zDBVirtualTable::ModuleName::get (protected)
//
// Exposes the name of the module used to create this virtual table

generic<class _cursor>
String^ zDBVirtualTable<_cursor>::ModuleName::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return m_args->ModuleName;
}

//---------------------------------------------------------------------------
// zDBVirtualTable::OverrideFunction
//
// Allows the virtual table to override a scalar function implementation
// with it's own version of that function.
//
// Arguments:
//
//	name		- Name of the function to be overloaded
//	argCount	- Number of arguments that the function accepts
//	function	- zDBFunction delegate to invoke when function is called

generic<class _cursor>
void zDBVirtualTable<_cursor>::OverrideFunction(String^ name, int argCount, 
	zDBFunction^ function)
{
	PinnedStringPtr				pinName;		// Pinned function name
	FunctionMapIterator			it;				// Collection iterator
	zDBFunctionWrapper^			wrapper;		// Function wrapper instance
	GCHandle					gchandle;		// Delegate GCHandle structure
	intptr_t					pthandle;		// Serialized GCHandle structure
	int							nResult;		// Result from function call

	CHECK_DISPOSED(m_disposed);
	if(name == nullptr) throw gcnew ArgumentNullException();
	if(function == nullptr) throw gcnew ArgumentNullException();

	pinName = PtrToStringChars(name);
	FunctionMapKey key = FunctionMapKey(pinName, argCount);

	// The first thing we need to do is tell SQLite we're planning on overriding this
	// particular function.  Too bad there's no "unoverload function" call we can make,
	// this ends up rather permanent from the connection's perspective.
	ENGINE_ISSUE(3.3.8, "There is no Unicode version of sqlite3_overload_function");

	nResult = sqlite3_overload_function(m_args->Connection->Handle, AutoAnsiString(name), argCount);
	if(nResult != SQLITE_OK) throw gcnew zDBException(m_args->Connection->Handle, nResult);

	// If a function with the same signature already exists in the collection, free
	// it's zDBFunctionWrapper GCHandle and get it out of there.  We could technically
	// just re-use the entry, but what fun would that be.

	it = m_pFuncs->find(key);
	if(it != m_pFuncs->end()) { GCHandle::FromIntPtr(IntPtr(it->second)).Free(); m_pFuncs->erase(it); }

	// Create the new function wrapper instance that gets serialized into the
	// collection, and set up the database handle now.  Virtual Table functions
	// disappear as soon as the table does, or the connection is closed, so the
	// handle can be considered immutable. (Unlike the main zDBFunctionCollection)

	wrapper = gcnew zDBFunctionWrapper(function);
	wrapper->DatabaseHandle = m_args->Connection->Handle;
	gchandle = GCHandle::Alloc(wrapper);
	pthandle = reinterpret_cast<intptr_t>(GCHandle::ToIntPtr(gchandle).ToPointer());

	// And finally .. insert the new wrapper into the collection using the generated
	// key and serialized GCHandle.  Be sure to release the GCHandle on ANY exceptions

	try { m_pFuncs->insert(std::make_pair(key, pthandle)); }
	catch(Exception^) { gchandle.Free(); throw; }
}

//---------------------------------------------------------------------------
// zDBVirtualTable::TableName::get (protected)
//
// Exposes the name of the virtual table as specified in the CREATE VIRTUAL
// TABLE statement

generic<class _cursor>
String^ zDBVirtualTable<_cursor>::TableName::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return m_args->TableName;
}

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)
