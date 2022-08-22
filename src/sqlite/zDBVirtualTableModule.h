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

#ifndef __ZDBVIRTUALTABLEMODULE_H_
#define __ZDBVIRTUALTABLEMODULE_H_
#pragma once

#include "AutoAnsiString.h"						// Include AutoAnsiString decls
#include "GCHandleRef.h"						// Include GCHandleRef declarations
#include "VirtualTable.h"						// Include VirtualTable declarations
#include "VirtualTableCursor.h"					// Include VirtualTableCursor decls
#include "zDBArgumentCollection.h"				// Include zDBArgumentCollection decls
#include "zDBException.h"						// Include zDBException declarations
#include "zDBExceptions.h"						// Include zDB exception declarations
#include "zDBFunction.h"						// Include zDBFunction declarations
#include "zDBFunctionWrapper.h"					// Include zDBFunctionWrapper declarations
#include "zDBNonTransactionalVirtualTable.h"	// Include zDBNonTransactionalVirtualTable
#include "zDBReadOnlyVirtualTable.h"			// Include zDBReadOnlyVirtualTable decls
#include "zDBResult.h"							// Include zDBResult declarations
#include "zDBUtil.h"							// Include zDBUtil declarations
#include "zDBVirtualTable.h"					// Include zDBVirtualTable declarations
#include "zDBVirtualTableBase.h"				// Include zDBVirtualTableBase decls
#include "zDBVirtualTableConstructorArgs.h"		// Include zDBVirtualTableConstructorArgs

#pragma warning(push, 4)						// Enable maximum compiler warnings

using namespace System;
using namespace System::Collections::ObjectModel;
using namespace System::Data;
using namespace System::Data::Common;

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// Class zDBVirtualTableModule (internal, static)
//
// zDBVirtualTableModule is really just a broken out set of stuff that deals
// with the SQLite module implementation.  The code was muddying up the main
// virtual table class files, so I just offloaded it all into it's own space
//---------------------------------------------------------------------------

ref class zDBVirtualTableModule abstract sealed
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

#endif		// __ZDBVIRTUALTABLEMODULE_H_
