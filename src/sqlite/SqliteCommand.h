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

#ifndef __SQLITECOMMAND_H_
#define __SQLITECOMMAND_H_
#pragma once

#include "ITrackableObject.h"			// Include ITrackableObject decls
#include "ObjectTracker.h"				// Include ObjectTracker decls
#include "SqliteParameter.h"				// Include SqliteParameter declarations
#include "SqliteParameterCollection.h"		// Include SqliteParameterCollection decls
#include "SqliteQuery.h"					// Include SqliteQuery declarations
#include "SqliteStatement.h"				// Include SqliteStatement decls

#pragma warning(push, 4)				// Enable maximum compiler warnings

using namespace System;
using namespace System::Data;
using namespace System::Data::Common;

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// Forward Class Declarations
//---------------------------------------------------------------------------

ref class SqliteConnection;				// SqliteConnection.h
ref class SqliteDataAdapter;				// SqliteDataAdapter.h
ref class SqliteDataReader;				// SqliteDataReader.h
ref class SqliteTransaction;				// SqliteTransaction.h

//---------------------------------------------------------------------------
// Class SqliteCommand
//
// Represents a SQL statement to be executed against the database.  Note that
// this provider does not support Stored Procedures and all commands must
// be based on a text SQL statement.
//---------------------------------------------------------------------------

public ref class SqliteCommand sealed : public DbCommand
{
public:

	//-----------------------------------------------------------------------
	// Constructors

	SqliteCommand() 
		{ Construct(String::Empty, nullptr, SqliteCommandType::Text); }

	SqliteCommand(String^ commandText)
		{ Construct(commandText, nullptr, SqliteCommandType::Text); }

	SqliteCommand(String^ commandText, SqliteCommandType commandType)
		{ Construct(commandText, nullptr, commandType); }

	SqliteCommand(String^ commandText, SqliteConnection^ connection)
		{ Construct(commandText, connection, SqliteCommandType::Text); }

	SqliteCommand(String^ commandText, SqliteConnection^ connection, SqliteCommandType commandType)
		{ Construct(commandText, connection, commandType); }

	//-----------------------------------------------------------------------
	// Methods

	// Cancel (DbCommand)
	//
	// Attempts to cancel the currently executing command
	virtual void Cancel(void) override;

	// ExecuteNonQuery (DbCommand)
	// 
	// Executes the currently set SQL command text as a non-query
	virtual int ExecuteNonQuery(void) override;

	// ExecuteReader
	//
	// Executes the SQL command and returns a SqliteDataReader to process the results
	SqliteDataReader^ ExecuteReader(void) new { return ExecuteReader(SqliteCommandBehavior::Default); }
	SqliteDataReader^ ExecuteReader(SqliteCommandBehavior behavior);

	// ExecuteScalar (DbCommand)
	//
	// Executes the currently set SQL command text as a scalar query
	virtual Object^ ExecuteScalar(void) override;

	// Prepare (DbCommand)
	//
	// Compiles the provided command text for repeated executions
	virtual void Prepare(void) override;

	//-----------------------------------------------------------------------
	// Properties

	// CommandText (DbCommand)
	//
	// Gets or sets the SQL command text to be executed by this command
	virtual property String^ CommandText
	{
		String^ get(void) override;
		void set(String^ value) override;
	}

	// CommandTimeout (DbCommand)
	//
	// Gets or sets the amount of time, in seconds, that a command will wait
	virtual property int CommandTimeout
	{
		int get(void) override;
		void set(int value) override;
	}

	// CommandType
	//
	// Gets or sets the type of command is being used
	virtual property SqliteCommandType CommandType
	{
		SqliteCommandType get(void) new;
		void set(SqliteCommandType value);
	}

	// Connection
	//
	// Gets or sets the SqliteConnection object to use with this command
	property SqliteConnection^ Connection
	{
		SqliteConnection^ get(void) new;
		void set(SqliteConnection^ value);
	}
	
	// DesignTimeVisible (DbCommand)
	//
	// Rediculous property indicating if this object shows up in a designer.
	// Not even worth checking the object disposal flag on this one
	virtual property bool DesignTimeVisible
	{
		bool get(void) override { return m_designTimeVisible; }
		void set(bool value) override { m_designTimeVisible = value; }
	}

	// GenericCommandType (DbCommand.CommandType)
	//
	// Gets or sets the type of command is being used
	virtual property Data::CommandType GenericCommandType
	{
		Data::CommandType get(void) = DbCommand::CommandType::get;
		void set(Data::CommandType value) = DbCommand::CommandType::set;
	}

	// GenericUpdatedRowSource (DbCommand.UpdatedRowSource)
	//
	// Gets or sets how results are applied in a SqliteDataAdapter instance
	virtual property UpdateRowSource GenericUpdatedRowSource
	{
		UpdateRowSource get(void) = DbCommand::UpdatedRowSource::get;
		void set(UpdateRowSource value) = DbCommand::UpdatedRowSource::set;
	}

	// UpdatedRowSource (DbCommand)
	//
	// Gets or sets how results are applied in a SqliteDataAdapter instance
	virtual property SqliteUpdateRowSource UpdatedRowSource
	{
		SqliteUpdateRowSource get(void) new { return m_updatedRowSource; }
		void set(SqliteUpdateRowSource value);
	}

	// Parameters
	//
	// Gets a reference to the contained SqliteParameterCollection object
	property SqliteParameterCollection^ Parameters { SqliteParameterCollection^ get(void) new; }

protected:

	//-----------------------------------------------------------------------
	// Protected Member Functions

	// CreateDbParameter (DbCommand)
	//
	// Creates a new parameter object on it's generic interface
	virtual DbParameter^ CreateDbParameter(void) override;

	// ExecuteDbDataReader (DbCommand)
	//
	// Executes the currently set command via a generic data reader
	virtual DbDataReader^ ExecuteDbDataReader(CommandBehavior behavior) override;

	//-----------------------------------------------------------------------
	// Protected Properties

	// DbConnection (DbCommand)
	//
	// Gets/sets the generic connection object to use with this command
	virtual property Data::Common::DbConnection^ DbConnection
	{
		Data::Common::DbConnection^ get(void) override;
		void set(Data::Common::DbConnection^ value) override;
	}

	// DbParameterCollection (DbCommand)
	//
	// Accesses the command's parameter collection via the generic interface
	virtual property Data::Common::DbParameterCollection^ DbParameterCollection
	{
		Data::Common::DbParameterCollection^ get(void) override;
	}

	// TODO: DbTransaction
	//
	// 
	virtual property Data::Common::DbTransaction^ DbTransaction
	{
		Data::Common::DbTransaction^ get(void) override { return nullptr; }
		void set(Data::Common::DbTransaction^ value) override { (value); }
	}

private:

	// DESTRUCTOR
	~SqliteCommand();

	//-----------------------------------------------------------------------
	// Private Constants

	// DEFAULT_TIMEOUT
	//
	// The default setting for CommandTimeout, in seconds
	literal int DEFAULT_TIMEOUT = 30;

	//-----------------------------------------------------------------------
	// Private Member Functions

	// Construct
	//
	// Acts as the class pseudo-constructor to aid in overloaded calls
	void Construct(String^ commandText, SqliteConnection^ conn, SqliteCommandType type);

	// GetCommandText
	//
	// Gets appropriate query command text based on the CommandType
	String^ GetCommandText(void);

	// UncompileQuery
	//
	// If there is a prepared query in place, this cleans it up
	void UncompileQuery(void);

	//-----------------------------------------------------------------------
	// Member Variables
	
	bool					m_disposed;				// Object disposal flag
	String^					m_commandText;			// SQL command text string
	SqliteConnection^			m_conn;					// The associated connection
	int						m_timeout;				// BUSY timeout value
	SqliteQuery^				m_compiledQuery;		// Prepared query object
	bool					m_designTimeVisible;	// Stupid designer property
	SqliteParameterCollection^ m_params;				// Contained parameter col
	ObjectTracker^			m_readerTracker;		// DataReader tracker
	SqliteUpdateRowSource		m_updatedRowSource;		// Stupid data adapter property
	SqliteCommandType			m_commandType;			// Command type code
};

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)

#endif		// __SQLITECOMMAND_H_
