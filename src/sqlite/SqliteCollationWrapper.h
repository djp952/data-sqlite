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

#ifndef __SQLITECOLLATIONWRAPPER_H_
#define __SQLITECOLLATIONWRAPPER_H_
#pragma once

#include "SqliteCollation.h"				// Include SqliteCollation declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

using namespace System;

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// Forward Class Declarations
//---------------------------------------------------------------------------

ref class SqliteConnection;				// SqliteConnection.h

//---------------------------------------------------------------------------
// Class SqliteCollationWrapper (internal)
//
// SqliteCollationWrapper represents a wrapper around the main SqliteCollation 
// delegate. This wrapper allows us to associate a SQLite database handle 
// with the collation so the connection context is known at the time of 
// execution. I frankly don't see how that's useful, but I did it for scalar 
// functions so I may as well do it here for consistency reasons.
//---------------------------------------------------------------------------

ref class SqliteCollationWrapper
{
public:

	SqliteCollationWrapper(SqliteCollation^ collation) : m_std(collation) {}
	SqliteCollationWrapper(SqliteBinaryCollation^ collation) : m_bin(collation) {}

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

	SqliteCollation^				m_std;			// Collation delegate
	SqliteBinaryCollation^			m_bin;			// Collation delegate
	sqlite3*					m_hDatabase;	// Active database handle
};

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)

#endif		// __SQLITECOLLATIONWRAPPER_H_
