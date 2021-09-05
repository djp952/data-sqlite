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

#ifndef __ZDBAGGREGATEWRAPPER_H_
#define __ZDBAGGREGATEWRAPPER_H_
#pragma once

#pragma warning(push, 4)				// Enable maximum compiler warnings

using namespace System;

BEGIN_NAMESPACE(zuki)
BEGIN_NAMESPACE(data)
BEGIN_NAMESPACE(dbms)

//---------------------------------------------------------------------------
// Class zDBAggregateWrapper (internal)
//
// zDBAggregateWrapper represents a wrapper around a registered aggregate
// class type.  This allows us to associate a SQLite database handle with
// the aggregate so the connection context is known at execution time.
// This kinda slows things down a tad, but it represents a significant
// improvement over my 1.1 provider in that the format of boolean, date,
// and GUID results can be set automatically for the application.
//---------------------------------------------------------------------------

ref class zDBAggregateWrapper
{
public:

	zDBAggregateWrapper(Type^ aggregateType) : m_type(aggregateType) {}

	//-----------------------------------------------------------------------
	// Public Properties

	// AggregateType
	//
	// Gets the type to create an instance of to implement the aggregate
	property Type^ AggregateType { Type^ get(void) { return m_type; } }

	// DatabaseHandle
	//
	// Gets/sets the database handle to associate with this function.  Changes
	// every time the connection is opened or closed, or at least that was the plan
	property sqlite3* DatabaseHandle
	{
		sqlite3* get(void) { return m_hDatabase; }
		void set(sqlite3* value) { m_hDatabase = value; }
	}

private:

	//-----------------------------------------------------------------------
	// Member Variables

	Type^						m_type;			// Aggregate class type
	sqlite3*					m_hDatabase;	// Active database handle
};

//---------------------------------------------------------------------------

END_NAMESPACE(dbms)
END_NAMESPACE(data)
END_NAMESPACE(zuki)

#pragma warning(pop)

#endif		// __ZDBAGGREGATEWRAPPER_H_
