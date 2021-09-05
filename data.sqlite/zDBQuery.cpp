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

#include "stdafx.h"					// Include project pre-compiled headers
#include "zDBQuery.h"				// Include zDBQuery declarations

#pragma warning(push, 4)			// Enable maximum compiler warnings

BEGIN_NAMESPACE(zuki)
BEGIN_NAMESPACE(data)
BEGIN_NAMESPACE(dbms)

//---------------------------------------------------------------------------
// zDBQuery Constructor
//
// Arguments:
//
//	pDatabase		- Pointer to the SQLITE database handle wrapper
//	query			- The complete SQL query to parse into statements

zDBQuery::zDBQuery(DatabaseHandle* pDatabase, String^ query) : m_col(gcnew List<zDBStatement^>())
{
	PinnedStringPtr			pinSql;			// Pinned SQL string pointer
	PinnedStringPtr			pinStmt;		// Pointer to the current statement
	int						nResult;		// Result from function call
	sqlite3_stmt*			hStatement;		// Next statement handle
	StatementHandle*		pStatement;		// Statement handle wrapper
	const void*				pvNext;			// Pointer to the next statement
	size_t					stmtlen;		// Current statement string length
	String^					sqlstmt;		// Current SQL statement

	if(!pDatabase) throw gcnew ArgumentNullException();		// Cannot be a NULL handle
	if(query == nullptr) query = String::Empty;				// Replace NULL with ""

	pinSql = PtrToStringChars(query);		// Pin the SQL string into LPCWSTR
	pinStmt = pinSql;						// Copy as the initial start point

	try {

		// Continually break up and prepare each distinct SQL statement in the
		// command text until we hit the NULL terminator

		do {

			nResult = sqlite3_prepare16(pDatabase->Handle, pinStmt, -1, &hStatement, &pvNext);

			// Grab a copy of this particular SQL statement as parsed by the engine

			stmtlen = reinterpret_cast<const wchar_t*>(pvNext) - pinStmt;
			sqlstmt = query->Substring(static_cast<int>(pinStmt - pinSql), static_cast<int>(stmtlen));

			// If the call to prepare16 failed, throw an exception (now you see why
			// we take the time to break out a copy of the exact SQL statement)

			if(nResult != SQLITE_OK) throw gcnew zDBException(pDatabase->Handle, nResult, 
				String::Format("Preparing SQL statement [{0}]", sqlstmt));

			// Create a new zDBStatement object for this statement, and then
			// add it into our local ArrayList collection for safe keeping ...

			if(hStatement) {
				
				// It's important to always release the initial reference, just as
				// if were working with a COM object, since we're not holding the
				// reference to the wrapped handle anywhere

				pStatement = new StatementHandle(this, pDatabase, hStatement);

				try { m_col->Add(gcnew zDBStatement(pStatement, sqlstmt)); }
				finally { pStatement->Release(this); }
			}

			pinStmt += stmtlen;					// Move the string pointer along
		
		} while(*reinterpret_cast<const wchar_t*>(pvNext));
	}

	// It's important to invoke the dtor here, since we need to destroy any
	// SQL statements that were successfully prepared before the exception

	catch(Exception^) { this->~zDBQuery(); throw; }
}

//---------------------------------------------------------------------------
// zDBQuery Destructor

zDBQuery::~zDBQuery()
{
	// Dispose of any and all zDBStatements that we happen to contain
	// if this class happens to be deterministically disposed of itself

	for each(zDBStatement^ statement in m_col) delete statement;
	m_col->Clear();

	m_disposed = true;					// Object has been disposed of
}

//---------------------------------------------------------------------------
// zDBQuery::ChangeCount::get
//
// Gets the total number of changes made by each statement in this query
//
// Arguments:
//
//	NONE

int zDBQuery::ChangeCount::get(void)
{
	int					changes = 0;		// Total number of changes

	CHECK_DISPOSED(m_disposed);

	// Not too horrible .. just enumerate all the statements and tally up
	// all of their change counts into a single variable

	for each(zDBStatement^ statement in m_col) changes += statement->ChangeCount;
	return changes;
}

//---------------------------------------------------------------------------
// zDBQuery::defualt::get [int]
//
// Returns a zDBStatement object from the collection via index
//
// Arguments:
//
//	index			- Index into the collection

zDBStatement^ zDBQuery::default::get(int index)
{
	CHECK_DISPOSED(m_disposed);
	return m_col[index];
}

//---------------------------------------------------------------------------
// zDBQuery::GetEnumerator
//
// Returns an enumerator for zDBStatement objects
//
// Arguments:
//
//	NONE

Generic::IEnumerator<zDBStatement^>^ zDBQuery::GetEnumerator(void)
{
	CHECK_DISPOSED(m_disposed);
	return m_col->GetEnumerator();
}

//---------------------------------------------------------------------------
// zDBQuery::StatementCount::get
//
// Returns the number of individual statements that have been compiled
//
// Arguments:
//
//	NONE

int zDBQuery::StatementCount::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return m_col->Count;
}

//---------------------------------------------------------------------------

END_NAMESPACE(dbms)
END_NAMESPACE(data)
END_NAMESPACE(zuki)

#pragma warning(pop)
