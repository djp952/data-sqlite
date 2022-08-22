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

#include "stdafx.h"					// Include project pre-compiled headers
#include "zDBResult.h"				// Include zDBResult declarations
#include "zDBConnection.h"			// Include zDBConnection declarations

#pragma warning(push, 4)			// Enable maximum compiler warnings

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// zDBResult::SetArgument
//
// Sets the result of this function to be an exact copy of an existing
// zDBArgument object.  Does not really apply to how this class is used
// with virtual tables, but since the application cannot create zDBArguments,
// it's not really a problem, just a useless function
//
// Arguments:
//
//	value		- Value to be set as the result of this function

void zDBResult::SetArgument(zDBArgument^ value)
{
	CHECK_DISPOSED(m_disposed);
	if(value->IsDisposed()) throw gcnew ObjectDisposedException("value");

	sqlite3_result_value(m_context, value->Handle);
}

//---------------------------------------------------------------------------
// zDBResult::SetBinaryStream
//
// Sets the result of this function to be a special zDBBinaryStream object,
// which is the most efficient way of returning large blobs from functions
//
// Arguments:
//
//	value		- The zDBBinaryStream to use as the function result

void zDBResult::SetBinaryStream(zDBBinaryStream^ value)
{
	int					cbData;				// Length of the value data
	void*				pvData;				// Pointer to the value data

	CHECK_DISPOSED(m_disposed);
	if(!value->CanRead) throw gcnew InvalidOperationException();

	cbData = value->Lock(&pvData);			// Lock down the object

	// Thanks to the handy-dandy zDBBinaryStream class, we can perform a
	// more efficient callback here that allows SQLite to keep the buffer
	// around as long as it needs to and access it directly, instead of
	// having to make a private copy of the data like with SetBytes()

	sqlite3_result_blob(m_context, pvData, cbData, zDBBinaryStream::OnRelease);
}

//---------------------------------------------------------------------------
// zDBResult::SetBoolean
//
// Sets the result of this function to be a boolean value, using a default
// format.  If a connection object has been referenced, that format is used,
// otherwise the provider default is used.
//
// Arguments:
//
//	value		- Value to be set as the result of this function

void zDBResult::SetBoolean(bool value)
{
	CHECK_DISPOSED(m_disposed);
	return SetBoolean(value, (m_conn != nullptr) ? m_conn->BooleanFormat : zDBBooleanFormat::OneZero);
}

//---------------------------------------------------------------------------
// zDBResult::SetBoolean
//
// Sets the result of this function to be a boolean value, using the specified
// formatting since SQLite does not have a native boolean type.
//
// Arguments:
//
//	value		- Value to be set as the result of this function
//	fmt			- Formatting option

void zDBResult::SetBoolean(bool value, zDBBooleanFormat fmt)
{
	// We don't even need to check if the class is disposed of here, since
	// all we're doing is calling into one of the main result functions

	switch(fmt) {
		
		case zDBBooleanFormat::OneZero:			return SetInt32((value) ? 1 : 0);
		case zDBBooleanFormat::NegativeOneZero:	return SetInt32((value) ? -1 : 0);
		case zDBBooleanFormat::TrueFalse:		return SetString((value) ? "true" : "false");

		default: throw gcnew ArgumentException();
	}
}

//---------------------------------------------------------------------------
// zDBResult::SetBytes
//
// Sets the result of this function to be a BLOB based on a byte array
//
// Arguments:
//
//	value		- Value to be set as the result of this function
//	offset		- Offset into the byte array to begin copying data
//	length		- Length of the data to copy from the array

void zDBResult::SetBytes(array<System::Byte>^ value, int offset, int length)
{
	CHECK_DISPOSED(m_disposed);
	if(length < 0) throw gcnew ArgumentOutOfRangeException();
	if((offset < 0) || (offset >= value->Length)) throw gcnew ArgumentOutOfRangeException();
	if((offset + length) > value->Length) throw gcnew ArgumentOutOfRangeException();

	// Note that "offset >= value->Length" guarantees that there is as least
	// one byte to read in here, so we don't need a special case for zero-length

	PinnedBytePtr pinBytes = &value[0];
	sqlite3_result_blob(m_context, &pinBytes[offset], length, SQLITE_TRANSIENT);
}

//---------------------------------------------------------------------------
// zDBResult::SetChar
//
// Sets the result of this function to be a single character value.  This is
// basically the same thing as calling SetString() with a 1-character long
// string, or SetChars() with an array length of one.
//
// Arguments:
//
//	value		- Value to set as the result of this function

void zDBResult::SetChar(__wchar_t value)
{
	CHECK_DISPOSED(m_disposed);
	sqlite3_result_text16(m_context, &value, 1, SQLITE_TRANSIENT);
}

//---------------------------------------------------------------------------
// zDBResult::SetChars
//
// Sets the result of this function to be TEST based on a character array
//
// Arguments:
//
//	value		- Value to be set as the result of this function
//	offset		- Offset into the byte array to begin copying data
//	length		- Length of the data to copy from the array

void zDBResult::SetChars(array<__wchar_t>^ value, int offset, int length)
{
	CHECK_DISPOSED(m_disposed);
	if(length < 0) throw gcnew ArgumentOutOfRangeException();
	if((offset < 0) || (offset >= value->Length)) throw gcnew ArgumentOutOfRangeException();
	if((offset + length) > value->Length) throw gcnew ArgumentOutOfRangeException();

	// Note that "offset >= value->Length" guarantees that there is as least
	// one byte to read in here, so we don't need a special case for zero-length

	PinnedCharPtr pinChars = &value[0];
	sqlite3_result_text16(m_context, &pinChars[offset], length * sizeof(wchar_t), SQLITE_TRANSIENT);
}

//---------------------------------------------------------------------------
// zDBResult::SetDateTime
//
// Sets the result of this function to be a date/time value, using a default
// format.  If a connection object has been referenced, that format is used,
// otherwise the provider default is used.
//
// Arguments:
//
//	value		- Value to be set as the result of this function

void zDBResult::SetDateTime(DateTime value)
{
	CHECK_DISPOSED(m_disposed);
	return SetDateTime(value, (m_conn != nullptr) ? m_conn->DateTimeFormat : zDBDateTimeFormat::ISO8601);
}

//---------------------------------------------------------------------------
// zDBResult::SetDateTime
//
// Sets the result of this function to be a date/time value, using the specified
// formatting since SQLite does not have a native date/time type.
//
// Arguments:
//
//	value		- Value to be set as the result of this function
//	fmt			- Formatting option

void zDBResult::SetDateTime(DateTime value, zDBDateTimeFormat fmt)
{
	// We don't even need to check if the class is disposed of here, since
	// all we're doing is calling into one of the main result functions

	switch(fmt) {

		case zDBDateTimeFormat::ISO8601:			return SetString(value.ToString("yyyy-mm-dd HH:mm:ss"));
		case zDBDateTimeFormat::RFC1123:			return SetString(value.ToString("r", DateTimeFormatInfo::InvariantInfo));
		case zDBDateTimeFormat::Sortable:			return SetString(value.ToString("s", DateTimeFormatInfo::InvariantInfo));
		case zDBDateTimeFormat::Ticks:				return SetInt64(value.Ticks);
		case zDBDateTimeFormat::UniversalSortable:	return SetString(value.ToString("u", DateTimeFormatInfo::InvariantInfo));
		
		default: throw gcnew ArgumentException();
	}
}

//---------------------------------------------------------------------------
// zDBResult::SetDouble
//
// Sets the result of this function to be a 64bit floating point value
//
// Arguments:
//
//	value		- Value to set as the result of this function

void zDBResult::SetDouble(double value)
{
	CHECK_DISPOSED(m_disposed);
	sqlite3_result_double(m_context, value);
}

//---------------------------------------------------------------------------
// zDBResult::SetError
//
// Sets the result of this function to be an error message.  Implies that an
// error state should be set, otherwise use SetString()
//
// Arguments:
//
//	value		- Value to set as the result of this function

void zDBResult::SetError(String^ value)
{
	CHECK_DISPOSED(m_disposed);

	// It's undoubtedly more efficient to just let SQLite allocate a copy
	// of this string (SQLITE_TRANSIENT) than for us to use the Marshal class,
	// which I've determined is beneath contempt for most operations

	PinnedStringPtr	pinValue = PtrToStringChars(value);
	sqlite3_result_error16(m_context, pinValue, -1);
}

//---------------------------------------------------------------------------
// zDBResult::SetGuid
//
// Sets the result of this function to be a GUID value, using a default
// format.  If a connection object has been referenced, that format is used,
// otherwise the provider default is used.
//
// Arguments:
//
//	value		- Value to be set as the result of this function

void zDBResult::SetGuid(Guid value)
{
	CHECK_DISPOSED(m_disposed);
	return SetGuid(value, (m_conn != nullptr) ? m_conn->GuidFormat : zDBGuidFormat::Binary);
}

//---------------------------------------------------------------------------
// zDBResult::SetGuid
//
// Sets the result of this function to be a GUID value, using the specified
// formatting since SQLite does not have a native GUID type.
//
// Arguments:
//
//	value		- Value to be set as the result of this function
//	fmt			- Formatting option

void zDBResult::SetGuid(Guid value, zDBGuidFormat fmt)
{
	// We don't even need to check if the class is disposed of here, since
	// all we're doing is calling into one of the main result functions

	switch(fmt) {
		
		case zDBGuidFormat::Binary:			return SetBytes(value.ToByteArray());
		case zDBGuidFormat::Bracketed:		return SetString(value.ToString("B"));
		case zDBGuidFormat::HexString:		return SetString(value.ToString("N"));
		case zDBGuidFormat::Hyphenated:		return SetString(value.ToString("D"));
		case zDBGuidFormat::Parenthetic:	return SetString(value.ToString("P"));

		default: throw gcnew ArgumentException();
	}
}

//---------------------------------------------------------------------------
// zDBResult::SetInt32
//
// Sets the result of this function to be a 32bit integer value
//
// Arguments:
//
//	value		- Value to set as the result of this function

void zDBResult::SetInt32(int value)
{
	CHECK_DISPOSED(m_disposed);
	sqlite3_result_int(m_context, value);
}

//---------------------------------------------------------------------------
// zDBResult::SetInt64
//
// Sets the result of this function to be a 64bit integer value
//
// Arguments:
//
//	value		- Value to set as the result of this function

void zDBResult::SetInt64(__int64 value)
{
	CHECK_DISPOSED(m_disposed);
	sqlite3_result_int64(m_context, value);
}

//---------------------------------------------------------------------------
// zDBResult::SetNull
//
// Sets the result of this function to be NULL
//
// Arguments:
//
//	NONE

void zDBResult::SetNull(void)
{
	CHECK_DISPOSED(m_disposed);
	sqlite3_result_null(m_context);
}

//---------------------------------------------------------------------------
// zDBResult::SetString
//
// Sets the result of this function to be a string value
//
// Arguments:
//
//	value		- Value to set as the result of this function

void zDBResult::SetString(String^ value)
{
	CHECK_DISPOSED(m_disposed);

	// It's undoubtedly more efficient to just let SQLite allocate a copy
	// of this string (SQLITE_TRANSIENT) than for us to use the Marshal class,
	// which I've determined is beneath contempt for most operations

	PinnedStringPtr	pinValue = PtrToStringChars(value);
	sqlite3_result_text16(m_context, pinValue, -1, SQLITE_TRANSIENT);
}

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)
