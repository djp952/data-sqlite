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

#ifndef __SQLITEEXCEPTIONS_H_
#define __SQLITEEXCEPTIONS_H_
#pragma once

#include "SqliteEnumerations.h"			// Include Sqlite enumeration decls
#include "SqliteException.h"				// Include SqliteException declarations
#include "SqliteType.h"					// Include SqliteType declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

using namespace System;
using namespace System::Data::Common;

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// Forward Class Declarations
//---------------------------------------------------------------------------

ref class SqliteParameter;					// SqliteParameter.h

//---------------------------------------------------------------------------
// Class SqliteExceptions (internal)
//
// Provides a top-level class around all of the provider specific exceptions.
// Note that many derive from standard exception classes, but some also 
// derive from the public SqliteException.
//
// STANDARD EXCEPTIONS: Things that are caused by an application error, like
// an invalid argument or an invalid operation
//
// PROVIDER EXCEPTIONS: Things that are caused by code in this provider that
// exists to interoperate between .NET and SQLite.
//---------------------------------------------------------------------------

ref struct SqliteExceptions sealed
{
	// BindingTransferException (SQLITEEXCEPTION)
	//
	// Thrown when parameters cannot be rebound to a recompiled statement
	ref struct BindingTransferException sealed : public SqliteException
	{
		BindingTransferException(String^ sql, sqlite3* hDatabase, int nResult);
		static String^ GenerateContext(String^ sql);
	};

	// CommandTypeUnknownException
	//
	// Thrown when an invalid SqliteCommandType code is encountered
	ref struct CommandTypeUnknownException sealed : public ArgumentException
	{
		CommandTypeUnknownException(SqliteCommandType value);
		static String^ GenerateMessage(SqliteCommandType value);
	};

	// DbTypeUnknownException
	//
	// Thrown when an invalid SqliteType code is encountered
	ref struct DbTypeUnknownException sealed : public ArgumentException
	{
		DbTypeUnknownException(SqliteType value);
		static String^ GenerateMessage(SqliteType value);
	};

	// InconsistentQuoteIdentifiersException
	//
	// Thrown when the command builder quote prefix and suffixes are inconsistent
	ref struct InconsistentQuoteIdentifiersException : public InvalidOperationException
	{
		InconsistentQuoteIdentifiersException(String^ prefix, String^ suffix);
		static String^ GenerateMessage(String^ prefix, String^ suffix);
	};

	// InvalidAggregateException
	//
	// Thrown when an invalid type is passed into SqliteAggregateCollection:Add
	ref struct InvalidAggregateException sealed : public ArgumentException
	{
		InvalidAggregateException(Type^ aggregateType);
		static String^ GenerateMessage(Type^ aggregateType);
	};

	// InvalidColumnNameException
	//
	// Thrown when an invalid column name is specified
	ref struct InvalidColumnNameException sealed : public ArgumentException
	{
		InvalidColumnNameException(String^ name);
		static String^ GenerateMessage(String^ name);
	};

	// InvalidVirtualTableException
	//
	// Thrown when an invalid type is passed into SqliteConnection::RegisterVirtualTable
	ref struct InvalidVirtualTableException sealed : public ArgumentException
	{
		InvalidVirtualTableException(Type^ tableType);
		static String^ GenerateMessage(Type^ tableType);
	};

	// NoDataPresentException
	//
	// Thrown when attempting to read a value from a DataReader or the internal
	// query statement class, but there is no result set yet
	ref struct NoDataPresentException sealed : public InvalidOperationException
	{
		NoDataPresentException();
	};

	// OpenReaderException
	//
	// Thrown when an attempt is made to modify something about a command
	// object while there is an outstanding data reader open
	ref struct OpenReaderException sealed : public InvalidOperationException
	{
		OpenReaderException();
	};

	// ParameterBindingException (SQLITEEXCEPTION)
	//
	// Thrown when a parameter cannot be bound to a compiled statement for some
	// reason other than a direct SQLite error
	ref struct ParameterBindingException sealed : public SqliteException
	{
		ParameterBindingException(SqliteParameter^ param, int index, String^ reason);
		ParameterBindingException(SqliteParameter^ param, int index, sqlite3* hDatabase, int nResult);

		static String^ GenerateMessage(SqliteParameter^ param, int index, String^ reason);
		static String^ GenerateContext(SqliteParameter^ param, int index);
	};

	// StatementRecompileException (SQLITEEXCEPTION)
	//
	// Thrown when a statement fails to recompile after a schema change
	ref struct StatementRecompileException sealed : public SqliteException
	{
		StatementRecompileException(String^ sql, sqlite3* hDatabase, int nResult);
		static String^ GenerateContext(String^ sql);
	};

	// StatementStepException (SQLITEEXCEPTION)
	//
	// Thrown when a statement step fails to execute properly
	ref struct StatementStepException sealed : public SqliteException
	{
		StatementStepException(String^ sql, sqlite3* hDatabase, int nResult);
		static String^ GenerateContext(String^ sql);
	};

	// UpdateRowSourceUnknownException
	//
	// Thrown when an invalid SqliteUpdateRowSource code is encountered
	ref struct UpdateRowSourceUnknownException sealed : public ArgumentException
	{
		UpdateRowSourceUnknownException(SqliteUpdateRowSource value);
		static String^ GenerateMessage(SqliteUpdateRowSource value);
	};
};


//---------------------------------------------------------------------------
// CONNECTION EXCEPTIONS
//---------------------------------------------------------------------------

// ConnectionBusyException
//
// Thrown when the application attempts to access a property or method of
// a SqliteConnection that requires the connection to be open and not currently
// executing a SQL command or fetching a result set
public ref class ConnectionBusyException sealed : public InvalidOperationException
{
internal:

	ConnectionBusyException() : InvalidOperationException("This operation cannot be "
		"performed against a database connection that is currently executing a command.") {}
};

// ConnectionClosedException
//
// Thrown when the application attempts to access a property or method of
// a SqliteConnection that requires the connection to be open.  Note that this
// is NOT the same thing as a Disposed connection
public ref class ConnectionClosedException sealed : public InvalidOperationException
{
internal:

	ConnectionClosedException() : InvalidOperationException("This operation cannot be "
		"performed against a closed database connection.") {}
};

// ConnectionDisposedException
//
// Thrown when a method is called that requires a valid connection and it's disposed of
public ref class ConnectionDisposedException sealed : public ObjectDisposedException
{
internal:

	ConnectionDisposedException() : ObjectDisposedException("The database connection "
		"associated with this operation has been disposed of and cannot be accessed.") {}
};

// ConnectionNullException
//
// Thrown when a method is called that requires a valid connection and it's NULL
public ref class ConnectionNullException sealed : public ArgumentNullException
{
internal:

	ConnectionNullException() : ArgumentNullException("No database connection has been "
		"associated with this operation.") {}
};

// ConnectionOpenException
//
// Thrown when the application attempts to access a property or method of
// a SqliteConnection that requires the connection to be closed.
public ref class ConnectionOpenException sealed : public InvalidOperationException
{
internal:

	ConnectionOpenException() : InvalidOperationException("This operation cannot be "
		"performed against an open database connection.") {}
};

// ConnectionRollbackException
//
// Thrown when a method is called that requires a valid connection and it's in the process
// of rolling back a nested transaction
public ref class ConnectionRollbackException sealed : public InvalidOperationException
{
internal:

	ConnectionRollbackException() : InvalidOperationException("The database connection "
		"associated with this operation is in the process of rolling back a nested transaction.") {}
};

//---------------------------------------------------------------------------
// PARAMETER EXCEPTIONS
//---------------------------------------------------------------------------

// ParameterCollectionLockedException
//
// Thrown when an attempt is made to modify the contents of a locked
// SqliteParameterCollection.  They lock when their parent command is active
public ref class ParameterCollectionLockedException : public InvalidOperationException
{
internal:

	ParameterCollectionLockedException() : InvalidOperationException("The contents "
		"of the parameter collection cannot be modified while the command is executing") {}
};

// ParameterExistsException
//
// Thrown when an attempt is made to add a named parameter that already
// exists as part of the collection
public ref class ParameterExistsException sealed : public ArgumentException
{
internal:

	ParameterExistsException(String^ name) : ArgumentException(GenerateMessage(name)) {}

private:

	static String^ GenerateMessage(String^ name)
	{
		if((name == nullptr) || (name->Length == 0)) return "The parameter already exists in this collection";
		else return String::Format("The parameter [{0}] already exists in this collection", name);
	}
};

// ParameterFormatException
//
// Thrown when a malformed parameter name is used
public ref class ParameterFormatException sealed : public ArgumentException
{
internal:

	ParameterFormatException(String^ name) : ArgumentException(String::Format("The parameter name [{0}] "
		"is not valid.  Parameter names must start with a colon and be at least 2 characters in length", name)) {}

};

// ParameterLockedException
//
// Thrown when an attempt is made to modify a locked parameter value
public ref class ParameterLockedException sealed : public InvalidOperationException
{
internal:

	ParameterLockedException() : InvalidOperationException("The state of a "
		"parameter cannot be modified while the command is executing") {}
};

// ParameterNameMismatchException
//
// Thrown when a named parameter is passed into the SqliteParameterCollection's string indexer,
// but the string index does not match the parameter name
public ref class ParameterNameMismatchException sealed : public ArgumentException
{
internal:

	ParameterNameMismatchException(String^ name, DbParameter^ param) : ArgumentException
		(String::Format("The collection key name [{0}] does not match the parameter name [{1}]",
			name, param->ParameterName)) {}
};

// ParameterParentException
//
// Thrown when an attempt is made to add a parameter to a collection that is already
// a member of another collection
public ref class ParameterParentException sealed : public ArgumentException
{
internal:

	ParameterParentException(String^ name) : ArgumentException(GenerateMessage(name)) {}

private:

	static String^ GenerateMessage(String^ name)
	{
		if((name == nullptr) || (name->Length == 0)) return "The parameter is already a member of another parameter collection";
		else return String::Format("The parameter [{0}] is already a member of another parameter collection", name);
	}
};

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)

#endif	// __SQLITEEXCEPTIONS_H_
