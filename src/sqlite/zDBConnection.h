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

#ifndef __ZDBCONNECTION_H_
#define __ZDBCONNECTION_H_
#pragma once

#include "DatabaseExtensions.h"			// Include DatabaseExtensions decls
#include "DatabaseHandle.h"				// Include DatabaseHandle declarations
#include "zDBAggregateCollection.h"		// Include zDBAggregateCollection decls
#include "zDBCollationCollection.h"		// Include zDBCollationCollection decls
#include "zDBConnectionHooks.h"			// Include zDB connection hook decls
#include "zDBConnectionStringBuilder.h"	// Include zDBConnectionStringBuilder
#include "zDBCryptoKey.h"				// Include zDBCryptoKey declarations
#include "zDBDelegates.h"				// Include zDB delegate declarations
#include "zDBEnumerations.h"			// Include zDB enumeration declarations
#include "zDBExceptions.h"				// Include zDB exception declarations
#include "zDBFunctionCollection.h"		// Include zDBFunctionCollection decls
#include "zDBPermission.h"				// Include zDBPermission declarations
#include "zDBUtil.h"					// Include zDBUtil class declarations
#include "zDBVirtualTableModule.h"		// Include zDBVirtualTableModule decls

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

ref class zDBCommand;					// zDBCommand.h
ref class zDBMetaData;					// zDBMetaData.h
ref class zDBTransaction;				// zDBTransaction.h

//---------------------------------------------------------------------------
// Class zDBConnection
//
// Represents an open connection to a SQLite database.  Unlike the previous 
// version of this ADO.NET provider, all create-only options MUST be 
// specified as part of the connection string.  This was done to align more
// closely with the way the built-in ADO.NET providers operate.
//---------------------------------------------------------------------------

public ref class zDBConnection sealed : public DbConnection, public ITrackableObject
{
public:

	//-----------------------------------------------------------------------
	// Constructors

	zDBConnection() { Construct(nullptr); }
	zDBConnection(String^ connectionString) { Construct(connectionString); }

	//-----------------------------------------------------------------------
	// Events

	// Authorize
	//
	// Fired whenever an action that needs to be authorized occurs in the database
	event zDBAuthorizeEventHandler^ Authorize
	{
		void add(zDBAuthorizeEventHandler^ handler) { m_authHook->Add(handler); }
		void remove(zDBAuthorizeEventHandler^ handler) { m_authHook->Remove(handler); }
	}

	// CollationNeeded
	//
	// Fired whenever an unknown collation sequence has been encountered to give
	// the handler a change to register one
	event zDBCollationNeededEventHandler^ CollationNeeded
	{
		void add(zDBCollationNeededEventHandler^ handler) { m_collationHook->Add(handler); }
		void remove(zDBCollationNeededEventHandler^ handler) { m_collationHook->Remove(handler); }
	}

	// RowChanged
	//
	// Fired whenever a row has been changed in a database table
	event zDBRowChangedEventHandler^ RowChanged
	{
		void add(zDBRowChangedEventHandler^ handler) { m_updateHook->Add(handler); }
		void remove(zDBRowChangedEventHandler^ handler) { m_updateHook->Remove(handler); }
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
	event zDBProfileEventHandler^ StatementProfile
	{
		void add(zDBProfileEventHandler^ handler) { m_profileHook->Add(handler); }
		void remove(zDBProfileEventHandler^ handler) { m_profileHook->Remove(handler); }
	}

	// StatementTrace
	//
	// Fired whenever SQLite sends out a trace message
	event zDBTraceEventHandler^ StatementTrace
	{
		void add(zDBTraceEventHandler^ handler) { m_traceHook->Add(handler); }
		void remove(zDBTraceEventHandler^ handler) { m_traceHook->Remove(handler); }
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
	zDBTransaction^ BeginTransaction(void) new { return BeginTransaction(zDBLockMode::Deferred); }
	zDBTransaction^ BeginTransaction(zDBLockMode mode);

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
	// Creates a new zDBCommand object pre-initialized with this connection
	zDBCommand^ CreateCommand(void) new;

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
	// Gets schema information for the data source of this zDBConnection
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
	property zDBAggregateCollection^ Aggregates { zDBAggregateCollection^ get(void); }

	// AutoVacuum
	//
	// Determines if this database was created with auto-vacuuming or not
	property bool AutoVacuum { bool get(void); }

	// BooleanFormat
	//
	// Defines the custom boolean data formatting and coercion setting.
	// Modifiable when the connection is open
	property zDBBooleanFormat BooleanFormat
	{
		zDBBooleanFormat get(void);
		void set(zDBBooleanFormat value);
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
	property zDBCollationCollection^ Collations { zDBCollationCollection^ get(void); }

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
	property zDBDateTimeFormat DateTimeFormat
	{
		zDBDateTimeFormat get(void);
		void set(zDBDateTimeFormat value);
	}

	// Encoding
	//
	// Determines the text encoding mode that this database was created with
	property zDBTextEncodingMode Encoding { zDBTextEncodingMode get(void); }

	// FieldEncryptionPassword
	//
	// Changes the contained field encyption password
	property SecureString^ FieldEncryptionPassword { void set(SecureString^ value); }

	// Functions
	//
	// Gets a reference to the contained function collection
	property zDBFunctionCollection^ Functions { zDBFunctionCollection^ get(void); }

	// GuidFormat
	//
	// Defines the custom GUID data formatting and coercion setting.
	// Modifiable when the connection is open
	property zDBGuidFormat GuidFormat
	{
		zDBGuidFormat get(void);
		void set(zDBGuidFormat value);
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
	property zDBSynchronousMode SynchronousMode
	{
		zDBSynchronousMode get(void);
		void set(zDBSynchronousMode value);
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
	property zDBTemporaryStorageMode TemporaryStorageMode
	{
		zDBTemporaryStorageMode get(void);
		void set(zDBTemporaryStorageMode value);
	}

	// TransactionMode
	//
	// Determines the custom transaction mode the connection was opened with
	property zDBTransactionMode TransactionMode { zDBTransactionMode get(void); }

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
	void CommitTransaction(zDBTransaction^ trans);

	// FindConnection (static)
	//
	// Attempts to locate a zDBConnection instance from a sqlite3* handle
	static zDBConnection^ FindConnection(sqlite3* hDatabase);

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
	// Registers a zDBDataReader against this connection so it can be automatically
	// closed out when the connection is closed.
	__int64 RegisterDataReader(zDBDataReader^ reader);

	// RollbackTransaction
	//
	// Rolls back an outstanding database transaction
	void RollbackTransaction(zDBTransaction^ trans);

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
	static zDBConnection()
	{
		DatabaseExtensions::Register();
		s_handleMapper = gcnew Dictionary<intptr_t, zDBConnection^>();
	}

	// DESTRUCTOR / FINALIZER
	~zDBConnection();
	!zDBConnection();

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
	zDBConnectionStringBuilder^	m_cs;			// Contained connection options
	ConnectionState				m_state;		// Current connection state
	zDBCryptoKey^				m_fieldKey;		// Field-Level encryption key

	// TRANSACTION CONTROL

	List<zDBTransaction^>^		m_openTrans;		// Outstanding transactions
	zDBLockMode					m_openTransMode;	// Current transaction mode
	int							m_openTransCount;	// Outstanding transaction count

	// DATAREADER CONTROL

	Dictionary<__int64, zDBDataReader^>^	m_readers;		// Open zDBDataReaders
	static __int64							s_cookie = 0;	// zDBDataReader cookie

	// HOOK EVENTS

	zDBConnectionAuthorizationHook^		m_authHook;			// Authorize hook
	zDBConnectionCollationNeededHook^	m_collationHook;	// CollationNeeded hook
	zDBConnectionCommitHook^			m_commitHook;		// TransactionCommitting hook
	zDBConnectionProfileHook^			m_profileHook;		// StatementProfile hook
	zDBConnectionProgressHook^			m_progressHook;		// Progress hook
	zDBConnectionRollbackHook^			m_rollbackHook;		// TransactionRollback hook
	zDBConnectionTraceHook^				m_traceHook;		// StatementTrace hook
	zDBConnectionUpdateHook^			m_updateHook;		// RowChanged hook

	// VIRTUAL TABLE MODULES

	List<GCHandle>^					m_modules;			// Registered modules

	// FUNCTIONS, AGGREGATES, COLLATIONS

	zDBAggregateCollection^			m_aggregates;		// Registered aggregates
	zDBCollationCollection^			m_collations;		// Registered collations
	zDBFunctionCollection^			m_functions;		// Registered functions

	// NON-MODIFIABLE CONNECTION PROPERTIES AND PRAGMAS

	bool					m_autoVacuum;			// PRAGMA AUTO_VACUUM
	bool					m_compatibleFormat;		// PRAGMA LEGACY_FILE_FORMAT
	zDBTextEncodingMode		m_encoding;				// PRAGMA ENCODING
	int						m_pageSize;				// PRAGMA PAGE_SIZE
	zDBTransactionMode		m_transactionMode;		// TRANSACTION MODE=

	// CONNECTION<->DBHANDLE MAPPER

	static Dictionary<intptr_t, zDBConnection^>^	s_handleMapper;
};

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)

#endif		// __ZDBCONNECTION_H_
