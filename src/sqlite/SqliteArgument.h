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

#ifndef __SQLITEARGUMENT_H_
#define __SQLITEARGUMENT_H_
#pragma once

#include "ITrackableObject.h"			// Include ITrackableObject decls

#pragma warning(push, 4)				// Enable maximum compiler warnings
#pragma warning(disable:4100)			// "unreferenced formal parameter"

using namespace System;
using namespace System::Data;

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// Class SqliteArgument
//
// SqliteArgument is used to provide a wrapper around function arguments as well
// as arguments passed into virtual table cursors.
//
// Note that arguments (or sqlite3_values, if you prefer) are designed to
// be volatile and must be properly disposed of when done working with them.
// This allows for the performance we need and prevents applications from
// sneaking references to the arguments out of context and accessing bad data
// long after the underlying SQLite pointers have disappeared.
//---------------------------------------------------------------------------

public ref class SqliteArgument sealed : public IConvertible, public ITrackableObject
{
public:

	//-----------------------------------------------------------------------
	// Overloaded Operators

	static explicit operator bool					(SqliteArgument^ rhs) { return rhs->ToBoolean(); }
	static explicit operator System::Byte			(SqliteArgument^ rhs) { return rhs->ToByte(); }
	static explicit operator array<System::Byte>^	(SqliteArgument^ rhs) { return rhs->ToBytes(); }
	static explicit operator __wchar_t				(SqliteArgument^ rhs) { return rhs->ToChar(); }
	static explicit operator array<Char>^			(SqliteArgument^ rhs) { return rhs->ToChars(); }
	static explicit operator DateTime				(SqliteArgument^ rhs) { return rhs->ToDateTime(); }
	static explicit operator double					(SqliteArgument^ rhs) { return rhs->ToDouble(); }
	static explicit operator Guid					(SqliteArgument^ rhs) { return rhs->ToGuid(); }
	static explicit operator short					(SqliteArgument^ rhs) { return rhs->ToInt16(); }
	static explicit operator int					(SqliteArgument^ rhs) { return rhs->ToInt32(); }
	static explicit operator __int64				(SqliteArgument^ rhs) { return rhs->ToInt64(); }
	static explicit operator SByte					(SqliteArgument^ rhs) { return rhs->ToSByte(); }
	static explicit operator float					(SqliteArgument^ rhs) { return rhs->ToSingle(); }
	static explicit operator String^				(SqliteArgument^ rhs) { return rhs->ToString(); }
	static explicit operator unsigned short			(SqliteArgument^ rhs) { return rhs->ToUInt16(); }
	static explicit operator unsigned int			(SqliteArgument^ rhs) { return rhs->ToUInt32(); }
	static explicit operator unsigned __int64		(SqliteArgument^ rhs) { return rhs->ToUInt64(); }
	
	//-----------------------------------------------------------------------
	// Member Functions

	// GetTypeCode (IConvertible)
	//
	// Gets a TypeCode that defines the underlying data type of the argument
	virtual TypeCode GetTypeCode(void);

	// ToBoolean
	//
	// Converts this argument into a System.Boolean
	bool ToBoolean(void);

	// ToByte
	//
	// Converts this argument into a System.Byte
	System::Byte ToByte(void);

	// ToBytes
	//
	// Converts this argument into a byte array
	array<System::Byte>^ ToBytes(void);

	// ToChar
	//
	// Converts this argument into a System.Char
	__wchar_t ToChar(void);

	// ToChars
	//
	// Converts this argument into a char array
	array<__wchar_t>^ ToChars(void);

	// ToDateTime
	//
	// Converts this argument into a System.DateTime
	DateTime ToDateTime(void);

	// ToDouble
	//
	// Converts this argument into a System.Double
	double ToDouble(void);

	// ToGuid
	//
	// Converts the argument into a System.Guid
	Guid ToGuid(void);

	// ToInt16
	//
	// Converts this argument into a System.Int16
	short ToInt16(void) { return static_cast<short>(ToInt32()); }

	// ToInt32
	//
	// Converts this argument into a System.Int32
	int ToInt32(void);

	// ToInt64
	//
	// Converts this argument into a System.Int64
	__int64 ToInt64(void);

	// ToSByte
	//
	// Converts this argument into a System.SByte
	SByte ToSByte(void) { return static_cast<SByte>(ToInt32()); }

	// ToSingle
	//
	// Converts this argument into a System.Single
	float ToSingle(void) { return static_cast<float>(ToDouble()); }

	// ToString (Object)
	//
	// Converts this argument into a System.String
	virtual String^ ToString(void) override;

	// ToUInt16
	//
	// Converts this argument into a System.UInt16
	unsigned short ToUInt16(void) { return static_cast<unsigned short>(ToInt32()); }

	// ToUInt32
	//
	// Converts this argument into a System.UInt32
	unsigned int ToUInt32(void) { return static_cast<unsigned int>(ToInt32()); }

	// ToUInt64
	//
	// Converts this argument into a System.UInt64
	unsigned __int64 ToUInt64(void) { return static_cast<unsigned int>(ToInt64()); }

	//-----------------------------------------------------------------------
	// Properties

	// IsNull
	//
	// Determines if this value is NULL or not (ok if disposed of)
	property bool IsNull { bool get(void) { return (m_type == SQLITE_NULL); } }

	// Size
	//
	// Gets the size of the argument data, in bytes (ok if disposed of)
	property int Size { int get(void) { return m_length; } }

	// Value
	//
	// Retrieves the value as a generic object, using the best conversion
	// possible based on the underlying SQLite data type
	property Object^ Value { Object^ get(void); }

internal:

	// INTERNAL CONSTRUCTOR
	SqliteArgument(sqlite3_value* value) : m_value(value), 
		m_type(sqlite3_value_type(value)), m_length(sqlite3_value_bytes(value)) {}

	//-----------------------------------------------------------------------
	// Internal Member Functions

	// IsDisposed (ITrackableObject)
	//
	// Exposes this object's internal dispose state
	virtual bool IsDisposed(void) sealed = ITrackableObject::IsDisposed
	{
		return m_disposed; 
	}

	// ToType (IConvertible)
	//
	// Attempts to change this value into the specified type
	virtual Object^	ToType(Type^ type, IFormatProvider^ fmt) sealed = IConvertible::ToType;

	//-----------------------------------------------------------------------
	// Internal Properties

	// Handle
	//
	// Exposes the internal sqlite3_value handle, since SqliteResult can take
	// advantage of a specific optimization involving it
	property sqlite3_value* Handle { sqlite3_value* get(void) { return m_value; } }

private:

	// DESTRUCTOR
	~SqliteArgument() { m_value = NULL; m_disposed = true; }

	//-----------------------------------------------------------------------
	// Private Member Functions

	// ToBoolean (IConvertible)
	//
	// Converts this binary data into a System.Boolean
	virtual bool ToBoolean(IFormatProvider^ fmt) sealed = IConvertible::ToBoolean { return ToBoolean(); }

	// ToByte (IConvertible)
	//
	// Converts this binary data into a System.Byte
	virtual System::Byte ToByte(IFormatProvider^ fmt) sealed = IConvertible::ToByte { return ToByte(); }

	// ToChar (IConvertible)
	//
	// Converts this binary value into a System.Char
	virtual __wchar_t ToChar(IFormatProvider^ fmt) sealed = IConvertible::ToChar { return ToChar(); }

	// ToDateTime (IConvertible)
	//
	// Converts this binary value into a System.DateTime
	virtual DateTime ToDateTime(IFormatProvider^ fmt) sealed = IConvertible::ToDateTime { return ToDateTime(); }

	// ToDecimal (IConvertible)
	//
	// Not supported
	virtual Decimal ToDecimal(IFormatProvider^ fmt) sealed = IConvertible::ToDecimal { throw gcnew InvalidCastException(); }

	// ToDouble (IConvertible)
	//
	// Converts this binary data into a System.Double
	virtual double ToDouble(IFormatProvider^ fmt) sealed = IConvertible::ToDouble {	return ToDouble(); }

	// ToInt16 (IConvertible)
	//
	// Converts this binary data into a System.Int16
	virtual short ToInt16(IFormatProvider^ fmt) sealed = IConvertible::ToInt16 { return ToInt16(); }

	// ToInt32 (IConvertible)
	//
	// Converts this binary data into a System.Int32
	virtual int ToInt32(IFormatProvider^ fmt) sealed = IConvertible::ToInt32 { return ToInt32(); }

	// ToInt64 (IConvertible)
	//
	// Converts this binary data into a System.Int64
	virtual __int64 ToInt64(IFormatProvider^ fmt) sealed = IConvertible::ToInt64 { return ToInt64(); }

	// ToSByte (IConvertible)
	//
	// Converts this binary data into a System.SByte
	virtual SByte ToSByte(IFormatProvider^ fmt) sealed = IConvertible::ToSByte { return ToSByte(); }

	// ToSingle (IConvertible)
	//
	// Converts this binary data into a System.Single
	virtual float ToSingle(IFormatProvider^ fmt) sealed = IConvertible::ToSingle { return ToSingle(); }

	// ToString (IConvertible)
	//
	// Converts this binary data into a System.String.  Always returns a constant
	// "<binary value>".  Use ToBytes() and convert manually as necessary
	virtual String^	ToString(IFormatProvider^ fmt) sealed = IConvertible::ToString { return ToString(); }

	// ToUInt16 (IConvertible)
	//
	// Converts this binary data into a System.UInt16
	virtual unsigned short ToUInt16(IFormatProvider^ fmt) sealed = IConvertible::ToUInt16 { return ToUInt16(); }

	// ToUInt32 (IConvertible)
	//
	// Converts this binary data into a System.UInt32
	virtual unsigned int ToUInt32(IFormatProvider^ fmt) sealed = IConvertible::ToUInt32 { return ToUInt32(); }

	// ToUInt64 (IConvertible)
	//
	// Converts this binary data into a System.UInt64
	virtual unsigned __int64 ToUInt64(IFormatProvider^ fmt) sealed = IConvertible::ToUInt64 { return ToUInt64(); }

	//-----------------------------------------------------------------------
	// Member Variables

	bool					m_disposed;			// Object disposal flag
	sqlite3_value*			m_value;			// Contained value pointer
	int						m_type;				// Value SQLite data type
	int						m_length;			// Length of the data
};

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)

#endif		// __SQLITEARGUMENT_H_
