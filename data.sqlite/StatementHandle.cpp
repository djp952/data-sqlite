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

#include "stdafx.h"						// Include project pre-compiled headers
#include "StatementHandle.h"			// Include StatementHandle declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings
#pragma warning(disable:4100)			// "unreferenced formal parameter"

//---------------------------------------------------------------------------
// StatementHandle Constructor
//
// Arguments:
//
//	caller			- Object calling the ctor (for tracing purposes)
//	pDatabase		- Pointer to the database handle wrapper
//	hStatement		- Statement handle to wrap and take ownership of

StatementHandle::StatementHandle(Object^ caller, DatabaseHandle* pDatabase, sqlite3_stmt* hStatement)
	: m_pDatabase(pDatabase), m_hStatement(hStatement), m_cRefCount(1)
{
	if(!m_pDatabase) throw gcnew ArgumentNullException();
	if(!m_hStatement) throw gcnew ArgumentNullException();

#ifdef ZDB_TRACE_HANDLEREF
	Debug::WriteLine(String::Format("StatementHandle 0x{0:X}:{1:X} ---> 1 (NEW via {2})", 
		IntPtr(m_pDatabase), IntPtr(this),
		caller->GetType()->Name));
#endif

	m_pDatabase->AddRef(nullptr);			// <--- This lives as long as we do
}

//---------------------------------------------------------------------------
// StatementHandle Destructor (private)

StatementHandle::~StatementHandle()
{
	try {
	
		int nResult = sqlite3_finalize(m_hStatement);	
		if(nResult != SQLITE_OK) {} /* TODO (REMOVED): throw gcnew zDBException(m_pDatabase->Handle, nResult); */

#ifdef ZDB_TRACE_HANDLEREF
	Debug::WriteLine(String::Format("StatementHandle 0x{0:X}:{1:X} destroyed.", 
		IntPtr(m_pDatabase), IntPtr(this)));
#endif

	}
	finally { m_pDatabase->Release(nullptr); }	// Always release the database
}

//---------------------------------------------------------------------------
// StatementHandle::operator =
//
// Assigns a new handle to this wrapper without de-referencing the database
//
// Arguments:
//
//	hStatement		- The new SQLite statement handle to assign

StatementHandle& StatementHandle::operator =(sqlite3_stmt* hStatement)
{
	int nResult = sqlite3_finalize(m_hStatement);
	if(nResult != SQLITE_OK) throw gcnew zDBException(m_pDatabase->Handle, nResult);

	m_hStatement = hStatement;				// Switch out the handle

#ifdef ZDB_TRACE_HANDLEREF
	Debug::WriteLine(String::Format("StatementHandle 0x{0:X}:{1:X} internal handle replaced.",
		IntPtr(m_pDatabase), IntPtr(this)));
#endif

	return *this;							// Return ourselves to caller
}

//---------------------------------------------------------------------------
// StatementHandle::AddRef
//
// Increments the object reference count
//
// Arguments:
//
//	caller		- Object calling AddRef() for tracing purposes

void StatementHandle::AddRef(Object^ caller)
{
	InterlockedIncrement(&m_cRefCount);		// Increment reference count

#ifdef ZDB_TRACE_HANDLEREF
	Debug::WriteLine(String::Format("StatementHandle 0x{0:X}:{1:X} ---> {2} ({3})", 
		IntPtr(m_pDatabase), IntPtr(this),
		m_cRefCount, caller->GetType()->Name));
#endif
}

//---------------------------------------------------------------------------
// StatementHandle::Release
//
// Releases a reference count on the object, and destroys itself at zero
//
// Arguments:
//
//	caller		- Object calling Release() for tracing purposes

void StatementHandle::Release(Object^ caller)
{
#ifdef ZDB_TRACE_HANDLEREF
	Debug::WriteLine(String::Format("StatementHandle 0x{0:X}:{1:X} <--- {2} ({3})", 
		IntPtr(m_pDatabase), IntPtr(this),
		m_cRefCount - 1, caller->GetType()->Name));
#endif

	if(InterlockedDecrement(&m_cRefCount) == 0) delete this;
}

//---------------------------------------------------------------------------

#pragma warning(pop)
