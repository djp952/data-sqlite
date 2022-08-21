//---------------------------------------------------------------------------
// Copyright (c) 2008-2021 Michael G. Brehm
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

#include "stdafx.h"					// Include project pre-compiled headers
#include "zDBArgument.h"			// Include zDBArgument declarations

#pragma warning(push, 4)			// Enable maximum compiler warnings
#pragma warning(disable:4100)		// "unreferenced formal parameter"

BEGIN_NAMESPACE(zuki)
BEGIN_NAMESPACE(data)
BEGIN_NAMESPACE(dbms)

//---------------------------------------------------------------------------
// zDBArgument::GetTypeCode
//
// Determines the best matching TypeCode based on the underlying SQLite data
// type of the argument
//
// Arguments:
//
//	NONE

TypeCode zDBArgument::GetTypeCode(void)
{
	CHECK_DISPOSED(m_disposed);

	// Nothing overly fancy, as we have no idea what the declared data
	// type is, we only know what SQLite is handing to us here

	switch(m_type) {

		case SQLITE_INTEGER:	return TypeCode::Int64;
		case SQLITE_FLOAT:		return TypeCode::Double;
		case SQLITE_TEXT:		return TypeCode::String;
		case SQLITE_BLOB:		return TypeCode::Object;
		case SQLITE_NULL:		return TypeCode::DBNull;
		default:				return TypeCode::Object;
	}
}

//---------------------------------------------------------------------------
// zDBArgument::ToBoolean
//
// Converts this argument into a System.Boolean
//
// Arguments:
//
//	NONE

bool zDBArgument::ToBoolean(void)
{
	CHECK_DISPOSED(m_disposed);

	// Booleans can come from pretty much anything, if we allow non-zero
	// numeric values to mean "true".  Don't perform an implicit SQLite
	// data conversion to int, go off of the true data type here

	switch(m_type) {

		case SQLITE_INTEGER:	return (ToInt32() != 0);
		case SQLITE_FLOAT:		return (ToDouble() != 0.0);
		case SQLITE_TEXT:		return Convert::ToBoolean(ToString());
		case SQLITE_BLOB:		return (ToByte() != 0);
		default:				throw gcnew InvalidCastException();
	}
}

//---------------------------------------------------------------------------
// zDBArgument::ToByte
//
// Converts this argument into a System.Byte
//
// Arguments:
//
//	NONE

System::Byte zDBArgument::ToByte(void)
{
	CHECK_DISPOSED(m_disposed);
	if(m_type == SQLITE_NULL) throw gcnew InvalidCastException();
	if(m_length < sizeof(unsigned char)) throw gcnew InvalidCastException();

	return *reinterpret_cast<const unsigned char*>(sqlite3_value_blob(m_value));
}

//---------------------------------------------------------------------------
// zDBArgument::ToBytes
//
// Converts this argument into a System.Byte[]
//
// Arguments:
//
//	NONE

array<System::Byte>^ zDBArgument::ToBytes(void)
{
	array<System::Byte>^	bytes;			// Resultant array
	PinnedBytePtr			pinBytes;		// Pinned array pointer

	CHECK_DISPOSED(m_disposed);
	if(m_type == SQLITE_NULL) throw gcnew InvalidCastException();

	// Allocate the new managed array, and if we're not going to
	// copy any data into it, we're done

	bytes = gcnew array<System::Byte>(m_length);
	if(m_length == 0) return bytes;

	// Pin down the managed array in memory so we can blast the data
	// in there directly from the SQLite data buffer

	pinBytes = &bytes[0];
	memcpy_s(pinBytes, m_length, sqlite3_value_blob(m_value), m_length);

	return bytes;							// Return resultant array
}

//---------------------------------------------------------------------------
// zDBArgument::ToChar
//
// Converts this argument into a System.Char
//
// Arguments:
//
//	NONE

__wchar_t zDBArgument::ToChar(void)
{
	CHECK_DISPOSED(m_disposed);
	if(m_type == SQLITE_NULL) throw gcnew InvalidCastException();
	if(m_length < sizeof(wchar_t)) throw gcnew InvalidCastException();

	return *reinterpret_cast<const wchar_t*>(sqlite3_value_blob(m_value));
}

//---------------------------------------------------------------------------
// zDBArgument::ToChars
//
// Converts this argument into a System.Char[]
//
// Arguments:
//
//	NONE

array<__wchar_t>^ zDBArgument::ToChars(void)
{
	array<__wchar_t>^		chars;			// Resultant array
	PinnedCharPtr			pinChars;		// Pinned array pointer
	int						length;			// Unicode value length
	int						cch;			// Character count

	CHECK_DISPOSED(m_disposed);
	if(m_type == SQLITE_NULL) throw gcnew InvalidCastException();

	// We can't use the contained length correctly when casting into
	// a Unicode string.  Use the specialized version instead.  Note
	// that this still returns the number of BYTES, not characters

	length = sqlite3_value_bytes16(m_value);
	
	if((length % sizeof(wchar_t)) != 0) throw gcnew InvalidCastException();
	cch = (length / sizeof(wchar_t));

	// Allocate the new managed array, and if we're not going to
	// copy any data into it, we're done

	chars = gcnew array<__wchar_t>(cch);
	if(cch == 0) return chars;

	// Pin down the managed array in memory so we can blast the data
	// in there directly from the SQLite data buffer

	pinChars = &chars[0];
	wmemcpy_s(pinChars, cch, reinterpret_cast<const wchar_t*>(sqlite3_value_text16(m_value)), cch);

	return chars;							// Return resultant array
}

//---------------------------------------------------------------------------
// zDBArgument::ToDateTime
//
// Converts this argument into a System.DateTime
//
// Arguments:
//
//	NONE

DateTime zDBArgument::ToDateTime(void)
{
	CHECK_DISPOSED(m_disposed);
	if(m_type == SQLITE_NULL) throw gcnew InvalidCastException();
	
	// SQLITE_INTEGER --> Assume 64bit tick count
	if(m_type == SQLITE_INTEGER) return DateTime(ToInt64());

	// SQLITE_TEXT --> Assume convertible string
	else if(m_type == SQLITE_TEXT) return DateTime::Parse(ToString());

	// SQLITE_BLOB --> Assume 64bit tick count
	else if((m_type == SQLITE_BLOB) && (m_length == sizeof(__int64)))
		return DateTime(BitConverter::ToInt64(ToBytes(), 0));

	else throw gcnew InvalidCastException();
}

//---------------------------------------------------------------------------
// zDBArgument::ToDouble
//
// Converts this argument into a System.Double
//
// Arguments:
//
//	NONE

double zDBArgument::ToDouble(void)
{
	CHECK_DISPOSED(m_disposed);
	if(m_type == SQLITE_NULL) throw gcnew InvalidCastException();

	return sqlite3_value_double(m_value);	// <-- Let SQLite handle conversions
}

//---------------------------------------------------------------------------
// zDBArgument::ToGuid
//
// Converts this argument into a System.Guid
//
// Arguments:
//
//	NONE

Guid zDBArgument::ToGuid(void)
{
	CHECK_DISPOSED(m_disposed);
	if(m_type == SQLITE_NULL) throw gcnew InvalidCastException();
	
	// GUIDs can come from 16 byte BLOBs and strings.  Any problems
	// with those conversions will also throw an InvalidCastException

	if((m_type == SQLITE_BLOB) && (m_length == 16)) return Guid(ToBytes());
	else if(m_type == SQLITE_TEXT) return Guid(ToString());

	else throw gcnew InvalidCastException();
}

//---------------------------------------------------------------------------
// zDBArgument::ToInt32
//
// Converts this argument into a System.Int32
//
// Arguments:
//
//	NONE

int zDBArgument::ToInt32(void)
{
	CHECK_DISPOSED(m_disposed);
	if(m_type == SQLITE_NULL) throw gcnew InvalidCastException();

	return sqlite3_value_int(m_value);		// <-- Let SQLite handle conversions
}

//---------------------------------------------------------------------------
// zDBArgument::ToInt64
//
// Converts this argument into a System.Int64
//
// Arguments:
//
//	NONE

__int64 zDBArgument::ToInt64(void)
{
	CHECK_DISPOSED(m_disposed);
	if(m_type == SQLITE_NULL) throw gcnew InvalidCastException();

	return sqlite3_value_int64(m_value);	// <-- Let SQLite handle conversions
}

//---------------------------------------------------------------------------
// zDBArgument::ToString
//
// Converts this argument into a System.String
//
// Arguments:
//
//	NONE

String^ zDBArgument::ToString(void)
{
	CHECK_DISPOSED(m_disposed);
	if(m_type == SQLITE_NULL) throw gcnew InvalidCastException();
	
	if(m_length == 0) return String::Empty;
	else return gcnew String(reinterpret_cast<const wchar_t*>(sqlite3_value_text16(m_value)));
}

//---------------------------------------------------------------------------
// zDBArgument::ToType (private)
//
// Attempts to coerce the contained binary data into the specified type.  Here
// to appease IConvertible, and does not allow you to specify an offset
//
// Arguments:
//
//	type		- Data type to try and coerce the data into
//	fmt			- Ignored

Object^	zDBArgument::ToType(Type^ type, IFormatProvider^ fmt)
{
	CHECK_DISPOSED(m_disposed);
	if(type == nullptr) throw gcnew ArgumentNullException();

	// Byte[], Char[] and Guid are supported, but do not have corresponding
	// entries in the TypeCode enumeration

	if(type == array<System::Byte>::typeid) return ToBytes();	// Byte[] --> ToBytes
	if(type == array<__wchar_t>::typeid) return ToChars();		// Char[] --> ToChars
	if(type == Guid::typeid) return ToGuid();					// Guid   --> ToGuid

	// Depending on the TypeCode, call into the appropriate conversion.
	// Note that TypeCode::Decimal is not supported by this class

	switch(Type::GetTypeCode(type)) {

		case TypeCode::Boolean:		return ToBoolean();
		case TypeCode::Byte:		return ToByte();
		case TypeCode::Char:		return ToChar();
		case TypeCode::DateTime:	return ToDateTime();
		case TypeCode::DBNull:		return DBNull::Value;
		case TypeCode::Double:		return ToDouble();
		case TypeCode::Empty:		return gcnew array<System::Byte>(0);
		case TypeCode::Int16:		return ToInt16();
		case TypeCode::Int32:		return ToInt32();
		case TypeCode::Int64:		return ToInt64();
		case TypeCode::Object:		return ToBytes();
		case TypeCode::SByte:		return ToSByte();
		case TypeCode::Single:		return ToSingle();
		case TypeCode::String:		return ToString();
		case TypeCode::UInt16:		return ToUInt16();
		case TypeCode::UInt32:		return ToUInt32();
		case TypeCode::UInt64:		return ToUInt64();

		default: throw gcnew InvalidCastException();
	}
}

//---------------------------------------------------------------------------
// zDBArgument::Value::get
//
// Retrieves the value of this argument using the best fit conversion based
// on the data type SQLite maintains for this value
//
// Arguments:
//
//	NONE

Object^ zDBArgument::Value::get(void)
{
	CHECK_DISPOSED(m_disposed);

	switch(m_type) {

		case SQLITE_INTEGER : return ToInt64();
		case SQLITE_FLOAT	: return ToDouble();
		case SQLITE_TEXT	: return ToString();
		case SQLITE_BLOB	: return ToBytes();
		case SQLITE_NULL	: return DBNull::Value;
		default				: throw gcnew InvalidCastException();
	}
}

//---------------------------------------------------------------------------

END_NAMESPACE(dbms)
END_NAMESPACE(data)
END_NAMESPACE(zuki)

#pragma warning(pop)
