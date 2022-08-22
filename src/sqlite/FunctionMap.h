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

#ifndef __FUNCTIONMAP_H_
#define __FUNCTIONMAP_H_
#pragma once

#include <map>							// Include STL map<> declarations
#include <string>						// Include STL string<> declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

//---------------------------------------------------------------------------
// Class FunctionMapKey
//
// Used as the key type for the FunctionMap collection.  The Argument field
// means something different depending on if you're working with functions
// or collations.  With functions, this defines the number of arguments.  With
// collations, this is the encoding mode used when SQLite invokes the callback.
//---------------------------------------------------------------------------

struct FunctionMapKey
{
	FunctionMapKey(const wchar_t* name, int arg) : Name(name), Argument(arg) {}
	FunctionMapKey(const FunctionMapKey& rhs) : Name(rhs.Name), Argument(rhs.Argument) {}

	//-----------------------------------------------------------------------
	// Overloaded Operators

	FunctionMapKey& operator=(const FunctionMapKey& rhs)
	{
		Name = rhs.Name;
		Argument = rhs.Argument;
		return *this;
	}

	//-----------------------------------------------------------------------
	// Public Fields

	std::wstring	Name;				// Function name string
	int				Argument;			// Argument data (depends on context)
};

//---------------------------------------------------------------------------
// Class FunctionMapKeyComparer
//
// Used as the "less" binary function for STL collections
//---------------------------------------------------------------------------

struct FunctionMapKeyComparer
{
	bool operator ()(const FunctionMapKey& lhs, const FunctionMapKey& rhs) const 
	{
		// Use the string portion of the key as the main comparison, and only
		// when they are equal (case-insensitive), head over to the argument.
		
		int result = _wcsicmp(lhs.Name.c_str(), rhs.Name.c_str());
		if(result == 0) return (lhs.Argument < rhs.Argument);
		else return (result < 0);
	}
};

//---------------------------------------------------------------------------
// Typedef FunctionMap
//
// FunctionMap implements an STL map<> of FunctionMapKeys->intptr_t.  This
// is used as the collection basis in SqliteAggregateCollection, SqliteCollationCollection
// and SqliteFunctionCollection so that the class finalizers can reliably release
// any serialized GCHandles that still exist
//---------------------------------------------------------------------------

typedef std::map<FunctionMapKey, intptr_t, FunctionMapKeyComparer>	FunctionMap;
typedef FunctionMap::iterator										FunctionMapIterator;

//---------------------------------------------------------------------------

#pragma warning(pop)

#endif		// __FUNCTIONMAP_H_
