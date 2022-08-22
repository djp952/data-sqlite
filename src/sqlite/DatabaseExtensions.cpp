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
#include "DatabaseExtensions.h"		// Include DatabaseExtensions declarations
#include "zDBConnection.h"			// Include zDBConnection declarations

#pragma warning(push, 4)			// Enable maximum compiler warnings

//---------------------------------------------------------------------------
// DatabaseExtensions::BoolFunc (private, static)
//
// Implements the BOOL() scalar function
//
// Arguments:
//
//	context			- SQLite user function context data
//	argc			- Number of function arguments
//	argv			- Function arguments

void DatabaseExtensions::BoolFunc(sqlite3_context* context, int argc, sqlite3_value** argv)
{
	sqlite3*					hDatabase;		// Parent SQLite database handle
	gcroot<zDBConnection^>		conn;			// Parent zDBConnection object
	bool						value;			// Coerced boolean value

	Debug::Assert(argc == 1);

	try {
		
		if(sqlite3_value_type(argv[0]) == SQLITE_NULL) return sqlite3_result_null(context);

		// The SQLite database handle should have been set as the user data for
		// this function, which we can then map back into a zDBConnection object

		hDatabase = reinterpret_cast<sqlite3*>(sqlite3_user_data(context));
		conn = zDBConnection::FindConnection(hDatabase);
		if(static_cast<zDBConnection^>(conn) == nullptr) throw gcnew Exception("Invalid database handle");

		value = ValueToBoolean(argv[0]);		// Convert the value into a boolean

		// Depending on the format of the parent connection, return the coerced value
		// as either an integer or a string back to SQLite ...

		switch(conn->BooleanFormat) {

			case zDBBooleanFormat::OneZero: 
				return sqlite3_result_int(context, (value) ? 1 : 0);

			case zDBBooleanFormat::NegativeOneZero: 
				return sqlite3_result_int(context, (value) ? -1 : 0);

			case zDBBooleanFormat::TrueFalse: 
				return sqlite3_result_text(context, (value) ? "true" : "false", -1, SQLITE_STATIC);
		}
	}

	catch(Exception^ ex) { sqlite3_result_error(context, AutoAnsiString("BOOL(): " + ex->Message), -1); }
}

//---------------------------------------------------------------------------
// DatabaseExtensions::CompressFunc (private, static)
//
// Implements the COMPRESS() scalar function that accepts just one argument,
// which indicates that a default compression level should be used
//
// Arguments:
//
//	context			- SQLite user function context data
//	argc			- Number of function arguments
//	argv			- Function arguments

void DatabaseExtensions::CompressFunc(sqlite3_context* context, int argc, sqlite3_value** argv)
{
	Debug::Assert(argc == 1);
	return CompressInternal(context, argv[0], -1);
}

//---------------------------------------------------------------------------
// DatabaseExtensions::CompressFuncEx (private, static)
//
// Implements the COMPRESS() scalar function that accepts two arguments, the
// field to be compressed, and the compression level to use
//
// Arguments:
//
//	context			- SQLite user function context data
//	argc			- Number of function arguments
//	argv			- Function arguments

void DatabaseExtensions::CompressFuncEx(sqlite3_context* context, int argc, sqlite3_value** argv)
{
	int				compLevel;					// Compression level to use

	Debug::Assert(argc == 2);					// Should always be 2 for this overload

	// The compression level argument must have INTEGER affinity

	if(sqlite3_value_type(argv[1]) != SQLITE_INTEGER)
		return sqlite3_result_error(context, "COMPRESS(): Compression level argument must be an integer", -1);

	// The compression level must be between -1 (default) and 9 (maximum)

	compLevel = sqlite3_value_int(argv[1]);
	if((compLevel < -1) || (compLevel > 9))
		return sqlite3_result_error(context, "COMPRESS(): Compression level must be between -1 and 9", -1);

	return CompressInternal(context, argv[0], compLevel);
}

//---------------------------------------------------------------------------
// DatabaseExtensions::CompressInternal (private, static)
//
// Common implementation for the COMPRESS() scalar function overloads
//
// Arguments:
//
//	context			- SQLite user function context data
//	arg				- Field to be compressed
//	level			- Compression level to be used for the operation

void DatabaseExtensions::CompressInternal(sqlite3_context* context, sqlite3_value* arg, int level)
{
	int						cbInputData;		// Length of the data to compress
	z_stream				zStream;			// ZLIB stream state structure
	zDBBinaryStream^		outStream;			// Output zDBBinaryStream object
	array<System::Byte>^	rgBuffer;			// Generic byte array reference
	PinnedBytePtr			pinBuffer;			// Generic pinning array pointer
	int						cbOutData;			// Length of the output data
	void*					pvOutData;			// Pointer to the output data
	int						zResult;			// Result from ZLIB function

	try {

		cbInputData = sqlite3_value_bytes(arg);		// Get input data length

		// The maximum allowable data size for this internal operation is 16MB,
		// mainly because that's quite generous as it is, but the header is also
		// set up to only handle a 24-bit length specifier

		if(cbInputData > 0xFFFFFF)
			throw gcnew Exception("Cannot compress data larger than 16MB in length");
	
		// Initialize the deflater stream using the specified compression level.
		// If successful, we must ensure that deflateEnd() is called to prevent leaks

		memset(&zStream, 0, sizeof(zStream));
		zResult = deflateInit(&zStream, level);
		if(zResult != Z_OK) throw gcnew zlibException(zResult);
		
		try {

			outStream = gcnew zDBBinaryStream(cbInputData);	// Create the stream
			
			// Create and append the COMPRESSION_HEADER, and more importantly,
			// seed the output data length with the size of this header

			rgBuffer = CreateCompressionHeader(sqlite3_value_type(arg), cbInputData);
			outStream->Write(rgBuffer, 0, rgBuffer->Length);

			rgBuffer = gcnew array<System::Byte>(8192);		// Allocate the read buffer
			pinBuffer = &rgBuffer[0];						// Pin it down in memory

			zStream.avail_in = cbInputData;
			zStream.next_in = reinterpret_cast<Bytef*>(const_cast<void*>(sqlite3_value_blob(arg)));

			// deflate() doesn't have much in the way of failure conditions,
			// so we can pretty much just loop all willy-nilly here.  I'm 
			// placing a great deal of faith in ZLIB's documentation ...

			do { 

				zStream.avail_out = 8192;			// 8K of buffer to play with
				zStream.next_out = pinBuffer;		// Reset the base pointer
				deflate(&zStream, Z_FINISH);		// Compress the next block
				
				// Write the next block of data into the memory stream, and keep
				// track of how much data we've actually written in there

				outStream->Write(rgBuffer, 0, 8192 - zStream.avail_out);
			
			} while(zStream.avail_out == 0);

			// Thanks to the handy-dandy zDBBinaryStream class, we can perform a
			// more efficient callback here that allows SQLite to keep the buffer
			// around as long as it needs to and access it directly

			cbOutData = outStream->Lock(&pvOutData);
			sqlite3_result_blob(context, pvOutData, cbOutData, zDBBinaryStream::OnRelease);
		}
		
		finally { deflateEnd(&zStream); }			// Always close the deflater
	}

	catch(Exception^ ex) { sqlite3_result_error(context, AutoAnsiString("COMPRESS(): " + ex->Message), -1); }
}

//---------------------------------------------------------------------------
// DatabaseExtensions::CreateCompressionHeader (private, static)
//
// Creates a COMPRESSION_HEADER as a managed byte array
//
// Arguments:
//
//	dataType		- The original data's SQLite type affinity
//	length			- The original data's uncompressed length

array<System::Byte>^ DatabaseExtensions::CreateCompressionHeader(int dataType, unsigned long length)
{
	// Create a managed byte array just big enough for the header data, and
	// then immediately pin it in memory so we can safely address it

	array<System::Byte>^ rgHeader = gcnew array<System::Byte>(sizeof(COMPRESSION_HEADER));
	PinnedBytePtr pinHeader = &rgHeader[0];

	// Make life easy by casting out a PCOMPRESSION_HEADER pointer ..

	PCOMPRESSION_HEADER pHeader = reinterpret_cast<PCOMPRESSION_HEADER>(pinHeader);

	pHeader->signature = 0x0E;				// "E" is for Empyre Group
	pHeader->dataType = dataType;			// Save the original data type
	pHeader->length = length;				// Save the original data length

	return rgHeader;						// Return the managed byte array
}

//---------------------------------------------------------------------------
// DatabaseExtensions::DateTimeFunc (private, static)
//
// Implements the DATETIME() scalar function
//
// Arguments:
//
//	context			- SQLite user function context data
//	argc			- Number of function arguments
//	argv			- Function arguments

void DatabaseExtensions::DateTimeFunc(sqlite3_context* context, int argc, sqlite3_value** argv)
{
	sqlite3*					hDatabase;		// Parent SQLite database handle
	gcroot<zDBConnection^>		conn;			// Parent zDBConnection object
	DateTime					value;			// Coerced System.DateTime value
	
	Debug::Assert(argc == 1);

	try {

		if(sqlite3_value_type(argv[0]) == SQLITE_NULL) return sqlite3_result_null(context);

		// The SQLite database handle should have been set as the user data for
		// this function, which we can then map back into a zDBConnection object

		hDatabase = reinterpret_cast<sqlite3*>(sqlite3_user_data(context));
		conn = zDBConnection::FindConnection(hDatabase);
		if(static_cast<zDBConnection^>(conn) == nullptr) throw gcnew Exception("Invalid database handle");

		value = ValueToDateTime(argv[0]);		// Convert the value into a DateTime

		// Depending on the format of the parent connection, return the coerced value
		// as either a string or a 64 bit integer ...

		switch(conn->DateTimeFormat) {

			case zDBDateTimeFormat::ISO8601:
				return sqlite3_result_text(context, AutoAnsiString(value.ToString("yyyy-mm-dd HH:mm:ss")), 
					-1, SQLITE_TRANSIENT);

			case zDBDateTimeFormat::Sortable:
				return sqlite3_result_text(context, AutoAnsiString(value.ToString("s", DateTimeFormatInfo::InvariantInfo)), 
					-1, SQLITE_TRANSIENT);

			case zDBDateTimeFormat::UniversalSortable:
				return sqlite3_result_text(context, AutoAnsiString(value.ToString("u", DateTimeFormatInfo::InvariantInfo)), 
					-1, SQLITE_TRANSIENT);

			case zDBDateTimeFormat::RFC1123:
				return sqlite3_result_text(context, AutoAnsiString(value.ToString("r", DateTimeFormatInfo::InvariantInfo)), 
					-1, SQLITE_TRANSIENT);

			case zDBDateTimeFormat::Ticks:
				return sqlite3_result_int64(context, value.Ticks);
		}
	}
	catch(Exception^ ex) { sqlite3_result_error(context, AutoAnsiString("DATETIME(): " + ex->Message), -1); }
}

//---------------------------------------------------------------------------
// DatabaseExtensions::DecompressFunc (private, static)
//
// Implements the DECOMPRESS() scalar function
//
// Arguments:
//
//	context			- SQLite user function context data
//	argc			- Number of function arguments
//	argv			- Function arguments

void DatabaseExtensions::DecompressFunc(sqlite3_context* context, int argc, sqlite3_value** argv)
{
	PCOMPRESSION_HEADER		pHeader;			// Pointer to the header
	unsigned char*			rgBuffer = NULL;	// Decompression buffer
	z_stream				zStream;			// ZLIB stream state structure
	int						zResult;			// Result from ZLIB function

	// If for some reason no arguments were passed, or the first argument
	// is NULL, the result is implicitly NULL by default

	if((argc == 0) || (sqlite3_value_type(argv[0]) == SQLITE_NULL)) return sqlite3_result_null(context);

	try {

		// Anything compressed by COMPRESS() will be of type SQLITE_BLOB and
		// will be at least big enough to hold the COMPRESSION_HEADER struct

		if((sqlite3_value_type(argv[0]) != SQLITE_BLOB) || (sqlite3_value_bytes(argv[0]) < sizeof(COMPRESSION_HEADER)))
			throw gcnew Exception("Invalid argument type");

		// Do a very quick cursory check to see if this field was compressed
		// with COMPRESS().  While it's obviously not a fool-proof mechanism, it
		// should be more then enough since ZLIB will also catch this as well

		pHeader = reinterpret_cast<PCOMPRESSION_HEADER>(const_cast<void*>(sqlite3_value_blob(argv[0])));
		if(pHeader->signature != 0x0E) throw gcnew Exception("Field not compressed with COMPRESS()");

		// Initialize the inflater stream.  If the initialization is successful, we
		// have to be certain that inflateEnd() is called at some point to prevent
		// ZLIB from leaking memory on us ...

		memset(&zStream, 0, sizeof(zStream));
		zResult = inflateInit(&zStream);
		if(zResult != Z_OK) throw gcnew zlibException(zResult);

		try {

			// Allocate the decompression buffer with malloc(), which allows us
			// to pass it directly to SQLite and save a memory copy operation

			rgBuffer = reinterpret_cast<unsigned char*>(malloc(pHeader->length));
			if(!rgBuffer) throw gcnew Win32Exception(ERROR_NOT_ENOUGH_MEMORY);

			// All the sizes are known, so the inflate can take place in a single
			// pass.  Set up the z_stream with all the necessary information, then
			// go ahead and ask ZLIB to perform it's decompression magic ...

			zStream.avail_in = sqlite3_value_bytes(argv[0]) - sizeof(COMPRESSION_HEADER);
			zStream.next_in = reinterpret_cast<Bytef*>(const_cast<void*>(sqlite3_value_blob(argv[0])));
			zStream.next_in += sizeof(COMPRESSION_HEADER);

			zStream.avail_out = pHeader->length;
			zStream.next_out = rgBuffer;	

			zResult = inflate(&zStream, Z_FINISH);
			if(zResult != Z_STREAM_END) throw gcnew Exception("Field contains uncompressed or corrupt data");

			// Depending on what kind of data was originally compressed here, we need
			// to call the appropriate SQLite function to set the right affinity

			switch(pHeader->dataType) {

				// SQLITE_INTEGER: The data would have been encrypted as an ANSI
				// string, so perform the necessary affinity type conversion

				case SQLITE_INTEGER: 

					sqlite3_result_int64(context, _atoi64(reinterpret_cast<char*>(rgBuffer)));
					break;

				// SQLITE_FLOAT: The data would have been encrypted as an ANSI
				// string, so perform the necessary affinity type conversion

				case SQLITE_FLOAT:

					sqlite3_result_double(context, atof(reinterpret_cast<char*>(rgBuffer)));
					break;

				// SQLITE_TEXT: The encrypted data was a Unicode string.  Cheat the system
				// by passing in free() to release the data to get as much efficiency as we
				// can out of this little routine ...

				case SQLITE_TEXT:

					sqlite3_result_text16(context, rgBuffer, pHeader->length, free);

					rgBuffer = NULL;		// <--- Prevent free() in finally {}
					break;

				// SQLITE_BLOB: The encrypted data was a binary BLOB.  Once again
				// we bypass releasing the memory in the finally block and let SQLite
				// take care of it when it's done using it by calling free() on it's own

				case SQLITE_BLOB: 

					sqlite3_result_blob(context, rgBuffer, pHeader->length, free);
					
					rgBuffer = NULL;		// <--- Prevent free() in finally {}
					break;

				default: throw gcnew Exception("Unrecognized data type");
			}
		}
		
		finally { 

			inflateEnd(&zStream);				// Clean up the ZLIB inflater
			if(rgBuffer) free(rgBuffer);		// Release decompression buffer
		}
	}

	catch(Exception^ ex) { sqlite3_result_error(context, AutoAnsiString("DECOMPRESS(): " + ex->Message), -1); }
}

//---------------------------------------------------------------------------
// DatabaseExtensions::DecryptFunc (private, static)
//
// Implements the DECRYPT() scalar function
//
// Arguments:
//
//	context			- SQLite user function context data
//	argc			- Number of function arguments
//	argv			- Function arguments

void DatabaseExtensions::DecryptFunc(sqlite3_context* context, int argc, sqlite3_value** argv)
{
	sqlite3*					hDatabase;			// Parent SQLite database handle
	gcroot<zDBConnection^>		conn;				// Parent zDBConnection object
	PENCRYPTION_HEADER			pHeader;			// Encryption header
	DWORD						cbBuffer;			// Allocated buffer size
	unsigned char*				rgBuffer;			// Decryption buffer
	
	Debug::Assert(argc == 1);

	try {

		if(sqlite3_value_type(argv[0]) == SQLITE_NULL) return sqlite3_result_null(context);

		// The SQLite database handle should have been set as the user data for
		// this function, which we can then map back into a zDBConnection object

		hDatabase = reinterpret_cast<sqlite3*>(sqlite3_user_data(context));
		conn = zDBConnection::FindConnection(hDatabase);
		if(static_cast<zDBConnection^>(conn) == nullptr) throw gcnew Exception("Invalid database handle");

		// Anything compressed by ENCRYPT() will be of type SQLITE_BLOB and
		// will be at least big enough to hold the ENCRYPTION_HEADER struct

		if((sqlite3_value_type(argv[0]) != SQLITE_BLOB) || (sqlite3_value_bytes(argv[0]) < sizeof(ENCRYPTION_HEADER)))
			throw gcnew Exception("Invalid argument type");

		// Do a very quick cursory check to see if this field was encrypted with ENCRYPT()
		// Note that the 'eDE' signature is a holdover for compatibility purposes with 1.1

		pHeader = reinterpret_cast<PENCRYPTION_HEADER>(const_cast<void*>(sqlite3_value_blob(argv[0])));
		if(strncmp(pHeader->signature, "eDE", 3) != 0) throw gcnew Exception("Field not encrypted with ENCRYPT()");

		// Determine the length of the decrypted buffer and allocate it using malloc()

		cbBuffer = sqlite3_value_bytes(argv[0]) - sizeof(ENCRYPTION_HEADER);
		rgBuffer = reinterpret_cast<unsigned char*>(malloc(cbBuffer));
		if(!rgBuffer) throw gcnew Win32Exception(ERROR_NOT_ENOUGH_MEMORY);

		try {

			// CryptoAPI works in-place, so the encrypted data needs to be copied into
			// the decryption buffer. Note the skipping of the ENCRYPTION_HEADER

			memcpy(rgBuffer, reinterpret_cast<unsigned char*>(pHeader) + sizeof(ENCRYPTION_HEADER), cbBuffer);

			// We're ready to go finally ... decrypt the data 

			if(!CryptDecrypt(conn->FieldEncryptionKey, NULL, TRUE, 0, rgBuffer, &cbBuffer))
				throw gcnew Win32Exception(GetLastError());

			// Depending on what kind of data was originally encrypted here, we need
			// to call the appropriate SQLite function to set the right affinity

			switch(pHeader->dataType) {

				// SQLITE_INTEGER: The data would have been encrypted as an ANSI
				// string, so perform the necessary affinity type conversion

				case SQLITE_INTEGER: 

					sqlite3_result_int64(context, _atoi64(reinterpret_cast<char*>(rgBuffer)));
					break;

				// SQLITE_FLOAT: The data would have been encrypted as an ANSI
				// string, so perform the necessary affinity type conversion

				case SQLITE_FLOAT:

					sqlite3_result_double(context, atof(reinterpret_cast<char*>(rgBuffer)));
					break;

				// SQLITE_TEXT: The encrypted data was a Unicode string.  Rather
				// than use eDBResult, we cheat the system on this one to get as
				// much efficiency as we can out of this routine

				case SQLITE_TEXT:

					sqlite3_result_text16(context, rgBuffer, cbBuffer, free);

					rgBuffer = NULL;		// <--- Prevent free() in __finally {}
					break;

				// SQLITE_BLOB: The encrypted data was a binary BLOB.  Once again
				// we bypass eDBResult and hook SQLite up with direct access to the
				// data buffer as well as a means to release it when it's finished

				case SQLITE_BLOB: 

					sqlite3_result_blob(context, rgBuffer, cbBuffer, free);
					
					rgBuffer = NULL;		// <--- Prevent free() in __finally {}
					break;

				default: throw gcnew Exception("Unrecognized data type");
			}
		}

		finally { if(rgBuffer) free(rgBuffer); }		// Release decryption buffer
	}

	catch(Exception^ ex) { sqlite3_result_error(context, AutoAnsiString("DECRYPT(): " + ex->Message), -1); }
}

//---------------------------------------------------------------------------
// DatabaseExtensions::EncryptFunc (private, static)
//
// Implements the ENCRYPT() scalar function
//
// Arguments:
//
//	context			- SQLite user function context data
//	argc			- Number of function arguments
//	argv			- Function arguments

void DatabaseExtensions::EncryptFunc(sqlite3_context* context, int argc, sqlite3_value** argv)
{
	sqlite3*					hDatabase;			// Parent SQLite database handle
	gcroot<zDBConnection^>		conn;				// Parent zDBConnection object
	DWORD						cbRequired;			// Required buffer space
	PENCRYPTION_HEADER			pHeader;			// Encryption header
	DWORD						cbBuffer;			// Allocated buffer size
	unsigned char*				rgBuffer;			// Decryption buffer

	Debug::Assert(argc == 1);

	try {

		if(sqlite3_value_type(argv[0]) == SQLITE_NULL) return sqlite3_result_null(context);

		// The SQLite database handle should have been set as the user data for
		// this function, which we can then map back into a zDBConnection object

		hDatabase = reinterpret_cast<sqlite3*>(sqlite3_user_data(context));
		conn = zDBConnection::FindConnection(hDatabase);
		if(static_cast<zDBConnection^>(conn) == nullptr) throw gcnew Exception("Invalid database handle");

		// The first step is to ask CryptoAPI to tell us how much buffer space
		// we're going to need to actually encrypt something of [arg] bytes big

		cbRequired = sqlite3_value_bytes(argv[0]);
		if(!CryptEncrypt(conn->FieldEncryptionKey, NULL, TRUE, 0, NULL, &cbRequired, cbRequired))
			throw gcnew Win32Exception(GetLastError());

		// Allocate an unmanaged heap buffer of the required size with malloc(), 
		// since free() can be passed into SQLite down below quite easily ..

		cbBuffer = cbRequired + sizeof(ENCRYPTION_HEADER);
		rgBuffer = reinterpret_cast<unsigned char*>(malloc(cbBuffer));
		if(!rgBuffer) throw gcnew Win32Exception(ERROR_NOT_ENOUGH_MEMORY);

		// Initialize the new buffer by setting up the header data and copying
		// the original unencrypted data into it (CryptoAPI uses it in-place)

		pHeader = reinterpret_cast<PENCRYPTION_HEADER>(rgBuffer);
		pHeader->dataType = static_cast<unsigned __int8>(sqlite3_value_type(argv[0]));

		memcpy(pHeader->signature, "eDE", 3);	
		memcpy(&rgBuffer[sizeof(ENCRYPTION_HEADER)], sqlite3_value_blob(argv[0]), sqlite3_value_bytes(argv[0]));

		// Attempt to really encrypt the data this time through now that the
		// buffer is of the correct size and has been set up with the data

		cbRequired = sqlite3_value_bytes(argv[0]);
		if(!CryptEncrypt(conn->FieldEncryptionKey, NULL, TRUE, 0, &rgBuffer[sizeof(ENCRYPTION_HEADER)], 
			&cbRequired, cbBuffer - sizeof(ENCRYPTION_HEADER))) {
			
			free(rgBuffer);									// Release encryption buffer
			throw gcnew Win32Exception(GetLastError());		// Throw the exception
		}

		// Pass the resultant encrypted data back to SQLite as a BLOB value, telling
		// it to just call into the standard CRT free() function to release it ...

		sqlite3_result_blob(context, rgBuffer, cbBuffer, free);
	}

	catch(Exception^ ex) { sqlite3_result_error(context, AutoAnsiString("ENCRYPT(): " + ex->Message), -1); }
}

//---------------------------------------------------------------------------
// DatabaseExtensions::ExtensionInit (private, static)
//
// Callback from SQLite used to initialize the database extensions for a new
// database instance
//
// Arguments:
//
//	hDatabase			- SQLite database handle
//	ppszErrorMessage	- On failure, contains the error message
//	pApiRoutines		- Pointer to the SQLite API routines

void DatabaseExtensions::ExtensionInit(sqlite3* hDatabase, char** ppszErrorMessage,
	const sqlite3_api_routines* pApiRoutines)
{
	UNREFERENCED_PARAMETER(ppszErrorMessage);
	UNREFERENCED_PARAMETER(pApiRoutines);

	// BOOL(value)
	sqlite3_create_function(hDatabase, "bool", 1, SQLITE_ANY, hDatabase, BoolFunc, NULL, NULL);

	// COMPRESS(value, [level])
	sqlite3_create_function(hDatabase, "compress", 1, SQLITE_ANY, hDatabase, CompressFunc, NULL, NULL);
	sqlite3_create_function(hDatabase, "compress", 2, SQLITE_ANY, hDatabase, CompressFuncEx, NULL, NULL);
	
	// DATETIME(value)
	sqlite3_create_function(hDatabase, "datetime", 1, SQLITE_ANY, hDatabase, DateTimeFunc, NULL, NULL);

	// DECOMPRESS(value)
	sqlite3_create_function(hDatabase, "decompress", 1, SQLITE_ANY, hDatabase, DecompressFunc, NULL, NULL);

	// DECRYPT(value)
	sqlite3_create_function(hDatabase, "decrypt", 1, SQLITE_ANY, hDatabase, DecryptFunc, NULL, NULL);

	// ENCRYPT(value)
	sqlite3_create_function(hDatabase, "encrypt", 1, SQLITE_ANY, hDatabase, EncryptFunc, NULL, NULL);

	// GUID(value)
	sqlite3_create_function(hDatabase, "guid", 1, SQLITE_ANY, hDatabase, GuidFunc, NULL, NULL);
}

//---------------------------------------------------------------------------
// DatabaseExtensions::GuidFunc (private, static)
//
// Implements the GUID() scalar function
//
// Arguments:
//
//	context			- SQLite user function context data
//	argc			- Number of function arguments
//	argv			- Function arguments

void DatabaseExtensions::GuidFunc(sqlite3_context* context, int argc, sqlite3_value** argv)
{
	sqlite3*					hDatabase;		// Parent SQLite database handle
	gcroot<zDBConnection^>		conn;			// Parent zDBConnection object
	Guid						value;			// Coerced Guid value
	array<System::Byte>^		rgValue;		// Value as a byte array
	PinnedBytePtr				pinValue;		// Pinned value buffer

	Debug::Assert(argc == 1);

	try {

		if(sqlite3_value_type(argv[0]) == SQLITE_NULL) return sqlite3_result_null(context);

		// The SQLite database handle should have been set as the user data for
		// this function, which we can then map back into a zDBConnection object

		hDatabase = reinterpret_cast<sqlite3*>(sqlite3_user_data(context));
		conn = zDBConnection::FindConnection(hDatabase);
		if(static_cast<zDBConnection^>(conn) == nullptr) throw gcnew Exception("Invalid database handle");

		value = ValueToGuid(argv[0]);		// Convert the value into a Guid

		// Depending on the format of the parent connection, return the coerced value
		// as either a string or a BLOB back to the caller ...

		switch(conn->GuidFormat) {

			case zDBGuidFormat::Binary:
				rgValue = value.ToByteArray();
				pinValue = &rgValue[0];
				return sqlite3_result_blob(context, pinValue, rgValue->Length, SQLITE_TRANSIENT);

			case zDBGuidFormat::HexString:
				return sqlite3_result_text(context, AutoAnsiString(value.ToString("N")), -1, SQLITE_TRANSIENT);

			case zDBGuidFormat::Hyphenated:
				return sqlite3_result_text(context, AutoAnsiString(value.ToString("D")), -1, SQLITE_TRANSIENT);

			case zDBGuidFormat::Bracketed:
				return sqlite3_result_text(context, AutoAnsiString(value.ToString("B")), -1, SQLITE_TRANSIENT);

			case zDBGuidFormat::Parenthetic:
				return sqlite3_result_text(context, AutoAnsiString(value.ToString("P")), -1, SQLITE_TRANSIENT);
		}
	}

	catch(Exception^ ex) { sqlite3_result_error(context, AutoAnsiString("GUID(): " + ex->Message), -1); }
}

//---------------------------------------------------------------------------
// DatabaseExtensions::Register (static)
//
// Registers the extensions with SQLite.  Only necessary to call this once
// per process -- see zDBConnection's static constructor
//
// Arguments:
//
//	NONE

void DatabaseExtensions::Register(void)
{
	// TODO: FIX ME
	//
	//int nResult = sqlite3_auto_extension(ExtensionInit);
	//if(nResult != SQLITE_OK) throw gcnew zDBException(nResult);
}

//---------------------------------------------------------------------------
// DatabaseExtensions::ValueToBoolean (static, private)
//
// Attempts to coerce a value into a boolean value as best as possible
//
// Arguments:
//	
//	value		- Value to be coerced

bool DatabaseExtensions::ValueToBoolean(sqlite3_value* value)
{
	String^					boolString;			// For text->bool conversions

	// Booleans can be stored in the database as integer or string values.  Anything
	// else will generate an InvalidCastException

	switch(sqlite3_value_type(value)) {

		// INTEGER: TRUE if non-zero
		case SQLITE_INTEGER:

			return sqlite3_value_int(value) != 0;

		// TEXT: Use Convert::ToBoolean
		case SQLITE_TEXT:

			boolString = gcnew String(reinterpret_cast<const wchar_t*>(sqlite3_value_text16(value)));
			return Convert::ToBoolean(boolString);

		default: throw gcnew InvalidCastException();
	}
}

//---------------------------------------------------------------------------
// DatabaseExtensions::ValueToDateTime (static, private)
//
// Attempts to coerce a value into a date/time value as best as possible
//
// Arguments:
//	
//	value		- Value to be coerced

DateTime DatabaseExtensions::ValueToDateTime(sqlite3_value* value)
{
	String^					dateString;			// For text->date conversions

	// Booleans can be stored in the database as integer or string values.  Anything
	// else will generate an InvalidCastException

	switch(sqlite3_value_type(value)) {

		// INTEGER: Assume this date was stored as a tick count
		case SQLITE_INTEGER:

			return DateTime(sqlite3_value_int64(value));

		// TEXT: Use DateTime::Parse
		case SQLITE_TEXT:

			dateString = gcnew String(reinterpret_cast<const wchar_t*>(sqlite3_value_text16(value)));
			return DateTime::Parse(dateString);

		default: throw gcnew InvalidCastException();
	}
}

//---------------------------------------------------------------------------
// DatabaseExtensions::ValueToGuid (static, private)
//
// Attempts to coerce a value into a Guid value as best as possible
//
// Arguments:
//	
//	value		- Value to be coerced

Guid DatabaseExtensions::ValueToGuid(sqlite3_value* value)
{
	String^					guidString;			// For text->guid conversions
	const GUID*				puuid;				// Pointer to unmanaged GUID data

	// Booleans can be stored in the database as integer or string values.  Anything
	// else will generate an InvalidCastException

	switch(sqlite3_value_type(value)) {

		// BLOB: Use the Guid constructor that accepts all the component values
		case SQLITE_BLOB:
			
			if(sqlite3_value_bytes(value) != sizeof(GUID)) throw gcnew InvalidCastException();
			puuid = reinterpret_cast<const GUID*>(sqlite3_value_blob(value));

			return Guid(puuid->Data1, puuid->Data2, puuid->Data3, puuid->Data4[0],
				puuid->Data4[1], puuid->Data4[2], puuid->Data4[3], puuid->Data4[4], puuid->Data4[5],
				puuid->Data4[6], puuid->Data4[7]);

		// TEXT: Use Guid string constructor 
		case SQLITE_TEXT:

			guidString = gcnew String(reinterpret_cast<const wchar_t*>(sqlite3_value_text16(value)));
			return Guid(guidString);

		default: throw gcnew InvalidCastException();
	}
}

//---------------------------------------------------------------------------

#pragma warning(pop)
