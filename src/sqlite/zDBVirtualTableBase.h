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

#ifndef __ZDBVIRTUALTABLEBASE_H_
#define __ZDBVIRTUALTABLEBASE_H_
#pragma once

#include "zDBEnumerations.h"			// Include zDB enumeration decls
#include "zDBFunction.h"				// Include zDBFunction declarations
#include "zDBFunctionWrapper.h"			// Include zDBFunctionWrapper decls
#include "zDBIndexSelectionArgs.h"		// Include zDBIndexInfo declarations
#include "zDBVirtualTableCursor.h"		// Include zDBVirtualTableCursor decls

#pragma warning(push, 4)				// Enable maximum compiler warnings

using namespace System;
using namespace System::Collections::ObjectModel;
using namespace System::Data;
using namespace System::Data::Common;

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// Interface zDBVirtualTableBase (internal)
//
// Interface that all specialized zDBVirtualTable classes derive so that the
// unmanaged portion of the implementation can access what it needs to using
// generic types instead of the application-defined specific types
//---------------------------------------------------------------------------

interface class zDBVirtualTableBase : public IDisposable
{
	// BeginTransaction
	//
	// Begins a transaction against the virtual table
	void BeginTransaction(void);

	// Close
	//
	// Called when the virtual table is being closed or is about to be dropped
	// from the database
	void Close(void);

	// CommitTransaction
	//
	// Commits the current transaction against the virtual table
	void CommitTransaction(void);

	// Create
	//
	// Called when the virtual table is being created in the database.
	// Connect() will always be called immediately after this, so in some
	// cases where there is no backing store this can go unimplemented.
	void Create(void);

	// CreateCursor
	//
	// Creates a new cursor for accessing data in the virtual table
	zDBVirtualTableCursor^ CreateCursor(void);

	// DeleteRow
	//
	// Deletes the row with the specified ROWID from the virtual table
	void DeleteRow(__int64 rowid);

	// Drop
	//
	// Called when the virtual table is being dropped from the database.
	// Disconnect() will always be called to close the table first, so in
	// some cases where there is no backing store this can go unimplemented.
	void Drop(void);

	// FindFunction
	//
	// Called when a function has been declared overridden to allow the
	// virtual table to provide SQLite with the new implementation
	bool FindFunction(String^ name, int argc, GCHandle% funcwrapper);

	// GetCreateTableStatement
	//
	// Returns the schema of the virtual table via a CREATE TABLE statement
	String^ GetCreateTableStatement(String^ name);

	// InsertRow
	//
	// Inserts a new row with the specified ROWID to the virtual table
	void InsertRow(__int64 rowid, zDBArgumentCollection^ values);

	// NewRowID
	//
	// Must create a new and unique ROWID for this virtual table
	__int64 NewRowID(void);

	// Open
	//
	// Called when a virtual table is being opened, or has just been created
	// in the database.
	void Open(void);

	// RollbackTransaction
	//
	// Rolls back the current transaction against the virtual table
	void RollbackTransaction(void);

	// SelectBestIndex
	//
	// Called when a virtual table is being queried, and SQLite needs to know
	// what the best index to use will be, given the provided constraints
	void SelectBestIndex(zDBIndexSelectionArgs^ args);

	// Synchronize
	//
	// Forces the virtual table to synchronize itself with it's backing store
	void Synchronize(void);

	// UpdateRow
	//
	// Updates the contents of a row with the specified values
	void UpdateRow(__int64 rowid, zDBArgumentCollection^ values);

	// UpdateRowID
	//
	// Changes the ROWID of a specific row. Can happen when users enter things
	// like "UPDATE [mytable] SET ROWID = ROWID + 1 WHERE ..."
	void UpdateRowID(__int64 oldrowid, __int64 newrowid);
};

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)

#endif		// __ZDBVIRTUALTABLEBASE_H_
