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
#include "zDBCommand.h"				// Include zDBCommand declarations
#include "zDBConnection.h"			// Include zDBConnection declarations
#include "zDBDataAdapter.h"			// Include zDBDataAdapter declarations
#include "zDBDataReader.h"			// Include zDBDataReader declarations
#include "zDBTransaction.h"			// Include zDBTransaction declarations

#pragma warning(push, 4)			// Enable maximum compiler warnings

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// zDBCommand Destructor

zDBCommand::~zDBCommand()
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
// zDBCommand::Cancel
//
// Tries to cancel any pending operations against the database. Calls through
// to the attached zDBConnection object to do the work, since there is no
// command-level implementation of this concept in SQLite.
//
// Arguments:
//
//	NONE

void zDBCommand::Cancel(void)
{
	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckConnectionValid(m_conn);

	m_conn->Interrupt();			// Attempt to interrupt the engine
}

//---------------------------------------------------------------------------
// zDBCommand::CommandText::get
//
// Retrieves a copy of the currently set SQL command text

String^ zDBCommand::CommandText::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return m_commandText;
}

//---------------------------------------------------------------------------
// zDBCommand::CommandText::set
//
// Changes the SQL command text to be exeuted by this command

void zDBCommand::CommandText::set(String^ value)
{
	CHECK_DISPOSED(m_disposed);
	if(m_readerTracker->IsAlive) throw gcnew zDBExceptions::OpenReaderException();

	UncompileQuery();					// Must uncompile if command changes
	m_commandText = value;				// Change contained command text
}

//---------------------------------------------------------------------------
// zDBCommand::CommandTimeout::get
//
// Gets the configured number of seconds that the command will wait to 
// execute if the database is busy

int zDBCommand::CommandTimeout::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return m_timeout;
}

//---------------------------------------------------------------------------
// zDBCommand::CommandTimeout::set
//
// Sets the number of seconds that the command will wait to execute if the
// database is busy

void zDBCommand::CommandTimeout::set(int value)
{
	CHECK_DISPOSED(m_disposed);
	if(m_readerTracker->IsAlive) throw gcnew zDBExceptions::OpenReaderException();

	if(m_timeout < 0) throw gcnew ArgumentOutOfRangeException();
	m_timeout = value;
}

//---------------------------------------------------------------------------
// zDBCommand::CommandType::get
//
// Returns the type of command this is

zDBCommandType zDBCommand::CommandType::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return m_commandType;
}

//---------------------------------------------------------------------------
// zDBCommand::CommandType::set
//
// Sets the type of command this is, which alters the CommandText format
// and perhaps even the behavior at some point

void zDBCommand::CommandType::set(zDBCommandType value)
{
	CHECK_DISPOSED(m_disposed);
	if(m_readerTracker->IsAlive) throw gcnew zDBExceptions::OpenReaderException();
	
	if(value == m_commandType) return;			// Same, don't do anything at all

	// Currently we only support TEXT and TABLEDIRECT command types.  This might
	// be extended if I can come up with anything interesting to extend it with.
	// If so, zDBCommandType should probably get zDBType treatment with a nice
	// hearty value struct instead of an enumeration

	if((value != zDBCommandType::Text) && (value != zDBCommandType::TableDirect))
		throw gcnew zDBExceptions::CommandTypeUnknownException(value);

	UncompileQuery();				// Must uncompile if type changes
	m_commandType = value;			// Change the stored command type
}

//---------------------------------------------------------------------------
// zDBCommand::Connection::get
//
// Retrieves a reference to the connection this command is associated with

zDBConnection^ zDBCommand::Connection::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return m_conn;
}

//---------------------------------------------------------------------------
// zDBCommand::Connection::set
//
// Changes the zDBConnection object that will be used to execute this command.
// Any previously compiled query created with Prepare() will be lost.

void zDBCommand::Connection::set(zDBConnection^ value)
{
	CHECK_DISPOSED(m_disposed);
	if(m_readerTracker->IsAlive) throw gcnew zDBExceptions::OpenReaderException();

	if(m_conn == value) return;				// Same, don't do anything at all

	UncompileQuery();			// Must uncompile if connection changes
	m_conn = value;				// Changed stored connection reference
}

//---------------------------------------------------------------------------
// zDBCommand::Construct (private)
//
// Acts as a pseudo-constructor for the class due to the number of real
// constructor overloads to deal with.
//
// Arguments:
//
//	commandText		- SQL command text to automatically set
//	conn			- zDBConnection object to automatically set
//	type			- zDBCommandType to automatically set

void zDBCommand::Construct(String^ commandText, zDBConnection^ conn, zDBCommandType type)
{
	m_designTimeVisible = true;
	m_updatedRowSource = zDBUpdateRowSource::FirstReturnedRecord;
	m_timeout = DEFAULT_TIMEOUT;

	// Construct the zDBParameterCollection local to this command as
	// well as the special ObjectTracker we use to keep an eye on readers

	m_params = gcnew zDBParameterCollection();
	m_readerTracker = gcnew ObjectTracker();

	CommandText = commandText;					// Apply command text
	Connection = conn;							// Apply connection
	CommandType = type;							// Apply command type
}

//---------------------------------------------------------------------------
// zDBCommand::CreateDbParameter (protected)
//
// Creates a new parameter object for this command on it's generic interface
//
// Arguments:
//
//	NONE

DbParameter^ zDBCommand::CreateDbParameter(void)
{
	CHECK_DISPOSED(m_disposed);
	return gcnew zDBParameter();
}

//---------------------------------------------------------------------------
// zDBCommand::DbConnection::get (protected)
//
// Retrieves the stored connection object's generic interface

Data::Common::DbConnection^ zDBCommand::DbConnection::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return Connection;			// Invoke the strongly-typed property
}

//---------------------------------------------------------------------------
// zDBCommand::DbConnection::set (protected)
//
// Sets the stored connection object via the generic interface

void zDBCommand::DbConnection::set(Data::Common::DbConnection^ value)
{
	CHECK_DISPOSED(m_disposed);

	// If the provided connection object isn't a zDBConnection, something
	// has gone totally whack with the DbCommand base class ...

	Connection = safe_cast<zDBConnection^>(value);
}

//---------------------------------------------------------------------------
// zDBCommand::DbParameterCollection::get (protected)
//
// Gets a reference to the parameters collection on the generic interface

Data::Common::DbParameterCollection^ zDBCommand::DbParameterCollection::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return m_params;
}

//---------------------------------------------------------------------------
// zDBCommand::ExecuteDbDataReader (protected)
//
// Executes the command and returns a generic data reader to process it
//
// Arguments:
//
//	behavior		- DataReader command behavior flags

DbDataReader^ zDBCommand::ExecuteDbDataReader(CommandBehavior behavior)
{
	CHECK_DISPOSED(m_disposed);
	return ExecuteReader(static_cast<zDBCommandBehavior>(behavior));
}

//---------------------------------------------------------------------------
// zDBCommand::ExecuteNonQuery
//
// Executes a SQL statement against the current connection and returns
// the number of rows that were affected by that statement
//
// Arguments:
//
//	NONE

int zDBCommand::ExecuteNonQuery(void)
{
	zDBQuery^				query;				// Reference to the query object
	int						changes = 0;		// Total number of changes by query
	int						nResult;			// Result from function call

	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckConnectionReady(m_conn);

	if(m_readerTracker->IsAlive) throw gcnew zDBExceptions::OpenReaderException();

	// Only perform the security demand if Prepare() hasn't already been called.
	// This is actually an expensive operation, and this helps alot without
	// compromising anything security-wise

	if(m_compiledQuery == nullptr) zDBConnection::ExecutePermission->Demand();

	// If we already have a compiled query, use it.  Otherwise, we need to compile
	// a new query based on the current CommandText property value

	if(m_compiledQuery != nullptr) query = m_compiledQuery;
	else query = gcnew zDBQuery(m_conn->HandlePointer, GetCommandText());

	try { 
		
		m_params->Lock();						// Lock down the parameters

		try {

			// Always set the busy timeout immediately before executing the command

			nResult = sqlite3_busy_timeout(m_conn->Handle, m_timeout * 1000);
			if(nResult != SQLITE_OK) throw gcnew zDBException(m_conn->Handle, nResult);

			// For every statement in the compiled query, bind the local parameter
			// collection to it, and execute it.  It takes care of everything else

			for each(zDBStatement^ statement in query) {
				
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
// zDBCommand::ExecuteReader
//
// Executes the compiled query and returns a zDBDataReader that can be used
// to access the result set(s)
//
// Arguments:
//
//	behavior			- zDBCommandBehavior to be used for this reader

zDBDataReader^ zDBCommand::ExecuteReader(zDBCommandBehavior behavior)
{
	zDBDataReader^				reader;		// The new zDBDataReader object

	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckConnectionReady(m_conn);

	if(m_readerTracker->IsAlive) throw gcnew zDBExceptions::OpenReaderException();

	// Only perform the security demand if Prepare() hasn't already been called.
	// This is actually an expensive operation, and this helps alot without
	// compromising anything security-wise

	if(m_compiledQuery == nullptr) zDBConnection::ExecutePermission->Demand();

	// The zDBDataReader is quite self-sufficient.  We can just throw the proper
	// arguments to the constructor and it pretty much takes care of everything else

	if(m_compiledQuery) reader = gcnew zDBDataReader(this, m_compiledQuery, behavior);
	else reader = gcnew zDBDataReader(this, GetCommandText(), behavior);

	m_readerTracker->SetObject(reader);		// Track the DataReader
	return reader;
}

//---------------------------------------------------------------------------
// zDBCommand::ExecuteScalar
//
// Executes a SQL query against the current connection and returns the
// first value returned by any result set in the query.  All remaining
// statements after grabbing the value are treated as non-queries.
//
// Arguments:
//
//	NONE

Object^ zDBCommand::ExecuteScalar(void)
{
	zDBQuery^			query;					// Reference to the query object
	Object^				result = nullptr;		// Result from this function
	int					nResult;				// Result from function call

	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckConnectionReady(m_conn);

	if(m_readerTracker->IsAlive) throw gcnew zDBExceptions::OpenReaderException();

	// Only perform the security demand if Prepare() hasn't already been called.
	// This is actually an expensive operation, and this helps alot without
	// compromising anything security-wise

	if(m_compiledQuery == nullptr) zDBConnection::ExecutePermission->Demand();

	// If we already have a compiled query, use it.  Otherwise, we need to compile
	// a new query based on the current CommandText property value

	if(m_compiledQuery != nullptr) query = m_compiledQuery;
	else query = gcnew zDBQuery(m_conn->HandlePointer, GetCommandText());

	try { 

		m_params->Lock();						// Lock down the parameters

		try {

			// Always set the busy timeout immediately before executing the command

			nResult = sqlite3_busy_timeout(m_conn->Handle, m_timeout * 1000);
			if(nResult != SQLITE_OK) throw gcnew zDBException(m_conn->Handle, nResult);

			// Iterate over all of the individual statements and decide what to
			// do with them.  Once we have a scalar result, any remaining statements
			// become non-queries from our perspective

			for each(zDBStatement^ statement in query) {

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
// zDBCommand::GenericCommandType::get
//
// Retrieves the generic CommandType of this command object

Data::CommandType zDBCommand::GenericCommandType::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return static_cast<Data::CommandType>(CommandType);
}

//---------------------------------------------------------------------------
// zDBCommand::GenericCommandType::set
//
// Changes the generic CommandType of this command object

void zDBCommand::GenericCommandType::set(Data::CommandType value)
{
	CHECK_DISPOSED(m_disposed);
	if(value == Data::CommandType::StoredProcedure) throw gcnew NotSupportedException();
	CommandType = static_cast<zDBCommandType>(value);
}

//---------------------------------------------------------------------------
// zDBCommand::GenericUpdatedRowSource::get
//
// Returns the generic version of UpdatedRowSource

UpdateRowSource zDBCommand::GenericUpdatedRowSource::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return static_cast<UpdateRowSource>(UpdatedRowSource);
}

//---------------------------------------------------------------------------
// zDBCommand::GenericUpdatedRowSource::set
//
// Changes the generic UpdatedRowSource of this command object

void zDBCommand::GenericUpdatedRowSource::set(UpdateRowSource value)
{
	CHECK_DISPOSED(m_disposed);
	if((value == UpdateRowSource::Both) || (value == UpdateRowSource::OutputParameters))
		throw gcnew NotSupportedException();

	UpdatedRowSource = static_cast<zDBUpdateRowSource>(value);
}

//---------------------------------------------------------------------------
// zDBCommand::GetCommandText (private)
//
// Gets an appropriate command text based on the command type
//
// Arguments:
//
//	NONE

String^ zDBCommand::GetCommandText(void)
{
	CHECK_DISPOSED(m_disposed);

	// TEXT: Just return whatever the user has set up in the public property

	if(m_commandType != zDBCommandType::TableDirect) return m_commandText;

	// TABLEDIRECT: While not exactly the same thing as what OLEDB supports,
	// the basic gist is that it becomes a "SELECT * FROM [table]".  If the
	// user didn't put a table name in there, well, that's their problem

	return String::Format("SELECT * FROM [{0}]", (m_commandText != nullptr) ? 
		m_commandText : String::Empty);
}

//---------------------------------------------------------------------------
// zDBCommand::Parameters::get
//
// Gets a reference to the contained zDBParameterCollection object

zDBParameterCollection^ zDBCommand::Parameters::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return m_params;
}

//---------------------------------------------------------------------------
// zDBCommand::Prepare
//
// Attempts to pre-compile the current command text, generally because the
// caller is going to be executing a parameterized query multiple times
//
// Arguments:
//
//	NONE

void zDBCommand::Prepare(void)
{
	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckConnectionReady(m_conn);

	// Perform the execute permission check here, which allows us to bypass
	// it in the ExecuteXXXXX() functions if Prepare() has been called

	zDBConnection::ExecutePermission->Demand();

	// Get rid of any previously compiled query data, and compile the current
	// query set up in our local CommandText property

	UncompileQuery();
	m_compiledQuery = gcnew zDBQuery(m_conn->HandlePointer, GetCommandText());
}

//---------------------------------------------------------------------------
// zDBCommand::UncompileQuery (private)
//
// Helper function used to ensure that any contained pre-compiled query 
// generated with Prepare() is disposed of properly and set back to NULL
//
// Arguments:
//
//	NONE

void zDBCommand::UncompileQuery(void)
{
	if(m_compiledQuery != nullptr) delete m_compiledQuery;
	m_compiledQuery = nullptr;
}

//---------------------------------------------------------------------------
// zDBCommand::UpdatedRowSource::set
//
// Specifies how query command results are applied to the row being updated

void zDBCommand::UpdatedRowSource::set(zDBUpdateRowSource value)
{
	CHECK_DISPOSED(m_disposed);

	// Currently we only support NONE and FIRSTRETURNEDRECORD update types.

	if((value != zDBUpdateRowSource::None) && (value != zDBUpdateRowSource::FirstReturnedRecord))
		throw gcnew zDBExceptions::UpdateRowSourceUnknownException(value);

	m_updatedRowSource = value;		// Change the updated row source flag
}

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)
