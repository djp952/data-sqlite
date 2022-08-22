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

#ifndef __ZDBSCHEMAINFO_H_
#define __ZDBSCHEMAINFO_H_
#pragma once

#include "AutoAnsiString.h"				// Include AutoAnsiString declarations
#include "GCHandleRef.h"				// Include GCHandleRef declarations
#include "zDBException.h"				// Include zDBException declarations
#include "zDBUtil.h"					// Include zDBUtil declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

using namespace System;
using namespace System::Data;
using namespace System::Diagnostics;
using namespace System::Runtime::InteropServices;

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// Class zDBSchemaInfo (internal, static)
//
// Defines a number of methods that retrieve PRAGMA-based schema information
// from a SQLite database.  Designed to be used outside of the normal ADO.NET
// path, in that it does everything it needs to by working directly with SQLite
//---------------------------------------------------------------------------

ref class zDBSchemaInfo abstract sealed
{
public:

	//-----------------------------------------------------------------------
	// Member Functions

	// GetCollationList
	//
	// Generates a COLLATIONLIST schema table for the specified database
	static DataTable^ GetCollationList(sqlite3* hDatabase);

	// GetDatabaseList
	//
	// Generates a DATABASELIST schema table for the specified database
	static DataTable^ GetDatabaseList(sqlite3* hDatabase);

	// GetForeignKeyList
	//
	// Generates a FOREIGNKEYLIST schema table for the specified table
	static DataTable^ GetForeignKeyList(sqlite3* hDatabase, String^ table);

	// GetIndexInfo
	//
	// Generates an INDEXINFO schema table for the specified index
	static DataTable^ GetIndexInfo(sqlite3* hDatabase, String^ index);

	// GetIndexList
	//
	// Generates an INDEXLIST schema table for the specified table
	static DataTable^ GetIndexList(sqlite3* hDatabase, String^ table);

	// GetMaster
	//
	// Generates a MASTER schema table for the specified database
	static DataTable^ GetMaster(sqlite3* hDatabase) { return GetMaster(hDatabase, nullptr); }	
	static DataTable^ GetMaster(sqlite3* hDatabase, String^ type);

	// GetTableInfo
	//
	// Generates a TABLEINFO schema table for the specified table
	static DataTable^ GetTableInfo(sqlite3* hDatabase, String^ table);

	// GetTempMaster
	//
	// Generates a TEMPMASTER schema table for the specified database
	static DataTable^ GetTempMaster(sqlite3* hDatabase) { return GetTempMaster(hDatabase, nullptr); }
	static DataTable^ GetTempMaster(sqlite3* hDatabase, String^ type);

private:

	// STATIC CONSTRUCTOR
	static zDBSchemaInfo() { StaticConstruct(); }

	//-----------------------------------------------------------------------
	// Private Member Functions

	// StaticConstruct
	//
	// Implements the meat of the static class constructor
	static void StaticConstruct(void);

	//-----------------------------------------------------------------------
	// Member Variables

	static DataTable^		s_collationList;	// Master COLLATIONLIST table
	static DataTable^		s_databaseList;		// Master DATABASELIST table
	static DataTable^		s_foreignKeyList;	// Master FOREIGNKEYLIST table
	static DataTable^		s_indexInfo;		// Master INDEXINFO table
	static DataTable^		s_indexList;		// Master INDEXLIST table
	static DataTable^		s_master;			// Master schema
	static DataTable^		s_tableInfo;		// Master TABLEINFO table
	static DataTable^		s_tempMaster;		// Temp master schema
};

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)

#endif		// __ZDBSCHEMAINFO_H_
