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
#include "SqliteParameter.h"			// Include SqliteParameter declarations
#include "SqliteParameterCollection.h"	// Include SqliteParameterCollection decls

#pragma warning(push, 4)			// Enable maximum compiler warnings

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// SqliteParameter::Construct (private)
//
// Acts as a pseudo-constructor to deal with all the overloads
//
// Arguments:
//
//	name			- Parameter name
//	dataType		- Parameter SqliteType
//	size			- Size of the parameter data
//	isNullable		- Application-defined nullable flag
//	srcColumn		- Application-defined source column name
//	srcVersion		- Application-defined source DataRowVersion
//	value			- Value of the parameter

void SqliteParameter::Construct(String^ name, SqliteType dataType, int size, 
	bool isNullable, String^ srcColumn, DataRowVersion srcVersion, Object^ value)
{
	m_value = gcnew SqliteParameterValue();	// Construct the wrapper class
	m_nullMapping = false;					// Not from any argument here

	ParameterName = name;			// Save the parameter name
	IsNullable = isNullable;		// Save the nullable flag
	SourceColumn = srcColumn;		// Save the source column name
	SourceVersion = srcVersion;		// Save the source version name

	// Set the object value first, which will call ResetDbType() automatically
	// and pick something for it.  Then, if the caller indicated a specific
	// DbType to be used for this parameter, change it afterwards to match.

	Value = value;
	if(dataType != SqliteType::Auto) DbType = dataType;
	if(size > 0) Size = size;
}

//---------------------------------------------------------------------------
// SqliteParameter::DbType::set
//
// Sets the provider-specific DbType code for this parameter

void SqliteParameter::DbType::set(SqliteType value)
{
	if(m_locked) throw gcnew ParameterLockedException();

	m_providerDbType = value;
	m_genericDbType = value.ToDbType(m_value->Value);
}

//---------------------------------------------------------------------------
// SqliteParameter::Direction::set
//
// Sets the direction of this parameter.  Must be ParameterDirection::Input

void SqliteParameter::Direction::set(ParameterDirection value)
{
	if(m_locked) throw gcnew ParameterLockedException();
	if(value != ParameterDirection::Input) throw gcnew NotSupportedException();
}

//---------------------------------------------------------------------------
// SqliteParameter::GenericDbType::set
//
// Sets the generic DbType for this parameter

void SqliteParameter::GenericDbType::set(Data::DbType value)
{
	if(m_locked) throw gcnew ParameterLockedException();

	m_genericDbType = value;
	m_providerDbType = SqliteType::FromDbType(value);
}

//---------------------------------------------------------------------------
// SqliteParameter::IsNullable::set
//
// Sets a flag indicating if this parameter is nullable or not

void SqliteParameter::IsNullable::set(bool value)
{
	if(m_locked) throw gcnew ParameterLockedException();
	m_nullable = value;
}

//---------------------------------------------------------------------------
// SqliteParameter::IsUnnamed::get (internal)
//
// Determines if this is an unnamed parameter or not

bool SqliteParameter::IsUnnamed::get(void)
{
	// If the parameter name is NULL, zero-length, or set to "?", this is
	// an unnamed parameter ...

	return ((m_name == nullptr) || (m_name->Length == 0) || (String::Compare(m_name, "?") == 0));
}

//---------------------------------------------------------------------------
// SqliteParameter::ParameterName::set
//
// Sets the name assigned to this parameter

void SqliteParameter::ParameterName::set(String^ value)
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
// SqliteParameter::ResetDbType
//
// Resets the DbType value based on the actual object set as the value
//
// Arguments:
//
//	NONE

void SqliteParameter::ResetDbType(void)
{
	if(m_locked) throw gcnew ParameterLockedException();

	m_providerDbType = m_value->ProviderDbType;
	m_genericDbType = m_value->GenericDbType;
}

//---------------------------------------------------------------------------
// SqliteParameter::Size::set
//
// Sets the maximum size of the data within the column

void SqliteParameter::Size::set(int value)
{
	if(value < 0) throw gcnew ArgumentException();
	if(m_locked) throw gcnew ParameterLockedException();

	m_size = value;
}

//---------------------------------------------------------------------------
// SqliteParameter::SourceColumn::set
//
// Sets the source column name mapped in a DataSet

void SqliteParameter::SourceColumn::set(String^ value)
{
	if(m_locked) throw gcnew ParameterLockedException();
	m_srcColumn = value;
}

//---------------------------------------------------------------------------
// SqliteParameter::SourceColumnNullMapping::set
//
// Sets a flag indicating if the source column is nullable

void SqliteParameter::SourceColumnNullMapping::set(bool value)
{
	if(m_locked) throw gcnew ParameterLockedException();
	m_nullMapping = value;
}

//---------------------------------------------------------------------------
// SqliteParameter::SourceVersion::set
//
// Sets the DataRowVersion to use when loading the value. 

void SqliteParameter::SourceVersion::set(DataRowVersion value)
{
	if(m_locked) throw gcnew ParameterLockedException();
	m_srcVersion = value;
}

//---------------------------------------------------------------------------
// SqliteParameter::Value::get
//
// Gets a reference to the value set for this parameter. Note that we cannot
// return this to the caller if the parameter is locked, since they can
// still modify the contents.  This is obviously not fool-proof, as they can
// still have an outside reference to the object, but most coders are lazy
// enough that this can stop 90% of the problems

Object^ SqliteParameter::Value::get(void)
{
	if(m_locked) throw gcnew ParameterLockedException();
	return m_value->Value;
}

//---------------------------------------------------------------------------
// SqliteParameter::Value::set
//
// Sets the value of the parameter and resets the DbType based on the value

void SqliteParameter::Value::set(Object^ value)
{
	if(m_locked) throw gcnew ParameterLockedException();
	
	m_value->Value = value;				// Set the new value reference

	m_providerDbType = m_value->ProviderDbType;		// Set .DbType
	m_genericDbType = m_value->GenericDbType;		// Set .GenericDbType
	m_size = m_value->Size;							// Set .Size
}

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)
