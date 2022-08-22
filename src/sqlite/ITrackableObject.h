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

#ifndef __ITRACKABLEOBJECT_H_
#define __ITRACKABLEOBJECT_H_
#pragma once

#pragma warning(push, 4)				// Enable maximum compiler warnings

using namespace System;

//---------------------------------------------------------------------------
// Interface ITrackableObject (internal)
//
// Interface that allows certain classes to keep track of other class
// instance lifetimes.  See zDBConnection and zDBCommand for places that 
// this gets used.
//---------------------------------------------------------------------------

interface class ITrackableObject : public IDisposable
{
	// IsDisposed
	//
	// Exposes an object's internal disposal state.  If the object is disposed,
	// it will be assumed that it's dead and doesn't need to be kept anymore
	bool IsDisposed(void);
};

//---------------------------------------------------------------------------

#pragma warning(pop)

#endif		// __ITRACKABLEOBJECT_H_
