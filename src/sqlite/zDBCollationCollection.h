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

#ifndef __ZDBCOLLATIONCOLLECTION_H_
#define __ZDBCOLLATIONCOLLECTION_H_
#pragma once

#include "AutoAnsiString.h"				// Include AutoAnsiString declarations
#include "DatabaseHandle.h"				// Include DatabaseHandle declarations
#include "FunctionMap.h"				// Include FunctionMap declarations
#include "GCHandleRef.h"				// Include GCHandleRef declarations
#include "zDBCollation.h"				// Include zDBCollation declarations
#include "zDBCollationWrapper.h"		// Include zDBCollationWrapper declarations
#include "zDBEnumerations.h"			// Include zDB enumeration declarations
#include "zDBException.h"				// Include zDBException declarations

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
// Class zDBCollationCollection
//
// zDBCollationCollection implements a collection of zDBCollation delegates
// that is used by zDBConnection to manage application-defined collations.
// A wrapper class is used to assist in linking the delegate with it's
// parent connection, just like the zDBFunctionCollection implementation.
//---------------------------------------------------------------------------

public ref class zDBCollationCollection sealed
{
public:

	//-----------------------------------------------------------------------
	// Member Functions

	// Add
	//
	// Attempts to add a new collation implementation to this collection
	void Add(String^ name, zDBCollation^ collation);
	void Add(String^ name, zDBCollationEncoding encoding, zDBBinaryCollation^ collation);

	// Clear
	//
	// Removes all registered collation from the collection
	void Clear(void);

	// Remove
	//
	// Removes a collation implementation from this collection
	bool Remove(String^ name);
	bool Remove(String^ name, zDBCollationEncoding encoding);

internal:

	// INTERNAL CONSTRUCTOR
	zDBCollationCollection();

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
	!zDBCollationCollection();

	//-----------------------------------------------------------------------
	// Private Member Functions

	// InstallCollation
	//
	// Installs a collation into the specified database connection
	static void InstallCollation(sqlite3 *hDatabase, std::wstring name, int encoding,
		intptr_t funcwrapper);

	// RemoveCollation
	//
	// Uninstalls a collation from the specified database connection
	static void RemoveCollation(sqlite3 *hDatabase, std::wstring name, int encoding,
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

#endif		// __ZDBCOLLATIONCOLLECTION_H_
