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
#include "SqliteCommand.h"				// Include SqliteCommand declarations
#include "SqliteConnection.h"			// Include SqliteConnection declarations
#include "SqliteDataAdapter.h"			// Include SqliteDataAdapter declarations
#include "SqliteDataReader.h"			// Include SqliteDataReader declarations
#include "SqliteTransaction.h"			// Include SqliteTransaction declarations

#pragma warning(push, 4)			// Enable maximum compiler warnings

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// SqliteCommand Destructor

SqliteCommand::~SqliteCommand()
{
	// If there is an active data reader hanging out, dispose of it.  This
	// probably isn't standard provider behavior since DbCommand is not a
	// disposable object by default.  I prefer to be able to control this.

	if(m_readerTracker->IsAlive) delete m_readerTracker->Instance;
	delete m_readerTracker;

	UncompileQuery();				// Uncompile any compiled queries
	m_disposed = true;				// Object is now disposed of
}

//---------------------------------------------------------------------------
// SqliteCommand::Cancel
//
// Tries to cancel any pending operations against the database. Calls through
// to the attached SqliteConnection object to do the work, since there is no
// command-level implementation of this concept in SQLite.
//
// Arguments:
//
//	NONE

void SqliteCommand::Cancel(void)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckConnectionValid(m_conn);

	m_conn->Interrupt();			// Attempt to interrupt the engine
}

//---------------------------------------------------------------------------
// SqliteCommand::CommandText::get
//
// Retrieves a copy of the currently set SQL command text

String^ SqliteCommand::CommandText::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return m_commandText;
}

//---------------------------------------------------------------------------
// SqliteCommand::CommandText::set
//
// Changes the SQL command text to be exeuted by this command

void SqliteCommand::CommandText::set(String^ value)
{
	CHECK_DISPOSED(m_disposed);
	if(m_readerTracker->IsAlive) throw gcnew SqliteExceptions::OpenReaderException();

	UncompileQuery();					// Must uncompile if command changes
	m_commandText = value;				// Change contained command text
}

//---------------------------------------------------------------------------
// SqliteCommand::CommandTimeout::get
//
// Gets the configured number of seconds that the command will wait to 
// execute if the database is busy

int SqliteCommand::CommandTimeout::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return m_timeout;
}

//---------------------------------------------------------------------------
// SqliteCommand::CommandTimeout::set
//
// Sets the number of seconds that the command will wait to execute if the
// database is busy

void SqliteCommand::CommandTimeout::set(int value)
{
	CHECK_DISPOSED(m_disposed);
	if(m_readerTracker->IsAlive) throw gcnew SqliteExceptions::OpenReaderException();

	if(m_timeout < 0) throw gcnew ArgumentOutOfRangeException();
	m_timeout = value;
}

//---------------------------------------------------------------------------
// SqliteCommand::CommandType::get
//
// Returns the type of command this is

SqliteCommandType SqliteCommand::CommandType::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return m_commandType;
}

//---------------------------------------------------------------------------
// SqliteCommand::CommandType::set
//
// Sets the type of command this is, which alters the CommandText format
// and perhaps even the behavior at some point

void SqliteCommand::CommandType::set(SqliteCommandType value)
{
	CHECK_DISPOSED(m_disposed);
	if(m_readerTracker->IsAlive) throw gcnew SqliteExceptions::OpenReaderException();
	
	if(value == m_commandType) return;			// Same, don't do anything at all

	// Currently we only support TEXT and TABLEDIRECT command types.  This might
	// be extended if I can come up with anything interesting to extend it with.
	// If so, SqliteCommandType should probably get SqliteType treatment with a nice
	// hearty value struct instead of an enumeration

	if((value != SqliteCommandType::Text) && (value != SqliteCommandType::TableDirect))
		throw gcnew SqliteExceptions::CommandTypeUnknownException(value);

	UncompileQuery();				// Must uncompile if type changes
	m_commandType = value;			// Change the stored command type
}

//---------------------------------------------------------------------------
// SqliteCommand::Connection::get
//
// Retrieves a reference to the connection this command is associated with

SqliteConnection^ SqliteCommand::Connection::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return m_conn;
}

//---------------------------------------------------------------------------
// SqliteCommand::Connection::set
//
// Changes the SqliteConnection object that will be used to execute this command.
// Any previously compiled query created with Prepare() will be lost.

void SqliteCommand::Connection::set(SqliteConnection^ value)
{
	CHECK_DISPOSED(m_disposed);
	if(m_readerTracker->IsAlive) throw gcnew SqliteExceptions::OpenReaderException();

	if(m_conn == value) return;				// Same, don't do anything at all

	UncompileQuery();			// Must uncompile if connection changes
	m_conn = value;				// Changed stored connection reference
}

//---------------------------------------------------------------------------
// SqliteCommand::Construct (private)
//
// Acts as a pseudo-constructor for the class due to the number of real
// constructor overloads to deal with.
//
// Arguments:
//
//	commandText		- SQL command text to automatically set
//	conn			- SqliteConnection object to automatically set
//	type			- SqliteCommandType to automatically set

void SqliteCommand::Construct(String^ commandText, SqliteConnection^ conn, SqliteCommandType type)
{
	m_designTimeVisible = true;
	m_updatedRowSource = SqliteUpdateRowSource::FirstReturnedRecord;
	m_timeout = DEFAULT_TIMEOUT;

	// Construct the SqliteParameterCollection local to this command as
	// well as the special ObjectTracker we use to keep an eye on readers

	m_params = gcnew SqliteParameterCollection();
	m_readerTracker = gcnew ObjectTracker();

	CommandText = commandText;					// Apply command text
	Connection = conn;							// Apply connection
	CommandType = type;							// Apply command type
}

//---------------------------------------------------------------------------
// SqliteCommand::CreateDbParameter (protected)
//
// Creates a new parameter object for this command on it's generic interface
//
// Arguments:
//
//	NONE

DbParameter^ SqliteCommand::CreateDbParameter(void)
{
	CHECK_DISPOSED(m_disposed);
	return gcnew SqliteParameter();
}

//---------------------------------------------------------------------------
// SqliteCommand::DbConnection::get (protected)
//
// Retrieves the stored connection object's generic interface

Data::Common::DbConnection^ SqliteCommand::DbConnection::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return Connection;			// Invoke the strongly-typed property
}

//---------------------------------------------------------------------------
// SqliteCommand::DbConnection::set (protected)
//
// Sets the stored connection object via the generic interface

void SqliteCommand::DbConnection::set(Data::Common::DbConnection^ value)
{
	CHECK_DISPOSED(m_disposed);

	// If the provided connection object isn't a SqliteConnection, something
	// has gone totally whack with the DbCommand base class ...

	Connection = safe_cast<SqliteConnection^>(value);
}

//---------------------------------------------------------------------------
// SqliteCommand::DbParameterCollection::get (protected)
//
// Gets a reference to the parameters collection on the generic interface

Data::Common::DbParameterCollection^ SqliteCommand::DbParameterCollection::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return m_params;
}

//---------------------------------------------------------------------------
// SqliteCommand::ExecuteDbDataReader (protected)
//
// Executes the command and returns a generic data reader to process it
//
// Arguments:
//
//	behavior		- DataReader command behavior flags

DbDataReader^ SqliteCommand::ExecuteDbDataReader(CommandBehavior behavior)
{
	CHECK_DISPOSED(m_disposed);
	return ExecuteReader(static_cast<SqliteCommandBehavior>(behavior));
}

//---------------------------------------------------------------------------
// SqliteCommand::ExecuteNonQuery
//
// Executes a SQL statement against the current connection and returns
// the number of rows that were affected by that statement
//
// Arguments:
//
//	NONE

int SqliteCommand::ExecuteNonQuery(void)
{
	SqliteQuery^				query;				// Reference to the query object
	int						changes = 0;		// Total number of changes by query
	int						nResult;			// Result from function call

	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckConnectionReady(m_conn);

	if(m_readerTracker->IsAlive) throw gcnew SqliteExceptions::OpenReaderException();

	// Only perform the security demand if Prepare() hasn't already been called.
	// This is actually an expensive operation, and this helps alot without
	// compromising anything security-wise

	if(m_compiledQuery == nullptr) SqliteConnection::ExecutePermission->Demand();

	// If we already have a compiled query, use it.  Otherwise, we need to compile
	// a new query based on the current CommandText property value

	if(m_compiledQuery != nullptr) query = m_compiledQuery;
	else query = gcnew SqliteQuery(m_conn->HandlePointer, GetCommandText());

	try { 
		
		m_params->Lock();						// Lock down the parameters

		try {

			// Always set the busy timeout immediately before executing the command

			nResult = sqlite3_busy_timeout(m_conn->Handle, m_timeout * 1000);
			if(nResult != SQLITE_OK) throw gcnew SqliteException(m_conn->Handle, nResult);

			// For every statement in the compiled query, bind the local parameter
			// collection to it, and execute it.  It takes care of everything else

			for each(SqliteStatement^ statement in query) {
				
				statement->BindParameters(m_params, m_conn);		
				changes += statement->ExecuteNonQuery();
			}
		}

		finally { m_params->Unlock(); }					// Unlock the parameters

	} // outer try

	finally { if(query != m_compiledQuery) delete query; }

	return changes;							// Return the total change count
}

//---------------------------------------------------------------------------
// SqliteCommand::ExecuteReader
//
// Executes the compiled query and returns a SqliteDataReader that can be used
// to access the result set(s)
//
// Arguments:
//
//	behavior			- SqliteCommandBehavior to be used for this reader

SqliteDataReader^ SqliteCommand::ExecuteReader(SqliteCommandBehavior behavior)
{
	SqliteDataReader^				reader;		// The new SqliteDataReader object

	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckConnectionReady(m_conn);

	if(m_readerTracker->IsAlive) throw gcnew SqliteExceptions::OpenReaderException();

	// Only perform the security demand if Prepare() hasn't already been called.
	// This is actually an expensive operation, and this helps alot without
	// compromising anything security-wise

	if(m_compiledQuery == nullptr) SqliteConnection::ExecutePermission->Demand();

	// The SqliteDataReader is quite self-sufficient.  We can just throw the proper
	// arguments to the constructor and it pretty much takes care of everything else

	if(m_compiledQuery) reader = gcnew SqliteDataReader(this, m_compiledQuery, behavior);
	else reader = gcnew SqliteDataReader(this, GetCommandText(), behavior);

	m_readerTracker->SetObject(reader);		// Track the DataReader
	return reader;
}

//---------------------------------------------------------------------------
// SqliteCommand::ExecuteScalar
//
// Executes a SQL query against the current connection and returns the
// first value returned by any result set in the query.  All remaining
// statements after grabbing the value are treated as non-queries.
//
// Arguments:
//
//	NONE

Object^ SqliteCommand::ExecuteScalar(void)
{
	SqliteQuery^			query;					// Reference to the query object
	Object^				result = nullptr;		// Result from this function
	int					nResult;				// Result from function call

	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckConnectionReady(m_conn);

	if(m_readerTracker->IsAlive) throw gcnew SqliteExceptions::OpenReaderException();

	// Only perform the security demand if Prepare() hasn't already been called.
	// This is actually an expensive operation, and this helps alot without
	// compromising anything security-wise

	if(m_compiledQuery == nullptr) SqliteConnection::ExecutePermission->Demand();

	// If we already have a compiled query, use it.  Otherwise, we need to compile
	// a new query based on the current CommandText property value

	if(m_compiledQuery != nullptr) query = m_compiledQuery;
	else query = gcnew SqliteQuery(m_conn->HandlePointer, GetCommandText());

	try { 

		m_params->Lock();						// Lock down the parameters

		try {

			// Always set the busy timeout immediately before executing the command

			nResult = sqlite3_busy_timeout(m_conn->Handle, m_timeout * 1000);
			if(nResult != SQLITE_OK) throw gcnew SqliteException(m_conn->Handle, nResult);

			// Iterate over all of the individual statements and decide what to
			// do with them.  Once we have a scalar result, any remaining statements
			// become non-queries from our perspective

			for each(SqliteStatement^ statement in query) {

				statement->BindParameters(m_params, m_conn);	// Bind params

				// If we already have our scalar result value, just exec this
				// statement as a non-query.  Otherwise, execute it as a scalar.
				// This ends up giving us the very first object returned by the
				// query, yet still executes all remaining statements ...

				if(result != nullptr) statement->ExecuteNonQuery();
				else result = statement->ExecuteScalar();
			}
		}

		finally { m_params->Unlock(); }					// Unlock the parameters

	} // outer try

	finally { if(query != m_compiledQuery) delete query; }

	return result;						// Return the scalar return value
}

//---------------------------------------------------------------------------
// SqliteCommand::GenericCommandType::get
//
// Retrieves the generic CommandType of this command object

Data::CommandType SqliteCommand::GenericCommandType::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return static_cast<Data::CommandType>(CommandType);
}

//---------------------------------------------------------------------------
// SqliteCommand::GenericCommandType::set
//
// Changes the generic CommandType of this command object

void SqliteCommand::GenericCommandType::set(Data::CommandType value)
{
	CHECK_DISPOSED(m_disposed);
	if(value == Data::CommandType::StoredProcedure) throw gcnew NotSupportedException();
	CommandType = static_cast<SqliteCommandType>(value);
}

//---------------------------------------------------------------------------
// SqliteCommand::GenericUpdatedRowSource::get
//
// Returns the generic version of UpdatedRowSource

UpdateRowSource SqliteCommand::GenericUpdatedRowSource::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return static_cast<UpdateRowSource>(UpdatedRowSource);
}

//---------------------------------------------------------------------------
// SqliteCommand::GenericUpdatedRowSource::set
//
// Changes the generic UpdatedRowSource of this command object

void SqliteCommand::GenericUpdatedRowSource::set(UpdateRowSource value)
{
	CHECK_DISPOSED(m_disposed);
	if((value == UpdateRowSource::Both) || (value == UpdateRowSource::OutputParameters))
		throw gcnew NotSupportedException();

	UpdatedRowSource = static_cast<SqliteUpdateRowSource>(value);
}

//---------------------------------------------------------------------------
// SqliteCommand::GetCommandText (private)
//
// Gets an appropriate command text based on the command type
//
// Arguments:
//
//	NONE

String^ SqliteCommand::GetCommandText(void)
{
	CHECK_DISPOSED(m_disposed);

	// TEXT: Just return whatever the user has set up in the public property

	if(m_commandType != SqliteCommandType::TableDirect) return m_commandText;

	// TABLEDIRECT: While not exactly the same thing as what OLEDB supports,
	// the basic gist is that it becomes a "SELECT * FROM [table]".  If the
	// user didn't put a table name in there, well, that's their problem

	return String::Format("SELECT * FROM [{0}]", (m_commandText != nullptr) ? 
		m_commandText : String::Empty);
}

//---------------------------------------------------------------------------
// SqliteCommand::Parameters::get
//
// Gets a reference to the contained SqliteParameterCollection object

SqliteParameterCollection^ SqliteCommand::Parameters::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return m_params;
}

//---------------------------------------------------------------------------
// SqliteCommand::Prepare
//
// Attempts to pre-compile the current command text, generally because the
// caller is going to be executing a parameterized query multiple times
//
// Arguments:
//
//	NONE

void SqliteCommand::Prepare(void)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckConnectionReady(m_conn);

	// Perform the execute permission check here, which allows us to bypass
	// it in the ExecuteXXXXX() functions if Prepare() has been called

	SqliteConnection::ExecutePermission->Demand();

	// Get rid of any previously compiled query data, and compile the current
	// query set up in our local CommandText property

	UncompileQuery();
	m_compiledQuery = gcnew SqliteQuery(m_conn->HandlePointer, GetCommandText());
}

//---------------------------------------------------------------------------
// SqliteCommand::UncompileQuery (private)
//
// Helper function used to ensure that any contained pre-compiled query 
// generated with Prepare() is disposed of properly and set back to NULL
//
// Arguments:
//
//	NONE

void SqliteCommand::UncompileQuery(void)
{
	if(m_compiledQuery != nullptr) delete m_compiledQuery;
	m_compiledQuery = nullptr;
}

//---------------------------------------------------------------------------
// SqliteCommand::UpdatedRowSource::set
//
// Specifies how query command results are applied to the row being updated

void SqliteCommand::UpdatedRowSource::set(SqliteUpdateRowSource value)
{
	CHECK_DISPOSED(m_disposed);

	// Currently we only support NONE and FIRSTRETURNEDRECORD update types.

	if((value != SqliteUpdateRowSource::None) && (value != SqliteUpdateRowSource::FirstReturnedRecord))
		throw gcnew SqliteExceptions::UpdateRowSourceUnknownException(value);

	m_updatedRowSource = value;		// Change the updated row source flag
}

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)
