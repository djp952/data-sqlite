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

#ifndef __SQLITETRANSACTION_H_
#define __SQLITETRANSACTION_H_
#pragma once

#pragma warning(push, 4)				// Enable maximum compiler warnings

using namespace System;
using namespace System::Data;
using namespace System::Data::Common;

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// Forward Class Declarations

ref class SqliteConnection;				// SqliteConnection.h

//---------------------------------------------------------------------------
// Class SqliteTransaction
//
// Implments the transaction control object for the ADO.NET provider.  Note
// that the transactions themselves are implemented in the connection object,
// this is just an object model convenience.
//---------------------------------------------------------------------------

public ref class SqliteTransaction sealed : public DbTransaction
{
public:

	//-----------------------------------------------------------------------
	// Member Functions

	// Commit (DbTransaction)
	//
	// Commits the transaction
	virtual void Commit(void) override;

	// Rollback (DbTransaction)
	//
	// Rolls back the transaction
	virtual void Rollback(void) override;

	//-----------------------------------------------------------------------
	// Properties

	// Connection
	//
	// Gets a reference to the contained SqliteConnection object instance
	property SqliteConnection^ Connection { SqliteConnection^ get(void) new; }

	// IsolationLevel (DbTransaction)
	//
	// Gets the isolation level of the transaction
	virtual property Data::IsolationLevel IsolationLevel { Data::IsolationLevel get(void) override; }

protected:

	//-----------------------------------------------------------------------
	// Protected Properties

	// DbConnection (DbTransaction)
	//
	// Gets a reference to the contained connection object on it's generic interface
	virtual property Common::DbConnection^ DbConnection { Common::DbConnection^ get(void) override; }

internal:

	// INTERNAL CONSTRUCTOR
	SqliteTransaction(SqliteConnection^ conn);

private:

	// DESTRUCTOR
	~SqliteTransaction();

	//-----------------------------------------------------------------------
	// Member Variables
	
	bool					m_disposed;			// Object disposal flag
	bool					m_closed;			// Transaction closed flag
	SqliteConnection^			m_conn;				// Referenced connection
};

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)

#endif		// __SQLITETRANSACTION_H_
