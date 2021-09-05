//---------------------------------------------------------------------------
// Copyright (c) 2008-2021 Michael G. Brehm
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
#include "zDBConnection.h"			// Include zDBConnection declarations
#include "zDBCommand.h"				// Include zDBCommand declarations
#include "zDBDataReader.h"			// Include zDBDataReader declarations
#include "zDBMetaData.h"			// Include zDBMetaData declarations
#include "zDBTransaction.h"			// Include zDBTransaction declarations

#pragma warning(push, 4)			// Enable maximum compiler warnings
#pragma warning(disable:4100)		// "unreferenced formal parameter"

BEGIN_NAMESPACE(zuki)
BEGIN_NAMESPACE(data)
BEGIN_NAMESPACE(dbms)

//---------------------------------------------------------------------------
// zDBConnection Destructor

zDBConnection::~zDBConnection()
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

	this->!zDBConnection();			// Invoke the finalizer
	m_disposed = true;				// Object has been disposed of
}

//---------------------------------------------------------------------------
// zDBConnection Finalizer

zDBConnection::!zDBConnection()
{
	if(m_pDatabase) m_pDatabase->Release(this);		// Release the reference
	m_pDatabase = NULL;								// Reset pointer to NULL
}

//---------------------------------------------------------------------------
// zDBConnection::Aggregates::get
//
// Returns a reference to the contained collection of aggregate functions

zDBAggregateCollection^ zDBConnection::Aggregates::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return m_aggregates;
}

//---------------------------------------------------------------------------
// zDBConnection::ApplyConnectionPragmas (private)
//
// Applies all of the necessary PRAGMA commands to a newly opened database
// based on the contents of the stored connection string
//
// Arguments:
//
//	NONE

void zDBConnection::ApplyConnectionPragmas(void)
{
	String^				query;			// SQL query string to set a PRAGMA

	Debug::Assert(m_state == ConnectionState::Open);

	// PRAGMA AUTO_VACUUM = { 0 | 1 }
	query = String::Format("PRAGMA AUTO_VACUUM = {0}", m_cs->AutoVacuum ? "1" : "0");
	zDBUtil::ExecuteNonQuery(m_pDatabase->Handle, query);

	// PRAGMA CACHE_SIZE = { n }
	query = String::Format("PRAGMA CACHE_SIZE = {0}", m_cs->CacheSize);
	zDBUtil::ExecuteNonQuery(m_pDatabase->Handle, query);

	// PRAGMA CASE_SENSITIVE_LIKE = { 0 | 1 }
	query = String::Format("PRAGMA CASE_SENSITIVE_LIKE = {0}", m_cs->CaseSensitiveLike ? "1" : "0");
	zDBUtil::ExecuteNonQuery(m_pDatabase->Handle, query);

	// PRAGMA ENCODING = { UTF-8 | UTF-16 | UTF-16le | UTF-16be }
	query = String::Format("PRAGMA ENCODING = '{0}'", zDBUtil::EncodingToPragma(m_cs->Encoding));
	zDBUtil::ExecuteNonQuery(m_pDatabase->Handle, query);

	// PRAGMA LEGACY_FILE_FORMAT = { 0 | 1 }
	query = String::Format("PRAGMA LEGACY_FILE_FORMAT = {0}", m_cs->CompatibleFileFormat ? "1" : "0");
	zDBUtil::ExecuteNonQuery(m_pDatabase->Handle, query);

	// PRAGMA PAGE_SIZE = { n }
	query = String::Format("PRAGMA PAGE_SIZE = {0}", m_cs->PageSize);
	zDBUtil::ExecuteNonQuery(m_pDatabase->Handle, query);

	// PRAGMA SYNCHRONOUS = { 0 | OFF | 1 | NORMAL | 2 | FULL  }
	query = String::Format("PRAGMA SYNCHRONOUS = {0}", static_cast<int>(m_cs->SynchronousMode));
	zDBUtil::ExecuteNonQuery(m_pDatabase->Handle, query);

	// PRAGMA TEMP_STORE = { 0 | DEFAULT | 1 | FILE | 2 | MEMORY }
	query = String::Format("PRAGMA TEMP_STORE = {0}", static_cast<int>(m_cs->TemporaryStorageMode));
	zDBUtil::ExecuteNonQuery(m_pDatabase->Handle, query);

	// PRAGMA TEMP_STORE_DIRECTORY = { 'path' }
	query = String::Format("PRAGMA TEMP_STORE_DIRECTORY = '{0}'", m_cs->TemporaryStorageFolder);
	zDBUtil::ExecuteNonQuery(m_pDatabase->Handle, query);
}

//---------------------------------------------------------------------------
// zDBConnection::Attach
//
// Attaches another SQLite database as a new catalog in this connection.  The
// attached database is accessed as database-name.table-name.  There is a hard-
// coded maximum of 10 allowed attached databases in the engine
//
// Arguments:
//
//	path			- Path to the database file to be attached
//	databaseName	- Name to assign to the attached database

void zDBConnection::Attach(String^ path, String^ databaseName)
{
	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckConnectionReady(this);		// <-- READY, not OPEN

	String^ query = String::Format("ATTACH DATABASE '{0}' AS [{1}]", path, databaseName);
	zDBUtil::ExecuteNonQuery(m_pDatabase->Handle, query);
}

//---------------------------------------------------------------------------
// zDBConnection::AutoVacuum::get
//
// Retrieves the configured AUTO_VACUUM setting for the open database

bool zDBConnection::AutoVacuum::get(void)
{
	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckConnectionOpen(this);

	return m_autoVacuum;			// Return the configured value
}

//---------------------------------------------------------------------------
// zDBConnection::BeginDbTransaction (protected)
//
// Generic means of creating a new transaction object
//
// Arguments:
//
//	isolationLevel		- Transaction isolation level.  Ignored.

DbTransaction^ zDBConnection::BeginDbTransaction(Data::IsolationLevel isolationLevel)
{
	return BeginTransaction(zDBLockMode::Deferred);
}

//---------------------------------------------------------------------------
// zDBConnection::BeginTransaction
//
// Begins a new transaction against this database connection
//
// Arguments:
//
//	mode		- Locking mode to use for the transaction

zDBTransaction^ zDBConnection::BeginTransaction(zDBLockMode mode)
{
	zDBTransaction^				trans;			// The new transaction object

	CHECK_DISPOSED(m_disposed);
	ExecutePermission->Demand();
	zDBUtil::CheckConnectionReady(this);

	if((m_openTrans->Count > 0) && (m_transactionMode == zDBTransactionMode::Single))
		throw gcnew Exception("TODO:nested transaction exception");

	if((m_openTrans->Count > 0) && (mode != m_openTransMode))
		throw gcnew Exception("TODO:different transaction lock mode exception");

	if(Interlocked::Increment(m_openTransCount) == 1) {

		switch(mode) {

			case zDBLockMode::Exclusive: 
				zDBUtil::ExecuteNonQuery(m_pDatabase->Handle, "BEGIN EXCLUSIVE TRANSACTION");
				break;

			case zDBLockMode::Immediate: 
				zDBUtil::ExecuteNonQuery(m_pDatabase->Handle, "BEGIN IMMEDIATE TRANSACTION");
				break;

			default: zDBUtil::ExecuteNonQuery(m_pDatabase->Handle, "BEGIN DEFERRED TRANSACTION");
		}

		m_openTransMode = mode;
	}

	trans = gcnew zDBTransaction(this);
	m_openTrans->Add(trans);
	return trans;
}

//---------------------------------------------------------------------------
// zDBConnection::BooleanFormat::get
//
// Retrieves the configured boolean formatting and coercion specifier

zDBBooleanFormat zDBConnection::BooleanFormat::get(void)
{
	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckConnectionOpen(this);

	return m_cs->BooleanFormat;			// Return currently set value
}

//---------------------------------------------------------------------------
// zDBConnection::BooleanFormat::set
//
// Changes the boolan formatting and coercion format.  If the format is 
// different than what is already set, the connection string is also updated

void zDBConnection::BooleanFormat::set(zDBBooleanFormat value)
{
	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckConnectionOpen(this);

	// Since these properties have no representation in the engine itself,
	// we can just use the connection string to hold the current state

	if(value != m_cs->BooleanFormat) m_cs->BooleanFormat = value;
}

//---------------------------------------------------------------------------
// zDBConnection::CacheSize::get
//
// Retrieves the currently set PRAGMA CACHE_SIZE value

int zDBConnection::CacheSize::get(void)
{
	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckConnectionOpen(this);

	return m_cs->CacheSize;				// Return currently set value
}

//---------------------------------------------------------------------------
// zDBConnection::CacheSize::set
//
// Changes the PRAGMA CACHE_SIZE value.  If the value is different than
// what is already set, the connection string is also updated

void zDBConnection::CacheSize::set(int value)
{
	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckConnectionReady(this);		// <-- READY, not OPEN

	if(value < 0) throw gcnew ArgumentOutOfRangeException();

	// PRAGMA CACHE_SIZE = { n }
	zDBUtil::ExecuteNonQuery(m_pDatabase->Handle, String::Format("PRAGMA CACHE_SIZE = {0}", value));

	// Reload the value to make sure we get the configured value, and 
	// apply it to the stored connection string as necessary

	value = Convert::ToInt32(zDBUtil::ExecuteScalar(m_pDatabase->Handle, "PRAGMA CACHE_SIZE"));
	if(value != m_cs->CacheSize) m_cs->CacheSize = value;
}

//---------------------------------------------------------------------------
// zDBConnection::CaseSensitiveLike::get
//
// Retrieves the currently set PRAGMA CASE_SENSITIVE_LIKE value

bool zDBConnection::CaseSensitiveLike::get(void)
{
	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckConnectionOpen(this);

	return m_cs->CaseSensitiveLike;			// Return currently set value
}

//---------------------------------------------------------------------------
// zDBConnection::CaseSensitiveLike::set
//
// Changes the PRAGMA CASE_SENSITIVE_LIKE value.  If the value is different
// than what is already set, the connection string is also updated

void zDBConnection::CaseSensitiveLike::set(bool value)
{
	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckConnectionReady(this);		// <-- READY, not OPEN

	// PRAGMA CASE_SENSITIVE_LIKE = { 0 | 1 }
	zDBUtil::ExecuteNonQuery(m_pDatabase->Handle, String::Format("PRAGMA CASE_SENSITIVE_LIKE = {0}", value ? 1 : 0));

	// This particular PRAGMA doesn't appear to work property, so we read it
	// by asking to compare a lowercase 'a' with an uppercase 'A' instead
	ENGINE_ISSUE(3.3.8, "PRAGMA CASE_SENSITIVE_LIKE does not return a value when queried");

	value = (Convert::ToInt32(zDBUtil::ExecuteScalar(m_pDatabase->Handle, "SELECT 'a' NOT LIKE 'A'")) != 0);
	if(value != m_cs->CaseSensitiveLike) m_cs->CaseSensitiveLike = value;
}

//---------------------------------------------------------------------------
// zDBConnection::CheckIntegrity
//
// Detaches a previously attached SQLite database file by it's catalog name
//
// Arguments:
//
//	databaseName	- Name of the database to be detached

String^ zDBConnection::CheckIntegrity(void)
{
	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckConnectionReady(this);		// <-- READY, not OPEN

	return zDBUtil::ExecuteScalar(m_pDatabase->Handle, "PRAGMA INTEGRITY_CHECK");
}

//---------------------------------------------------------------------------
// zDBConnection::Close (private)
//
// Closes the connection to the database.  Any outstanding transactions will
// be rolled back, and any outstanding data readers will be disposed of
//
// Arguments:
//
//	fireStateChange		- Argument used to control firing of OnStateChange

void zDBConnection::Close(bool fireStateChange)
{
	List<zDBDataReader^>^		readers;		// Data Readers to be closed

	CHECK_DISPOSED(m_disposed);
	if(m_state == ConnectionState::Closed) return;

	Interrupt();					// Attempt to interrupt anything going on

	// Rollback any and all outstanding transaction objects against this connection

	for each(zDBTransaction^ trans in m_openTrans) trans->Rollback();

	// Automatically dispose of any active data readers that remain
	// open against this connection.  Since the data readers will remove
	// themselves on disposal, use a COPY of the collection to avoid screwing
	// up the enumerator on the main Dictionary<> instance here

	readers = gcnew List<zDBDataReader^>();

	for each(KeyValuePair<__int64, zDBDataReader^>^ item in m_readers)
		if(!item->Value->IsClosed) readers->Add(item->Value);

	for each(zDBDataReader^ reader in readers) delete reader;

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
// zDBConnection::Collations::get
//
// Returns a reference to the contained collection of collation functions

zDBCollationCollection^ zDBConnection::Collations::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return m_collations;
}

//---------------------------------------------------------------------------
// zDBConnection::CommitTransaction (internal)
//
// Commits an outstanding database transaction
//
// Arguments:
//
//	trans		- zDBTransaction to be committed

void zDBConnection::CommitTransaction(zDBTransaction^ trans)
{
	int				index;				// Index into trans List<T>

	CHECK_DISPOSED(m_disposed);
	if(trans == nullptr) throw gcnew ArgumentNullException();

	// Look for the zDBTransaction object in our local cache, and if it's
	// not in there, it wasn't created by us so we can't go committing anything

	index = m_openTrans->IndexOf(trans);
	if(index == -1) throw gcnew ArgumentException();
	m_openTrans->RemoveAt(index);

	// Even if the zDBTransaction exists, we can be at a transaction count
	// of zero if a nested ROLLBACK has occurred on this connection

	if(m_openTransCount == 0) throw gcnew InvalidOperationException();

	// Decrement the outstanding transaction counter, and if it has fallen
	// to zero from this commit, go ahead and try to commit the transaction

	if(Interlocked::Decrement(m_openTransCount) == 0)
		zDBUtil::ExecuteNonQuery(m_pDatabase->Handle, "COMMIT TRANSACTION");
}

//---------------------------------------------------------------------------
// zDBConnection::CompatibleFileFormat::get
//
// Retrieves the configured LEGACY_FILE_FORMAT setting for the open database

bool zDBConnection::CompatibleFileFormat::get(void)
{
	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckConnectionOpen(this);

	return m_compatibleFormat;		// Return the configured value
}

//---------------------------------------------------------------------------
// zDBConnection::ConnectionString::get
//
// Returns a copy of the current connection's connection string.  Properties
// that are configurable while the connection is open will be reflected here

String^ zDBConnection::ConnectionString::get(void)
{
	CHECK_DISPOSED(m_disposed);

	return m_cs->ConnectionString;		// Return the connection string
}

//---------------------------------------------------------------------------
// zDBConnection::ConnectionString::set
//
// Alters the database connection string.  Cannot be changed unless the
// connection is closed, however any dynamic properties explicitly changed
// while the connection is open will be reflected by the getter

void zDBConnection::ConnectionString::set(String^ value)
{
	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckConnectionClosed(this);

	m_cs->ConnectionString = value;		// Change the connection string
}

//---------------------------------------------------------------------------
// zDBConnection::Construct (private)
//
// Helper function used to implement the meat of the various constructors
//
// Arguments:
//
//	connectionString		- Optional connection string to initialize with

void zDBConnection::Construct(String^ connectionString)
{
	m_state = ConnectionState::Closed;
	m_cs = gcnew zDBConnectionStringBuilder(connectionString);
	m_openTrans = gcnew List<zDBTransaction^>();
	m_fieldKey = gcnew zDBCryptoKey(gcnew SecureString());

	m_authHook = gcnew zDBConnectionAuthorizationHook(this);
	m_collationHook = gcnew zDBConnectionCollationNeededHook(this);
	m_commitHook = gcnew zDBConnectionCommitHook(this);
	m_profileHook = gcnew zDBConnectionProfileHook(this);
	m_progressHook = gcnew zDBConnectionProgressHook(this);
	m_rollbackHook = gcnew zDBConnectionRollbackHook(this);
	m_traceHook = gcnew zDBConnectionTraceHook(this);
	m_updateHook = gcnew zDBConnectionUpdateHook(this);

	m_readers = gcnew Dictionary<__int64, zDBDataReader^>();
	m_modules = gcnew List<GCHandle>();

	m_aggregates = gcnew zDBAggregateCollection();
	m_collations = gcnew zDBCollationCollection();
	m_functions = gcnew zDBFunctionCollection();
}

//---------------------------------------------------------------------------
// zDBConnection::CreateCommand
//
// Creates and returns a new zDBCommand object with the Connection property
// already set up to use this connection object
//
// Arguments:
//
//	NONE

zDBCommand^ zDBConnection::CreateCommand(void)
{
	CHECK_DISPOSED(m_disposed);
	return gcnew zDBCommand(String::Empty, this);
}

//---------------------------------------------------------------------------
// zDBConnection::CreateDbCommand (protected)
//
// Implements the generic method for creating a command object
//
// Arguments:
//
//	NONE

DbCommand^ zDBConnection::CreateDbCommand(void)
{
	return CreateCommand();				// Use the specific implementation
}

//---------------------------------------------------------------------------
// zDBConnection::CreateExecutePermission (private, static)
//
// Constructs and initializes the value for the ExecutePermission field
//
// Arguments:
//
//	NONE

CodeAccessPermission^ zDBConnection::CreateExecutePermission(void)
{
	// This is taken lock, stock, and barrel from the SqlClientConnection
	// class.  I'm making an assumption it will work dandy until I take
	// the time to determine exactly what, if anything, I want to customize
	// for the Code Access Security of this ADO.NET provider.

	zDBPermission^ perm = gcnew zDBPermission(PermissionState::None);
	perm->Add(String::Empty, String::Empty, KeyRestrictionBehavior::AllowOnly);
	return perm;
}

//---------------------------------------------------------------------------
// zDBConnection::Database::get
//
// Retrieves the name of the current catalog/database, or in the case of
// SQLite a hard-coded name of "main" since catalogs aren't supported

String^ zDBConnection::Database::get(void)
{
	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckConnectionOpen(this);

	return MAIN_CATALOG_NAME;		// Always set to "main" no matter what
}

//---------------------------------------------------------------------------
// zDBConnection::DataSource::get
//
// Retrieves the file name of the main database

String^ zDBConnection::DataSource::get(void)
{
	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckConnectionOpen(this);

	return m_cs->DataSource;			// Return the database file name
}

//---------------------------------------------------------------------------
// zDBConnection::DateTimeFormat::get
//
// Retrieves the configured date/time formatting and coercion specifier

zDBDateTimeFormat zDBConnection::DateTimeFormat::get(void)
{
	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckConnectionOpen(this);

	return m_cs->DateTimeFormat;		// Return currently set value
}

//---------------------------------------------------------------------------
// zDBConnection::DateTimeFormat::set
//
// Changes the date/time formatting and coercion format.  If the format is 
// different than what is already set, the connection string is also updated

void zDBConnection::DateTimeFormat::set(zDBDateTimeFormat value)
{
	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckConnectionOpen(this);

	// Since these properties have no representation in the engine itself,
	// we can just use the connection string to hold the current state

	if(value != m_cs->DateTimeFormat) m_cs->DateTimeFormat = value;
}

//---------------------------------------------------------------------------
// zDBConnection::Detach
//
// Detaches a previously attached SQLite database file by it's catalog name
//
// Arguments:
//
//	databaseName	- Name of the database to be detached

void zDBConnection::Detach(String^ databaseName)
{
	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckConnectionReady(this);		// <-- READY, not OPEN

	String^ query = String::Format("DETACH DATABASE [{0}]", databaseName);
	zDBUtil::ExecuteNonQuery(m_pDatabase->Handle, query);
}

//---------------------------------------------------------------------------
// zDBConnection::Encoding::get
//
// Retrieves the configured ENCODING setting for the open database

zDBTextEncodingMode zDBConnection::Encoding::get(void)
{
	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckConnectionOpen(this);

	return m_encoding;				// Return the configured value
}

//---------------------------------------------------------------------------
// zDBConnection::FieldEncryptionKey::get (internal)
//
// Returns the HCRYPTKEY for the field level encryption

HCRYPTKEY zDBConnection::FieldEncryptionKey::get(void)
{
	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckConnectionOpen(this);

	return m_fieldKey->Key;
}

//---------------------------------------------------------------------------
// zDBConnection::FieldEncryptionPassword::set
//
// Changes the encryption password to be used when working with the built-in
// ENCRYPT() and DECRYPT() scalar functions

void zDBConnection::FieldEncryptionPassword::set(SecureString^ value)
{
	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckConnectionOpen(this);

	delete m_fieldKey;
	m_fieldKey = gcnew zDBCryptoKey(value); 
}

//---------------------------------------------------------------------------
// zDBConnection::FindConnection (static, internal)
//
// Attempts to map a standard sqlite3* handle into it's parent zDBConnection
// object instance.  Used primarily by virtual tables.
//
// Arguments:
//
//	hDatabase		- Database handle to locate the zDBConnection for

zDBConnection^ zDBConnection::FindConnection(sqlite3* hDatabase)
{
	zDBConnection^			conn;			// Located connection handle

	intptr_t ptHandle = reinterpret_cast<intptr_t>(hDatabase);

	if(s_handleMapper->TryGetValue(ptHandle, conn)) return conn;
	else return nullptr;
}

//---------------------------------------------------------------------------
// zDBConnection::Functions::get
//
// Returns a reference to the contained collection of scalar functions

zDBFunctionCollection^ zDBConnection::Functions::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return m_functions;
}

//---------------------------------------------------------------------------
// zDBConnection::GetHandle (internal)
//
// Retrieves a reference counted copy of the contained database handle.
// The caller must use Release() when finished, much like in COM objects
//
// Arguments:
//
//	ppDatabaseHandle	- Pointer to receive the copy of the handle class

void zDBConnection::GetHandle(DatabaseHandle **ppDatabaseHandle)
{
	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckConnectionOpen(this);

	*ppDatabaseHandle = m_pDatabase;		// Copy the instance pointer
	m_pDatabase->AddRef(this);				// AddRef() before returning
}

//---------------------------------------------------------------------------
// zDBConnection::GetSchema
//
// Retrieves a DataTable containing database schema information
//
// Arguments:
//
//	collectionName		- Specifies the name of the schema to return
//	restrictionValues	- Specifies the restriction values for the schema

DataTable^ zDBConnection::GetSchema(String^ collectionName, array<String^>^ restrictionValues)
{
	CHECK_DISPOSED(m_disposed);
	ExecutePermission->Demand();

	// The details of implementing metadata are quite ugly and tedious,
	// so they have been offloaded into a class of their own ...

	return zDBMetaData::Generate(this, collectionName, restrictionValues);
}

//---------------------------------------------------------------------------
// zDBConnection::GuidFormat::get
//
// Retrieves the configured GUID formatting and coercion specifier

zDBGuidFormat zDBConnection::GuidFormat::get(void)
{
	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckConnectionOpen(this);

	return m_cs->GuidFormat;		// Return currently set value
}

//---------------------------------------------------------------------------
// zDBConnection::GuidFormat::set
//
// Changes the GUID formatting and coercion format.  If the format is 
// different than what is already set, the connection string is also updated

void zDBConnection::GuidFormat::set(zDBGuidFormat value)
{
	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckConnectionOpen(this);

	// Since these properties have no representation in the engine itself,
	// we can just use the connection string to hold the current state

	if(value != m_cs->GuidFormat) m_cs->GuidFormat = value;
}

//---------------------------------------------------------------------------
// zDBConnection::Handle::get (internal)
//
// Retrieves a non referenced counted copy of the contained database handle.
// If a reference counted copy is required, use the GetHandle() method instead
// of just AddRef-ing the returned pointer

DatabaseHandle& zDBConnection::Handle::get(void)
{
	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckConnectionOpen(this);

	return *m_pDatabase;					// Return a class reference
}

//---------------------------------------------------------------------------
// zDBConnection::HandlePointer::get (internal)
//
// Retrieves a non referenced counted pointer to the contained database handle.
// If a reference counted copy is required, use the GetHandle() method instead
// of just AddRef-ing the returned pointer

DatabaseHandle* zDBConnection::HandlePointer::get(void)
{
	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckConnectionOpen(this);

	return m_pDatabase;					// Return a class pointer
}

//---------------------------------------------------------------------------
// zDBConnection::Interrupt (internal)
//
// Attempts to abort any currently pending database operations at the
// SQLite engine level.  Does not indicate success or failure, though.
//
// Arguments:
//
//	NONE

void zDBConnection::Interrupt(void)
{
	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckConnectionOpen(this);

	sqlite3_interrupt(m_pDatabase->Handle);		// Abort pending operations
}

//---------------------------------------------------------------------------
// zDBConnection::InTransaction::get (internal)
//
// Determines if the ENGINE is currently in a transaction.  This has nothing
// to do with the provider transactions, although one would expect that if
// the provider is in a transaction, the engine better be as well

bool zDBConnection::InTransaction::get(void)
{
	CHECK_DISPOSED(m_disposed);
	if(m_state == ConnectionState::Closed) return false;

	// If the engine is in autocommit mode (non-zero return value), there
	// is no specific transaction in progress

	return (sqlite3_get_autocommit(m_pDatabase->Handle) == 0);
}

//---------------------------------------------------------------------------
// zDBConnection::LoadConfiguredPragmas (private)
//
// Loads the pragmas that may be permanent in the open database and sets the
// values of the member variables that expose them to the application
//
// Arguments:
//
//	NONE

void zDBConnection::LoadConfiguredPragmas(void)
{
	String^				result;			// Result from a scalar query

	Debug::Assert(m_state == ConnectionState::Open);

	// PRAGMA AUTO_VACUUM
	result = zDBUtil::ExecuteScalar(m_pDatabase->Handle, "PRAGMA AUTO_VACUUM");
	m_autoVacuum = (Convert::ToInt32(result) == 1);

	// PRAGMA LEGACY_FILE_FORMAT
	result = zDBUtil::ExecuteScalar(m_pDatabase->Handle, "PRAGMA LEGACY_FILE_FORMAT");
	m_compatibleFormat = (Convert::ToInt32(result) == 1);

	// PRAGMA ENCODING
	result = zDBUtil::ExecuteScalar(m_pDatabase->Handle, "PRAGMA ENCODING");
	m_encoding = zDBUtil::PragmaToEncoding(result);

	// PRAGMA PAGE_SIZE
	result = zDBUtil::ExecuteScalar(m_pDatabase->Handle, "PRAGMA PAGE_SIZE");
	m_pageSize = Convert::ToInt32(result);
}

//---------------------------------------------------------------------------
// zDBConnection::Open
//
// Opens the database connection, utilizing the contents of the stored
// connection string.  Also reads all of the non-modifiable connection
// properties to ensure that they are correct for the current database
//
// Arguments:
//
//	NONE

void zDBConnection::Open(void)
{
	sqlite3*				hDatabase = NULL;		// The new database handle
	int						nResult;				// Result from function call

	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckConnectionClosed(this);

	Debug::Assert(m_pDatabase == NULL);				// Should always be NULL here
	
	// CONNECTION STRING CODE ACCESS SECURITY
	//
	// The SqlClient provider jumps through a million hoops to accomplish something
	// that essentially boils down to this (I think).  What we do is create a new
	// zDBPermission object with the connection string we're about to use, and then
	// demand that new permission.  If anything has been set in the configuration or
	// in the code of the application to prevent this connection string, we'll die
	// right here and now with a SecurityException.
	//
	// For some examples on what values to set to restrict the connection string, 
	// start with the MSDN topic SqlClientPermission.Add() and work from there.

	zDBPermission^ openPerm = gcnew zDBPermission(PermissionState::None);
	openPerm->Add(m_cs->ToString(), String::Empty, KeyRestrictionBehavior::AllowOnly);
	openPerm->Demand();

	// Attempt to open the main SQLite database handle.  Note that we have to
	// use the ANSI version at all times, according to the SQLite documentation.
	// We also have to be sure to call sqlite3_close in the event of an error

	nResult = sqlite3_open(AutoAnsiString(m_cs->DataSource), &hDatabase);
	if(nResult != SQLITE_OK) { sqlite3_close(hDatabase); throw gcnew zDBException(nResult); }

	// I can't seem to find a place in SQLite that actually uses the extended error
	// codes, at least in the Windows build.  There is no reason to enable this until
	// they are used.  zDBException will need to change as well to accomodate it

	ENGINE_ISSUE(3.3.8, "Extended result codes are seemingly unused in the engine");
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
// zDBConnection::PageSize::get
//
// Retrieves the configured page size for the open database

int zDBConnection::PageSize::get(void)
{
	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckConnectionOpen(this);

	return m_pageSize;				// Return the configured value
}

//---------------------------------------------------------------------------
// zDBConnection::RegisterDataReader (internal)
//
// Registers a zDBDataReader with this connection so it can be automatically
// disposed of when the connection is closed
//
// Arguments:
//
//	reader		- zDBDataReader to be registered

__int64 zDBConnection::RegisterDataReader(zDBDataReader^ reader)
{
	__int64				cookie;			// Cookie to be returned

	CHECK_DISPOSED(m_disposed);
	if(reader == nullptr) throw gcnew ArgumentNullException();

	cookie = Interlocked::Increment(s_cookie);	// Increment cookie
	m_readers->Add(cookie, reader);				// Insert into collection

	return cookie;						// Return registration cookie
}

//---------------------------------------------------------------------------
// zDBConnection::RegisterVirtualTable
//
// Registers a zDBVirtualTable<> derived type with the SQLite engine as a
// virtual table implementation
//
// Arguments:
//
//	vtableType		- System::Type of the virtual table
//	moduleName		- Optional module name to assign, can be NULL from overloads

void zDBConnection::RegisterVirtualTable(Type^ vtableType, String^ moduleName)
{
	GCHandle				gchandle;			// Type GCHandle (strong)
	int						nResult;			// Result from function call

	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckConnectionOpen(this);

	if((vtableType == nullptr) || (moduleName == nullptr)) throw gcnew ArgumentNullException();

	// Make sure that the type provided is something that derives from the
	// necessary base class / interface for zDB virtual tables

	if(!zDBVirtualTableModule::IsValidVirtualTableType(vtableType))
		throw gcnew zDBExceptions::InvalidVirtualTableException(vtableType);

	gchandle = GCHandle::Alloc(vtableType);		// Allocate the strong GCHandle

	// Attempt to register the type as the implementation for the module name,
	// using a serialized version of the GCHandle as the module context pointer

	nResult = sqlite3_create_module(m_pDatabase->Handle, AutoAnsiString(moduleName),
		zDBVirtualTableModule::GetMethods(vtableType), GCHandle::ToIntPtr(gchandle).ToPointer());
	if(nResult != SQLITE_OK) {

		// SQLite wasn't happy with us trying to register that module, so release
		// the GCHandle and throw the exception back up to the caller

		gchandle.Free();
		throw gcnew zDBException(m_pDatabase->Handle, nResult);
	}

	m_modules->Add(gchandle);				// <--- TRACK THE GCHANDLE INSTANCE
}

//---------------------------------------------------------------------------
// zDBConnection::RollbackInProgress::get (internal)
//
// Plugs a hole in my little pseudo-nested transaction plan.  Determines if
// a portion of a nested transaction has been rolled back, but there are
// still outstanding transaction objects that have not been closed yet
//
// Arguments:
//
//	NONE

bool zDBConnection::RollbackInProgress::get(void)
{
	// If the current transaction reference count is zero, yet there are 
	// still open transaction objects, a rollback is currently in progress
	// and we shouldn't be doing ANYTHING that might change the state

	return ((m_openTransCount == 0) && (m_openTrans->Count > 0));
}

//---------------------------------------------------------------------------
// zDBConnection::RollbackTransaction (internal)
//
// Rolls back an outstanding database transaction
//
// Arguments:
//
//	trans		- zDBTransaction to be committed

void zDBConnection::RollbackTransaction(zDBTransaction^ trans)
{
	int				index;				// Index into trans List<T>

	CHECK_DISPOSED(m_disposed);
	if(trans == nullptr) throw gcnew ArgumentNullException();

	// Look for the zDBTransaction object in our local cache, and if it's
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
		zDBUtil::ExecuteNonQuery(m_pDatabase->Handle, "ROLLBACK TRANSACTION");
}

//---------------------------------------------------------------------------
// zDBConnection::ServerVersion::get
//
// Gets the database server version, or in this case the SQLite engine version

String^ zDBConnection::ServerVersion::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return zDBUtil::FastPtrToStringAnsi(sqlite3_libversion());
}

//---------------------------------------------------------------------------
// zDBConnection::State::get
//
// Returns the current state of the connection object.  If the connection
// object has been disposed of, returns ConnectionState::Closed instead of
// throwing an ObjectDisposedException like everything else

ConnectionState zDBConnection::State::get(void)
{
	return (m_disposed) ? ConnectionState::Closed : m_state;
}

//---------------------------------------------------------------------------
// zDBConnection::StatementProgressFrequency::get
//
// Determines the frequency, in opcodes, that the progress event will fire

int zDBConnection::StatementProgressFrequency::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return m_progressHook->Frequency;
}

//---------------------------------------------------------------------------
// zDBConnection::StatementProgressFrequency::set
//
// Changes the frequency, in opcodes, that the progress event will fire.
// Lower values indicate a more frequent firing of the event

void zDBConnection::StatementProgressFrequency::set(int value)
{
	CHECK_DISPOSED(m_disposed);
	m_progressHook->Frequency = value;
}

//---------------------------------------------------------------------------
// zDBConnection::SynchronousMode::get
//
// Retrieves the currently set PRAGMA SYNCHRONOUS value

zDBSynchronousMode zDBConnection::SynchronousMode::get(void)
{
	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckConnectionOpen(this);

	return m_cs->SynchronousMode;			// Return currently set value
}

//---------------------------------------------------------------------------
// zDBConnection::SynchronousMode::set
//
// Changes the PRAGMA SYNCHRONOUS value.  If the value is different than
// what is already set, the connection string is also updated

void zDBConnection::SynchronousMode::set(zDBSynchronousMode value)
{
	String^				query;				// SQL query command text
	int					result;				// Result from a scalar query

	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckConnectionReady(this);		// <-- READY, not OPEN

	// PRAGMA SYNCHRONOUS = { 0 | OFF | 1 | NORMAL | 2 | FULL }
	query = String::Format("PRAGMA SYNCHRONOUS = {0}", static_cast<int>(value));
	zDBUtil::ExecuteNonQuery(m_pDatabase->Handle, query);

	// Reload the value to make sure we get the configured value, and 
	// apply it to the stored connection string as necessary

	result = Convert::ToInt32(zDBUtil::ExecuteScalar(m_pDatabase->Handle, "PRAGMA SYNCHRONOUS"));
	
	value = static_cast<zDBSynchronousMode>(result);
	if(value != m_cs->SynchronousMode) m_cs->SynchronousMode = value;
}

//---------------------------------------------------------------------------
// zDBConnection::TemporaryStorageFolder::get
//
// Retrieves the currently set PRAGMA TEMP_STORE_DIRECTORY value

String^ zDBConnection::TemporaryStorageFolder::get(void)
{
	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckConnectionOpen(this);

	return m_cs->TemporaryStorageFolder;	// Return currently set value
}

//---------------------------------------------------------------------------
// zDBConnection::TemporaryStorageFolder::set
//
// Changes the PRAGMA TEMP_STORE_DIRECTORY value.  If the value is different
// than what is already set, the connection string is also updated

void zDBConnection::TemporaryStorageFolder::set(String^ value)
{
	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckConnectionReady(this);		// <-- READY, not OPEN

	// PRAGMA TEMP_STORE_DIRECTORY = { path }
	zDBUtil::ExecuteNonQuery(m_pDatabase->Handle, String::Format("PRAGMA TEMP_STORE_DIRECTORY = '{0}'", value));

	// Reload the value to make sure we get the configured value, and 
	// apply it to the stored connection string as necessary

	value = zDBUtil::ExecuteScalar(m_pDatabase->Handle, "PRAGMA TEMP_STORE_DIRECTORY");
	
	if(String::Compare(value, m_cs->TemporaryStorageFolder, true) != 0)
		m_cs->TemporaryStorageFolder = value;
}

//---------------------------------------------------------------------------
// zDBConnection::TemporaryStorageMode::get
//
// Retrieves the currently set PRAGMA TEMP_STORE value

zDBTemporaryStorageMode zDBConnection::TemporaryStorageMode::get(void)
{
	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckConnectionOpen(this);

	return m_cs->TemporaryStorageMode;		// Return currently set value
}

//---------------------------------------------------------------------------
// zDBConnection::TemporaryStorageMode::set
//
// Changes the PRAGMA TEMP_STORE value.  If the value is different than
// what is already set, the connection string is also updated

void zDBConnection::TemporaryStorageMode::set(zDBTemporaryStorageMode value)
{
	String^				query;				// SQL query command text
	int					result;				// Result from a scalar query

	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckConnectionReady(this);		// <-- READY, not OPEN

	// PRAGMA TEMP_STORE = { 0 | DEFAULT | 1 | FILE | 2 | MEMORY }
	query = String::Format("PRAGMA TEMP_STORE = {0}", static_cast<int>(value));
	zDBUtil::ExecuteNonQuery(m_pDatabase->Handle, query);

	// Reload the value to make sure we get the configured value, and 
	// apply it to the stored connection string as necessary

	result = Convert::ToInt32(zDBUtil::ExecuteScalar(m_pDatabase->Handle, "PRAGMA TEMP_STORE"));
	
	value = static_cast<zDBTemporaryStorageMode>(result);
	if(value != m_cs->TemporaryStorageMode) m_cs->TemporaryStorageMode = value;
}

//---------------------------------------------------------------------------
// zDBConnection::TransactionMode::get
//
// Retrieves the configured custom transaction mode for the open database

zDBTransactionMode zDBConnection::TransactionMode::get(void)
{
	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckConnectionOpen(this);

	return m_transactionMode;				// Return the configured value
}

//---------------------------------------------------------------------------
// zDBConnection::UnRegisterDataReader (internal)
//
// Removes an existing data reader registration.  Does not throw an exception
// if the cookie is invalid, but will assert in DEBUG builds
//
// Arguments:
//
//	cookie		- Cookie value returned from RegisterDataReader

void zDBConnection::UnRegisterDataReader(__int64 cookie)
{
	CHECK_DISPOSED(m_disposed);

	Debug::Assert(m_readers->ContainsKey(cookie));
	m_readers->Remove(cookie);
}

//---------------------------------------------------------------------------
// zDBConnection::Vacuum
//
// Cleans the main database (but not any attached ones) by copying the entire
// thing into a temporary file and then reloading it.  This has no effect
// on databases set up with the PRAGMA AUTO_VACUUM option
//
// Arguments:
//
//	NONE

void zDBConnection::Vacuum(void)
{
	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckConnectionReady(this);		// <-- READY, not OPEN

	zDBUtil::ExecuteNonQuery(m_pDatabase->Handle, "VACUUM");
}

//---------------------------------------------------------------------------

END_NAMESPACE(dbms)
END_NAMESPACE(data)
END_NAMESPACE(zuki)

#pragma warning(pop)
