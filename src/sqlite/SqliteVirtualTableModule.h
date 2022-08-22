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

#ifndef __SQLITEVIRTUALTABLEMODULE_H_
#define __SQLITEVIRTUALTABLEMODULE_H_
#pragma once

#include "AutoAnsiString.h"						// Include AutoAnsiString decls
#include "GCHandleRef.h"						// Include GCHandleRef declarations
#include "VirtualTable.h"						// Include VirtualTable declarations
#include "VirtualTableCursor.h"					// Include VirtualTableCursor decls
#include "SqliteArgumentCollection.h"				// Include SqliteArgumentCollection decls
#include "SqliteException.h"						// Include SqliteException declarations
#include "SqliteExceptions.h"						// Include Sqlite exception declarations
#include "SqliteFunction.h"						// Include SqliteFunction declarations
#include "SqliteFunctionWrapper.h"					// Include SqliteFunctionWrapper declarations
#include "SqliteNonTransactionalVirtualTable.h"	// Include SqliteNonTransactionalVirtualTable
#include "SqliteReadOnlyVirtualTable.h"			// Include SqliteReadOnlyVirtualTable decls
#include "SqliteResult.h"							// Include SqliteResult declarations
#include "SqliteUtil.h"							// Include SqliteUtil declarations
#include "SqliteVirtualTable.h"					// Include SqliteVirtualTable declarations
#include "SqliteVirtualTableBase.h"				// Include SqliteVirtualTableBase decls
#include "SqliteVirtualTableConstructorArgs.h"		// Include SqliteVirtualTableConstructorArgs

#pragma warning(push, 4)						// Enable maximum compiler warnings

using namespace System;
using namespace System::Collections::ObjectModel;
using namespace System::Data;
using namespace System::Data::Common;

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// Class SqliteVirtualTableModule (internal, static)
//
// SqliteVirtualTableModule is really just a broken out set of stuff that deals
// with the SQLite module implementation.  The code was muddying up the main
// virtual table class files, so I just offloaded it all into it's own space
//---------------------------------------------------------------------------

ref class SqliteVirtualTableModule abstract sealed
{
public:

	//-----------------------------------------------------------------------
	// Member Functions

	// GetMethods
	//
	// Returns a pointer to one of the local sqlite3_module implementations
	// based on the type of virtual table being registered
	static const sqlite3_module* GetMethods(Type^ vtableType);

	// IsValidVirtualTableType
	//
	// Determines if the specified type represents a valid virtual table
	static bool IsValidVirtualTableType(Type^ vtableType);
};

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)

#endif		// __SQLITEVIRTUALTABLEMODULE_H_
