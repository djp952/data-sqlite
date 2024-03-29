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

#ifndef __SQLITEPERMISSION_H_
#define __SQLITEPERMISSION_H_
#pragma once

#include "SqlitePermissionAttribute.h"		// Include SqlitePermissionAttribute decls

#pragma warning(push, 4)				// Enable maximum compiler warnings

using namespace System;
using namespace System::Data;
using namespace System::Data::Common;
using namespace System::Security;
using namespace System::Security::Permissions;

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// Class SqlitePermission
//
// SqlitePermission is ... useful ... somehow.  Not really sure how, so this is
// pretty much just a stub until I figure out why Code Access Security is a 
// useful thing in ADO.NET.
//---------------------------------------------------------------------------

public ref class SqlitePermission sealed : public DBDataPermission
{
public:

	//-----------------------------------------------------------------------
	// Constructors

	SqlitePermission(PermissionState state) : DBDataPermission(state) {}

	//-----------------------------------------------------------------------
	// Member Functions

	// Copy (DBDataPermission)
	//
	// Creates a copy of this permission object
	virtual IPermission^ Copy(void) override { return gcnew SqlitePermission(this); }

internal:

	// INTERNAL CONSTRUCTOR
	SqlitePermission(SqlitePermissionAttribute^ attribute) : DBDataPermission(attribute) {}

private:

	// PRIVATE COPY CONSTRUCTOR
	SqlitePermission(SqlitePermission^ rhs) : DBDataPermission(rhs) {}
};

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)

#endif		// __SQLITEPERMISSION_H_
