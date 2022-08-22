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

#ifndef __ZDBSTATEMENTMETADATA_H_
#define __ZDBSTATEMENTMETADATA_H_
#pragma once

#include "AutoAnsiString.h"				// Include AutoAnsiString declarations
#include "StatementHandle.h"			// Include StatementHandle declarations
#include "zDBSchemaInfo.h"				// Include zDBSchemaInfo declarations
#include "zDBType.h"					// Include zDBType declarations
#include "zDBUtil.h"					// Include zDBUtil declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Data;
using namespace System::Runtime::InteropServices;

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// Class zDBStatementMetaData (internal)
//
// Creates and maintains metadata for a zDBStatement instance.  Broken out
// for clarity, since it's a lot of very dry ugly code
//---------------------------------------------------------------------------

ref class zDBStatementMetaData sealed
{
public:

	// Constructor
	//
	// Accepts a pointer to a StatementHandle wrapper class
	zDBStatementMetaData(StatementHandle* pStatement);

	//-----------------------------------------------------------------------
	// Member Functions

	// BuildSchemaTable
	//
	// Generates a DataTable containing result set schema information
	DataTable^ BuildSchemaTable(void);

	// GetCatalog
	//
	// Gets the catalog name for a statement column, if available
	String^ GetCatalogName(int ordinal);

	// GetName
	//
	// Gets the name associated with a statement column
	String^ GetName(int ordinal);

	// GetDataTypeName
	//
	// Gets the declared type name of the statement column
	String^ GetDataTypeName(int ordinal);

	// GetFieldType
	//
	// Gets the standard .NET data type for the specified column
	Type^ GetFieldType(int ordinal);

	// GetProviderSpecificDbType
	//
	// Gets the provider specific DbType for the specified column
	zDBType GetProviderSpecificDbType(int ordinal);

	// GetProviderSpecificFieldType
	//
	// Gets the provider specific data type for the specified column
	Type^ GetProviderSpecificFieldType(int ordinal);

	// GetSchemaName
	//
	// Gets the base schema name for a statement column, if available
	String^ GetSchemaName(int ordinal);

	// GetTableName
	//
	// Gets the base table name for a statement column, if available
	String^ GetTableName(int ordinal);

	//-----------------------------------------------------------------------
	// Properties

	// FieldCount
	//
	// Retrieves the number of field defined by the statement
	property int FieldCount { int get(void); }

private:

	// STATIC CONSTRUCTOR
	static zDBStatementMetaData() { StaticConstruct(); }

	// DESTRUCTOR / FINALIZER
	~zDBStatementMetaData() { this->!zDBStatementMetaData(); m_disposed = true; }
	!zDBStatementMetaData();

	//-----------------------------------------------------------------------
	// Private Data Types

	// FieldTypes
	//
	// Defines the standard .NET and provider-specific Type^ for a field
	value struct FieldTypes
	{
		FieldTypes(Type^ standardType, Type^ providerType) :
			StandardType(standardType), ProviderType(providerType) {}

		bool IsNull(void) { return ((StandardType == nullptr) || (ProviderType == nullptr)); }

		initonly Type^		StandardType;			// Standard type object
		initonly Type^		ProviderType;			// Provider type object
	};

	//-----------------------------------------------------------------------
	// Private Member Functions

	// AddSchemaTableRow
	//
	// Generates a single DataRow for the schema DataTable
	void AddSchemaTableRow(DataTable^ schema, int ordinal);

	// CacheFieldTypes
	//
	// Caches the data types for a field via it's ordinal
	void CacheFieldTypes(int ordinal);

	// StaticConstruct
	//
	// Called by the static constructor to initialize the declaration mapper
	static void StaticConstruct(void);

	//-----------------------------------------------------------------------
	// Member Variables
	
	bool					m_disposed;		// Object disposal flag
	StatementHandle*		m_pStatement;	// SQLite statement handle
	int						m_fields;		// Number of statement fields
	array<FieldTypes>^		m_types;		// Instance type information

	static Dictionary<String^, FieldTypes>^ s_declarationMapper;
	static DataTable^						s_template;
};

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)

#endif		// __ZDBSTATEMENTMETADATA_H_
