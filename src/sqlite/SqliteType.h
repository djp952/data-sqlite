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

#ifndef __SQLITETYPE_H_
#define __SQLITETYPE_H_
#pragma once

#include "SqliteEnumerations.h"			// Include Sqlite enumeration decls

#pragma warning(push, 4)				// Enable maximum compiler warnings

using namespace System;
using namespace System::Data;

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// Class SqliteType
//
// Defines the Sqlite specific data types of a field, property or parameter.
// Also provides a means to convert to and from a standard DbType code.  I
// changed this from a simple enum to group the conversions along with it.
//---------------------------------------------------------------------------

public value class SqliteType
{
public:

	//-----------------------------------------------------------------------
	// Overloaded Operators

	bool operator ==(SqliteType rhs) { return rhs.m_code == this->m_code; }
	bool operator !=(SqliteType rhs) { return rhs.m_code != this->m_code; }

	//-----------------------------------------------------------------------
	// Member Functions

	// FromDbType
	//
	// Converts a DbType code into a SqliteType code
	static SqliteType FromDbType(DbType dbtype);

	// ToDbType
	//
	// Converts this SqliteType into a DbType. If the Object^ overload is used,
	// a "best fit" representation for the object passed will be generated
	DbType ToDbType(void) { return ToDbType(nullptr); }
	DbType ToDbType(Object^ value);

	//-----------------------------------------------------------------------
	// Fields

	static initonly SqliteType Binary		= SqliteType(SqliteTypeCode::Binary);
	static initonly SqliteType Boolean		= SqliteType(SqliteTypeCode::Boolean);
	static initonly SqliteType DateTime	= SqliteType(SqliteTypeCode::DateTime);
	static initonly SqliteType Float		= SqliteType(SqliteTypeCode::Float);
	static initonly SqliteType Guid		= SqliteType(SqliteTypeCode::Guid);
	static initonly SqliteType Integer		= SqliteType(SqliteTypeCode::Integer);
	static initonly SqliteType Long		= SqliteType(SqliteTypeCode::Long);
	static initonly SqliteType Null		= SqliteType(SqliteTypeCode::Null);
	static initonly SqliteType String		= SqliteType(SqliteTypeCode::String);

internal:

	//-----------------------------------------------------------------------
	// Internal Member Functions

	// FromType
	//
	// Converts from a Type^ reference into a SqliteType
	static SqliteType FromType(Type^ type);

	//-----------------------------------------------------------------------
	// Internal Properties

	// Value
	//
	// Gets the underlying SqliteTypeCode value
	property SqliteTypeCode Value { SqliteTypeCode get(void) { return m_code; } }

	//-----------------------------------------------------------------------
	// Internal Fields

	// Auto
	//
	// Used by SqliteParameter to indicate an auto-detect data type
	static initonly SqliteType Auto = SqliteType(SqliteTypeCode::Auto);

private:

	// PRIVATE CONSTRUCTOR
	SqliteType(SqliteTypeCode code) : m_code(code) {}

	//-----------------------------------------------------------------------
	// Member Variables
	
	initonly SqliteTypeCode		m_code;			// The underlying type code
};

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)

#endif		// __SQLITETYPE_H_
