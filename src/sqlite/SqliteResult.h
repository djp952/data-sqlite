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

#ifndef __SQLITERESULT_H_
#define __SQLITERESULT_H_
#pragma once

#include "SqliteArgument.h"				// Include SqliteArgument declarations
#include "SqliteBinaryStream.h"			// Include SqliteBinaryStream decls
#include "SqliteEnumerations.h"			// Include Sqlite enumeration decls

#pragma warning(push, 4)				// Enable maximum compiler warnings
#pragma warning(disable:4100)			// "unreferenced formal parameter"

using namespace System;
using namespace System::Data;
using namespace System::Globalization;

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// Forward Class Declarations
//---------------------------------------------------------------------------

ref class SqliteConnection;				// SqliteConnection.h

//---------------------------------------------------------------------------
// Class SqliteResult
//
// SqliteResult is used to provide a wrapper around function result values as
// well as results passed back from virtual table cursors. In the case of
// virtual tables, see SqliteValue which uses this class internally.
//
// Note that this class contains a volatile pointer into SQLite and must
// be disposed of before the pointer becomes invalid so the application
// cannot copy out a reference to this and hurt itself by using that pointer
//---------------------------------------------------------------------------

public ref class SqliteResult sealed
{
public:

	//-----------------------------------------------------------------------
	// Overloaded Operators

	// NOTE: For whatever reason, only C++ seems to benefit from assignment
	// operators in .NET.  This is a real shame, because I'd much rather see
	// people write code like this ...
	//
	// result = 123;
	//
	// ... than this monstrosity ...
	//
	// result.SetInt32(123);  (or result.op_Assign(123), but that's worse)
	//
	// But for now, I have to provide the longer SetXXXX versions and just
	// cross my fingers that other people will see the usefulness of assignment
	// operators at some point in the future and they'll stick it into the
	// other primary .NET languages.  When they do, we'll be ready!

	SqliteResult^ operator=(SqliteArgument^ rhs)			{ SetArgument(rhs); return this; }
	SqliteResult^ operator=(SqliteBinaryStream^ rhs)		{ SetBinaryStream(rhs); return this; }
	SqliteResult^ operator=(bool rhs)					{ SetBoolean(rhs); return this; }
	SqliteResult^ operator=(System::Byte rhs)			{ SetByte(rhs); return this; }
	SqliteResult^ operator=(array<System::Byte>^ rhs)	{ SetBytes(rhs); return this; }
	SqliteResult^ operator=(__wchar_t rhs)				{ SetChar(rhs); return this; }
	SqliteResult^ operator=(array<__wchar_t>^ rhs)		{ SetChars(rhs); return this; }
	SqliteResult^ operator=(DateTime rhs)				{ SetDateTime(rhs); return this; }
	SqliteResult^ operator=(double rhs)				{ SetDouble(rhs); return this; }
	SqliteResult^ operator=(Exception^ rhs)			{ SetError(rhs->Message); return this; }
	SqliteResult^ operator=(Guid rhs)					{ SetGuid(rhs); return this; }
	SqliteResult^ operator=(short rhs)					{ SetInt16(rhs); return this; }
	SqliteResult^ operator=(int rhs)					{ SetInt32(rhs); return this; }
	SqliteResult^ operator=(__int64 rhs)				{ SetInt64(rhs); return this; }
	SqliteResult^ operator=(DBNull rhs)				{ SetNull(); return this; }
	SqliteResult^ operator=(SByte rhs)					{ SetSByte(rhs); return this; }
	SqliteResult^ operator=(float rhs)					{ SetSingle(rhs); return this; }
	SqliteResult^ operator=(String^ rhs)				{ SetString(rhs); return this; }
	SqliteResult^ operator=(unsigned short rhs)		{ SetUInt16(rhs); return this; }
	SqliteResult^ operator=(unsigned int rhs)			{ SetUInt32(rhs); return this; }
	SqliteResult^ operator=(unsigned __int64 rhs)		{ SetUInt64(rhs); return this; }

	//-----------------------------------------------------------------------
	// Member Functions

	// SetArgument
	//
	// Sets the result to be an exact copy of a SqliteArgument object.  Does
	// not relate to the virtual table use pattern
	void SetArgument(SqliteArgument^ value);

	// SetBinaryStream
	//
	// Sets the result to be a BLOB based on a SqliteBinaryStream object instance.
	// This is the most efficient means of transmitting a large binary result
	// out of the function/virtual table, since it does not require SQLite to
	// make a copy of the binary data itself.  That said, once you associate it
	// with a result like this, it's contents become immutable.  That's not 
	// really an issue when dealing with functions and virtual tables, though.
	void SetBinaryStream(SqliteBinaryStream^ value);

	// SetBoolean
	//
	// Sets the result to be a boolean value.  The format used to return that
	// value can optionally be specified, and if not the default will be used.
	void SetBoolean(bool value);
	void SetBoolean(bool value, SqliteBooleanFormat fmt);

	// SetByte
	//
	// Sets the result to be an unsigned 8bit integer value. To set a 1-byte
	// BLOB value, SetBytes() must be used instead.  This is generally what I
	// think people will want to do when calling this.
	void SetByte(System::Byte value) { SetInt32(static_cast<int>(value)); }

	// SetBytes
	//
	// Sets the result to be an array of bytes (BLOB)
	void SetBytes(array<System::Byte>^ value) { SetBytes(value, 0, value->Length); }
	void SetBytes(array<System::Byte>^ value, int length) { SetBytes(value, 0, length); }
	void SetBytes(array<System::Byte>^ value, int offset, int length);

	// SetChar
	//
	// Sets the result to be a single unicode character.  This is the same
	// as calling SetString() with a 1 character long string.
	void SetChar(__wchar_t value);

	// SetChars
	//
	// Sets the result to be an array of unicode characters (TEXT)
	void SetChars(array<__wchar_t>^ value) { SetChars(value, 0, value->Length); }
	void SetChars(array<__wchar_t>^ value, int length) { SetChars(value, 0, length); }
	void SetChars(array<__wchar_t>^ value, int offset, int length);

	// SetDateTime
	//
	// Sets the result to be a date/time value.  The format used to return that
	// value can optionally be specified, and if not the default will be used.
	void SetDateTime(DateTime value);
	void SetDateTime(DateTime value, SqliteDateTimeFormat fmt);

	// SetDouble
	//
	// Sets the result to be a 64bit floating point value
	void SetDouble(double value);

	// SetError
	//
	// Sets the result to be an error message
	void SetError(String^ message);

	// SetGuid
	//
	// Sets the result to be a GUID value.  The format used to return that
	// value can optionally be specified, and if not the default will be used.
	void SetGuid(Guid value);
	void SetGuid(Guid value, SqliteGuidFormat fmt);

	// SetInt16
	//
	// Sets the result to be a 16bit integer-based value
	void SetInt16(short value) { SetInt32(static_cast<int>(value)); }

	// SetInt32
	//
	// Sets the result to be a 32bit integer-based value
	void SetInt32(int value);

	// SetInt64
	//
	// Sets the result to be a 64bit integer-based value
	void SetInt64(__int64 value);

	// SetNull
	//
	// Sets the result to be NULL
	void SetNull(void);

	// SetSByte
	//
	// Sets the result to be an 8bit integer-based value
	void SetSByte(SByte value) { SetInt32(static_cast<int>(value)); }

	// SetSingle
	//
	// Sets the result to be a 32bit floating point value
	void SetSingle(float value) { SetDouble(static_cast<double>(value)); }

	// SetString
	//
	// Sets the result to be a string-based value
	void SetString(String^ value);

	// SetUInt16
	//
	// Sets the result to be an unsigned 16bit integer value
	void SetUInt16(unsigned short value) { SetInt32(static_cast<int>(value)); }

	// SetUInt32
	//
	// Sets the result to be an unsigned 32bit integer value
	void SetUInt32(unsigned int value) { SetInt32(static_cast<int>(value)); }

	// SetUInt64
	//
	// Sets the result to be an unsigned 64bit integer value
	void SetUInt64(unsigned __int64 value) { SetInt64(static_cast<__int64>(value)); }

internal:

	// INTERNAL CONSTRUCTORS
	SqliteResult(sqlite3_context* context) : m_context(context) {}
	SqliteResult(SqliteConnection^ conn, sqlite3_context* context) : m_conn(conn), m_context(context) {}

private:

	// DESTRUCTOR
	~SqliteResult() { m_context = NULL; m_disposed = true; }

	//-----------------------------------------------------------------------
	// Member Variables

	bool					m_disposed;			// Object disposal flag
	SqliteConnection^			m_conn;				// Parent connection reference
	sqlite3_context*		m_context;			// Contained context pointer
};

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)

#endif		// __SQLITERESULT_H_
