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
#include "SqliteDataAdapter.h"			// Include SqliteDataAdapter declarations

#pragma warning(push, 4)			// Enable maximum compiler warnings

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// SqliteDataAdapter Constructor
//
// Arguments:
//
//	NONE

SqliteDataAdapter::SqliteDataAdapter()
{
	Disposed += gcnew EventHandler(this, &SqliteDataAdapter::OnDispose);
}

//---------------------------------------------------------------------------
// SqliteDataAdapter Constructor
//
// Arguments:
//
//	command		- SqliteCommand instance to apply to SelectCommand

SqliteDataAdapter::SqliteDataAdapter(SqliteCommand^ command)
{
	Disposed += gcnew EventHandler(this, &SqliteDataAdapter::OnDispose);
	SelectCommand = command;
}

//---------------------------------------------------------------------------
// SqliteDataAdapter Constructor
//
// Arguments:
//
//	commandText		- SQL command to initialize SelectCommand with
//	connection		- SqliteConnection instance to use

SqliteDataAdapter::SqliteDataAdapter(String^ commandText, SqliteConnection^ connection)
{
	Disposed += gcnew EventHandler(this, &SqliteDataAdapter::OnDispose);
	SelectCommand = gcnew SqliteCommand(commandText, connection);
}

//---------------------------------------------------------------------------
// SqliteDataAdapter Constructor
//
// Arguments:
//
//	commandText			- SQL command to initialize SelectCommand with
//	connectionString	- Connection string to initialize SelectCommand with

SqliteDataAdapter::SqliteDataAdapter(String^ commandText, String^ connectionString)
{
	Disposed += gcnew EventHandler(this, &SqliteDataAdapter::OnDispose);
	SelectCommand = gcnew SqliteCommand(commandText, gcnew SqliteConnection(connectionString));
}

//---------------------------------------------------------------------------
// SqliteDataAdapter::DeleteCommand::get
//
// Gets a reference to the contained DELETE command object

SqliteCommand^ SqliteDataAdapter::DeleteCommand::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return m_delete;
}

//---------------------------------------------------------------------------
// SqliteDataAdapter::DeleteCommand::set
//
// Changes the contained DELETE command object instance

void SqliteDataAdapter::DeleteCommand::set(SqliteCommand^ value)
{
	CHECK_DISPOSED(m_disposed);
	m_delete = value;
}

//---------------------------------------------------------------------------
// SqliteDataAdapter::InsertCommand::get
//
// Gets a reference to the contained INSERT command object

SqliteCommand^ SqliteDataAdapter::InsertCommand::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return m_insert;
}

//---------------------------------------------------------------------------
// SqliteDataAdapter::InsertCommand::set
//
// Changes the contained INSERT command object instance

void SqliteDataAdapter::InsertCommand::set(SqliteCommand^ value)
{
	CHECK_DISPOSED(m_disposed);
	m_insert = value;
}

//---------------------------------------------------------------------------
// SqliteDataAdapter::SelectCommand::get
//
// Gets a reference to the contained SELECT command object

SqliteCommand^ SqliteDataAdapter::SelectCommand::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return m_select;
}

//---------------------------------------------------------------------------
// SqliteDataAdapter::SelectCommand::set
//
// Changes the contained SELECT command object instance

void SqliteDataAdapter::SelectCommand::set(SqliteCommand^ value)
{
	CHECK_DISPOSED(m_disposed);
	m_select = value;
}

//---------------------------------------------------------------------------
// SqliteDataAdapter::UpdateCommand::get
//
// Gets a reference to the contained UPDATE command object

SqliteCommand^ SqliteDataAdapter::UpdateCommand::get(void)
{
	CHECK_DISPOSED(m_disposed);
	return m_update;
}

//---------------------------------------------------------------------------
// SqliteDataAdapter::UpdateCommand::set
//
// Changes the contained UPDATE command object instance

void SqliteDataAdapter::UpdateCommand::set(SqliteCommand^ value)
{
	CHECK_DISPOSED(m_disposed);
	m_update = value;
}

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)
