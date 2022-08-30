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

#include "stdafx.h"

#include "SqliteConnection.h"
#include "SqliteCommand.h"
#include "SqliteDataReader.h"
#include "SqliteMetaData.h"
#include "SqliteTransaction.h"

#pragma warning(push, 4)
#pragma warning(disable:4100)

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// SqliteConnection Destructor

SqliteConnection::~SqliteConnection()
{
	Close();						// Force the connection closed

	delete m_authHook;				// Dispose of the AUTHORIZE hook object
	delete m_collationHook;			// Dispose of the COLLATION hook object
	delete m_commitHook;			// Dispose of the COMMIT hook object
	delete m_profileHook;			// Dispose of the PROFILE hook object
	delete m_progressHook;			// Dispose of the PROGRESS hook object
	delete m_rollbackHook;			// Dispose of the ROLLBACK hook object
	delete m_traceHook;				// Dispose of the TRACE hook object
	delete m_updateHook;			// Dispose of the UPDATE hook object

	m_aggregates->InternalDispose();	// Dispose of the aggregate collection
	m_collations->InternalDispose();	// Dispose of the collation collection
	m_functions->InternalDispose();		// Dispose of the function collection

	this->!SqliteConnection();			// Invoke the finalizer
	m_disposed = true;				// Object has been disposed of
}

//---------------------------------------------------------------------------
// SqliteConnection Finalizer

SqliteConnection::!SqliteConnection()
{
	if(m_pDatabase) m_pDatabase->Release(this);		// Release the reference
	m_pDatabase = NULL;								// Reset pointer to NULL
}

//---------------------------------------------------------------------------
// SqliteConnection::Aggregates::get
//
// Returns a reference to the contained collection of aggregate functions

SqliteAggregateCollection^ SqliteConnection::Aggregates::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return m_aggregates;
}

//---------------------------------------------------------------------------
// SqliteConnection::ApplyConnectionPragmas (private)
//
// Applies all of the necessary PRAGMA commands to a newly opened database
// based on the contents of the stored connection string
//
// Arguments:
//
//	NONE

void SqliteConnection::ApplyConnectionPragmas(void)
{
	String^				query;			// SQL query string to set a PRAGMA

	Debug::Assert(m_state == ConnectionState::Open);

	// PRAGMA AUTO_VACUUM = { 0 | 1 }
	query = String::Format("PRAGMA AUTO_VACUUM = {0}", m_cs->AutoVacuum ? "1" : "0");
	SqliteUtil::ExecuteNonQuery(m_pDatabase->Handle, query);

	// PRAGMA CACHE_SIZE = { n }
	query = String::Format("PRAGMA CACHE_SIZE = {0}", m_cs->CacheSize);
	SqliteUtil::ExecuteNonQuery(m_pDatabase->Handle, query);

	// PRAGMA CASE_SENSITIVE_LIKE = { 0 | 1 }
	query = String::Format("PRAGMA CASE_SENSITIVE_LIKE = {0}", m_cs->CaseSensitiveLike ? "1" : "0");
	SqliteUtil::ExecuteNonQuery(m_pDatabase->Handle, query);

	// PRAGMA ENCODING = { UTF-8 | UTF-16 | UTF-16le | UTF-16be }
	query = String::Format("PRAGMA ENCODING = '{0}'", SqliteUtil::EncodingToPragma(m_cs->Encoding));
	SqliteUtil::ExecuteNonQuery(m_pDatabase->Handle, query);

	// PRAGMA LEGACY_FILE_FORMAT = { 0 | 1 }
	query = String::Format("PRAGMA LEGACY_FILE_FORMAT = {0}", m_cs->CompatibleFileFormat ? "1" : "0");
	SqliteUtil::ExecuteNonQuery(m_pDatabase->Handle, query);

	// PRAGMA PAGE_SIZE = { n }
	query = String::Format("PRAGMA PAGE_SIZE = {0}", m_cs->PageSize);
	SqliteUtil::ExecuteNonQuery(m_pDatabase->Handle, query);

	// PRAGMA SYNCHRONOUS = { 0 | OFF | 1 | NORMAL | 2 | FULL  }
	query = String::Format("PRAGMA SYNCHRONOUS = {0}", static_cast<int>(m_cs->SynchronousMode));
	SqliteUtil::ExecuteNonQuery(m_pDatabase->Handle, query);

	// PRAGMA TEMP_STORE = { 0 | DEFAULT | 1 | FILE | 2 | MEMORY }
	query = String::Format("PRAGMA TEMP_STORE = {0}", static_cast<int>(m_cs->TemporaryStorageMode));
	SqliteUtil::ExecuteNonQuery(m_pDatabase->Handle, query);

	// PRAGMA TEMP_STORE_DIRECTORY = { 'path' }
	query = String::Format("PRAGMA TEMP_STORE_DIRECTORY = '{0}'", m_cs->TemporaryStorageFolder);
	SqliteUtil::ExecuteNonQuery(m_pDatabase->Handle, query);
}

//---------------------------------------------------------------------------
// SqliteConnection::Attach
//
// Attaches another SQLite database as a new catalog in this connection.  The
// attached database is accessed as database-name.table-name.  There is a hard-
// coded maximum of 10 allowed attached databases in the engine
//
// Arguments:
//
//	path			- Path to the database file to be attached
//	databaseName	- Name to assign to the attached database

void SqliteConnection::Attach(String^ path, String^ databaseName)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckConnectionReady(this);		// <-- READY, not OPEN

	String^ query = String::Format("ATTACH DATABASE '{0}' AS [{1}]", path, databaseName);
	SqliteUtil::ExecuteNonQuery(m_pDatabase->Handle, query);
}

//---------------------------------------------------------------------------
// SqliteConnection::AutoVacuum::get
//
// Retrieves the configured AUTO_VACUUM setting for the open database

bool SqliteConnection::AutoVacuum::get(void)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckConnectionOpen(this);

	return m_autoVacuum;			// Return the configured value
}

//---------------------------------------------------------------------------
// SqliteConnection::BeginDbTransaction (protected)
//
// Generic means of creating a new transaction object
//
// Arguments:
//
//	isolationLevel		- Transaction isolation level.  Ignored.

DbTransaction^ SqliteConnection::BeginDbTransaction(Data::IsolationLevel isolationLevel)
{
	return BeginTransaction(SqliteLockMode::Deferred);
}

//---------------------------------------------------------------------------
// SqliteConnection::BeginTransaction
//
// Begins a new transaction against this database connection
//
// Arguments:
//
//	mode		- Locking mode to use for the transaction

SqliteTransaction^ SqliteConnection::BeginTransaction(SqliteLockMode mode)
{
	SqliteTransaction^				trans;			// The new transaction object

	CHECK_DISPOSED(m_disposed);
	ExecutePermission->Demand();
	SqliteUtil::CheckConnectionReady(this);

	if((m_openTrans->Count > 0) && (m_transactionMode == SqliteTransactionMode::Single))
		throw gcnew Exception("TODO:nested transaction exception");

	if((m_openTrans->Count > 0) && (mode != m_openTransMode))
		throw gcnew Exception("TODO:different transaction lock mode exception");

	if(Interlocked::Increment(m_openTransCount) == 1) {

		switch(mode) {

			case SqliteLockMode::Exclusive: 
				SqliteUtil::ExecuteNonQuery(m_pDatabase->Handle, "BEGIN EXCLUSIVE TRANSACTION");
				break;

			case SqliteLockMode::Immediate: 
				SqliteUtil::ExecuteNonQuery(m_pDatabase->Handle, "BEGIN IMMEDIATE TRANSACTION");
				break;

			default: SqliteUtil::ExecuteNonQuery(m_pDatabase->Handle, "BEGIN DEFERRED TRANSACTION");
		}

		m_openTransMode = mode;
	}

	trans = gcnew SqliteTransaction(this);
	m_openTrans->Add(trans);
	return trans;
}

//---------------------------------------------------------------------------
// SqliteConnection::BooleanFormat::get
//
// Retrieves the configured boolean formatting and coercion specifier

SqliteBooleanFormat SqliteConnection::BooleanFormat::get(void)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckConnectionOpen(this);

	return m_cs->BooleanFormat;			// Return currently set value
}

//---------------------------------------------------------------------------
// SqliteConnection::BooleanFormat::set
//
// Changes the boolan formatting and coercion format.  If the format is 
// different than what is already set, the connection string is also updated

void SqliteConnection::BooleanFormat::set(SqliteBooleanFormat value)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckConnectionOpen(this);

	// Since these properties have no representation in the engine itself,
	// we can just use the connection string to hold the current state

	if(value != m_cs->BooleanFormat) m_cs->BooleanFormat = value;
}

//---------------------------------------------------------------------------
// SqliteConnection::CacheSize::get
//
// Retrieves the currently set PRAGMA CACHE_SIZE value

int SqliteConnection::CacheSize::get(void)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckConnectionOpen(this);

	return m_cs->CacheSize;				// Return currently set value
}

//---------------------------------------------------------------------------
// SqliteConnection::CacheSize::set
//
// Changes the PRAGMA CACHE_SIZE value.  If the value is different than
// what is already set, the connection string is also updated

void SqliteConnection::CacheSize::set(int value)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckConnectionReady(this);		// <-- READY, not OPEN

	if(value < 0) throw gcnew ArgumentOutOfRangeException();

	// PRAGMA CACHE_SIZE = { n }
	SqliteUtil::ExecuteNonQuery(m_pDatabase->Handle, String::Format("PRAGMA CACHE_SIZE = {0}", value));

	// Reload the value to make sure we get the configured value, and 
	// apply it to the stored connection string as necessary

	value = Convert::ToInt32(SqliteUtil::ExecuteScalar(m_pDatabase->Handle, "PRAGMA CACHE_SIZE"));
	if(value != m_cs->CacheSize) m_cs->CacheSize = value;
}

//---------------------------------------------------------------------------
// SqliteConnection::CaseSensitiveLike::get
//
// Retrieves the currently set PRAGMA CASE_SENSITIVE_LIKE value

bool SqliteConnection::CaseSensitiveLike::get(void)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckConnectionOpen(this);

	return m_cs->CaseSensitiveLike;			// Return currently set value
}

//---------------------------------------------------------------------------
// SqliteConnection::CaseSensitiveLike::set
//
// Changes the PRAGMA CASE_SENSITIVE_LIKE value.  If the value is different
// than what is already set, the connection string is also updated

void SqliteConnection::CaseSensitiveLike::set(bool value)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckConnectionReady(this);		// <-- READY, not OPEN

	// PRAGMA CASE_SENSITIVE_LIKE = { 0 | 1 }
	SqliteUtil::ExecuteNonQuery(m_pDatabase->Handle, String::Format("PRAGMA CASE_SENSITIVE_LIKE = {0}", value ? 1 : 0));

	// This particular PRAGMA doesn't appear to work property, so we read it
	// by asking to compare a lowercase 'a' with an uppercase 'A' instead
	ENGINE_ISSUE(3.3.8, "PRAGMA CASE_SENSITIVE_LIKE does not return a value when queried");

	value = (Convert::ToInt32(SqliteUtil::ExecuteScalar(m_pDatabase->Handle, "SELECT 'a' NOT LIKE 'A'")) != 0);
	if(value != m_cs->CaseSensitiveLike) m_cs->CaseSensitiveLike = value;
}

//---------------------------------------------------------------------------
// SqliteConnection::CheckIntegrity
//
// Detaches a previously attached SQLite database file by it's catalog name
//
// Arguments:
//
//	databaseName	- Name of the database to be detached

String^ SqliteConnection::CheckIntegrity(void)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckConnectionReady(this);		// <-- READY, not OPEN

	return SqliteUtil::ExecuteScalar(m_pDatabase->Handle, "PRAGMA INTEGRITY_CHECK");
}

//---------------------------------------------------------------------------
// SqliteConnection::Close (private)
//
// Closes the connection to the database.  Any outstanding transactions will
// be rolled back, and any outstanding data readers will be disposed of
//
// Arguments:
//
//	fireStateChange		- Argument used to control firing of OnStateChange

void SqliteConnection::Close(bool fireStateChange)
{
	List<SqliteDataReader^>^		readers;		// Data Readers to be closed

	CHECK_DISPOSED(m_disposed);
	if(m_state == ConnectionState::Closed) return;

	Interrupt();					// Attempt to interrupt anything going on

	// Rollback any and all outstanding transaction objects against this connection

	for each(SqliteTransaction^ trans in m_openTrans) trans->Rollback();

	// Automatically dispose of any active data readers that remain
	// open against this connection.  Since the data readers will remove
	// themselves on disposal, use a COPY of the collection to avoid screwing
	// up the enumerator on the main Dictionary<> instance here

	readers = gcnew List<SqliteDataReader^>();

	for each(KeyValuePair<__int64, SqliteDataReader^>^ item in m_readers)
		if(!item->Value->IsClosed) readers->Add(item->Value);

	for each(SqliteDataReader^ reader in readers) delete reader;

	Debug::Assert(m_readers->Count == 0);		// Should be zero now
	m_readers->Clear();							// Clear out the collection

	// Invoke all of the OnCloseConnection() handlers for the hooked functions
	// so they can remove their handlers before we really shut down SQLite here

	m_authHook->OnCloseConnection();
	m_collationHook->OnCloseConnection();
	m_commitHook->OnCloseConnection();
	m_profileHook->OnCloseConnection();
	m_progressHook->OnCloseConnection();
	m_rollbackHook->OnCloseConnection();
	m_traceHook->OnCloseConnection();
	m_updateHook->OnCloseConnection();

	// Invoke all of the OnCloseConnection() handlers for the user-defined
	// function collections before we shut down SQLite here

	m_aggregates->OnCloseConnection();
	m_collations->OnCloseConnection();
	m_functions->OnCloseConnection();

	// Remove ourselves from the handle mapper before the handle is destroyed

	s_handleMapper->Remove(reinterpret_cast<intptr_t>(m_pDatabase->Handle));

	// Remove the field encryption password from the connection when it's closed

	FieldEncryptionPassword = gcnew SecureString();

	// After everything outstanding has been nuked, we should technically be
	// back to a ConnectionState of Open.  Throw in a little sanity check for fun,
	// and because I did the same thing in the original provider code

	if(m_state != ConnectionState::Open) 
		throw gcnew Exception("INTERNAL FAILURE: Connection object did not unwind.");

	// The spiffy database handle object will automatically call sqlite3_close
	// when the last reference to it has been released.  We don't call it here.
	
	m_pDatabase->Release(this);				// Release the database handle
	m_pDatabase = NULL;						// Reset the pointer back to NULL

	// Any registered virtual table modules get cleaned up AFTER the connection
	// has been closed.  Not really important, but it makes the most sense

	for each(GCHandle gchandle in m_modules) gchandle.Free();
	m_modules->Clear();

	m_state = ConnectionState::Closed;		// The connection is now closed
	
	if(fireStateChange) 
		OnStateChange(gcnew StateChangeEventArgs(ConnectionState::Open, ConnectionState::Closed));
}

//---------------------------------------------------------------------------
// SqliteConnection::Collations::get
//
// Returns a reference to the contained collection of collation functions

SqliteCollationCollection^ SqliteConnection::Collations::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return m_collations;
}

//---------------------------------------------------------------------------
// SqliteConnection::CommitTransaction (internal)
//
// Commits an outstanding database transaction
//
// Arguments:
//
//	trans		- SqliteTransaction to be committed

void SqliteConnection::CommitTransaction(SqliteTransaction^ trans)
{
	int				index;				// Index into trans List<T>

	CHECK_DISPOSED(m_disposed);
	if(trans == nullptr) throw gcnew ArgumentNullException();

	// Look for the SqliteTransaction object in our local cache, and if it's
	// not in there, it wasn't created by us so we can't go committing anything

	index = m_openTrans->IndexOf(trans);
	if(index == -1) throw gcnew ArgumentException();
	m_openTrans->RemoveAt(index);

	// Even if the SqliteTransaction exists, we can be at a transaction count
	// of zero if a nested ROLLBACK has occurred on this connection

	if(m_openTransCount == 0) throw gcnew InvalidOperationException();

	// Decrement the outstanding transaction counter, and if it has fallen
	// to zero from this commit, go ahead and try to commit the transaction

	if(Interlocked::Decrement(m_openTransCount) == 0)
		SqliteUtil::ExecuteNonQuery(m_pDatabase->Handle, "COMMIT TRANSACTION");
}

//---------------------------------------------------------------------------
// SqliteConnection::CompatibleFileFormat::get
//
// Retrieves the configured LEGACY_FILE_FORMAT setting for the open database

bool SqliteConnection::CompatibleFileFormat::get(void)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckConnectionOpen(this);

	return m_compatibleFormat;		// Return the configured value
}

//---------------------------------------------------------------------------
// SqliteConnection::ConnectionString::get
//
// Returns a copy of the current connection's connection string.  Properties
// that are configurable while the connection is open will be reflected here

String^ SqliteConnection::ConnectionString::get(void)
{
	CHECK_DISPOSED(m_disposed);

	return m_cs->ConnectionString;		// Return the connection string
}

//---------------------------------------------------------------------------
// SqliteConnection::ConnectionString::set
//
// Alters the database connection string.  Cannot be changed unless the
// connection is closed, however any dynamic properties explicitly changed
// while the connection is open will be reflected by the getter

void SqliteConnection::ConnectionString::set(String^ value)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckConnectionClosed(this);

	m_cs->ConnectionString = value;		// Change the connection string
}

//---------------------------------------------------------------------------
// SqliteConnection::Construct (private)
//
// Helper function used to implement the meat of the various constructors
//
// Arguments:
//
//	connectionString		- Optional connection string to initialize with

void SqliteConnection::Construct(String^ connectionString)
{
	m_state = ConnectionState::Closed;
	m_cs = gcnew SqliteConnectionStringBuilder(connectionString);
	m_openTrans = gcnew List<SqliteTransaction^>();
	m_fieldKey = gcnew SqliteCryptoKey(gcnew SecureString());

	m_authHook = gcnew SqliteConnectionAuthorizationHook(this);
	m_collationHook = gcnew SqliteConnectionCollationNeededHook(this);
	m_commitHook = gcnew SqliteConnectionCommitHook(this);
	m_profileHook = gcnew SqliteConnectionProfileHook(this);
	m_progressHook = gcnew SqliteConnectionProgressHook(this);
	m_rollbackHook = gcnew SqliteConnectionRollbackHook(this);
	m_traceHook = gcnew SqliteConnectionTraceHook(this);
	m_updateHook = gcnew SqliteConnectionUpdateHook(this);

	m_readers = gcnew Dictionary<__int64, SqliteDataReader^>();
	m_modules = gcnew List<GCHandle>();

	m_aggregates = gcnew SqliteAggregateCollection();
	m_collations = gcnew SqliteCollationCollection();
	m_functions = gcnew SqliteFunctionCollection();
}

//---------------------------------------------------------------------------
// SqliteConnection::CreateCommand
//
// Creates and returns a new SqliteCommand object with the Connection property
// already set up to use this connection object
//
// Arguments:
//
//	NONE

SqliteCommand^ SqliteConnection::CreateCommand(void)
{
	CHECK_DISPOSED(m_disposed);
	return gcnew SqliteCommand(String::Empty, this);
}

//---------------------------------------------------------------------------
// SqliteConnection::CreateDbCommand (protected)
//
// Implements the generic method for creating a command object
//
// Arguments:
//
//	NONE

DbCommand^ SqliteConnection::CreateDbCommand(void)
{
	return CreateCommand();				// Use the specific implementation
}

//---------------------------------------------------------------------------
// SqliteConnection::CreateExecutePermission (private, static)
//
// Constructs and initializes the value for the ExecutePermission field
//
// Arguments:
//
//	NONE

CodeAccessPermission^ SqliteConnection::CreateExecutePermission(void)
{
	// This is taken lock, stock, and barrel from the SqlClientConnection
	// class.  I'm making an assumption it will work dandy until I take
	// the time to determine exactly what, if anything, I want to customize
	// for the Code Access Security of this ADO.NET provider.

	SqlitePermission^ perm = gcnew SqlitePermission(PermissionState::None);
	perm->Add(String::Empty, String::Empty, KeyRestrictionBehavior::AllowOnly);
	return perm;
}

//---------------------------------------------------------------------------
// SqliteConnection::Database::get
//
// Retrieves the name of the current catalog/database, or in the case of
// SQLite a hard-coded name of "main" since catalogs aren't supported

String^ SqliteConnection::Database::get(void)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckConnectionOpen(this);

	return MAIN_CATALOG_NAME;		// Always set to "main" no matter what
}

//---------------------------------------------------------------------------
// SqliteConnection::DataSource::get
//
// Retrieves the file name of the main database

String^ SqliteConnection::DataSource::get(void)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckConnectionOpen(this);

	return m_cs->DataSource;			// Return the database file name
}

//---------------------------------------------------------------------------
// SqliteConnection::DateTimeFormat::get
//
// Retrieves the configured date/time formatting and coercion specifier

SqliteDateTimeFormat SqliteConnection::DateTimeFormat::get(void)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckConnectionOpen(this);

	return m_cs->DateTimeFormat;		// Return currently set value
}

//---------------------------------------------------------------------------
// SqliteConnection::DateTimeFormat::set
//
// Changes the date/time formatting and coercion format.  If the format is 
// different than what is already set, the connection string is also updated

void SqliteConnection::DateTimeFormat::set(SqliteDateTimeFormat value)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckConnectionOpen(this);

	// Since these properties have no representation in the engine itself,
	// we can just use the connection string to hold the current state

	if(value != m_cs->DateTimeFormat) m_cs->DateTimeFormat = value;
}

//---------------------------------------------------------------------------
// SqliteConnection::Detach
//
// Detaches a previously attached SQLite database file by it's catalog name
//
// Arguments:
//
//	databaseName	- Name of the database to be detached

void SqliteConnection::Detach(String^ databaseName)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckConnectionReady(this);		// <-- READY, not OPEN

	String^ query = String::Format("DETACH DATABASE [{0}]", databaseName);
	SqliteUtil::ExecuteNonQuery(m_pDatabase->Handle, query);
}

//---------------------------------------------------------------------------
// SqliteConnection::Encoding::get
//
// Retrieves the configured ENCODING setting for the open database

SqliteTextEncodingMode SqliteConnection::Encoding::get(void)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckConnectionOpen(this);

	return m_encoding;				// Return the configured value
}

//---------------------------------------------------------------------------
// SqliteConnection::FieldEncryptionKey::get (internal)
//
// Returns the HCRYPTKEY for the field level encryption

HCRYPTKEY SqliteConnection::FieldEncryptionKey::get(void)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckConnectionOpen(this);

	return m_fieldKey->Key;
}

//---------------------------------------------------------------------------
// SqliteConnection::FieldEncryptionPassword::set
//
// Changes the encryption password to be used when working with the built-in
// ENCRYPT() and DECRYPT() scalar functions

void SqliteConnection::FieldEncryptionPassword::set(SecureString^ value)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckConnectionOpen(this);

	delete m_fieldKey;
	m_fieldKey = gcnew SqliteCryptoKey(value); 
}

//---------------------------------------------------------------------------
// SqliteConnection::FindConnection (static, internal)
//
// Attempts to map a standard sqlite3* handle into it's parent SqliteConnection
// object instance.  Used primarily by virtual tables.
//
// Arguments:
//
//	hDatabase		- Database handle to locate the SqliteConnection for

SqliteConnection^ SqliteConnection::FindConnection(sqlite3* hDatabase)
{
	SqliteConnection^			conn;			// Located connection handle

	intptr_t ptHandle = reinterpret_cast<intptr_t>(hDatabase);

	if(s_handleMapper->TryGetValue(ptHandle, conn)) return conn;
	else return nullptr;
}

//---------------------------------------------------------------------------
// SqliteConnection::Functions::get
//
// Returns a reference to the contained collection of scalar functions

SqliteFunctionCollection^ SqliteConnection::Functions::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return m_functions;
}

//---------------------------------------------------------------------------
// SqliteConnection::GetHandle (internal)
//
// Retrieves a reference counted copy of the contained database handle.
// The caller must use Release() when finished, much like in COM objects
//
// Arguments:
//
//	ppDatabaseHandle	- Pointer to receive the copy of the handle class

void SqliteConnection::GetHandle(DatabaseHandle **ppDatabaseHandle)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckConnectionOpen(this);

	*ppDatabaseHandle = m_pDatabase;		// Copy the instance pointer
	m_pDatabase->AddRef(this);				// AddRef() before returning
}

//---------------------------------------------------------------------------
// SqliteConnection::GetSchema
//
// Retrieves a DataTable containing database schema information
//
// Arguments:
//
//	collectionName		- Specifies the name of the schema to return
//	restrictionValues	- Specifies the restriction values for the schema

DataTable^ SqliteConnection::GetSchema(String^ collectionName, array<String^>^ restrictionValues)
{
	CHECK_DISPOSED(m_disposed);
	ExecutePermission->Demand();

	// The details of implementing metadata are quite ugly and tedious,
	// so they have been offloaded into a class of their own ...

	return SqliteMetaData::Generate(this, collectionName, restrictionValues);
}

//---------------------------------------------------------------------------
// SqliteConnection::GuidFormat::get
//
// Retrieves the configured GUID formatting and coercion specifier

SqliteGuidFormat SqliteConnection::GuidFormat::get(void)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckConnectionOpen(this);

	return m_cs->GuidFormat;		// Return currently set value
}

//---------------------------------------------------------------------------
// SqliteConnection::GuidFormat::set
//
// Changes the GUID formatting and coercion format.  If the format is 
// different than what is already set, the connection string is also updated

void SqliteConnection::GuidFormat::set(SqliteGuidFormat value)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckConnectionOpen(this);

	// Since these properties have no representation in the engine itself,
	// we can just use the connection string to hold the current state

	if(value != m_cs->GuidFormat) m_cs->GuidFormat = value;
}

//---------------------------------------------------------------------------
// SqliteConnection::Handle::get (internal)
//
// Retrieves a non referenced counted copy of the contained database handle.
// If a reference counted copy is required, use the GetHandle() method instead
// of just AddRef-ing the returned pointer

DatabaseHandle& SqliteConnection::Handle::get(void)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckConnectionOpen(this);

	return *m_pDatabase;					// Return a class reference
}

//---------------------------------------------------------------------------
// SqliteConnection::HandlePointer::get (internal)
//
// Retrieves a non referenced counted pointer to the contained database handle.
// If a reference counted copy is required, use the GetHandle() method instead
// of just AddRef-ing the returned pointer

DatabaseHandle* SqliteConnection::HandlePointer::get(void)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckConnectionOpen(this);

	return m_pDatabase;					// Return a class pointer
}

//---------------------------------------------------------------------------
// SqliteConnection::Interrupt (internal)
//
// Attempts to abort any currently pending database operations at the
// SQLite engine level.  Does not indicate success or failure, though.
//
// Arguments:
//
//	NONE

void SqliteConnection::Interrupt(void)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckConnectionOpen(this);

	sqlite3_interrupt(m_pDatabase->Handle);		// Abort pending operations
}

//---------------------------------------------------------------------------
// SqliteConnection::InTransaction::get (internal)
//
// Determines if the ENGINE is currently in a transaction.  This has nothing
// to do with the provider transactions, although one would expect that if
// the provider is in a transaction, the engine better be as well

bool SqliteConnection::InTransaction::get(void)
{
	CHECK_DISPOSED(m_disposed);
	if(m_state == ConnectionState::Closed) return false;

	// If the engine is in autocommit mode (non-zero return value), there
	// is no specific transaction in progress

	return (sqlite3_get_autocommit(m_pDatabase->Handle) == 0);
}

//---------------------------------------------------------------------------
// SqliteConnection::LoadConfiguredPragmas (private)
//
// Loads the pragmas that may be permanent in the open database and sets the
// values of the member variables that expose them to the application
//
// Arguments:
//
//	NONE

void SqliteConnection::LoadConfiguredPragmas(void)
{
	String^				result;			// Result from a scalar query

	Debug::Assert(m_state == ConnectionState::Open);

	// PRAGMA AUTO_VACUUM
	m_autoVacuum = false;
	result = SqliteUtil::ExecuteScalar(m_pDatabase->Handle, "PRAGMA AUTO_VACUUM");
	if(!String::IsNullOrEmpty(result)) m_autoVacuum = (Convert::ToInt32(result) == 1);

	// PRAGMA LEGACY_FILE_FORMAT
	//
	// TODO: This pragma is obsolete and has been removed
	m_compatibleFormat = false;
	result = SqliteUtil::ExecuteScalar(m_pDatabase->Handle, "PRAGMA LEGACY_FILE_FORMAT");
	if(!String::IsNullOrEmpty(result)) m_compatibleFormat = (Convert::ToInt32(result) == 1);

	// PRAGMA ENCODING
	m_encoding = SqliteTextEncodingMode::UTF8;
	result = SqliteUtil::ExecuteScalar(m_pDatabase->Handle, "PRAGMA ENCODING");
	if(!String::IsNullOrEmpty(result)) m_encoding = SqliteUtil::PragmaToEncoding(result);

	// PRAGMA PAGE_SIZE
	m_pageSize = 1024;				// TODO: As of SQLITE 3.12.0, this is now 4096, this code is still based on 3.3.8
	result = SqliteUtil::ExecuteScalar(m_pDatabase->Handle, "PRAGMA PAGE_SIZE");
	if(!String::IsNullOrEmpty(result)) m_pageSize = Convert::ToInt32(result);
}

//---------------------------------------------------------------------------
// SqliteConnection::Open
//
// Opens the database connection, utilizing the contents of the stored
// connection string.  Also reads all of the non-modifiable connection
// properties to ensure that they are correct for the current database
//
// Arguments:
//
//	NONE

void SqliteConnection::Open(void)
{
	sqlite3*				hDatabase = NULL;		// The new database handle
	int						nResult;				// Result from function call

	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckConnectionClosed(this);

	Debug::Assert(m_pDatabase == NULL);				// Should always be NULL here
	
	// CONNECTION STRING CODE ACCESS SECURITY
	//
	// The SqlClient provider jumps through a million hoops to accomplish something
	// that essentially boils down to this (I think).  What we do is create a new
	// SqlitePermission object with the connection string we're about to use, and then
	// demand that new permission.  If anything has been set in the configuration or
	// in the code of the application to prevent this connection string, we'll die
	// right here and now with a SecurityException.
	//
	// For some examples on what values to set to restrict the connection string, 
	// start with the MSDN topic SqlClientPermission.Add() and work from there.

	SqlitePermission^ openPerm = gcnew SqlitePermission(PermissionState::None);
	openPerm->Add(m_cs->ToString(), String::Empty, KeyRestrictionBehavior::AllowOnly);
	openPerm->Demand();

	// Attempt to open the main SQLite database handle.  Note that we have to
	// use the ANSI version at all times, according to the SQLite documentation.
	// We also have to be sure to call sqlite3_close in the event of an error

	nResult = sqlite3_open(AutoAnsiString(m_cs->DataSource), &hDatabase);
	if(nResult != SQLITE_OK) { sqlite3_close(hDatabase); throw gcnew SqliteException(nResult); }

	// I can't seem to find a place in SQLite that actually uses the extended error
	// codes, at least in the Windows build.  There is no reason to enable this until
	// they are used.  SqliteException will need to change as well to accomodate it

	// TODO: Resolve this
	//ENGINE_ISSUE(3.3.8, "Extended result codes are seemingly unused in the engine");
	//sqlite3_extended_result_codes(hDatabase, 1);

	// Set the provided connection string's ALLOW EXTENSIONS property, which dictates
	// if low-level extension APIs will be allowed by this connection or not

	sqlite3_enable_load_extension(hDatabase, (m_cs->AllowExtensions ? 1 : 0));

	m_pDatabase = new DatabaseHandle(this, hDatabase);		// Hand off the db handle	
	m_state = ConnectionState::Open;						// Connection is now open

	// Now that the connection is open, add ourselves to the handle mapper
	// before doing anything else so that things like Virtual Tables can find us

	s_handleMapper->Add(reinterpret_cast<intptr_t>(hDatabase), this);

	// Apply all of the connection string options IMMEDIATELY so that the special
	// ones can be set for a new database.  Afterwards, load the non-modifiable
	// options back in, since they may or may not be the same for an existing file

	try { 

		m_transactionMode = m_cs->TransactionMode;	// Set the transaction mode

		ApplyConnectionPragmas();					// Apply connection PRAGMAs
		LoadConfiguredPragmas();					// Load the actual values

		// After the connection has been opened, apply the field level encryption
		// password configured on the connection string, if there was one ...

		FieldEncryptionPassword = m_cs->GetFieldEncryptionPassword();
	}

	catch(Exception^) { Close(false); throw; }		// Close and re-throw

	// Allow all of the custom function collections to tie into SQLite now
	// if they already have delegates/objects to be registered

	m_aggregates->OnOpenConnection(m_pDatabase);
	m_collations->OnOpenConnection(m_pDatabase);
	m_functions->OnOpenConnection(m_pDatabase);

	// Allow all of the hooked event helpers to tie into SQLite now if they
	// already have handlers associated with them

	m_authHook->OnOpenConnection(m_pDatabase);
	m_collationHook->OnOpenConnection(m_pDatabase);
	m_commitHook->OnOpenConnection(m_pDatabase);
	m_profileHook->OnOpenConnection(m_pDatabase);
	m_progressHook->OnOpenConnection(m_pDatabase);
	m_rollbackHook->OnOpenConnection(m_pDatabase);
	m_traceHook->OnOpenConnection(m_pDatabase);
	m_updateHook->OnOpenConnection(m_pDatabase);

	// STATECHANGE: CLOSED->OPEN
	OnStateChange(gcnew StateChangeEventArgs(ConnectionState::Closed, ConnectionState::Open));
}

//---------------------------------------------------------------------------
// SqliteConnection::PageSize::get
//
// Retrieves the configured page size for the open database

int SqliteConnection::PageSize::get(void)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckConnectionOpen(this);

	return m_pageSize;				// Return the configured value
}

//---------------------------------------------------------------------------
// SqliteConnection::RegisterDataReader (internal)
//
// Registers a SqliteDataReader with this connection so it can be automatically
// disposed of when the connection is closed
//
// Arguments:
//
//	reader		- SqliteDataReader to be registered

__int64 SqliteConnection::RegisterDataReader(SqliteDataReader^ reader)
{
	__int64				cookie;			// Cookie to be returned

	CHECK_DISPOSED(m_disposed);
	if(reader == nullptr) throw gcnew ArgumentNullException();

	cookie = Interlocked::Increment(s_cookie);	// Increment cookie
	m_readers->Add(cookie, reader);				// Insert into collection

	return cookie;						// Return registration cookie
}

//---------------------------------------------------------------------------
// SqliteConnection::RegisterVirtualTable
//
// Registers a SqliteVirtualTable<> derived type with the SQLite engine as a
// virtual table implementation
//
// Arguments:
//
//	vtableType		- System::Type of the virtual table
//	moduleName		- Optional module name to assign, can be NULL from overloads

void SqliteConnection::RegisterVirtualTable(Type^ vtableType, String^ moduleName)
{
	GCHandle				gchandle;			// Type GCHandle (strong)
	int						nResult;			// Result from function call

	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckConnectionOpen(this);

	if((vtableType == nullptr) || (moduleName == nullptr)) throw gcnew ArgumentNullException();

	// Make sure that the type provided is something that derives from the
	// necessary base class / interface for Sqlite virtual tables

	if(!SqliteVirtualTableModule::IsValidVirtualTableType(vtableType))
		throw gcnew SqliteExceptions::InvalidVirtualTableException(vtableType);

	gchandle = GCHandle::Alloc(vtableType);		// Allocate the strong GCHandle

	// Attempt to register the type as the implementation for the module name,
	// using a serialized version of the GCHandle as the module context pointer

	nResult = sqlite3_create_module(m_pDatabase->Handle, AutoAnsiString(moduleName),
		SqliteVirtualTableModule::GetMethods(vtableType), GCHandle::ToIntPtr(gchandle).ToPointer());
	if(nResult != SQLITE_OK) {

		// SQLite wasn't happy with us trying to register that module, so release
		// the GCHandle and throw the exception back up to the caller

		gchandle.Free();
		throw gcnew SqliteException(m_pDatabase->Handle, nResult);
	}

	m_modules->Add(gchandle);				// <--- TRACK THE GCHANDLE INSTANCE
}

//---------------------------------------------------------------------------
// SqliteConnection::RollbackInProgress::get (internal)
//
// Plugs a hole in my little pseudo-nested transaction plan.  Determines if
// a portion of a nested transaction has been rolled back, but there are
// still outstanding transaction objects that have not been closed yet
//
// Arguments:
//
//	NONE

bool SqliteConnection::RollbackInProgress::get(void)
{
	// If the current transaction reference count is zero, yet there are 
	// still open transaction objects, a rollback is currently in progress
	// and we shouldn't be doing ANYTHING that might change the state

	return ((m_openTransCount == 0) && (m_openTrans->Count > 0));
}

//---------------------------------------------------------------------------
// SqliteConnection::RollbackTransaction (internal)
//
// Rolls back an outstanding database transaction
//
// Arguments:
//
//	trans		- SqliteTransaction to be committed

void SqliteConnection::RollbackTransaction(SqliteTransaction^ trans)
{
	int				index;				// Index into trans List<T>

	CHECK_DISPOSED(m_disposed);
	if(trans == nullptr) throw gcnew ArgumentNullException();

	// Look for the SqliteTransaction object in our local cache, and if it's
	// not in there, it wasn't created by us so we can't go committing anything

	index = m_openTrans->IndexOf(trans);
	if(index == -1) throw gcnew ArgumentException();
	m_openTrans->RemoveAt(index);

	// Rollback operations are rather different than commit operations, since
	// we support pseudo-nested transactions in this provider.  We always force
	// the transaction count to zero, and if there were ANY outstanding transactions
	// on this connection, we issue the ROLLBACK immediately.
	//
	// NOTE: Because of this methodology, it's important to check the RollbackInProgress
	// property of this connection before executing commands.  It's been mostly taken
	// care of by exposing the new CheckConnectionReady() helper function.  USE IT.

	if(Interlocked::Exchange(m_openTransCount, 0) != 0) 
		SqliteUtil::ExecuteNonQuery(m_pDatabase->Handle, "ROLLBACK TRANSACTION");
}

//---------------------------------------------------------------------------
// SqliteConnection::ServerVersion::get
//
// Gets the database server version, or in this case the SQLite engine version

String^ SqliteConnection::ServerVersion::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return SqliteUtil::FastPtrToStringAnsi(sqlite3_libversion());
}

//---------------------------------------------------------------------------
// SqliteConnection::State::get
//
// Returns the current state of the connection object.  If the connection
// object has been disposed of, returns ConnectionState::Closed instead of
// throwing an ObjectDisposedException like everything else

ConnectionState SqliteConnection::State::get(void)
{
	return (m_disposed) ? ConnectionState::Closed : m_state;
}

//---------------------------------------------------------------------------
// SqliteConnection::StatementProgressFrequency::get
//
// Determines the frequency, in opcodes, that the progress event will fire

int SqliteConnection::StatementProgressFrequency::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return m_progressHook->Frequency;
}

//---------------------------------------------------------------------------
// SqliteConnection::StatementProgressFrequency::set
//
// Changes the frequency, in opcodes, that the progress event will fire.
// Lower values indicate a more frequent firing of the event

void SqliteConnection::StatementProgressFrequency::set(int value)
{
	CHECK_DISPOSED(m_disposed);
	m_progressHook->Frequency = value;
}

//---------------------------------------------------------------------------
// SqliteConnection::SynchronousMode::get
//
// Retrieves the currently set PRAGMA SYNCHRONOUS value

SqliteSynchronousMode SqliteConnection::SynchronousMode::get(void)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckConnectionOpen(this);

	return m_cs->SynchronousMode;			// Return currently set value
}

//---------------------------------------------------------------------------
// SqliteConnection::SynchronousMode::set
//
// Changes the PRAGMA SYNCHRONOUS value.  If the value is different than
// what is already set, the connection string is also updated

void SqliteConnection::SynchronousMode::set(SqliteSynchronousMode value)
{
	String^				query;				// SQL query command text
	int					result;				// Result from a scalar query

	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckConnectionReady(this);		// <-- READY, not OPEN

	// PRAGMA SYNCHRONOUS = { 0 | OFF | 1 | NORMAL | 2 | FULL }
	query = String::Format("PRAGMA SYNCHRONOUS = {0}", static_cast<int>(value));
	SqliteUtil::ExecuteNonQuery(m_pDatabase->Handle, query);

	// Reload the value to make sure we get the configured value, and 
	// apply it to the stored connection string as necessary

	result = Convert::ToInt32(SqliteUtil::ExecuteScalar(m_pDatabase->Handle, "PRAGMA SYNCHRONOUS"));
	
	value = static_cast<SqliteSynchronousMode>(result);
	if(value != m_cs->SynchronousMode) m_cs->SynchronousMode = value;
}

//---------------------------------------------------------------------------
// SqliteConnection::TemporaryStorageFolder::get
//
// Retrieves the currently set PRAGMA TEMP_STORE_DIRECTORY value

String^ SqliteConnection::TemporaryStorageFolder::get(void)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckConnectionOpen(this);

	return m_cs->TemporaryStorageFolder;	// Return currently set value
}

//---------------------------------------------------------------------------
// SqliteConnection::TemporaryStorageFolder::set
//
// Changes the PRAGMA TEMP_STORE_DIRECTORY value.  If the value is different
// than what is already set, the connection string is also updated

void SqliteConnection::TemporaryStorageFolder::set(String^ value)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckConnectionReady(this);		// <-- READY, not OPEN

	// PRAGMA TEMP_STORE_DIRECTORY = { path }
	SqliteUtil::ExecuteNonQuery(m_pDatabase->Handle, String::Format("PRAGMA TEMP_STORE_DIRECTORY = '{0}'", value));

	// Reload the value to make sure we get the configured value, and 
	// apply it to the stored connection string as necessary

	value = SqliteUtil::ExecuteScalar(m_pDatabase->Handle, "PRAGMA TEMP_STORE_DIRECTORY");
	
	if(String::Compare(value, m_cs->TemporaryStorageFolder, true) != 0)
		m_cs->TemporaryStorageFolder = value;
}

//---------------------------------------------------------------------------
// SqliteConnection::TemporaryStorageMode::get
//
// Retrieves the currently set PRAGMA TEMP_STORE value

SqliteTemporaryStorageMode SqliteConnection::TemporaryStorageMode::get(void)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckConnectionOpen(this);

	return m_cs->TemporaryStorageMode;		// Return currently set value
}

//---------------------------------------------------------------------------
// SqliteConnection::TemporaryStorageMode::set
//
// Changes the PRAGMA TEMP_STORE value.  If the value is different than
// what is already set, the connection string is also updated

void SqliteConnection::TemporaryStorageMode::set(SqliteTemporaryStorageMode value)
{
	String^				query;				// SQL query command text
	int					result;				// Result from a scalar query

	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckConnectionReady(this);		// <-- READY, not OPEN

	// PRAGMA TEMP_STORE = { 0 | DEFAULT | 1 | FILE | 2 | MEMORY }
	query = String::Format("PRAGMA TEMP_STORE = {0}", static_cast<int>(value));
	SqliteUtil::ExecuteNonQuery(m_pDatabase->Handle, query);

	// Reload the value to make sure we get the configured value, and 
	// apply it to the stored connection string as necessary

	result = Convert::ToInt32(SqliteUtil::ExecuteScalar(m_pDatabase->Handle, "PRAGMA TEMP_STORE"));
	
	value = static_cast<SqliteTemporaryStorageMode>(result);
	if(value != m_cs->TemporaryStorageMode) m_cs->TemporaryStorageMode = value;
}

//---------------------------------------------------------------------------
// SqliteConnection::TransactionMode::get
//
// Retrieves the configured custom transaction mode for the open database

SqliteTransactionMode SqliteConnection::TransactionMode::get(void)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckConnectionOpen(this);

	return m_transactionMode;				// Return the configured value
}

//---------------------------------------------------------------------------
// SqliteConnection::UnRegisterDataReader (internal)
//
// Removes an existing data reader registration.  Does not throw an exception
// if the cookie is invalid, but will assert in DEBUG builds
//
// Arguments:
//
//	cookie		- Cookie value returned from RegisterDataReader

void SqliteConnection::UnRegisterDataReader(__int64 cookie)
{
	CHECK_DISPOSED(m_disposed);

	Debug::Assert(m_readers->ContainsKey(cookie));
	m_readers->Remove(cookie);
}

//---------------------------------------------------------------------------
// SqliteConnection::Vacuum
//
// Cleans the main database (but not any attached ones) by copying the entire
// thing into a temporary file and then reloading it.  This has no effect
// on databases set up with the PRAGMA AUTO_VACUUM option
//
// Arguments:
//
//	NONE

void SqliteConnection::Vacuum(void)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckConnectionReady(this);		// <-- READY, not OPEN

	SqliteUtil::ExecuteNonQuery(m_pDatabase->Handle, "VACUUM");
}

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)
