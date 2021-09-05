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

#ifndef __ZDBREADONLYVIRTUALTABLE_H_
#define __ZDBREADONLYVIRTUALTABLE_H_
#pragma once

#include "zDBVirtualTable.h"		// Include zDBVirtualTable declarations

#pragma warning(push, 4)			// Enable maximum compiler warnings
#pragma warning(disable:4100)		// "unreferenced formal parameter"

using namespace System;
using namespace System::Data;

BEGIN_NAMESPACE(zuki)
BEGIN_NAMESPACE(data)
BEGIN_NAMESPACE(dbms)

//---------------------------------------------------------------------------
// Class zDBReadOnlyVirtualTable
//
// zDBReadOnlyVirtualTable is a specialization of zDBVirtualTable that when
// used prevents certain callbacks from being registered, and renders the
// virtual table read-only from the engine perspective.
//---------------------------------------------------------------------------

generic<class Cursor>
where Cursor : zDBVirtualTableCursor
public ref class zDBReadOnlyVirtualTable abstract : public zDBVirtualTable<Cursor>
{
protected:

	//-----------------------------------------------------------------------
	// Protected Member Functions

	// BeginTransaction
	//
	// Begins a new transaction against the virtual table
	virtual void BeginTransaction(void) override sealed
	{
		throw gcnew NotImplementedException();
	}

	// CommitTransaction
	//
	// Commits the current transaction open against the virtual table
	virtual void CommitTransaction(void) override sealed
	{
		throw gcnew NotImplementedException();
	}

	// DeleteRow
	//
	// Deletes the row with the specified ROWID from the virtual table
	virtual void DeleteRow(__int64 rowid) override sealed
	{ 
		throw gcnew NotImplementedException(); 
	}

	// InsertRow
	//
	// Inserts a new row with the specified ROWID and values into the table
	virtual void InsertRow(__int64 rowid, zDBArgumentCollection^ values) override sealed
	{ 
		throw gcnew NotImplementedException(); 
	}

	// NewRowID
	//
	// Generates a new unique ROWID for the virtual table
	virtual __int64 NewRowID(void) override sealed 
	{ 
		throw gcnew NotImplementedException(); 
	}

	// OnSynchronize
	//
	// Forces the virtual table to synchronize with it's backing store
	virtual void OnSynchronize(void) override sealed
	{
		throw gcnew NotImplementedException();
	}

	// RollbackTransaction
	//
	// Rolls back the current transaction open against the virtual table
	virtual void RollbackTransaction(void) override sealed
	{
		throw gcnew NotImplementedException();
	}

	// UpdateRow
	//
	// Updates the contents of the row with the specified ROWID in the table
	virtual void UpdateRow(__int64 rowid, zDBArgumentCollection^ values) override sealed
	{ 
		throw gcnew NotImplementedException(); 
	}

	// UpdateRowID
	//
	// Replaces an existing ROWID code with a new one.  In this wrapper, you
	// do not update the row contents, that will be done with a subsequent
	// call into UpdateRow
	virtual void UpdateRowID(__int64 oldrowid, __int64 newrowid) override sealed
	{ 
		throw gcnew NotImplementedException(); 
	}
};

//---------------------------------------------------------------------------

END_NAMESPACE(dbms)
END_NAMESPACE(data)
END_NAMESPACE(zuki)

#pragma warning(pop)

#endif		// __ZDBREADONLYVIRTUALTABLE_H_
