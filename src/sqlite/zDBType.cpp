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
#include "zDBType.h"				// Include zDBType declarations

#pragma warning(push, 4)			// Enable maximum compiler warnings

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// zDBType::FromDbType (static)
//
// Generates the most appropriate zDBType for any given DbType
//
// Arguments:
//
//	dbtype		- DbType to be converted into a zDBType

zDBType zDBType::FromDbType(DbType dbtype)
{
	switch(dbtype) {

		// BINARY: BINARY
		case DbType::Binary: return zDBType::Binary;
			
		// BOOLEAN: BOOLEAN
		case DbType::Boolean: return zDBType::Boolean;

		// DATETIME: DATE, DATETIME, TIME
		case DbType::Date:
		case DbType::DateTime:
		case DbType::Time: return zDBType::DateTime;

		// FLOAT: CURRENCY, DECIMAL, DOUBLE, SINGLE, VARNUMERIC
		case DbType::Currency:
		case DbType::Decimal:
		case DbType::Double:
		case DbType::Single:
		case DbType::VarNumeric: return zDBType::Float;

		// GUID: GUID
		case DbType::Guid: return zDBType::Guid;

		// INTEGER: SBYTE, BYTE, INT16, UINT16, INT32, UINT32
		case DbType::Byte:
		case DbType::Int16:
		case DbType::Int32:
		case DbType::SByte:
		case DbType::UInt16:
		case DbType::UInt32: return zDBType::Integer;

		// LONG: INT64, UINT64
		case DbType::Int64:
		case DbType::UInt64: return zDBType::Long;

		// STRING: EVERYTHING ELSE
		default: return zDBType::String;
	}
}

//---------------------------------------------------------------------------
// zDBType::FromType (internal)
//
// Converts from a System.Type into a zDBType.  For internal use only because
// the Type provided is expected to directly match a zDBType code
//
// Arguments:
//
//	type		- System.Type to convert into a zDBType

zDBType zDBType::FromType(Type^ type)
{
	if(type == array<System::Byte>::typeid) return zDBType::Binary;
	else if(type == bool::typeid) return zDBType::Boolean;
	else if(type == DateTime::typeid) return zDBType::DateTime;
	else if(type == double::typeid) return zDBType::Float;
	else if(type == Guid::typeid) return zDBType::Guid;
	else if(type == int::typeid) return zDBType::Integer;
	else if(type == __int64::typeid) return zDBType::Long;
	else if(type == DBNull::typeid) return zDBType::Null;
	else return zDBType::String;
}

//---------------------------------------------------------------------------
// zDBType::ToDbType
//
// Converts this zDBType into the best DbType representation possible.  The
// provided value is used to assist in the coercion
//
// Arguments:
//
//	value		- Object whose type will assist in the coercion

DbType zDBType::ToDbType(Object^ value)
{
	// Many of the reverse conversions need to know what data type the object 
	// really is to make the most accurate choice possible.  If a NULL reference
	// was passed in, set it to typeof(Object), which won't match anything

	Type^ valueType = (value != nullptr) ? value->GetType() : Object::typeid;

	switch(m_code) {

		// BINARY --> BINARY
		case zDBTypeCode::Binary: return DbType::Binary;

		// BOOLEAN --> BOOLEAN
		case zDBTypeCode::Boolean: return DbType::Boolean;

		// DATETIME --> DATETIME
		case zDBTypeCode::DateTime: return DbType::DateTime;

		// FLOAT --> DECIMAL / SINGLE / DOUBLE
		case zDBTypeCode::Float: 

			if(valueType == Decimal::typeid) return DbType::Decimal;
			else if(valueType == Single::typeid) return DbType::Single;
			else return DbType::Double;

		// GUID --> GUID
		case zDBTypeCode::Guid: return DbType::Guid;

		// INTEGER --> SBYTE / BYTE / INT16 / UINT16 / INT32 / UINT32
		case zDBTypeCode::Integer: 

			if(valueType == SByte::typeid) return DbType::SByte;
			else if(valueType == Byte::typeid) return DbType::Byte;
			else if(valueType == Int16::typeid) return DbType::Int16;
			else if(valueType == UInt16::typeid) return DbType::UInt16;
			else if(valueType == UInt32::typeid) return DbType::UInt32;
			else return DbType::Int32;

		// LONG --> INT64 / UINT64
		case zDBTypeCode::Long:

			if(valueType == UInt64::typeid) return DbType::UInt64;
			else return DbType::Int64;

		// NULL --> OBJECT
		case zDBTypeCode::Null: return DbType::Object;

		// STRING --> STRING / STRINGFIXEDLENGTH
		case zDBTypeCode::String:

			if(valueType == array<Char>::typeid) return DbType::StringFixedLength;
			else return DbType::String;
		
		default: return DbType::Object;		// <-- Technically shouldn't happen
	}
}

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)
