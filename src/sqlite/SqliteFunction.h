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

#ifndef __SQLITEFUNCTION_H_
#define __SQLITEFUNCTION_H_
#pragma once

#include "SqliteArgumentCollection.h"		// Include SqliteArgumentCollection decls
#include "SqliteResult.h"					// Include SqliteResult declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

using namespace System;

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// Forward Class Declarations
//---------------------------------------------------------------------------

ref class SqliteConnection;				// SqliteConnection.h

//---------------------------------------------------------------------------
// Delegate SqliteFunction
//
// Unlike aggregates, scalar functions can be implemented with a simple
// delegate.  Even though this is a carryover from the 1.1 provider, it works
// out really well with the new virtual table function overloading as well.
//---------------------------------------------------------------------------

public delegate void SqliteFunction(SqliteConnection^ conn, SqliteArgumentCollection^ args, 
	SqliteResult^ result);

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)

#endif		// __SQLITEFUNCTION_H_
