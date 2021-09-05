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

#ifndef __ZDBFUNCTIONCOLLECTION_H_
#define __ZDBFUNCTIONCOLLECTION_H_
#pragma once

#include "AutoAnsiString.h"				// Include AutoAnsiString declarations
#include "DatabaseHandle.h"				// Include DatabaseHandle declarations
#include "FunctionMap.h"				// Include FunctionMap declarations
#include "GCHandleRef.h"				// Include GCHandleRef declarations
#include "zDBException.h"				// Include zDBException declarations
#include "zDBFunction.h"				// Include zDBFunction declarations
#include "zDBFunctionWrapper.h"			// Include zDBFunctionWrapper declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings
#pragma warning(disable:4461)			// "finalizer without destructor"

using namespace System;
using namespace System::Collections;
using namespace System::Collections::Generic;
using namespace System::Data;
using namespace System::Runtime::InteropServices;

BEGIN_NAMESPACE(zuki)
BEGIN_NAMESPACE(data)
BEGIN_NAMESPACE(dbms)

//---------------------------------------------------------------------------
// Class zDBFunctionCollection
//
// zDBFunctionCollection implements a collection of zDBFunction delegates
// that is used by zDBConnection to manage application-defined scalar 
// functions.  The zDBFunctionImpl class is used as the actual context object
// so that the database handle can be maintained/changed.  This allows the
// zDBResult object to automatically convert boolean, date/time and GUID
// results into the proper format.  None of that existed in the 1.1 provider.
//
// Another completely new behavior is how this deals with duplicate functions.
// The old version threw an exception, but to be more in line with how SQLite
// handles things, duplicate functions are overridden instead.  There is no
// "stacking" of functions -- if one replaces another, the first one is gone.
//---------------------------------------------------------------------------

public ref class zDBFunctionCollection sealed
{
public:

	//-----------------------------------------------------------------------
	// Member Functions

	// Add
	//
	// Attempts to add a new function implementation to this collection.
	// You can have multiple versions of the same function that accept a
	// different number of arguments.  If no argument count is specified,
	// that implies that the function accepts any number of arguments
	void Add(String^ name, zDBFunction^ function) { return Add(name, -1, function); }
	void Add(String^ name, int argCount, zDBFunction^ function);

	// Clear
	//
	// Removes all registered functions from the collection
	void Clear(void);

	// Remove
	//
	// Removes a function implementation from this collection.  If the version
	// without an argument count is called, ALL functions with a matching name
	// will be removed.  To specifically remove just a dynamic argument version,
	// pass -1 as the argument count to the more specific implementation
	bool Remove(String^ name);
	bool Remove(String^ name, int argCount);

internal:

	// INTERNAL CONSTRUCTOR
	zDBFunctionCollection();

	//-----------------------------------------------------------------------
	// Internal Member Functions

	// InternalDispose
	//
	// Behaves as a pseudo-Disposal mechanism to prevent it from being called
	// outside of the ADO.NET Provider library
	void InternalDispose(void);

	// OnCloseConnection
	//
	// Invoked when the parent connection has been closed to revoke all
	// scalar functions without removing them from this collection
	void OnCloseConnection(void);

	// OnOpenConnection
	//
	// Invoked when the parent connection has been opened to (re)register
	// all scalar functions contained in this collection
	void OnOpenConnection(DatabaseHandle* pDatabase);

private:

	// FINALIZER
	!zDBFunctionCollection();

	//-----------------------------------------------------------------------
	// Private Member Functions

	// InstallFunction
	//
	// Installs a function into the specified database connection
	static void InstallFunction(sqlite3 *hDatabase, std::wstring name, int argCount, 
		intptr_t funcwrapper);

	// RemoveFunction
	//
	// Uninstalls a function from the specified database connection
	static void RemoveFunction(sqlite3 *hDatabase, std::wstring name, int argCount,
		intptr_t funcwrapper);

	//-----------------------------------------------------------------------
	// Member Variables

	bool						m_disposed;			// Object disposal flag
	DatabaseHandle*				m_pDatabase;		// SQLite database handle
	FunctionMap*				m_pCol;				// Function map collection
};

//---------------------------------------------------------------------------

END_NAMESPACE(dbms)
END_NAMESPACE(data)
END_NAMESPACE(zuki)

#pragma warning(pop)

#endif		// __ZDBFUNCTIONCOLLECTION_H_
