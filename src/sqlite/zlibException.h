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

#ifndef __ZLIBEXCEPTION_H_
#define __ZLIBEXCEPTION_H_
#pragma once

#pragma warning(push, 4)				// Enable maximum compiler warnings

using namespace System;

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// Class zlibException
//
// zlibException is the generic exception thrown when a ZLIB engine error
// has occurred
//---------------------------------------------------------------------------

public ref class zlibException sealed : public Exception
{
internal:

	//-----------------------------------------------------------------------
	// Constructor
	//
	// Arguments:
	//
	//	nResult		- The ZLIB error code returned

	zlibException(int nResult) : Exception(GenerateMessage(nResult)) {}

	//-----------------------------------------------------------------------
	// Constructor
	//
	// Arguments:
	//
	//	nResult		- The ZLIB error code returned
	//	context		- A context message to append at the end

	zlibException(int nResult, String^ context)
		: Exception(String::Format("{0}\r\n\r\nContext:\r\n{1}", GenerateMessage(nResult), context)) {}
	
private:
	
	//-----------------------------------------------------------------------
	// Private Member Functions

	static String^ GenerateMessage(int nResult)
	{
		return String::Format("ZLIB Compression Error {0}: {1}", nResult, 
			Marshal::PtrToStringAnsi(IntPtr(const_cast<char*>(zError(nResult)))));
	}
};

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)

#endif	// __ZLIBEXCEPTION_H_
