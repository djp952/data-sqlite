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

#ifndef __ZDBDATAADAPTER_H_
#define __ZDBDATAADAPTER_H_
#pragma once

#include "zDBCommand.h"					// Include zDBCommand declarations
#include "zDBConnection.h"				// Include zDBConnection declarations
#include "zDBDelegates.h"				// Include zDB delegate decls
#include "zDBEnumerations.h"			// Include zDB enumeration decls
#include "zDBEventArgs.h"				// Include zDB eventarg declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings
#pragma warning(disable:4100)			// "unreferenced formal parameter"

using namespace System;
using namespace System::Data;
using namespace System::Data::Common;

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// Class zDBDataAdapter
//
// Implements the data adapter for the ADO.NET provider, including all of the
// necessary strongly typed additional methods and properties.
//---------------------------------------------------------------------------

public ref class zDBDataAdapter sealed : public DbDataAdapter
{
public:

	//-----------------------------------------------------------------------
	// Constructors

	zDBDataAdapter();
	zDBDataAdapter(zDBCommand^ command);
	zDBDataAdapter(String^ commandText, zDBConnection^ connection);
	zDBDataAdapter(String^ commandText, String^ connectionString);

	//-----------------------------------------------------------------------
	// Events

	// RowUpdated
	//
	// Provider-specific version of the RowUpdated event
	event zDBRowUpdatedEventHandler^ RowUpdated;

	// RowUpdating
	//
	// Provider-specific version of the RowUpdating event
	event zDBRowUpdatingEventHandler^ RowUpdating;

	//-----------------------------------------------------------------------
	// Properties

	// DeleteCommand
	//
	// Gets or sets the DELETE command object instance
	property zDBCommand^ DeleteCommand
	{
		zDBCommand^ get(void) new;
		void set(zDBCommand^ value);
	}

	// InsertCommand
	//
	// Gets or sets the INSERT command object instance
	property zDBCommand^ InsertCommand
	{
		zDBCommand^ get(void) new;
		void set(zDBCommand^ value);
	}

	// SelectCommand
	//
	// Gets or sets the SELECT command object instance
	property zDBCommand^ SelectCommand
	{
		zDBCommand^ get(void) new;
		void set(zDBCommand^ value);
	}

	// UpdateCommand
	//
	// Gets or sets the UPDATE command object instance
	property zDBCommand^ UpdateCommand
	{
		zDBCommand^ get(void) new;
		void set(zDBCommand^ value);
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
		return gcnew zDBRowUpdatedEventArgs(row, cmd, type, mapping); 
	}

	// CreateRowUpdatingEvent (DbDataAdapter)
	//
	// Generates a provider-specific instance of RowUpdatingEventArgs
	virtual RowUpdatingEventArgs^ CreateRowUpdatingEvent(DataRow^ row, IDbCommand^ cmd,
		StatementType type, DataTableMapping^ mapping) override
	{ 
		return gcnew zDBRowUpdatingEventArgs(row, cmd, type, mapping); 
	}

	// OnRowUpdated
	//
	// Implements the specific version of this event
	virtual void OnRowUpdated(RowUpdatedEventArgs^ value) override
	{
		RowUpdated(this, safe_cast<zDBRowUpdatedEventArgs^>(value));
	}

	// OnRowUpdating
	//
	// Implements the specific version of this event
	virtual void OnRowUpdating(RowUpdatingEventArgs^ value) override
	{
		RowUpdating(this, safe_cast<zDBRowUpdatingEventArgs^>(value));
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
			DeleteCommand = safe_cast<zDBCommand^>(value); 
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
			InsertCommand = safe_cast<zDBCommand^>(value); 
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
			SelectCommand = safe_cast<zDBCommand^>(value); 
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
			UpdateCommand = safe_cast<zDBCommand^>(value); 
		}
	}

	//-----------------------------------------------------------------------
	// Member Variables

	bool					m_disposed;			// Object disposal flag
	zDBCommand^				m_delete;			// Contained DELETE command
	zDBCommand^				m_insert;			// Contained INSERT command
	zDBCommand^				m_select;			// Contained SELECT command
	zDBCommand^				m_update;			// Contained UPDATE command
};

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)

#endif		// __ZDBDATAADAPTER_H_
