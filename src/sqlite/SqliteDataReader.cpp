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
#include "SqliteDataReader.h"			// Include SqliteDataReader declarations
#include "SqliteConnection.h"			// Include SqliteConnection declarations
#include "SqliteCommand.h"				// Include SqliteCommand declarations

#pragma warning(push, 4)			// Enable maximum compiler warnings

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// SqliteDataReader Constructor (internal; compiled query)
//
// Arguments:
//
//	command			- Parent eDBCommand object
//	query			- Reference to an existing compiled query object
//	behavior		- Reader behavior flags

SqliteDataReader::SqliteDataReader(SqliteCommand^ command, SqliteQuery^ query, SqliteCommandBehavior behavior) 
	: m_behavior(behavior), m_query(query), m_disposeQuery(false)
{
	Debug::Assert(command != nullptr);				// Should never be NULL
	Debug::Assert(m_query != nullptr);				// Should never be NULL

	try {

		// We need to maintain references to both the parent connection as well
		// as the command's parameter collection while this is open.  By registering
		// with the connection object below, we ensure we'll get nuked if it closes,
		// and by locking the parameter collection we ensure that can't change.

		m_conn = command->Connection;
		m_params = command->Parameters;

		SqliteUtil::CheckConnectionOpen(m_conn);			// Check connection status
		m_cookie = m_conn->RegisterDataReader(this);	// Register this data reader
		m_params->Lock();								// Lock down all parameters

		NextResult();									// Move to first result set
	}

	catch(Exception^) { this->~SqliteDataReader(); throw; }
}

//---------------------------------------------------------------------------
// SqliteDataReader Constructor (internal; string query)
//
// Arguments:
//
//	command			- The parent eDBCommand object
//	query			- SQL command text to be compiled and executed
//	behavior		- Reader bahavior flags

SqliteDataReader::SqliteDataReader(SqliteCommand^ command, String^ query, SqliteCommandBehavior behavior) : 
	m_behavior(behavior), m_disposeQuery(true)
{
	Debug::Assert(command != nullptr);

	try {

		// We need to maintain references to both the parent connection as well
		// as the command's parameter collection while this is open.  By registering
		// with the connection object below, we ensure we'll get nuked if it closes,
		// and by locking the parameter collection we ensure that can't change.

		m_conn = command->Connection;
		m_params = command->Parameters;

		SqliteUtil::CheckConnectionOpen(m_conn);			// Check connection status
		m_cookie = m_conn->RegisterDataReader(this);	// Register with the connection
		m_params->Lock();								// Lock down all parameters

		// We need to create a new query object based on the SQL command, since it was
		// not prepared in advance on our behalf by the SqliteCommand object instance

		m_query = gcnew SqliteQuery(m_conn->HandlePointer, query);

		NextResult();									// Move to first result set
	}

	catch(Exception^) { this->~SqliteDataReader(); throw; }
}

//---------------------------------------------------------------------------
// SqliteDataReader Destructor

SqliteDataReader::~SqliteDataReader()
{
	if(m_stmt != nullptr) m_stmt->Reset();	// Reset statement as necessary

	// If the SqliteCommand object was not pre-compiled, the m_query object is
	// something that we compiled in the constructor, and it must be released

	if(m_disposeQuery && (m_query != nullptr)) { delete m_query; m_query = nullptr; }

	// Remove ourselves from the connection's active reader collection, and if
	// requested close that connection.  Note the try/catch since the connection
	// may have been disposed of before we were, and that's not really an error

	try { 
	
		m_conn->UnRegisterDataReader(m_cookie);
		if(IsCommandBehavior(SqliteCommandBehavior::CloseConnection)) m_conn->Close(); 
	}

	catch(Exception^) { /* DO NOTHING */ }

	m_params->Unlock();					// Unlock command parameters
	m_disposed = true;					// Object is now disposed of
}

//---------------------------------------------------------------------------
// SqliteDataReader::Close
//
// Closes the data reader and disposes of this object instance

void SqliteDataReader::Close(void)
{
	if(m_disposed) return;				// Allow this even when disposed

	while(NextResult());				// Process all statements
	delete this;						// Dispose of ourselves
}

//---------------------------------------------------------------------------
// SqliteDataReader::default::get [int]
//
// Retrieves the value located at the specified ordinal

Object^ SqliteDataReader::default::get(int ordinal)
{
	CHECK_DISPOSED(m_disposed);
	return GetValue(ordinal);
}

//---------------------------------------------------------------------------
// SqliteDataReader::default::get [String^]
//
// Retrieves the value located at the specified column

Object^ SqliteDataReader::default::get(String^ name)
{
	CHECK_DISPOSED(m_disposed);
	return GetValue(GetOrdinal(name));
}

//---------------------------------------------------------------------------
// SqliteDataReader::CheckStatementStatus (private, static)
//
// Helper function used to make sure a SqliteStatement is ready or not
//
// Arguments:
//
//	statement		- The statement to be checked

void SqliteDataReader::CheckStatementStatus(SqliteStatement^ statement)
{
	if((statement == nullptr) || (statement->Status != SqliteStatementStatus::ResultReady))
		throw gcnew SqliteExceptions::NoDataPresentException();
}

//---------------------------------------------------------------------------
// SqliteDataReader::Depth::get
//
// Gets the current nesting depth of the data reader

int SqliteDataReader::Depth::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return 0;						// <-- Not really supported
}

//---------------------------------------------------------------------------
// SqliteDataReader::FieldCount::get
//
// Retrieves the number of columns present in the current result set

int SqliteDataReader::FieldCount::get(void)
{
	CHECK_DISPOSED(m_disposed);

	// Judging by the behavior of the SqlDataReader, we return zero here if
	// we're not currently positioned on a valid result set .. don't throw

	return (m_stmt != nullptr) ? m_stmt->FieldCount : 0;
}

//---------------------------------------------------------------------------
// SqliteDataReader::GetBoolean
//
// Retrieves a boolean value from the current row in the current result set
//
// Arguments:
//
//	ordinal			- Column ordinal to retrieve the value from

bool SqliteDataReader::GetBoolean(int ordinal)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckDataRecordOrdinal(this, ordinal);
	CheckStatementStatus(m_stmt);

	return m_stmt->GetBoolean(ordinal);
}

//---------------------------------------------------------------------------
// SqliteDataReader::GetBinaryReader
//
// Retrieves an instance of SqliteBinaryReader from the current result set.  Note
// that the instance only remains valid as long as we're on this row.  It will
// automatically be disposed of when the row pointer moves
//
// Arguments:
//
//	ordinal			- Column ordinal to retrieve the value from

SqliteBinaryReader^ SqliteDataReader::GetBinaryReader(int ordinal)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckDataRecordOrdinal(this, ordinal);
	CheckStatementStatus(m_stmt);

	return m_stmt->GetBinaryReader(ordinal);
}

//---------------------------------------------------------------------------
// SqliteDataReader::GetByte
//
// Retrieves a byte value from the current row in the current result set
//
// Arguments:
//
//	ordinal			- Column ordinal to retrieve the value from

System::Byte SqliteDataReader::GetByte(int ordinal)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckDataRecordOrdinal(this, ordinal);
	CheckStatementStatus(m_stmt);

	return m_stmt->GetByte(ordinal);
}

//---------------------------------------------------------------------------
// SqliteDataReader::GetBytes
//
// Retrieves a specific range of bytes from the current row of the result set
//
// Arguments:
//
//	ordinal			- Ordinal value of the column to be retrieved
//	fieldOffset		- Offset into the column's data to begin reading
//	buffer			- Destination buffer.  Set to NULL to just get length
//	bufferOffset	- Offset into destination buffer to begin writing
//	count			- Maximum number of bytes to copy into the buffer

__int64 SqliteDataReader::GetBytes(int ordinal, __int64 fieldOffset, array<System::Byte>^ buffer, 
	int bufferOffset, int count)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckDataRecordOrdinal(this, ordinal);
	CheckStatementStatus(m_stmt);

	// Just call through to the IDataRecord implementation of the statement object

	return m_stmt->GetBytes(ordinal, fieldOffset, buffer, bufferOffset, count);
}

///---------------------------------------------------------------------------
// SqliteDataReader::GetChar
//
// Retrieves a character value from the current row in the current result set
//
// Arguments:
//
//	ordinal			- Column ordinal to retrieve the value from

__wchar_t SqliteDataReader::GetChar(int ordinal)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckDataRecordOrdinal(this, ordinal);
	CheckStatementStatus(m_stmt);

	return m_stmt->GetChar(ordinal);
}

//---------------------------------------------------------------------------
// SqliteDataReader::GetChars
//
// Retrieves a specific range of chars from the current row of the result set
//
// Arguments:
//
//	ordinal			- Ordinal value of the column to be retrieved
//	fieldOffset		- Offset into the column's data to begin reading
//	buffer			- Destination buffer.  Set to NULL to just get length
//	bufferOffset	- Offset into destination buffer to begin writing
//	count			- Maximum number of bytes to copy into the buffer

__int64 SqliteDataReader::GetChars(int ordinal, __int64 fieldOffset, array<Char>^ buffer, 
	int bufferOffset, int count)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckDataRecordOrdinal(this, ordinal);
	CheckStatementStatus(m_stmt);

	return m_stmt->GetChars(ordinal, fieldOffset, buffer, bufferOffset, count);
}

//---------------------------------------------------------------------------
// SqliteDataReader::GetDataTypeName
//
// Retrieves the underlying data type associated with a result set column
//
// Arguments:
//
//	ordinal			- Column ordinal to retrieve the value from

String^ SqliteDataReader::GetDataTypeName(int ordinal)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckDataRecordOrdinal(this, ordinal);
	if(m_stmt == nullptr) throw gcnew SqliteExceptions::NoDataPresentException();

	return m_stmt->GetDataTypeName(ordinal);
}

//---------------------------------------------------------------------------
// SqliteDataReader::GetDateTime
//
// Retrieves a DateTime value from the current row in the current result set
//
// Arguments:
//
//	ordinal			- Column ordinal to retrieve the value from

DateTime SqliteDataReader::GetDateTime(int ordinal)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckDataRecordOrdinal(this, ordinal);
	CheckStatementStatus(m_stmt);

	return m_stmt->GetDateTime(ordinal);
}

//---------------------------------------------------------------------------
// SqliteDataReader::GetDecimal
//
// Retrieves a signed fixed-point value from the current row in the result set
//
// Arguments:
//
//	ordinal			- Column ordinal to retrieve the value from

Decimal SqliteDataReader::GetDecimal(int ordinal)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckDataRecordOrdinal(this, ordinal);
	CheckStatementStatus(m_stmt);

	return m_stmt->GetDecimal(ordinal);
}

//---------------------------------------------------------------------------
// SqliteDataReader::GetDouble
//
// Retrieves a double precision IEEE floating point value from the current 
// row in the current result set
//
// Arguments:
//
//	ordinal			- Column ordinal to retrieve the value from

double SqliteDataReader::GetDouble(int ordinal)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckDataRecordOrdinal(this, ordinal);
	CheckStatementStatus(m_stmt);

	return m_stmt->GetDouble(ordinal);
}

//---------------------------------------------------------------------------
// SqliteDataReader::GetEnumerator
//
// Returns a new DbEnumerator object that wraps this data reader
//
// Arguments:
//
//	NONE

Collections::IEnumerator^ SqliteDataReader::GetEnumerator(void)
{
	CHECK_DISPOSED(m_disposed);
	return gcnew DbEnumerator(this, IsCommandBehavior(SqliteCommandBehavior::CloseConnection));
}

//---------------------------------------------------------------------------
// SqliteDataReader::GetFieldType
//
// Retrieves the System::Type that would be returned if you were to call
// GetValue() for the specified column
//
// Arguments:
//
//	ordinal			- Column ordinal to retrieve the value from

Type^ SqliteDataReader::GetFieldType(int ordinal)
{
	CHECK_DISPOSED(m_disposed);
	if(m_stmt == nullptr) throw gcnew SqliteExceptions::NoDataPresentException();

	return m_stmt->GetFieldType(ordinal);
}

//---------------------------------------------------------------------------
// SqliteDataReader::GetFloat
//
// Retrieves a single precision IEEE floating point value from the current 
// row in the current result set
//
// Arguments:
//
//	ordinal			- Column ordinal to retrieve the value from

float SqliteDataReader::GetFloat(int ordinal)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckDataRecordOrdinal(this, ordinal);
	CheckStatementStatus(m_stmt);

	return m_stmt->GetFloat(ordinal);
}

//---------------------------------------------------------------------------
// SqliteDataReader::GetGuid
//
// Retrieves a UUID value from the current row in the result set
//
// Arguments:
//
//	ordinal			- Column ordinal to retrieve the value from

Guid SqliteDataReader::GetGuid(int ordinal)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckDataRecordOrdinal(this, ordinal);
	CheckStatementStatus(m_stmt);

	return m_stmt->GetGuid(ordinal);
}

//---------------------------------------------------------------------------
// SqliteDataReader::GetInt16
//
// Retrieves a 16 bit integer value from the current row in the current result set
//
// Arguments:
//
//	ordinal			- Column ordinal to retrieve the value from

short SqliteDataReader::GetInt16(int ordinal)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckDataRecordOrdinal(this, ordinal);
	CheckStatementStatus(m_stmt);

	return m_stmt->GetInt16(ordinal);
}

//---------------------------------------------------------------------------
// ezDataReader::GetInt32
//
// Retrieves a 32 bit integer value from the current row in the current result set
//
// Arguments:
//
//	ordinal			- Column ordinal to retrieve the value from

int SqliteDataReader::GetInt32(int ordinal)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckDataRecordOrdinal(this, ordinal);
	CheckStatementStatus(m_stmt);

	return m_stmt->GetInt32(ordinal);
}

//---------------------------------------------------------------------------
// SqliteDataReader::GetInt64
//
// Retrieves a 64 bit integer value from the current row in the current result set
//
// Arguments:
//
//	ordinal			- Column ordinal to retrieve the value from

__int64 SqliteDataReader::GetInt64(int ordinal)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckDataRecordOrdinal(this, ordinal);
	CheckStatementStatus(m_stmt);

	return m_stmt->GetInt64(ordinal);
}

//---------------------------------------------------------------------------
// SqliteDataReader::GetName
//
// Retrieves the name of a result set column based on it's ordinal
//
// Arguments:
//
//	ordinal			- Column ordinal to look up

String^ SqliteDataReader::GetName(int ordinal)
{
	CHECK_DISPOSED(m_disposed);
	if(m_stmt == nullptr) throw gcnew SqliteExceptions::NoDataPresentException();

	return m_stmt->GetName(ordinal);
}

//---------------------------------------------------------------------------
// SqliteDataReader::GetOrdinal
//
// Retrieves the ordinal value of a result set column based on it's name.
//
// Arguments:
//
//	name			- Name of the column to be looked up

int SqliteDataReader::GetOrdinal(String^ name)
{
	CHECK_DISPOSED(m_disposed);
	if(m_stmt == nullptr) throw gcnew SqliteExceptions::NoDataPresentException();

	return m_stmt->GetOrdinal(name);
}

//---------------------------------------------------------------------------
// SqliteDataReader::GetProviderSpecificFieldType
//
// Retrieves the System::Type that represents the underlying provider type
// for a specific column.  Not sure what good this method really is
//
// Arguments:
//
//	ordinal			- Column ordinal to retrieve the value from

Type^ SqliteDataReader::GetProviderSpecificFieldType(int ordinal)
{
	CHECK_DISPOSED(m_disposed);
	if(m_stmt == nullptr) throw gcnew SqliteExceptions::NoDataPresentException();

	return m_stmt->GetProviderSpecificFieldType(ordinal);
}

//---------------------------------------------------------------------------
// SqliteDataReader::GetProviderSpecificValue
//
// Retrieves the value at the specified ordinal using the provider-specific
// data type instead of the standard .NET data type
//
// Arguments:
//
//	ordinal			- Column ordinal to retrieve the value from

Object^ SqliteDataReader::GetProviderSpecificValue(int ordinal)
{
	CHECK_DISPOSED(m_disposed);
	if(m_stmt == nullptr) throw gcnew SqliteExceptions::NoDataPresentException();

	return m_stmt->GetProviderSpecificValue(ordinal);
}

//---------------------------------------------------------------------------
// SqliteDataReader::GetProviderSpecificValues
//
// Retrieves an array of objects from the current row in the result set.  The
// data type of each value can be retrieved with GetProviderSpecificFieldType.  
// Note that if the specified column in the result set is NULL, the caller will 
// get back a DBNull::Value object, not something of the type reported.
//
// Arguments:
//
//	values			- The array of Object references to load with row data

int SqliteDataReader::GetProviderSpecificValues(array<Object^>^ values)
{
	CHECK_DISPOSED(m_disposed);
	if(m_stmt == nullptr) throw gcnew SqliteExceptions::NoDataPresentException();

	return m_stmt->GetProviderSpecificValues(values);
}

//---------------------------------------------------------------------------
// SqliteDataReader::GetSchemaTable
//
// Generates a DataTable containing schema information about the current
// result set pointed to by the data reader
//
// Arguments:
//
//	NONE

DataTable^ SqliteDataReader::GetSchemaTable(void)
{
	CHECK_DISPOSED(m_disposed);
	if(m_stmt == nullptr) throw gcnew SqliteExceptions::NoDataPresentException();

	return m_stmt->GetSchemaTable();		// Generate a schema table
}

//---------------------------------------------------------------------------
// SqliteDataReader::GetString
//
// Retrieves a string value from the current row in the current result set
//
// Arguments:
//
//	ordinal			- Column ordinal to retrieve the value from

String^ SqliteDataReader::GetString(int ordinal)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckDataRecordOrdinal(this, ordinal);
	CheckStatementStatus(m_stmt);

	return m_stmt->GetString(ordinal);
}

//---------------------------------------------------------------------------
// SqliteDataReader::GetValue
//
// Retrieves a value from the current row in the current result set.  The
// data type of the value can be retrieved with GetFieldType.  Note that if
// the specified column of the result set is NULL, the caller will get back
// a DBNull::Value object, NOT something of the type reported by GetFieldType.
//
// Arguments:
//
//	ordinal			- Column ordinal to retrieve the value from

Object^ SqliteDataReader::GetValue(int ordinal)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckDataRecordOrdinal(this, ordinal);
	CheckStatementStatus(m_stmt);

	return m_stmt->GetValue(ordinal);			// Call through to get value
}

//---------------------------------------------------------------------------
// SqliteDataReader::GetValues
//
// Retrieves an array of objects from the current row in the result set.  The
// data type of each value can be retrieved with GetFieldType.  Note that if
// the specified column in the result set is NULL, the caller will get back
// a DBNull::Value object, not something of the type reported by GetFieldType
//
// Arguments:
//
//	values			- The array of Object references to load with row data

int SqliteDataReader::GetValues(array<Object^>^ values) 
{
	CHECK_DISPOSED(m_disposed);
	CheckStatementStatus(m_stmt);

	return m_stmt->GetValues(values);
}

//---------------------------------------------------------------------------
// SqliteDataReader::HasRows
//
// Determines if this data reader has any rows or not

bool SqliteDataReader::HasRows::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return ((m_stmt != nullptr) && (m_stmt->Status == SqliteStatementStatus::ResultReady));
}

//---------------------------------------------------------------------------
// SqliteDataReader::IsCommandBehavior (private)
//
// Determines if a specific SqliteCommandBehavior flag is set or not
//
// Arguments:
//
//	behavior		- CommandBehavior mask to be tested

bool SqliteDataReader::IsCommandBehavior(SqliteCommandBehavior flag)
{
	return ((m_behavior & flag) == flag);
}

//---------------------------------------------------------------------------
// SqliteDataReader::IsDBNull
//
// Determines if the specified column in the result set is NULL
//
// Arguments:
//
//	ordinal			- Column ordinal to retrieve the value from

bool SqliteDataReader::IsDBNull(int ordinal)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckDataRecordOrdinal(this, ordinal);
	CheckStatementStatus(m_stmt);

	return m_stmt->IsDBNull(ordinal);
}

//---------------------------------------------------------------------------
// SqliteDataReader::NextResult
//
// Moves the data reader to the next result set present in the current query,
// or returns FALSE if there are no more result sets
//
// Arguments:
//
//	NONE

bool SqliteDataReader::NextResult(void)
{
	bool			skipResults = false;			// Flag to skip all results

	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckConnectionOpen(m_conn);

	if(m_stmt != nullptr) {
	
		m_changes += m_stmt->ChangeCount;	// Tally up the total changes
		m_stmt->Reset();					// Reset executing statement

		// If the command is supposed to only return one result set, and
		// we're in here because m_stmt was non-NULL, we're going to want 
		// to skip all remaining result sets below

		if(IsCommandBehavior(SqliteCommandBehavior::SingleResult)) skipResults = true;
	}

	while(m_stmtIndex < m_query->StatementCount) {

		m_stmt = m_query[m_stmtIndex];
		m_stmt->BindParameters(m_params, m_conn);
		m_stmtIndex++;

		// If we are set up in SchemaOnly mode, we want to still return TRUE
		// for each statement that will generate a result set, but never
		// actually execute ANY of the statements.  [.Read() is also hooked
		// up to not work at all for CommandBehavior::SchemaOnly]

		if(IsCommandBehavior(SqliteCommandBehavior::SchemaOnly)) {

			if(m_stmt->GeneratesResultSet) return true;
			else continue;
		}

		// If the next statment generates a result set, stop walking the
		// collection and return TRUE back to the caller.  If not, treat
		// it as a non-query, execute it, and move on to the next one

		if((m_stmt->GeneratesResultSet) && (!skipResults)) return true;
		else m_changes += m_stmt->ExecuteNonQuery();
	}

	m_stmt = nullptr;						// No more statement left
	return false;							// No more statements left
}

//---------------------------------------------------------------------------
// SqliteDataReader::Read
//
// Moves to next result set row in the currently executing statement
//
// Arguments:
//
//	NONE

bool SqliteDataReader::Read(void)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckConnectionOpen(m_conn);
	
	// If the defined reader behavior includes SchemaOnly, we don't actually
	// do anything here.  Nothing at all ...

	if(IsCommandBehavior(SqliteCommandBehavior::SchemaOnly)) return false;

	// If there is no current statement being executed, or it has completed
	// successfully, just return FALSE to the caller (not an exception)

	if(m_stmt == nullptr) return false;
	if(m_stmt->Status == SqliteStatementStatus::Completed) return false;

	// If we're set up in SingleRow mode, we only read the first row (as in
	// status == Prepared right now) from the result set for the caller

	if(IsCommandBehavior(SqliteCommandBehavior::SingleRow)) {
		if(m_stmt->Status != SqliteStatementStatus::Prepared) return false;
	}

	// Move the statement along a step.  If a row is returned, return TRUE

	return (m_stmt->Step() == SqliteStatementStatus::ResultReady);
}

//---------------------------------------------------------------------------
// SqliteDataReader::RecordsAffected::get
//
// Returns the current running total of rows affected by the query

int SqliteDataReader::RecordsAffected::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return m_changes;
}

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)
