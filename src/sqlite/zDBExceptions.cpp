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
#include "zDBExceptions.h"			// Include zDBExceptions declarations
#include "zDBParameter.h"			// Include zDBParameter declarations

#pragma warning(push, 4)			// Enable maximum compiler warnings

BEGIN_NAMESPACE(zuki)
BEGIN_NAMESPACE(data)
BEGIN_NAMESPACE(dbms)

//---------------------------------------------------------------------------
// zDBExceptions::BindingTransferException
//---------------------------------------------------------------------------

zDBExceptions::BindingTransferException::BindingTransferException(String^ sql, 
	sqlite3* hDatabase, int nResult) : zDBException(hDatabase, nResult, GenerateContext(sql)) {}

String^ zDBExceptions::BindingTransferException::GenerateContext(String^ sql)
{
	return String::Format("Transfering parameters for statement [{0}] after the "
		"statement was recompiled due to a schema change", sql);
}

//---------------------------------------------------------------------------
// zDBExceptions::CommandTypeUnknownException
//---------------------------------------------------------------------------

zDBExceptions::CommandTypeUnknownException::CommandTypeUnknownException(zDBCommandType value) :
	ArgumentException(GenerateMessage(value)) {}

String^ zDBExceptions::CommandTypeUnknownException::GenerateMessage(zDBCommandType value)
{
	return String::Format("The numeric value [{0}] does not represent a valid zDBCommandType", 
		static_cast<int>(value));
}

//---------------------------------------------------------------------------
// zDBExceptions::DbTypeUnknownException
//---------------------------------------------------------------------------

zDBExceptions::DbTypeUnknownException::DbTypeUnknownException(zDBType value) :
	ArgumentException(GenerateMessage(value)) {}

String^ zDBExceptions::DbTypeUnknownException::GenerateMessage(zDBType value)
{
	return String::Format("The numeric value [{0}] does not represent a valid zDBTypeCode", 
		static_cast<int>(value.Value));
}

//---------------------------------------------------------------------------
// zDBExceptions::InconsistentQuoteIdentifiersException
//---------------------------------------------------------------------------

zDBExceptions::InconsistentQuoteIdentifiersException::InconsistentQuoteIdentifiersException(String^ prefix, String^ suffix) :
	InvalidOperationException(GenerateMessage(prefix, suffix)) {}

String^ zDBExceptions::InconsistentQuoteIdentifiersException::GenerateMessage(String^ prefix, String^ suffix)
{
	return String::Format("The prefix string '{0}' and the suffix string '{1}' specified "
		"to generated quoted identifiers are inconsistent", prefix, suffix);
}

//---------------------------------------------------------------------------
// zDBExceptions::InvalidAggregateException
//---------------------------------------------------------------------------

zDBExceptions::InvalidAggregateException::InvalidAggregateException(Type^ aggregateType) :
	ArgumentException(GenerateMessage(aggregateType)) {}

String^ zDBExceptions::InvalidAggregateException::GenerateMessage(Type^ aggregateType)
{
	return String::Format("The type {0} does not derive from the zDBAggregate class and "
		"cannot be used to implement a aggregate function.", aggregateType->Name);
}

//---------------------------------------------------------------------------
// zDBExceptions::InvalidColumnNameException
//---------------------------------------------------------------------------

zDBExceptions::InvalidColumnNameException::InvalidColumnNameException(String^ name) :
	ArgumentException(GenerateMessage(name)) {}

String^ zDBExceptions::InvalidColumnNameException::GenerateMessage(String^ name)
{
	return String::Format("The name [{0}] is not a valid column name.", name);
}

//---------------------------------------------------------------------------
// zDBExceptions::InvalidVirtualTableException
//---------------------------------------------------------------------------

zDBExceptions::InvalidVirtualTableException::InvalidVirtualTableException(Type^ tableType) :
	ArgumentException(GenerateMessage(tableType)) {}

String^ zDBExceptions::InvalidVirtualTableException::GenerateMessage(Type^ tableType)
{
	return String::Format("The type {0} does not derive from the zDBVirtualTable<Cursor> generic "
		"class and cannot be used to implement a database virtual table.", tableType->Name);
}

//---------------------------------------------------------------------------
// zDBExceptions::NoDataPresentException
//---------------------------------------------------------------------------

zDBExceptions::NoDataPresentException::NoDataPresentException() :
	InvalidOperationException("Invalid attempt to read when no data is present") {}

//---------------------------------------------------------------------------
// zDBExceptions::OpenReaderException
//---------------------------------------------------------------------------

zDBExceptions::OpenReaderException::OpenReaderException() :
	InvalidOperationException("This operation cannot be performed while "
		"there is an open data reader against this command object") {}

//---------------------------------------------------------------------------
// zDBExceptions::ParameterBindingException
//---------------------------------------------------------------------------

zDBExceptions::ParameterBindingException::ParameterBindingException(zDBParameter^ param, int index, 
	String^ reason) : zDBException(GenerateMessage(param, index, reason)) {}

zDBExceptions::ParameterBindingException::ParameterBindingException(zDBParameter^ param, int index, 
	sqlite3* hDatabase, int nResult) : zDBException(hDatabase, nResult, GenerateContext(param, index)) {}

String^ zDBExceptions::ParameterBindingException::GenerateContext(zDBParameter^ param, int index)
{
	if(param->IsUnnamed) return String::Format("Binding unnamed parameter at index [{0}]", index);
	else return String::Format("Binding named parameter [{0}]", param->ParameterName);
}

String^ zDBExceptions::ParameterBindingException::GenerateMessage(zDBParameter^ param, int index, String^ reason)
{
	return String::Format("Unable to bind {0} [{1}]\r\n{2}",
		param->IsUnnamed ? "unnamed parameter at index" : "named parameter",
		param->IsUnnamed ? index.ToString() : param->ParameterName, reason);
}

//---------------------------------------------------------------------------
// zDBExceptions::StatementRecompileException
//---------------------------------------------------------------------------

zDBExceptions::StatementRecompileException::StatementRecompileException(String^ sql, 
	sqlite3* hDatabase, int nResult) : zDBException(hDatabase, nResult, GenerateContext(sql)) {}

String^ zDBExceptions::StatementRecompileException::GenerateContext(String^ sql)
{
	return String::Format("Recompiling statement [{0}] due to schema change", sql);
}

//---------------------------------------------------------------------------
// zDBExceptions::StatementStepException
//---------------------------------------------------------------------------

zDBExceptions::StatementStepException::StatementStepException(String^ sql, 
	sqlite3* hDatabase, int nResult) : zDBException(hDatabase, nResult, GenerateContext(sql)) {}

String^ zDBExceptions::StatementStepException::GenerateContext(String^ sql)
{
	return String::Format("Executing statement [{0}]", sql);
}

//---------------------------------------------------------------------------
// zDBExceptions::UpdateRowSourceUnknownException
//---------------------------------------------------------------------------

zDBExceptions::UpdateRowSourceUnknownException::UpdateRowSourceUnknownException(zDBUpdateRowSource value) :
	ArgumentException(GenerateMessage(value)) {}

String^ zDBExceptions::UpdateRowSourceUnknownException::GenerateMessage(zDBUpdateRowSource value)
{
	return String::Format("The numeric value [{0}] does not represent a valid zDBUpdateRowSource", 
		static_cast<int>(value));
}

//---------------------------------------------------------------------------

END_NAMESPACE(dbms)
END_NAMESPACE(data)
END_NAMESPACE(zuki)

#pragma warning(pop)
