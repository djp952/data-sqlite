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
#include "SqliteEventArgs.h"			// Include SqliteEventArgs declarations

#pragma warning(push, 4)			// Enable maximum compiler warnings

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// SQLITEAUTHORIZEEVENTARGS IMPLEMENTATION
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// SqliteAuthorizeEventArgs Constructor (internal)
//
// Arguments:
//
//	action			- SQLite authorizer action code
//	arg0			- First string argument into the authorizer
//	arg1			- Second string argument into the authorizer
//	arg2			- Third string argument into the authorizer
//	indirectCaller	- Name of the indirect caller, if applicable

SqliteAuthorizeEventArgs::SqliteAuthorizeEventArgs(int action, const char* arg0, const char* arg1, 
	const char* arg2, const char* indirectCaller)
{
	m_action = static_cast<SqliteAuthorizeAction>(action);
	m_response = SqliteAuthorizeResponse::Allow;
	
	// Construct the argument array.  We can't be smart and wait until the user
	// asks for them (which would be a nice performace increase), since there is
	// nothing to stop them from yanking out a reference to this object and keep
	// it alive much longer than the argument pointers SQLite is giving to us.
	// (FastPtrToStringAnsi was created specifically for this class)

	array<String^>^ args = gcnew array<String^>(3);
	args[0] = (arg0) ? SqliteUtil::FastPtrToStringAnsi(arg0) : String::Empty;
	args[1] = (arg1) ? SqliteUtil::FastPtrToStringAnsi(arg1) : String::Empty;
	args[2] = (arg2) ? SqliteUtil::FastPtrToStringAnsi(arg2) : String::Empty;
	m_args = Array::AsReadOnly(args);

	m_indirect = (indirectCaller) ? SqliteUtil::FastPtrToStringAnsi(indirectCaller) : String::Empty;
}

//---------------------------------------------------------------------------
// SQLITECOLLATIONNEEDEDEVENTARGS IMPLEMENTATION
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// SqliteCollationNeededEventArgs Constructor (internal)
//
// Arguments:
//
//	encoding		- Requested encoding of the collation
//	name			- Required name of the collation

SqliteCollationNeededEventArgs::SqliteCollationNeededEventArgs(int encoding, const void* name)
{
	m_encoding = static_cast<SqliteCollationEncoding>(encoding);
	m_name = Marshal::PtrToStringUni(IntPtr(const_cast<void*>(name)));
}

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)
