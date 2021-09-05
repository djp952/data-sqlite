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

#ifndef __ZDBAGGREGATE_H_
#define __ZDBAGGREGATE_H_
#pragma once

#include "zDBArgumentCollection.h"		// Include zDBArgumentCollection decls
#include "zDBResult.h"					// Include zDBResult declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

using namespace System;
using namespace System::Data;

BEGIN_NAMESPACE(zuki)
BEGIN_NAMESPACE(data)
BEGIN_NAMESPACE(dbms)

//---------------------------------------------------------------------------
// Class zDBAggregate
//
// zDBAggregate is the base class that must be implemented by all custom
// aggregate function classes.  With this new provider, I've decided to
// make this work significantly differently than it used to.  Now you'll
// need to register a TYPE instead of an INSTANCE.  This lets the provider
// construct and destroy the aggregates as needed, which solves the problem
// of the derived class needing to maintain it's own state, which was kinda
// icky and I didn't like it much.
//
// Note: The ability for SQLite to count the number of rows on the aggregate's
// behalf has been deprecated and does not appear here.  The aggregate must
// keep track of that on it's own now.  Another good reason to switch to
// the type-based single instance methodology.
//---------------------------------------------------------------------------

public ref class zDBAggregate abstract
{
protected public:

	// Accumulate (must override)
	//
	// Called once per row to allow accumulation of the final result
	virtual void Accumulate(zDBArgumentCollection^ args) abstract;

	// GetResult (must override)
	//
	// Called when all rows have been processed to get the final result
	virtual void GetResult(zDBResult^ result) abstract;

private:

	// DESTRUCTOR (STUB)
	~zDBAggregate() {}
};

//---------------------------------------------------------------------------

END_NAMESPACE(dbms)
END_NAMESPACE(data)
END_NAMESPACE(zuki)

#pragma warning(pop)

#endif		// __ZDBAGGREGATE_H_
