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
#include "zDBEventArgs.h"			// Include zDBEventArgs declarations

#pragma warning(push, 4)			// Enable maximum compiler warnings

BEGIN_NAMESPACE(zuki)
BEGIN_NAMESPACE(data)
BEGIN_NAMESPACE(dbms)

//---------------------------------------------------------------------------
// ZDBAUTHORIZEEVENTARGS IMPLEMENTATION
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// zDBAuthorizeEventArgs Constructor (internal)
//
// Arguments:
//
//	action			- SQLite authorizer action code
//	arg0			- First string argument into the authorizer
//	arg1			- Second string argument into the authorizer
//	arg2			- Third string argument into the authorizer
//	indirectCaller	- Name of the indirect caller, if applicable

zDBAuthorizeEventArgs::zDBAuthorizeEventArgs(int action, const char* arg0, const char* arg1, 
	const char* arg2, const char* indirectCaller)
{
	m_action = static_cast<zDBAuthorizeAction>(action);
	m_response = zDBAuthorizeResponse::Allow;
	
	// Construct the argument array.  We can't be smart and wait until the user
	// asks for them (which would be a nice performace increase), since there is
	// nothing to stop them from yanking out a reference to this object and keep
	// it alive much longer than the argument pointers SQLite is giving to us.
	// (FastPtrToStringAnsi was created specifically for this class)

	array<String^>^ args = gcnew array<String^>(3);
	args[0] = (arg0) ? zDBUtil::FastPtrToStringAnsi(arg0) : String::Empty;
	args[1] = (arg1) ? zDBUtil::FastPtrToStringAnsi(arg1) : String::Empty;
	args[2] = (arg2) ? zDBUtil::FastPtrToStringAnsi(arg2) : String::Empty;
	m_args = Array::AsReadOnly(args);

	m_indirect = (indirectCaller) ? zDBUtil::FastPtrToStringAnsi(indirectCaller) : String::Empty;
}

//---------------------------------------------------------------------------
// ZDBCOLLATIONNEEDEDEVENTARGS IMPLEMENTATION
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// zDBCollationNeededEventArgs Constructor (internal)
//
// Arguments:
//
//	encoding		- Requested encoding of the collation
//	name			- Required name of the collation

zDBCollationNeededEventArgs::zDBCollationNeededEventArgs(int encoding, const void* name)
{
	m_encoding = static_cast<zDBCollationEncoding>(encoding);
	m_name = Marshal::PtrToStringUni(IntPtr(const_cast<void*>(name)));
}

//---------------------------------------------------------------------------

END_NAMESPACE(dbms)
END_NAMESPACE(data)
END_NAMESPACE(zuki)

#pragma warning(pop)
