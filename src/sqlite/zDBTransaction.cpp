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
#include "zDBTransaction.h"			// Include zDBTransaction declarations
#include "zDBConnection.h"			// Include zDBConnection declarations

#pragma warning(push, 4)			// Enable maximum compiler warnings

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// zDBTransaction Constructor
//
// Arguments:
//
//	conn		- Parent zDBConnection object instance

zDBTransaction::zDBTransaction(zDBConnection^ conn) : m_conn(conn) {}

//---------------------------------------------------------------------------
// zDBTransaction Destructor

zDBTransaction::~zDBTransaction()
{
	if(!m_closed) Rollback();			// Rollback on disposal
	m_disposed = true;					// Object is now disposed of
}

//---------------------------------------------------------------------------
// zDBTransaction::Commit
//
// Commits this outstanding transaction
//
// Arguments:
//
//	NONE

void zDBTransaction::Commit(void)
{
	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckConnectionValid(m_conn);
	if(m_closed) throw gcnew InvalidOperationException();

	m_conn->CommitTransaction(this);		// Commit the transaction
	m_closed = true;						// We're done now
}

//---------------------------------------------------------------------------
// zDBTransaction::Connection::get
//
// Gets a reference to the connection that owns this transaction

zDBConnection^ zDBTransaction::Connection::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return m_conn;
}

//---------------------------------------------------------------------------
// zDBTransaction::DbConnection::get (protected)
//
// Gets a reference to the connection that owns this transaction, on it's
// generic interface

DbConnection^ zDBTransaction::DbConnection::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return m_conn;
}

//---------------------------------------------------------------------------
// zDBTransaction::IsolationLevel::get
//
// Returns the isolation level of the transaction.  Not supported in SQLite

Data::IsolationLevel zDBTransaction::IsolationLevel::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return Data::IsolationLevel::Unspecified;
}

//---------------------------------------------------------------------------
// zDBTransaction::Rollback
//
// Rolls back this outstanding transaction
//
// Arguments:
//
//	NONE

void zDBTransaction::Rollback(void)
{
	CHECK_DISPOSED(m_disposed);
	zDBUtil::CheckConnectionValid(m_conn);
	if(m_closed) throw gcnew InvalidOperationException();

	m_conn->RollbackTransaction(this);		// Roll back the transaction
	m_closed = true;						// We're done now
}

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)
