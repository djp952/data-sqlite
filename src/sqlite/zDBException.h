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

#ifndef __ZDBEXCEPTION_H_
#define __ZDBEXCEPTION_H_
#pragma once

#pragma warning(push, 4)				// Enable maximum compiler warnings

using namespace System;
using namespace System::Data::Common;

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// Class zDBException
//
// The exception that is thrown when a database engine error has occurred.
// Also used as a base class for all custom provider exceptions to allow
// applications to simply "catch(zDBException^)" instead of specific ones.
// This works out pretty well, since a provider exception is essentially the
// same thing as an engine exception to the end user.
//---------------------------------------------------------------------------

public ref class zDBException : public DbException
{
public:

	//-----------------------------------------------------------------------
	// Properties

	// IsEngineException
	//
	// Flag determining if this is a SQLite engine error or not
	property bool IsEngineException { bool get(void) { return (ErrorCode != SQLITE_PROVIDER); } }

	// IsProviderException
	//
	// Flag determining if this is a provider-based error or not
	property bool IsProviderException { bool get(void) { return (ErrorCode == SQLITE_PROVIDER); } }

protected private:

	// PROTECTED CONSTRUCTORS
	//
	// Use these when deriving from the zDBException class for a custom
	// provider exception instead of an engine exception
	zDBException(String^ message);

internal:

	// INTERNAL CONSTRUCTORS
	//
	// Use these when a SQLite engine error has occurred
	zDBException(int nResult);
	zDBException(int nResult, String^ context);
	zDBException(sqlite3* hDatabase, int nResult);
	zDBException(sqlite3* hDatabase, int nResult, String^ context);

private:

	//-----------------------------------------------------------------------
	// Private Constants

	// SQLITE_PROVIDER
	//
	// Indicates a provider specific error, not a SQLite engine error
	literal int SQLITE_PROVIDER = 255;

	//-----------------------------------------------------------------------
	// Private Member Functions

	static String^ GenerateMessage(sqlite3* hDatabase, int nResult);
	static HRESULT MakeHResult(int nResult);
};

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)

#endif		// __ZDBEXCEPTION_H_
