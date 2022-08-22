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

#ifndef __SQLITEFACTORY_H_
#define __SQLITEFACTORY_H_
#pragma once

#include "SqliteCommand.h"					// Include SqliteCommand declarations
#include "SqliteCommandBuilder.h"			// Include SqliteCommandBuilder decls
#include "SqliteConnection.h"				// Include SqliteConnection declarations
#include "SqliteConnectionStringBuilder.h"	// Include SqliteConnectionStringBuilder
#include "SqliteDataAdapter.h"				// Include SqliteDataAdapter declarations
#include "SqliteDataSourceEnumerator.h"	// Include SqliteDataSourceEnumerator decls
#include "SqliteParameter.h"				// Include SqliteParameter decls
#include "SqlitePermission.h"				// Include SqlitePermission decls

#pragma warning(push, 4)				// Enable maximum compiler warnings

using namespace System;
using namespace System::Data;
using namespace System::Data::Common;
using namespace System::Security;
using namespace System::Security::Permissions;

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// Class SqliteFactory
//
// Represents a set of methods for creating instances of the zuki.data.dbms 
// provider's implementation of the data source classes. 
//---------------------------------------------------------------------------

public ref class SqliteFactory sealed : public DbProviderFactory
{
public:

	//-----------------------------------------------------------------------
	// Constructors

	SqliteFactory() {}

	//-----------------------------------------------------------------------
	// Member Functions

	// CreateCommand
	//
	// Creates a new SqliteCommand object instance
	SqliteCommand^ CreateCommand(void) new { return gcnew SqliteCommand(); }

	// CreateCommandBuilder
	//
	// Creates a new SqliteCommandBuilder object instance
	SqliteCommandBuilder^ CreateCommandBuilder(void) new { return gcnew SqliteCommandBuilder(); }

	// CreateConnection
	//
	// Creates a new SqliteConnection object instance
	SqliteConnection^ CreateConnection(void) new { return gcnew SqliteConnection(); }

	// CreateConnectionStringBuilder
	//
	// Creates a new SqliteConnection object instance
	SqliteConnectionStringBuilder^ CreateConnectionStringBuilder(void) new { return gcnew SqliteConnectionStringBuilder(); }

	// CreateDataAdapter
	//
	// Creates a new SqliteDataAdapter object instance
	SqliteDataAdapter^ CreateDataAdapter(void) new { return gcnew SqliteDataAdapter(); }

	// CreateDataSourceEnumerator
	//
	// Creates a new SqliteDataSourceEnumerator object instance
	SqliteDataSourceEnumerator^ CreateDataSourceEnumerator(void) new { return gcnew SqliteDataSourceEnumerator(); }

	// CreateGenericCommand (DbProviderFactory.CreateCommand)
	//
	// Creates a new SqliteCommand object instance on it's generic interface
	virtual DbCommand^ CreateGenericCommand(void) sealed 
		= DbProviderFactory::CreateCommand { return CreateCommand(); }

	// CreateGenericCommandBuilder (DbProviderFactory.CreateCommandBuilder)
	//
	// Creates a new SqliteCommandBuilder object instance on it's generic interface
	virtual DbCommandBuilder^ CreateGenericCommandBuilder(void) sealed 
		= DbProviderFactory::CreateCommandBuilder { return CreateCommandBuilder(); }

	// CreateGenericConnection (DbProviderFactory.CreateConnection)
	//
	// Creates a new SqliteConnection object instance on it's generic interface
	virtual DbConnection^ CreateGenericConnection(void) sealed 
		= DbProviderFactory::CreateConnection { return CreateConnection(); }

	// CreateGenericConnectionStringBuilder (DbProviderFactory.CreateConnectionStringBuilder)
	//
	// Creates a new SqliteConnection object instance on it's generic interface
	virtual DbConnectionStringBuilder^ CreateGenericConnectionStringBuilder(void) sealed 
		= DbProviderFactory::CreateConnectionStringBuilder { return CreateConnectionStringBuilder(); }

	// CreateGenericDataAdapter (DbProviderFactory.CreateDataAdapter)
	//
	// Creates a new SqliteDataAdapter object instance on it's generic interface
	virtual DbDataAdapter^ CreateGenericDataAdapter(void) sealed 
		= DbProviderFactory::CreateDataAdapter { return CreateDataAdapter(); }

	// CreateGenericDataSourceEnumerator (DbProviderFactory.CreateDataSourceEnumerator)
	//
	// Creates a new SqliteDataSourceEnumerator object instance on it's generic interface
	virtual DbDataSourceEnumerator^ CreateGenericDataSourceEnumerator(void) sealed 
		= DbProviderFactory::CreateDataSourceEnumerator { return CreateDataSourceEnumerator(); }

	// CreateGenericParameter (DbProviderFactory.CreateParameter)
	//
	// Creates a new SqliteParameter object instance on it's generic interface
	virtual DbParameter^ CreateGenericParameter(void) sealed 
		= DbProviderFactory::CreateParameter { return CreateParameter(); }

	// CreateGenericPermission (DbProviderFactory.CreatePermission)
	//
	// Creates a new SqlitePermission object instance on it's generic interface
	virtual CodeAccessPermission^ CreateGenericPermission(PermissionState state) sealed
		= DbProviderFactory::CreatePermission { return CreatePermission(state); }

	// CreateParameter
	//
	// Creates a new SqliteParameter object instance
	SqliteParameter^ CreateParameter(void) new { return gcnew SqliteParameter(); }

	// CreatePermission
	//
	// Creates a new SqlitePermission object instance
	SqlitePermission^ CreatePermission(PermissionState state) new { return gcnew SqlitePermission(state); }

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

#endif		// __SQLITEFACTORY_H_
