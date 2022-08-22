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

#ifndef __DATABASEEXTENSIONS_H_
#define __DATABASEEXTENSIONS_H_
#pragma once

#include "AutoAnsiString.h"				// Include AutoAnsiString declarations
#include "zDBArgument.h"				// Include zDBArgument declarations
#include "zDBBinaryStream.h"			// Include zDBBinaryStream declarations
#include "zDBEnumerations.h"			// Include zDB enumeration decls
#include "zDBException.h"				// Include zDBException declarations
#include "zlibException.h"				// Include zlibException declarations

using namespace System;
using namespace System::ComponentModel;
using namespace System::Diagnostics;
using namespace System::Globalization;
using namespace System::Runtime::InteropServices;
using namespace zuki::data::sqlite;

#pragma warning(push, 4)				// Enable maximum compiler warnings

//---------------------------------------------------------------------------
// Class DatabaseExtensions
//
// DatabaseExtensions combines all of the custom functions, collations and
// virtual tables that are specific to this implementation of SQLite.  In
// the previous version these were all implemented separately by using the
// provided managed classes.  This is much cleaner and also has the benefit
// of avoiding reflection tools (for the most part)
//---------------------------------------------------------------------------

class DatabaseExtensions
{
public:

	//-----------------------------------------------------------------------
	// Member Functions

	// Register
	//
	// Registers the database extensions with SQLite.  Only necessary to be
	// done once -- see zDBConnection's static constructor
	static void Register(void);

private:

	// DISABLED COPY CONSTRUCTOR / ASSIGNMENT OPERATOR
	DatabaseExtensions(const DatabaseExtensions& rhs);
	DatabaseExtensions& operator=(const DatabaseExtensions& rhs);

	//-----------------------------------------------------------------------
	// Private Type Declarations

	// COMPRESSION_HEADER - The special header used when compressing data
	// with the COMPRESS() scalar function (32 bits)

	typedef struct {

		unsigned __int32	signature	: 4;		// 0x0E signature
		unsigned __int32	dataType	: 4;		// Original data type
		unsigned __int32	length		: 24;		// Original data length
	
	} COMPRESSION_HEADER, *PCOMPRESSION_HEADER;

	// ENCRYPTION_HEADER - The special header used when encrypting data
	// with the ENCRYPT() scalar function (32 bits)
	// Note: "eDE" left in place for compatbility with previous version

	typedef struct {
		
		unsigned __int8	dataType;				// Original data type
		char			signature[3];			// "eDE" ASCII signature
	
	} ENCRYPTION_HEADER, *PENCRYPTION_HEADER;

	//-----------------------------------------------------------------------
	// Private Member Functions

	// BoolFunc
	//
	// Implements the BOOL() scalar function
	static void BoolFunc(sqlite3_context* context, int argc, sqlite3_value** argv); 

	// CompressFunc / CompressFuncEx
	//
	// Implements the COMPRESS() scalar function
	static void CompressFunc(sqlite3_context* context, int argc, sqlite3_value** argv); 
	static void CompressFuncEx(sqlite3_context* context, int argc, sqlite3_value** argv); 

	// CompressInternal
	//
	// Internal implementation of the COMPRESS() scalar function
	static void CompressInternal(sqlite3_context* context, sqlite3_value* arg, int level); 

	// CreateCompressionHeader
	//
	// Generates a COMPRESSION_HEADER as a managed array of bytes
	static array<System::Byte>^ CreateCompressionHeader(int dataType, unsigned long length);

	// DateTimeFunc
	//
	// Implements the DATETIME() scalar function
	static void DateTimeFunc(sqlite3_context* context, int argc, sqlite3_value** argv); 

	// DecompressFunc
	//
	// Implements the DECOMPRESS() scalar function
	static void DecompressFunc(sqlite3_context* context, int argc, sqlite3_value** argv); 

	// DecryptFunc
	//
	// Implements the DECRYPT() scalar function
	static void DecryptFunc(sqlite3_context* context, int argc, sqlite3_value** argv); 

	// EncryptFunc
	//
	// Implements the ENCRYPT() scalar function
	static void EncryptFunc(sqlite3_context* context, int argc, sqlite3_value** argv); 

	// ExtensionInit
	//
	// Callback provided to SQLite to initialize the extensions for a 
	// specific database instance
	static void ExtensionInit(sqlite3* hDatabase, char** ppszErrorMessage,
		const sqlite3_api_routines* pApiRoutines);

	// GuidFunc
	//
	// Implements the GUID() scalar function
	static void GuidFunc(sqlite3_context* context, int argc, sqlite3_value** argv); 

	// ValueToBoolean
	//
	// Coerces a value into a boolean value as best as possible
	static bool ValueToBoolean(sqlite3_value* value);

	// ValueToDateTime
	//
	// Coerces a value into a System::DateTime value as best as possible
	static DateTime ValueToDateTime(sqlite3_value* value);

	// ValueToGuid
	//
	// Coerces a value into a System::Guid as best as possible
	static Guid ValueToGuid(sqlite3_value* value);
};

//---------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __DATABASEEXTENSIONS_H_
