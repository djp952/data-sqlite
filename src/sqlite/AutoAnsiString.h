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

#ifndef __AUTOANSISTRING_H_
#define __AUTOANSISTRING_H_
#pragma once

using namespace System;
using namespace System::Runtime::InteropServices;

#pragma warning(push, 4)				// Enable maximum compiler warnings

//---------------------------------------------------------------------------
// Class AutoAnsiString
//
// AutoAnsiString implements a simple System::String->ANSI String conversion
// class. There are a load of places in the code where we need to convert
// strings back and forth, and the try/finally blocks can quickly get out
// of hand.  This class ensures that the heap memory is released property
// without the need for any fancy unwind semantics
//---------------------------------------------------------------------------

class AutoAnsiString
{
public:

	//-----------------------------------------------------------------------
	// Constructor
	//
	// Arguments:
	//
	//	string		- Managed string to be converted into an ANSI string

	explicit AutoAnsiString(String^ string) : 
	  m_ptr(reinterpret_cast<intptr_t>(Marshal::StringToHGlobalAnsi(string).ToPointer())) {}

	//-----------------------------------------------------------------------
	// Destructor
	//
	// Automatically releases the allocated string buffer from the heap

	~AutoAnsiString() { Marshal::FreeHGlobal(IntPtr(m_ptr)); }

	//-----------------------------------------------------------------------
	// Overloaded Operators

	operator const char*() const { return reinterpret_cast<char*>(m_ptr); }

private:

	AutoAnsiString(const AutoAnsiString& rhs);
	AutoAnsiString& operator=(const AutoAnsiString& rhs);

	//-----------------------------------------------------------------------
	// Member Variables

	intptr_t				m_ptr;			// Pointer to managed buffer
};

//---------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __AUTOANSISTRING_H_
