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

#ifndef __VIRTUALTABLE_H_
#define __VIRTUALTABLE_H_
#pragma once

#include "zDBVirtualTableBase.h"		// Include zDBVirtualTableBase decls

using namespace System;
using namespace System::Diagnostics;
using namespace zuki::data::dbms;

#pragma warning(push, 4)				// Enable maximum compiler warnings

//---------------------------------------------------------------------------
// Class VirtualTable
//
// VirtualTable extends the standard sqlite3_vtab structure into a full
// blown (unmanaged) class object.  This extension adds a pointer to the
// GCHandle of the managed zDBVirtualTable<>, and provides overloads so
// it can be treated as if it were a managed zDBVirtualTableBase.  Handy.
//---------------------------------------------------------------------------

class VirtualTable : public sqlite3_vtab
{
public:

	//-----------------------------------------------------------------------
	// Overloaded Operators

	// Instance
	operator Object^() const { return GCHandle::FromIntPtr(IntPtr(m_ptr)).Target; }
	operator zDBVirtualTableBase^() const { return static_cast<zDBVirtualTableBase^>(GCHandle::FromIntPtr(IntPtr(m_ptr)).Target); }
	
	// Pointer-To-Member
	zDBVirtualTableBase^ operator ->() const { return operator zDBVirtualTableBase^(); }

	// Equality
	bool operator ==(zDBVirtualTableBase^ rhs) { return (operator zDBVirtualTableBase^() == rhs); }

	// Inequality
	bool operator !=(zDBVirtualTableBase^ rhs) { return (operator zDBVirtualTableBase^() != rhs); }

	//-----------------------------------------------------------------------
	// Member Functions

	// Cast
	//
	// Casts an instance of VirtualTable from a base sqlite3_vtab pointer.
	static VirtualTable& Cast(sqlite3_vtab* pBase)
	{
		Debug::Assert(pBase != NULL);
		Debug::Assert(!IsBadReadPtr(pBase, sizeof(VirtualTable)));

		return *reinterpret_cast<VirtualTable*>(pBase);
	}

	// Create
	//
	// Creates an instance of VirtualTable based on an object instance.  A strong
	// GCHandle is allocated for the object, so once this call completes there
	// must be a call to Destroy() at some point to release it
	static VirtualTable& Create(Object^ instance)
	{
		GCHandle			gchandle;		// Allocated object GCHandle
		VirtualTable*		pInstance;		// Pointer to VirtualTable instance

		if(instance == nullptr) throw gcnew ArgumentNullException();

		gchandle = GCHandle::Alloc(instance);	// Allocate the GCHandle
		
		// Attempt to create the VirtualTable instance using the GCHandle.
		// Be sure to release it before throwing if something goes wrong

		pInstance = new VirtualTable(GCHandle::ToIntPtr(gchandle).ToPointer());
		if(!pInstance) { gchandle.Free(); throw gcnew OutOfMemoryException(); }

		return *pInstance;					// Return VirtualTable& reference
	}

	// Destroy
	//
	// Destroys an instance of VirtualTable.  The underlying GCHandle will be
	// released as well via the class destructor
	static void Destroy(VirtualTable& instance)
	{
		delete &instance;
	}

	// SetError
	//
	// Sets an error message into the base sqlite3_vtab structure
	void SetError(String^ message)
	{
		if(zErrMsg) sqlite3_free(zErrMsg);		// Release as necessary
		zErrMsg = NULL;							// Reset to NULL

		// Use sqlite3_mprintf() to assign a new error message to the
		// sqlite3_vtab structure's zErrMsg member ...

		zErrMsg = sqlite3_mprintf(AutoAnsiString(message));
	}

private:

	// PRIVATE CONSTRUCTOR / DESTRUCTOR
	explicit VirtualTable(void* gchandle)
	{
		memset(this, 0, sizeof(VirtualTable));		// NULL all members first
		m_ptr = gchandle;							// Copy in the specified handle
	}

	~VirtualTable() { GCHandle::FromIntPtr(IntPtr(m_ptr)).Free(); }

	// DISABLED COPY CONSTRUCTOR / ASSIGNMENT OPERATOR
	VirtualTable(const VirtualTable& rhs);
	VirtualTable& operator=(const VirtualTable& rhs);

	//-----------------------------------------------------------------------
	// Member Variables

	void*						m_ptr;			// Pointer to managed object
};

//---------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __VIRTUALTABLE_H_
