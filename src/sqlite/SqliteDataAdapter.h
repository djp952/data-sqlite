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

#ifndef __SQLITEDATAADAPTER_H_
#define __SQLITEDATAADAPTER_H_
#pragma once

#include "SqliteCommand.h"					// Include SqliteCommand declarations
#include "SqliteConnection.h"				// Include SqliteConnection declarations
#include "SqliteDelegates.h"				// Include Sqlite delegate decls
#include "SqliteEnumerations.h"			// Include Sqlite enumeration decls
#include "SqliteEventArgs.h"				// Include Sqlite eventarg declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings
#pragma warning(disable:4100)			// "unreferenced formal parameter"

using namespace System;
using namespace System::Data;
using namespace System::Data::Common;

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// Class SqliteDataAdapter
//
// Implements the data adapter for the ADO.NET provider, including all of the
// necessary strongly typed additional methods and properties.
//---------------------------------------------------------------------------

public ref class SqliteDataAdapter sealed : public DbDataAdapter
{
public:

	//-----------------------------------------------------------------------
	// Constructors

	SqliteDataAdapter();
	SqliteDataAdapter(SqliteCommand^ command);
	SqliteDataAdapter(String^ commandText, SqliteConnection^ connection);
	SqliteDataAdapter(String^ commandText, String^ connectionString);

	//-----------------------------------------------------------------------
	// Events

	// RowUpdated
	//
	// Provider-specific version of the RowUpdated event
	event SqliteRowUpdatedEventHandler^ RowUpdated;

	// RowUpdating
	//
	// Provider-specific version of the RowUpdating event
	event SqliteRowUpdatingEventHandler^ RowUpdating;

	//-----------------------------------------------------------------------
	// Properties

	// DeleteCommand
	//
	// Gets or sets the DELETE command object instance
	property SqliteCommand^ DeleteCommand
	{
		SqliteCommand^ get(void) new;
		void set(SqliteCommand^ value);
	}

	// InsertCommand
	//
	// Gets or sets the INSERT command object instance
	property SqliteCommand^ InsertCommand
	{
		SqliteCommand^ get(void) new;
		void set(SqliteCommand^ value);
	}

	// SelectCommand
	//
	// Gets or sets the SELECT command object instance
	property SqliteCommand^ SelectCommand
	{
		SqliteCommand^ get(void) new;
		void set(SqliteCommand^ value);
	}

	// UpdateCommand
	//
	// Gets or sets the UPDATE command object instance
	property SqliteCommand^ UpdateCommand
	{
		SqliteCommand^ get(void) new;
		void set(SqliteCommand^ value);
	}

protected:

	//-----------------------------------------------------------------------
	// Protected Member Functions

	// CreateRowUpdatedEvent (DbDataAdapter)
	//
	// Generates a provider-specific instance of RowUpdatedEventArgs
	virtual RowUpdatedEventArgs^ CreateRowUpdatedEvent(DataRow^ row, IDbCommand^ cmd,
		StatementType type, DataTableMapping^ mapping) override
	{ 
		return gcnew SqliteRowUpdatedEventArgs(row, cmd, type, mapping); 
	}

	// CreateRowUpdatingEvent (DbDataAdapter)
	//
	// Generates a provider-specific instance of RowUpdatingEventArgs
	virtual RowUpdatingEventArgs^ CreateRowUpdatingEvent(DataRow^ row, IDbCommand^ cmd,
		StatementType type, DataTableMapping^ mapping) override
	{ 
		return gcnew SqliteRowUpdatingEventArgs(row, cmd, type, mapping); 
	}

	// OnRowUpdated
	//
	// Implements the specific version of this event
	virtual void OnRowUpdated(RowUpdatedEventArgs^ value) override
	{
		RowUpdated(this, safe_cast<SqliteRowUpdatedEventArgs^>(value));
	}

	// OnRowUpdating
	//
	// Implements the specific version of this event
	virtual void OnRowUpdating(RowUpdatingEventArgs^ value) override
	{
		RowUpdating(this, safe_cast<SqliteRowUpdatingEventArgs^>(value));
	}

private:

	//-----------------------------------------------------------------------
	// Private Member Functions

	// OnDispose
	//
	// Event handler for the base class's Dispose event
	void OnDispose(Object^ sender, EventArgs^ args) { m_disposed = true; }

	//-----------------------------------------------------------------------
	// Private Properties

	// DbDeleteCommand (IDbDataAdapter)
	//
	// Gets/sets the DELETE command via it's generic interface
	virtual property IDbCommand^ DbDeleteCommand
	{
		IDbCommand^ get(void) sealed = IDbDataAdapter::DeleteCommand::get { return DeleteCommand; }
		void set(IDbCommand^ value) sealed = IDbDataAdapter::DeleteCommand::set 
		{ 
			DeleteCommand = safe_cast<SqliteCommand^>(value); 
		}
	}

	// DbInsertCommand (IDbDataAdapter)
	//
	// Gets/sets the INSERT command via it's generic interface
	virtual property IDbCommand^ DbInsertCommand
	{
		IDbCommand^ get(void) sealed = IDbDataAdapter::InsertCommand::get { return InsertCommand; }
		void set(IDbCommand^ value) sealed = IDbDataAdapter::InsertCommand::set 
		{ 
			InsertCommand = safe_cast<SqliteCommand^>(value); 
		}
	}

	// DbSelectCommand (IDbDataAdapter)
	//
	// Gets/sets the SELECT command via it's generic interface
	virtual property IDbCommand^ DbSelectCommand
	{
		IDbCommand^ get(void) sealed = IDbDataAdapter::SelectCommand::get { return SelectCommand; }
		void set(IDbCommand^ value) sealed = IDbDataAdapter::SelectCommand::set 
		{ 
			SelectCommand = safe_cast<SqliteCommand^>(value); 
		}
	}

	// DbUpdateCommand (IDbDataAdapter)
	//
	// Gets/sets the UPDATE command via it's generic interface
	virtual property IDbCommand^ DbUpdateCommand
	{
		IDbCommand^ get(void) sealed = IDbDataAdapter::UpdateCommand::get { return UpdateCommand; }
		void set(IDbCommand^ value) sealed = IDbDataAdapter::UpdateCommand::set 
		{ 
			UpdateCommand = safe_cast<SqliteCommand^>(value); 
		}
	}

	//-----------------------------------------------------------------------
	// Member Variables

	bool					m_disposed;			// Object disposal flag
	SqliteCommand^				m_delete;			// Contained DELETE command
	SqliteCommand^				m_insert;			// Contained INSERT command
	SqliteCommand^				m_select;			// Contained SELECT command
	SqliteCommand^				m_update;			// Contained UPDATE command
};

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)

#endif		// __SQLITEDATAADAPTER_H_
