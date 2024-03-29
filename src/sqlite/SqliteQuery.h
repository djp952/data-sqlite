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

#ifndef __SQLITEQUERY_H_
#define __SQLITEQUERY_H_
#pragma once

#include "DatabaseHandle.h"				// Include DatabaseHandle decls
#include "StatementHandle.h"			// Include StatementHandle decls
#include "SqliteException.h"				// Include SqliteException declarations
#include "SqliteStatement.h"				// Include SqliteStatement decls

#pragma warning(push, 4)				// Enable maximum compiler warnings

using namespace System;
using namespace System::Collections;
using namespace System::Collections::Generic;

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// Class SqliteQuery (internal)
//
// SqliteQuery implements an enumerable collection of SqliteStatement objects.
// This collection is build by parsing a single SQL statement string into
// all of it's individual statements.
//---------------------------------------------------------------------------

ref class SqliteQuery sealed : public Generic::IEnumerable<SqliteStatement^>
{
public:

	// Constructor
	// 
	// Accepts an existing SQLITE database handle and the SQL query text
	SqliteQuery(DatabaseHandle* pDatabase, String^ query);

	//-----------------------------------------------------------------------
	// Member Functions

	// GetEnumerator (IEnumerable<T>)
	//
	// Returns an IEnumerator<T> against the member collection
	virtual Generic::IEnumerator<SqliteStatement^>^ GetEnumerator(void);

	//-----------------------------------------------------------------------
	// Properties

	// ChangeCount
	//
	// Retrieves the total number of rows affected by the entire query
	property int ChangeCount { int get(void); }

	// StatementCount
	//
	// Retrieves the number of statements in the collection
	property int StatementCount { int get(void); }

	//-----------------------------------------------------------------------
	// Indexers

	property SqliteStatement^ default[int] { SqliteStatement^ get(int index); }

private:

	// DESTRUCTOR
	~SqliteQuery();

	//-----------------------------------------------------------------------
	// Private Member Functions

	// IEnumerator::GetEnumerator
	virtual Collections::IEnumerator^ _GetEnumerator(void) sealed = 
		Collections::IEnumerable::GetEnumerator { return GetEnumerator(); }

	//-----------------------------------------------------------------------
	// Member Variables

	bool						m_disposed;		// Object disposal flag
	List<SqliteStatement^>^		m_col;			// Contained collection
};

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)

#endif		// __SQLITEQUERY_H_
