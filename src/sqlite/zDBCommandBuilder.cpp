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

#include "stdafx.h"					// Include project pre-compiled headers
#include "zDBCommandBuilder.h"		// Include zDBCommandBuilder declarations

#pragma warning(push, 4)			// Enable maximum compiler warnings
#pragma warning(disable:4100)		// "unreferenced formal parameter"

BEGIN_NAMESPACE(zuki)
BEGIN_NAMESPACE(data)
BEGIN_NAMESPACE(dbms)

//---------------------------------------------------------------------------
// zDBCommandBuilder Constructor
//
// Arguments:
//
//	NONE

zDBCommandBuilder::zDBCommandBuilder()
{
	__super::QuotePrefix = "[";			// Default to using a square bracket
	__super::QuoteSuffix = "]";			// Default to using a square bracket
}

//---------------------------------------------------------------------------
// zDBCommandBuilder Constructor
//
// Arguments:
//
//	adapter		- Data adapter instance to automatically assign

zDBCommandBuilder::zDBCommandBuilder(zDBDataAdapter^ adapter)
{
	__super::QuotePrefix = "[";			// Default to using a square bracket
	__super::QuoteSuffix = "]";			// Default to using a square bracket
	DataAdapter = adapter;				// Assign the data adapter instance
}

//---------------------------------------------------------------------------
// zDBCommandBuilder::ApplyParameterInfo

void zDBCommandBuilder::ApplyParameterInfo(DbParameter^ param, DataRow^ row, 
	StatementType type, bool whereClause)
{
	zDBParameter^			paramObject;		// Parameter as zDBParameter
	Type^					paramType;			// Parameter data type
	zDBType					newDbType;			// New data type to be set

	paramObject = safe_cast<zDBParameter^>(param);
	paramType = safe_cast<Type^>(row[SchemaTableOptionalColumn::ProviderSpecificDataType]);

	newDbType = zDBType::FromType(paramType);
	if(newDbType != paramObject->DbType) paramObject->DbType = newDbType;

	// TODO: Precision and Scale, if I end up implementing them at all.
	// It's something I want to do, but not right away -- not that important
}

//---------------------------------------------------------------------------
// zDBCommandBuilder::CatalogLocation::set
//
// Changes the location where a catalog name appears relative to the schema name

void zDBCommandBuilder::CatalogLocation::set(Common::CatalogLocation value)
{
	if(value != Common::CatalogLocation::Start) throw gcnew ArgumentException();
}

//---------------------------------------------------------------------------
// zDBCommandBuilder::CatalogSeparator::set
//
// Changes the string used to separate out a catalog in a fully qualified name

void zDBCommandBuilder::CatalogSeparator::set(String^ value)
{
	if(value == nullptr) throw gcnew ArgumentNullException();
	if(value != ".") throw gcnew ArgumentException();
}

//---------------------------------------------------------------------------
// zDBCommandBuilder::QuoteIdentifier
//
// Converts an unquoted identifier into a quoted identifier
//
// Arguments:
//
//	unquoted		- The unquoted identifier string

String^ zDBCommandBuilder::QuoteIdentifier(String^ unquoted)
{
	String^					prefix;		// Quoted identifier prefix
	String^					suffix;		// Quoted identifier suffix

	prefix = QuotePrefix;				// Grab the prefix string
	suffix = QuoteSuffix;				// Grab the suffix string

	// First check to make sure the quote prefix and suffix are consistent.  The 
	// setters prevent them from ever being NULL or empty, so this should be fine

	if(((prefix == "\"") && (suffix != "\"")) || ((prefix == "[") && (suffix != "]")))
		throw gcnew zDBExceptions::InconsistentQuoteIdentifiersException(prefix, suffix);

	return String::Format("{0}{1}{2}", prefix, unquoted, suffix);
}

//---------------------------------------------------------------------------
// zDBCommandBuilder::QuotePrefix::set
//
// Changes the quoted identifier prefix string

void zDBCommandBuilder::QuotePrefix::set(String^ value)
{
	if(value == nullptr) throw gcnew ArgumentNullException();
	if((value != "\"") && (value != "[")) throw gcnew ArgumentException();
	__super::QuotePrefix = value;
}

//---------------------------------------------------------------------------
// zDBCommandBuilder::QuoteSuffix::set
//
// Changes the quoted identifier suffix string

void zDBCommandBuilder::QuoteSuffix::set(String^ value)
{
	if(value == nullptr) throw gcnew ArgumentNullException();
	if((value != "\"") && (value != "]")) throw gcnew ArgumentException();
	__super::QuoteSuffix = value;
}

//---------------------------------------------------------------------------
// zDBCommandBuilder::SchemaSeparator::set
//
// Changes the string used to separate out a schema in a fully qualified name

void zDBCommandBuilder::SchemaSeparator::set(String^ value)
{
	if(value == nullptr) throw gcnew ArgumentNullException();
	if(value != ".") throw gcnew ArgumentException();
}

//---------------------------------------------------------------------------
// zDBCommandBuilder::SetRowUpdatingHandler
//
// Registers a provider-specific version of a DataAdapter's row updating event
//
// Arguments:
//
//	adapter			- The DataAdapter instance to be registered with

void zDBCommandBuilder::SetRowUpdatingHandler(DbDataAdapter ^adapter)
{
	zDBDataAdapter^	da = safe_cast<zDBDataAdapter^>(adapter);	// Cast it

	// Per documentation, if the adapter is the same instance, it gets unregistered
	// rather than registered.  The SqlClient provider also does it exactly like this

	if(adapter == __super::DataAdapter) 
		da->RowUpdating -= gcnew zDBRowUpdatingEventHandler(this, &zDBCommandBuilder::OnRowUpdating);

	else da->RowUpdating += gcnew zDBRowUpdatingEventHandler(this, &zDBCommandBuilder::OnRowUpdating);
}

//---------------------------------------------------------------------------
// zDBCommandBuilder::UnquoteIdentifier
//
// Converts a quoted identifier into an unquoted identifier
//
// Arguments:
//
//	quoted			- The quoted identifier string

String^ zDBCommandBuilder::UnquoteIdentifier(String^ quoted)
{
	String^					prefix;		// Quoted identifier prefix
	String^					suffix;		// Quoted identifier suffix

	prefix = QuotePrefix;				// Grab the prefix string
	suffix = QuoteSuffix;				// Grab the suffix string

	// First check to make sure the quote prefix and suffix are consistent.  The 
	// setters prevent them from ever being NULL or empty, so this should be fine

	if(((prefix == "\"") && (suffix != "\"")) || ((prefix == "[") && (suffix != "]")))
		throw gcnew zDBExceptions::InconsistentQuoteIdentifiersException(prefix, suffix);

	if(quoted->StartsWith(prefix)) quoted = quoted->Substring(1);
	if(quoted->EndsWith(suffix)) quoted = quoted->Substring(0, quoted->Length - 1);

	return quoted;
}

//---------------------------------------------------------------------------

END_NAMESPACE(dbms)
END_NAMESPACE(data)
END_NAMESPACE(zuki)

#pragma warning(pop)
