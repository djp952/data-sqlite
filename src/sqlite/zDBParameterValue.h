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

#ifndef __ZDBPARAMETERVALUE_H_
#define __ZDBPARAMETERVALUE_H_
#pragma once

#include "StatementHandle.h"			// Include StatementHandle decls
#include "zDBEnumerations.h"			// Include zDB enumeration decls
#include "zDBType.h"					// Include zDBType declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

using namespace System;
using namespace System::Data;
using namespace System::Data::Common;
using namespace System::Globalization;
using namespace System::Runtime::InteropServices;
using namespace System::Text;

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// Class zDBParameterValue (internal)
//
// zDBParameterValue encapsulates all of the logic and code needed to work
// with a specific parameter value that is contained within a zDBParameter.
//---------------------------------------------------------------------------

ref class zDBParameterValue sealed
{
public:

	zDBParameterValue() { Value = DBNull::Value; }

	//-----------------------------------------------------------------------
	// Member Functions

	// ToBinary
	//
	// Coerces the contained value into a binary value
	array<System::Byte>^ ToBinary(void);

	// ToBoolean
	//
	// Coerces the contained value into a boolean value
	bool ToBoolean(void);

	// ToDateTime
	//
	// Coerces the contained value into a date/time value
	DateTime ToDateTime(void);

	// ToFloat
	//
	// Coerces the contained value into a floating point value
	double ToFloat(void);

	// ToGuid
	//
	// Coerces the contained value into a GUID value
	Guid ToGuid(void);

	// ToInteger
	//
	// Coerces the contained value into a 32 bit integer value
	int ToInteger(void);

	// ToLong
	//
	// Coerces the contained value into a 64 bit integer value
	__int64 ToLong(void);

	// ToString
	//
	// Coerces the contained value into a string
	String^ ToString(void) new;

	//-----------------------------------------------------------------------
	// Properties

	// GenericDbType
	//
	// Gets the most appropriate generic DbType for the object
	property Data::DbType GenericDbType { Data::DbType get(void) { return m_genericDbType; } }

	// ProviderDbType
	//
	// Gets the most appropraite provider-specific zDBType for the object
	property zDBType ProviderDbType { zDBType get(void) { return m_providerDbType; } }

	// Size
	//
	// Gets the size, in bytes, of the parameter value for strings and arrays
	property int Size { int get(void); }

	// Value
	//
	// Gets or sets the contained value
	property Object^ Value 
	{ 
		Object^ get(void) { return m_value; }
		void set(Object^ value); 
	}

private:

	//-----------------------------------------------------------------------
	// Private Member Functions

	// ByteArrayToDouble
	//
	// Helper function to convert a byte array into a double
	static double ByteArrayToDouble(array<System::Byte>^ value);

	// ByteArrayToInt32
	//
	// Helper function to convert a byte array into a 32 bit integer
	static int ByteArrayToInt32(array<System::Byte>^ value);

	//-----------------------------------------------------------------------
	// Member Variables

	Object^					m_value;			// Wrapped object instance
	Type^					m_type;				// COR data type of value
	zDBType					m_providerDbType;	// Specific DbType of value
	Data::DbType			m_genericDbType;	// Generic DbType of value
};

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)

#endif		// __ZDBPARAMETERVALUE_H_
