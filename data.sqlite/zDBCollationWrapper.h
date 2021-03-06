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

#ifndef __ZDBCOLLATIONWRAPPER_H_
#define __ZDBCOLLATIONWRAPPER_H_
#pragma once

#include "zDBCollation.h"				// Include zDBCollation declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

using namespace System;

BEGIN_NAMESPACE(zuki)
BEGIN_NAMESPACE(data)
BEGIN_NAMESPACE(dbms)

//---------------------------------------------------------------------------
// Forward Class Declarations
//---------------------------------------------------------------------------

ref class zDBConnection;				// zDBConnection.h

//---------------------------------------------------------------------------
// Class zDBCollationWrapper (internal)
//
// zDBCollationWrapper represents a wrapper around the main zDBCollation 
// delegate. This wrapper allows us to associate a SQLite database handle 
// with the collation so the connection context is known at the time of 
// execution. I frankly don't see how that's useful, but I did it for scalar 
// functions so I may as well do it here for consistency reasons.
//---------------------------------------------------------------------------

ref class zDBCollationWrapper
{
public:

	zDBCollationWrapper(zDBCollation^ collation) : m_std(collation) {}
	zDBCollationWrapper(zDBBinaryCollation^ collation) : m_bin(collation) {}

	//-----------------------------------------------------------------------
	// Member Functions

	// Invoke
	//
	// Invokes the contained delegate given the arguments provided
	int Invoke(const void* pvLeft, int cbLeft, const void* pvRight, int cbRight);

	//-----------------------------------------------------------------------
	// Public Properties

	// DatabaseHandle
	//
	// Gets/sets the database handle to associate with this function.  Changes
	// every time the connection is opened or closed, or at least that was the plan
	property sqlite3* DatabaseHandle
	{
		sqlite3* get(void) { return m_hDatabase; }
		void set(sqlite3* value) { m_hDatabase = value; }
	}

private:

	//-----------------------------------------------------------------------
	// Private Member Functions

	// InvokeBinary
	//
	// Invokes the binary byte array-based collation delegate
	int InvokeBinary(const void* pvLeft, int cbLeft, const void* pvRight, int cbRight);

	// InvokeString
	//
	// Invokes the string-based collation delegate
	int InvokeString(const void* pvLeft, int cbLeft, const void* pvRight, int cbRight);

	//-----------------------------------------------------------------------
	// Member Variables

	zDBCollation^				m_std;			// Collation delegate
	zDBBinaryCollation^			m_bin;			// Collation delegate
	sqlite3*					m_hDatabase;	// Active database handle
};

//---------------------------------------------------------------------------

END_NAMESPACE(dbms)
END_NAMESPACE(data)
END_NAMESPACE(zuki)

#pragma warning(pop)

#endif		// __ZDBCOLLATIONWRAPPER_H_
