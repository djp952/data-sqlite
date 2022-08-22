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

#ifndef __SQLITEINDEXCONSTRAINT_H_
#define __SQLITEINDEXCONSTRAINT_H_
#pragma once

#include "SqliteEnumerations.h"			// Include Sqlite enumeration decls

#pragma warning(push, 4)				// Enable maximum compiler warnings

using namespace System;
using namespace System::Data;
using namespace System::Data::Common;

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// Class SqliteIndexConstraint
//
// SqliteIndexConstraint is a managed wrapper around the sqlite3_index_info's
// aConstraint array.  This information is used with virtual tables, when
// the xBestIndex callback is invoked to allow the virtual table to select
// the 'best' index to use for a given SQL query against it.
//---------------------------------------------------------------------------

public ref class SqliteIndexConstraint sealed
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

	// DoubleCheck
	//
	// Gets/Sets a flag indicating if SQLite should double-check this
	// constraint for each row or not
	property bool DoubleCheck
	{
		bool get(void) { return m_doubleCheck; }
		void set(bool value) { m_doubleCheck = value; }
	}

	// FilterArgumentIndex
	//
	// Gets/Sets the index into the Filter() argument list where this
	// constraint will appear. Note that this is base 1, not base zero
	property int FilterArgumentIndex
	{
		int get(void) { return m_filterArgIndex; }
		void set(int value) { m_filterArgIndex = value; }
	}

	// IsUsable
	//
	// Determines if this constraint is usable or not
	property bool IsUsable
	{
		bool get(void) { return m_usable; }
	}

	// Operator
	//
	// Gets the constraint operator value
	property SqliteSearchOperator Operator
	{
		SqliteSearchOperator get(void) { return m_op; }
	}

internal:

	// INTERNAL CONSTRUCTOR
	SqliteIndexConstraint(const sqlite3_index_info::sqlite3_index_constraint* constraint,
		sqlite3_index_info::sqlite3_index_constraint_usage* usage)
	{
		m_ordinal = constraint->iColumn;
		m_op = static_cast<SqliteSearchOperator>(constraint->op);
		m_usable = (constraint->usable != 0);

		m_filterArgIndex = usage->argvIndex;		// Set default value
		m_doubleCheck = (usage->omit == 0);			// Set default value
	}

private:

	//-----------------------------------------------------------------------
	// Member Variables

	int						m_ordinal;				// aConstraint.iColumn
	SqliteSearchOperator		m_op;					// aConstraint.op
	bool					m_usable;				// aConstraint.usable
	int						m_filterArgIndex;		// aConstraintUsage.argvIndex
	bool					m_doubleCheck;			// aConstraintUsage.omit
};

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)

#endif		// __SQLITEINDEXCONSTRAINT_H_
