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

#ifndef __SQLITESTATEMENT_H_
#define __SQLITESTATEMENT_H_
#pragma once

#include "ITrackableObject.h"			// Include ITrackableObject decls
#include "ObjectTracker.h"				// Include ObjectTracker decls
#include "StatementHandle.h"			// Include StatementHandle decls
#include "SqliteBinaryReader.h"			// Include SqliteBinaryReader decls
#include "SqliteEnumerations.h"			// Include Sqlite enumeration decls
#include "SqliteExceptions.h"				// Include Sqlite exception delcarations
#include "SqliteException.h"				// Include SqliteException declarations
#include "SqliteParameter.h"				// Include SqliteParameter decls
#include "SqliteParameterCollection.h"		// Include SqliteParameterCollection decls
#include "SqliteStatementMetaData.h"		// Include SqliteStatementMetaData decls
#include "SqliteType.h"					// Include SqliteType declarations
#include "SqliteUtil.h"					// Include SqliteUtil declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings
#pragma warning(disable:4100)			// "unreferenced formal parameter"

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Data;
using namespace System::Runtime::InteropServices;

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// Forward Class Declarations
//---------------------------------------------------------------------------

ref class SqliteConnection;				// SqliteConnection.h

//---------------------------------------------------------------------------
// Class SqliteStatement
//
// SqliteStatement implments a wrapper around a single compiled SQL statement
// and all of the fun things you can do with it
//---------------------------------------------------------------------------

ref class SqliteStatement sealed : public IDataRecord
{
public:

	// Constructor
	//
	// Accepts an existing SQLITE statement handle as well as the SQL text
	// that is associated with this specific statement
	SqliteStatement(StatementHandle* pStatement, String^ sql);

	//-----------------------------------------------------------------------
	// Member Functions

	// BindParameters
	//
	// Binds all of the parameters in a SqliteParameterCollection to this statement
	void BindParameters(SqliteParameterCollection^ params, SqliteConnection^ conn);

	// ExecuteNonQuery
	//
	// Executes the statement as a non-query and returns the affected row count
	int	ExecuteNonQuery(void);

	// ExecuteScalar
	//
	// Executes the statement and extrapolates a scalar result from it
	Object^	ExecuteScalar(void);

	// GetBinary
	//
	// Retrieves an instance of the special SqliteBinaryReader class
	SqliteBinaryReader^ GetBinaryReader(int ordinal);

	// GetBoolean (IDataRecord)
	//
	// Retrieves the specified value as a boolean
	virtual bool GetBoolean(int ordinal);

	// GetByte (IDataRecord)
	//
	// Retrieves the specified value as a byte
	virtual System::Byte GetByte(int ordinal);

	// GetBytes (IDataRecord)
	//
	// Copies the specified value into an existing array of bytes
	virtual __int64 GetBytes(int ordinal, __int64 fieldOffset, array<System::Byte>^ buffer, int bufferOffset, int count);
	
	// GetChar (IDataRecord)
	//
	// Retrieves the specified value as a single character
	virtual Char GetChar(int ordinal);

	// GetChars (IDataRecord)
	//
	// Copies the specified value into an existing array of characters
	virtual __int64 GetChars(int ordinal, __int64 fieldOffset, array<Char>^ buffer, int bufferOffset, int count);

	// GetData (IDataRecord)
	//
	// Not supported.  Would potentially be used for getting a nested DataReader
	virtual IDataReader^ GetData(int index) { throw gcnew NotSupportedException(); }

	// GetDataTypeName (IDataRecord)
	//
	// Retrieves the data type name of the column, as defined in the table schema
	virtual String^ GetDataTypeName(int ordinal);

	// GetDateTime (IDataRecord)
	//
	// Retrieves the specified value as a DateTime
	virtual DateTime GetDateTime(int ordinal);

	// GetDecimal (IDataRecord)
	//
	// Retrieves the specified value as a Decimal
	virtual Decimal GetDecimal(int ordinal);

	// GetDouble (IDataRecord)
	//
	// Retrieves the specified value as a double precision IEEE float
	virtual double GetDouble(int ordinal);

	// GetFieldType (IDataRecord)
	//
	// Gets the System.Type of the object that will be returned from GetValue
	virtual Type^ GetFieldType(int ordinal);

	// GetFloat (IDataRecord)
	//
	// Retrieves the specified value as a single precision IEEE float
	virtual float GetFloat(int ordinal);

	// GetGuid (IDataRecord)
	//
	// Retrieves the specified value as a GUID
	virtual Guid GetGuid(int ordinal);

	// GetInt (IDataRecord)
	//
	// Retrieves the specified value as a 16 bit signed integer
	virtual short GetInt16(int ordinal);
	
	// GetInt32 (IDataRecord)
	//
	// Retrieves the specified value as a 32 bit signed integer
	virtual int GetInt32(int ordinal);
	
	// GetInt64 (IDataRecord)
	//
	// Retrieves the specified value as a 64 bit signed integer
	virtual __int64 GetInt64(int ordinal);
	
	// GetName (IDataRecord)
	//
	// Looks up the name of a column based on it's ordinal value
	virtual String^ GetName(int ordinal);
	
	// GetOrdinal (IDataRecord)
	//
	// Attempts to map a column name into an ordinal
	virtual int GetOrdinal(String^ name);

	// GetProviderSpecificFieldType
	//
	// Gets the underlying provider-specific field type for a column
	Type^ GetProviderSpecificFieldType(int ordinal);

	// GetProviderSpecificValue
	//
	// Get the specified value as the type from GetProviderSpecificFieldType()
	Object^ GetProviderSpecificValue(int ordinal);
	
	// GetProviderSpecificValues
	//
	// Get all fields as an array of provider-specific data type objects
	virtual int GetProviderSpecificValues(array<Object^>^ values);

	// GetSchemaTable
	//
	// Generates a DataTable with result set schema information
	DataTable^ GetSchemaTable(void);
	
	// GetString (IDataRecord)
	//
	// Retrieves the specified value as a string
	virtual String^ GetString(int ordinal);
	
	// GetValue (IDataRecord)
	//
	// Retrieves the specified value as the type from GetFieldType()
	virtual Object^ GetValue(int ordinal);
	
	// GetValues (IDataRecord)
	//
	// Retrieves all of the field data as an array of objects
	virtual int GetValues(array<Object^>^ values);

	// IsDBNull (IDataRecord)
	//
	// Determines if the value of the specified column is NULL
	virtual bool IsDBNull(int ordinal);

	// Reset
	//
	// Forces a reset of the statement handle
	void Reset(void);

	// Step
	//
	// Executes the next step of the statement and returns an updated status code
	SqliteStatementStatus	Step(void);

	//-----------------------------------------------------------------------
	// Properties

	// ChangeCount
	//
	// Retrieves the number of rows affected by the statement
	property int ChangeCount { int get(void); }
	
	// FieldCount (IDataRecord)
	//
	// Gets the number of columns in the statement's result set
	virtual property int FieldCount { int get(void); }

	// GeneratesResultSet
	//
	// Determines if this statement will generate a result set or not
	property bool GeneratesResultSet { bool get(void); }

	// Sql
	//
	// Gets a reference to the contained SQL text for this statement
	property String^ Sql { String^ get(void); }

	// Status
	//
	// Exposes the current status of the statement
	property SqliteStatementStatus Status { SqliteStatementStatus get(void); }

	//-----------------------------------------------------------------------
	// Indexers

	// via int (IDataRecord)
	//
	// Gets a field value by ordinal
	virtual property Object^ default[int] { Object^ get(int ordinal); }

	// via String^ (IDataRecord)
	//
	// Gets a field value by name
	virtual property Object^ default[String^] { Object^ get(String^ name); }

private:

	// DESTRUCTOR / FINALIZER
	~SqliteStatement();
	!SqliteStatement();

	//-----------------------------------------------------------------------
	// Private Member Functions

	// BindBinaryParameter
	//
	// Binds a binary parameter from the collection to the statement
	void BindBinaryParameter(SqliteParameter^ param, int index, array<System::Byte>^ value, int length);

	// BindBooleanParameter
	//
	// Binds a boolean parameter from the collection to the statement
	void BindBooleanParameter(SqliteParameter^ param, int index, bool value, SqliteBooleanFormat format);

	// BindDateTimeParameter
	//
	// Binds a date/time parameter from the collection to the statement
	void BindDateTimeParameter(SqliteParameter^ param, int index, DateTime value, SqliteDateTimeFormat format);

	// BindFloatParameter
	//
	// Binds a floating point parameter from the collection to the statement
	void BindFloatParameter(SqliteParameter^ param, int index, double value);

	// BindGuidParameter
	//
	// Binds a GUID parameter from the collection to the statement
	void BindGuidParameter(SqliteParameter^ param, int index, Guid value, SqliteGuidFormat format);

	// BindIntegerParameter
	//
	// Binds a 32 bit integer parameter from the collection to the statement
	void BindIntegerParameter(SqliteParameter^ param, int index, int value);

	// BindLongParameter
	//
	// Binds a 64 bit integer parameter from the collection to the statement
	void BindLongParameter(SqliteParameter^ param, int index, __int64 value);

	// BindNullParameter
	//
	// Binds a NULL parameter from the collection to the statement
	void BindNullParameter(SqliteParameter^ param, int index);

	// BindStringParameter
	//
	// Binds a string parameter from the collection to the statement
	void BindStringParameter(SqliteParameter^ param, int index, String^ value, int length);

	// FormatBoolean
	//
	// Massages a boolean value based on a SqliteBooleanFormat
	static Object^ FormatBoolean(bool value, SqliteBooleanFormat format);

	// FormatDateTime
	//
	// Massages a DateTime value based on a SqliteDateTimeFormat
	static Object^ FormatDateTime(DateTime value, SqliteDateTimeFormat format);

	// FormatGuid
	//
	// Massages a Guid value based on a SqliteGuidFormat
	static Object^ FormatGuid(Guid value, SqliteGuidFormat format);

	// GetValueAs
	//
	// Retrieves the specified value as the specified data type
	Object^ GetValueAs(int ordinal, Type^ type);
	
	// RecompileStatement
	//
	// Recompiles the statement after a step fails with a schema error
	void RecompileStatement(void);

	//-----------------------------------------------------------------------
	// Member Variables
	
	bool						m_disposed;		// Object disposal flag
	StatementHandle*			m_pStatement;	// Statement handle wrapper
	SqliteStatementMetaData^		m_metadata;		// Statement metadata info
	String^						m_sql;			// Statement SQL command text
	SqliteStatementStatus			m_status;		// Current statement status
	int							m_changes;		// Rows affected by query
	List<GCHandle>^				m_pins;			// Pinned parameters
	List<ITrackableObject^>^	m_binaries;		// Open SqliteBinaryReaders
};

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)

#endif		// __SQLITESTATEMENT_H_
