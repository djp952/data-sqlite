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
#include "SqliteParameterValue.h"		// Include SqliteParameterValue declarations

#pragma warning(push, 4)			// Enable maximum compiler warnings

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// Type Declarations
//---------------------------------------------------------------------------

// TYPECODE_MAP
//
// Defines the global structure of the TypeCode->DbType map array

typedef struct {
	
	TypeCode	typeCode;				// TypeCode (and index) value
	SqliteType		providerDbType;			// Best SqliteType value
	DbType		genericDbType;			// Best generic DbType value

} TYPECODE_MAP_ENTRY, *PTYPECODE_MAP_ENTRY, *PTYPECODE_MAP;

//---------------------------------------------------------------------------
// Global Variables
//---------------------------------------------------------------------------

// g_typeCodeMap
//
// A global array of TypeCode->DbType mappings.  Note that there is no TypeCode
// for value 17, but in order to use the TypeCode as the array index we add one
TYPECODE_MAP_ENTRY g_typeCodeMap[] = { 

	{ TypeCode::Empty,		SqliteType::Null,		DbType::Object },		// 0
	{ TypeCode::Object,		SqliteType::String,	DbType::String },		// 1
	{ TypeCode::DBNull,		SqliteType::Null,		DbType::Object },		// 2
	{ TypeCode::Boolean,	SqliteType::Boolean,	DbType::Boolean },		// 3
	{ TypeCode::Char,		SqliteType::String,	DbType::String },		// 4
	{ TypeCode::SByte,		SqliteType::Integer,	DbType::SByte },		// 5
	{ TypeCode::Byte,		SqliteType::Integer,	DbType::Byte },			// 6
	{ TypeCode::Int16,		SqliteType::Integer,	DbType::Int16 },		// 7
	{ TypeCode::UInt16,		SqliteType::Integer,	DbType::UInt16 },		// 8
	{ TypeCode::Int32,		SqliteType::Integer,	DbType::Int32 },		// 9
	{ TypeCode::UInt32,		SqliteType::Integer,	DbType::UInt32 },		// 10
	{ TypeCode::Int64,		SqliteType::Long,		DbType::Int64 },		// 11
	{ TypeCode::UInt64,		SqliteType::Long,		DbType::UInt64 },		// 12
	{ TypeCode::Single,		SqliteType::Float,		DbType::Single },		// 13
	{ TypeCode::Double,		SqliteType::Float,		DbType::Double },		// 14
	{ TypeCode::Decimal,	SqliteType::Float,		DbType::Decimal },		// 15
	{ TypeCode::DateTime,	SqliteType::DateTime,	DbType::DateTime },		// 16
	{ (TypeCode)17,			SqliteType::Null,		DbType::Object },		// 17 (unused)
	{ TypeCode::String,		SqliteType::String,	DbType::String },		// 18
};

//---------------------------------------------------------------------------
// SqliteParameterValue::ByteArrayToDouble (private, static)
//
// Helper function to convert a byte array into a double
//
// Arguments:
//
//	value		- Byte array to be converted

double SqliteParameterValue::ByteArrayToDouble(array<System::Byte>^ value)
{
	Debug::Assert(value != nullptr);

	if(value->Length >= sizeof(double)) return BitConverter::ToDouble(value, 0);
	else if(value->Length >= sizeof(float)) return BitConverter::ToSingle(value, 0);
	else return 0.0;
}

//---------------------------------------------------------------------------
// SqliteParameterValue::ByteArrayToInt32 (private, static)
//
// Helper function to convert a byte array into a 32 bit integer
//
// Arguments:
//
//	value		- Byte array to be converted

int SqliteParameterValue::ByteArrayToInt32(array<System::Byte>^ value)
{
	Debug::Assert(value != nullptr);

	if(value->Length >= sizeof(int)) return BitConverter::ToInt32(value, 0);
	else if(value->Length >= sizeof(short)) return BitConverter::ToInt16(value, 0);
	else if(value->Length == 1) return value[0];
	else return 0;
}

//---------------------------------------------------------------------------
// SqliteParameterValue::Size::get
//
// Gets the size of the data within the column.  For arrays, this is the
// number of elements.  For strings, this is the number of characters.
// For everything else it's zero since it's not used.

int SqliteParameterValue::Size::get(void)
{
	if(m_type->IsArray) return static_cast<Array^>(m_value)->Length;
	else if(m_type == String::typeid) return static_cast<String^>(m_value)->Length;
	else return 0;
}

//---------------------------------------------------------------------------
// SqliteParameterValue::ToBinary
//
// Coerces the contained value into a byte array
//
// Arguments:
//
//	NONE

array<System::Byte>^ SqliteParameterValue::ToBinary(void)
{
	if(m_type == array<System::Byte>::typeid) return static_cast<array<System::Byte>^>(m_value);

	// SPECIAL CASES: DBNULL, BYTE, SBYTE
	if(m_value == DBNull::Value) return gcnew array<System::Byte>(0);
	if(m_type == Byte::typeid) return gcnew array<System::Byte>(1){ static_cast<System::Byte>(m_value) };
	if(m_type == SByte::typeid) return gcnew array<System::Byte>(1){ static_cast<System::Byte>(static_cast<SByte>(m_value)) };

	// SPECIAL CASES: CHAR[], STRING, GUID
	if(m_type == array<Char>::typeid) return Array::ConvertAll(static_cast<array<Char>^>(m_value), gcnew Converter<Char, System::Byte>(Convert::ToByte));
	if(m_type == String::typeid) return Encoding::Default->GetBytes(static_cast<String^>(m_value));
	if(m_type == Guid::typeid) return static_cast<Guid>(m_value).ToByteArray();

	// SPECIAL CASES: BITCONVERTER TYPES
	switch(Type::GetTypeCode(m_type)) {

		case TypeCode::Boolean:		return BitConverter::GetBytes(static_cast<bool>(m_value));
		case TypeCode::Char:		return BitConverter::GetBytes(static_cast<char>(m_value));
		case TypeCode::DateTime:	return BitConverter::GetBytes(static_cast<DateTime>(m_value).ToBinary());
		case TypeCode::Decimal:		return BitConverter::GetBytes(Decimal::ToDouble(static_cast<Decimal>(m_value)));
		case TypeCode::Double:		return BitConverter::GetBytes(static_cast<double>(m_value));
		case TypeCode::Int16:		return BitConverter::GetBytes(static_cast<short>(m_value));
		case TypeCode::Int32:		return BitConverter::GetBytes(static_cast<int>(m_value));
		case TypeCode::Int64:		return BitConverter::GetBytes(static_cast<__int64>(m_value));
		case TypeCode::Single:		return BitConverter::GetBytes(static_cast<float>(m_value));
		case TypeCode::UInt16:		return BitConverter::GetBytes(static_cast<unsigned short>(m_value));
		case TypeCode::UInt32:		return BitConverter::GetBytes(static_cast<unsigned int>(m_value));
		case TypeCode::UInt64:		return BitConverter::GetBytes(static_cast<unsigned __int64>(m_value));
	}

	// This will pretty much always throw an exception, but at least it will be consistent

	return safe_cast<array<System::Byte>^>(Convert::ChangeType(m_value, array<System::Byte>::typeid));
}

//---------------------------------------------------------------------------
// SqliteParameterValue::ToBoolean
//
// Coerces the contained value into a boolean value
//
// Arguments:
//
//	NONE

bool SqliteParameterValue::ToBoolean(void)
{
	if(m_type == bool::typeid) return static_cast<bool>(m_value);

	// SPECIAL CASES: DBNULL, BYTE[]
	if(m_value == DBNull::Value) return false;
	if(m_type == array<System::Byte>::typeid) return BitConverter::ToBoolean(static_cast<array<System::Byte>^>(m_value), 0);

	return Convert::ToBoolean(m_value);			// Standard conversion
}

//---------------------------------------------------------------------------
// SqliteParameterValue::ToDateTime
//
// Coerces the contained value into a DateTime value
//
// Arguments:
//
//	NONE

DateTime SqliteParameterValue::ToDateTime(void)
{
	if(m_type == DateTime::typeid) return static_cast<DateTime>(m_value);

	// SPECIAL CASES: DBNULL, BYTE[], STRING
	if(m_value == DBNull::Value) return DateTime(0);
	if(m_type == array<System::Byte>::typeid) return DateTime(BitConverter::ToInt64(static_cast<array<System::Byte>^>(m_value), 0));
	if(m_type == String::typeid) return DateTime::Parse(static_cast<String^>(m_value), DateTimeFormatInfo::InvariantInfo);

	return Convert::ToDateTime(m_value);		// Standard conversion
}

//---------------------------------------------------------------------------
// SqliteParameterValue::ToFloat
//
// Coerces the contained value into a floating point value
//
// Arguments:
//
//	NONE

double SqliteParameterValue::ToFloat(void)
{
	if(m_type == double::typeid) return static_cast<double>(m_value);

	// SPECIAL CASES: DBNULL, BYTE[]
	if(m_value == DBNull::Value) return 0.0;
	if(m_type == array<System::Byte>::typeid) return ByteArrayToDouble(static_cast<array<System::Byte>^>(m_value));

	return Convert::ToDouble(m_value);			// Standard conversion
}

//---------------------------------------------------------------------------
// SqliteParameterValue::ToGuid
//
// Coerces the contained value into a GUID value
//
// Arguments:
//
//	NONE

Guid SqliteParameterValue::ToGuid(void)
{
	if(m_type == Guid::typeid) return static_cast<Guid>(m_value);

	// SPECIAL CASES: DBNULL, BYTE[]
	if(m_value == DBNull::Value) return Guid::Empty;
	if(m_type == array<System::Byte>::typeid) return Guid(static_cast<array<System::Byte>^>(m_value));

	return Guid(Convert::ToString(m_value));		// Standard conversion
}

//---------------------------------------------------------------------------
// SqliteParameterValue::ToInteger
//
// Coerces the contained value into a 32 bit integer value
//
// Arguments:
//
//	NONE

int SqliteParameterValue::ToInteger(void)
{
	if(m_type == int::typeid) return static_cast<int>(m_value);

	// SPECIAL CASES: DBNULL, BYTE[]
	if(m_value == DBNull::Value) return 0;
	if(m_type == array<System::Byte>::typeid) return ByteArrayToInt32(static_cast<array<System::Byte>^>(m_value));

	return Convert::ToInt32(m_value);			// Standard conversion
}

//---------------------------------------------------------------------------
// SqliteParameterValue::ToLong
//
// Coerces the contained value into a 64 bit integer value
//
// Arguments:
//
//	NONE

__int64 SqliteParameterValue::ToLong(void)
{
	if(m_type == __int64::typeid) return static_cast<__int64>(m_value);

	// SPECIAL CASES: DBNULL, BYTE[]
	if(m_value == DBNull::Value) return 0;
	if(m_type == array<System::Byte>::typeid) return BitConverter::ToInt64(static_cast<array<System::Byte>^>(m_value), 0);

	return Convert::ToInt64(m_value);			// Standard conversion
}

//---------------------------------------------------------------------------
// SqliteParameterValue::ToString
//
// Coerces the contained value into a string
//
// Arguments:
//
//	NONE

String^ SqliteParameterValue::ToString(void)
{
	if(m_type == String::typeid) return static_cast<String^>(m_value);

	// SPECIAL CASES: DBNULL, BYTE[], CHAR[]

	if(m_value == DBNull::Value) return String::Empty;
	if(m_type == array<System::Byte>::typeid) return Encoding::Default->GetString(static_cast<array<System::Byte>^>(m_value));
	if(m_type == array<Char>::typeid) return gcnew String(static_cast<array<Char>^>(m_value));

	return Convert::ToString(m_value);			// Standard conversion
}

//---------------------------------------------------------------------------
// SqliteParameterValue::Value::set
//
// Changes the contained object instance.  All data types will be automatically
// derived every time the object changes

void SqliteParameterValue::Value::set(Object^ value)
{
	TypeCode				typeCode;		// New value's System::TypeCode

	// Whenever a NULL reference is provided, we just switcheroo it into
	// a DBNull::Value and recursively invoke ourselves ...

	if(value == nullptr) { Value = DBNull::Value; return; }

	m_value = value;						// Save the object reference
	m_type = value->GetType();				// Get the object COR type
	typeCode = Type::GetTypeCode(m_type);	// Get the System::TypeCode as well

	Debug::Assert(typeCode != TypeCode::Empty);		// Should never happen

	// Use the type code returned to determine the "best" DbType codes for this
	// particular value.  Note that more work needs to be done for TypeCode::Object

	m_providerDbType = g_typeCodeMap[static_cast<int>(typeCode)].providerDbType;
	m_genericDbType = g_typeCodeMap[static_cast<int>(typeCode)].genericDbType;

	if(typeCode != TypeCode::Object) return;		// Done if not "Object"

	// When we get back a type code of "Object", we have to do a little more
	// figuring here, since it can be any number of things that we support. If it's
	// not in this list, it will left at SqliteType::String and DbType::String

	// GUID   --> SqliteType::Guid; DbType::Guid
	else if(m_type == Guid::typeid) {

		m_providerDbType = SqliteType::Guid;
		m_genericDbType = Data::DbType::Guid;
	}

	// BYTE[] --> SqliteType::Binary; DbType::Binary
	if(m_type == array<System::Byte>::typeid) { 

		m_providerDbType = SqliteType::Binary;
		m_genericDbType = Data::DbType::Binary;
	}

	// CHAR[] --> SqliteType::String; DbType::StringFixedLength
	else if(m_type == array<Char>::typeid) {

		m_providerDbType = SqliteType::String;
		m_genericDbType = Data::DbType::StringFixedLength;
	}
}

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)
