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

#ifndef __ZDBDATASOURCEENUMERATOR_H_
#define __ZDBDATASOURCEENUMERATOR_H_
#pragma once

#pragma warning(push, 4)				// Enable maximum compiler warnings

using namespace System;
using namespace System::Data;
using namespace System::Data::Common;
using namespace System::IO;

BEGIN_NAMESPACE(zuki)
BEGIN_NAMESPACE(data)
BEGIN_NAMESPACE(dbms)

//---------------------------------------------------------------------------
// Class zDBDataSourceEnumerator
//
// Enumerates all of the SQLite databases present in a specified directory.
// When the specific constructor or the directory name property cannot be 
// accessed, this will only scan the current working directory.
//---------------------------------------------------------------------------

public ref class zDBDataSourceEnumerator sealed : public DbDataSourceEnumerator
{
public:

	//-----------------------------------------------------------------------
	// Constructors

	zDBDataSourceEnumerator();
	zDBDataSourceEnumerator(String^ path);

	//-----------------------------------------------------------------------
	// Member Functions

	// GetDataSources (DbDataSourceEnumerator)
	//
	// Returns a DataTable with all the enumerated data source information
	virtual DataTable^ GetDataSources(void) override;

	//-----------------------------------------------------------------------
	// Properties

	// Folder
	//
	// Gets/sets the file system path that is used for the enumeration
	property String^ Folder
	{
		String^ get(void) { return m_path; }
		void set(String^ value);
	}

private:

	// STATIC CONSTRUCTOR
	static zDBDataSourceEnumerator() { StaticConstruct(); }

	//-----------------------------------------------------------------------
	// Private Member Functions

	// IsDatabase
	//
	// Determines if the specified file is a database file or not
	static bool IsDatabase(String^ filename, System::Byte% writeVer, System::Byte% readVer);

	// StaticConstruct
	//
	// Acts on behalf of the static constructor to initialize static data
	static void StaticConstruct(void);

	//-----------------------------------------------------------------------
	// Member Variables

	String^					m_path;			// Path to be scanned for files
	static DataTable^		s_template;		// DataTable template instance
};

//---------------------------------------------------------------------------

END_NAMESPACE(dbms)
END_NAMESPACE(data)
END_NAMESPACE(zuki)

#pragma warning(pop)

#endif		// __ZDBDATASOURCEENUMERATOR_H_
