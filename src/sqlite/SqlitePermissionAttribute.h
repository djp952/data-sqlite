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

#ifndef __SQLITEPERMISSIONATTRIBUTE_H_
#define __SQLITEPERMISSIONATTRIBUTE_H_
#pragma once

#pragma warning(push, 4)				// Enable maximum compiler warnings

using namespace System;
using namespace System::Data;
using namespace System::Data::Common;
using namespace System::Security;
using namespace System::Security::Permissions;

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// Forward Class Declarations
//---------------------------------------------------------------------------

ref class SqlitePermission;				// SqlitePermission.h

//---------------------------------------------------------------------------
// Class SqlitePermissionAttribute
//
// SqlitePermissionAttribute associates a security action with a custom security 
// attribute.  Pretty basic stuff, but this is all the built-in ADO.NET
// providers seem to do with their implementations, so I'm assuming this will
// be sufficient (once I realize how Code Access Security is a useful thing
// when dealing with ADO.NET providers)
//---------------------------------------------------------------------------

public ref class SqlitePermissionAttribute sealed : public DBDataPermissionAttribute
{
public:

	//-----------------------------------------------------------------------
	// Constructors

	SqlitePermissionAttribute(SecurityAction action) : DBDataPermissionAttribute(action) {}

	//-----------------------------------------------------------------------
	// Member Functions

	// CreatePermission
	//
	// Creates a permission object that can be serialized into an assembly's metadata
	virtual IPermission^ CreatePermission(void) override;
};

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)

#endif		// __SQLITEPERMISSIONATTRIBUTE_H_
