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

#ifndef __SQLITEVIRTUALTABLECURSOR_H_
#define __SQLITEVIRTUALTABLECURSOR_H_
#pragma once

#include "SqliteArgument.h"				// Include SqliteArgument declarations
#include "SqliteArgumentCollection.h"		// Include SqliteArgumentCollection decls
#include "SqliteIndexIdentifier.h"			// Include SqliteIndexIdentifier decls
#include "SqliteResult.h"					// Include SqliteResult declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

using namespace System;
using namespace System::Data;
using namespace System::Data::Common;

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// Class SqliteVirtualTableCursor
//
// SqliteVirtualTableCursor is the base class from which all virtual table
// cursors must derive.  In order for the provider code to know what cursor
// goes with what virtual table, the cursor type must be declared as a
// generic argument to the SqliteVirtualTable class:
//
//	internal class MyCursor : public SqliteVirtualTableCursor
//	internal class MyTable : public SqliteVirtualTable<MyCursor> 
//---------------------------------------------------------------------------

public ref class SqliteVirtualTableCursor abstract
{
protected public:

	// PROTECTED CONSTRUCTOR
	SqliteVirtualTableCursor() {}

	//-----------------------------------------------------------------------
	// Protected/Public Member Functions

	// Close
	//
	//
	virtual void Close(void) abstract;

	// GetRowID
	//
	//
	virtual __int64 GetRowID(void) abstract;

	// GetValue
	//
	//
	virtual void GetValue(int ordinal, SqliteResult^ result) abstract;

	// MoveNext
	//
	//
	virtual bool MoveNext(void) abstract;

	// SetFilter (must override)
	//
	// Invoked to set/change the filter information for this cursor.  Provides
	// the index identifier created by the SqliteVirtualTable::SelectBestIndex()
	// method as the first parameter, and a collection of SqliteArguments that
	// represent all constraints set up with a .FilterArgumentIndex from that
	// same call into SqliteVirtualTable::SelectBestIndex().
	virtual bool SetFilter(SqliteIndexIdentifier^ index, SqliteArgumentCollection^ args) abstract;
};

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)

#endif		// __SQLITEVIRTUALTABLECURSOR_H_
