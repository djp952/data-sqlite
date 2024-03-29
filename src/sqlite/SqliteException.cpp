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

#include "stdafx.h"						// Include project pre-compiled headers
#include "SqliteException.h"				// Include SqliteException declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// SqliteException Constructor (protected)
//
// Arguments:
//
//	message		- Provider-specific error message to set on exception

SqliteException::SqliteException(String^ message) : DbException(message, SQLITE_PROVIDER)
{
	HResult = MakeHResult(SQLITE_PROVIDER);		// Set the HResult value
}

//---------------------------------------------------------------------------
// SqliteException Constructor (internal)
//
// Arguments:
//
//	nResult		- SQLite status code

SqliteException::SqliteException(int nResult) : 
	DbException(GenerateMessage(NULL, nResult), nResult) 
{
	HResult = MakeHResult(nResult);				// Set the HResult value
}

//---------------------------------------------------------------------------
// SqliteException Constructor (internal)
//
// Arguments:
//
//	nResult		- SQLite status code
//	context		- Context information string

SqliteException::SqliteException(int nResult, String^ context) :
	DbException(String::Format("{0}\r\n\r\nContext:\r\n{1}", 
	GenerateMessage(NULL, nResult), context), nResult) 
{
	HResult = MakeHResult(nResult);			// Set the HResult value
}

//---------------------------------------------------------------------------
// SqliteException Constructor (internal)
//
// Arguments:
//
//	hDatabase	- SQLite database handle
//	nResult		- SQLite status code

SqliteException::SqliteException(sqlite3* hDatabase, int nResult) : 
	DbException(GenerateMessage(hDatabase, nResult), nResult)
{
	HResult = MakeHResult(nResult);			// Set the HResult value
}

//---------------------------------------------------------------------------
// SqliteException Constructor (internal)
//
// Arguments:
//
//	hDatabase	- SQLite database handle
//	nResult		- SQLite status code
//	context		- Context information string

SqliteException::SqliteException(sqlite3* hDatabase, int nResult, String^ context) :
	DbException(String::Format("{0}\r\n\r\nContext:\r\n{1}", 
	GenerateMessage(hDatabase, nResult), context), nResult)
{
	HResult = MakeHResult(nResult);			// Set the HResult value
}

//---------------------------------------------------------------------------
// SqliteException::GenerateMessage (static, private)
//
// Generates a text-based SQLite error message from an integer result code
//
// Arguments:
//
//	hDatabase	- Optional SQLite database handle
//	nResult		- SQLite status code

String^ SqliteException::GenerateMessage(sqlite3* hDatabase, int nResult)
{
	return String::Format("DBMS Error {0}: {1}", nResult,
		gcnew String(reinterpret_cast<LPCWSTR>(sqlite3_errmsg16(hDatabase))));
}

//---------------------------------------------------------------------------
// SqliteException::MakeHResult (static, private)
//
// Generates an HRESULT from a SQLite error code
//
// Arguments:
//
//	nResult		- Result from the SQLite function call

HRESULT SqliteException::MakeHResult(int nResult)
{
	// A SqliteException HRESULT uses FACILITY_ITF and bases all codes at 0x200
	// per the Microsoft HRESULT specification

	return MAKE_HRESULT((nResult == SQLITE_OK) ? SEVERITY_SUCCESS : SEVERITY_ERROR, 
		FACILITY_ITF, 0x200 + nResult);
}

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)
