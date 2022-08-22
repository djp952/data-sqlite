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

#ifndef __ZDBDELEGATES_H_
#define __ZDBDELEGATES_H_
#pragma once

#include "zDBEventArgs.h"				// Include zDB eventarg declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

using namespace System;

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// Delegate zDBAuthorizeEventHandler
//
// Used by zDBConnection to raise a hooked authorization event
public delegate void zDBAuthorizeEventHandler(Object^ sender, zDBAuthorizeEventArgs^ args);

//---------------------------------------------------------------------------
// Delegate zDBCollationNeededEventHandler
//
// Used by zDBConnection to raise a hooked collation needed event
public delegate void zDBCollationNeededEventHandler(Object^ sender, zDBCollationNeededEventArgs^ args);

//---------------------------------------------------------------------------
// Delegate zDBProfileEventHandler
//
// Used by zDBConnection to raise a hooked profile message event
public delegate void zDBProfileEventHandler(Object^ sender, zDBProfileEventArgs^ args);

//---------------------------------------------------------------------------
// Delegate zDBRowChangedEventHandler
//
// Used by zDBConnection to raise a hooked DELETE/INSERT/UPDATE event
public delegate void zDBRowChangedEventHandler(Object^ sender, zDBRowChangedEventArgs^ args);

//---------------------------------------------------------------------------
// Delegate zDBRowUpdatingEventHandler
//
// Used by zDBDataAdapter to raise an event
public delegate void zDBRowUpdatingEventHandler(Object^ sender, zDBRowUpdatingEventArgs^ args);

//---------------------------------------------------------------------------
// Delegate zDBRowUpdatedEventHandler
//
// Used by zDBDataAdapter to raise an event
public delegate void zDBRowUpdatedEventHandler(Object^ sender, zDBRowUpdatedEventArgs^ args);

//---------------------------------------------------------------------------
// Delegate zDBTraceEventHandler
//
// Used by zDBConnection to raise a hooked trace message event
public delegate void zDBTraceEventHandler(Object^ sender, zDBTraceEventArgs^ args);

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)

#endif		// __ZDBDELEGATES_H_
