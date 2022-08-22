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
#include "zDBStatement.h"			// Include zDBStatement declarations
#include "zDBConnection.h"			// Include zDBConnection declarations

#pragma warning(push, 4)			// Enable maximum compiler warnings

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// zDBStatement Constructor
//
// Arguments:
//
//	pStatement		- Pointer to the StatementHandle object to use
//	sql				- Original SQL command text used for this statement

zDBStatement::zDBStatement(StatementHandle *pStatement, String^ sql) : 
	m_pStatement(pStatement), m_sql((sql != nullptr) ? sql : String::Empty), 
	m_status(zDBStatementStatus::Prepared)
{
	if(!pStatement) throw gcnew ArgumentNullException();	// Cannot be NULL

	m_metadata = gcnew zDBStatementMetaData(m_pStatement);	// Create metadata
	m_pins = gcnew List<GCHandle>();						// Create List<T>
	m_binaries = gcnew List<ITrackableObject^>();			// Create List<T>

	m_pStatement->AddRef(this);				// We're keeping this object locally
}

//---------------------------------------------------------------------------
// zDBStatement Destructor

zDBStatement::~zDBStatement()
{
	Reset();			// <--- IMPORTANT: Reset the statement on disposal

	if(m_metadata != nullptr) delete m_metadata;	// Destroy metadata
	m_metadata = nullptr;

	this->!zDBStatement();			// Invoke the finalizer
	m_disposed = true;				// Object is now disposed of
}

//---------------------------------------------------------------------------
// zDBStatement Finalizer

zDBStatement::!zDBStatement()
{
	if(m_pStatement) m_pStatement->Release(this);	// Release statement handle
	m_pStatement = NULL;							// Reset pointer to NULL
}

//---------------------------------------------------------------------------
// zDBStatement::BindBinaryParameter (private)
//
// Binds a binary value to the current query statement
//
// Arguments:
//
//	param			- zDBParameter object (for reference only)
//	index			- SQLite parameter index
//	value			- Binary value to be bound
//	length			- Length to use when binding

void zDBStatement::BindBinaryParameter(zDBParameter^ param, int index, 
	array<System::Byte>^ value, int length)
{
	GCHandle				gcHandle;		// Pinning GCHandle structure
	PinnedBytePtr			pinValue;		// Locally pinned object pointer
	int						nResult;		// Result from function call

	// If the call didn't come with a specific size in mind, or it's larger than
	// the data in the buffer, reset length to just use the buffer contents

	if((length == 0) || (length > value->Length)) length = value->Length;

	// The Byte[] object gets pinned twice.  Once by GCHandle::Alloc, and once
	// by pin_ptr<>.  The GCHandle is the one that gets held onto until Reset(), 
	// and the pin_ptr<> goes away when we return from this function

	m_pins->Add(GCHandle::Alloc(value, GCHandleType::Pinned));
	pinValue = (length) ? &value[0] : nullptr;

	// Attempt to bind the parameter to the statement, using SQLITE_STATIC since it's 
	// pinned and avoid a potentially costly memory allocation here

	nResult = sqlite3_bind_blob(m_pStatement->Handle, index, pinValue, length, SQLITE_STATIC);
	if(nResult != SQLITE_OK) throw gcnew zDBExceptions::ParameterBindingException(param, 
		index, m_pStatement->DBHandle, nResult);
}

//---------------------------------------------------------------------------
// zDBStatement::BindBooleanParameter (private)
//
// Binds a boolean value to the curent query statement
//
// Arguments:
//
//	param			- zDBParameter object (for reference only)
//	index			- SQLite parameter index
//	value			- Boolean value to be bound
//	format			- Connection-specific boolean format flag

void zDBStatement::BindBooleanParameter(zDBParameter^ param, int index, 
	bool value, zDBBooleanFormat format)
{
	Object^				formatted;			// Formatted boolean value
	Type^				type;				// Formatted value data type

	// SQLite doesn't natively support a boolean data type, so we format
	// the value based on the provided flag and invoke another binder here

	formatted = FormatBoolean(value, format);
	type = formatted->GetType();

	// BOOLEAN->INTEGER
	if(type == int::typeid) 
		BindIntegerParameter(param, index, static_cast<int>(formatted));

	// BOOLEAN->STRING
	else if(type == String::typeid) 
		BindStringParameter(param, index, static_cast<String^>(formatted), 0);

	// UNEXPECTED FORMAT
	else throw gcnew zDBExceptions::ParameterBindingException(param, index, 
		"INTERNAL FAILURE: Invalid data type from boolean coercion");
}

//---------------------------------------------------------------------------
// zDBStatement::BindDateTimeParameter (private)
//
// Binds a date/time value to the current query statement
//
// Arguments:
//
//	param			- zDBParameter object (for reference only)
//	index			- SQLite parameter index
//	value			- DateTime value to be bound
//	format			- Connection-specific date/time format flag

void zDBStatement::BindDateTimeParameter(zDBParameter^ param, int index, 
	DateTime value, zDBDateTimeFormat format)
{
	Object^				formatted;			// Formatted date/time value
	Type^				type;				// Formatted value data type

	// SQLite doesn't natively support a date/time data type, so we format
	// the value based on the provided flag and invoke another binder here

	formatted = FormatDateTime(value, format);
	type = formatted->GetType();

	// DATETIME->LONG
	if(type == __int64::typeid) 
		BindLongParameter(param, index, static_cast<__int64>(formatted));

	// DATETIME->STRING
	else if(type == String::typeid) 
		BindStringParameter(param, index, static_cast<String^>(formatted), 0);

	// UNEXPECTED FORMAT
	else throw gcnew zDBExceptions::ParameterBindingException(param, index, 
		"INTERNAL FAILURE: Invalid data type from date/time coercion");
}

//---------------------------------------------------------------------------
// zDBStatement::BindFloatParameter (private)
//
// Binds a floating point value to the current query statement
//
// Arguments:
//
//	param			- zDBParameter object (for reference only)
//	index			- SQLite parameter index
//	value			- Double value to be bound

void zDBStatement::BindFloatParameter(zDBParameter^ param, int index, double value)
{
	int nResult = sqlite3_bind_double(m_pStatement->Handle, index, value);
	if(nResult != SQLITE_OK) throw gcnew zDBExceptions::ParameterBindingException(param, 
		index, m_pStatement->DBHandle, nResult);
}

//---------------------------------------------------------------------------
// zDBStatement::BindGuidParameter (private)
//
// Binds a GUID value to the current query statement
//
// Arguments:
//
//	param			- zDBParameter object (for reference only)
//	index			- SQLite parameter index
//	value			- Guid value to be bound
//	format			- Connection-specific GUID format flag

void zDBStatement::BindGuidParameter(zDBParameter^ param, int index, 
	Guid value, zDBGuidFormat format)
{
	Object^				formatted;			// Formatted date/time value
	Type^				type;				// Formatted value data type

	// SQLite doesn't natively support a GUID data type, so we format
	// the value based on the provided flag and invoke another binder here

	formatted = FormatGuid(value, format);
	type = formatted->GetType();

	// GUID->BINARY
	if(type == array<System::Byte>::typeid) 
		BindBinaryParameter(param, index, static_cast<array<System::Byte>^>(formatted), 0);

	// GUID->STRING
	else if(type == String::typeid) 
		BindStringParameter(param, index, static_cast<String^>(formatted), 0);

	// UNEXPECTED FORMAT
	else throw gcnew zDBExceptions::ParameterBindingException(param, index, 
		"INTERNAL FAILURE: Invalid data type from GUID coercion");
}

//---------------------------------------------------------------------------
// zDBStatement::BindIntegerParameter (private)
//
// Binds a 32 bit integer value to the current query statement
//
// Arguments:
//
//	param			- zDBParameter object (for reference only)
//	index			- SQLite parameter index
//	value			- Int32 value to be bound

void zDBStatement::BindIntegerParameter(zDBParameter^ param, int index, int value)
{
	int nResult = sqlite3_bind_int(m_pStatement->Handle, index, value);
	if(nResult != SQLITE_OK) throw gcnew zDBExceptions::ParameterBindingException(param, 
		index, m_pStatement->DBHandle, nResult);
}

//---------------------------------------------------------------------------
// zDBStatement::BindLongParameter (private)
//
// Binds a 64 bit integer value to the current query statement
//
// Arguments:
//
//	param			- zDBParameter object (for reference only)
//	index			- SQLite parameter index
//	value			- Int64 value to be bound

void zDBStatement::BindLongParameter(zDBParameter^ param, int index, __int64 value)
{
	int nResult = sqlite3_bind_int64(m_pStatement->Handle, index, value);
	if(nResult != SQLITE_OK) throw gcnew zDBExceptions::ParameterBindingException(param, 
		index, m_pStatement->DBHandle, nResult);
}

//---------------------------------------------------------------------------
// zDBStatement::BindNullParameter (private)
//
// Binds a NULL value to the current query statement
//
// Arguments:
//
//	param			- zDBParameter object (for reference only)
//	index			- SQLite parameter index

void zDBStatement::BindNullParameter(zDBParameter^ param, int index)
{
	int nResult = sqlite3_bind_null(m_pStatement->Handle, index);
	if(nResult != SQLITE_OK) throw gcnew zDBExceptions::ParameterBindingException(param, 
		index, m_pStatement->DBHandle, nResult);
}

//---------------------------------------------------------------------------
// zDBStatement::BindStringParameter (private)
//
// Binds a string value to the current query statement
//
// Arguments:
//
//	param			- zDBParameter object (for reference only)
//	index			- SQLite parameter index
//	value			- String value to be bound
//	length			- Length to use when binding

void zDBStatement::BindStringParameter(zDBParameter^ param, int index, 
	String^ value, int length)
{
	GCHandle				gcHandle;		// Pinning GCHandle structure
	PinnedStringPtr			pinValue;		// Locally pinned object pointer
	int						nResult;		// Result from function call

	// If the call didn't come with a specific size in mind, or it's larger than
	// the data in the string, reset length to just use the string contents.

	if((length == 0) || (length > value->Length)) length = value->Length;

	// The Byte[] object gets pinned twice.  Once by GCHandle::Alloc, and once
	// by pin_ptr<>.  The GCHandle is the one that gets held onto until Reset(), 
	// and the pin_ptr<> goes away when we return from this function

	m_pins->Add(GCHandle::Alloc(value, GCHandleType::Pinned));
	pinValue = (length) ? PtrToStringChars(value) : nullptr;

	// Attempt to bind the parameter to the statement, using SQLITE_STATIC since it's 
	// pinned and avoid a potentially costly memory allocation here

	length *= sizeof(wchar_t);
	nResult = sqlite3_bind_text16(m_pStatement->Handle, index, pinValue, length, SQLITE_STATIC);
	if(nResult != SQLITE_OK) throw gcnew zDBExceptions::ParameterBindingException(param, 
		index, m_pStatement->DBHandle, nResult);
}

//---------------------------------------------------------------------------
// zDBStatement::BindParameters
//
// Binds a zDBParameter collection to this statement
//
// Arguments:
//
//	params			- Reference to the zDBParameterCollection to be bound
//	conn			- Reference to the zDBConnection object (for coercion)

void zDBStatement::BindParameters(zDBParameterCollection^ params, zDBConnection^ conn)
{
	int						cParams;			// Number of parameters to bind
	const char*				pszParamName;		// *ANSI* parameter name string
	int						cUnnamed = 0;		// Number of unnamed parameters
	zDBParameter^			param;				// Reference to the zDBParameter object
	zDBParameterValue^		paramValue;			// Current parameter value instance

	CHECK_DISPOSED(m_disposed);
	if(m_status != zDBStatementStatus::Prepared) throw gcnew InvalidOperationException();

	try {

		cParams = sqlite3_bind_parameter_count(m_pStatement->Handle);
		for(int index = 0; index < cParams; index++) {

			pszParamName = sqlite3_bind_parameter_name(m_pStatement->Handle, index + 1);

			// If there was no name associated with the parameter, get the next
			// unnamed parameter value from the collection.  Otherwise, ask the
			// collection to give us the value that's associated with this name

			if(!pszParamName) param = params->GetUnnamedParameter(cUnnamed++);
			else param = params->GetNamedParameter(zDBUtil::FastPtrToStringAnsi(pszParamName));
			
			// Depending on the data type of the parameter, call the appropriate helper
			// routine to actually bind it to the statement.  Some of these actually pin
			// the values, which is why we need the try/catch handler around this thing

			paramValue = param->ValueWrapper;

			switch(param->DbType.Value) {

				case zDBTypeCode::Binary: 
					BindBinaryParameter(param, index + 1, paramValue->ToBinary(), param->Size); 
					break;

				case zDBTypeCode::Boolean: 
					BindBooleanParameter(param, index + 1, paramValue->ToBoolean(), conn->BooleanFormat); 
					break;

				case zDBTypeCode::DateTime:
					BindDateTimeParameter(param, index + 1, paramValue->ToDateTime(), conn->DateTimeFormat);
					break;

				case zDBTypeCode::Float:
					BindFloatParameter(param, index + 1, paramValue->ToFloat());
					break;

				case zDBTypeCode::Guid:
					BindGuidParameter(param, index + 1, paramValue->ToGuid(), conn->GuidFormat);
					break;

				case zDBTypeCode::Integer:
					BindIntegerParameter(param, index + 1, paramValue->ToInteger());
					break;

				case zDBTypeCode::Long:
					BindLongParameter(param, index + 1, paramValue->ToLong());
					break;

				case zDBTypeCode::Null:
					BindNullParameter(param, index + 1);
					break;

				case zDBTypeCode::String:
					BindStringParameter(param, index + 1, paramValue->ToString(), param->Size);
					break;

				default: throw gcnew zDBExceptions::DbTypeUnknownException(param->DbType);
			}
		}
	} // try
	
	catch(Exception^) { Reset(); throw; }		// IMPORTANT: Reset on exception
}

//---------------------------------------------------------------------------
// zDBStatement::ChangeCount::get
//
// Determines the number of rows that have been affected by this statement
// up to this point in time.  This is a running counter than can be checked
// before the status is "Completed", but I'm not certain why you'd want to
//
// Arguments:
//
//	NONE

int zDBStatement::ChangeCount::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return m_changes;
}

//---------------------------------------------------------------------------
// zDBStatement::default::get [int]
//
// Retrieves the value located at the specified ordinal

Object^ zDBStatement::default::get(int ordinal)
{
	CHECK_DISPOSED(m_disposed);
	return GetValue(ordinal);
}

//---------------------------------------------------------------------------
// zDBStatement::default::get [String^]
//
// Retrieves the value located at the specified column

Object^ zDBStatement::default::get(String^ name)
{
	CHECK_DISPOSED(m_disposed);
	return GetValue(GetOrdinal(name));
}

//---------------------------------------------------------------------------
// zDBStatement::ExecuteNonQuery
//
// Executes the statement until it's completed, ignoring any results that
// are generated along the way.
//
// Arguments:
//
//	NONE

int zDBStatement::ExecuteNonQuery(void)
{
	CHECK_DISPOSED(m_disposed);
	if(m_status != zDBStatementStatus::Prepared) throw gcnew InvalidOperationException();

	// The only thing we care about is the first step of the operation.  Unless
	// an exception occurs, this will return Completed or ResultReady.  If Completed,
	// the statement is obviously done.  If ResultReady, it generated a result which
	// also means that the non-query portion is done.  Either way, it's done.

	try { Step(); return m_changes; }			// Single-step statement
	finally { Reset(); }						// Reset before leaving
}

//---------------------------------------------------------------------------
// zDBStatement::ExecuteScalar
//
// Executes the statement until it's completed and returns the first thing
// that comes back in a result set as a scalar result object
//
// Arguments:
//
//	NONE

Object^ zDBStatement::ExecuteScalar(void)
{
	CHECK_DISPOSED(m_disposed);
	if(m_status != zDBStatementStatus::Prepared) throw gcnew InvalidOperationException();

	try {

		// If after the first step into the statement we have a result set row,
		// and there's at least one value in the row (which you would certainly
		// think was always the case), return the value at position zero.  For
		// any other condition (zDBStatementStatus::Completed), return nullptr

		if((Step() == zDBStatementStatus::ResultReady) && 
			(m_metadata->FieldCount > 0)) return GetValue(0);
		else return nullptr;
	}
	
	finally { Reset(); }						// Reset before leaving
}

//---------------------------------------------------------------------------
// zDBStatement::FieldCount::get
//
// Returns the number of columns in the statement's result set

int zDBStatement::FieldCount::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return m_metadata->FieldCount;
}

//---------------------------------------------------------------------------
// zDBStatement::FormatBoolean (static)
//
// Converts a boolean value into a generic Object based on a BooleanFormat
//
// Arguments:
//
//	value			- The boolean value to be formatted
//	format			- BooleanFormat flag

Object^ zDBStatement::FormatBoolean(bool value, zDBBooleanFormat format)
{
	switch(format) {

		case zDBBooleanFormat::NegativeOneZero :
			return (value) ? static_cast<int>(-1) : static_cast<int>(0);

		case zDBBooleanFormat::OneZero :
			return (value) ? static_cast<int>(1) : static_cast<int>(0);

		case zDBBooleanFormat::TrueFalse :
			return (value) ? "true" : "false";

		default : throw gcnew ArgumentOutOfRangeException();
	}
}

//---------------------------------------------------------------------------
// zDBStatement::FormatDateTime (static)
//
// Converts a DateTime structure into a generic Object based on a DateTimeFormat
//
// Arguments:
//
//	value			- The DateTime structure to be formatted
//	format			- DateTimeFormat flag

Object^ zDBStatement::FormatDateTime(DateTime value, zDBDateTimeFormat format)
{
	switch(format) {

		case zDBDateTimeFormat::Ticks : return value.Ticks;
		case zDBDateTimeFormat::ISO8601 : return value.ToString("yyyy-mm-dd HH:mm:ss");
		case zDBDateTimeFormat::Sortable : return value.ToString("s", DateTimeFormatInfo::InvariantInfo);
		case zDBDateTimeFormat::UniversalSortable : return value.ToString("u", DateTimeFormatInfo::InvariantInfo);
		case zDBDateTimeFormat::RFC1123 : return value.ToString("r", DateTimeFormatInfo::InvariantInfo);
		default: throw gcnew ArgumentOutOfRangeException();
	}
}

//---------------------------------------------------------------------------
// zDBStatement::FormatGuid (static)
//
// Converts a Guid structure into a generic Object based on a GuidFormat
//
// Arguments:
//
//	value			- The Guid structure to be formatted
//	format			- GuidFormat flag

Object^ zDBStatement::FormatGuid(Guid value, zDBGuidFormat format)
{
	switch(format) {

		case zDBGuidFormat::Binary: return value.ToByteArray();
		case zDBGuidFormat::Bracketed : return value.ToString("B");
		case zDBGuidFormat::HexString : return value.ToString("N");
		case zDBGuidFormat::Hyphenated : return value.ToString("D");
		case zDBGuidFormat::Parenthetic : return value.ToString("P");
		default: throw gcnew ArgumentOutOfRangeException();
	}
}

//---------------------------------------------------------------------------
// zDBStatement::GeneratesResultSet::get
//
// Determines if this query will generate a result set or not
//
// Arguments:
//
//	NONE

bool zDBStatement::GeneratesResultSet::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return (m_metadata->FieldCount > 0);
}

//---------------------------------------------------------------------------
// zDBStatement::GetBoolean
//
// Gets the specified column as a boolean value
//
// Arguments:
//
//	ordinal		- Ordinal value of the column to be retrieved

bool zDBStatement::GetBoolean(int ordinal)
{
	const wchar_t*			pwszValue;			// Pointer to a string value

	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckDataRecordOrdinal(this, ordinal);

	if(m_status != zDBStatementStatus::ResultReady) throw gcnew zDBExceptions::NoDataPresentException();

	// Booleans can be stored as either a string or an integer value in the database ...

	switch(sqlite3_column_type(m_pStatement->Handle, ordinal)) {

		case SQLITE_TEXT:

			// Yank out a pointer to the value as a Unicode string and see if it
			// can be converted into a boolean value.  Like always, change any problems
			// into a more generic InvalidCastException for the caller

			pwszValue = reinterpret_cast<const wchar_t*>(sqlite3_column_text16(m_pStatement->Handle, ordinal));

			try { return Convert::ToBoolean(gcnew String(pwszValue)); }
			catch(Exception^) { throw gcnew InvalidCastException(); }

		case SQLITE_INTEGER:

			// When the data is stored as an integer, booleans are quite simple

			return (sqlite3_column_int(m_pStatement->Handle, ordinal)) ? true : false;
		
		default: throw gcnew InvalidCastException();	// <--- BLOB / FLOAT / NULL
	}
}

//---------------------------------------------------------------------------
// zDBStatement::GetBinaryReader
//
// Gets the specified column as a special zDBBinaryReader instance
//
// Arguments:
//
//	ordinal		- Ordinal value of the column to be retrieved

zDBBinaryReader^ zDBStatement::GetBinaryReader(int ordinal)
{
	zDBBinaryReader^			binaryValue;		// Object to return to caller

	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckDataRecordOrdinal(this, ordinal);

	if(m_status != zDBStatementStatus::ResultReady) throw gcnew zDBExceptions::NoDataPresentException();
	if(sqlite3_column_type(m_pStatement->Handle, ordinal) == SQLITE_NULL) throw gcnew InvalidCastException();

	// Create the new object, cache it so we can kill it off when we Reset(), and
	// then return an instance of it to the calling application

	binaryValue = gcnew zDBBinaryReader(m_pStatement, ordinal);
	m_binaries->Add(binaryValue);
	return binaryValue;
}

//---------------------------------------------------------------------------
// zDBStatement::GetByte
//
// Gets the specified column as an 8 bit unsigned value
//
// Arguments:
//
//	ordinal		- Ordinal value of the column to be retrieved

System::Byte zDBStatement::GetByte(int ordinal)
{
	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckDataRecordOrdinal(this, ordinal);

	if(m_status != zDBStatementStatus::ResultReady) throw gcnew zDBExceptions::NoDataPresentException();
	if(sqlite3_column_type(m_pStatement->Handle, ordinal) == SQLITE_NULL) throw gcnew InvalidCastException();

	return static_cast<System::Byte>(sqlite3_column_int(m_pStatement->Handle, ordinal));
}

//---------------------------------------------------------------------------
// zDBStatement::GetBytes
//
// Reads a specific range of bytes from the specified column
//
// Arguments:
//
//	ordinal			- Ordinal value of the column to be retrieved
//	fieldOffset		- Offset into the column's data to begin reading
//	buffer			- Destination buffer.  Set to NULL to just get length
//	bufferOffset	- Offset into destination buffer to begin writing
//	count			- Maximum number of bytes to copy into the buffer

__int64 zDBStatement::GetBytes(int ordinal, __int64 fieldOffset, array<System::Byte>^ buffer, 
	int bufferOffset, int count)
{
	int							cbBlob;				// Length of the BLOB field
	PinnedBytePtr				pinBuffer;			// Pinned buffer pointer
	const unsigned char*		puBlob;				// BLOB data pointer

	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckDataRecordOrdinal(this, ordinal);

	// More than usual invariants here.  fieldOffset cannot be negative, nor can it
	// exceed the maximum value an Int32 can hold.  If buffer is non-NULL, that offset
	// is also bounds checked for negative/too big as well.

	if((fieldOffset < 0) || (fieldOffset > Int32::MaxValue)) throw gcnew ArgumentOutOfRangeException();
	if((buffer != nullptr) && ((bufferOffset < 0) || (bufferOffset >= buffer->Length))) throw gcnew ArgumentOutOfRangeException();
	
	if(m_status != zDBStatementStatus::ResultReady) throw gcnew zDBExceptions::NoDataPresentException();
	if(sqlite3_column_type(m_pStatement->Handle, ordinal) == SQLITE_NULL) throw gcnew InvalidCastException();

	// Get the length of the column data in bytes.  If the caller provided a NULL
	// buffer reference, just send that back as the result (see IDataRecord in MSDN)

	cbBlob = sqlite3_column_bytes(m_pStatement->Handle, ordinal);
	if(buffer == nullptr) return static_cast<__int64>(cbBlob);
	if(static_cast<int>(fieldOffset) >= cbBlob) throw gcnew ArgumentOutOfRangeException();

	// Determine the actual amount of data that we can copy.  Basically, it's the
	// MINIMUM of three things: the requested length, the length of the source data,
	// and the length of the destination buffer.  Bail if negative or zero

	count = min(count, cbBlob - static_cast<int>(fieldOffset));
	count = min(count, buffer->Length - bufferOffset);
	if(count <= 0) return 0;

	pinBuffer = &buffer[bufferOffset];			// Pin down the managed buffer

	// Grab a pointer to the specified offset within the BLOB data

	puBlob = reinterpret_cast<const unsigned char*>(sqlite3_column_blob(m_pStatement->Handle, ordinal));
	puBlob += fieldOffset;

	memcpy(pinBuffer, puBlob, count);			// Finally ... copy the data
	return static_cast<__int64>(count);			// Return total bytes processed
}

//---------------------------------------------------------------------------
// zDBStatement::GetChar
//
// Gets the specified column as a single character value
//
// Arguments:
//
//	ordinal		- Ordinal value of the column to be retrieved

Char zDBStatement::GetChar(int ordinal)
{
	const wchar_t*				pwszValue;		// Pointer to the string value

	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckDataRecordOrdinal(this, ordinal);

	if(m_status != zDBStatementStatus::ResultReady) throw gcnew zDBExceptions::NoDataPresentException();
	if(sqlite3_column_type(m_pStatement->Handle, ordinal) == SQLITE_NULL) throw gcnew InvalidCastException();

	// Grab a pointer to the string-based value of this column.  If the engine
	// gives us back a zero-length string, we behave as we would for a NULL since
	// the string simply isn't long enough

	pwszValue = reinterpret_cast<const wchar_t*>(sqlite3_column_text16(m_pStatement->Handle, ordinal));
	if(*pwszValue == NULL) throw gcnew InvalidCastException();

	return *pwszValue;							// Return first char in string
}

//---------------------------------------------------------------------------
// zDBStatement::GetChars
//
// Reads a specific range of characters from the specified column
//
// Arguments:
//
//	ordinal			- Ordinal value of the column to be retrieved
//	fieldOffset		- Offset into the column's data to begin reading
//	buffer			- Destination buffer.  Set to NULL to just get length
//	bufferOffset	- Offset into destination buffer to begin writing
//	count			- Maximum number of bytes to copy into the buffer

__int64 zDBStatement::GetChars(int ordinal, __int64 fieldOffset, array<Char>^ buffer, 
	int bufferOffset, int count)
{
	PinnedCharPtr			pinBuffer;			// Pinned buffer pointer
	const wchar_t*			pwszValue;			// Pointer to the value string
	size_t					cchValue;			// Length of value in wchar_ts

	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckDataRecordOrdinal(this, ordinal);

	// More than usual invariants here.  fieldOffset cannot be negative, nor can it
	// exceed the maximum value an Int32 can hold.  If buffer is non-NULL, that offset
	// is also bounds checked for negative/too big as well.

	if((fieldOffset < 0) || (fieldOffset > Int32::MaxValue)) throw gcnew ArgumentOutOfRangeException();
	if((buffer != nullptr) && ((bufferOffset < 0) || (bufferOffset >= buffer->Length))) throw gcnew ArgumentOutOfRangeException();

	if(m_status != zDBStatementStatus::ResultReady) throw gcnew zDBExceptions::NoDataPresentException();
	if(sqlite3_column_type(m_pStatement->Handle, ordinal) == SQLITE_NULL) throw gcnew InvalidCastException();

	// We'll need a pointer to the string value to start working the magic here

	pwszValue = reinterpret_cast<const wchar_t*>(sqlite3_column_text16(m_pStatement->Handle, ordinal));

	// Get the length of the string in characters.  If the caller provided a NULL
	// buffer reference, just send that back as the result (see IDataRecord in MSDN)

	cchValue = wcslen(pwszValue);
	if(!buffer) return static_cast<__int64>(cchValue);
	if(static_cast<size_t>(fieldOffset) >= cchValue) throw gcnew ArgumentOutOfRangeException();

	// Determine the actual amount of data that we can copy.  Basically, it's the
	// MINIMUM of three things: the requested length, the length of the source data,
	// and the length of the destination buffer.  Bail if negative or zero

	count = min(count, static_cast<int>(cchValue) - static_cast<int>(fieldOffset));
	count = min(count, buffer->Length - bufferOffset);
	if(count <= 0) return 0;

	pinBuffer = &buffer[bufferOffset];			// Pin down the managed buffer
	pwszValue += fieldOffset;					// Move the string pointer along

	// Copy the data into the pinned character array and return how long it is

	wmemcpy_s(pinBuffer, buffer->Length, pwszValue, count);
	return static_cast<__int64>(count);
}

//---------------------------------------------------------------------------
// zDBStatement::GetDataTypeName
//
// Returns the data type name for the specified column.  If the column is an
// expression, the type of the result will be used to force a generic mapping
//
// Arguments:
//
//	ordinal		- Ordinal value of the column to be retrieved

String^ zDBStatement::GetDataTypeName(int ordinal)
{
	String^						dataType;			// Data type name

	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckDataRecordOrdinal(this, ordinal);

	// The schema doesn't always provide a data type name, since expression columns
	// don't have a data type we can determine from just the schema.  Watch for it

	dataType = m_metadata->GetDataTypeName(ordinal);
	if((dataType != nullptr) && (dataType->Length > 0)) return dataType;

	// There is no schema information for this column, meaning it's an expression,
	// so we have to go by the result set column type rather than the schema

	switch(sqlite3_column_type(m_pStatement->Handle, ordinal)) {

		case SQLITE_INTEGER : return "INTEGER";
		case SQLITE_FLOAT   : return "FLOAT";
		case SQLITE_TEXT    : return "TEXT";
		case SQLITE_BLOB    : return "BLOB";
		case SQLITE_NULL	: return "NULL";
		default				: return "OBJECT";
	}
}

//---------------------------------------------------------------------------
// zDBStatement::GetDateTime
//
// Gets the specified column as a DateTime value.  The value must currently
// be stored in an ISO8601 string format, but I may change this in the future
//
// Arguments:
//
//	ordinal		- Ordinal value of the column to be retrieved

DateTime zDBStatement::GetDateTime(int ordinal)
{
	const wchar_t*			pwszValue;			// Pointer to a string value

	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckDataRecordOrdinal(this, ordinal);

	if(m_status != zDBStatementStatus::ResultReady) throw gcnew zDBExceptions::NoDataPresentException();

	// Depending on the internal data type of this column, we can construct the
	// DateTime a couple different ways ....

	switch(sqlite3_column_type(m_pStatement->Handle, ordinal)) {

		case SQLITE_TEXT:

			// Yank out a pointer to the value as a Unicode string.  The string
			// is expected to be in one of the standard ISO8601 date/time formats

			pwszValue = reinterpret_cast<const wchar_t*>(sqlite3_column_text16(m_pStatement->Handle, ordinal));

			// Attempt to convert this string into a DateTime value by using the
			// ParseExact method along with as much invariant info as possible.  If
			// the conversion fails, be consistent and throw an InvalidCast instead

			try { return DateTime::Parse(gcnew String(pwszValue), DateTimeFormatInfo::InvariantInfo); }
			catch(Exception^) { throw gcnew InvalidCastException(); }

		case SQLITE_INTEGER:

			// When the data is stored as an integer, assume that it represents ticks

			try { return DateTime(sqlite3_column_int64(m_pStatement->Handle, ordinal)); }
			catch(Exception^) { throw gcnew InvalidCastException(); }
		
		default: throw gcnew InvalidCastException();	// <--- BLOB / FLOAT / NULL
	}
}

//---------------------------------------------------------------------------
// zDBStatement::GetDecimal
//
// Gets the specified column as a fixed point signed numeric value.  This
// value can be extremely large, so we use a string->Decimal conversion rather
// than risk truncation by reading it in as a double precision float
//
// Arguments:
//
//	ordinal		- Ordinal value of the column to be retrieved

Decimal zDBStatement::GetDecimal(int ordinal)
{
	const wchar_t*			pwszValue;			// Value as a Unicode string

	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckDataRecordOrdinal(this, ordinal);

	if(m_status != zDBStatementStatus::ResultReady) throw gcnew zDBExceptions::NoDataPresentException();
	if(sqlite3_column_type(m_pStatement->Handle, ordinal) == SQLITE_NULL) throw gcnew InvalidCastException();

	// Grab the value of the specified column as a Unicode string to make sure
	// that nothing is going to get lost in the translation of very large values

	pwszValue = reinterpret_cast<const wchar_t*>(sqlite3_column_text16(m_pStatement->Handle, ordinal));
	return Convert::ToDecimal(gcnew String(pwszValue));
}

//---------------------------------------------------------------------------
// zDBStatement::GetDouble
//
// Gets the specified column as a double precision IEEE floating point value
//
// Arguments:
//
//	ordinal		- Ordinal value of the column to be retrieved

double zDBStatement::GetDouble(int ordinal)
{
	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckDataRecordOrdinal(this, ordinal);

	if(m_status != zDBStatementStatus::ResultReady) throw gcnew zDBExceptions::NoDataPresentException();
	if(sqlite3_column_type(m_pStatement->Handle, ordinal) == SQLITE_NULL) throw gcnew InvalidCastException();

	return sqlite3_column_double(m_pStatement->Handle, ordinal);
}

//---------------------------------------------------------------------------
// zDBStatement::GetFieldType
//
// Gets the System.Type that would be returned if one were to call GetValue.
//
// Arguments:
//
//	ordinal		- Ordinal value of the column to be looked up

Type^ zDBStatement::GetFieldType(int ordinal)
{
	CHECK_DISPOSED(m_disposed);
	return m_metadata->GetFieldType(ordinal);
}

//---------------------------------------------------------------------------
// zDBStatement::GetFloat
//
// Gets the specified column as a single precision IEEE floating point value
//
// Arguments:
//
//	ordinal		- Ordinal value of the column to be retrieved

float zDBStatement::GetFloat(int ordinal)
{
	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckDataRecordOrdinal(this, ordinal);

	if(m_status != zDBStatementStatus::ResultReady) throw gcnew zDBExceptions::NoDataPresentException();
	if(sqlite3_column_type(m_pStatement->Handle, ordinal) == SQLITE_NULL) throw gcnew InvalidCastException();

	return static_cast<float>(sqlite3_column_double(m_pStatement->Handle, ordinal));
}

//---------------------------------------------------------------------------
// zDBStatement::GetGuid
//
// Gets the specified column as a UUID value.  The underlying data can be
// either a string, OR a 16 byte long BLOB for this conversion to work
//
// Arguments:
//
//	ordinal		- Ordinal value of the column to be retrieved

Guid zDBStatement::GetGuid(int ordinal)
{
	const wchar_t*			pwszValue;			// Pointer to a string value
	array<System::Byte>^	blobValue;			// Managed BLOB array
	PinnedBytePtr			pinBlob;			// Pinned BLOB array pointer

	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckDataRecordOrdinal(this, ordinal);

	if(m_status != zDBStatementStatus::ResultReady) throw gcnew zDBExceptions::NoDataPresentException();

	// Depending on the internal data type of this column, we can construct a GUID
	// a couple different ways.....

	switch(sqlite3_column_type(m_pStatement->Handle, ordinal)) {

		case SQLITE_TEXT:

			// Yank out a pointer to the value as a Unicode string.  It can be
			// in any number of possible formats here.  See System::Guid in MSDN

			pwszValue = reinterpret_cast<const wchar_t*>(sqlite3_column_text16(m_pStatement->Handle, ordinal));

			// The default exception thrown by System::Guid is FormatException.
			// To remain consistent here, change that into an InvalidCast instead

			try { return Guid(gcnew String(pwszValue)); }
			catch(Exception^) { throw gcnew InvalidCastException(); }

		case SQLITE_BLOB:

			// The BLOB must be exactly 16 bytes long for this to be a valid GUID

			if(sqlite3_column_bytes(m_pStatement->Handle, ordinal) != 16) throw gcnew InvalidCastException();

			blobValue = gcnew array<System::Byte>(16);		// Allocate the Byte[] array
			pinBlob = &blobValue[0];						// Pin the Byte[] array

			// Since the managed array has been pinned, we can copy right into it

			memcpy_s(pinBlob, 16, sqlite3_column_blob(m_pStatement->Handle, ordinal), 16);

			// There shouldn't be any reason for this to fail, but just in case
			// we'll wrap it up and re-throw anything as an InvalidCastException

			try { return Guid(blobValue); }
			catch(Exception^) { throw gcnew InvalidCastException(); }
		
		default: throw gcnew InvalidCastException();	// <--- INTEGER / FLOAT / NULL
	}
}

//---------------------------------------------------------------------------
// zDBStatement::GetInt16
//
// Gets the specified column as a 16 bit integer value
//
// Arguments:
//
//	ordinal		- Ordinal value of the column to be retrieved

short zDBStatement::GetInt16(int ordinal)
{
	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckDataRecordOrdinal(this, ordinal);

	if(m_status != zDBStatementStatus::ResultReady) throw gcnew zDBExceptions::NoDataPresentException();
	if(sqlite3_column_type(m_pStatement->Handle, ordinal) == SQLITE_NULL) throw gcnew InvalidCastException();

	return static_cast<short>(sqlite3_column_int(m_pStatement->Handle, ordinal));
}

//---------------------------------------------------------------------------
// zDBStatement::GetInt32
//
// Gets the specified column as a 32 bit integer value
//
// Arguments:
//
//	ordinal		- Ordinal value of the column to be retrieved

int zDBStatement::GetInt32(int ordinal)
{
	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckDataRecordOrdinal(this, ordinal);

	if(m_status != zDBStatementStatus::ResultReady) throw gcnew zDBExceptions::NoDataPresentException();
	if(sqlite3_column_type(m_pStatement->Handle, ordinal) == SQLITE_NULL) throw gcnew InvalidCastException();

	return sqlite3_column_int(m_pStatement->Handle, ordinal);
}

//---------------------------------------------------------------------------
// zDBStatement::GetInt64
//
// Gets the specified column as a 64 bit integer value
//
// Arguments:
//
//	ordinal		- Ordinal value of the column to be retrieved

__int64 zDBStatement::GetInt64(int ordinal)
{
	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckDataRecordOrdinal(this, ordinal);

	if(m_status != zDBStatementStatus::ResultReady) throw gcnew zDBExceptions::NoDataPresentException();
	if(sqlite3_column_type(m_pStatement->Handle, ordinal) == SQLITE_NULL) throw gcnew InvalidCastException();

	return sqlite3_column_int64(m_pStatement->Handle, ordinal);
}

//---------------------------------------------------------------------------
// zDBStatement::GetName
//
// Looks up the name of a specific field in the result set
//
// Arguments:
//
//	ordinal			- The ordinal of the field in the result set

String^ zDBStatement::GetName(int ordinal)
{
	CHECK_DISPOSED(m_disposed);
	return m_metadata->GetName(ordinal);
}

//---------------------------------------------------------------------------
// zDBStatement::GetOrdinal
//
// Looks up the ordinal value of a field based on it's name.  This operation 
// is inefficient and should not be used if it can be avoided.
//
// Arguments:
//
//	name			- The name to be looked up (case-insensitive)

int zDBStatement::GetOrdinal(String^ name)
{
	CHECK_DISPOSED(m_disposed);

	// Just iterate over every single column and see if we have a match. Due
	// to the IDataRecord contract, we make 2 passes.  The first pass uses a
	// case sensitive search, the second pass does not.

	for(int index = 0; index < m_metadata->FieldCount; index++)
		if(String::Compare(name, GetName(index)) == 0) return index;

	for(int index = 0; index < m_metadata->FieldCount; index++)
		if(String::Compare(name, GetName(index), true) == 0) return index;

	throw gcnew zDBExceptions::InvalidColumnNameException(name);
}

//---------------------------------------------------------------------------
// zDBStatement::GetProviderSpecificFieldType
//
// Gets a System.Type that represents the underlying type of a column
//
// Arguments:
//
//	ordinal		- Ordinal value of the column to be retrieved

Type^ zDBStatement::GetProviderSpecificFieldType(int ordinal)
{
	CHECK_DISPOSED(m_disposed);
	return m_metadata->GetProviderSpecificFieldType(ordinal);
}

//---------------------------------------------------------------------------
// zDBStatement::GetProviderSpecificValue
//
// Gets the specified column value as the type from GetProviderSpecificFieldType()
//
// Arguments:
//
//	ordinal		- Ordinal value of the column to be retrieved

Object^ zDBStatement::GetProviderSpecificValue(int ordinal)
{
	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckDataRecordOrdinal(this, ordinal);

	return GetValueAs(ordinal, m_metadata->GetProviderSpecificFieldType(ordinal));
}

//---------------------------------------------------------------------------
// zDBStatement::GetProviderSpecificValues
//
// Loads up an array of Object references with data from the current row.
// Note that unlike what MSDN implies about efficiency, all this does is
// call GetValue() multiple times.  There is no secret performance increase
//
// Arguments:
//
//	values			- Array of Object references to be loaded with values

int zDBStatement::GetProviderSpecificValues(array<Object^>^ values)
{
	CHECK_DISPOSED(m_disposed);
	if(m_status != zDBStatementStatus::ResultReady) throw gcnew zDBExceptions::NoDataPresentException();

	// Determine how many objects we're going to copy, which is the MINIMUM
	// of the number of columns in the result set and what the target array can hold

	int count = min(values->Length, m_metadata->FieldCount);
	if(count <= 0) return 0;

	// Just call into GetValueAs a bunch of times to load up each slot in the array

	for(int index = 0; index < count; index++) 
		values[index] = GetValueAs(index, m_metadata->GetProviderSpecificFieldType(index));

	return count;							// Return number of values copied
}

//---------------------------------------------------------------------------
// zDBStatement::GetSchemaTable
//
// Generates a DataTable with schema information about the result set
//
// Arguments:
//
//	NONE

DataTable^ zDBStatement::GetSchemaTable(void)
{
	CHECK_DISPOSED(m_disposed);
	return m_metadata->BuildSchemaTable();
}

//---------------------------------------------------------------------------
// zDBStatement::GetString
//
// Gets the specified column as a string value
//
// Arguments:
//
//	ordinal		- Ordinal value of the column to be retrieved

String^ zDBStatement::GetString(int ordinal)
{
	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckDataRecordOrdinal(this, ordinal);

	if(m_status != zDBStatementStatus::ResultReady) throw gcnew zDBExceptions::NoDataPresentException();
	if(sqlite3_column_type(m_pStatement->Handle, ordinal) == SQLITE_NULL) throw gcnew InvalidCastException();

	return gcnew String(reinterpret_cast<const wchar_t*>(sqlite3_column_text16(m_pStatement->Handle, ordinal)));
}

//---------------------------------------------------------------------------
// zDBStatement::GetValue
//
// Gets the specified column value as the type from GetFieldType()
//
// Arguments:
//
//	ordinal		- Ordinal value of the column to be retrieved

Object^ zDBStatement::GetValue(int ordinal)
{
	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckDataRecordOrdinal(this, ordinal);

	return GetValueAs(ordinal, m_metadata->GetFieldType(ordinal));
}

//---------------------------------------------------------------------------
// zDBStatement::GetValueAs (private)
//
// Gets the specified column as an object of the specified type
//
// Arguments:
//
//	ordinal		- Ordinal value of the column to be retrieved
//	type		- Data type to try and coerce the value into

Object^ zDBStatement::GetValueAs(int ordinal, Type^ type)
{
	int						sqliteType;			// SQLite data type code
	__int64					i64Value;			// Int64 value
	double					dblValue;			// Double value
	const wchar_t*			pwszValue;			// String value

	if(m_status != zDBStatementStatus::ResultReady) throw gcnew zDBExceptions::NoDataPresentException();

	// If the value stored in the column is NULL, we disobey the data type and
	// return DBNull::Value instead.  Do this before we get the .NET data type,
	// since that lookup can be lengthy and would serve no purpose if NULL

	sqliteType = sqlite3_column_type(m_pStatement->Handle, ordinal);
	if(sqliteType == SQLITE_NULL) return DBNull::Value;

	// SPECIAL CASE HANDLERS
	if(type == bool::typeid) return GetBoolean(ordinal);
	if(type == DateTime::typeid) return GetDateTime(ordinal);
	if(type == Guid::typeid) return GetGuid(ordinal);

	// Depending on the INTERNAL data type of the value, yank it out and attempt
	// to convert it into the EXTERNAL data type reported by GetFieldType

	switch(sqliteType) {

		// SQLITE_INTEGER --> __int64 --> targetType		
		case SQLITE_INTEGER:

			// SPECIAL CASE: If target is Int32, call column_int to save time
			if(type == int::typeid) 
				return sqlite3_column_int(m_pStatement->Handle, ordinal);

			i64Value = sqlite3_column_int64(m_pStatement->Handle, ordinal);
			if(type == __int64::typeid) return i64Value;
			return Convert::ChangeType(i64Value, type);

		// SQLITE_FLOAT --> double --> targetType
		case SQLITE_FLOAT:
			
			dblValue = sqlite3_column_double(m_pStatement->Handle, ordinal);
			if(type == double::typeid) return dblValue;
			return Convert::ChangeType(dblValue, type);

		// SQLITE_BLOB --> zDBBinaryReader --> targetType
		case SQLITE_BLOB:

			return GetBinaryReader(ordinal)->ToType(type, nullptr);;

		// DEFAULT / SQLITE_TEXT --> String^ --> targetType
		default:

			pwszValue = reinterpret_cast<const wchar_t*>(sqlite3_column_text16(m_pStatement->Handle, ordinal));
			if(type == String::typeid) return gcnew String(pwszValue);
			return Convert::ChangeType(gcnew String(pwszValue), type);
	}
}

//---------------------------------------------------------------------------
// zDBStatement::GetValues
//
// Loads up an array of Object references with data from the current row.
// Note that unlike what MSDN implies about efficiency, all this does is
// call GetValue() multiple times.  There is no secret performance increase
//
// Arguments:
//
//	values			- Array of Object references to be loaded with values

int zDBStatement::GetValues(array<Object^>^ values)
{
	CHECK_DISPOSED(m_disposed);
	if(m_status != zDBStatementStatus::ResultReady) throw gcnew zDBExceptions::NoDataPresentException();

	// Determine how many objects we're going to copy, which is the MINIMUM
	// of the number of columns in the result set and what the target array can hold

	int count = min(values->Length, m_metadata->FieldCount);
	if(count <= 0) return 0;

	// Just call into GetValueAs a bunch of times to load up each slot in the array

	for(int index = 0; index < count; index++) 
		values[index] = GetValueAs(index, m_metadata->GetFieldType(index));

	return count;							// Return number of values copied
}

//---------------------------------------------------------------------------
// zDBStatement::IsDBNull
//
// Determines if the specified column is NULL in the current result set row
//
// Arguments:
//
//	ordinal		- Ordinal value of the column to be checked

bool zDBStatement::IsDBNull(int ordinal)
{
	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckDataRecordOrdinal(this, ordinal);

	if(m_status != zDBStatementStatus::ResultReady) throw gcnew zDBExceptions::NoDataPresentException();

	return (sqlite3_column_type(m_pStatement->Handle, ordinal) == SQLITE_NULL);
}

//---------------------------------------------------------------------------
// zDBStatement::RecompileStatement (private)
//
// Forces a recompile of the SQL statement, usually due to a schema change
//
// Arguments:
//
//	NONE

void zDBStatement::RecompileStatement(void)
{
	sqlite3_stmt*		hNewStatement;						// New handle
	PinnedStringPtr		pinSql = PtrToStringChars(m_sql);	// Original SQL
	int					nResult;							// SQLite result

	// Attempt to recompile the same SQL statement that was originally used

	nResult = sqlite3_prepare16(m_pStatement->DBHandle, pinSql, -1, &hNewStatement, NULL);
	if(nResult != SQLITE_OK) throw gcnew zDBExceptions::StatementRecompileException(m_sql,
		m_pStatement->DBHandle, nResult);

	// Transfer the bound parameters from the old statement to the new one
	// (This is an extremely handy feature ... I'm glad you can do this)

	nResult = sqlite3_transfer_bindings(m_pStatement->Handle, hNewStatement);
	if(nResult != SQLITE_OK) throw gcnew zDBExceptions::BindingTransferException(m_sql,
		m_pStatement->DBHandle, nResult);

	// NOTE: StatementHandle::operator= releases the previous handle
	// when you directly assign a new one to it like this

	*m_pStatement = hNewStatement;				// Presto-chango the handle
}

//---------------------------------------------------------------------------
// zDBStatement::Reset
//
// Forces a reset of the statement handle
//
// Arguments:
//
//	NONE

void zDBStatement::Reset(void)
{
	int						nResult;			// Result from function call

	CHECK_DISPOSED(m_disposed);

	// Dispose of any outstanding zDBBinaryReader objects and clear the collection

	for each(ITrackableObject^ obj in m_binaries)
		if(ObjectTracker::IsObjectAlive(obj)) delete obj;

	m_binaries->Clear();						// Remove all instances

	// Reset the SQLITE statement handle itself

	nResult = sqlite3_reset(m_pStatement->Handle);
	if(nResult != SQLITE_OK) throw gcnew zDBException(m_pStatement->DBHandle, nResult);

	// Clear all of the statement bindings

	nResult = sqlite3_clear_bindings(m_pStatement->Handle);
	if(nResult != SQLITE_OK) throw gcnew zDBException(m_pStatement->DBHandle, nResult);

	// Unpin any Byte[] and/or String parameter objects by freeing the GCHandle

	for each(GCHandle item in m_pins) item.Free();
	m_pins->Clear();

	m_status = zDBStatementStatus::Prepared;		// Back to prepared
	m_changes = 0;										// Back to zero
}

//---------------------------------------------------------------------------
// zDBStatement::Sql::get
//
// Returns a copy of the original SQL command text used for this statement
//
// Arguments:
//
//	NONE

String^ zDBStatement::Sql::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return m_sql;
}

//---------------------------------------------------------------------------
// zDBStatement::Status::Get
//
// Returns the current status of the statement
//
// Arguments:
//
//	NONE

zDBStatementStatus zDBStatement::Status::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return m_status;
}

//---------------------------------------------------------------------------
// zDBStatement::Step
//
// Executes the next step of the statement and returns the current state.
// In the event of an error, an exception is NOT thrown .. check .ErrorMessage
//
// Arguments:
//
//	NONE

zDBStatementStatus zDBStatement::Step(void)
{
	int					nResult;			// Result from function call

	CHECK_DISPOSED(m_disposed);

	// Dispose of any outstanding zDBBinaryReader objects and clear the collection

	for each(ITrackableObject^ obj in m_binaries)
		if(ObjectTracker::IsObjectAlive(obj)) delete obj;

	m_binaries->Clear();						// Remove all instances

	nResult = sqlite3_step(m_pStatement->Handle);		// <--- Execute the next step

	// If the step operation failed, and this was the first step of the query,
	// we can recover and retry if the problem was SQLITE_SCHEMA.  The only way
	// we can seemingly tell this is to call sqlite3_reset.  (sqlite3_expired 
	// does not catch every single bad thing that can happen)....
	ENGINE_ISSUE(3.3.8, "sqlite3_expired does not catch all possible problems");

	if((nResult == SQLITE_ERROR) && (m_status == zDBStatementStatus::Prepared)) {
		
		if(sqlite3_reset(m_pStatement->Handle) == SQLITE_SCHEMA) {
			
			m_status = zDBStatementStatus::Error;			// In case of exception
			RecompileStatement();							// Attempt a recompile
			nResult = sqlite3_step(m_pStatement->Handle);	// Retry the operation
		}
	}

	m_status = static_cast<zDBStatementStatus>(nResult);

	// If the engine gave us back something other than SQLITE_DONE or SQLITE_ROW,
	// bad things have happened.  Note: we don't support the retry on busy thing
	// since the busy timeout should have been set before this was executed

	if((nResult != SQLITE_DONE) && (nResult != SQLITE_ROW))
		throw gcnew zDBExceptions::StatementStepException(m_sql, m_pStatement->DBHandle, nResult);

	m_changes += sqlite3_changes(m_pStatement->DBHandle);
	return m_status;		
}

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)
