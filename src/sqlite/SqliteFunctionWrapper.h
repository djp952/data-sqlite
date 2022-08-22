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

#ifndef __SQLITEFUNCTIONWRAPPER_H_
#define __SQLITEFUNCTIONWRAPPER_H_
#pragma once

#include "SqliteArgumentCollection.h"		// Include SqliteArgumentCollection decls
#include "SqliteFunction.h"				// Include SqliteFunction declarations
#include "SqliteResult.h"					// Include SqliteResult declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

using namespace System;

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// Forward Class Declarations
//---------------------------------------------------------------------------

ref class SqliteConnection;				// SqliteConnection.h

//---------------------------------------------------------------------------
// Class SqliteFunctionWrapper (internal)
//
// SqliteFunctionWrapper represents a wrapper around the main SqliteFunction delegate.
// This wrapper allows us to associate a SQLite database handle with the
// function so the connection context is known at the time of execution.
// This kinda slows things down a tad, but it represents a significant
// improvement over my 1.1 provider in that the format of boolean, date,
// and GUID results can be set automatically for the application.  That's
// really the ONLY purpose to this thing
//---------------------------------------------------------------------------

ref class SqliteFunctionWrapper
{
public:

	SqliteFunctionWrapper(SqliteFunction^ func) : m_func(func) {}

	//-----------------------------------------------------------------------
	// Member Functions

	// Invoke
	//
	// Invokes the contained delegate given the arguments provided
	void Invoke(sqlite3_context* context, int argc, sqlite3_value** argv);

	//-----------------------------------------------------------------------
	// Public Properties

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

	SqliteFunction^				m_func;			// Function delegate
	sqlite3*					m_hDatabase;	// Active database handle
};

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)

#endif		// __SQLITEFUNCTIONWRAPPER_H_
