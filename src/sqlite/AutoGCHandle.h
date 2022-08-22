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

#ifndef __AUTOGCHANDLE_H_
#define __AUTOGCHANDLE_H_
#pragma once

using namespace System;
using namespace System::Runtime::InteropServices;

#pragma warning(push, 4)				// Enable maximum compiler warnings

//---------------------------------------------------------------------------
// Class AutoGCHandle
//
// AutoGCHandle implements a simple System::Object->GCHandle conversion
// class. There are times when a quick GCHandle is required to pass a managed
// object into a C callback as a void pointer, that's what this is for.  As
// soon as this class unwinds, the GCHandle will be released, so use carefully
//---------------------------------------------------------------------------

class AutoGCHandle
{
public:

	//-----------------------------------------------------------------------
	// Constructor
	//
	// Arguments:
	//
	//	object		- Managed object to be wrapped in a GCHandle temporarily

	explicit AutoGCHandle(Object^ object) :
		m_ptr(reinterpret_cast<intptr_t>(GCHandle::ToIntPtr(GCHandle::Alloc(object)).ToPointer())) {}

	//-----------------------------------------------------------------------
	// Destructor
	//
	// Automatically frees the allocated GCHandle reference

	~AutoGCHandle() { GCHandle::FromIntPtr(IntPtr(m_ptr)).Free(); }

	//-----------------------------------------------------------------------
	// Overloaded Operators

	operator const void*() const { return reinterpret_cast<void*>(m_ptr); }
	operator void*() const { return reinterpret_cast<void*>(m_ptr); }

private:

	AutoGCHandle(const AutoGCHandle& rhs);
	AutoGCHandle& operator=(const AutoGCHandle& rhs);

	//-----------------------------------------------------------------------
	// Member Variables

	intptr_t				m_ptr;			// Pointer to managed buffer
};

//---------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __AUTOGCHANDLE_H_
