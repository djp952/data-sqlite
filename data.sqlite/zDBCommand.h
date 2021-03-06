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

#ifndef __ZDBCOMMAND_H_
#define __ZDBCOMMAND_H_
#pragma once

#include "ITrackableObject.h"			// Include ITrackableObject decls
#include "ObjectTracker.h"				// Include ObjectTracker decls
#include "zDBParameter.h"				// Include zDBParameter declarations
#include "zDBParameterCollection.h"		// Include zDBParameterCollection decls
#include "zDBQuery.h"					// Include zDBQuery declarations
#include "zDBStatement.h"				// Include zDBStatement decls

#pragma warning(push, 4)				// Enable maximum compiler warnings

using namespace System;
using namespace System::Data;
using namespace System::Data::Common;

BEGIN_NAMESPACE(zuki)
BEGIN_NAMESPACE(data)
BEGIN_NAMESPACE(dbms)

//---------------------------------------------------------------------------
// Forward Class Declarations
//---------------------------------------------------------------------------

ref class zDBConnection;				// zDBConnection.h
ref class zDBDataAdapter;				// zDBDataAdapter.h
ref class zDBDataReader;				// zDBDataReader.h
ref class zDBTransaction;				// zDBTransaction.h

//---------------------------------------------------------------------------
// Class zDBCommand
//
// Represents a SQL statement to be executed against the database.  Note that
// this provider does not support Stored Procedures and all commands must
// be based on a text SQL statement.
//---------------------------------------------------------------------------

public ref class zDBCommand sealed : public DbCommand
{
public:

	//-----------------------------------------------------------------------
	// Constructors

	zDBCommand() 
		{ Construct(String::Empty, nullptr, zDBCommandType::Text); }

	zDBCommand(String^ commandText)
		{ Construct(commandText, nullptr, zDBCommandType::Text); }

	zDBCommand(String^ commandText, zDBCommandType commandType)
		{ Construct(commandText, nullptr, commandType); }

	zDBCommand(String^ commandText, zDBConnection^ connection)
		{ Construct(commandText, connection, zDBCommandType::Text); }

	zDBCommand(String^ commandText, zDBConnection^ connection, zDBCommandType commandType)
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
	// Executes the SQL command and returns a zDBDataReader to process the results
	zDBDataReader^ ExecuteReader(void) new { return ExecuteReader(zDBCommandBehavior::Default); }
	zDBDataReader^ ExecuteReader(zDBCommandBehavior behavior);

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
	virtual property zDBCommandType CommandType
	{
		zDBCommandType get(void) new;
		void set(zDBCommandType value);
	}

	// Connection
	//
	// Gets or sets the zDBConnection object to use with this command
	property zDBConnection^ Connection
	{
		zDBConnection^ get(void) new;
		void set(zDBConnection^ value);
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
	// Gets or sets how results are applied in a zDBDataAdapter instance
	virtual property UpdateRowSource GenericUpdatedRowSource
	{
		UpdateRowSource get(void) = DbCommand::UpdatedRowSource::get;
		void set(UpdateRowSource value) = DbCommand::UpdatedRowSource::set;
	}

	// UpdatedRowSource (DbCommand)
	//
	// Gets or sets how results are applied in a zDBDataAdapter instance
	virtual property zDBUpdateRowSource UpdatedRowSource
	{
		zDBUpdateRowSource get(void) new { return m_updatedRowSource; }
		void set(zDBUpdateRowSource value);
	}

	// Parameters
	//
	// Gets a reference to the contained zDBParameterCollection object
	property zDBParameterCollection^ Parameters { zDBParameterCollection^ get(void) new; }

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
	~zDBCommand();

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
	void Construct(String^ commandText, zDBConnection^ conn, zDBCommandType type);

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
	zDBConnection^			m_conn;					// The associated connection
	int						m_timeout;				// BUSY timeout value
	zDBQuery^				m_compiledQuery;		// Prepared query object
	bool					m_designTimeVisible;	// Stupid designer property
	zDBParameterCollection^ m_params;				// Contained parameter col
	ObjectTracker^			m_readerTracker;		// DataReader tracker
	zDBUpdateRowSource		m_updatedRowSource;		// Stupid data adapter property
	zDBCommandType			m_commandType;			// Command type code
};

//---------------------------------------------------------------------------

END_NAMESPACE(dbms)
END_NAMESPACE(data)
END_NAMESPACE(zuki)

#pragma warning(pop)

#endif		// __ZDBCOMMAND_H_
