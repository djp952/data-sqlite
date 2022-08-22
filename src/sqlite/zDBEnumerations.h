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

#ifndef __ZDBENUMERATIONS_H_
#define __ZDBENUMERATIONS_H_
#pragma once

#pragma warning(push, 4)				// Enable maximum compiler warnings

using namespace System;
using namespace System::Data;

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// Enum zDBAuthorizeAction
//
// Defines all the possible authorization request codes passed into a 
// connection's Authorize event.  1:1 correlation with the SQLite codes
//---------------------------------------------------------------------------

public enum struct zDBAuthorizeAction
{
	AlterTable				= SQLITE_ALTER_TABLE,
	Analyze					= SQLITE_ANALYZE,
	Attach					= SQLITE_ATTACH,
	CreateIndex				= SQLITE_CREATE_INDEX,
	CreateTable				= SQLITE_CREATE_TABLE,
	CreateTempIndex			= SQLITE_CREATE_TEMP_INDEX,
	CreateTempTable			= SQLITE_CREATE_TEMP_TABLE,
	CreateTempTrigger		= SQLITE_CREATE_TEMP_TRIGGER,
	CreateTempView			= SQLITE_CREATE_TEMP_VIEW,
	CreateTrigger			= SQLITE_CREATE_TRIGGER,
	CreateView				= SQLITE_CREATE_VIEW,
	CreateVirtualTable		= SQLITE_CREATE_VTABLE,
	Delete					= SQLITE_DELETE,
	Detach					= SQLITE_DETACH,
	DropIndex				= SQLITE_DROP_INDEX,
	DropTable				= SQLITE_DROP_TABLE,
	DropTempIndex			= SQLITE_DROP_TEMP_INDEX,
	DropTempTable			= SQLITE_DROP_TEMP_TABLE,
	DropTempTrigger			= SQLITE_DROP_TEMP_TRIGGER,
	DropTempView			= SQLITE_DROP_TEMP_VIEW,
	DropTrigger				= SQLITE_DROP_TRIGGER,
	DropView				= SQLITE_DROP_VIEW,
	DropVirtualTable		= SQLITE_DROP_VTABLE,
	Function				= SQLITE_FUNCTION,		// NEW: SQLite 3.3.8
	Insert					= SQLITE_INSERT,
	Pragma					= SQLITE_PRAGMA,
	Read					= SQLITE_READ,
	ReIndex					= SQLITE_REINDEX,
	Select					= SQLITE_SELECT,
	Transaction				= SQLITE_TRANSACTION,
	Update					= SQLITE_UPDATE,
};

//---------------------------------------------------------------------------
// Enum zDBAuthorizeResponse
//
// Defines all possible response codes that can be returned from a connection's
// Authorize event handler.  1:1 correlation with the SQLite codes
//---------------------------------------------------------------------------

public enum struct zDBAuthorizeResponse
{
	Allow				= SQLITE_OK,		// Statement is allowed to execute
	Deny				= SQLITE_DENY,		// Statement denied and error issued
	Ignore				= SQLITE_IGNORE,	// The statement is completely ignored
};

//---------------------------------------------------------------------------
// Enum zDBBooleanFormat
//
// BooleanFormat specifies how System.Boolean values are written to the database
// when used as command parameters.  To alleviate the potential problems with
// reading the different styles available, a custom user function called
// "BOOLEAN()" will be automatically added to every database.  This function
// will automatically cast the result set column into a common format.
//---------------------------------------------------------------------------

public enum struct zDBBooleanFormat
{
	OneZero				= 0,		// 1 or 0 (default)
	NegativeOneZero		= 1,		// -1 or 0
	TrueFalse			= 2,		// "true" or "false"
};

//---------------------------------------------------------------------------
// Enum zDBCollationEncoding
//
// Defines an encoding mode that is specified when registering a binary
// collation with the zDBConnection.Collations collection
//---------------------------------------------------------------------------

public enum struct zDBCollationEncoding
{
	UTF8				= SQLITE_UTF8,			// Use UTF8 encoding
	UTF16				= SQLITE_UTF16,			// Use native UTF16 encoding
	UTF16BigEndian		= SQLITE_UTF16BE,		// Use big-endian (Motorola) UTF16
	UTF16LittleEndian	= SQLITE_UTF16LE,		// Use litte-endian (Intel) UTF16
};

//---------------------------------------------------------------------------
// Enum zDBCommandBehavior
//
// Modifies the standard CommandBehavior to remove the things we don't
// support, as well as add some additional new things that we can do when
// executing a zDBCommand
//---------------------------------------------------------------------------

[Flags]
public enum struct zDBCommandBehavior
{
	Default				= CommandBehavior::Default,				// 0x0000
	SingleResult		= CommandBehavior::SingleResult,		// 0x0001
	SchemaOnly			= CommandBehavior::SchemaOnly,			// 0x0002
	SingleRow			= CommandBehavior::SingleRow,			// 0x0008
	CloseConnection		= CommandBehavior::CloseConnection,		// 0x0020
};

//---------------------------------------------------------------------------
// Enum zDBCommandType
//
// Modifies the standard CommandType to remove the things we don't support
// as well as add any additional new things that zDBCommand can do
//---------------------------------------------------------------------------

public enum struct zDBCommandType
{
	Text			= CommandType::Text,				// 0x0001
	TableDirect		= CommandType::TableDirect,			// 0x0200
};

//---------------------------------------------------------------------------
// Enum zDBDateTimeFormat
//
// DateTimeFormat specifies how System.DateTime values are written to the
// database when used as command parameters.  To alleviate potential problems
// when reading the different styles available, a custom user function
// called "DATETIME()" is automatically added to every database.  This function
// will automatically cast the result set column into the current format
//---------------------------------------------------------------------------

public enum struct zDBDateTimeFormat
{
	ISO8601				= 0,		// yyyy-mm-dd HH:mm:ss (default)
	Sortable			= 1,		// S / SortableDateTimePattern
	UniversalSortable	= 2,		// U / UniversalSortableDateTimePattern
	RFC1123				= 3,		// R / RFC1123Pattern
	Ticks				= 4,		// 100ns ticks since 01/01/0001 00:00:00
};

//---------------------------------------------------------------------------
// Enum zDBGuidFormat
//
// GuidFormat specifies how System.Guid values are written to the database
// when used as command parameters.  To alleviate the potential problems with
// reading the different styles available, a custom user function called
// "GUID()" will be automatically added to every database. This function will
// automatically cast the result set column into a common format.
//---------------------------------------------------------------------------

public enum struct zDBGuidFormat
{
	Binary				= 0,		// 16 byte binary BLOB (default)
	HexString			= 1,		// N / "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
	Hyphenated			= 2,		// D / "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"
	Bracketed			= 3,		// B / {xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}
	Parenthetic			= 4,		// P / (xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx)
};

//---------------------------------------------------------------------------
// Enum zDBLockMode
//
// Defines the locking modes available when beginning a transaction
//---------------------------------------------------------------------------

public enum struct zDBLockMode
{
	Deferred			= 0,		// DEFERRED (Default)
	Immediate			= 1,		// IMMEDIATE
	Exclusive			= 2,		// EXCLUSIVE
};

//---------------------------------------------------------------------------
// Enum zDBRowChangeType
//
// Defines the type of row change that incurred a RowChanged() event from
// the zDBConnection object
//---------------------------------------------------------------------------

public enum struct zDBRowChangeType
{
	Deleted				= SQLITE_DELETE,
	Inserted			= SQLITE_INSERT,
	Updated				= SQLITE_UPDATE,
};

//---------------------------------------------------------------------------
// Enum zDBSearchOperator
//
// Defines the various constraint operators used when dealing with the
// zDBSearchColumn class and virtual tables
//---------------------------------------------------------------------------

public enum struct zDBSearchOperator
{
	Equal				= SQLITE_INDEX_CONSTRAINT_EQ,		// 0x02
	GreaterThan			= SQLITE_INDEX_CONSTRAINT_GT,		// 0x04
	LessThanOrEqual		= SQLITE_INDEX_CONSTRAINT_LE,		// 0x08
	LessThan			= SQLITE_INDEX_CONSTRAINT_LT,		// 0x10
	GreaterThanOrEqual	= SQLITE_INDEX_CONSTRAINT_GE,		// 0x20
	Match				= SQLITE_INDEX_CONSTRAINT_MATCH,	// 0x40
};

//---------------------------------------------------------------------------
// Enum zDBSortDirection
//
// Used to define the ORDER BY direction of an index when dealing with the
// virtual table implementation
//---------------------------------------------------------------------------

public enum struct zDBSortDirection
{
	Ascending			= 0,		// ORDER BY [n] ASC
	Descending			= 1,		// ORDER BY [n] DESC
};

//---------------------------------------------------------------------------
// Enum zDBStatementStatus (internal)
//
// Defines the various result codes from zDBStatement::Step, as well as
// the zDBStatement::Status property
//---------------------------------------------------------------------------

enum struct zDBStatementStatus {

	Completed		= SQLITE_DONE,			// Query has completed
	DatabaseBusy	= SQLITE_BUSY,			// Database is busy ... can retry
	Error			= SQLITE_ERROR,			// Unrecoverable error
	HandleMisuse	= SQLITE_MISUSE,		// Invalid step / threading issue
	Prepared		= -1,					// Statement is ready to go
	ResultReady		= SQLITE_ROW,			// A result set row is ready
};

//---------------------------------------------------------------------------
// Enum zDBSynchronousMode
//
// Defines the current database's SQLite 'synchronous' flag
//---------------------------------------------------------------------------

public enum struct zDBSynchronousMode
{
	Normal				= 1,		// Balance (default for this provider)
	Full				= 2,		// Extremely safe, extremely slow
	Off					= 0,		// Extremely fast, much less safety
};

//---------------------------------------------------------------------------
// Enum zDBTemporaryStorageMode
//
// Defines the current database's SQLite 'temp_store' flag
//---------------------------------------------------------------------------

public enum struct zDBTemporaryStorageMode
{
	Default				= 0,		// Location determined by the code itself (default)
	File				= 1,		// Temp tables and indexes --> file
	Memory				= 2,		// Temp tables and indexes --> memory
};

//---------------------------------------------------------------------------
// Enum zDBTextEncodingMode
//
// Defines the internal text encoding mode for the database.  This property
// controls the call to sqlite3_open() inside of eDBConnection ONLY.
//---------------------------------------------------------------------------

public enum struct zDBTextEncodingMode
{
	UTF8				= 1,		// Use UTF8 encoding
	UTF16				= 0,		// Use native UTF16 encoding (default)
	UTF16LittleEndian	= 2,		// Use little-endian (Intel) UTF16 encoding
	UTF16BigEndian		= 3,		// Use big-endian (Motorola) UTF16 encoding
};

//---------------------------------------------------------------------------
// Enum zDBTransactionStyle
//
// Defines the current connection's transaction style.  By default, SQLite
// can only handle a single transaction, which leads to problems in modular
// code, so an additional mode(s) have been defined at the provider level
//---------------------------------------------------------------------------

public enum struct zDBTransactionMode
{
	Single				= 0,		// Default SQLite transaction mode (default)
	SimulateNested		= 1,		// Simulate nested transaction support
};

//---------------------------------------------------------------------------
// Enum zDBTypeCode (internal)
//
// Defines the codes that are used with zDBType
//---------------------------------------------------------------------------

enum struct zDBTypeCode
{
	Auto			= -1,			// Automatic value (zDBParameter)
	Binary			= 0,			// Binary / BLOB value
	Boolean			= 1,			// Boolean value
	DateTime		= 2,			// Date/Time value
	Float			= 3,			// Floating point value
	Guid			= 4,			// GUID value
	Integer			= 5,			// 32 bit integer value
	Long			= 6,			// 64 bit integer value
	Null			= 7,			// NULL value
	String			= 8,			// String value
};

//---------------------------------------------------------------------------
// Enum zDBUpdateRowSource
//
// Modifies the standard UpdateRowSource to remove the things we don't support
// as well as add any additional new things that zDBCommand can do
//---------------------------------------------------------------------------

public enum struct zDBUpdateRowSource
{
	None					= UpdateRowSource::None,
	FirstReturnedRecord		= UpdateRowSource::FirstReturnedRecord,
};

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)

#endif		// __ZDBENUMERATIONS_H_
