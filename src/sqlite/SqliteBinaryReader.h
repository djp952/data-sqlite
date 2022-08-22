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

#ifndef __SQLITEBINARYREADER_H_
#define __SQLITEBINARYREADER_H_
#pragma once

#include "ITrackableObject.h"			// Include ITrackableObject decls
#include "StatementHandle.h"			// Include StatementHandle decls

#pragma warning(push, 4)				// Enable maximum compiler warnings
#pragma warning(disable:4100)			// "unreferenced formal parameter"

using namespace System;

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// Class SqliteBinaryReader
//
// SqliteBinaryReader is the data type returned from the SqliteDataReader's GetValue()
// method.  This object originally just implemented enough to make a data
// adapter happy and display "<binary data>", but has since been expanded
// into something far more useful.  Now it's the most efficient way of accessing
// portions of a BLOB without automatically creating a byte array that 
// encompasses the entire data (and thus taking up twice the amount of local
// memory). In practice, the speed increase isn't that noticable until you
// start working with really large BLOB values.
//
// Oh, and the new DataGrid in .NET 2.0 doesn't behave the same way that the
// 1.1 version did, and it blows up on some Byte[] values anyway.  C'est la vie.
//---------------------------------------------------------------------------

public ref class SqliteBinaryReader sealed : public IConvertible, public ITrackableObject
{
public:

	//-----------------------------------------------------------------------
	// Overloaded Operators

	static explicit operator bool					(SqliteBinaryReader^ rhs) { return rhs->ToBoolean(); }
	static explicit operator System::Byte			(SqliteBinaryReader^ rhs) { return rhs->ToByte(); }
	static explicit operator array<System::Byte>^	(SqliteBinaryReader^ rhs) { return rhs->ToBytes(); }
	static explicit operator __wchar_t				(SqliteBinaryReader^ rhs) { return rhs->ToChar(); }
	static explicit operator array<Char>^			(SqliteBinaryReader^ rhs) { return rhs->ToChars(); }
	static explicit operator DateTime				(SqliteBinaryReader^ rhs) { return rhs->ToDateTime(); }
	static explicit operator double					(SqliteBinaryReader^ rhs) { return rhs->ToDouble(); }
	static explicit operator Guid					(SqliteBinaryReader^ rhs) { return rhs->ToGuid(); }
	static explicit operator short					(SqliteBinaryReader^ rhs) { return rhs->ToInt16(); }
	static explicit operator int					(SqliteBinaryReader^ rhs) { return rhs->ToInt32(); }
	static explicit operator __int64				(SqliteBinaryReader^ rhs) { return rhs->ToInt64(); }
	static explicit operator SByte					(SqliteBinaryReader^ rhs) { return rhs->ToSByte(); }
	static explicit operator float					(SqliteBinaryReader^ rhs) { return rhs->ToSingle(); }
	static explicit operator String^				(SqliteBinaryReader^ rhs) { return rhs->ToString(); }
	static explicit operator unsigned short			(SqliteBinaryReader^ rhs) { return rhs->ToUInt16(); }
	static explicit operator unsigned int			(SqliteBinaryReader^ rhs) { return rhs->ToUInt32(); }
	static explicit operator unsigned __int64		(SqliteBinaryReader^ rhs) { return rhs->ToUInt64(); }
	
	//-----------------------------------------------------------------------
	// Member Functions

	// ToBoolean
	//
	// Converts this binary value into a System.Boolean
	bool ToBoolean(void) { return ToBoolean(0); }
	bool ToBoolean(int offset);

	// ToByte
	//
	// Converts this binary value into a System.Byte
	System::Byte ToByte(void) { return ToByte(0); }
	System::Byte ToByte(int offset);

	// ToBytes
	//
	// Converts this binary value into a byte array
	array<System::Byte>^ ToBytes(void);
	array<System::Byte>^ ToBytes(int offset);
	array<System::Byte>^ ToBytes(int offset, int length);

	// ToChar
	//
	// Converts this binary value into a System.Char
	__wchar_t ToChar(void) { return ToChar(0); }
	__wchar_t ToChar(int offset);

	// ToChars
	//
	// Converts this binary value into a char array
	array<__wchar_t>^ ToChars(void);
	array<__wchar_t>^ ToChars(int offset);
	array<__wchar_t>^ ToChars(int offset, int length);

	// ToDateTime
	//
	// Converts this binary value into a System.DateTime
	DateTime ToDateTime(void) { return ToDateTime(0); }
	DateTime ToDateTime(int offset);

	// ToDouble
	//
	// Converts this binary value into a System.Double
	double ToDouble(void) { return ToDouble(0); }
	double ToDouble(int offset);

	// ToGuid
	//
	// Converts the binary value into a System.Guid
	Guid ToGuid(void) { return ToGuid(0); }
	Guid ToGuid(int offset);

	// ToInt16
	//
	// Converts this binary value into a System.Int16
	short ToInt16(void) { return ToInt16(0); }
	short ToInt16(int offset);

	// ToInt32
	//
	// Converts this binary value into a System.Int32
	int ToInt32(void) { return ToInt32(0); }
	int ToInt32(int offset);

	// ToInt64
	//
	// Converts this binary value into a System.Int64
	__int64 ToInt64(void) { return ToInt64(0); }
	__int64 ToInt64(int offset);

	// ToSByte
	//
	// Converts this binary value into a Systen.SByte
	SByte ToSByte(void) { return ToSByte(0); }
	SByte ToSByte(int offset);

	// ToSingle
	//
	// Converts this binary value into a System.Single
	float ToSingle(void) { return ToSingle(0); }
	float ToSingle(int offset);

	// ToString (Object)
	//
	// Returns a constant string, does not convert the binary data
	virtual String^ ToString(void) override;

	// ToUInt16
	//
	// Converts this binary value into a System.UInt16
	unsigned short ToUInt16(void) { return ToUInt16(0); }
	unsigned short ToUInt16(int offset);

	// ToUInt32
	//
	// Converts this binary value into a System.UInt32
	unsigned int ToUInt32(void) { return ToUInt32(0); }
	unsigned int ToUInt32(int offset);

	// ToUInt64
	//
	// Converts this binary value into a System.UInt64
	unsigned __int64 ToUInt64(void) { return ToUInt64(0); }
	unsigned __int64 ToUInt64(int offset);

	//-----------------------------------------------------------------------
	// Properties

	// Length
	//
	// Length of the underlying binary data
	property int Length { int get(void); }

internal:

	// INTERNAL CONSTRUCTOR
	SqliteBinaryReader(StatementHandle* pStatement, int nOrdinal);

	//-----------------------------------------------------------------------
	// Internal Member Functions

	// ToType (IConvertible)
	//
	// Attempts to change this value into the specified type
	virtual Object^	ToType(Type^ type, IFormatProvider^ fmt) sealed = IConvertible::ToType;

private:

	// DESTRUCTOR / FINALIZER
	~SqliteBinaryReader() { this->!SqliteBinaryReader(); m_disposed = true; }
	!SqliteBinaryReader();

	//-----------------------------------------------------------------------
	// Private Constants

	// BINARY_DATA_STRING
	//
	// Returned when ToString() is called against this object
	literal String^ BINARY_DATA_STRING = "SqliteBinaryReader({0})";

	//-----------------------------------------------------------------------
	// Private Member Functions

	// GetTypeCode (IConvertible)
	//
	// Returns a type code for this object -- not really supported
	virtual TypeCode GetTypeCode(void) sealed = IConvertible::GetTypeCode;

	// IsDisposed (ITrackableObject.IsDisposed)
	//
	// Determines if this object has been disposed of yet or not
	virtual bool IsDisposed(void) sealed = ITrackableObject::IsDisposed { return m_disposed; }

	// ToBoolean (IConvertible)
	//
	// Converts this binary data into a System.Boolean
	virtual bool ToBoolean(IFormatProvider^ fmt) sealed = IConvertible::ToBoolean { return ToBoolean(0); }

	// ToByte (IConvertible)
	//
	// Converts this binary data into a System.Byte
	virtual System::Byte ToByte(IFormatProvider^ fmt) sealed = IConvertible::ToByte { return ToByte(0); }

	// ToChar (IConvertible)
	//
	// Converts this binary value into a System.Char
	virtual __wchar_t ToChar(IFormatProvider^ fmt) sealed = IConvertible::ToChar { return ToChar(0); }

	// ToDateTime (IConvertible)
	//
	// Converts this binary value into a System.DateTime
	virtual DateTime ToDateTime(IFormatProvider^ fmt) sealed = IConvertible::ToDateTime { return ToDateTime(0); }

	// ToDecimal (IConvertible)
	//
	// Not supported
	virtual Decimal ToDecimal(IFormatProvider^ fmt) sealed = IConvertible::ToDecimal { throw gcnew InvalidCastException(); }

	// ToDouble (IConvertible)
	//
	// Converts this binary data into a System.Double
	virtual double ToDouble(IFormatProvider^ fmt) sealed = IConvertible::ToDouble {	return ToDouble(0); }

	// ToInt16 (IConvertible)
	//
	// Converts this binary data into a System.Int16
	virtual short ToInt16(IFormatProvider^ fmt) sealed = IConvertible::ToInt16 { return ToInt16(0); }

	// ToInt32 (IConvertible)
	//
	// Converts this binary data into a System.Int32
	virtual int ToInt32(IFormatProvider^ fmt) sealed = IConvertible::ToInt32 { return ToInt32(0); }

	// ToInt64 (IConvertible)
	//
	// Converts this binary data into a System.Int64
	virtual __int64 ToInt64(IFormatProvider^ fmt) sealed = IConvertible::ToInt64 { return ToInt64(0); }

	// ToSByte (IConvertible)
	//
	// Converts this binary data into a System.SByte
	virtual SByte ToSByte(IFormatProvider^ fmt) sealed = IConvertible::ToSByte { return ToSByte(0); }

	// ToSingle (IConvertible)
	//
	// Converts this binary data into a System.Single
	virtual float ToSingle(IFormatProvider^ fmt) sealed = IConvertible::ToSingle { return ToSingle(0); }

	// ToString (IConvertible)
	//
	// Converts this binary data into a System.String.  Always returns a constant
	// "<binary value>".  Use ToBytes() and convert manually as necessary
	virtual String^	ToString(IFormatProvider^ fmt) sealed = IConvertible::ToString { return ToString(); }

	// ToUInt16 (IConvertible)
	//
	// Converts this binary data into a System.UInt16
	virtual unsigned short ToUInt16(IFormatProvider^ fmt) sealed = IConvertible::ToUInt16 { return ToUInt16(0); }

	// ToUInt32 (IConvertible)
	//
	// Converts this binary data into a System.UInt32
	virtual unsigned int ToUInt32(IFormatProvider^ fmt) sealed = IConvertible::ToUInt32 { return ToUInt32(0); }

	// ToUInt64 (IConvertible)
	//
	// Converts this binary data into a System.UInt64
	virtual unsigned __int64 ToUInt64(IFormatProvider^ fmt) sealed = IConvertible::ToUInt64 { return ToUInt64(0); }

	//-----------------------------------------------------------------------
	// Member Variables

	bool						m_disposed;			// Object disposal flag
	StatementHandle*			m_pStatement;		// Contained statement handle
	int							m_ordinal;			// Result set ordinal
	int							m_cb;				// Size of the value data
};

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)

#endif	// __SQLITEBINARYREADER_H_
