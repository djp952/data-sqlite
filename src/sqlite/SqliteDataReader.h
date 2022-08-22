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

#ifndef __SQLITEDATAREADER_H_
#define __SQLITEDATAREADER_H_
#pragma once

#include "ITrackableObject.h"			// Include ITrackableObject decls
#include "SqliteBinaryReader.h"				// Include SqliteBinaryReader decls
#include "SqliteEnumerations.h"			// Include Sqlite enumeration decls
#include "SqliteException.h"				// Include SqliteException declarations
#include "SqliteExceptions.h"				// Include Sqlite exception decls
#include "SqliteQuery.h"					// Include SqliteQuery declarations
#include "SqliteStatement.h"				// Include SqliteStatement decls
#include "SqliteUtil.h"					// Include SqliteUtil declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

using namespace System;
using namespace System::Collections;
using namespace System::Data;
using namespace System::Data::Common;

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// Forward Class Declarations
//---------------------------------------------------------------------------

ref class SqliteConnection;				// SqliteConnection.h
ref class SqliteCommand;					// SqliteCommand.h

//---------------------------------------------------------------------------
// Class SqliteDataReader
//
// Implements the data reader object for the ADO.NET provider.  This object
// also implements the ITrackableObject interface so other classes can keep
// an eye on it with an instance of the ObjectTracker class.
//---------------------------------------------------------------------------

public ref class SqliteDataReader sealed : public DbDataReader, public ITrackableObject
{
public:

	//-----------------------------------------------------------------------
	// Member Functions

	// Close (DbDataReader)
	//
	// Closes the DataReader
	virtual void Close(void) override;

	// GetBoolean (DbDataReader)
	//
	// Retrieves the specified value as a boolean
	virtual bool GetBoolean(int ordinal) override;

	// GetBinaryReader
	//
	// Retrieves the specified value as a special SqliteBinaryReader object
	SqliteBinaryReader^ GetBinaryReader(int ordinal);

	// GetByte (DbDataReader)
	//
	// Retrieves the specified value as a byte
	virtual System::Byte GetByte(int ordinal) override;

	// GetBytes (DbDataReader)
	//
	// Copies the specified value into an existing array of bytes
	virtual __int64 GetBytes(int ordinal, __int64 fieldOffset, array<System::Byte>^ buffer, int bufferOffset, int count) override;
	
	// GetChar (DbDataReader)
	//
	// Retrieves the specified value as a single character
	virtual Char GetChar(int ordinal) override;

	// GetChars (DbDataReader)
	//
	// Copies the specified value into an existing array of characters
	virtual __int64 GetChars(int ordinal, __int64 fieldOffset, array<Char>^ buffer, int bufferOffset, int count) override;

	// GetDataTypeName (DbDataReader)
	//
	// Retrieves the data type name of the column, as defined in the table schema
	virtual String^ GetDataTypeName(int ordinal) override;

	// GetDateTime (DbDataReader)
	//
	// Retrieves the specified value as a DateTime
	virtual DateTime GetDateTime(int ordinal) override;

	// GetDecimal (DbDataReader)
	//
	// Retrieves the specified value as a Decimal
	virtual Decimal GetDecimal(int ordinal) override;

	// GetDouble (DbDataReader)
	//
	// Retrieves the specified value as a double precision IEEE float
	virtual double GetDouble(int ordinal) override;

	// GetEnumerator (DbDataReader)
	//
	// Retrieves an enumerator to iterate over the rows in the result set
	virtual Collections::IEnumerator^ GetEnumerator(void) override;

	// GetFieldType (DbDataReader)
	//
	// Gets the System.Type of the object that will be returned from GetValue
	virtual Type^ GetFieldType(int ordinal) override;

	// GetFloat (DbDataReader)
	//
	// Retrieves the specified value as a single precision IEEE float
	virtual float GetFloat(int ordinal) override;

	// GetGuid (DbDataReader)
	//
	// Retrieves the specified value as a GUID
	virtual Guid GetGuid(int ordinal) override;

	// GetInt (DbDataReader)
	//
	// Retrieves the specified value as a 16 bit signed integer
	virtual short GetInt16(int ordinal) override;
	
	// GetInt32 (DbDataReader)
	//
	// Retrieves the specified value as a 32 bit signed integer
	virtual int GetInt32(int ordinal) override;
	
	// GetInt64 (DbDataReader)
	//
	// Retrieves the specified value as a 64 bit signed integer
	virtual __int64 GetInt64(int ordinal) override;
	
	// GetName (DbDataReader)
	//
	// Looks up the name of a column based on it's ordinal value
	virtual String^ GetName(int ordinal) override;
	
	// GetOrdinal (DbDataReader)
	//
	// Attempts to map a column name into an ordinal
	virtual int GetOrdinal(String^ name) override;

	// GetProviderSpecificFieldType (DbDataReader)
	//
	// Returns the provider-specific field type of the specified column
	virtual Type^ GetProviderSpecificFieldType(int ordinal) override;
	
	// GetProviderSpecificValue (DbDataReader)
	//
	// Returns the provider-specific value of the specified column
	virtual Object^ GetProviderSpecificValue(int ordinal) override;

	// GetProviderSpecificValues (DbDataReader)
	//
	// Gets all of the provider-specific values from the current row
	virtual int GetProviderSpecificValues(array<Object^>^ values) override;

	// GetSchemaTable (DbDataReader)
	//
	// Gets a DataTable that describes the column metadata of the data reader
	virtual DataTable^ GetSchemaTable(void) override;
	
	// GetString (DbDataReader)
	//
	// Retrieves the specified value as a string
	virtual String^ GetString(int ordinal) override;
	
	// GetValue (DbDataReader)
	//
	// Retrieves the specified value as a generic object, using the
	// "best" data type we can figure out for it
	virtual Object^ GetValue(int ordinal) override;
	
	// GetValues (DbDataReader)
	//
	// Retrieves all of the field data as an array of objects
	virtual int GetValues(array<Object^>^ values) override;

	// IsDBNull (DbDataReader)
	//
	// Determines if the value of the specified column is NULL
	virtual bool IsDBNull(int ordinal) override;

	// NextResult (DbDataReader)
	//
	// Moves the data reader to the next result set
	virtual bool NextResult(void) override;

	// Read (DbDataReader)
	//
	// Advances the reader to the next row in the result set
	virtual bool Read(void) override;

	//-----------------------------------------------------------------------
	// Properties

	// Depth (DbDataReader)
	//
	// Exposes the current depth of the DataReader
	virtual property int Depth { int get(void) override; }

	// FieldCount (DbDataReader)
	//
	// Gets the number of columns in the current row
	virtual property int FieldCount { int get(void) override; }

	// HasRows (DbDataReader)
	//
	// Determines if there are any rows in the current data reader
	virtual property bool HasRows { bool get(void) override; }

	// IsClosed (DbDataReader)
	//
	// Determines if this data reader is closed or not
	virtual property bool IsClosed { bool get(void) override { return m_disposed; } }

	// RecordsAffected (DbDataReader)
	//
	// Determines the number of rows affected by the current result set
	virtual property int RecordsAffected { int get(void) override; }

	// VisibleFieldCount (DbDataReader)
	//
	// Gets the number of non-hidden fields in the result set
	virtual property int VisibleFieldCount { int get(void) override { return FieldCount; } }

	//-----------------------------------------------------------------------
	// Indexers

	// via int (DbDataReader)
	//
	// Gets a field value by ordinal
	virtual property Object^ default[int] { Object^ get(int ordinal) override; }

	// via String^ (DbDataReader)
	//
	// Gets a field value by name
	virtual property Object^ default[String^] { Object^ get(String^ name) override; }

internal:

	// INTERNAL CONSTRUCTORS
	SqliteDataReader(SqliteCommand^ command, SqliteQuery^ query, SqliteCommandBehavior behavior);
	SqliteDataReader(SqliteCommand^ command, String^ query, SqliteCommandBehavior behavior);

private:

	// DESTRUCTOR
	~SqliteDataReader();

	//-----------------------------------------------------------------------
	// Private Member Functions

	// CheckStatementStatus
	//
	// Simple helper function used to check a SqliteStatement status
	static void CheckStatementStatus(SqliteStatement^ statement);

	// IsCommandBehavior
	//
	// Determines if the specified command behavior flag is set
	bool IsCommandBehavior(SqliteCommandBehavior behavior);

	// IsObjectDisposed (ITrackableObject)
	//
	// Exposes the object's internal disposal status
	virtual bool IsObjectDisposed(void) sealed = ITrackableObject::IsDisposed
	{ 
		return m_disposed; 
	}

	//-----------------------------------------------------------------------
	// Member Variables

	bool						m_disposed;			// Object disposal flag
	SqliteConnection^				m_conn;				// Database connection
	__int64						m_cookie;			// Registration cookie
	SqliteQuery^					m_query;			// Contained compiled query
	SqliteCommandBehavior			m_behavior;			// Reader behavior flags
	bool						m_disposeQuery;		// Dispose of query on close?
	int							m_stmtIndex;		// Current statement index
	SqliteStatement^				m_stmt;				// Current statement object
	int							m_changes;			// Overall records affected
	SqliteParameterCollection^		m_params;			// Contained parameters collection
};

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)

#endif		// __SQLITEDATAREADER_H_
