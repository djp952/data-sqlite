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

#ifndef __SQLITEPARAMETER_H_
#define __SQLITEPARAMETER_H_
#pragma once

#include "SqliteEnumerations.h"			// Include Sqlite enumeration decls
#include "SqliteExceptions.h"				// Include Sqlite exception declarations
#include "SqliteParameterValue.h"			// Include SqliteParameterValue decls
#include "SqliteType.h"					// Include SqliteType declarations
#include "SqliteUtil.h"					// Include SqliteUtil declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

using namespace System;
using namespace System::Data;
using namespace System::Data::Common;
using namespace System::Text;

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// Forward Class Declarations
//---------------------------------------------------------------------------

ref class SqliteParameterCollection;		// SqliteParameterCollection.h

//---------------------------------------------------------------------------
// Class SqliteParameter
//
// Implements a single parameter value of a parameter collection
//---------------------------------------------------------------------------

public ref class SqliteParameter sealed : public DbParameter
{
public:

	//-----------------------------------------------------------------------
	// Constructors (lots and lots of them)

	// Progressive without an Object argument

	SqliteParameter()
		{ Construct(String::Empty, SqliteType::Auto, 0, false, String::Empty, DataRowVersion::Current, nullptr); }

	SqliteParameter(String^ name)
		{ Construct(name, SqliteType::Auto, 0, false, String::Empty, DataRowVersion::Current, nullptr); }

	SqliteParameter(String^ name, SqliteType dataType)
		{ Construct(name, dataType, 0, false, String::Empty, DataRowVersion::Current, nullptr); }

	SqliteParameter(String^ name, SqliteType dataType, int size)
		{ Construct(name, dataType, size, false, String::Empty, DataRowVersion::Current, nullptr); }

	SqliteParameter(String^ name, SqliteType dataType, int size, bool isNullable)
		{ Construct(name, dataType, size, isNullable, String::Empty, DataRowVersion::Current, nullptr); }

	SqliteParameter(String^ name, SqliteType dataType, int size, bool isNullable, String^ srcColumn)
		{ Construct(name, dataType, size, isNullable, srcColumn, DataRowVersion::Current, nullptr); }

	SqliteParameter(String^ name, SqliteType dataType, int size, bool isNullable, String^ srcColumn, DataRowVersion srcVersion)
		{ Construct(name, dataType, size, isNullable, srcColumn, srcVersion, nullptr); }

	// Progressive with Object argument

	SqliteParameter(Object^ value)
		{ Construct(String::Empty, SqliteType::Auto, 0, false, String::Empty, DataRowVersion::Current, value); }

	SqliteParameter(String^ name, Object^ value)
		{ Construct(name, SqliteType::Auto, 0, false, String::Empty, DataRowVersion::Current, value); }

	SqliteParameter(String^ name, SqliteType dataType, Object^ value)
		{ Construct(name, dataType, 0, false, String::Empty, DataRowVersion::Current, value); }

	SqliteParameter(String^ name, SqliteType dataType, int size, Object^ value)
		{ Construct(name, dataType, size, false, String::Empty, DataRowVersion::Current, value); }

	SqliteParameter(String^ name, SqliteType dataType, int size, bool isNullable, Object^ value)
		{ Construct(name, dataType, size, isNullable, String::Empty, DataRowVersion::Current, value); }

	SqliteParameter(String^ name, SqliteType dataType, int size, bool isNullable, String^ srcColumn, Object^ value)
		{ Construct(name, dataType, size, isNullable, srcColumn, DataRowVersion::Current, value); }

	SqliteParameter(String^ name, SqliteType dataType, int size, bool isNullable, String^ srcColumn, DataRowVersion srcVersion, Object^ value)
		{ Construct(name, dataType, size, isNullable, srcColumn, srcVersion, value); }

	// Progressive without name

	SqliteParameter(SqliteType dataType)
		{ Construct(String::Empty, dataType, 0, false, String::Empty, DataRowVersion::Current, nullptr); }

	SqliteParameter(SqliteType dataType, int size)
		{ Construct(String::Empty, dataType, size, false, String::Empty, DataRowVersion::Current, nullptr); }

	SqliteParameter(SqliteType dataType, int size, bool isNullable)
		{ Construct(String::Empty, dataType, size, isNullable, String::Empty, DataRowVersion::Current, nullptr); }

	SqliteParameter(SqliteType dataType, int size, bool isNullable, String^ srcColumn)
		{ Construct(String::Empty, dataType, size, isNullable, srcColumn, DataRowVersion::Current, nullptr); }

	SqliteParameter(SqliteType dataType, int size, bool isNullable, String^ srcColumn, DataRowVersion srcVersion)
		{ Construct(String::Empty, dataType, size, isNullable, srcColumn, srcVersion, nullptr); }
	
	// Progressive without name, but with Object argument

	SqliteParameter(SqliteType dataType, Object^ value)
		{ Construct(String::Empty, dataType, 0, false, String::Empty, DataRowVersion::Current, value); }

	SqliteParameter(SqliteType dataType, int size, Object^ value)
		{ Construct(String::Empty, dataType, size, false, String::Empty, DataRowVersion::Current, value); }

	SqliteParameter(SqliteType dataType, int size, bool isNullable, Object^ value)
		{ Construct(String::Empty, dataType, size, isNullable, String::Empty, DataRowVersion::Current, value); }

	SqliteParameter(SqliteType dataType, int size, bool isNullable, String^ srcColumn, Object^ value)
		{ Construct(String::Empty, dataType, size, isNullable, srcColumn, DataRowVersion::Current, value); }

	SqliteParameter(SqliteType dataType, int size, bool isNullable, String^ srcColumn, DataRowVersion srcVersion, Object^ value)
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
	virtual property SqliteType DbType
	{
		SqliteType get(void) new { return m_providerDbType; }
		void set(SqliteType value);
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
	// Gets or sets the reference to a parent SqliteParameterCollection instance
	property SqliteParameterCollection^ Parent
	{
		SqliteParameterCollection^ get(void) { return m_parent; }
		void set(SqliteParameterCollection^ value) { m_parent = value; }
	}

	// ValueWrapper
	//
	// Gets a reference to the SqliteParameterValue object itself
	property SqliteParameterValue^ ValueWrapper 
	{ 
		SqliteParameterValue^ get(void) { return m_value; }
	}

private:

	//-----------------------------------------------------------------------
	// Private Member Functions

	// Construct
	//
	// Acts as a pseudo-constructor to deal with all those beauties up there
	void Construct(String^ name, SqliteType dataType, int size, bool isNullable, 
		String^ srcColumn, DataRowVersion srcVersion, Object^ value);

	//-----------------------------------------------------------------------
	// Member Variables

	SqliteType						m_providerDbType;	// Provider DBTYPE code
	Data::DbType				m_genericDbType;	// Generic DBTYPE code
	String^						m_name;				// Parameter name
	bool						m_nullable;			// Nullability flag
	String^						m_srcColumn;		// Source column name
	bool						m_nullMapping;		// Source column null flag
	DataRowVersion				m_srcVersion;		// DataRowVersion flag
	SqliteParameterValue^			m_value;			// Parameter value
	int							m_size;				// Parameter size
	bool						m_locked;			// Parameter locked?
	SqliteParameterCollection^		m_parent;			// Parent collection
};

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)

#endif		// __SQLITEPARAMETER_H_
