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
#include "ObjectTracker.h"			// Include ObjectTracker declarations

#pragma warning(push, 4)			// Enable maximum compiler warnings

//---------------------------------------------------------------------------
// ObjectTracker::Free (private)
//
// Releases the contained GCHandle and resets everything
//
// Arguments:
//
//	NONE

void ObjectTracker::Free(void)
{
	if(m_alloc) m_gchandle.Free();			// Release the GCHandle
	m_alloc = false;						// Reset the allocation flag
}

//---------------------------------------------------------------------------
// ObjectTracker::Instance::get
//
// Gets a reference to the contained ITrackableObject instance, or nullptr
// if the object is no longer alive

ITrackableObject^ ObjectTracker::Instance::get(void)
{
	CHECK_DISPOSED(m_disposed);
	if(!IsAlive) return nullptr;

	return safe_cast<ITrackableObject^>(m_gchandle.Target);
}

//---------------------------------------------------------------------------
// ObjectTracker::IsObjectAlive (static)
//
// Helper function to test an ITrackableObject outside of this class
//
// Arguments:
//
//	obj		- Object instance to be tested

bool ObjectTracker::IsObjectAlive(ITrackableObject^ obj)
{
	if(obj == nullptr) return false;		// Null ref == not alive
	return !obj->IsDisposed();				// Use disposal status
}

//---------------------------------------------------------------------------
// ObjectTracker::IsAlive::get
//
// Determines if the object referenced herein is still alive or not

bool ObjectTracker::IsAlive::get(void)
{
	CHECK_DISPOSED(m_disposed);

	if((!m_alloc) || (!m_gchandle.IsAllocated)) return false;
	return IsObjectAlive(dynamic_cast<ITrackableObject^>(m_gchandle.Target));
}

//---------------------------------------------------------------------------
// ObjectTracker::SetObject
//
// Assigns a new ITrackableObject to this tracker instance.  Any previously
// set instance will be released automatically

void ObjectTracker::SetObject(ITrackableObject^ instance)
{
	CHECK_DISPOSED(m_disposed);

	Free();
	m_gchandle = GCHandle::Alloc(instance, GCHandleType::Weak);
	m_alloc = true;
}

//---------------------------------------------------------------------------

#pragma warning(pop)
