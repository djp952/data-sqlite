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

#ifndef __ZDBVIRTUALTABLECURSOR_H_
#define __ZDBVIRTUALTABLECURSOR_H_
#pragma once

#include "zDBArgument.h"				// Include zDBArgument declarations
#include "zDBArgumentCollection.h"		// Include zDBArgumentCollection decls
#include "zDBIndexIdentifier.h"			// Include zDBIndexIdentifier decls
#include "zDBResult.h"					// Include zDBResult declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

using namespace System;
using namespace System::Data;
using namespace System::Data::Common;

BEGIN_NAMESPACE(zuki)
BEGIN_NAMESPACE(data)
BEGIN_NAMESPACE(dbms)

//---------------------------------------------------------------------------
// Class zDBVirtualTableCursor
//
// zDBVirtualTableCursor is the base class from which all virtual table
// cursors must derive.  In order for the provider code to know what cursor
// goes with what virtual table, the cursor type must be declared as a
// generic argument to the zDBVirtualTable class:
//
//	internal class MyCursor : public zDBVirtualTableCursor
//	internal class MyTable : public zDBVirtualTable<MyCursor> 
//---------------------------------------------------------------------------

public ref class zDBVirtualTableCursor abstract
{
protected public:

	// PROTECTED CONSTRUCTOR
	zDBVirtualTableCursor() {}

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
	virtual void GetValue(int ordinal, zDBResult^ result) abstract;

	// MoveNext
	//
	//
	virtual bool MoveNext(void) abstract;

	// SetFilter (must override)
	//
	// Invoked to set/change the filter information for this cursor.  Provides
	// the index identifier created by the zDBVirtualTable::SelectBestIndex()
	// method as the first parameter, and a collection of zDBArguments that
	// represent all constraints set up with a .FilterArgumentIndex from that
	// same call into zDBVirtualTable::SelectBestIndex().
	virtual bool SetFilter(zDBIndexIdentifier^ index, zDBArgumentCollection^ args) abstract;
};

//---------------------------------------------------------------------------

END_NAMESPACE(dbms)
END_NAMESPACE(data)
END_NAMESPACE(zuki)

#pragma warning(pop)

#endif		// __ZDBVIRTUALTABLECURSOR_H_
