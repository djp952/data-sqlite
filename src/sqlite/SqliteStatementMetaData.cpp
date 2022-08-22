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
#include "SqliteStatementMetaData.h"	// Include SqliteStatementMetaData declarations

#pragma warning(push, 4)			// Enable maximum compiler warnings

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// SqliteStatementMetaData Constructor
//
// Arguments:
//
//	pStatement		- Pointer to the wrapped SQLite statement handle

SqliteStatementMetaData::SqliteStatementMetaData(StatementHandle* pStatement) :
	m_pStatement(pStatement)
{
	if(!m_pStatement) throw gcnew ArgumentNullException();
	m_pStatement->AddRef(this);

	m_fields = sqlite3_column_count(m_pStatement->Handle);	// Get field count
	m_types = gcnew array<FieldTypes>(m_fields);			// Create the cache
}

//---------------------------------------------------------------------------
// SqliteStatementMetaData Finalizer

SqliteStatementMetaData::!SqliteStatementMetaData()
{
	if(m_pStatement) m_pStatement->Release(this);
	m_pStatement = NULL;
}

//---------------------------------------------------------------------------
// SqliteStatementMetaData::AddSchemaTableRow (private)
//
// Generates a single DataRow for the result set schema table
//
// Arguments:
//
//	schema		- The schema DataTable to add the new row into
//	ordinal		- Result set field ordinal to build schema info for

void SqliteStatementMetaData::AddSchemaTableRow(DataTable^ schema, int ordinal)
{
	Type^				fieldType;				// Standard .NET field data type
	String^				columnName;				// Column result set name
	String^				catalogName;			// Column catalog name
	String^				tableName;				// Column table name
	String^				schemaName;				// Column schema name
	const char*			pszColumnDataType;		// Declared column data type
	const char*			pszCollationSequence;	// Collation sequence name
	int					bNotNull;				// NOT NULL flag
	int					bPrimaryKey;			// PRIMARY KEY flag
	int					bAutoIncrement;			// AUTOINCREMENT flag
	int					nResult;				// Result from function call

	DataRow^ row = schema->NewRow();			// Construct the new table row

	fieldType = GetFieldType(ordinal);			// Column data type
	columnName = GetName(ordinal);				// Column name
	catalogName = GetCatalogName(ordinal);		// SCHEMA: Catalog name
	tableName = GetTableName(ordinal);			// SCHEMA: Table name
	schemaName = GetSchemaName(ordinal);		// SCHEMA: Column name

	// TODO TODO TODO: FINISH

	// Required columns
	row[SchemaTableColumn::ColumnName]				= columnName;
	row[SchemaTableColumn::ColumnOrdinal]			= ordinal;
	//row[SchemaTableColumn::ColumnSize]			= GetMaxSize(ordinal); ???
	//row[SchemaTableColumn::NumericPrecision]		= DBNull::Value; ???
	//row[SchemaTableColumn::NumericScale]			= DBNull::Value; ???
	//row[SchemaTableColumn::IsUnique]				= false;
	row[SchemaTableOptionalColumn::BaseCatalogName]	= catalogName;
	row[SchemaTableColumn::BaseColumnName]			= schemaName;
	row[SchemaTableColumn::BaseSchemaName]			= String::Empty;
	row[SchemaTableColumn::BaseTableName]			= tableName;
	row[SchemaTableColumn::DataType]				= fieldType;
	/* TODO (REMOVED) row[SchemaTableColumn::ProviderType]			= GetProviderSpecificDbType(ordinal).Value; */
	row[SchemaTableColumn::IsAliased]				= ((schemaName->Length > 0) && (String::Compare(columnName, schemaName, true) != 0));
	row[SchemaTableColumn::IsExpression]			= (schemaName->Length == 0);
	row[SchemaTableColumn::IsLong]					= fieldType == array<System::Byte>::typeid;
	row["DataTypeName"]								= GetDataTypeName(ordinal);

	/* TODO (REMOVED) row[SchemaTableColumn::NonVersionedProviderType]			= GetProviderSpecificDbType(ordinal).Value; */
	row[SchemaTableOptionalColumn::ProviderSpecificDataType]	= GetProviderSpecificFieldType(ordinal);

	// SQLite has added a really wonderful new function that lets us get a lot more
	// information a lot more easily. If the call fails, just leave those particular 
	// items at their DataRow defaults, or any special default established above.

	nResult = sqlite3_table_column_metadata(m_pStatement->DBHandle, AutoAnsiString(catalogName),
		AutoAnsiString(tableName), AutoAnsiString(schemaName), &pszColumnDataType,
		&pszCollationSequence, &bNotNull, &bPrimaryKey, &bAutoIncrement);
	
	if(nResult == SQLITE_OK) {

		row["DataTypeName"]								= SqliteUtil::FastPtrToStringAnsi(pszColumnDataType);
		row[SchemaTableColumn::IsKey]					= (bPrimaryKey) ? true : false;
		row[SchemaTableColumn::AllowDBNull]				= (bNotNull) ? false : true;
		row[SchemaTableOptionalColumn::IsAutoIncrement] = (bAutoIncrement) ? true : false;

		// Since this is definately a base column, we can set up some final items here
		//
		// TODO: IsUnique needs stuff from SqliteSchemaInfo
	}

	//
	// TODO: Size, precision, scale come from datatypename
	//

	schema->Rows->Add(row);				// Add the new row to the table
}

//---------------------------------------------------------------------------
// SqliteStatementMetaData::BuildSchemaTable
//
// Generates a DataTable with schema information about this result set
//
// Arguments:
//
//	NONE

DataTable^ SqliteStatementMetaData::BuildSchemaTable(void)
{
	DataTable^			schema;			// Schema table to be returned

	schema = s_template->Clone();		// Clone the template table

	// Add a row to the table for each and every field in the result set.
	// Not everything is going to be populated quite like Microsoft wants
	// it to be, but such is life.

	for(int index = 0; index < m_fields; index++) AddSchemaTableRow(schema, index);
	schema->AcceptChanges();

	return schema;						// Return the generated schema table
}

//---------------------------------------------------------------------------
// SqliteStatementMetaData::CacheFieldTypes (private)
//
// Caches off the field types for a given field by it's ordinal value
//
// Arguments:
//
//	ordinal		- Field ordinal

void SqliteStatementMetaData::CacheFieldTypes(int ordinal)
{
	String^				_decltype;			// Declared field data type
	FieldTypes			fieldTypes;			// Looked up field types	

	_decltype = GetDataTypeName(ordinal);	// Get the declared data type

	// Try to match up the declared field type with something we have planned for
	// and if we can't default it to Object (standard) and String (provider)

	if(s_declarationMapper->TryGetValue(_decltype, fieldTypes)) m_types[ordinal] = fieldTypes;
	else m_types[ordinal] = FieldTypes(Object::typeid, String::typeid);
}

//---------------------------------------------------------------------------
// SqliteStatementMetaData::FieldCount::get
//
// Gets the number of fields defined in the statement metadata

int SqliteStatementMetaData::FieldCount::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return m_fields;
}

//---------------------------------------------------------------------------
// SqliteStatementMetaData::GetCatalogName
//
// Retrieves the base catalog name for a field, if it has one
//
// Arguments:
//
//	ordinal		- Field ordinal

String^ SqliteStatementMetaData::GetCatalogName(int ordinal)
{
	CHECK_DISPOSED(m_disposed);
	if((ordinal < 0) || (ordinal >= m_fields)) throw gcnew ArgumentOutOfRangeException();

	return gcnew String(reinterpret_cast<const wchar_t*>
		(sqlite3_column_database_name16(m_pStatement->Handle, ordinal)));
}

//---------------------------------------------------------------------------
// SqliteStatementMetaData::GetDataTypeName
//
// Retrieves the declared type name of a statement field, or comes up with
// a default based on the data type of SQLite won't tell us
//
// Arguments:
//
//	ordinal		- Field ordinal

String^ SqliteStatementMetaData::GetDataTypeName(int ordinal)
{
	CHECK_DISPOSED(m_disposed);
	if((ordinal < 0) || (ordinal >= m_fields)) throw gcnew ArgumentOutOfRangeException();

	// This does not try to come up with something if there is no decltype for the field.
	// We cannot determine what the result set column type will be, since there is no 
	// guarantee that the referenced statement even has a row loaded in it right now.  This
	// class obtains SCHEMA information, not the actual result set row information

	return gcnew String(reinterpret_cast<const wchar_t*>(sqlite3_column_decltype16(m_pStatement->Handle, ordinal)));
}

//---------------------------------------------------------------------------
// SqliteStatementMetaData::GetFieldType
//
// Retrieves the standard .NET data type for the specified field
//
// Arguments:
//
//	ordinal		- Field ordinal

Type^ SqliteStatementMetaData::GetFieldType(int ordinal)
{
	CHECK_DISPOSED(m_disposed);
	if((ordinal < 0) || (ordinal >= m_fields)) throw gcnew ArgumentOutOfRangeException();

	// If the data types for this field haven't been looked up yet, do that
	// now.  Then just return whatever data type is stored in the cache array

	if(m_types[ordinal].IsNull()) CacheFieldTypes(ordinal);
	return m_types[ordinal].StandardType;
}

//---------------------------------------------------------------------------
// SqliteStatementMetaData::GetName
//
// Retrieves the name of a specific statement field by its ordinal
//
// Arguments:
//
//	ordinal		- Field ordinal

String^ SqliteStatementMetaData::GetName(int ordinal)
{
	CHECK_DISPOSED(m_disposed);
	if((ordinal < 0) || (ordinal >= m_fields)) throw gcnew ArgumentOutOfRangeException();

	// SQLite returns the expression text itself as the column name if an
	// AS clause wasn't used.  I would prefer something more like "EXPRn",
	// but without modifying the engine itself, it's seemingly hopeless

	ENGINE_ISSUE(3.3.8, "Unnamed expression columns use expression text as name");
	return gcnew String(reinterpret_cast<const wchar_t*>
		(sqlite3_column_name16(m_pStatement->Handle, ordinal)));
}

//---------------------------------------------------------------------------
// SqliteStatementMetaData::GetProviderSpecificDbType
//
// Gets the provider specific DbType code for this statement column
//
// Arguments:
//
//	ordinal		- Field ordinal

SqliteType SqliteStatementMetaData::GetProviderSpecificDbType(int ordinal)
{
	CHECK_DISPOSED(m_disposed);
	if((ordinal < 0) || (ordinal >= m_fields)) throw gcnew ArgumentOutOfRangeException();

	return SqliteType::FromType(GetProviderSpecificFieldType(ordinal));
}

//---------------------------------------------------------------------------
// SqliteStatementMetaData::GetProviderSpecificFieldType
//
// Retrieves the provider .NET data type for the specified field
//
// Arguments:
//
//	ordinal		- Field ordinal

Type^ SqliteStatementMetaData::GetProviderSpecificFieldType(int ordinal)
{
	CHECK_DISPOSED(m_disposed);
	if((ordinal < 0) || (ordinal >= m_fields)) throw gcnew ArgumentOutOfRangeException();

	// If the data types for this field haven't been looked up yet, do that
	// now.  Then just return whatever data type is stored in the cache array

	if(m_types[ordinal].IsNull()) CacheFieldTypes(ordinal);
	return m_types[ordinal].ProviderType;
}

//---------------------------------------------------------------------------
// SqliteStatementMetaData::GetSchemaName
//
// Retrieves the base schema name for a field, if it has one
//
// Arguments:
//
//	ordinal		- Field ordinal

String^ SqliteStatementMetaData::GetSchemaName(int ordinal)
{
	CHECK_DISPOSED(m_disposed);
	if((ordinal < 0) || (ordinal >= m_fields)) throw gcnew ArgumentOutOfRangeException();

	return gcnew String(reinterpret_cast<const wchar_t*>
		(sqlite3_column_origin_name16(m_pStatement->Handle, ordinal)));
}

//---------------------------------------------------------------------------
// SqliteStatementMetaData::GetTableName
//
// Retrieves the base table name for a field, if it has one
//
// Arguments:
//
//	ordinal		- Field ordinal

String^ SqliteStatementMetaData::GetTableName(int ordinal)
{
	CHECK_DISPOSED(m_disposed);
	if((ordinal < 0) || (ordinal >= m_fields)) throw gcnew ArgumentOutOfRangeException();

	return gcnew String(reinterpret_cast<const wchar_t*>
		(sqlite3_column_table_name16(m_pStatement->Handle, ordinal)));
}

//---------------------------------------------------------------------------
// SqliteStatementMetaData::StaticConstruct (private, static)
//
// Initializes the contents of the static declaration mapping collection as
// well as generates the template DataTable that is returned from GetSchema
//
// Arguments:
//
//	NONE

void SqliteStatementMetaData::StaticConstruct(void)
{
	//
	// TEMPLATE SCHEMA DATA TABLE
	//

	s_template = gcnew DataTable("SchemaTable");		// Construct the static data table

	// Required columns
	s_template->Columns->Add(SchemaTableColumn::ColumnName,					String::typeid);
	s_template->Columns->Add(SchemaTableColumn::ColumnOrdinal,				int::typeid);
	s_template->Columns->Add(SchemaTableColumn::ColumnSize,					int::typeid);
	s_template->Columns->Add(SchemaTableColumn::NumericPrecision,			short::typeid);
	s_template->Columns->Add(SchemaTableColumn::NumericScale,				short::typeid);
	s_template->Columns->Add(SchemaTableColumn::IsUnique,					bool::typeid);
	s_template->Columns->Add(SchemaTableColumn::IsKey,						bool::typeid);
	s_template->Columns->Add(SchemaTableOptionalColumn::BaseCatalogName,	String::typeid);
	s_template->Columns->Add(SchemaTableColumn::BaseColumnName,				String::typeid);
	s_template->Columns->Add(SchemaTableColumn::BaseSchemaName,				String::typeid);
	s_template->Columns->Add(SchemaTableColumn::BaseTableName,				String::typeid);
	s_template->Columns->Add(SchemaTableColumn::DataType,					Type::typeid);
	s_template->Columns->Add(SchemaTableColumn::AllowDBNull,				bool::typeid);
	s_template->Columns->Add(SchemaTableColumn::ProviderType,				int::typeid);
	s_template->Columns->Add(SchemaTableColumn::IsAliased,					bool::typeid);
	s_template->Columns->Add(SchemaTableColumn::IsExpression,				bool::typeid);
	s_template->Columns->Add(SchemaTableOptionalColumn::IsAutoIncrement,	bool::typeid);
	s_template->Columns->Add(SchemaTableColumn::IsLong,						bool::typeid);
	s_template->Columns->Add("DataTypeName",								String::typeid);
	
	s_template->Columns->Add(SchemaTableOptionalColumn::ProviderSpecificDataType,	Type::typeid);
	s_template->Columns->Add(SchemaTableColumn::NonVersionedProviderType,			int::typeid);

	// 
	// DATA TYPE DECLARATION MAPPER
	// NOTE: Any changes in here should be reflected in the metadata XML, and vice-versa
	//

	s_declarationMapper = gcnew Dictionary<String^, FieldTypes>(StringComparer::InvariantCultureIgnoreCase);
	
	//										|											|
	// DATA TYPE DECLARATION				| STANDARD .NET TYPE						| PROVIDER TYPE
	//										|											|
	s_declarationMapper["bigint"]			= FieldTypes(__int64::typeid,				__int64::typeid);
	s_declarationMapper["binary"]			= FieldTypes(array<System::Byte>::typeid,	array<System::Byte>::typeid);
	s_declarationMapper["bit"]				= FieldTypes(bool::typeid,					bool::typeid);
	s_declarationMapper["blob"]				= FieldTypes(array<System::Byte>::typeid,	array<System::Byte>::typeid);
	s_declarationMapper["bool"]				= FieldTypes(bool::typeid,					bool::typeid);
	s_declarationMapper["boolean"]			= FieldTypes(bool::typeid,					bool::typeid);
	s_declarationMapper["byte"]				= FieldTypes(Byte::typeid,					int::typeid);
	s_declarationMapper["char"]				= FieldTypes(String::typeid,				String::typeid);
	s_declarationMapper["currency"]			= FieldTypes(Decimal::typeid,				double::typeid);
	s_declarationMapper["date"]				= FieldTypes(DateTime::typeid,				DateTime::typeid);
	s_declarationMapper["datetime"]			= FieldTypes(DateTime::typeid,				DateTime::typeid);
	s_declarationMapper["decimal"]			= FieldTypes(Decimal::typeid,				double::typeid);
	s_declarationMapper["double"]			= FieldTypes(double::typeid,				double::typeid);
	s_declarationMapper["float"]			= FieldTypes(double::typeid,				double::typeid);
	s_declarationMapper["guid"]				= FieldTypes(Guid::typeid,					Guid::typeid);
	s_declarationMapper["image"]			= FieldTypes(array<System::Byte>::typeid,	array<System::Byte>::typeid);
	s_declarationMapper["int"]				= FieldTypes(int::typeid,					int::typeid);
	s_declarationMapper["int16"]			= FieldTypes(short::typeid,					int::typeid);
	s_declarationMapper["int32"]			= FieldTypes(int::typeid,					int::typeid);
	s_declarationMapper["int64"]			= FieldTypes(__int64::typeid,				__int64::typeid);
	s_declarationMapper["int8"]				= FieldTypes(SByte::typeid,					int::typeid);
	s_declarationMapper["integer"]			= FieldTypes(__int64::typeid,				__int64::typeid);
	s_declarationMapper["logical"]			= FieldTypes(bool::typeid,					bool::typeid);
	s_declarationMapper["long"]				= FieldTypes(__int64::typeid,				__int64::typeid);
	s_declarationMapper["memo"]				= FieldTypes(array<System::Byte>::typeid,	array<System::Byte>::typeid);
	s_declarationMapper["money"]			= FieldTypes(Decimal::typeid,				double::typeid);
	s_declarationMapper["nchar"]			= FieldTypes(String::typeid,				String::typeid);
	s_declarationMapper["ntext"]			= FieldTypes(String::typeid,				String::typeid);
	s_declarationMapper["number"]			= FieldTypes(Decimal::typeid,				double::typeid);
	s_declarationMapper["numeric"]			= FieldTypes(Decimal::typeid,				double::typeid);
	s_declarationMapper["nvarchar"]			= FieldTypes(String::typeid,				String::typeid);
	s_declarationMapper["object"]			= FieldTypes(Object::typeid,				array<System::Byte>::typeid);
	s_declarationMapper["real"]				= FieldTypes(float::typeid,					double::typeid);
	s_declarationMapper["single"]			= FieldTypes(float::typeid,					double::typeid);
	s_declarationMapper["smalldatetime"]	= FieldTypes(DateTime::typeid,				DateTime::typeid);
	s_declarationMapper["smallint"]			= FieldTypes(short::typeid,					int::typeid);
	s_declarationMapper["smallmoney"]		= FieldTypes(Decimal::typeid,				double::typeid);
	s_declarationMapper["string"]			= FieldTypes(String::typeid,				String::typeid);
	s_declarationMapper["text"]				= FieldTypes(String::typeid,				String::typeid);
	s_declarationMapper["time"]				= FieldTypes(DateTime::typeid,				DateTime::typeid);
	s_declarationMapper["tinyint"]			= FieldTypes(SByte::typeid,					int::typeid);
	s_declarationMapper["uniqueidentifier"]	= FieldTypes(Guid::typeid,					Guid::typeid);
	s_declarationMapper["uuid"]				= FieldTypes(Guid::typeid,					Guid::typeid);
	s_declarationMapper["varbinary"]		= FieldTypes(array<System::Byte>::typeid,	array<System::Byte>::typeid);
	s_declarationMapper["varchar"]			= FieldTypes(String::typeid,				String::typeid);
	s_declarationMapper["variant"]			= FieldTypes(Object::typeid,				array<System::Byte>::typeid);
	s_declarationMapper["xml"]				= FieldTypes(String::typeid,				String::typeid);
}

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)
