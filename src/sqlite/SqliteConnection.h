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

#ifndef __SQLITECONNECTION_H_
#define __SQLITECONNECTION_H_
#pragma once

#include "DatabaseExtensions.h"			// Include DatabaseExtensions decls
#include "DatabaseHandle.h"				// Include DatabaseHandle declarations
#include "SqliteAggregateCollection.h"		// Include SqliteAggregateCollection decls
#include "SqliteCollationCollection.h"		// Include SqliteCollationCollection decls
#include "SqliteConnectionHooks.h"			// Include Sqlite connection hook decls
#include "SqliteConnectionStringBuilder.h"	// Include SqliteConnectionStringBuilder
#include "SqliteCryptoKey.h"				// Include SqliteCryptoKey declarations
#include "SqliteDelegates.h"				// Include Sqlite delegate declarations
#include "SqliteEnumerations.h"			// Include Sqlite enumeration declarations
#include "SqliteExceptions.h"				// Include Sqlite exception declarations
#include "SqliteFunctionCollection.h"		// Include SqliteFunctionCollection decls
#include "SqlitePermission.h"				// Include SqlitePermission declarations
#include "SqliteUtil.h"					// Include SqliteUtil class declarations
#include "SqliteVirtualTableModule.h"		// Include SqliteVirtualTableModule decls

#pragma warning(push, 4)				// Enable maximum compiler warnings
#pragma warning(disable:4100)			// "unreferenced formal parameter"

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Collections::Specialized;
using namespace System::ComponentModel;
using namespace System::Data;
using namespace System::Data::Common;
using namespace System::Security;
using namespace System::Threading;
using namespace System::Transactions;

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// Forward Class Declarations
//---------------------------------------------------------------------------

ref class SqliteCommand;					// SqliteCommand.h
ref class SqliteMetaData;					// SqliteMetaData.h
ref class SqliteTransaction;				// SqliteTransaction.h

//---------------------------------------------------------------------------
// Class SqliteConnection
//
// Represents an open connection to a SQLite database.  Unlike the previous 
// version of this ADO.NET provider, all create-only options MUST be 
// specified as part of the connection string.  This was done to align more
// closely with the way the built-in ADO.NET providers operate.
//---------------------------------------------------------------------------

public ref class SqliteConnection sealed : public DbConnection, public ITrackableObject
{
public:

	//-----------------------------------------------------------------------
	// Constructors

	SqliteConnection() { Construct(nullptr); }
	SqliteConnection(String^ connectionString) { Construct(connectionString); }

	//-----------------------------------------------------------------------
	// Events

	// Authorize
	//
	// Fired whenever an action that needs to be authorized occurs in the database
	event SqliteAuthorizeEventHandler^ Authorize
	{
		void add(SqliteAuthorizeEventHandler^ handler) { m_authHook->Add(handler); }
		void remove(SqliteAuthorizeEventHandler^ handler) { m_authHook->Remove(handler); }
	}

	// CollationNeeded
	//
	// Fired whenever an unknown collation sequence has been encountered to give
	// the handler a change to register one
	event SqliteCollationNeededEventHandler^ CollationNeeded
	{
		void add(SqliteCollationNeededEventHandler^ handler) { m_collationHook->Add(handler); }
		void remove(SqliteCollationNeededEventHandler^ handler) { m_collationHook->Remove(handler); }
	}

	// RowChanged
	//
	// Fired whenever a row has been changed in a database table
	event SqliteRowChangedEventHandler^ RowChanged
	{
		void add(SqliteRowChangedEventHandler^ handler) { m_updateHook->Add(handler); }
		void remove(SqliteRowChangedEventHandler^ handler) { m_updateHook->Remove(handler); }
	}

	// StatementProgress
	//
	// Fired periodically to allow the application to do things during a long
	// running database query.
	event CancelEventHandler^ StatementProgress
	{
		void add(CancelEventHandler^ handler) { m_progressHook->Add(handler); }
		void remove(CancelEventHandler^ handler) { m_progressHook->Remove(handler); }
	}

	// StatementProfile
	//
	// Fired whenever SQLite sends out a profile message
	event SqliteProfileEventHandler^ StatementProfile
	{
		void add(SqliteProfileEventHandler^ handler) { m_profileHook->Add(handler); }
		void remove(SqliteProfileEventHandler^ handler) { m_profileHook->Remove(handler); }
	}

	// StatementTrace
	//
	// Fired whenever SQLite sends out a trace message
	event SqliteTraceEventHandler^ StatementTrace
	{
		void add(SqliteTraceEventHandler^ handler) { m_traceHook->Add(handler); }
		void remove(SqliteTraceEventHandler^ handler) { m_traceHook->Remove(handler); }
	}

	// TransactionCommitting
	//
	// Fired whenever a transaction (including implicit) is committing
	event CancelEventHandler^ TransactionCommitting
	{
		void add(CancelEventHandler^ handler) { m_commitHook->Add(handler); }
		void remove(CancelEventHandler^ handler) { m_commitHook->Remove(handler); }
	}

	// TransactionRolledBack
	//
	// Fired whenever a transaction (including implicit) has been rolled back
	event EventHandler^ TransactionRolledBack
	{
		void add(EventHandler^ handler) { m_rollbackHook->Add(handler); }
		void remove(EventHandler^ handler) { m_rollbackHook->Remove(handler); }
	}

	//-----------------------------------------------------------------------
	// Member Functions

	// Attach
	//
	// Attaches another SQLite database as a new catalog in this connection
	void Attach(String^ path, String^ databaseName);

	// BeginTransaction
	//
	// Starts a new database transaction, optionally allowing a non-default
	// locking mechanism to be specified in the process
	SqliteTransaction^ BeginTransaction(void) new { return BeginTransaction(SqliteLockMode::Deferred); }
	SqliteTransaction^ BeginTransaction(SqliteLockMode mode);

	// ChangeDatabase (DbConnection)
	//
	// Changes the default catalog.  Not implemented by this provider.
	virtual void ChangeDatabase(String^ databaseName) override { throw gcnew NotImplementedException(); }

	// CheckIntegrity
	//
	// Checks the integrity of all objects in the database file
	String^ CheckIntegrity(void);

	// Close (DbConnection)
	//
	// Closes the connection to the database file
	virtual void Close(void) override { return Close(true); }

	// CreateCommand
	//
	// Creates a new SqliteCommand object pre-initialized with this connection
	SqliteCommand^ CreateCommand(void) new;

	// Detach
	//
	// Detaches a previously attached database file by catalog name
	void Detach(String^ databaseName);

	// TODO: EnlistTransaction (DbConnection)
	//
	// Enlists in the specified transaction as a distributed transaction
	virtual void EnlistTransaction(Transaction^ transaction) override { (transaction); }

	// GetSchema (DbConnection)
	//
	// Gets schema information for the data source of this SqliteConnection
	virtual DataTable^ GetSchema(void) override { return GetSchema(DbMetaDataCollectionNames::MetaDataCollections, nullptr); }
	virtual DataTable^ GetSchema(String^ collectionName) override { return GetSchema(collectionName, nullptr); }
	virtual DataTable^ GetSchema(String^ collectionName, array<String^>^ restrictionValues) override;

	// Open (DbConnection)
	//
	// Opens the database connection using the currently set information
	virtual void Open(void) override;

	// RegisterVirtualTable
	//
	// Registers a Virtual Table class with this connection
	void RegisterVirtualTable(Type^ vtableType, String^ moduleName);

	// Vacuum
	//
	// Cleans the main database by copying it into a temp file and reloading it
	void Vacuum(void);

	//-----------------------------------------------------------------------
	// Properties

	// Aggregates
	//
	// Gets a reference to the contained aggregate collection
	property SqliteAggregateCollection^ Aggregates { SqliteAggregateCollection^ get(void); }

	// AutoVacuum
	//
	// Determines if this database was created with auto-vacuuming or not
	property bool AutoVacuum { bool get(void); }

	// BooleanFormat
	//
	// Defines the custom boolean data formatting and coercion setting.
	// Modifiable when the connection is open
	property SqliteBooleanFormat BooleanFormat
	{
		SqliteBooleanFormat get(void);
		void set(SqliteBooleanFormat value);
	}

	// CacheSize
	//
	// Defines the number of disk pages to hold in memory as a cache
	// Modifiable when the connection is open
	property int CacheSize
	{
		int get(void);
		void set(int value);
	}

	// CaseSensitiveLike
	//
	// Determines if the SQL LIKE comparison should be case sensitive or not
	// Modifiable when the connection is open
	property bool CaseSensitiveLike
	{
		bool get(void);
		void set(bool value);
	}

	// Collations
	//
	// Gets a reference to the contained collation collection
	property SqliteCollationCollection^ Collations { SqliteCollationCollection^ get(void); }

	// CompatibleFileFormat
	//
	// Determines if this database was created in backwards compatibility mode
	property bool CompatibleFileFormat { bool get(void); }

	// ConnectionString (DbConnection)
	//
	// Provides a current and valid connection string based on the configured
	// connection properties.  Cannot be explicitly set if connection is open
	virtual property String^ ConnectionString
	{
		String^ get(void) override;
		void set(String^ value) override;
	}

	// ConnectionTimeout (DbConnection)
	//
	// Not implemented by this provider
	virtual property int ConnectionTimeout
	{
		int get(void) override { throw gcnew NotImplementedException(); }
	}

	// Database (DbConnection)
	//
	// Provides the current catalog name, which happens to always be
	// MAIN_CATALOG_NAME ("main") for the SQLite implementation
	virtual property String^ Database { String^ get(void) override; }

	// DataSource (DbConnection)
	//
	// Provides the main database file name for an open connection
	virtual property String^ DataSource { String^ get(void) override; }

	// DateTimeFormat
	//
	// Defines the custom date/time data formatting and coercion setting.
	// Modifiable when the connection is open
	property SqliteDateTimeFormat DateTimeFormat
	{
		SqliteDateTimeFormat get(void);
		void set(SqliteDateTimeFormat value);
	}

	// Encoding
	//
	// Determines the text encoding mode that this database was created with
	property SqliteTextEncodingMode Encoding { SqliteTextEncodingMode get(void); }

	// FieldEncryptionPassword
	//
	// Changes the contained field encyption password
	property SecureString^ FieldEncryptionPassword { void set(SecureString^ value); }

	// Functions
	//
	// Gets a reference to the contained function collection
	property SqliteFunctionCollection^ Functions { SqliteFunctionCollection^ get(void); }

	// GuidFormat
	//
	// Defines the custom GUID data formatting and coercion setting.
	// Modifiable when the connection is open
	property SqliteGuidFormat GuidFormat
	{
		SqliteGuidFormat get(void);
		void set(SqliteGuidFormat value);
	}

	// PageSize
	//
	// Determines the disk file page size the database was created with
	property int PageSize { int get(void); }

	// ServerVersion (DbConnection)
	//
	// Provides the SQLite database engine version
	virtual property String^ ServerVersion { String^ get(void) override; }

	// State (DbConnection)
	//
	// Determines the current state of the connection.
	// Can be called even if the object has been disposed of.
	virtual property ConnectionState State { ConnectionState get(void) override; }

	// StatementProgressFrequency
	//
	// Determines the frequency at which the StatementProgress event will fire.
	// Lower numbers indicate a more frequent firing of the event
	property int StatementProgressFrequency
	{
		int get(void);
		void set(int value);
	}

	// SynchronousMode
	//
	// Determines the synchronous mode for the database
	// Modifiable when the connection is open
	property SqliteSynchronousMode SynchronousMode
	{
		SqliteSynchronousMode get(void);
		void set(SqliteSynchronousMode value);
	}

	// TemporaryStorageFolder
	//
	// Determines the location that temporary tables and indicies will be stored
	// Modifiable when the connection is open
	property String^ TemporaryStorageFolder
	{
		String^ get(void);
		void set(String^ value);
	}

	// TemporaryStorageMode
	//
	// Determines the mode to use when storing temporary tables and indicies
	// Modifiable when the connection is open
	property SqliteTemporaryStorageMode TemporaryStorageMode
	{
		SqliteTemporaryStorageMode get(void);
		void set(SqliteTemporaryStorageMode value);
	}

	// TransactionMode
	//
	// Determines the custom transaction mode the connection was opened with
	property SqliteTransactionMode TransactionMode { SqliteTransactionMode get(void); }

protected:

	//-----------------------------------------------------------------------
	// Protected Member Functions

	// BeginTransaction (DbConnection)
	//
	// Generic way the base class generates a new transaction object
	virtual DbTransaction^ BeginDbTransaction(Data::IsolationLevel isolationLevel) override;

	// CreateDbCommand (DbConnection)
	//
	// Generic way the base class generates a new command object
	virtual DbCommand^ CreateDbCommand(void) override; 

internal:

	//-----------------------------------------------------------------------
	// Internal Member Functions

	// CommitTransaction
	//
	// Commits an outstanding database transaction
	void CommitTransaction(SqliteTransaction^ trans);

	// FindConnection (static)
	//
	// Attempts to locate a SqliteConnection instance from a sqlite3* handle
	static SqliteConnection^ FindConnection(sqlite3* hDatabase);

	// GetHandle
	//
	// Retrieves and AddRefs the contained database handle.  The caller must
	// call Release() on the handle when they're done with it.  Use the .Handle
	// property to just access it when AddRef()/Release() is not required
	void GetHandle(DatabaseHandle **ppDatabaseHandle);

	// Interrupt
	//
	// Attempts to abort any pending database operations
	void Interrupt(void);

	// IsDisposed (ITrackableObject)
	//
	// Exposes the object's internal disposal state
	virtual bool IsDisposed(void) sealed = ITrackableObject::IsDisposed { return m_disposed; }

	// IsHandleValid
	//
	// Determines if the calls to Handle/HandlePointer will actually work or not
	bool IsHandleValid(void) { return (m_pDatabase != NULL); }

	// RegisterDataReader
	//
	// Registers a SqliteDataReader against this connection so it can be automatically
	// closed out when the connection is closed.
	__int64 RegisterDataReader(SqliteDataReader^ reader);

	// RollbackTransaction
	//
	// Rolls back an outstanding database transaction
	void RollbackTransaction(SqliteTransaction^ trans);

	// UnRegisterDataReader
	//
	// Removes a data reader registration from this connection
	void UnRegisterDataReader(__int64 cookie);

	//-----------------------------------------------------------------------
	// Internal Properties

	// FieldEncryptionKey
	//
	// Returns a reference to the field-level encryption key
	property HCRYPTKEY FieldEncryptionKey { HCRYPTKEY get(void); }

	// Handle
	//
	// Returns a reference to the contained DatabaseHandle instance without
	// incrementing it's reference count.  Use GetHandle() instead of this
	// if a reference count is required for the operation.
	property DatabaseHandle& Handle { DatabaseHandle& get(void); }

	// HandlePointer
	//
	// Returns a pointer to the contained DatabaseHandle instance without
	// incrementing it's reference count.  Use GetHandle() instead of this
	// if a reference count is required for the operation.
	property DatabaseHandle* HandlePointer { DatabaseHandle* get(void); }

	// InTransaction
	//
	// Determines if the ENGINE is currently in a transaction or not.  This is NOT
	// the same thing as the provider transactions, although if the provider is in
	// a transaction, this better darn well be TRUE as well
	property bool InTransaction { bool get(void); }

	// RollbackInProgress
	//
	// TRUE if the nested transaction manager is in a rollback state
	property bool RollbackInProgress { bool get(void); }

	//-----------------------------------------------------------------------
	// Internal Fields

	// ExecutePermission
	//
	// Code Access Security permission that will be demanded before the user
	// can execute any commands against a database from this provider
	static initonly CodeAccessPermission^ ExecutePermission = CreateExecutePermission();

private:

	// STATIC CONSTRUCTOR
	static SqliteConnection()
	{
		DatabaseExtensions::Register();
		s_handleMapper = gcnew Dictionary<intptr_t, SqliteConnection^>();
	}

	// DESTRUCTOR / FINALIZER
	~SqliteConnection();
	!SqliteConnection();

	//-----------------------------------------------------------------------
	// Private Constants

	// MAIN_CATALOG_NAME
	//
	// Constant name of the main database instance/catalog
	literal String^ MAIN_CATALOG_NAME = "main";

	//-----------------------------------------------------------------------
	// Private Member Functions

	// ApplyConnectionPragmas
	//
	// Applies all PRAGMA commands to the connection after it's been opened
	void ApplyConnectionPragmas(void);

	// Close
	//
	// Internalized version of Close() that can control the firing of the
	// StateChange event
	void Close(bool fireStateChange);

	// Construct
	//
	// Helper function to the class constructor
	void Construct(String^ connectionString);

	// CreateExecutePermission
	//
	// Creates and initializes the static ExecutePermission field
	static CodeAccessPermission^ CreateExecutePermission(void);

	// LoadConfiguredPragmas
	//
	// Re-reads all of the PRAGMA settings from the database
	void LoadConfiguredPragmas(void);

	//-----------------------------------------------------------------------
	// Member Variables
	
	bool						m_disposed;		// Object disposal flag
	DatabaseHandle*				m_pDatabase;	// Contained database handle
	SqliteConnectionStringBuilder^	m_cs;			// Contained connection options
	ConnectionState				m_state;		// Current connection state
	SqliteCryptoKey^				m_fieldKey;		// Field-Level encryption key

	// TRANSACTION CONTROL

	List<SqliteTransaction^>^		m_openTrans;		// Outstanding transactions
	SqliteLockMode					m_openTransMode;	// Current transaction mode
	int							m_openTransCount;	// Outstanding transaction count

	// DATAREADER CONTROL

	Dictionary<__int64, SqliteDataReader^>^	m_readers;		// Open SqliteDataReaders
	static __int64							s_cookie = 0;	// SqliteDataReader cookie

	// HOOK EVENTS

	SqliteConnectionAuthorizationHook^		m_authHook;			// Authorize hook
	SqliteConnectionCollationNeededHook^	m_collationHook;	// CollationNeeded hook
	SqliteConnectionCommitHook^			m_commitHook;		// TransactionCommitting hook
	SqliteConnectionProfileHook^			m_profileHook;		// StatementProfile hook
	SqliteConnectionProgressHook^			m_progressHook;		// Progress hook
	SqliteConnectionRollbackHook^			m_rollbackHook;		// TransactionRollback hook
	SqliteConnectionTraceHook^				m_traceHook;		// StatementTrace hook
	SqliteConnectionUpdateHook^			m_updateHook;		// RowChanged hook

	// VIRTUAL TABLE MODULES

	List<GCHandle>^					m_modules;			// Registered modules

	// FUNCTIONS, AGGREGATES, COLLATIONS

	SqliteAggregateCollection^			m_aggregates;		// Registered aggregates
	SqliteCollationCollection^			m_collations;		// Registered collations
	SqliteFunctionCollection^			m_functions;		// Registered functions

	// NON-MODIFIABLE CONNECTION PROPERTIES AND PRAGMAS

	bool					m_autoVacuum;			// PRAGMA AUTO_VACUUM
	bool					m_compatibleFormat;		// PRAGMA LEGACY_FILE_FORMAT
	SqliteTextEncodingMode		m_encoding;				// PRAGMA ENCODING
	int						m_pageSize;				// PRAGMA PAGE_SIZE
	SqliteTransactionMode		m_transactionMode;		// TRANSACTION MODE=

	// CONNECTION<->DBHANDLE MAPPER

	static Dictionary<intptr_t, SqliteConnection^>^	s_handleMapper;
};

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)

#endif		// __SQLITECONNECTION_H_
