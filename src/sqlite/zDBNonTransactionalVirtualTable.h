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

#ifndef __ZDBNONTRANSACTIONALVIRTUALTABLE_H_
#define __ZDBNONTRANSACTIONALVIRTUALTABLE_H_
#pragma once

#include "zDBVirtualTable.h"		// Include zDBVirtualTable declarations

#pragma warning(push, 4)			// Enable maximum compiler warnings
#pragma warning(disable:4100)		// "unreferenced formal parameter"

using namespace System;
using namespace System::Data;

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// Class zDBNonTransactionalVirtualTable
//
// zDBNonTransactionalVirtualTable is a specialization of zDBVirtualTable
// that prevents the transaction-based callbacks from being registered, 
// therefore preventing the engine from attempting to include this table
// in it's transactions.
//---------------------------------------------------------------------------

generic<class Cursor>
where Cursor : zDBVirtualTableCursor
public ref class zDBNonTransactionalVirtualTable abstract : public zDBVirtualTable<Cursor>
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

	// RollbackTransaction
	//
	// Rolls back the current transaction open against the virtual table
	virtual void RollbackTransaction(void) override sealed
	{
		throw gcnew NotImplementedException();
	}
};

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)

#endif		// __ZDBNONTRANSACTIONALVIRTUALTABLE_H_
