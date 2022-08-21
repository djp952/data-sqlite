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

#ifndef __GCHANDLEREF_H_
#define __GCHANDLEREF_H_
#pragma once

using namespace System;
using namespace System::Runtime::InteropServices;

#pragma warning(push, 4)				// Enable maximum compiler warnings

//---------------------------------------------------------------------------
// Template Class GCHandleRef<T>
//
// GCHandleRef<T> implements a mechanism of making the conversion from a void*
// or an intptr_t back into an object reference less intrusive to the code.
// This modules does a great deal of low level GCHandle work, and it does
// get rather ugly now and again.  (The built-in gcroot<> template doesn't
// let you do this particular operation)
//---------------------------------------------------------------------------

template<class T>
class GCHandleRef
{
public:

	//-----------------------------------------------------------------------
	// Constructor
	//
	// Arguments:
	//
	//	pv		- A const void pointer representing a serialized GCHandle

	explicit GCHandleRef(const void *pv) : m_ptr(reinterpret_cast<intptr_t>(pv)) {}

	//-----------------------------------------------------------------------
	// Constructor
	//
	// Arguments:
	//
	//	ptr		- An intptr_t representing a serialized GCHandle

	explicit GCHandleRef(intptr_t ptr) : m_ptr(ptr) {}

	//-----------------------------------------------------------------------
	// Overloaded Operators

	// Instance
	operator Object^() const { return GCHandle::FromIntPtr(IntPtr(m_ptr)).Target; }
	operator T() const { return static_cast<T>(GCHandle::FromIntPtr(IntPtr(m_ptr)).Target); }
	
	// Pointer-To-Member
	T operator ->() const { return operator T(); }

	// Equality
	bool operator ==(T rhs) { return (operator T() == rhs); }

	// Inequality
	bool operator !=(T rhs) { return (operator T() != rhs); }

	//-----------------------------------------------------------------------
	// Properties

	// IsAllocated
	//
	// Used with WEAK GCHandle references to make sure the object is allocated
	// before attempting to access it via the pointer-to-member operator
	__declspec(property(get=GetIsAllocated))	bool IsAllocated;

	bool GetIsAllocated(void) const { return GCHandle::FromIntPtr(IntPtr(m_ptr)).IsAllocated; }

private:

	GCHandleRef(const GCHandleRef& rhs);
	GCHandleRef& operator=(const GCHandleRef& rhs);

	//-----------------------------------------------------------------------
	// Member Variables

	intptr_t				m_ptr;			// Serialized GCHandle data
};

//---------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __GCHANDLEREF_H_
