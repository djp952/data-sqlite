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

#ifndef __ZDBTYPE_H_
#define __ZDBTYPE_H_
#pragma once

#include "zDBEnumerations.h"			// Include zDB enumeration decls

#pragma warning(push, 4)				// Enable maximum compiler warnings

using namespace System;
using namespace System::Data;

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// Class zDBType
//
// Defines the zDB specific data types of a field, property or parameter.
// Also provides a means to convert to and from a standard DbType code.  I
// changed this from a simple enum to group the conversions along with it.
//---------------------------------------------------------------------------

public value class zDBType
{
public:

	//-----------------------------------------------------------------------
	// Overloaded Operators

	bool operator ==(zDBType rhs) { return rhs.m_code == this->m_code; }
	bool operator !=(zDBType rhs) { return rhs.m_code != this->m_code; }

	//-----------------------------------------------------------------------
	// Member Functions

	// FromDbType
	//
	// Converts a DbType code into a zDBType code
	static zDBType FromDbType(DbType dbtype);

	// ToDbType
	//
	// Converts this zDBType into a DbType. If the Object^ overload is used,
	// a "best fit" representation for the object passed will be generated
	DbType ToDbType(void) { return ToDbType(nullptr); }
	DbType ToDbType(Object^ value);

	//-----------------------------------------------------------------------
	// Fields

	static initonly zDBType Binary		= zDBType(zDBTypeCode::Binary);
	static initonly zDBType Boolean		= zDBType(zDBTypeCode::Boolean);
	static initonly zDBType DateTime	= zDBType(zDBTypeCode::DateTime);
	static initonly zDBType Float		= zDBType(zDBTypeCode::Float);
	static initonly zDBType Guid		= zDBType(zDBTypeCode::Guid);
	static initonly zDBType Integer		= zDBType(zDBTypeCode::Integer);
	static initonly zDBType Long		= zDBType(zDBTypeCode::Long);
	static initonly zDBType Null		= zDBType(zDBTypeCode::Null);
	static initonly zDBType String		= zDBType(zDBTypeCode::String);

internal:

	//-----------------------------------------------------------------------
	// Internal Member Functions

	// FromType
	//
	// Converts from a Type^ reference into a zDBType
	static zDBType FromType(Type^ type);

	//-----------------------------------------------------------------------
	// Internal Properties

	// Value
	//
	// Gets the underlying zDBTypeCode value
	property zDBTypeCode Value { zDBTypeCode get(void) { return m_code; } }

	//-----------------------------------------------------------------------
	// Internal Fields

	// Auto
	//
	// Used by zDBParameter to indicate an auto-detect data type
	static initonly zDBType Auto = zDBType(zDBTypeCode::Auto);

private:

	// PRIVATE CONSTRUCTOR
	zDBType(zDBTypeCode code) : m_code(code) {}

	//-----------------------------------------------------------------------
	// Member Variables
	
	initonly zDBTypeCode		m_code;			// The underlying type code
};

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)

#endif		// __ZDBTYPE_H_
