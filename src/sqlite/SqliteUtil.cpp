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
#include "SqliteUtil.h"				// Include SqliteUtil declarations
#include "SqliteConnection.h"			// Include SqliteConnection declarations
#include "SqliteParameter.h"			// Include SqliteParameter declarations
#include "SqliteStatement.h"			// Include SqliteStatement declarations

#pragma warning(push, 4)			// Enable maximum compiler warnings
#pragma warning(disable:4100)		// "unreferenced formal parameter"

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// sqliteutil_scalar_callback
//
// Callback used for SqliteUtil::ExecuteScalar
//
// Arguments:
//
//	context			- Context pointer passed into sqlite3_exec
//	argc			- Number of result set columns
//	argv			- Result set row values
//	azColName		- Array of column names

int sqliteutil_scalar_callback(void *context, int argc, char* argv[], char* azColName[])
{
	GCHandleRef<SqliteUtil::ScalarResultObject^> contextObject(context);

	Debug::Assert(contextObject != nullptr);
	if(contextObject == nullptr) return SQLITE_INTERNAL;

	// If there is at least one column in the results, and the first one isn't
	// NULL, allocate and convert a new managed String representing that value.
	// Don't use FastPtrToAnsiString since we can't really know how big the result
	// is going to end up being here.  99.9% sure there's nothing bad, but why risk it

	if(argc && argv[0]) contextObject->Result = Marshal::PtrToStringAnsi(IntPtr(argv[0]));
	return SQLITE_OK;
}

//---------------------------------------------------------------------------
// SqliteUtil::CheckConnectionClosed (static)
//
// Determines if the SqliteConnection instance is "closed", and throws an 
// exception if it isn't
//
// Arguments:
//
//	conn		- SqliteConnection instance to be checked

void SqliteUtil::CheckConnectionClosed(SqliteConnection^ conn)
{
	CheckConnectionValid(conn);

	if(conn->State == ConnectionState::Closed) return;
	throw gcnew ConnectionOpenException();
}

//---------------------------------------------------------------------------
// SqliteUtil::CheckConnectionOpen (static)
//
// Determines if the SqliteConnection instance is "open", and throws an 
// exception if it isn't
//
// Arguments:
//
//	conn		- SqliteConnection instance to be checked

void SqliteUtil::CheckConnectionOpen(SqliteConnection^ conn)
{
	CheckConnectionValid(conn);

	if(conn->State == ConnectionState::Open) return;
	if(conn->State == ConnectionState::Executing) return;
	if(conn->State == ConnectionState::Fetching) return;
	throw gcnew ConnectionClosedException();
}

//---------------------------------------------------------------------------
// SqliteUtil::CheckConnectionReady (static)
//
// Determines if a SqliteConnection is in a "ready" state, meaning it can
// accept and execute a new SQL query
//
// Arguments:
//
//	conn		- SqliteConnection instance to be checked

void SqliteUtil::CheckConnectionReady(SqliteConnection^ conn)
{
	CheckConnectionValid(conn);

	// If a rollback is in progress, further commands cannot be executed

	if(conn->RollbackInProgress) throw gcnew ConnectionRollbackException();

	if(conn->State == ConnectionState::Open) return;

	// We want to throw a different exception back up if the connection is
	// actually closed or just in a busy state of some kind

	if(conn->State == ConnectionState::Closed) throw gcnew ConnectionClosedException();
	else throw gcnew ConnectionBusyException();
}

//---------------------------------------------------------------------------
// SqliteUtil::CheckConnectionValid (static)
//
// Determines if a SqliteConnection is valid or not.  A null pointer, a disposed
// connection object, or a NULL database handle count as "invalid"
//
// Arguments:
//
//	conn		- SqliteConnection instance to be checked

void SqliteUtil::CheckConnectionValid(SqliteConnection^ conn)
{
	if(conn == nullptr) throw gcnew ConnectionNullException();
	if(conn->IsDisposed()) throw gcnew ConnectionDisposedException();
}

//---------------------------------------------------------------------------
// SqliteUtil::CheckDataRecordOrdinal (static)
//
// Tests an ordinal used against an IDataRecord and throws an appropriate
// exception if it's out of bounds
//
// Arguments:
//
//	record		- IDataRecord-based object reference
//	ordinal		- Ordinal to be tested for validity

void SqliteUtil::CheckDataRecordOrdinal(IDataRecord^ record, int ordinal)
{
	Debug::Assert(record != nullptr);

	if((ordinal >= 0) && (ordinal < record->FieldCount)) return;
	throw gcnew IndexOutOfRangeException();
}

////---------------------------------------------------------------------------
//// SqliteUtil::ColumnDefinitionToType (static)
////
//// Attempts to map a column definition's data type to an appropriate .NET
//// data type.  Not what you would call an exact science, either
////
//// Arguments:
////
////	definition			- The column-def's data type string
//
//Type^ SqliteUtil::ColumnDefinitionToType(String^ definition)
//{
//	PinnedStringPtr			pinDefinition;		// Pinned definition string
//
//	// A NULL string or an empty string cannot be determined, so don't waste time
//	// and just throw back a generic System.Object type
//
//	if((definition == nullptr) || (definition->Length == 0)) return Object::typeid;
//
//	definition = definition->ToLower();				// Convert to lower case
//	pinDefinition = PtrToStringChars(definition);	// Pin down the buffer
//	
//	// Let the games begin ... this is "quite inefficient", but I bet that
//	// sticking with raw C strings and wcsstr is better than String::IndexOf ...
//
//	// STRING
//	if(wcsstr(pinDefinition, L"text"))				return String::typeid;
//	if(wcsstr(pinDefinition, L"char"))				return String::typeid;
//	if(wcsstr(pinDefinition, L"string"))			return String::typeid;
//	if(wcsstr(pinDefinition, L"memo"))				return String::typeid;
//	if(wcsstr(pinDefinition, L"note"))				return String::typeid;
//	if(wcsstr(pinDefinition, L"xml"))				return String::typeid;
//
//	// INTEGER
//	// 
//	// Note that "INTEGER" and "LONG" are exact matches, but everything
//	// else is a contains situation.  i.e. "INT" ends up catching "INT32"
//
//	if(wcscmp(pinDefinition, L"integer") == 0)		return __int64::typeid;
//	if(wcscmp(pinDefinition, L"long") == 0)			return __int64::typeid;
//	if(wcsstr(pinDefinition, L"bigint"))			return __int64::typeid;
//	if(wcsstr(pinDefinition, L"int64"))				return __int64::typeid;
//	if(wcsstr(pinDefinition, L"smallint"))			return short::typeid;
//	if(wcsstr(pinDefinition, L"int16"))				return short::typeid;
//	if(wcsstr(pinDefinition, L"tinyint"))			return SByte::typeid;		
//	if(wcsstr(pinDefinition, L"int8"))				return SByte::typeid;
//	if(wcsstr(pinDefinition, L"int"))				return int::typeid;
//
//	// FLOATING POINT
//	if(wcsstr(pinDefinition, L"float"))				return double::typeid;
//	if(wcsstr(pinDefinition, L"double"))			return double::typeid;
//	if(wcsstr(pinDefinition, L"real"))				return float::typeid;
//	if(wcsstr(pinDefinition, L"single"))			return float::typeid;
//	
//	// DECIMAL
//	if(wcsstr(pinDefinition, L"number"))			return Decimal::typeid;
//	if(wcsstr(pinDefinition, L"numeric"))			return Decimal::typeid;
//	if(wcsstr(pinDefinition, L"decimal"))			return Decimal::typeid;
//	if(wcsstr(pinDefinition, L"money"))				return Decimal::typeid;
//	if(wcsstr(pinDefinition, L"currency"))			return Decimal::typeid;
//
//	// BINARY
//	if(wcsstr(pinDefinition, L"blob"))				return array<Byte>::typeid;
//	if(wcsstr(pinDefinition, L"binary"))			return array<Byte>::typeid;
//	if(wcsstr(pinDefinition, L"image"))				return array<Byte>::typeid;
//
//	// BOOLEAN
//	if(wcsstr(pinDefinition, L"bit"))				return Boolean::typeid;
//	if(wcsstr(pinDefinition, L"bool"))				return Boolean::typeid;
//	if(wcsstr(pinDefinition, L"logic"))				return Boolean::typeid;
//
//	// DATETIME
//	if(wcsstr(pinDefinition, L"date"))				return DateTime::typeid;
//	if(wcsstr(pinDefinition, L"time"))				return DateTime::typeid;
//
//	// GUID
//	if(wcsstr(pinDefinition, L"uniqueidentifier"))	return Guid::typeid;
//	if(wcsstr(pinDefinition, L"guid"))				return Guid::typeid;
//	if(wcsstr(pinDefinition, L"uuid"))				return Guid::typeid;
//
//	// BYTE
//	if(wcsstr(pinDefinition, L"byte"))				return Byte::typeid;
//
//	// NULL
//	if(wcsstr(pinDefinition, L"null"))				return DBNull::typeid;
//
//	// EVERYTHING ELSE
//	//
//	// "OBJECT" and "VARIANT" are the only specific cases based on the
//	// list of data types we expose via the XML metadata, but anything 
//	// we cannot parse ends up as a generic System.Object
//
//	return Object::typeid;
//}

//---------------------------------------------------------------------------
// SqliteUtil::EncodingToPragma (static)
//
// Converts a SqliteTextEncodingMode value into a string that can be used in
// a SQLite PRAGMA ENCODING statement
//
// Arguments:
//
//	encoding	- The encoding enumeration value to convert

String^ SqliteUtil::EncodingToPragma(SqliteTextEncodingMode encoding)
{
	switch(encoding) {

		case SqliteTextEncodingMode::UTF8: return "UTF-8";
		case SqliteTextEncodingMode::UTF16: return "UTF-16";
		case SqliteTextEncodingMode::UTF16LittleEndian: return "UTF-16le";
		case SqliteTextEncodingMode::UTF16BigEndian: return "UTF-16be";
	}

	throw gcnew ArgumentOutOfRangeException();	// Invalid value
}

//---------------------------------------------------------------------------
// SqliteUtil::ExecuteNonQuery (static)
//
// Executes a non query against a database.  Used internally for fairly
// simple things like setting database PRAGMAs or attaching files
//
// Arguments:
//
//	hDatabase	- SQLite database handle to be used
//	query		- SQL query to be executed against the connection

void SqliteUtil::ExecuteNonQuery(sqlite3* hDatabase, String^ query)
{
	// Attempt to execute the non-query against the current SQLite database.
	// We don't care about any results, so we don't set up the callback here

	int nResult = sqlite3_exec(hDatabase, AutoAnsiString(query), NULL, NULL, NULL);
	if(nResult != SQLITE_OK) throw gcnew SqliteException(hDatabase, nResult);
}

//---------------------------------------------------------------------------
// SqliteUtil::ExecuteScalar (static)
//
// Executes a scalar query against a database.  Used internally for 
// fairly simple things like reading database PRAGMAs or schema information
//
// Arguments:
//
//	conn		- SqliteConnection instance to execute against
//	query		- SQL query to be executed against the connection

String^ SqliteUtil::ExecuteScalar(sqlite3* hDatabase, String^ query)
{
	ScalarResultObject^		context;		// Reference to the result object
	int						nResult;		// Result from function call

	context = gcnew ScalarResultObject();		// Construct context object

	// Attempt to execute the scalar query against the database, passing in the
	// local callback function as well as the result object's GCHandle ...

	nResult = sqlite3_exec(hDatabase, AutoAnsiString(query), sqliteutil_scalar_callback, 
		AutoGCHandle(context), NULL);
	if(nResult != SQLITE_OK) throw gcnew SqliteException(hDatabase, nResult);

	// Automatically convert a NULL result string into an empty string instead
	
	return (context->Result != nullptr) ? context->Result : String::Empty;
}

//---------------------------------------------------------------------------
// SqliteUtil::FastPtrToStringAnsi (static)
//
// Significanltly faster version of Marshal::PtrToStringAnsi.  In testing,
// this routine performs over twice as fast.  Should not be used for very
// long strings, since it allocates memory on the stack
//
// Arguments:
//
//	psz			- Pointer to the ANSI string to be converted

String^ SqliteUtil::FastPtrToStringAnsi(const char *psz)
{
	return (psz) ? FastPtrToStringAnsi(psz, strlen(psz)) : nullptr;
}

//---------------------------------------------------------------------------
// SqliteUtil::FastPtrToStringAnsi (static)
//
// Significanltly faster version of Marshal::PtrToStringAnsi.  In testing,
// this routine performs over twice as fast.  Should not be used for very
// long strings, since it allocates memory on the stack
//
// Arguments:
//
//	psz			- Pointer to the ANSI string to be converted
//	cch			- Length of the input string, in characters

String^ SqliteUtil::FastPtrToStringAnsi(const char *psz, size_t cch)
{
	if(!psz) return nullptr;			// NULL in, NULL out
	if(cch == 0) return String::Empty;	// No need to convert zero-length

	// Use _alloca to quickly reserve some stack space for the conversion, and
	// invoke a standard MultiByteToWideChar to perform the Unicode conversion.
	
	wchar_t* rgwsz = reinterpret_cast<wchar_t*>(_alloca(cch * sizeof(wchar_t)));

	int int_cch = static_cast<int>(cch);
	if(MultiByteToWideChar(CP_ACP, 0, psz, int_cch, rgwsz, int_cch) == 0) throw gcnew Win32Exception();
	else return gcnew String(rgwsz, 0, int_cch);
}

//---------------------------------------------------------------------------
// SqliteUtil::PragmaToEncoding (static)
//
// Converts a PRAGMA ENCODING result string into a SqliteTextEncodingMode value
//
// Arguments:
//
//	pragma		- The result from a PRAGMA ENCODING query

SqliteTextEncodingMode SqliteUtil::PragmaToEncoding(String^ pragma)
{
	Debug::Assert(pragma != nullptr);

	// Note that the UTF16 comparisons are done with the most likely first.
	// A result of "UTF-16" isn't even possible, at least I don't think it is

	if(String::Compare(pragma, "UTF-8", true) == 0) return SqliteTextEncodingMode::UTF8;
	else if(String::Compare(pragma, "UTF-16le", true) == 0) return SqliteTextEncodingMode::UTF16LittleEndian;
	else if(String::Compare(pragma, "UTF-16be", true) == 0) return SqliteTextEncodingMode::UTF16BigEndian;
	else if(String::Compare(pragma, "UTF-16", true) == 0) return SqliteTextEncodingMode::UTF16;

	else throw gcnew ArgumentOutOfRangeException();		// Invalid value
}

//---------------------------------------------------------------------------
// SqliteUtil::ValidateDataSource (static)
//
// Validates that the specified data source value does not contain any
// invalid characters.  Takes into account the ":memory:" data source
//
// Arguments:
//
//	dataSource	- Data Source property to be validated

bool SqliteUtil::ValidateDataSource(String^ dataSource)
{
	Debug::Assert(dataSource != nullptr);

	if(String::Compare(dataSource, SqliteDataSource::Memory, true) == 0) return true;
	return ValidateFileName(dataSource);
}

//---------------------------------------------------------------------------
// SqliteUtil::ValidateFileName (static)
//
// Validates that the specified path does not contain any invalid characters
//
// Arguments:
//
//	path		- Path to be validated

bool SqliteUtil::ValidateFileName(String^ path)
{
	Debug::Assert(path != nullptr);

	try {

		// The built-in path splitters will throw an exception if the string
		// contains invalid characters, as defined by Path::GetInvalidxxxxChars()

		Path::GetDirectoryName(path);		// Test the folder name
		Path::GetFileName(path);			// Test the file name
		
		return true;						// Both are valid strings
	}
	
	catch(Exception^) { return false; }
}

//---------------------------------------------------------------------------
// SqliteUtil::ValidateFolderName (static)
//
// Validates that the specified path does not contain any invalid characters
//
// Arguments:
//
//	path		- Path to be validated

bool SqliteUtil::ValidateFolderName(String^ path)
{
	Debug::Assert(path != nullptr);
	return (path->IndexOfAny(Path::GetInvalidPathChars()) != -1);
}

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)
