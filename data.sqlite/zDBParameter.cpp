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
#include "zDBParameter.h"			// Include zDBParameter declarations
#include "zDBParameterCollection.h"	// Include zDBParameterCollection decls

#pragma warning(push, 4)			// Enable maximum compiler warnings

BEGIN_NAMESPACE(zuki)
BEGIN_NAMESPACE(data)
BEGIN_NAMESPACE(dbms)

//---------------------------------------------------------------------------
// zDBParameter::Construct (private)
//
// Acts as a pseudo-constructor to deal with all the overloads
//
// Arguments:
//
//	name			- Parameter name
//	dataType		- Parameter zDBType
//	size			- Size of the parameter data
//	isNullable		- Application-defined nullable flag
//	srcColumn		- Application-defined source column name
//	srcVersion		- Application-defined source DataRowVersion
//	value			- Value of the parameter

void zDBParameter::Construct(String^ name, zDBType dataType, int size, 
	bool isNullable, String^ srcColumn, DataRowVersion srcVersion, Object^ value)
{
	m_value = gcnew zDBParameterValue();	// Construct the wrapper class
	m_nullMapping = false;					// Not from any argument here

	ParameterName = name;			// Save the parameter name
	IsNullable = isNullable;		// Save the nullable flag
	SourceColumn = srcColumn;		// Save the source column name
	SourceVersion = srcVersion;		// Save the source version name

	// Set the object value first, which will call ResetDbType() automatically
	// and pick something for it.  Then, if the caller indicated a specific
	// DbType to be used for this parameter, change it afterwards to match.

	Value = value;
	if(dataType != zDBType::Auto) DbType = dataType;
	if(size > 0) Size = size;
}

//---------------------------------------------------------------------------
// zDBParameter::DbType::set
//
// Sets the provider-specific DbType code for this parameter

void zDBParameter::DbType::set(zDBType value)
{
	if(m_locked) throw gcnew ParameterLockedException();

	m_providerDbType = value;
	m_genericDbType = value.ToDbType(m_value->Value);
}

//---------------------------------------------------------------------------
// zDBParameter::Direction::set
//
// Sets the direction of this parameter.  Must be ParameterDirection::Input

void zDBParameter::Direction::set(ParameterDirection value)
{
	if(m_locked) throw gcnew ParameterLockedException();
	if(value != ParameterDirection::Input) throw gcnew NotSupportedException();
}

//---------------------------------------------------------------------------
// zDBParameter::GenericDbType::set
//
// Sets the generic DbType for this parameter

void zDBParameter::GenericDbType::set(Data::DbType value)
{
	if(m_locked) throw gcnew ParameterLockedException();

	m_genericDbType = value;
	m_providerDbType = zDBType::FromDbType(value);
}

//---------------------------------------------------------------------------
// zDBParameter::IsNullable::set
//
// Sets a flag indicating if this parameter is nullable or not

void zDBParameter::IsNullable::set(bool value)
{
	if(m_locked) throw gcnew ParameterLockedException();
	m_nullable = value;
}

//---------------------------------------------------------------------------
// zDBParameter::IsUnnamed::get (internal)
//
// Determines if this is an unnamed parameter or not

bool zDBParameter::IsUnnamed::get(void)
{
	// If the parameter name is NULL, zero-length, or set to "?", this is
	// an unnamed parameter ...

	return ((m_name == nullptr) || (m_name->Length == 0) || (String::Compare(m_name, "?") == 0));
}

//---------------------------------------------------------------------------
// zDBParameter::ParameterName::set
//
// Sets the name assigned to this parameter

void zDBParameter::ParameterName::set(String^ value)
{
	if(m_locked) throw gcnew ParameterLockedException();

	if((value != nullptr) && (value->Length > 0) && (String::Compare(value, "?"))) {

		// Make sure that the parameter name is of the proper format, and that this name 
		// does not already exist as a part of the parent parameter collection

		if((value[0] != ':') || (value->Length == 1)) throw gcnew ParameterFormatException(value);
		if((m_parent != nullptr) && m_parent->Contains(value)) throw gcnew ParameterExistsException(value);
	}

	m_name = (value != nullptr) ? value : String::Empty;	// Change the name
}

//---------------------------------------------------------------------------
// zDBParameter::ResetDbType
//
// Resets the DbType value based on the actual object set as the value
//
// Arguments:
//
//	NONE

void zDBParameter::ResetDbType(void)
{
	if(m_locked) throw gcnew ParameterLockedException();

	m_providerDbType = m_value->ProviderDbType;
	m_genericDbType = m_value->GenericDbType;
}

//---------------------------------------------------------------------------
// zDBParameter::Size::set
//
// Sets the maximum size of the data within the column

void zDBParameter::Size::set(int value)
{
	if(value < 0) throw gcnew ArgumentException();
	if(m_locked) throw gcnew ParameterLockedException();

	m_size = value;
}

//---------------------------------------------------------------------------
// zDBParameter::SourceColumn::set
//
// Sets the source column name mapped in a DataSet

void zDBParameter::SourceColumn::set(String^ value)
{
	if(m_locked) throw gcnew ParameterLockedException();
	m_srcColumn = value;
}

//---------------------------------------------------------------------------
// zDBParameter::SourceColumnNullMapping::set
//
// Sets a flag indicating if the source column is nullable

void zDBParameter::SourceColumnNullMapping::set(bool value)
{
	if(m_locked) throw gcnew ParameterLockedException();
	m_nullMapping = value;
}

//---------------------------------------------------------------------------
// zDBParameter::SourceVersion::set
//
// Sets the DataRowVersion to use when loading the value. 

void zDBParameter::SourceVersion::set(DataRowVersion value)
{
	if(m_locked) throw gcnew ParameterLockedException();
	m_srcVersion = value;
}

//---------------------------------------------------------------------------
// zDBParameter::Value::get
//
// Gets a reference to the value set for this parameter. Note that we cannot
// return this to the caller if the parameter is locked, since they can
// still modify the contents.  This is obviously not fool-proof, as they can
// still have an outside reference to the object, but most coders are lazy
// enough that this can stop 90% of the problems

Object^ zDBParameter::Value::get(void)
{
	if(m_locked) throw gcnew ParameterLockedException();
	return m_value->Value;
}

//---------------------------------------------------------------------------
// zDBParameter::Value::set
//
// Sets the value of the parameter and resets the DbType based on the value

void zDBParameter::Value::set(Object^ value)
{
	if(m_locked) throw gcnew ParameterLockedException();
	
	m_value->Value = value;				// Set the new value reference

	m_providerDbType = m_value->ProviderDbType;		// Set .DbType
	m_genericDbType = m_value->GenericDbType;		// Set .GenericDbType
	m_size = m_value->Size;							// Set .Size
}

//---------------------------------------------------------------------------

END_NAMESPACE(dbms)
END_NAMESPACE(data)
END_NAMESPACE(zuki)

#pragma warning(pop)
