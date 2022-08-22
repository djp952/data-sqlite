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

#include "stdafx.h"					// Include project pre-compiled headers
#include "SqliteCommandBuilder.h"		// Include SqliteCommandBuilder declarations

#pragma warning(push, 4)			// Enable maximum compiler warnings
#pragma warning(disable:4100)		// "unreferenced formal parameter"

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// SqliteCommandBuilder Constructor
//
// Arguments:
//
//	NONE

SqliteCommandBuilder::SqliteCommandBuilder()
{
	__super::QuotePrefix = "[";			// Default to using a square bracket
	__super::QuoteSuffix = "]";			// Default to using a square bracket
}

//---------------------------------------------------------------------------
// SqliteCommandBuilder Constructor
//
// Arguments:
//
//	adapter		- Data adapter instance to automatically assign

SqliteCommandBuilder::SqliteCommandBuilder(SqliteDataAdapter^ adapter)
{
	__super::QuotePrefix = "[";			// Default to using a square bracket
	__super::QuoteSuffix = "]";			// Default to using a square bracket
	DataAdapter = adapter;				// Assign the data adapter instance
}

//---------------------------------------------------------------------------
// SqliteCommandBuilder::ApplyParameterInfo

void SqliteCommandBuilder::ApplyParameterInfo(DbParameter^ param, DataRow^ row, 
	StatementType type, bool whereClause)
{
	SqliteParameter^			paramObject;		// Parameter as SqliteParameter
	Type^					paramType;			// Parameter data type
	SqliteType					newDbType;			// New data type to be set

	paramObject = safe_cast<SqliteParameter^>(param);
	paramType = safe_cast<Type^>(row[SchemaTableOptionalColumn::ProviderSpecificDataType]);

	newDbType = SqliteType::FromType(paramType);
	if(newDbType != paramObject->DbType) paramObject->DbType = newDbType;

	// TODO: Precision and Scale, if I end up implementing them at all.
	// It's something I want to do, but not right away -- not that important
}

//---------------------------------------------------------------------------
// SqliteCommandBuilder::CatalogLocation::set
//
// Changes the location where a catalog name appears relative to the schema name

void SqliteCommandBuilder::CatalogLocation::set(Common::CatalogLocation value)
{
	if(value != Common::CatalogLocation::Start) throw gcnew ArgumentException();
}

//---------------------------------------------------------------------------
// SqliteCommandBuilder::CatalogSeparator::set
//
// Changes the string used to separate out a catalog in a fully qualified name

void SqliteCommandBuilder::CatalogSeparator::set(String^ value)
{
	if(value == nullptr) throw gcnew ArgumentNullException();
	if(value != ".") throw gcnew ArgumentException();
}

//---------------------------------------------------------------------------
// SqliteCommandBuilder::QuoteIdentifier
//
// Converts an unquoted identifier into a quoted identifier
//
// Arguments:
//
//	unquoted		- The unquoted identifier string

String^ SqliteCommandBuilder::QuoteIdentifier(String^ unquoted)
{
	String^					prefix;		// Quoted identifier prefix
	String^					suffix;		// Quoted identifier suffix

	prefix = QuotePrefix;				// Grab the prefix string
	suffix = QuoteSuffix;				// Grab the suffix string

	// First check to make sure the quote prefix and suffix are consistent.  The 
	// setters prevent them from ever being NULL or empty, so this should be fine

	if(((prefix == "\"") && (suffix != "\"")) || ((prefix == "[") && (suffix != "]")))
		throw gcnew SqliteExceptions::InconsistentQuoteIdentifiersException(prefix, suffix);

	return String::Format("{0}{1}{2}", prefix, unquoted, suffix);
}

//---------------------------------------------------------------------------
// SqliteCommandBuilder::QuotePrefix::set
//
// Changes the quoted identifier prefix string

void SqliteCommandBuilder::QuotePrefix::set(String^ value)
{
	if(value == nullptr) throw gcnew ArgumentNullException();
	if((value != "\"") && (value != "[")) throw gcnew ArgumentException();
	__super::QuotePrefix = value;
}

//---------------------------------------------------------------------------
// SqliteCommandBuilder::QuoteSuffix::set
//
// Changes the quoted identifier suffix string

void SqliteCommandBuilder::QuoteSuffix::set(String^ value)
{
	if(value == nullptr) throw gcnew ArgumentNullException();
	if((value != "\"") && (value != "]")) throw gcnew ArgumentException();
	__super::QuoteSuffix = value;
}

//---------------------------------------------------------------------------
// SqliteCommandBuilder::SchemaSeparator::set
//
// Changes the string used to separate out a schema in a fully qualified name

void SqliteCommandBuilder::SchemaSeparator::set(String^ value)
{
	if(value == nullptr) throw gcnew ArgumentNullException();
	if(value != ".") throw gcnew ArgumentException();
}

//---------------------------------------------------------------------------
// SqliteCommandBuilder::SetRowUpdatingHandler
//
// Registers a provider-specific version of a DataAdapter's row updating event
//
// Arguments:
//
//	adapter			- The DataAdapter instance to be registered with

void SqliteCommandBuilder::SetRowUpdatingHandler(DbDataAdapter ^adapter)
{
	SqliteDataAdapter^	da = safe_cast<SqliteDataAdapter^>(adapter);	// Cast it

	// Per documentation, if the adapter is the same instance, it gets unregistered
	// rather than registered.  The SqlClient provider also does it exactly like this

	if(adapter == __super::DataAdapter) 
		da->RowUpdating -= gcnew SqliteRowUpdatingEventHandler(this, &SqliteCommandBuilder::OnRowUpdating);

	else da->RowUpdating += gcnew SqliteRowUpdatingEventHandler(this, &SqliteCommandBuilder::OnRowUpdating);
}

//---------------------------------------------------------------------------
// SqliteCommandBuilder::UnquoteIdentifier
//
// Converts a quoted identifier into an unquoted identifier
//
// Arguments:
//
//	quoted			- The quoted identifier string

String^ SqliteCommandBuilder::UnquoteIdentifier(String^ quoted)
{
	String^					prefix;		// Quoted identifier prefix
	String^					suffix;		// Quoted identifier suffix

	prefix = QuotePrefix;				// Grab the prefix string
	suffix = QuoteSuffix;				// Grab the suffix string

	// First check to make sure the quote prefix and suffix are consistent.  The 
	// setters prevent them from ever being NULL or empty, so this should be fine

	if(((prefix == "\"") && (suffix != "\"")) || ((prefix == "[") && (suffix != "]")))
		throw gcnew SqliteExceptions::InconsistentQuoteIdentifiersException(prefix, suffix);

	if(quoted->StartsWith(prefix)) quoted = quoted->Substring(1);
	if(quoted->EndsWith(suffix)) quoted = quoted->Substring(0, quoted->Length - 1);

	return quoted;
}

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)
