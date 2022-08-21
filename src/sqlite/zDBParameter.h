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

#ifndef __ZDBPARAMETER_H_
#define __ZDBPARAMETER_H_
#pragma once

#include "zDBEnumerations.h"			// Include zDB enumeration decls
#include "zDBExceptions.h"				// Include zDB exception declarations
#include "zDBParameterValue.h"			// Include zDBParameterValue decls
#include "zDBType.h"					// Include zDBType declarations
#include "zDBUtil.h"					// Include zDBUtil declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

using namespace System;
using namespace System::Data;
using namespace System::Data::Common;
using namespace System::Text;

BEGIN_NAMESPACE(zuki)
BEGIN_NAMESPACE(data)
BEGIN_NAMESPACE(dbms)

//---------------------------------------------------------------------------
// Forward Class Declarations
//---------------------------------------------------------------------------

ref class zDBParameterCollection;		// zDBParameterCollection.h

//---------------------------------------------------------------------------
// Class zDBParameter
//
// Implements a single parameter value of a parameter collection
//---------------------------------------------------------------------------

public ref class zDBParameter sealed : public DbParameter
{
public:

	//-----------------------------------------------------------------------
	// Constructors (lots and lots of them)

	// Progressive without an Object argument

	zDBParameter()
		{ Construct(String::Empty, zDBType::Auto, 0, false, String::Empty, DataRowVersion::Current, nullptr); }

	zDBParameter(String^ name)
		{ Construct(name, zDBType::Auto, 0, false, String::Empty, DataRowVersion::Current, nullptr); }

	zDBParameter(String^ name, zDBType dataType)
		{ Construct(name, dataType, 0, false, String::Empty, DataRowVersion::Current, nullptr); }

	zDBParameter(String^ name, zDBType dataType, int size)
		{ Construct(name, dataType, size, false, String::Empty, DataRowVersion::Current, nullptr); }

	zDBParameter(String^ name, zDBType dataType, int size, bool isNullable)
		{ Construct(name, dataType, size, isNullable, String::Empty, DataRowVersion::Current, nullptr); }

	zDBParameter(String^ name, zDBType dataType, int size, bool isNullable, String^ srcColumn)
		{ Construct(name, dataType, size, isNullable, srcColumn, DataRowVersion::Current, nullptr); }

	zDBParameter(String^ name, zDBType dataType, int size, bool isNullable, String^ srcColumn, DataRowVersion srcVersion)
		{ Construct(name, dataType, size, isNullable, srcColumn, srcVersion, nullptr); }

	// Progressive with Object argument

	zDBParameter(Object^ value)
		{ Construct(String::Empty, zDBType::Auto, 0, false, String::Empty, DataRowVersion::Current, value); }

	zDBParameter(String^ name, Object^ value)
		{ Construct(name, zDBType::Auto, 0, false, String::Empty, DataRowVersion::Current, value); }

	zDBParameter(String^ name, zDBType dataType, Object^ value)
		{ Construct(name, dataType, 0, false, String::Empty, DataRowVersion::Current, value); }

	zDBParameter(String^ name, zDBType dataType, int size, Object^ value)
		{ Construct(name, dataType, size, false, String::Empty, DataRowVersion::Current, value); }

	zDBParameter(String^ name, zDBType dataType, int size, bool isNullable, Object^ value)
		{ Construct(name, dataType, size, isNullable, String::Empty, DataRowVersion::Current, value); }

	zDBParameter(String^ name, zDBType dataType, int size, bool isNullable, String^ srcColumn, Object^ value)
		{ Construct(name, dataType, size, isNullable, srcColumn, DataRowVersion::Current, value); }

	zDBParameter(String^ name, zDBType dataType, int size, bool isNullable, String^ srcColumn, DataRowVersion srcVersion, Object^ value)
		{ Construct(name, dataType, size, isNullable, srcColumn, srcVersion, value); }

	// Progressive without name

	zDBParameter(zDBType dataType)
		{ Construct(String::Empty, dataType, 0, false, String::Empty, DataRowVersion::Current, nullptr); }

	zDBParameter(zDBType dataType, int size)
		{ Construct(String::Empty, dataType, size, false, String::Empty, DataRowVersion::Current, nullptr); }

	zDBParameter(zDBType dataType, int size, bool isNullable)
		{ Construct(String::Empty, dataType, size, isNullable, String::Empty, DataRowVersion::Current, nullptr); }

	zDBParameter(zDBType dataType, int size, bool isNullable, String^ srcColumn)
		{ Construct(String::Empty, dataType, size, isNullable, srcColumn, DataRowVersion::Current, nullptr); }

	zDBParameter(zDBType dataType, int size, bool isNullable, String^ srcColumn, DataRowVersion srcVersion)
		{ Construct(String::Empty, dataType, size, isNullable, srcColumn, srcVersion, nullptr); }
	
	// Progressive without name, but with Object argument

	zDBParameter(zDBType dataType, Object^ value)
		{ Construct(String::Empty, dataType, 0, false, String::Empty, DataRowVersion::Current, value); }

	zDBParameter(zDBType dataType, int size, Object^ value)
		{ Construct(String::Empty, dataType, size, false, String::Empty, DataRowVersion::Current, value); }

	zDBParameter(zDBType dataType, int size, bool isNullable, Object^ value)
		{ Construct(String::Empty, dataType, size, isNullable, String::Empty, DataRowVersion::Current, value); }

	zDBParameter(zDBType dataType, int size, bool isNullable, String^ srcColumn, Object^ value)
		{ Construct(String::Empty, dataType, size, isNullable, srcColumn, DataRowVersion::Current, value); }

	zDBParameter(zDBType dataType, int size, bool isNullable, String^ srcColumn, DataRowVersion srcVersion, Object^ value)
		{ Construct(String::Empty, dataType, size, isNullable, srcColumn, srcVersion, value); }

	//-----------------------------------------------------------------------
	// Member Functions

	// ResetDbType (DbParameter)
	//
	// Resets .DbType and .GenericDbType to their original values
	virtual void ResetDbType(void) override;

	//-----------------------------------------------------------------------
	// Properties

	// DbType
	//
	// Gets or sets the provider specific DbType of this parameter
	virtual property zDBType DbType
	{
		zDBType get(void) new { return m_providerDbType; }
		void set(zDBType value);
	}

	// Direction (DbParameter)
	//
	// Gets or sets the direction (input-only et al) of the parameter
	virtual property ParameterDirection Direction
	{
		ParameterDirection get(void) override { return ParameterDirection::Input; }
		void set(ParameterDirection value) override;
	}

	// GenericDbType (DbParameter.DbType)
	//
	// Gets or sets the DbType of this parameter
	virtual property Data::DbType GenericDbType
	{
		Data::DbType get(void) = DbParameter::DbType::get { return m_genericDbType; }
		void set(Data::DbType value) = DbParameter::DbType::set;
	}

	// IsNullable (DbParameter)
	//
	// Gets or sets a flag if this parameter is nullable or not
	virtual property bool IsNullable
	{
		bool get(void) override { return m_nullable; }
		void set(bool value) override;
	}

	// ParameterName (DbParameter)
	//
	// Gets or sets the parameter name
	virtual property String^ ParameterName
	{
		String^ get(void) override { return m_name; }
		void set(String^ value) override;
	}

	// Size (DbParameter)
	//
	// Gets or sets the maximum size of the data in the column
	virtual property int Size
	{
		int get(void) override { return m_size; }
		void set(int value) override;
	}

	// SourceColumn (DbParameter)
	//
	// Gets or sets the name of the source column mapped to a DataSet
	virtual property String^ SourceColumn
	{
		String^ get(void) override { return (m_srcColumn != nullptr) ? m_srcColumn : String::Empty; }
		void set(String^ value) override;
	}

	// SourceColumnNullMapping (DbParameter)
	//
	// Gets or sets a flag indicating if the source column is nullable
	virtual property bool SourceColumnNullMapping
	{
		bool get(void) override { return m_nullMapping; }
		void set(bool value) override;
	}

	// SourceVersion (DbParameter)
	//
	// Gets or sets the DataRowVersion to use with the parameter
	virtual property DataRowVersion SourceVersion
	{
		DataRowVersion get(void) override { return m_srcVersion; }
		void set(DataRowVersion value) override;
	}

	// ToString (Object)
	//
	// Overrides the default .ToString implementation
	virtual String^ ToString(void) override { return ParameterName; }

	// Value
	//
	// Gets or sets the value of this parameter
	virtual property Object^ Value
	{
		Object^ get(void) override;
		void set(Object^ value) override;
	}

internal:

	//-----------------------------------------------------------------------
	// Internal Properties

	// IsUnnamed
	//
	// Determines if this is an unnnamed parameter or not
	property bool IsUnnamed { bool get(void); }

	// Locked
	//
	// Gets or sets a flag indicating that this property is locked and should
	// not allow the application to change anything about it
	property bool Locked
	{
		bool get(void) { return m_locked; }
		void set(bool value) { m_locked = value; }
	}

	// Parent
	//
	// Gets or sets the reference to a parent zDBParameterCollection instance
	property zDBParameterCollection^ Parent
	{
		zDBParameterCollection^ get(void) { return m_parent; }
		void set(zDBParameterCollection^ value) { m_parent = value; }
	}

	// ValueWrapper
	//
	// Gets a reference to the zDBParameterValue object itself
	property zDBParameterValue^ ValueWrapper 
	{ 
		zDBParameterValue^ get(void) { return m_value; }
	}

private:

	//-----------------------------------------------------------------------
	// Private Member Functions

	// Construct
	//
	// Acts as a pseudo-constructor to deal with all those beauties up there
	void Construct(String^ name, zDBType dataType, int size, bool isNullable, 
		String^ srcColumn, DataRowVersion srcVersion, Object^ value);

	//-----------------------------------------------------------------------
	// Member Variables

	zDBType						m_providerDbType;	// Provider DBTYPE code
	Data::DbType				m_genericDbType;	// Generic DBTYPE code
	String^						m_name;				// Parameter name
	bool						m_nullable;			// Nullability flag
	String^						m_srcColumn;		// Source column name
	bool						m_nullMapping;		// Source column null flag
	DataRowVersion				m_srcVersion;		// DataRowVersion flag
	zDBParameterValue^			m_value;			// Parameter value
	int							m_size;				// Parameter size
	bool						m_locked;			// Parameter locked?
	zDBParameterCollection^		m_parent;			// Parent collection
};

//---------------------------------------------------------------------------

END_NAMESPACE(dbms)
END_NAMESPACE(data)
END_NAMESPACE(zuki)

#pragma warning(pop)

#endif		// __ZDBPARAMETER_H_
