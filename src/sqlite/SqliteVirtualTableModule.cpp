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
#include "SqliteVirtualTableModule.h"	// Include SqliteVirtualTableModule declarations
#include "SqliteConnection.h"			// Include SqliteConnection declarations

#pragma warning(push, 4)			// Enable maximum compiler warnings
#pragma warning(disable:4100)		// "unreferenced formal parameter"

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// Local Function Prototypes
//---------------------------------------------------------------------------

static int sqlite_vtab_begin		(sqlite3_vtab*);
static int sqlite_vtab_bestindex	(sqlite3_vtab*, sqlite3_index_info*);
static int sqlite_vtab_close		(sqlite3_vtab_cursor*);
static int sqlite_vtab_column		(sqlite3_vtab_cursor*, sqlite3_context*, int);
static int sqlite_vtab_commit		(sqlite3_vtab*);
static int sqlite_vtab_connect		(sqlite3*, void*, int, const char* const*, sqlite3_vtab**, char**);
static int sqlite_vtab_create		(sqlite3*, void*, int, const char* const*, sqlite3_vtab**, char**);
static int sqlite_vtab_ctor		(sqlite3*, void*, int, const char* const*, sqlite3_vtab**, char**, bool);
static int sqlite_vtab_destroy		(sqlite3_vtab*);
static int sqlite_vtab_disconnect	(sqlite3_vtab*);
static int sqlite_vtab_dtor		(sqlite3_vtab*, bool);
static int sqlite_vtab_eof			(sqlite3_vtab_cursor*);
static int sqlite_vtab_findfunc	(sqlite3_vtab*, int, const char*, void(**)(sqlite3_context*, int, sqlite3_value**), void**);
static int sqlite_vtab_filter		(sqlite3_vtab_cursor*, int, const char*, int, sqlite3_value**);
static void sqlite_vtab_func		(sqlite3_context*, int, sqlite3_value**);
static int sqlite_vtab_next		(sqlite3_vtab_cursor*);
static int sqlite_vtab_open		(sqlite3_vtab*, sqlite3_vtab_cursor**);
static int sqlite_vtab_rollback	(sqlite3_vtab*);
static int sqlite_vtab_rowid		(sqlite3_vtab_cursor*, sqlite_int64*);
static int sqlite_vtab_sync		(sqlite3_vtab*);
static int sqlite_vtab_update		(sqlite3_vtab*, int, sqlite3_value**, sqlite_int64*);

//---------------------------------------------------------------------------
// sqlite_vtab_module
//
// Global structure used for all read/write virtual table module registrations.  
// The entry points are always the same and we dynamically set things up based 
// on context.

static const sqlite3_module sqlite_vtab_module = {

	0,							// iVersion
	sqlite_vtab_create,			// xCreate
	sqlite_vtab_connect,			// xConnect
	sqlite_vtab_bestindex,			// xBestIndex
	sqlite_vtab_disconnect,		// xDisconnect
	sqlite_vtab_destroy,			// xDestroy
	sqlite_vtab_open,				// xOpen
	sqlite_vtab_close,				// xClose
	sqlite_vtab_filter,			// xFilter
	sqlite_vtab_next,				// xNext
	sqlite_vtab_eof,				// xEof
	sqlite_vtab_column,			// xColumn
	sqlite_vtab_rowid,				// xRowid
	sqlite_vtab_update,			// xUpdate
	sqlite_vtab_begin,				// xBegin
	sqlite_vtab_sync,				// xSync
	sqlite_vtab_commit,			// xCommit
	sqlite_vtab_rollback,			// xRollback
	sqlite_vtab_findfunc,			// xFindFunction
};

//---------------------------------------------------------------------------
// sqlite_vtab_module_notrans
//
// Global structure used for all non-transactional virtual table module 
// registrations.  The entry points are always the same and we dynamically 
// set things up based on context.

static const sqlite3_module sqlite_vtab_module_notrans = {

	0,							// iVersion
	sqlite_vtab_create,			// xCreate
	sqlite_vtab_connect,			// xConnect
	sqlite_vtab_bestindex,			// xBestIndex
	sqlite_vtab_disconnect,		// xDisconnect
	sqlite_vtab_destroy,			// xDestroy
	sqlite_vtab_open,				// xOpen
	sqlite_vtab_close,				// xClose
	sqlite_vtab_filter,			// xFilter
	sqlite_vtab_next,				// xNext
	sqlite_vtab_eof,				// xEof
	sqlite_vtab_column,			// xColumn
	sqlite_vtab_rowid,				// xRowid
	sqlite_vtab_update,			// xUpdate
	NULL,						// xBegin
	sqlite_vtab_sync,				// xSync
	NULL,						// xCommit
	NULL,						// xRollback
	sqlite_vtab_findfunc,			// xFindFunction
};

//---------------------------------------------------------------------------
// sqlite_vtab_module_readonly
//
// Global structure used for all read-only virtual table module registrations.  
// The entry points are always the same and we dynamically set things up based 
// on context.

static const sqlite3_module sqlite_vtab_module_readonly = {

	0,							// iVersion
	sqlite_vtab_create,			// xCreate
	sqlite_vtab_connect,			// xConnect
	sqlite_vtab_bestindex,			// xBestIndex
	sqlite_vtab_disconnect,		// xDisconnect
	sqlite_vtab_destroy,			// xDestroy
	sqlite_vtab_open,				// xOpen
	sqlite_vtab_close,				// xClose
	sqlite_vtab_filter,			// xFilter
	sqlite_vtab_next,				// xNext
	sqlite_vtab_eof,				// xEof
	sqlite_vtab_column,			// xColumn
	sqlite_vtab_rowid,				// xRowid
	NULL,						// xUpdate
	NULL,						// xBegin
	NULL,						// xSync
	NULL,						// xCommit
	NULL,						// xRollback
	sqlite_vtab_findfunc,			// xFindFunction
};

//---------------------------------------------------------------------------
// sqlite_vtab_begin
//
// Implements the xBegin callback for all Sqlite virtual tables
//
// Arguments:
//
//	pVirtualTable	- Pointer to the current virtual table instance

static int sqlite_vtab_begin(sqlite3_vtab* pVirtualTable)
{
	VirtualTable& virtualTable = VirtualTable::Cast(pVirtualTable);

	try { virtualTable->BeginTransaction(); }
	catch(Exception^ ex) { virtualTable.SetError(ex->Message); return SQLITE_ERROR; }

	return SQLITE_OK;
}

//---------------------------------------------------------------------------
// sqlite_vtab_bestindex
//
// Implements the xBestIndex callback for all Sqlite virtual table modules
//
// Arguments:
//
//	pVirtualTable		- Pointer to the virtual table
//	pIndexInfo			- Pointer to index information from the engine

int sqlite_vtab_bestindex(sqlite3_vtab* pVirtualTable, sqlite3_index_info* pIndexInfo)
{
	gcroot<SqliteIndexSelectionArgs^>		args;		// Managed argument data

	VirtualTable& virtualTable = VirtualTable::Cast(pVirtualTable);

	try {

		args = gcnew SqliteIndexSelectionArgs(pIndexInfo);	// Wrap the index info
		virtualTable->SelectBestIndex(args);			// Invoke vtable class
		args->OutputToStructure(pIndexInfo);			// Apply output data
	}

	catch(Exception^ ex) { virtualTable.SetError(ex->Message); return SQLITE_ERROR; }

	return SQLITE_OK;
}

//---------------------------------------------------------------------------
// sqlite_vtab_close
//
// Implements the xClose callback for all Sqlite virtual tables
//
// Arguments:
//
//	pCursor			- Pointer to the cursor to be closed and deallocated

static int sqlite_vtab_close(sqlite3_vtab_cursor* pCursor)
{
	VirtualTableCursor& cursor = VirtualTableCursor::Cast(pCursor);
		
	try {

		cursor->Close();							// Close the cursor
		delete static_cast<Object^>(cursor);		// Kill the cursor
		VirtualTableCursor::Destroy(cursor);		// Destroy instance
	}

	// Since the goal is to destroy the virtual table, I doubt it would be
	// a good idea to try and send back an error message here.

	catch(Exception^) { return SQLITE_ERROR; }

	return SQLITE_OK;				// Table successfully disconnected
}

//---------------------------------------------------------------------------
// sqlite_vtab_column
//
// Implements the xColumn callback for all Sqlite virtual table modules
//
// Arguments:
//
//	pCursor			- Pointer to the current cursor object
//	pResultContext	- Result context pointer
//	ordinal			- Ordinal of the column to retrieve

static int sqlite_vtab_column(sqlite3_vtab_cursor* pCursor, sqlite3_context* pResultContext, 
	int ordinal)
{
	gcroot<SqliteResult^>			result;			// Resultant value from cursor

	VirtualTableCursor& cursor = VirtualTableCursor::Cast(pCursor);

	try {

		result = gcnew SqliteResult(pResultContext);		// Construct argument
		
		try { cursor->GetValue(ordinal, result); }		// Invoke managed class
		finally { delete result; }						// Always dispose of this
	}

	catch(Exception^ ex) { cursor.SetError(ex->Message); return SQLITE_ERROR; }

	return SQLITE_OK;
}

//---------------------------------------------------------------------------
// sqlite_vtab_commit
//
// Implements the xCommit callback for all Sqlite virtual tables
//
// Arguments:
//
//	pVirtualTable	- Pointer to the current virtual table instance

static int sqlite_vtab_commit(sqlite3_vtab* pVirtualTable)
{
	VirtualTable& virtualTable = VirtualTable::Cast(pVirtualTable);

	try { virtualTable->CommitTransaction(); }
	catch(Exception^ ex) { virtualTable.SetError(ex->Message); return SQLITE_ERROR; }

	return SQLITE_OK;
}

//---------------------------------------------------------------------------
// sqlite_vtab_connect
//
// Implements the xConnect callback for all Sqlite virtual table modules
//
// Arguments:
//
//	hDatabase		- SQLite database handle
//	context			- Context pointer from call to sqlite3_create_module
//	argc			- Creation argument count
//	argv			- Creation arguments (see documentation)
//	ppVirtualTable	- On success, contains the new sqlite3_vtab structure
//	ppszError		- Provides engine with an error message on failure

static int sqlite_vtab_connect(sqlite3* hDatabase, void* context, int argc, const char* const* argv, 
	sqlite3_vtab** ppVirtualTable, char** ppszError)
{
	// Invoke sqlite_vtab_ctor, passing FALSE as the final argument to indicate
	// that there is no need to call into the virtual table's Create() entry

	return sqlite_vtab_ctor(hDatabase, context, argc, argv, ppVirtualTable, ppszError, false);
}

//---------------------------------------------------------------------------
// sqlite_vtab_create
//
// Implements the xCreate callback for all Sqlite virtual table modules
//
// Arguments:
//
//	hDatabase		- SQLite database handle
//	context			- Context pointer from call to sqlite3_create_module
//	argc			- Creation argument count
//	argv			- Creation arguments (see documentation)
//	ppVirtualTable	- On success, contains the new sqlite3_vtab structure
//	ppszError		- Provides engine with an error message on failure

static int sqlite_vtab_create(sqlite3* hDatabase, void* context, int argc, const char* const* argv, 
	sqlite3_vtab** ppVirtualTable, char** ppszError)
{
	// Invoke sqlite_vtab_ctor, passing TRUE as the final argument to indicate
	// that the virtual table's Create() entry point should be invoked

	return sqlite_vtab_ctor(hDatabase, context, argc, argv, ppVirtualTable, ppszError, true);
}

//---------------------------------------------------------------------------
// sqlite_vtab_ctor
//
// Implements the common functionality required by both sqlite_vtab_connect as
// well as sqlite_vtab_create.  The final argument indicates if Create() should
// be called or not.
//
// Arguments:
//
//	hDatabase		- SQLite database handle
//	context			- Context pointer from call to sqlite3_create_module
//	argc			- Creation argument count
//	argv			- Creation arguments (see documentation)
//	ppVirtualTable	- On success, contains the new sqlite3_vtab structure
//	ppszError		- Provides engine with an error message on failure
//	invokeCreate	- Flag to invoke the Create() entry point of the table object

static int sqlite_vtab_ctor(sqlite3* hDatabase, void* context, int argc, const char* const* argv, 
	sqlite3_vtab** ppVirtualTable, char** ppszError, bool invokeCreate)
{
	GCHandleRef<Type^>			type(context);		// Virtual table data type
	gcroot<Object^>				instance;			// Virtual table instance
	gcroot<String^>				schema;				// Virtual table schema information
	int							nResult;			// Result from function call

	Debug::Assert(context != NULL);					// Should never be NULL here

	*ppVirtualTable = NULL;					// Initialize [out] pointer to NULL
	*ppszError = NULL;						// Initialize [out] pointer to NULL

	try {

		// Push a new instance of SqliteVirtualTableConstructor args into the special
		// cache so the virtual table can find it, and activate the class instance.

		SqliteVirtualTableConstructorArgs::Push(SqliteConnection::FindConnection(hDatabase), argc, argv);
		instance = Activator::CreateInstance(type);

		// Construct the VirtualTable wrapper around the instance reference, and
		// start another nested try/catch to make sure it gets destroyed if anything
		// bad happens while we're completing the virtual table initializations

		VirtualTable& virtualTable = VirtualTable::Create(instance);

		try {

			// We need to provide SQLite with a CREATE TABLE statement that defines 
			// the schema of this table.  The table base class has all that code in it.

			schema = virtualTable->GetCreateTableStatement(SqliteUtil::FastPtrToStringAnsi(argv[2]));

			nResult = sqlite3_declare_vtab(hDatabase, AutoAnsiString(schema));
			if(nResult != SQLITE_OK) throw gcnew SqliteException(hDatabase, nResult);

			if(invokeCreate) virtualTable->Create();	// CREATE NEW TABLE
			virtualTable->Open();						// OPEN TABLE

			*ppVirtualTable = &virtualTable;			// Return pointer to SQLite
		}
		
		// When bad things happen in the nested try, we have to release the 
		// VirtualTable so it's contained GCHandle is deallocated properly

		catch(Exception^) { VirtualTable::Destroy(virtualTable); throw; }
	}
	
	// When general bad things happen, provide the error message to SQLite via
	// the ppszError argument, and return a standard SQLITE_ERROR code back

	catch(Exception^ ex) {
	
		*ppszError = sqlite3_mprintf(AutoAnsiString(ex->Message));
		return SQLITE_ERROR;	
	}

	return SQLITE_OK;						// Table created successfully
}

//---------------------------------------------------------------------------
// sqlite_vtab_destroy
//
// Implements the xDestroy callback for all Sqlite virtual tables
//
// Arguments:
//
//	pVirtualTable		- Pointer to the virtual table to be destroyed

static int sqlite_vtab_destroy(sqlite3_vtab* pVirtualTable)
{
	// Invoke sqlite_vtab_dtor, providing TRUE as the final argument to indicate
	// that the virtual table is being dropped from the database completely

	return sqlite_vtab_dtor(pVirtualTable, true);
}

//---------------------------------------------------------------------------
// sqlite_vtab_disconnect
//
// Implements the xDisconnect callback for all Sqlite virtual tables
//
// Arguments:
//
//	pVirtualTable		- Pointer to the virtual table to be disconnected

static int sqlite_vtab_disconnect(sqlite3_vtab* pVirtualTable)
{
	// Invoke sqlite_vtab_dtor, providing FALSE as the final argument to indicate
	// that the virtual table is being disconnected, not dropped

	return sqlite_vtab_dtor(pVirtualTable, false);
}

//---------------------------------------------------------------------------
// sqlite_vtab_dtor
//
// Provides the common implementation required by sqlite_vtab_disconnection and
// sqlite_vtab_destroy.   The final argument indicates if Drop() should be
// called or not.
//
// Arguments:
//
//	pVirtualTable		- Pointer to the virtual table to be disconnected
//	invokeDrop			- Flag to indicate if Drop() should be called

static int sqlite_vtab_dtor(sqlite3_vtab* pVirtualTable, bool invokeDrop)
{
	VirtualTable& virtualTable = VirtualTable::Cast(pVirtualTable);
		
	try {

		virtualTable->Close();							// CLOSE TABLE
		if(invokeDrop) virtualTable->Drop();			// DROP TABLE

		delete static_cast<Object^>(virtualTable);		// Invoke .Dispose
		VirtualTable::Destroy(virtualTable);			// Destroy instance
	}

	// Since the goal is to destroy the virtual table, I doubt it would be
	// a good idea to try and send back an error message here.

	catch(Exception^) { return SQLITE_ERROR; }

	return SQLITE_OK;				// Table successfully disconnected
}

//---------------------------------------------------------------------------
// sqlite_vtab_eof
//
// Implements the xEof callback for all Sqlite virtual tables
//
// Arguments:
//
//	pCursor			- Pointer to the current cursor object

static int sqlite_vtab_eof(sqlite3_vtab_cursor* pCursor)
{
	VirtualTableCursor& cursor = VirtualTableCursor::Cast(pCursor);
	return cursor.RowPresent ? 0 : 1;
}

//---------------------------------------------------------------------------
// sqlite_vtab_filter
//
// Implements the xFilter callback for all Sqlite virtual tables
//
// Arguments:
//
//	pCursor			- Pointer to the current cursor object
//	indexNum		- Implementation specific index number
//	indexString		- Implementation specific index string
//	argc			- Number of filter arguments
//	argv			- Array of filter argument values

static int sqlite_vtab_filter(sqlite3_vtab_cursor* pCursor, int indexNum, const char* indexString, 
	int argc, sqlite3_value** argv)
{
	gcroot<SqliteIndexIdentifier^>			index;		// Selected index information
	gcroot<SqliteArgumentCollection^>		args;		// Filter argument collection

	VirtualTableCursor& cursor = VirtualTableCursor::Cast(pCursor);

	try {

		index = gcnew SqliteIndexIdentifier(indexNum, indexString);	// Construct object
		args = gcnew SqliteArgumentCollection(argc, argv);				// Construct object
		
		// Invoke the SetFilter() method on the managed cursor object, and be
		// sure to dispose of the SqliteArgumentCollection under all circumstances

		try { cursor.RowPresent = cursor->SetFilter(index, args); }
		finally { delete args; }
	}

	catch(Exception^ ex) { cursor.SetError(ex->Message); return SQLITE_ERROR; }

	return SQLITE_OK;
}

//---------------------------------------------------------------------------
// sqlite_vtab_findfunc
//
// Implements the xFindFunction callback for all Sqlite virtual tables
//
// Arguments:
//
//	pVirtualTable	- Pointer to the current virtual table instance

static int sqlite_vtab_findfunc(sqlite3_vtab* pVirtualTable, int argc, const char* name,
	void(**pxfunc)(sqlite3_context*, int, sqlite3_value**), void** context)
{
	GCHandle				funcwrapper;		// SqliteFunctionWrapper instance

	*pxfunc = NULL;						// Initialize [out] pointer to null
	*context = NULL;					// Initialize [out] pointer to null

	VirtualTable& virtualTable = VirtualTable::Cast(pVirtualTable);

	try {

		// Ask the virtual table if it wants to overload the specified function or not.
		// It will return a GCHandle through funcwrapper that we pass as the context
		// for the scalar function (see sqlite_vtab_func below)

		if(!virtualTable->FindFunction(SqliteUtil::FastPtrToStringAnsi(name), argc, funcwrapper)) return 0;
		
		*pxfunc = sqlite_vtab_func;
		*context = GCHandle::ToIntPtr(funcwrapper).ToPointer();
	}

	catch(Exception^ ex) { virtualTable.SetError(ex->Message); return 0; }

	return 1;
}

//---------------------------------------------------------------------------
// sqlite_vtab_func
//
// Local version of a scalar invocation callback for overriden functions
// in virtual tables.  Not much difference between this and the real one
// in SqliteFunctionCollection, but it might end up that way at some point
//
// Arguments:
//
//	context		- Context/Result information provided by SQLite
//	argc		- Number of function arguments
//	argv		- Array of function argument values

void sqlite_vtab_func(sqlite3_context* context, int argc, sqlite3_value** argv)
{
	GCHandleRef<SqliteFunctionWrapper^> func(sqlite3_user_data(context));

	try { func->Invoke(context, argc, argv); }
	catch(Exception^ ex) { sqlite3_result_error(context, AutoAnsiString(ex->Message), -1); }
}

//---------------------------------------------------------------------------
// sqlite_vtab_next
//
// Implements the xNext callback for all Sqlite virtual tables
//
// Arguments:
//
//	pCursor			- Pointer to the current cursor object

static int sqlite_vtab_next(sqlite3_vtab_cursor* pCursor)
{
	VirtualTableCursor& cursor = VirtualTableCursor::Cast(pCursor);

	try { cursor.RowPresent = cursor->MoveNext(); }
	catch(Exception^ ex) { cursor.SetError(ex->Message); return SQLITE_ERROR; }

	return SQLITE_OK;
}

//---------------------------------------------------------------------------
// sqlite_vtab_open
//
// Implements the xOpen callback for all Sqlite virtual tables
//
// Arguments:
//
//	pVirtualTable		- Pointer to the current virtual table
//	ppCursor			- On success, contains the new cursor object

static int sqlite_vtab_open(sqlite3_vtab* pVirtualTable, sqlite3_vtab_cursor** ppCursor)
{
	gcroot<SqliteVirtualTableCursor^>		instance;	// Managed cursor instance

	VirtualTable& virtualTable = VirtualTable::Cast(pVirtualTable);

	try {

		instance = virtualTable->CreateCursor();		// Generate a new cursor

		// Construct the VirtualTableCursor wrapper around the instance reference
		// and return a pointer to that back to SQLite

		VirtualTableCursor& cursor = VirtualTableCursor::Create(instance);
		*ppCursor = &cursor;
	}

	catch(Exception^ ex) { virtualTable.SetError(ex->Message); return SQLITE_ERROR; }

	return SQLITE_OK;
}

//---------------------------------------------------------------------------
// sqlite_vtab_rollback
//
// Implements the xRollback callback for all Sqlite virtual tables
//
// Arguments:
//
//	pVirtualTable	- Pointer to the current virtual table instance

static int sqlite_vtab_rollback(sqlite3_vtab* pVirtualTable)
{
	VirtualTable& virtualTable = VirtualTable::Cast(pVirtualTable);

	try { virtualTable->RollbackTransaction(); }
	catch(Exception^ ex) { virtualTable.SetError(ex->Message); return SQLITE_ERROR; }

	return SQLITE_OK;
}

//---------------------------------------------------------------------------
// sqlite_vtab_rowid
//
// Implements the xRowid callback for all Sqlite virtual tables
//
// Arguments:
//
//	pCursor			- Pointer to the current cursor object
//	pRowid			- Pointer to receive the current ROWID value

static int sqlite_vtab_rowid(sqlite3_vtab_cursor* pCursor, sqlite_int64* pRowid)
{
	VirtualTableCursor& cursor = VirtualTableCursor::Cast(pCursor);

	try { *pRowid = cursor->GetRowID(); }
	catch(Exception^ ex) { cursor.SetError(ex->Message); return SQLITE_ERROR; }

	return SQLITE_OK;
}

//---------------------------------------------------------------------------
// sqlite_vtab_sync
//
// Implements the xSync callback for all Sqlite virtual tables
//
// Arguments:
//
//	pVirtualTable	- Pointer to the current virtual table instance

static int sqlite_vtab_sync(sqlite3_vtab* pVirtualTable)
{
	VirtualTable& virtualTable = VirtualTable::Cast(pVirtualTable);

	try { virtualTable->Synchronize(); }
	catch(Exception^ ex) { virtualTable.SetError(ex->Message); return SQLITE_ERROR; }

	return SQLITE_OK;
}

//---------------------------------------------------------------------------
// sqlite_vtab_update
//
// Implements the xUpdate callback for all Sqlite virtual tables
//
// Arguments:
//
//	pVirtualTable	- Pointer to the current virtual table
//	argc			- Number of update arguments
//	argv			- Array of update values
//	pRowid			- Contains/receives the ROWID value, depending on the operation

static int sqlite_vtab_update(sqlite3_vtab* pVirtualTable, int argc, sqlite3_value** argv, 
	sqlite_int64* pRowid)
{
	__int64^							delrowid;		// ROWID to be deleted (boxed)
	__int64^							insrowid;		// ROWID to be inserted (boxed)
	gcroot<SqliteArgumentCollection^>		values;			// Table value collection

	VirtualTable& virtualTable = VirtualTable::Cast(pVirtualTable);

	// argv[0] is the ROWID to be deleted (updates are considered to be a delete/insert
	// sort of thing ... see logic in SQLite documentation for virtual tables)

	if(sqlite3_value_type(argv[0]) != SQLITE_NULL) *delrowid = sqlite3_value_int64(argv[0]);

	// argv[1] is the ROWID to be inserted (updates are considered to be a delete/insert
	// sort of thing ... see logic in SQLite documentation for virtual tables)

	if((argc > 1) && (sqlite3_value_type(argv[1]) != SQLITE_NULL)) *insrowid = sqlite3_value_int64(argv[1]);

	try {

		// Depending on the arguments provided, we can break this up into
		// distinct DELETE, INSERT and UPDATE operations so the derived virtual
		// table class doesn't have to worry about all of this.

		// DELETE
		if(argc == 1) virtualTable->DeleteRow(*delrowid);

		// INSERT
		else if((argc > 1) && (delrowid == nullptr)) {

			if(insrowid == nullptr) insrowid = virtualTable->NewRowID();
			values = gcnew SqliteArgumentCollection(argc - 2, &argv[2]);

			try {
			
				virtualTable->InsertRow(*insrowid, values);		// Insert the row
				*pRowid = *insrowid;							// Return the ROWID
			}

			finally { delete static_cast<Object^>(values); }	// Always kill the args
		}

		// UPDATE
		else if((argc > 1) && (delrowid != nullptr)) {

			// There is a special case here that occurs when SQL along the lines of
			// "UPDATE x SET ROWID = ROWID + 1 WHERE .." has been executed.  According
			// to the documentation, we need to change the destination row's ROWID
			// as well as update it's values.  I've broken that out into two distinct
			// steps for ease of implementation in the derived virtual table class.

			if((insrowid != nullptr) && (*delrowid != *insrowid)) 
				virtualTable->UpdateRowID(*delrowid, *insrowid);

			if(insrowid == nullptr) insrowid = virtualTable->NewRowID();
			values = gcnew SqliteArgumentCollection(argc - 2, &argv[2]);
			
			try {

				virtualTable->UpdateRow(*delrowid, values);		// Update the row
				*pRowid = *insrowid;							// Return the ROWID
			}

			finally { delete static_cast<Object^>(values); }	// Always kill the args
		}

		// ERROR - INVALID OPERATION
		else throw gcnew InvalidOperationException();
	}

	catch(Exception^ ex) { virtualTable.SetError(ex->Message); return SQLITE_ERROR; }

	return SQLITE_OK;
}

//---------------------------------------------------------------------------
// SQLITEVIRTUALTABLEMODULE IMPLEMENTATION
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// SqliteVirtualTableModule::GetMethods (static)
//
// Returns a pointer to one of the locally defined sqlite3_module structs,
// based on the type of virtual table being provided
//
// Arguments:
//
//	vtableType		- Type of the virtual table being registered

const sqlite3_module* SqliteVirtualTableModule::GetMethods(Type^ vtableType)
{
	Type^				baseType;			// Base type of the vtable type
	
	// By the time this is called, it is expected that the table type is
	// valid, so we lose the try/catch and let any exceptions just fly.

	baseType = vtableType->BaseType->GetGenericTypeDefinition();

	if(baseType == SqliteVirtualTable::typeid) return &sqlite_vtab_module;
	else if(baseType == SqliteReadOnlyVirtualTable::typeid) return &sqlite_vtab_module_readonly;
	else if(baseType == SqliteNonTransactionalVirtualTable::typeid) return &sqlite_vtab_module_notrans;
	else throw gcnew ArgumentException();
}

//---------------------------------------------------------------------------
// SqliteVirtualTableModule::IsValidVirtualTableType (static)
//
// Determines if the specified type represents a valid virtual table
//
// Arguments:
//
//	vtableType		- Type to be tested for virtual table compliance

bool SqliteVirtualTableModule::IsValidVirtualTableType(Type^ vtableType)
{
	try { 

		// All virtual tables must derive from SqliteVirtualTable or SqliteReadOnlyVirtualTable ...

		Type^ type = vtableType->BaseType->GetGenericTypeDefinition();
		if((type != SqliteVirtualTable::typeid) && (type != SqliteReadOnlyVirtualTable::typeid) &&
			(type != SqliteNonTransactionalVirtualTable::typeid)) return false;
	}

	catch(Exception^) { return false; }

	return true;					// All object data type tests passed
}

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)
