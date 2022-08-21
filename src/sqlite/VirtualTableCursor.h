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

#ifndef __VIRTUALTABLECURSOR_H_
#define __VIRTUALTABLECURSOR_H_
#pragma once

#include "zDBVirtualTableCursor.h"		// Include zDBVirtualTableCursor decls

using namespace System;
using namespace System::Diagnostics;
using namespace zuki::data::dbms;

#pragma warning(push, 4)				// Enable maximum compiler warnings

//---------------------------------------------------------------------------
// Class VirtualTableCursor
//
// VirtualTableCursor extends the standard sqlite3_vtab_cursor structure 
// into a full blown (unmanaged) class object.  This extension adds a pointer 
// to the GCHandle of the managed zDBVirtualTableCursor, and provides overloads 
// so it can be treated as if it were a managed zDBVirtualTableBase.  Handy.
//---------------------------------------------------------------------------

class VirtualTableCursor : public sqlite3_vtab_cursor
{
public:

	//-----------------------------------------------------------------------
	// Overloaded Operators

	// Instance
	operator Object^() const { return GCHandle::FromIntPtr(IntPtr(m_ptr)).Target; }
	operator zDBVirtualTableCursor^() const { return static_cast<zDBVirtualTableCursor^>(GCHandle::FromIntPtr(IntPtr(m_ptr)).Target); }
	
	// Pointer-To-Member
	zDBVirtualTableCursor^ operator ->() const { return operator zDBVirtualTableCursor^(); }

	// Equality
	bool operator ==(zDBVirtualTableCursor^ rhs) { return (operator zDBVirtualTableCursor^() == rhs); }

	// Inequality
	bool operator !=(zDBVirtualTableCursor^ rhs) { return (operator zDBVirtualTableCursor^() != rhs); }

	//-----------------------------------------------------------------------
	// Member Functions

	// Cast
	//
	// Casts an instance of VirtualTableCursor from a base sqlite3_vtab_cursor
	static VirtualTableCursor& Cast(sqlite3_vtab_cursor* pBase)
	{
		Debug::Assert(pBase != NULL);
		Debug::Assert(!IsBadReadPtr(pBase, sizeof(VirtualTableCursor)));

		return *reinterpret_cast<VirtualTableCursor*>(pBase);
	}

	// Create
	//
	// Creates an instance of VirtualTableCursor based on an object instance.  
	// A strong GCHandle is allocated for the object, so once this call completes 
	// there must be a call to Destroy() at some point to release it
	static VirtualTableCursor& Create(Object^ instance)
	{
		GCHandle			gchandle;		// Allocated object GCHandle
		VirtualTableCursor*	pInstance;		// Pointer to VirtualTableCursor

		if(instance == nullptr) throw gcnew ArgumentNullException();

		gchandle = GCHandle::Alloc(instance);	// Allocate the GCHandle
		
		// Attempt to create the VirtualTableCursor instance using the GCHandle.
		// Be sure to release it before throwing if something goes wrong

		pInstance = new VirtualTableCursor(GCHandle::ToIntPtr(gchandle).ToPointer());
		if(!pInstance) { gchandle.Free(); throw gcnew OutOfMemoryException(); }

		return *pInstance;					// Return VirtualTableCursor&
	}

	// Destroy
	//
	// Destroys an instance of VirtualTableCursor.  The underlying GCHandle will 
	// be released as well via the class destructor
	static void Destroy(VirtualTableCursor& instance)
	{
		delete &instance;
	}

	// SetError
	//
	// Sets an error message into the base sqlite3_vtab structure
	void SetError(String^ message)
	{
		if(pVtab->zErrMsg) sqlite3_free(pVtab->zErrMsg);	// Release as necessary
		pVtab->zErrMsg = NULL;								// Reset to NULL

		// Use sqlite3_mprintf() to assign a new error message to the
		// sqlite3_vtab structure's zErrMsg member ...

		pVtab->zErrMsg = sqlite3_mprintf(AutoAnsiString(message));
	}

	//-----------------------------------------------------------------------
	// Properties

	// RowPresent
	//
	// Boolean value representing if a row is present at the current cursor
	// location or not.  This is used to completely eliminate the xEof
	// callback from the managed implementation
	__declspec(property(get=GetRowPresent, put=SetRowPresent)) bool RowPresent;

	bool GetRowPresent(void) const { return m_row; }
	void SetRowPresent(bool value) { m_row = value; }

private:

	// PRIVATE CONSTRUCTOR / DESTRUCTOR
	explicit VirtualTableCursor(void* gchandle)
	{
		memset(this, 0, sizeof(VirtualTableCursor));	// NULL all members first
		m_ptr = gchandle;								// Copy in the specified handle
	}

	~VirtualTableCursor() { GCHandle::FromIntPtr(IntPtr(m_ptr)).Free(); }

	// DISABLED COPY CONSTRUCTOR / ASSIGNMENT OPERATOR
	VirtualTableCursor(const VirtualTableCursor& rhs);
	VirtualTableCursor& operator=(const VirtualTableCursor& rhs);

	//-----------------------------------------------------------------------
	// Member Variables

	void*						m_ptr;			// Pointer to managed object
	bool						m_row;			// Flag if a row is present
};

//---------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __VIRTUALTABLECURSOR_H_
