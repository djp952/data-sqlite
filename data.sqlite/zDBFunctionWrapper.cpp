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
#include "zDBFunctionWrapper.h"		// Include zDBFunctionWrapper declarations
#include "zDBConnection.h"			// Include zDBConnection declarations

#pragma warning(push, 4)			// Enable maximum compiler warnings

BEGIN_NAMESPACE(zuki)
BEGIN_NAMESPACE(data)
BEGIN_NAMESPACE(dbms)

//---------------------------------------------------------------------------
// zDBFunctionWrapper::Invoke
//
// Invokes the contained delegate using the provided argument information
//
// Arguments:
//
//	context		- SQLite function context pointer
//	argc		- Number of function arguments
//	argv		- Array of function argument values

void zDBFunctionWrapper::Invoke(sqlite3_context* context, int argc, sqlite3_value** argv)
{
	zDBArgumentCollection^		args;		// Function argument collection
	zDBResult^					result;		// Function result object

	if(m_func == nullptr) return;			// No delegate, nothing to do

	zDBConnection^ conn = zDBConnection::FindConnection(m_hDatabase);

	// Both the arguments as well as the result need to be disposed of
	// as soon as we are finished with them, so use an ugly little nested
	// try/finally setup in here

	args = gcnew zDBArgumentCollection(argc, argv);
	try { 

		result = gcnew zDBResult(conn, context);

		try { m_func(conn, args, result); }		// Invoke the delegate
		finally { delete result; }				// Dispose of result
	}
	
	finally { delete args; }					// Dispose of arguments

	GC::KeepAlive(conn);						// Keep the connection alive
}	

//---------------------------------------------------------------------------

END_NAMESPACE(dbms)
END_NAMESPACE(data)
END_NAMESPACE(zuki)

#pragma warning(pop)
