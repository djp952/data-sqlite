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

#ifndef __SQLITECOMMANDBUILDER_H_
#define __SQLITECOMMANDBUILDER_H_
#pragma once

#include "SqliteCommand.h"					// Include SqliteCommand declarations
#include "SqliteDataAdapter.h"				// Include SqliteDataAdapter declarations
#include "SqliteDelegates.h"				// Include Sqlite delegate declarations
#include "SqliteEventArgs.h"				// Include Sqlite eventarg declarations
#include "SqliteExceptions.h"				// Include Sqlite exception declarations
#include "SqliteParameter.h"				// Include SqliteParameter declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

using namespace System;
using namespace System::Data;
using namespace System::Data::Common;
using namespace System::Globalization;

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// Class SqliteCommandBuilder
//
// SqliteCommandBuilder implements the command builder object for the ADO.NET
// provider.  I skipped making one of these the first time, gotta do it now.
//---------------------------------------------------------------------------

public ref class SqliteCommandBuilder sealed : public DbCommandBuilder
{
public:

	//-----------------------------------------------------------------------
	// Constructors

	SqliteCommandBuilder();
	SqliteCommandBuilder(SqliteDataAdapter^ adapter);

	//-----------------------------------------------------------------------
	// Member Functions

	// GetDeleteCommand
	//
	// Gets the automatically generated DELETE command
	SqliteCommand^ GetDeleteCommand(void) new { return safe_cast<SqliteCommand^>(__super::GetDeleteCommand()); }
	SqliteCommand^ GetDeleteCommand(bool useColumnsForParameterNames) new
	{ 
		return safe_cast<SqliteCommand^>(__super::GetDeleteCommand(useColumnsForParameterNames)); 
	}

	// GetInsertCommand
	//
	// Gets the automatically generated INSERT command
	SqliteCommand^ GetInsertCommand(void) new  { return safe_cast<SqliteCommand^>(__super::GetInsertCommand()); }
	SqliteCommand^ GetInsertCommand(bool useColumnsForParameterNames) new
	{ 
		return safe_cast<SqliteCommand^>(__super::GetInsertCommand(useColumnsForParameterNames)); \
	}

	// GetUpdateCommand
	//
	// Gets the automatically generated UPDATE command
	SqliteCommand^ GetUpdateCommand(void) new  { return safe_cast<SqliteCommand^>(__super::GetUpdateCommand()); }
	SqliteCommand^ GetUpdateCommand(bool useColumnsForParameterNames) new
	{ 
		return safe_cast<SqliteCommand^>(__super::GetUpdateCommand(useColumnsForParameterNames)); 
	}

	// QuoteIdenfifier (DbCommandBuilder)
	//
	// Quotes the specified identifier with the currently set prefix and suffix
	virtual String^ QuoteIdentifier(String^ unquoted) override;

	// UnquoteIdentifier (DbCommandBuilder)
	//
	// Removes the quotes from the specified identifier
	virtual String^ UnquoteIdentifier(String^ quoted) override;

	//-----------------------------------------------------------------------
	// Properties

	// CatalogLocation (DbCommandBuilder)
	//
	// Gets/sets the location of a catalog name in relation to a schema name
	virtual property Common::CatalogLocation CatalogLocation
	{
		Common::CatalogLocation get(void) override { return Common::CatalogLocation::Start; }
		void set(Common::CatalogLocation value) override;
	}

	// CatalogSeparator (DbCommandBuilder)
	//
	// Gets/Sets the string used to separate a catalog from a table/schema name
	virtual property String^ CatalogSeparator
	{
		String^ get(void) override { return "."; }
		void set(String^ value) override;
	}

	// DataAdapter
	//
	// Gets or sets the data adapter that this command builder is working with
	virtual property SqliteDataAdapter^ DataAdapter
	{
		SqliteDataAdapter^ get(void) new { return safe_cast<SqliteDataAdapter^>(__super::DataAdapter); }
		void set(SqliteDataAdapter^ value) { __super::DataAdapter = value; }
	}

	// QuotePrefix (DbCommandBuilder)
	//
	// Gets/Sets the quoted identifier prefix string
	virtual property String^ QuotePrefix
	{
		String^ get(void) override { return __super::QuotePrefix; }
		void set(String^ value) override;
	}

	// QuoteSuffix (DbCommandBuilder)
	//
	// Gets/Sets the quoted identifier suffix string
	virtual property String^ QuoteSuffix
	{
		String^ get(void) override { return __super::QuoteSuffix; }
		void set(String^ value) override;
	}

	// SchemaSeparator (DbCommandBuilder)
	//
	// Gets/Sets the string used to separate a schema from a table/catalog name
	virtual property String^ SchemaSeparator
	{
		String^ get(void) override { return "."; }
		void set(String^ value) override;
	}

protected:

	//-----------------------------------------------------------------------
	// Protected Member Functions

	// ApplyParameterInfo (DbCommandBuilder)
	//
	// Allows the command builder to handle additional parameter properties
	virtual void ApplyParameterInfo(DbParameter^ param, DataRow^ row, StatementType type, bool whereClause) override;

	// GetParameterName (DbCommandBuilder)
	//
	// Gets the name of the specified parameter
	virtual String^ GetParameterName(int ordinal) override { return ":p" + ordinal.ToString(); }
	virtual String^ GetParameterName(String^ name) override { return ":" + name; }

	// GetParameterPlaceholder (DbCommandBuilder)
	//
	// Returns the placeholder for the specified parameter
	virtual String^ GetParameterPlaceholder(int ordinal) override { return GetParameterName(ordinal); }

	// SetRowUpdatingEventHandler (DbCommandBuilder)
	//
	// Registers this DbCommandBuilder with the adapter's RowUpdating event
	virtual void SetRowUpdatingHandler(DbDataAdapter^ adapter) override;

private:

	//-----------------------------------------------------------------------
	// Private Member Functions

	// OnRowUpdating
	//
	// Handler for the provider-specific handler installed by SetRowUpdatingHandler
	void OnRowUpdating(Object^ sender, SqliteRowUpdatingEventArgs^ args)
	{
		UNREFERENCED_PARAMETER(sender);
		__super::RowUpdatingHandler(args);
	}
};

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)

#endif		// __SQLITECOMMANDBUILDER_H_
