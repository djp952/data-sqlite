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

#ifndef __ZDBINDEXSELECTIONARGS_H_
#define __ZDBINDEXSELECTIONARGS_H_
#pragma once

#include "AutoAnsiString.h"				// Include AutoAnsiString decls
#include "zDBEnumerations.h"			// Include zDB enumeration decls
#include "zDBIndexConstraint.h"			// Include zDBIndexConstraint decls
#include "zDBIndexIdentifier.h"			// Include zDBIndexIdentifier decls
#include "zDBIndexSortColumn.h"			// Include zDBIndexSortColumn decls

#pragma warning(push, 4)				// Enable maximum compiler warnings

using namespace System;
using namespace System::Collections::ObjectModel;
using namespace System::Data;
using namespace System::Diagnostics;

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// Class zDBIndexSelectionArgs
//
// zDBIndexSelectionArgs is a managed wrapper around the sqlite3_index_info 
// structure.  This information is used with virtual tables, when the xBestIndex 
// callback is invoked to allow the virtual table to select the 'best' index 
// to use for a given SQL query against it.
//---------------------------------------------------------------------------

public ref class zDBIndexSelectionArgs sealed
{
public:

	//-----------------------------------------------------------------------
	// Properties

	// Constraints
	//
	// Gets a reference to the contained zDBIndexConstraint collection
	property ReadOnlyCollection<zDBIndexConstraint^>^ Constraints
	{
		ReadOnlyCollection<zDBIndexConstraint^>^ get(void) { return m_constraints; }
	}

	// EstimatedCost
	//
	// Gets/sets a value indicating what the relative cost of the selected index
	// will be.  SQLite may call xBestIndex multiple times to see what combination
	// of constraints will generate the most efficient query
	property double EstimatedCost
	{
		double get(void) { return m_estimatedCost; }
		void set(double value) { m_estimatedCost = value; }
	}

	// Identifier
	//
	// Gets a reference to the contained index identifier class, which wraps up
	// both the index code and index string values from SQLite
	property zDBIndexIdentifier^ Identifier
	{
		zDBIndexIdentifier^ get(void) { return m_identifier; }
	}

	// SortColumns
	//
	// Gets a reference to the contained zDBIndexSortColumn collection
	property ReadOnlyCollection<zDBIndexSortColumn^>^ SortColumns
	{
		ReadOnlyCollection<zDBIndexSortColumn^>^ get(void) { return m_sortcols; }
	}

	// SortRequired
	//
	// Gets/Sets a flag indicating if SQLite will have to sort the data
	// after it's been retrieved or not
	property bool SortRequired
	{
		bool get(void) { return m_sortRequired; }
		void set(bool value) { m_sortRequired = value; }
	}

internal:

	// INTERNAL CONSTRUCTOR
	zDBIndexSelectionArgs(sqlite3_index_info* info);

	//-----------------------------------------------------------------------
	// Internal Member Functions

	// OutputToStructure
	//
	// Takes the output information provided by the user and applies it to
	// a sqlite3_index_info structure
	void OutputToStructure(sqlite3_index_info* info);

private:

	//-----------------------------------------------------------------------
	// Member Variables

	ReadOnlyCollection<zDBIndexConstraint^>^	m_constraints;		// aConstraint[]
	ReadOnlyCollection<zDBIndexSortColumn^>^	m_sortcols;			// aOrderBy[]

	zDBIndexIdentifier^		m_identifier;				// idxNum, idxStr
	bool					m_sortRequired;				// orderByConsumed
	double					m_estimatedCost;			// estimatedCost
};

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)

#endif		// __ZDBINDEXSELECTIONARGS_H_
