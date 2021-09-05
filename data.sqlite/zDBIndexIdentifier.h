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

#ifndef __ZDBINDEXIDENTIFIER_H_
#define __ZDBINDEXIDENTIFIER_H_
#pragma once

#pragma warning(push, 4)				// Enable maximum compiler warnings

using namespace System;
using namespace System::Runtime::InteropServices;

BEGIN_NAMESPACE(zuki)
BEGIN_NAMESPACE(data)
BEGIN_NAMESPACE(dbms)

//---------------------------------------------------------------------------
// Class zDBIndexIdentifier
//
// zDBIndexIdentifier is a managed wrapper around the sqlite3_index_info's
// idxNum and idzStr values. This information is used with virtual tables,
// both when the xBestIndex callback is invoked to allow the virtual table to 
// select an index, and when the xFilter callback is invoked with that info
//---------------------------------------------------------------------------

public ref class zDBIndexIdentifier sealed
{
public:

	//-----------------------------------------------------------------------
	// Properties

	// Code
	//
	// Gets/Sets a user-defined code to identify the selected index
	property int Code
	{
		int get(void) { return m_code; }
		void set(int value) { m_code = value; }
	}

	// Description
	//
	// Gets/Sets a user-defined description to identify the selected index
	property String^ Description
	{
		String^ get(void) { return m_desc; }
		void set(String^ value) { m_desc = value; }
	}
internal:

	// INTERNAL CONSTRUCTOR
	zDBIndexIdentifier(int idxNum, const char* idxStr)
	{
		// There's nothing to stop the application from using a HUGE
		// string to identify the index, so don't use FastPtrToStringAnsi

		m_code = idxNum;
		m_desc = Marshal::PtrToStringAnsi(IntPtr(const_cast<char*>(idxStr)));
	}

private:

	//-----------------------------------------------------------------------
	// Member Variables

	int						m_code;					// idxNum
	String^					m_desc;					// idxStr
};

//---------------------------------------------------------------------------

END_NAMESPACE(dbms)
END_NAMESPACE(data)
END_NAMESPACE(zuki)

#pragma warning(pop)

#endif		// __ZDBINDEXIDENTIFIER_H_
