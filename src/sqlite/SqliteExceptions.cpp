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
#include "SqliteExceptions.h"			// Include SqliteExceptions declarations
#include "SqliteParameter.h"			// Include SqliteParameter declarations

#pragma warning(push, 4)			// Enable maximum compiler warnings

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// SqliteExceptions::BindingTransferException
//---------------------------------------------------------------------------

SqliteExceptions::BindingTransferException::BindingTransferException(String^ sql, 
	sqlite3* hDatabase, int nResult) : SqliteException(hDatabase, nResult, GenerateContext(sql)) {}

String^ SqliteExceptions::BindingTransferException::GenerateContext(String^ sql)
{
	return String::Format("Transfering parameters for statement [{0}] after the "
		"statement was recompiled due to a schema change", sql);
}

//---------------------------------------------------------------------------
// SqliteExceptions::CommandTypeUnknownException
//---------------------------------------------------------------------------

SqliteExceptions::CommandTypeUnknownException::CommandTypeUnknownException(SqliteCommandType value) :
	ArgumentException(GenerateMessage(value)) {}

String^ SqliteExceptions::CommandTypeUnknownException::GenerateMessage(SqliteCommandType value)
{
	return String::Format("The numeric value [{0}] does not represent a valid SqliteCommandType", 
		static_cast<int>(value));
}

//---------------------------------------------------------------------------
// SqliteExceptions::DbTypeUnknownException
//---------------------------------------------------------------------------

SqliteExceptions::DbTypeUnknownException::DbTypeUnknownException(SqliteType value) :
	ArgumentException(GenerateMessage(value)) {}

String^ SqliteExceptions::DbTypeUnknownException::GenerateMessage(SqliteType value)
{
	return String::Format("The numeric value [{0}] does not represent a valid SqliteTypeCode", 
		static_cast<int>(value.Value));
}

//---------------------------------------------------------------------------
// SqliteExceptions::InconsistentQuoteIdentifiersException
//---------------------------------------------------------------------------

SqliteExceptions::InconsistentQuoteIdentifiersException::InconsistentQuoteIdentifiersException(String^ prefix, String^ suffix) :
	InvalidOperationException(GenerateMessage(prefix, suffix)) {}

String^ SqliteExceptions::InconsistentQuoteIdentifiersException::GenerateMessage(String^ prefix, String^ suffix)
{
	return String::Format("The prefix string '{0}' and the suffix string '{1}' specified "
		"to generated quoted identifiers are inconsistent", prefix, suffix);
}

//---------------------------------------------------------------------------
// SqliteExceptions::InvalidAggregateException
//---------------------------------------------------------------------------

SqliteExceptions::InvalidAggregateException::InvalidAggregateException(Type^ aggregateType) :
	ArgumentException(GenerateMessage(aggregateType)) {}

String^ SqliteExceptions::InvalidAggregateException::GenerateMessage(Type^ aggregateType)
{
	return String::Format("The type {0} does not derive from the SqliteAggregate class and "
		"cannot be used to implement a aggregate function.", aggregateType->Name);
}

//---------------------------------------------------------------------------
// SqliteExceptions::InvalidColumnNameException
//---------------------------------------------------------------------------

SqliteExceptions::InvalidColumnNameException::InvalidColumnNameException(String^ name) :
	ArgumentException(GenerateMessage(name)) {}

String^ SqliteExceptions::InvalidColumnNameException::GenerateMessage(String^ name)
{
	return String::Format("The name [{0}] is not a valid column name.", name);
}

//---------------------------------------------------------------------------
// SqliteExceptions::InvalidVirtualTableException
//---------------------------------------------------------------------------

SqliteExceptions::InvalidVirtualTableException::InvalidVirtualTableException(Type^ tableType) :
	ArgumentException(GenerateMessage(tableType)) {}

String^ SqliteExceptions::InvalidVirtualTableException::GenerateMessage(Type^ tableType)
{
	return String::Format("The type {0} does not derive from the SqliteVirtualTable<Cursor> generic "
		"class and cannot be used to implement a database virtual table.", tableType->Name);
}

//---------------------------------------------------------------------------
// SqliteExceptions::NoDataPresentException
//---------------------------------------------------------------------------

SqliteExceptions::NoDataPresentException::NoDataPresentException() :
	InvalidOperationException("Invalid attempt to read when no data is present") {}

//---------------------------------------------------------------------------
// SqliteExceptions::OpenReaderException
//---------------------------------------------------------------------------

SqliteExceptions::OpenReaderException::OpenReaderException() :
	InvalidOperationException("This operation cannot be performed while "
		"there is an open data reader against this command object") {}

//---------------------------------------------------------------------------
// SqliteExceptions::ParameterBindingException
//---------------------------------------------------------------------------

SqliteExceptions::ParameterBindingException::ParameterBindingException(SqliteParameter^ param, int index, 
	String^ reason) : SqliteException(GenerateMessage(param, index, reason)) {}

SqliteExceptions::ParameterBindingException::ParameterBindingException(SqliteParameter^ param, int index, 
	sqlite3* hDatabase, int nResult) : SqliteException(hDatabase, nResult, GenerateContext(param, index)) {}

String^ SqliteExceptions::ParameterBindingException::GenerateContext(SqliteParameter^ param, int index)
{
	if(param->IsUnnamed) return String::Format("Binding unnamed parameter at index [{0}]", index);
	else return String::Format("Binding named parameter [{0}]", param->ParameterName);
}

String^ SqliteExceptions::ParameterBindingException::GenerateMessage(SqliteParameter^ param, int index, String^ reason)
{
	return String::Format("Unable to bind {0} [{1}]\r\n{2}",
		param->IsUnnamed ? "unnamed parameter at index" : "named parameter",
		param->IsUnnamed ? index.ToString() : param->ParameterName, reason);
}

//---------------------------------------------------------------------------
// SqliteExceptions::StatementRecompileException
//---------------------------------------------------------------------------

SqliteExceptions::StatementRecompileException::StatementRecompileException(String^ sql, 
	sqlite3* hDatabase, int nResult) : SqliteException(hDatabase, nResult, GenerateContext(sql)) {}

String^ SqliteExceptions::StatementRecompileException::GenerateContext(String^ sql)
{
	return String::Format("Recompiling statement [{0}] due to schema change", sql);
}

//---------------------------------------------------------------------------
// SqliteExceptions::StatementStepException
//---------------------------------------------------------------------------

SqliteExceptions::StatementStepException::StatementStepException(String^ sql, 
	sqlite3* hDatabase, int nResult) : SqliteException(hDatabase, nResult, GenerateContext(sql)) {}

String^ SqliteExceptions::StatementStepException::GenerateContext(String^ sql)
{
	return String::Format("Executing statement [{0}]", sql);
}

//---------------------------------------------------------------------------
// SqliteExceptions::UpdateRowSourceUnknownException
//---------------------------------------------------------------------------

SqliteExceptions::UpdateRowSourceUnknownException::UpdateRowSourceUnknownException(SqliteUpdateRowSource value) :
	ArgumentException(GenerateMessage(value)) {}

String^ SqliteExceptions::UpdateRowSourceUnknownException::GenerateMessage(SqliteUpdateRowSource value)
{
	return String::Format("The numeric value [{0}] does not represent a valid SqliteUpdateRowSource", 
		static_cast<int>(value));
}

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)
