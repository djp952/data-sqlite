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
#include "SqliteSchemaInfo.h"			// Include SqliteSchemaInfo declarations

#pragma warning(push, 4)			// Enable maximum compiler warnings
#pragma warning(disable:4100)		// "Unreferenced formal parameter"

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// sqliteschemainfo_result_callback
//
// Callback used for all of the SqliteSchemaInfo queries against the database.
// In each instance, the contents of the result set are transferred into the
// provided DataTable instance.
//
// Arguments:
//
//	context			- Context pointer passed into sqlite3_exec
//	argc			- Number of result set columns
//	argv			- Result set row values
//	azColName		- Array of column names

int sqliteschemainfo_result_callback(void *context, int argc, char* argv[], char* azColName[])
{		
	GCHandleRef<DataTable^>			table(context);		// Target DataTable instance
	gcroot<DataRow^>				row;				// The new row to add to the table

	Debug::Assert(table != nullptr);
	if(table == nullptr) return SQLITE_INTERNAL;

	try {

		row = table->NewRow();							// Construct a new DataRow

		// Iterate over all the columns in the result set and add them into the new DataRow
		// as converted ANSI strings.  All the schema table columns are of type String.
		
		for(int index = 0; index < argc; index++) 
			row->default[index] = SqliteUtil::FastPtrToStringAnsi(argv[index]);

		table->Rows->Add(row);							// Insert the new data row
	}
	
	catch(Exception^) { return SQLITE_INTERNAL; }		// SQLITE_INTERNAL on exception

	return SQLITE_OK;
}

//---------------------------------------------------------------------------
// SqliteSchemaInfo::GetCollationList (static)
//
// Generates a COLLATIONLIST schema for the specified database instance
//
// Arguments:
//
//	hDatabase		- Referenced SQLite database handle

DataTable^ SqliteSchemaInfo::GetCollationList(sqlite3* hDatabase)
{
	DataTable^				schema;			// New schema DataTable object
	GCHandle				gchandle;		// GCHandle for the DataTable
	String^					sql;			// SQL query to be executed
	int						nResult;		// Result from function call

	if(!hDatabase) throw gcnew ArgumentNullException();

	schema = s_collationList->Clone();		// Clone the existing DataTable
	gchandle = GCHandle::Alloc(schema);		// Get a GCHandle to it

	try {

		sql = "PRAGMA COLLATION_LIST";

		// Invoke SQLite to execute the generated SQL statement, passing it the GCHandle
		// to the DataTable so sqliteschema_result_callback can fill it up for us

		nResult = sqlite3_exec(hDatabase, AutoAnsiString(sql), sqliteschemainfo_result_callback,
			GCHandle::ToIntPtr(gchandle).ToPointer(), NULL);
		if(nResult != SQLITE_OK) throw gcnew SqliteException(hDatabase, nResult);
	}

	finally { gchandle.Free(); }			// Always release the GCHandle

	return schema;							
}

//---------------------------------------------------------------------------
// SqliteSchemaInfo::GetDatabaseList (static)
//
// Generates a DATABASELIST schema for the specified database instance
//
// Arguments:
//
//	hDatabase		- Referenced SQLite database handle

DataTable^ SqliteSchemaInfo::GetDatabaseList(sqlite3* hDatabase)
{
	DataTable^				schema;			// New schema DataTable object
	GCHandle				gchandle;		// GCHandle for the DataTable
	String^					sql;			// SQL query to be executed
	int						nResult;		// Result from function call

	if(!hDatabase) throw gcnew ArgumentNullException();

	schema = s_databaseList->Clone();		// Clone the existing DataTable
	gchandle = GCHandle::Alloc(schema);		// Get a GCHandle to it

	try {

		sql = "PRAGMA DATABASE_LIST";

		// Invoke SQLite to execute the generated SQL statement, passing it the GCHandle
		// to the DataTable so sqliteschema_result_callback can fill it up for us

		nResult = sqlite3_exec(hDatabase, AutoAnsiString(sql), sqliteschemainfo_result_callback,
			GCHandle::ToIntPtr(gchandle).ToPointer(), NULL);
		if(nResult != SQLITE_OK) throw gcnew SqliteException(hDatabase, nResult);
	}

	finally { gchandle.Free(); }			// Always release the GCHandle

	return schema;							
}

//---------------------------------------------------------------------------
// SqliteSchemaInfo::GetForeignKeyList (static)
//
// Generates a FOREIGNKEYLIST schema for the specified database instance
//
// Arguments:
//
//	hDatabase		- Referenced SQLite database handle
//	table			- Name of the table to get the information for

DataTable^ SqliteSchemaInfo::GetForeignKeyList(sqlite3* hDatabase, String^ table)
{
	DataTable^				schema;			// New schema DataTable object
	GCHandle				gchandle;		// GCHandle for the DataTable
	String^					sql;			// SQL query to be executed
	int						nResult;		// Result from function call

	if(!hDatabase) throw gcnew ArgumentNullException();

	schema = s_foreignKeyList->Clone();		// Clone the existing DataTable
	gchandle = GCHandle::Alloc(schema);		// Get a GCHandle to it

	try {

		sql = String::Format("PRAGMA FOREIGN_KEY_LIST('{0}')", table);

		// Invoke SQLite to execute the generated SQL statement, passing it the GCHandle
		// to the DataTable so sqliteschema_result_callback can fill it up for us

		nResult = sqlite3_exec(hDatabase, AutoAnsiString(sql), sqliteschemainfo_result_callback,
			GCHandle::ToIntPtr(gchandle).ToPointer(), NULL);
		if(nResult != SQLITE_OK) throw gcnew SqliteException(hDatabase, nResult);
	}

	finally { gchandle.Free(); }			// Always release the GCHandle

	return schema;							
}

//---------------------------------------------------------------------------
// SqliteSchemaInfo::GetIndexInfo (static)
//
// Generates an INDEXINFO schema for the specified database instance
//
// Arguments:
//
//	hDatabase		- Referenced SQLite database handle
//	index			- Name of the index to get the information for

DataTable^ SqliteSchemaInfo::GetIndexInfo(sqlite3* hDatabase, String^ index)
{
	DataTable^				schema;			// New schema DataTable object
	GCHandle				gchandle;		// GCHandle for the DataTable
	String^					sql;			// SQL query to be executed
	int						nResult;		// Result from function call

	if(!hDatabase) throw gcnew ArgumentNullException();

	schema = s_indexInfo->Clone();			// Clone the existing DataTable
	gchandle = GCHandle::Alloc(schema);		// Get a GCHandle to it

	try {

		sql = String::Format("PRAGMA INDEX_INFO('{0}')", index);

		// Invoke SQLite to execute the generated SQL statement, passing it the GCHandle
		// to the DataTable so sqliteschema_result_callback can fill it up for us

		nResult = sqlite3_exec(hDatabase, AutoAnsiString(sql), sqliteschemainfo_result_callback,
			GCHandle::ToIntPtr(gchandle).ToPointer(), NULL);
		if(nResult != SQLITE_OK) throw gcnew SqliteException(hDatabase, nResult);
	}

	finally { gchandle.Free(); }			// Always release the GCHandle

	return schema;							
}

//---------------------------------------------------------------------------
// SqliteSchemaInfo::GetIndexList (static)
//
// Generates an INDEXLIST schema for the specified database instance
//
// Arguments:
//
//	hDatabase		- Referenced SQLite database handle
//	table			- Name of the table to get the information for

DataTable^ SqliteSchemaInfo::GetIndexList(sqlite3* hDatabase, String^ table)
{
	DataTable^				schema;			// New schema DataTable object
	GCHandle				gchandle;		// GCHandle for the DataTable
	String^					sql;			// SQL query to be executed
	int						nResult;		// Result from function call

	if(!hDatabase) throw gcnew ArgumentNullException();

	schema = s_indexList->Clone();			// Clone the existing DataTable
	gchandle = GCHandle::Alloc(schema);		// Get a GCHandle to it

	try {

		sql = String::Format("PRAGMA INDEX_LIST('{0}')", table);

		// Invoke SQLite to execute the generated SQL statement, passing it the GCHandle
		// to the DataTable so sqliteschema_result_callback can fill it up for us

		nResult = sqlite3_exec(hDatabase, AutoAnsiString(sql), sqliteschemainfo_result_callback,
			GCHandle::ToIntPtr(gchandle).ToPointer(), NULL);
		if(nResult != SQLITE_OK) throw gcnew SqliteException(hDatabase, nResult);
	}

	finally { gchandle.Free(); }			// Always release the GCHandle

	return schema;							
}

//---------------------------------------------------------------------------
// SqliteSchemaInfo::GetMaster (static)
//
// Generates a MASTER schema for the specified database instance
//
// Arguments:
//
//	hDatabase		- Referenced SQLite database handle
//	type			- Optional filter for the TYPE schema column

DataTable^ SqliteSchemaInfo::GetMaster(sqlite3* hDatabase, String^ type)
{
	DataTable^				schema;			// New schema DataTable object
	GCHandle				gchandle;		// GCHandle for the DataTable
	String^					sql;			// SQL query to be executed
	int						nResult;		// Result from function call

	if(!hDatabase) throw gcnew ArgumentNullException();

	schema = s_master->Clone();				// Clone the existing DataTable
	gchandle = GCHandle::Alloc(schema);		// Get a GCHandle to it

	try {

		sql = "SELECT * FROM SQLITE_MASTER";
		if(type != nullptr) sql += String::Format(" WHERE TYPE = '{0}'", type);

		// Invoke SQLite to execute the generated SQL statement, passing it the GCHandle
		// to the DataTable so sqliteschema_result_callback can fill it up for us

		nResult = sqlite3_exec(hDatabase, AutoAnsiString(sql), sqliteschemainfo_result_callback,
			GCHandle::ToIntPtr(gchandle).ToPointer(), NULL);
		if(nResult != SQLITE_OK) throw gcnew SqliteException(hDatabase, nResult);
	}

	finally { gchandle.Free(); }			// Always release the GCHandle

	return schema;							
}

//---------------------------------------------------------------------------
// SqliteSchemaInfo::GetTableInfo (static)
//
// Generates a TABLEINFO schema for the specified database instance
//
// Arguments:
//
//	hDatabase		- Referenced SQLite database handle
//	table			- Name of the table to get the information for

DataTable^ SqliteSchemaInfo::GetTableInfo(sqlite3* hDatabase, String^ table)
{
	DataTable^				schema;			// New schema DataTable object
	GCHandle				gchandle;		// GCHandle for the DataTable
	String^					sql;			// SQL query to be executed
	int						nResult;		// Result from function call

	if(!hDatabase) throw gcnew ArgumentNullException();

	schema = s_tableInfo->Clone();			// Clone the existing DataTable
	gchandle = GCHandle::Alloc(schema);		// Get a GCHandle to it

	try {

		sql = String::Format("PRAGMA TABLE_INFO('{0}')", table);

		// Invoke SQLite to execute the generated SQL statement, passing it the GCHandle
		// to the DataTable so sqliteschema_result_callback can fill it up for us

		nResult = sqlite3_exec(hDatabase, AutoAnsiString(sql), sqliteschemainfo_result_callback,
			GCHandle::ToIntPtr(gchandle).ToPointer(), NULL);
		if(nResult != SQLITE_OK) throw gcnew SqliteException(hDatabase, nResult);
	}

	finally { gchandle.Free(); }			// Always release the GCHandle

	return schema;							
}

//---------------------------------------------------------------------------
// SqliteSchemaInfo::GetTempMaster (static)
//
// Generates a TEMPMASTER schema for the specified database instance
//
// Arguments:
//
//	hDatabase		- Referenced SQLite database handle
//	type			- Value to filter TYPE column by

DataTable^ SqliteSchemaInfo::GetTempMaster(sqlite3* hDatabase, String^ type)
{
	DataTable^				schema;			// New schema DataTable object
	GCHandle				gchandle;		// GCHandle for the DataTable
	String^					sql;			// SQL query to be executed
	int						nResult;		// Result from function call

	if(!hDatabase) throw gcnew ArgumentNullException();

	schema = s_tempMaster->Clone();			// Clone the existing DataTable
	gchandle = GCHandle::Alloc(schema);		// Get a GCHandle to it

	try {

		sql = "SELECT * FROM SQLITE_TEMP_MASTER";
		if(type != nullptr) sql += String::Format(" WHERE TYPE = '{0}'", type);

		// Invoke SQLite to execute the generated SQL statement, passing it the GCHandle
		// to the DataTable so sqliteschema_result_callback can fill it up for us

		nResult = sqlite3_exec(hDatabase, AutoAnsiString(sql), sqliteschemainfo_result_callback,
			GCHandle::ToIntPtr(gchandle).ToPointer(), NULL);
		if(nResult != SQLITE_OK) throw gcnew SqliteException(hDatabase, nResult);
	}

	finally { gchandle.Free(); }			// Always release the GCHandle

	return schema;							
}

//---------------------------------------------------------------------------
// SqliteSchemaInfo::StaticConstruct (private, static)
//
// Implements the static constructor code.  Initializes all of the static
// template DataTable instances with the required schema.  The columns are
// currently only properly documented by looking at the code in pragma.c
//
// Arguments:
//
//	NONE

void SqliteSchemaInfo::StaticConstruct(void)
{
	// COLLATIONLIST
	s_collationList = gcnew DataTable("CollationList");
	s_collationList->Columns->Add("seq", String::typeid);
	s_collationList->Columns->Add("name", String::typeid);

	// DATABASELIST
	s_databaseList = gcnew DataTable("DatabaseList");
	s_databaseList->Columns->Add("seq", String::typeid);
	s_databaseList->Columns->Add("name", String::typeid);
	s_databaseList->Columns->Add("file", String::typeid);

	// FOREIGNKEYLIST
	s_foreignKeyList = gcnew DataTable("ForeignKeyList");
	s_foreignKeyList->Columns->Add("id", String::typeid);
	s_foreignKeyList->Columns->Add("seq", String::typeid);
	s_foreignKeyList->Columns->Add("table", String::typeid);
	s_foreignKeyList->Columns->Add("from", String::typeid);
	s_foreignKeyList->Columns->Add("to", String::typeid);

	// INDEXINFO
	s_indexInfo = gcnew DataTable("IndexInfo");
	s_indexInfo->Columns->Add("seqno", String::typeid);
	s_indexInfo->Columns->Add("cid", String::typeid);
	s_indexInfo->Columns->Add("name", String::typeid);

	// INDEXLIST
	s_indexList = gcnew DataTable("IndexList");
	s_indexList->Columns->Add("seq", String::typeid);
	s_indexList->Columns->Add("name", String::typeid);
	s_indexList->Columns->Add("unique", String::typeid);

	// MASTER
	s_master = gcnew DataTable("Master");
	s_master->Columns->Add("type", String::typeid);
	s_master->Columns->Add("name", String::typeid);
	s_master->Columns->Add("tbl_name", String::typeid);
	s_master->Columns->Add("rootpage", int::typeid);
	s_master->Columns->Add("sql", String::typeid);

	// TABLEINFO
	s_tableInfo = gcnew DataTable("TableInfo");
	s_tableInfo->Columns->Add("cid", String::typeid);
	s_tableInfo->Columns->Add("name", String::typeid);
	s_tableInfo->Columns->Add("type", String::typeid);
	s_tableInfo->Columns->Add("notnull", String::typeid);
	s_tableInfo->Columns->Add("dflt_value", String::typeid);
	s_tableInfo->Columns->Add("pk", String::typeid);

	// TEMPMASTER
	s_tempMaster = gcnew DataTable("TempMaster");
	s_tempMaster->Columns->Add("type", String::typeid);
	s_tempMaster->Columns->Add("name", String::typeid);
	s_tempMaster->Columns->Add("tbl_name", String::typeid);
	s_tempMaster->Columns->Add("rootpage", int::typeid);
	s_tempMaster->Columns->Add("sql", String::typeid);
}

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)
