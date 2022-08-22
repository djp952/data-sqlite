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
#include "SqliteTransaction.h"			// Include SqliteTransaction declarations
#include "SqliteConnection.h"			// Include SqliteConnection declarations

#pragma warning(push, 4)			// Enable maximum compiler warnings

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// SqliteTransaction Constructor
//
// Arguments:
//
//	conn		- Parent SqliteConnection object instance

SqliteTransaction::SqliteTransaction(SqliteConnection^ conn) : m_conn(conn) {}

//---------------------------------------------------------------------------
// SqliteTransaction Destructor

SqliteTransaction::~SqliteTransaction()
{
	if(!m_closed) Rollback();			// Rollback on disposal
	m_disposed = true;					// Object is now disposed of
}

//---------------------------------------------------------------------------
// SqliteTransaction::Commit
//
// Commits this outstanding transaction
//
// Arguments:
//
//	NONE

void SqliteTransaction::Commit(void)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckConnectionValid(m_conn);
	if(m_closed) throw gcnew InvalidOperationException();

	m_conn->CommitTransaction(this);		// Commit the transaction
	m_closed = true;						// We're done now
}

//---------------------------------------------------------------------------
// SqliteTransaction::Connection::get
//
// Gets a reference to the connection that owns this transaction

SqliteConnection^ SqliteTransaction::Connection::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return m_conn;
}

//---------------------------------------------------------------------------
// SqliteTransaction::DbConnection::get (protected)
//
// Gets a reference to the connection that owns this transaction, on it's
// generic interface

DbConnection^ SqliteTransaction::DbConnection::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return m_conn;
}

//---------------------------------------------------------------------------
// SqliteTransaction::IsolationLevel::get
//
// Returns the isolation level of the transaction.  Not supported in SQLite

Data::IsolationLevel SqliteTransaction::IsolationLevel::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return Data::IsolationLevel::Unspecified;
}

//---------------------------------------------------------------------------
// SqliteTransaction::Rollback
//
// Rolls back this outstanding transaction
//
// Arguments:
//
//	NONE

void SqliteTransaction::Rollback(void)
{
	CHECK_DISPOSED(m_disposed);
	SqliteUtil::CheckConnectionValid(m_conn);
	if(m_closed) throw gcnew InvalidOperationException();

	m_conn->RollbackTransaction(this);		// Roll back the transaction
	m_closed = true;						// We're done now
}

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)
