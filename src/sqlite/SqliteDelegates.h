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

#ifndef __SQLITEDELEGATES_H_
#define __SQLITEDELEGATES_H_
#pragma once

#include "SqliteEventArgs.h"				// Include Sqlite eventarg declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

using namespace System;

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// Delegate SqliteAuthorizeEventHandler
//
// Used by SqliteConnection to raise a hooked authorization event
public delegate void SqliteAuthorizeEventHandler(Object^ sender, SqliteAuthorizeEventArgs^ args);

//---------------------------------------------------------------------------
// Delegate SqliteCollationNeededEventHandler
//
// Used by SqliteConnection to raise a hooked collation needed event
public delegate void SqliteCollationNeededEventHandler(Object^ sender, SqliteCollationNeededEventArgs^ args);

//---------------------------------------------------------------------------
// Delegate SqliteProfileEventHandler
//
// Used by SqliteConnection to raise a hooked profile message event
public delegate void SqliteProfileEventHandler(Object^ sender, SqliteProfileEventArgs^ args);

//---------------------------------------------------------------------------
// Delegate SqliteRowChangedEventHandler
//
// Used by SqliteConnection to raise a hooked DELETE/INSERT/UPDATE event
public delegate void SqliteRowChangedEventHandler(Object^ sender, SqliteRowChangedEventArgs^ args);

//---------------------------------------------------------------------------
// Delegate SqliteRowUpdatingEventHandler
//
// Used by SqliteDataAdapter to raise an event
public delegate void SqliteRowUpdatingEventHandler(Object^ sender, SqliteRowUpdatingEventArgs^ args);

//---------------------------------------------------------------------------
// Delegate SqliteRowUpdatedEventHandler
//
// Used by SqliteDataAdapter to raise an event
public delegate void SqliteRowUpdatedEventHandler(Object^ sender, SqliteRowUpdatedEventArgs^ args);

//---------------------------------------------------------------------------
// Delegate SqliteTraceEventHandler
//
// Used by SqliteConnection to raise a hooked trace message event
public delegate void SqliteTraceEventHandler(Object^ sender, SqliteTraceEventArgs^ args);

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)

#endif		// __SQLITEDELEGATES_H_