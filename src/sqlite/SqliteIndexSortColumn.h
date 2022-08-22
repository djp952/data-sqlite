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

#ifndef __SQLITEINDEXSORTCOLUMN_H_
#define __SQLITEINDEXSORTCOLUMN_H_
#pragma once

#include "SqliteEnumerations.h"			// Include Sqlite enumeration decls

#pragma warning(push, 4)				// Enable maximum compiler warnings

using namespace System;
using namespace System::Data;
using namespace System::Data::Common;

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// Class SqliteIndexSortColumn
//
// SqliteIndexSortColumn is a managed wrapper around the sqlite3_index_info's
// aOrderBy array.  This information is used with virtual tables, when
// the xBestIndex callback is invoked to allow the virtual table to select
// the 'best' index to use for a given SQL query against it.
//---------------------------------------------------------------------------

public ref class SqliteIndexSortColumn sealed
{
public:

	//-----------------------------------------------------------------------
	// Properties

	// ColumnOrdinal
	//
	// Gets the column ordinal on the left-hand size of the constraint
	property int ColumnOrdinal
	{
		int get(void) { return m_ordinal; }
	}

	// Direction
	//
	// Defines the direction of the referenced ORDER BY expression
	property SqliteSortDirection Direction
	{
		SqliteSortDirection get(void) { return m_direction; }
	}

internal:

	// INTERNAL CONSTRUCTOR
	SqliteIndexSortColumn(const sqlite3_index_info::sqlite3_index_orderby* orderby)
	{
		m_ordinal = orderby->iColumn;
		m_direction = (orderby->desc) ? SqliteSortDirection::Descending : SqliteSortDirection::Ascending;
	}

private:

	//-----------------------------------------------------------------------
	// Member Variables

	int						m_ordinal;				// .iColumn
	SqliteSortDirection		m_direction;			// .desc
};

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)

#endif		// __SQLITEINDEXSORTCOLUMN_H_
