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
#include "SqliteCollationWrapper.h"	// Include SqliteCollationWrapper declarations
#include "SqliteConnection.h"			// Include SqliteConnection declarations

#pragma warning(push, 4)			// Enable maximum compiler warnings

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// SqliteCollationWrapper::Invoke
//
// Invokes the contained delegate.  Depending on what kind of delegate was
// provided, the data will be treated as native UTF16 strings or a pair of
// byte arrays.
//
// Arguments:
//
//	pvLeft			- Pointer to the left-hand value
//	cbLeft			- Size of the left-hand value, in bytes
//	pvRight			- Pointer to the right-hand value
//	cbRight			- Size of the right-hand value, in bytes

int SqliteCollationWrapper::Invoke(const void* pvLeft, int cbLeft, const void* pvRight, int cbRight)
{
	// Call into one of the specialized delegate invokers to do the conversion
	// and work.  If for some reason there is no delegate, we have no way to 
	// communicate that back to SQLite so just return zero

	if(m_std != nullptr) return InvokeString(pvLeft, cbLeft, pvRight, cbRight);
	else if(m_bin != nullptr) return InvokeBinary(pvLeft, cbLeft, pvRight, cbRight);
	else return 0;
}

//---------------------------------------------------------------------------
// SqliteCollationWrapper::InvokeBinary
//
// Invokes the contained byte array collation delegate
//
// Arguments:
//
//	pvLeft			- Pointer to the left-hand value
//	cbLeft			- Size of the left-hand value, in bytes
//	pvRight			- Pointer to the right-hand value
//	cbRight			- Size of the right-hand value, in bytes

int SqliteCollationWrapper::InvokeBinary(const void* pvLeft, int cbLeft, const void* pvRight, int cbRight)
{
	array<System::Byte>^		left;			// Left-hand array of bytes
	array<System::Byte>^		right;			// Right-hand array of bytes
	PinnedBytePtr				pinBytes;		// Pinned byte array pointer
	int							result;			// Result of collation function

	Debug::Assert(m_bin != nullptr);

	// Allocate and initialize the left-hand collation byte array

	left = gcnew array<System::Byte>(cbLeft);
	if(cbLeft) {

		pinBytes = &left[0];
		memcpy_s(pinBytes, left->Length, pvLeft, cbLeft);
	}

	// Allocate and initialize the right-hand collection byte array
	right = gcnew array<System::Byte>(cbRight);
	if(cbRight) {

		pinBytes = &right[0];
		memcpy_s(pinBytes, right->Length, pvRight, cbRight);
	}

	SqliteConnection^ conn = SqliteConnection::FindConnection(m_hDatabase);

	result = m_bin(conn, left, right);			// Invoke the delegate handler
	if(conn != nullptr) GC::KeepAlive(conn);	// Keep alive until here
	return result;								// Return result from collation
}

//---------------------------------------------------------------------------
// SqliteCollationWrapper::InvokeString
//
// Invokes the contained string collation delegate
//
// Arguments:
//
//	pvLeft			- Pointer to the left-hand value
//	cbLeft			- Size of the left-hand value, in bytes
//	pvRight			- Pointer to the right-hand value
//	cbRight			- Size of the right-hand value, in bytes

int SqliteCollationWrapper::InvokeString(const void* pvLeft, int cbLeft, const void* pvRight, int cbRight)
{
	String^						left;			// Left-hand string for comparison
	String^						right;			// Right-hand string for comparison
	int							result;			// Result of collation function

	Debug::Assert(m_std != nullptr);

	// Allocate and initialize the managed strings.  We always pass UTF16 into create()
	// for this provider, so this would work just dandy.  It's also much faster than
	// calling Marshal::PtrToStringUni, which uses a StringBuilder of all things

	left = gcnew String(reinterpret_cast<const wchar_t*>(pvLeft), 0, cbLeft / sizeof(wchar_t));
	right = gcnew String(reinterpret_cast<const wchar_t*>(pvRight), 0, cbRight / sizeof(wchar_t));
	
	SqliteConnection^ conn = SqliteConnection::FindConnection(m_hDatabase);

	result = m_std(conn, left, right);			// Invoke the delegate handler
	if(conn != nullptr) GC::KeepAlive(conn);	// Keep alive until here
	return result;								// Return result from collation
}

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)
