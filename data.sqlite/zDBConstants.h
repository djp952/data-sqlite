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

#ifndef __ZDBCONSTANTS_H_
#define __ZDBCONSTANTS_H_
#pragma once

#pragma warning(push, 4)				// Enable maximum compiler warnings

using namespace System;

BEGIN_NAMESPACE(zuki)
BEGIN_NAMESPACE(data)
BEGIN_NAMESPACE(dbms)

//---------------------------------------------------------------------------
// Class zDBDataSource
//
// Provides a list of constants for use with the Data Source= connection
// string property.  Pretty short list, too ..
//---------------------------------------------------------------------------

public ref struct zDBDataSource abstract sealed
{
	static initonly String^	Memory = ":memory:";
};

//---------------------------------------------------------------------------
// Class zDBMetaDataCollectionNames
//
// Provides a list of constants for all of the provider specific metadata
// collection names accessible from zDBConnection::GetSchema
//---------------------------------------------------------------------------

public ref struct zDBMetaDataCollectionNames abstract sealed
{
	static initonly String^		Collations			= "Collations";
	static initonly String^		Columns				= "Columns";
	static initonly String^		Databases			= "Databases";
	static initonly String^		ForeignKeys			= "ForeignKeys";
	static initonly String^		Indexes				= "Indexes";
	static initonly String^		IndexColumns		= "IndexColumns";
	static initonly String^		Tables				= "Tables";
	static initonly String^		Triggers			= "Triggers";
	static initonly String^		Views				= "Views";
	static initonly String^		ViewColumns			= "ViewColumns";
};

//---------------------------------------------------------------------------

END_NAMESPACE(dbms)
END_NAMESPACE(data)
END_NAMESPACE(zuki)

#pragma warning(pop)

#endif	// __ZDBCONSTANTS_H_
