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

#ifndef __ZDBFACTORY_H_
#define __ZDBFACTORY_H_
#pragma once

#include "zDBCommand.h"					// Include zDBCommand declarations
#include "zDBCommandBuilder.h"			// Include zDBCommandBuilder decls
#include "zDBConnection.h"				// Include zDBConnection declarations
#include "zDBConnectionStringBuilder.h"	// Include zDBConnectionStringBuilder
#include "zDBDataAdapter.h"				// Include zDBDataAdapter declarations
#include "zDBDataSourceEnumerator.h"	// Include zDBDataSourceEnumerator decls
#include "zDBParameter.h"				// Include zDBParameter decls
#include "zDBPermission.h"				// Include zDBPermission decls

#pragma warning(push, 4)				// Enable maximum compiler warnings

using namespace System;
using namespace System::Data;
using namespace System::Data::Common;
using namespace System::Security;
using namespace System::Security::Permissions;

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// Class zDBFactory
//
// Represents a set of methods for creating instances of the zuki.data.dbms 
// provider's implementation of the data source classes. 
//---------------------------------------------------------------------------

public ref class zDBFactory sealed : public DbProviderFactory
{
public:

	//-----------------------------------------------------------------------
	// Constructors

	zDBFactory() {}

	//-----------------------------------------------------------------------
	// Member Functions

	// CreateCommand
	//
	// Creates a new zDBCommand object instance
	zDBCommand^ CreateCommand(void) new { return gcnew zDBCommand(); }

	// CreateCommandBuilder
	//
	// Creates a new zDBCommandBuilder object instance
	zDBCommandBuilder^ CreateCommandBuilder(void) new { return gcnew zDBCommandBuilder(); }

	// CreateConnection
	//
	// Creates a new zDBConnection object instance
	zDBConnection^ CreateConnection(void) new { return gcnew zDBConnection(); }

	// CreateConnectionStringBuilder
	//
	// Creates a new zDBConnection object instance
	zDBConnectionStringBuilder^ CreateConnectionStringBuilder(void) new { return gcnew zDBConnectionStringBuilder(); }

	// CreateDataAdapter
	//
	// Creates a new zDBDataAdapter object instance
	zDBDataAdapter^ CreateDataAdapter(void) new { return gcnew zDBDataAdapter(); }

	// CreateDataSourceEnumerator
	//
	// Creates a new zDBDataSourceEnumerator object instance
	zDBDataSourceEnumerator^ CreateDataSourceEnumerator(void) new { return gcnew zDBDataSourceEnumerator(); }

	// CreateGenericCommand (DbProviderFactory.CreateCommand)
	//
	// Creates a new zDBCommand object instance on it's generic interface
	virtual DbCommand^ CreateGenericCommand(void) sealed 
		= DbProviderFactory::CreateCommand { return CreateCommand(); }

	// CreateGenericCommandBuilder (DbProviderFactory.CreateCommandBuilder)
	//
	// Creates a new zDBCommandBuilder object instance on it's generic interface
	virtual DbCommandBuilder^ CreateGenericCommandBuilder(void) sealed 
		= DbProviderFactory::CreateCommandBuilder { return CreateCommandBuilder(); }

	// CreateGenericConnection (DbProviderFactory.CreateConnection)
	//
	// Creates a new zDBConnection object instance on it's generic interface
	virtual DbConnection^ CreateGenericConnection(void) sealed 
		= DbProviderFactory::CreateConnection { return CreateConnection(); }

	// CreateGenericConnectionStringBuilder (DbProviderFactory.CreateConnectionStringBuilder)
	//
	// Creates a new zDBConnection object instance on it's generic interface
	virtual DbConnectionStringBuilder^ CreateGenericConnectionStringBuilder(void) sealed 
		= DbProviderFactory::CreateConnectionStringBuilder { return CreateConnectionStringBuilder(); }

	// CreateGenericDataAdapter (DbProviderFactory.CreateDataAdapter)
	//
	// Creates a new zDBDataAdapter object instance on it's generic interface
	virtual DbDataAdapter^ CreateGenericDataAdapter(void) sealed 
		= DbProviderFactory::CreateDataAdapter { return CreateDataAdapter(); }

	// CreateGenericDataSourceEnumerator (DbProviderFactory.CreateDataSourceEnumerator)
	//
	// Creates a new zDBDataSourceEnumerator object instance on it's generic interface
	virtual DbDataSourceEnumerator^ CreateGenericDataSourceEnumerator(void) sealed 
		= DbProviderFactory::CreateDataSourceEnumerator { return CreateDataSourceEnumerator(); }

	// CreateGenericParameter (DbProviderFactory.CreateParameter)
	//
	// Creates a new zDBParameter object instance on it's generic interface
	virtual DbParameter^ CreateGenericParameter(void) sealed 
		= DbProviderFactory::CreateParameter { return CreateParameter(); }

	// CreateGenericPermission (DbProviderFactory.CreatePermission)
	//
	// Creates a new zDBPermission object instance on it's generic interface
	virtual CodeAccessPermission^ CreateGenericPermission(PermissionState state) sealed
		= DbProviderFactory::CreatePermission { return CreatePermission(state); }

	// CreateParameter
	//
	// Creates a new zDBParameter object instance
	zDBParameter^ CreateParameter(void) new { return gcnew zDBParameter(); }

	// CreatePermission
	//
	// Creates a new zDBPermission object instance
	zDBPermission^ CreatePermission(PermissionState state) new { return gcnew zDBPermission(state); }

	//-----------------------------------------------------------------------
	// Properties

	// CanCreateDataSourceEnumerator (DbProviderFactory)
	//
	// Determines if this factory is capable of producing a data source enumerator
	virtual property bool CanCreateDataSourceEnumerator
	{
		bool get(void) override { return true; }
	}
};

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)

#endif		// __ZDBFACTORY_H_
