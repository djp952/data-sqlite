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

#ifndef __STDAFX_H_
#define __STDAFX_H_
#pragma once

//---------------------------------------------------------------------------
// CRT / Win32 Declarations
//---------------------------------------------------------------------------

#define WINVER				0x0601			// Windows 7
#define	_WIN32_WINNT		0x0601			// Windows 7

#include <windows.h>		// Include base Win32 declarations
#include <vcclr.h>			// Include VC CLR extensions
#include <malloc.h>			// Include CRT memory allocation declarations
#include <wchar.h>			// Include Unicode string API declarations
#include <crtdbg.h>			// Include C runtime library debugging helpers

//---------------------------------------------------------------------------
// Macros
//---------------------------------------------------------------------------

// CHECK_DISPOSED
//
// Used throughout to make object disposed exceptions easier to read and not
// require hard-coding a class name into the statement.  This will throw the
// function name, but that's actually better in my opinion

#define CHECK_DISPOSED(__flag) \
	if(__flag) throw gcnew ObjectDisposedException(gcnew String(__FUNCTION__));

//---------------------------------------------------------------------------
// Pinned Pointer Types

typedef pin_ptr<unsigned __int8>	PinnedBytePtr;
typedef pin_ptr<wchar_t>			PinnedCharPtr;
typedef pin_ptr<const wchar_t>		PinnedStringPtr;
typedef pin_ptr<void>				PinnedVoidPtr;

//---------------------------------------------------------------------------
// SQLite
//---------------------------------------------------------------------------

#include <sqlite3.h>

// LNK4248: unresolved typeref token
//
struct sqlite3 {};
struct sqlite3_api_routines {};
struct sqlite3_context {};
struct sqlite3_stmt {};
struct sqlite3_value {};

// ENGINE_ISSUE
//
// Used to denote something that appears to be a problem with the specific
// version of SQLite being used.  Will cause a Debug assertion if the engine
// version changes so I can see if it has been fixed or not.
//
// Example:
//
// ENGINE_ISSUE(3.3.7, "PRAGMA CASE_SENSITIVE_LIKE does not return a value");

#ifdef _DEBUG
#define ENGINE_ISSUE(__version, __message) \
	System::Diagnostics::Debug::Assert(gcnew Version(SQLITE_VERSION) <= gcnew Version(#__version), __message);
#else
#define ENGINE_ISSUE(__version, __message)
#endif	// _DEBUG

//---------------------------------------------------------------------------
// ZLIB Compression Library
//---------------------------------------------------------------------------

#include <zlib.h>					// Include main ZLIB declarations

//---------------------------------------------------------------------------
// Project-Wide #defines
//---------------------------------------------------------------------------

// SQLITE_TRACE_HANDLEREF
//
// Monitors the reference counted SQLite handle AddRef() and Release()
#ifdef _DEBUG
#define SQLITE_TRACE_HANDLEREF
#endif

// sqlite_TRACE_CONNECTIONHOOKS
//
// Monitors the SqliteConnectionHook install/uninstalls
#ifdef _DEBUG
#define sqlite_TRACE_CONNECTIONHOOKS
#endif

// sqlite_TRACE_FUNCTIONS
//
// Monitors the activity of the SqliteAggregate/Function/Collation collections
#ifdef _DEBUG
#define sqlite_TRACE_FUNCTIONS
#endif

//---------------------------------------------------------------------------

#endif	// __STDAFX_H_
