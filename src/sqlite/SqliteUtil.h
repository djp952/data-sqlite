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

#ifndef __SQLITEUTIL_H_
#define __SQLITEUTIL_H_
#pragma once

#include "AutoAnsiString.h"				// Include AutoAnsiString declarations
#include "AutoGCHandle.h"				// Include AutoGCHandle declarations
#include "GCHandleRef.h"				// Include GCHandleRef declarations
#include "SqliteConstants.h"				// Include Sqlite constant declarations
#include "SqliteEnumerations.h"			// Include Sqlite enumeration declarations
#include "SqliteExceptions.h"				// Include Sqlite exception declarations
#include "SqliteType.h"					// Include SqliteType declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

using namespace System;
using namespace System::ComponentModel;
using namespace System::Data;
using namespace System::Diagnostics;
using namespace System::Globalization;
using namespace System::IO;

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// Forward Class Declarations
//---------------------------------------------------------------------------

ref class SqliteConnection;					// SqliteConnection.h
ref class SqliteParameter;						// SqliteParameter.h
ref class SqliteStatement;						// SqliteStatement.h

//---------------------------------------------------------------------------
// Class SqliteUtil (static, internal)
//
// Implements a series of generally useful static methods used here and
// there and everywhere in this library.
//---------------------------------------------------------------------------

ref class SqliteUtil abstract sealed
{
public:

	// SqliteConnection State Helpers
	//
	// Throws an exception if the required connection state is not valid
	static void	CheckConnectionClosed(SqliteConnection^ conn);
	static void	CheckConnectionOpen(SqliteConnection^ conn);
	static void	CheckConnectionReady(SqliteConnection^ conn);
	static void CheckConnectionValid(SqliteConnection^ conn);

	// IDataRecord helpers
	static void CheckDataRecordOrdinal(IDataRecord^ record, int ordinal);

	// Simple SQL query processors
	//
	// Used internally for simple stuff like PRAGMAs and whatnot
	static void		ExecuteNonQuery(sqlite3* hDatabase, String^ query);
	static String^	ExecuteScalar(sqlite3* hDatabase, String^ query);

	static String^	FastPtrToStringAnsi(const char* psz);
	static String^	FastPtrToStringAnsi(const char* psz, size_t cch);

	// SqliteTextEncodingMode <--> PRAGMA ENCODING
	static String^				EncodingToPragma(SqliteTextEncodingMode encoding);
	static SqliteTextEncodingMode  PragmaToEncoding(String^ pragma);

	// Data Validation (nothrow)
	static bool	ValidateDataSource(String^ dataSource);
	static bool	ValidateFileName(String^ path);
	static bool	ValidateFolderName(String^ path);

internal:

	//-----------------------------------------------------------------------
	// Internal Data Types

	// ScalarResultObject
	//
	// Used to pass a scalar string value back from an ExecuteScalar operation
	ref struct ScalarResultObject sealed
	{
		ScalarResultObject() : Result(nullptr) {}
		
		String^	Result;		// Result from the scalar SQL query as a string
	};
};

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)

#endif		// __SQLITEUTIL_H_
