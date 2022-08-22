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

#ifndef __SQLITECOLLATION_H_
#define __SQLITECOLLATION_H_
#pragma once

#pragma warning(push, 4)				// Enable maximum compiler warnings

using namespace System;

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// Forward Class Declarations
//---------------------------------------------------------------------------

ref class SqliteConnection;				// SqliteConnection.h

//---------------------------------------------------------------------------
// Delegate SqliteCollation
//
// Defines a collation delegate.  This standard version is used to compare
// two strings with the native machine UTF16 encoding.  If something other
// than this is required, use SqliteBinaryCollation, which provides both arguments
// as an array of bytes that can be treated however the implementation sees fit
//---------------------------------------------------------------------------

public delegate int SqliteCollation(SqliteConnection^ conn, String^ left, String^ right);

//---------------------------------------------------------------------------
// Delegate SqliteBinaryCollation
//
// Defines a collation delegate.  This version provides the data as two byte
// arrays that can be treated however the implementation sees fit.  If just
// comparing strings and the native UTF16 encoding is acceptable, use the
// standard SqliteCollation instead.
//---------------------------------------------------------------------------

public delegate int SqliteBinaryCollation(SqliteConnection^ conn, array<System::Byte>^ left, array<System::Byte>^ right);

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)

#endif		// __SQLITECOLLATION_H_
