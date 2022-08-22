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

#ifndef __ZDBARGUMENTCOLLECTION_H_
#define __ZDBARGUMENTCOLLECTION_H_
#pragma once

#include "zDBArgument.h"				// Include zDBArgument declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Collections::ObjectModel;

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// Class zDBArgumentCollection
//
// zDBArgumentCollection represents a read-only collection of zDBArgument
// object instances.  Essentially just a typedef of ReadOnlyCollection<>
// with some constructors to make my own life slightly easier
//
// Note that arguments (or sqlite3_values, if you prefer) are designed to
// be volatile and must be properly disposed of when done working with them.
// This allows for the performance we need and prevents applications from
// sneaking references to the arguments out of context and accessing bad data
// long after the underlying SQLite pointers have disappeared.
//---------------------------------------------------------------------------

public ref class zDBArgumentCollection sealed : public ReadOnlyCollection<zDBArgument^>
{
internal:

	// INTERNAL CONSTRUCTORS
	zDBArgumentCollection(int argc, sqlite3_value** argv) : ReadOnlyCollection(MakeList(argc, argv)) {}

private:

	// DESTRUCTOR
	~zDBArgumentCollection() { for each(zDBArgument^ arg in this) delete arg; }

	//-----------------------------------------------------------------------
	// Private Member Functions

	// MakeList
	//
	// Creates a List<zDBArgument^>^ that can be used to back the underlying
	// ReadOnlyCollection.  Provided mainly for flexibility
	static List<zDBArgument^>^ MakeList(int argc, sqlite3_value** argv)
	{
		List<zDBArgument^>^ list = gcnew List<zDBArgument^>(argc);
		for(int index = 0; index < argc; index++) list->Add(gcnew zDBArgument(argv[index]));

		return list;				// Return the generated List<> object
	}
};

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)

#endif		// __ZDBARGUMENTCOLLECTION_H_
