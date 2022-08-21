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
#include "zDBBinaryReader.h"			// Include zDBBinaryReader declarations

#pragma warning(push, 4)			// Enable maximum compiler warnings
#pragma warning(disable:4100)		// "unreferenced formal parameter"

BEGIN_NAMESPACE(zuki)
BEGIN_NAMESPACE(data)
BEGIN_NAMESPACE(dbms)

//---------------------------------------------------------------------------
// zDBBinaryReader Constructor (internal)
//
// Arguments:
//
//	pStatement		- Pointer to the parent statement handle
//	ordinal			- Ordinal value of this binary value result

zDBBinaryReader::zDBBinaryReader(StatementHandle* pStatement, int ordinal) :
	m_pStatement(pStatement), m_ordinal(ordinal)
{
	if(!m_pStatement) throw gcnew ArgumentNullException();
	if(ordinal < 0) throw gcnew ArgumentOutOfRangeException();

	// Get how much data there is to work with now, so we don't have to
	// do it each and every time.  This is supposed to be an OPTIMIZED
	// way of working with binary values, after all.

	m_cb = sqlite3_column_bytes(m_pStatement->Handle, m_ordinal);
	m_pStatement->AddRef(this);
}

//---------------------------------------------------------------------------
// zDBBinaryReader Finalizer

zDBBinaryReader::!zDBBinaryReader()
{
	if(m_pStatement) m_pStatement->Release(this);	// Release statement handle
	m_pStatement = NULL;							// Reset pointer to NULL
}

//---------------------------------------------------------------------------
// zDBBinaryReader::GetTypeCode (private)
//
// Gets a TypeCode for this object -- not supported, at least not well
//
// Arguments:
//
//	NONE

TypeCode zDBBinaryReader::GetTypeCode(void)
{
	CHECK_DISPOSED(m_disposed);
	return TypeCode::Object;
}

//---------------------------------------------------------------------------
// zDBBinaryReader::Length::get
//
// Returns the length of the underlying binary data

int zDBBinaryReader::Length::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return m_cb;
}

//---------------------------------------------------------------------------
// zDBBinaryReader::ToBoolean
//
// Converts the binary value into a System.Boolean.  Significantly more optimized
// in that it works directly with a single byte of data from the source buffer
//
// Arguments:
//
//	offset		- Index into binary data from which to convert

bool zDBBinaryReader::ToBoolean(int offset)
{
	CHECK_DISPOSED(m_disposed);
	return ToByte(offset) != 0;				// Use ToByte() instead
}

//---------------------------------------------------------------------------
// zDBBinaryReader::ToByte
//
// Converts the binary value into a System.Byte.  Significantly more optimized
// in that it works directly with a single byte of data from the source buffer
//
// Arguments:
//
//	offset		- Index into binary data from which to convert

System::Byte zDBBinaryReader::ToByte(int offset)
{
	const unsigned char*		pu;			// Pointer into the data

	CHECK_DISPOSED(m_disposed);
	if(offset < 0) throw gcnew ArgumentException();
	if(offset > (m_cb - 1)) throw gcnew ArgumentOutOfRangeException();

	pu = reinterpret_cast<const unsigned char*>(sqlite3_column_blob(m_pStatement->Handle, m_ordinal));
	return *(pu + offset);
}

//---------------------------------------------------------------------------
// zDBBinaryReader::ToBytes
//
// Returns a byte array comprised of the entire underlying BLOB data
//
// Arguments:
//
//	NONE

array<System::Byte>^ zDBBinaryReader::ToBytes(void)
{
	CHECK_DISPOSED(m_disposed);
	return ToBytes(0, m_cb);
}

//---------------------------------------------------------------------------
// zDBBinaryReader::ToBytes
//
// Returns a byte array from a starting offset to the end of the data
//
// Arguments:
//
//	offset		- Offset into the binary data to start from

array<System::Byte>^ zDBBinaryReader::ToBytes(int offset)
{
	CHECK_DISPOSED(m_disposed);
	return ToBytes(offset, m_cb - offset);
}

//---------------------------------------------------------------------------
// zDBBinaryReader::ToBytes
//
// Returns a specific portion of the binary data as a byte array
//
// Arguments:
//
//	offset		- Offset into the binary data to start from
//	length		- Length of the binary data to return

array<System::Byte>^ zDBBinaryReader::ToBytes(int offset, int length)
{
	const unsigned char*		puData;			// Pointer into raw data
	array<System::Byte>^		rg;				// Managed byte array of data
	PinnedBytePtr				pinRg;			// Pinned pointer into rg[]

	CHECK_DISPOSED(m_disposed);
	if(offset < 0) throw gcnew ArgumentException();
	if(length < 0) throw gcnew ArgumentException();
	if((offset + length) > m_cb) throw gcnew ArgumentOutOfRangeException();

	// Special case: If the caller wants zero bytes, don't go pinning
	// pointers and copying nothing.  Just return what they want

	if(length == 0) return gcnew array<System::Byte>(0);

	rg = gcnew array<System::Byte>(length);		// Create return array
	pinRg = &rg[0];								// Pin and get a pointer

	// Only copy the amount of data that the caller is looking for

	puData = reinterpret_cast<const unsigned char*>(sqlite3_column_blob(m_pStatement->Handle, m_ordinal));
	memcpy_s(pinRg, length, puData + offset, length);
	return rg;
}

//---------------------------------------------------------------------------
// zDBBinaryReader::ToChar
//
// Converts the binary value into a System.Char.  Significantly more optimized
// in that it only allocates and copies exactly the amount of data necessary
// from the SQLite data buffer into the managed target buffer.
//
// Arguments:
//
//	offset		- Index into binary data from which to convert

__wchar_t zDBBinaryReader::ToChar(int offset)
{
	int						cb = sizeof(__wchar_t);	// Number of bytes to alloc/copy
	const unsigned char*	puData;					// Pointer into raw data
	array<System::Byte>^	rg;						// Managed byte array of data
	PinnedBytePtr			pinRg;					// Pinned pointer into rg[]

	CHECK_DISPOSED(m_disposed);
	if(offset < 0) throw gcnew ArgumentException();
	if(offset > (m_cb - cb)) throw gcnew ArgumentOutOfRangeException();

	rg = gcnew array<System::Byte>(cb);		// Allocate the local buffer
	pinRg = &rg[0];							// Pin it down for memcpy_s

	// Copy only the amount of data we specifically need from SQLite, and attempt
	// to convert it using a standard binary conversion mechanism

	puData = reinterpret_cast<const unsigned char*>(sqlite3_column_blob(m_pStatement->Handle, m_ordinal));
	memcpy_s(pinRg, cb, puData + offset, cb);
	return BitConverter::ToChar(rg, 0);
}

//---------------------------------------------------------------------------
// zDBBinaryReader::ToChars
//
// Returns a char array comprised of the entire underlying BLOB data
//
// Arguments:
//
//	NONE

array<__wchar_t>^ zDBBinaryReader::ToChars(void)
{
	CHECK_DISPOSED(m_disposed);
	return ToChars(0, m_cb);
}

//---------------------------------------------------------------------------
// zDBBinaryReader::ToChars
//
// Returns a char array from a starting offset to the end of the data
//
// Arguments:
//
//	offset		- Offset into the binary data to start from

array<__wchar_t>^ zDBBinaryReader::ToChars(int offset)
{
	CHECK_DISPOSED(m_disposed);
	return ToChars(offset, m_cb - offset);
}

//---------------------------------------------------------------------------
// zDBBinaryReader::ToChars
//
// Returns a specific portion of the binary data as a char array
//
// Arguments:
//
//	offset		- Offset into the binary data to start from
//	length		- Length of the binary data to return

array<__wchar_t>^ zDBBinaryReader::ToChars(int offset, int length)
{
	int							cb;				// Local size information
	const wchar_t*				pwchData;		// Pointer into raw data
	array<__wchar_t>^			rg;				// Managed char array of data
	PinnedCharPtr				pinRg;			// Pinned pointer into rg[]
	int							cch;			// Unicode character count

	CHECK_DISPOSED(m_disposed);
	if(offset < 0) throw gcnew ArgumentException();
	if(length < 0) throw gcnew ArgumentException();
	if((offset % sizeof(wchar_t)) != 0) throw gcnew ArgumentException();
	if((length % sizeof(wchar_t)) != 0) throw gcnew ArgumentException();

	// When accessing things as Unicode character data, ask SQLite for a
	// specific size that will properly adjusted as necessary

	cb = sqlite3_column_bytes16(m_pStatement->Handle, m_ordinal);
	if((offset + length) > cb) throw gcnew ArgumentOutOfRangeException();

	// Special case: If the caller wants zero bytes, don't go pinning
	// pointers and copying nothing.  Just return what they want

	if(length == 0) return gcnew array<__wchar_t>(0);

	cch = length / sizeof(wchar_t);				// Calculate character count
	rg = gcnew array<__wchar_t>(cch);			// Create return array
	pinRg = &rg[0];								// Pin and get a pointer

	// Only copy the amount of data that the caller is looking for from
	// the SQLite buffer into the managed Char array buffer

	pwchData = reinterpret_cast<const wchar_t*>(sqlite3_column_text16(m_pStatement->Handle, m_ordinal));
	wmemcpy_s(pinRg, cch, pwchData + (offset / sizeof(wchar_t)), cch);
	return rg;
}

//---------------------------------------------------------------------------
// zDBBinaryReader::ToDateTime
//
// Converts the binary value into a System.DateTime.  Significantly more optimized
// in that it only allocates and copies exactly the amount of data necessary
// from the SQLite data buffer into the managed target buffer.
//
// Arguments:
//
//	offset		- Index into binary data from which to convert

DateTime zDBBinaryReader::ToDateTime(int offset)
{
	CHECK_DISPOSED(m_disposed);
	return DateTime(ToInt64(offset));		// Use ToInt64() instead
}

//---------------------------------------------------------------------------
// zDBBinaryReader::ToDouble
//
// Converts the binary value into a System.Double.  Significantly more optimized
// in that it only allocates and copies exactly the amount of data necessary
// from the SQLite data buffer into the managed target buffer.
//
// Arguments:
//
//	offset		- Index into binary data from which to convert

double zDBBinaryReader::ToDouble(int offset)
{
	int						cb = sizeof(double);	// Number of bytes to alloc/copy
	const unsigned char*	puData;					// Pointer into raw data
	array<System::Byte>^	rg;						// Managed byte array of data
	PinnedBytePtr			pinRg;					// Pinned pointer into rg[]

	CHECK_DISPOSED(m_disposed);
	if(offset < 0) throw gcnew ArgumentException();
	if(offset > (m_cb - cb)) throw gcnew ArgumentOutOfRangeException();

	rg = gcnew array<System::Byte>(cb);		// Allocate the local buffer
	pinRg = &rg[0];							// Pin it down for memcpy_s

	// Copy only the amount of data we specifically need from SQLite, and attempt
	// to convert it using a standard binary conversion mechanism

	puData = reinterpret_cast<const unsigned char*>(sqlite3_column_blob(m_pStatement->Handle, m_ordinal));
	memcpy_s(pinRg, cb, puData + offset, cb);
	return BitConverter::ToDouble(rg, 0);
}

//---------------------------------------------------------------------------
// zDBBinaryReader::ToGuid
//
// Converts the binary value into a System.Guid.  Significantly more optimized
// in that it only allocates and copies exactly the amount of data necessary
// from the SQLite data buffer into the managed target buffer.
//
// Arguments:
//
//	offset		- Index into binary data from which to convert

Guid zDBBinaryReader::ToGuid(int offset)
{
	CHECK_DISPOSED(m_disposed);
	return Guid(ToBytes(0, 16));
}

//---------------------------------------------------------------------------
// zDBBinaryReader::ToInt16
//
// Converts the binary value into a System.Int16.  Significantly more optimized
// in that it only allocates and copies exactly the amount of data necessary
// from the SQLite data buffer into the managed target buffer.
//
// Arguments:
//
//	offset		- Index into binary data from which to convert

short zDBBinaryReader::ToInt16(int offset)
{
	int						cb = sizeof(short);		// Number of bytes to alloc/copy
	const unsigned char*	puData;					// Pointer into raw data
	array<System::Byte>^	rg;						// Managed byte array of data
	PinnedBytePtr			pinRg;					// Pinned pointer into rg[]

	CHECK_DISPOSED(m_disposed);
	if(offset < 0) throw gcnew ArgumentException();
	if(offset > (m_cb - cb)) throw gcnew ArgumentOutOfRangeException();

	rg = gcnew array<System::Byte>(cb);		// Allocate the local buffer
	pinRg = &rg[0];							// Pin it down for memcpy_s

	// Copy only the amount of data we specifically need from SQLite, and attempt
	// to convert it using a standard binary conversion mechanism

	puData = reinterpret_cast<const unsigned char*>(sqlite3_column_blob(m_pStatement->Handle, m_ordinal));
	memcpy_s(pinRg, cb, puData + offset, cb);
	return BitConverter::ToInt16(rg, 0);
}

//---------------------------------------------------------------------------
// zDBBinaryReader::ToInt32
//
// Converts the binary value into a System.Int32.  Significantly more optimized
// in that it only allocates and copies exactly the amount of data necessary
// from the SQLite data buffer into the managed target buffer.
//
// Arguments:
//
//	offset		- Index into binary data from which to convert

int zDBBinaryReader::ToInt32(int offset)
{
	int						cb = sizeof(int);		// Number of bytes to alloc/copy
	const unsigned char*	puData;					// Pointer into raw data
	array<System::Byte>^	rg;						// Managed byte array of data
	PinnedBytePtr			pinRg;					// Pinned pointer into rg[]

	CHECK_DISPOSED(m_disposed);
	if(offset < 0) throw gcnew ArgumentException();
	if(offset > (m_cb - cb)) throw gcnew ArgumentOutOfRangeException();

	rg = gcnew array<System::Byte>(cb);		// Allocate the local buffer
	pinRg = &rg[0];							// Pin it down for memcpy_s

	// Copy only the amount of data we specifically need from SQLite, and attempt
	// to convert it using a standard binary conversion mechanism

	puData = reinterpret_cast<const unsigned char*>(sqlite3_column_blob(m_pStatement->Handle, m_ordinal));
	memcpy_s(pinRg, cb, puData + offset, cb);
	return BitConverter::ToInt32(rg, 0);
}

//---------------------------------------------------------------------------
// zDBBinaryReader::ToInt64
//
// Converts the binary value into a System.Int64.  Significantly more optimized
// in that it only allocates and copies exactly the amount of data necessary
// from the SQLite data buffer into the managed target buffer.
//
// Arguments:
//
//	offset		- Index into binary data from which to convert

__int64 zDBBinaryReader::ToInt64(int offset)
{
	int						cb = sizeof(__int64);	// Number of bytes to alloc/copy
	const unsigned char*	puData;					// Pointer into raw data
	array<System::Byte>^	rg;						// Managed byte array of data
	PinnedBytePtr			pinRg;					// Pinned pointer into rg[]

	CHECK_DISPOSED(m_disposed);
	if(offset < 0) throw gcnew ArgumentException();
	if(offset > (m_cb - cb)) throw gcnew ArgumentOutOfRangeException();

	rg = gcnew array<System::Byte>(cb);		// Allocate the local buffer
	pinRg = &rg[0];							// Pin it down for memcpy_s

	// Copy only the amount of data we specifically need from SQLite, and attempt
	// to convert it using a standard binary conversion mechanism

	puData = reinterpret_cast<const unsigned char*>(sqlite3_column_blob(m_pStatement->Handle, m_ordinal));
	memcpy_s(pinRg, cb, puData + offset, cb);
	return BitConverter::ToInt64(rg, 0);
}

//---------------------------------------------------------------------------
// zDBBinaryReader::ToSByte
//
// Converts the binary value into a System.SByte.  Significantly more optimized
// in that it works directly with a single byte of data from the source buffer
//
// Arguments:
//
//	offset		- Index into binary data from which to convert

SByte zDBBinaryReader::ToSByte(int offset)
{
	const char*					pu;			// Pointer into the data

	CHECK_DISPOSED(m_disposed);
	if(offset < 0) throw gcnew ArgumentException();
	if(offset > (m_cb - 1)) throw gcnew ArgumentOutOfRangeException();

	pu = reinterpret_cast<const char*>(sqlite3_column_blob(m_pStatement->Handle, m_ordinal));
	return *(pu + offset);
}

//---------------------------------------------------------------------------
// zDBBinaryReader::ToSingle
//
// Converts the binary value into a System.Single.  Significantly more optimized
// in that it only allocates and copies exactly the amount of data necessary
// from the SQLite data buffer into the managed target buffer.
//
// Arguments:
//
//	offset		- Index into binary data from which to convert

float zDBBinaryReader::ToSingle(int offset)
{
	int						cb = sizeof(float);		// Number of bytes to alloc/copy
	const unsigned char*	puData;					// Pointer into raw data
	array<System::Byte>^	rg;						// Managed byte array of data
	PinnedBytePtr			pinRg;					// Pinned pointer into rg[]

	CHECK_DISPOSED(m_disposed);
	if(offset < 0) throw gcnew ArgumentException();
	if(offset > (m_cb - cb)) throw gcnew ArgumentOutOfRangeException();

	rg = gcnew array<System::Byte>(cb);		// Allocate the local buffer
	pinRg = &rg[0];							// Pin it down for memcpy_s

	// Copy only the amount of data we specifically need from SQLite, and attempt
	// to convert it using a standard binary conversion mechanism

	puData = reinterpret_cast<const unsigned char*>(sqlite3_column_blob(m_pStatement->Handle, m_ordinal));
	memcpy_s(pinRg, cb, puData + offset, cb);
	return BitConverter::ToSingle(rg, 0);
}

//---------------------------------------------------------------------------
// zDBBinaryReader::ToString
//
// Overrides the standard ToString() implementation.  In this case we return
// a constant string that will appear in DataGrids and such things when a
// data adapter is used against the parent statement.  IConvertible::ToString
// behaves differently and tries to construct an actual string from the data
//
// Arguments:
//
//	NONE

String^ zDBBinaryReader::ToString(void)
{
	CHECK_DISPOSED(m_disposed);
	return String::Format(BINARY_DATA_STRING, m_cb);;
}

//---------------------------------------------------------------------------
// zDBBinaryReader::ToType (private)
//
// Attempts to coerce the contained binary data into the specified type.  Here
// to appease IConvertible, and does not allow you to specify an offset
//
// Arguments:
//
//	type		- Data type to try and coerce the data into
//	fmt			- Ignored

Object^	zDBBinaryReader::ToType(Type^ type, IFormatProvider^ fmt)
{
	CHECK_DISPOSED(m_disposed);
	if(type == nullptr) throw gcnew ArgumentNullException();

	// Byte[], Char[] and Guid are supported, but do not have corresponding
	// entries in the TypeCode enumeration

	if(type == array<System::Byte>::typeid) return ToBytes();		// Byte[] --> ToBytes
	if(type == array<__wchar_t>::typeid) return ToChars();			// Char[] --> ToChars
	if(type == Guid::typeid) return ToGuid();						// Guid   --> ToGuid

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
// zDBBinaryReader::ToUInt16
//
// Converts the binary value into a System.UInt16.  Significantly more optimized
// in that it only allocates and copies exactly the amount of data necessary
// from the SQLite data buffer into the managed target buffer.
//
// Arguments:
//
//	offset		- Index into binary data from which to convert

unsigned short zDBBinaryReader::ToUInt16(int offset)
{
	int						cb = sizeof(unsigned short);	// Number of bytes to alloc/copy
	const unsigned char*	puData;							// Pointer into raw data
	array<System::Byte>^	rg;								// Managed byte array of data
	PinnedBytePtr			pinRg;							// Pinned pointer into rg[]

	CHECK_DISPOSED(m_disposed);
	if(offset < 0) throw gcnew ArgumentException();
	if(offset > (m_cb - cb)) throw gcnew ArgumentOutOfRangeException();

	rg = gcnew array<System::Byte>(cb);		// Allocate the local buffer
	pinRg = &rg[0];							// Pin it down for memcpy_s

	// Copy only the amount of data we specifically need from SQLite, and attempt
	// to convert it using a standard binary conversion mechanism

	puData = reinterpret_cast<const unsigned char*>(sqlite3_column_blob(m_pStatement->Handle, m_ordinal));
	memcpy_s(pinRg, cb, puData + offset, cb);
	return BitConverter::ToUInt16(rg, 0);
}

//---------------------------------------------------------------------------
// zDBBinaryReader::ToUInt32
//
// Converts the binary value into a System.UInt32.  Significantly more optimized
// in that it only allocates and copies exactly the amount of data necessary
// from the SQLite data buffer into the managed target buffer.
//
// Arguments:
//
//	offset		- Index into binary data from which to convert

unsigned int zDBBinaryReader::ToUInt32(int offset)
{
	int						cb = sizeof(unsigned int);		// Number of bytes to alloc/copy
	const unsigned char*	puData;							// Pointer into raw data
	array<System::Byte>^	rg;								// Managed byte array of data
	PinnedBytePtr			pinRg;							// Pinned pointer into rg[]

	CHECK_DISPOSED(m_disposed);
	if(offset < 0) throw gcnew ArgumentException();
	if(offset > (m_cb - cb)) throw gcnew ArgumentOutOfRangeException();

	rg = gcnew array<System::Byte>(cb);		// Allocate the local buffer
	pinRg = &rg[0];							// Pin it down for memcpy_s

	// Copy only the amount of data we specifically need from SQLite, and attempt
	// to convert it using a standard binary conversion mechanism

	puData = reinterpret_cast<const unsigned char*>(sqlite3_column_blob(m_pStatement->Handle, m_ordinal));
	memcpy_s(pinRg, cb, puData + offset, cb);
	return BitConverter::ToUInt32(rg, 0);
}

//---------------------------------------------------------------------------
// zDBBinaryReader::ToUInt64
//
// Converts the binary value into a System.UInt64.  Significantly more optimized
// in that it only allocates and copies exactly the amount of data necessary
// from the SQLite data buffer into the managed target buffer.
//
// Arguments:
//
//	offset		- Index into binary data from which to convert

unsigned __int64 zDBBinaryReader::ToUInt64(int offset)
{
	int						cb = sizeof(unsigned __int64);	// Number of bytes to alloc/copy
	const unsigned char*	puData;							// Pointer into raw data
	array<System::Byte>^	rg;								// Managed byte array of data
	PinnedBytePtr			pinRg;							// Pinned pointer into rg[]

	CHECK_DISPOSED(m_disposed);
	if(offset < 0) throw gcnew ArgumentException();
	if(offset > (m_cb - cb)) throw gcnew ArgumentOutOfRangeException();

	rg = gcnew array<System::Byte>(cb);		// Allocate the local buffer
	pinRg = &rg[0];							// Pin it down for memcpy_s

	// Copy only the amount of data we specifically need from SQLite, and attempt
	// to convert it using a standard binary conversion mechanism

	puData = reinterpret_cast<const unsigned char*>(sqlite3_column_blob(m_pStatement->Handle, m_ordinal));
	memcpy_s(pinRg, cb, puData + offset, cb);
	return BitConverter::ToUInt64(rg, 0);
}

//---------------------------------------------------------------------------

END_NAMESPACE(dbms)
END_NAMESPACE(data)
END_NAMESPACE(zuki)

#pragma warning(pop)
