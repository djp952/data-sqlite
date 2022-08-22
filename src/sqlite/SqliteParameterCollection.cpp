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

#include "stdafx.h"						// Include project pre-compiled headers
#include "SqliteParameterCollection.h"		// Include SqliteParameterCollection decls
#include "SqliteConnection.h"				// Include SqliteConnection declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// SqliteParameterCollection::Add
//
// Inserts a new eDBParameter object into the collection
//
// Arguments:
//
//	param			- Reference to the SqliteParameter object to be added

SqliteParameter^ SqliteParameterCollection::Add(SqliteParameter^ param)
{
	TestAddParameter(param);			// Test add conditions

	m_col->Add(param);					// Insert into the collection
	param->Parent = this;				// Assign it's parent as us
	return param;						// Return back to the caller
}

//---------------------------------------------------------------------------
// SqliteParameterCollection::AddRange
//
// Adds an array full of SqliteParameter objects to the collection
//
// Arguments:
//
//	values		- Array of values to be added

void SqliteParameterCollection::AddRange(array<SqliteParameter^>^ values)
{
	// Validate that the contents of the array are going to work for this
	// collection first to avoid adding anything if it's just going to fail
	
	for each(SqliteParameter^ param in values) TestAddParameter(param);
	for each(SqliteParameter^ param in values) m_col->Add(param);
}

//---------------------------------------------------------------------------
// SqliteParameterCollection::Clear
//
// Removes all items from the collection
//
// Arguments:
//
//	NONE

void SqliteParameterCollection::Clear(void)
{
	if(m_locked) throw gcnew ParameterCollectionLockedException();

	for each(SqliteParameter^ param in m_col) param->Parent = nullptr;
	m_col->Clear();
}

//---------------------------------------------------------------------------
// SqliteParameterCollection::Contains
//
// Determines if a specific named parameter exists in this collection
//
// Arguments:
//
//	name			- SqliteParameter.ParameterName to look for

bool SqliteParameterCollection::Contains(String^ name)
{
	if(name == nullptr) throw gcnew ArgumentNullException();
	
	// Just walk the collection, looking for a SqliteParameter with that name

	for each(SqliteParameter^ param in m_col)
		if(String::Compare(param->ParameterName, name, true) == 0) return true;

	return false;				// The specified parameter name does not exist
}

//---------------------------------------------------------------------------
// SqliteParameterCollection::GetNamedParameter (internal)
//
// Internal way of getting a specific named parameter, or NULL if there is
// no parameter with that name
//
// Arguments:
//
//	name		- Name of the parameter to locate in this collection

SqliteParameter^ SqliteParameterCollection::GetNamedParameter(String^ name)
{
	if(name == nullptr) throw gcnew ArgumentNullException();
	if(name->Length == 0) throw gcnew ArgumentException();

	// Just walk the collection, looking for a SqliteParameter with that name

	for each(SqliteParameter^ param in m_col)
		if(String::Compare(param->ParameterName, name, true) == 0) return param;

	return nullptr;					// The specified parameter does not exist
}

//---------------------------------------------------------------------------
// SqliteParameterCollection::GetUnnamedParameter (internal)
//
// Easy way for the SqliteStatement object to get an unnamed parameter object from
// this collection, or a NULL reference if none exists.  The unnamed parameters 
// are done by index.  The first unnamed parameter is index 0, the next index 1, etc.
//
// Arguments:
//
//	index		- Index of the unnamed parameter to locate in this collection
//	conn		- Reference to the connection object (for custom formats)

SqliteParameter^ SqliteParameterCollection::GetUnnamedParameter(int index)
{
	int					cUnnamed = 0;		// Unnamed parameters found
	
	if(index < 0) throw gcnew IndexOutOfRangeException();

	// Just walk the collection, looking for the nth unnamed parameter

	for(int paramIndex = 0; paramIndex < m_col->Count; paramIndex++) {

		if(m_col[index]->IsUnnamed) {
			
			if(cUnnamed == index) return m_col[index];
			cUnnamed ++;
		}
	}

	return nullptr;				// The specified parameter does not exist
}

//---------------------------------------------------------------------------
// SqliteParameterCollection::IndexOf
//
// Locates the index of a specific parameter in the collection, or -1
//
// Arguments:
//
//	name		- SqliteParameter.ParameterName to locate in the collection

int SqliteParameterCollection::IndexOf(String^ name)
{
	if(name == nullptr) throw gcnew ArgumentNullException();
	if(name->Length == 0) throw gcnew ArgumentException();

	// Just walk the collection to see if we can find this named parameter

	for(int index = 0; index < m_col->Count; index++)
		if(String::Compare(m_col[index]->ParameterName, name, true) == 0) return index;
				
	return -1;						// The requested object does not exist
}

//---------------------------------------------------------------------------
// SqliteParameterCollection::Insert
//
// Inserts a new parameter object at the specified index
//
// Arguments:
//
//	index		- Collection index where the object gets inserted
//	param		- The eDBParameter object to be inserted

void SqliteParameterCollection::Insert(int index, SqliteParameter^ param)
{
	TestAddParameter(param);			// Test the parameter for inclusion
	m_col->Insert(index, param);		// Will throw if index out of range
	param->Parent = this;				// Assign the parent object property
}

//---------------------------------------------------------------------------
// SqliteParameterCollection::Lock (internal)
//
// Locks the collection and all of the contained parameters so that nothing
// can be changed during the execution of a SqliteCommand
//
// Arguments:
//
//	NONE

void SqliteParameterCollection::Lock(void)
{
	Debug::Assert(m_locked == false);

	for each(SqliteParameter^ param in m_col) param->Locked = true;
	m_locked = true;
}

//---------------------------------------------------------------------------
// SqliteParameterCollection::Remove
//
// Removes a specific item from the collection
//
// Arguments:
//
//	param			- Reference to the object to be removed

void SqliteParameterCollection::Remove(SqliteParameter^ param)
{
	if(param == nullptr) throw gcnew ArgumentNullException();
	if(m_locked) throw gcnew ParameterCollectionLockedException();

	// Try to remove the parameter from the collection, and if we succeed
	// remove it's parent collection reference

	if(m_col->Remove(param)) param->Parent = nullptr;
}

//---------------------------------------------------------------------------
// SqliteParameterCollection::RemoveAt
//
// Removes a specific item from the collection
//
// Arguments:
//
//	index		- Index of the item to be removed

void SqliteParameterCollection::RemoveAt(int index)
{
	if((index < 0) || (index >= m_col->Count)) throw gcnew ArgumentOutOfRangeException();
	if(m_locked) throw gcnew ParameterCollectionLockedException();

	m_col[index]->Parent = nullptr;		// Will also throw if index is no good
	m_col->RemoveAt(index);
}

//---------------------------------------------------------------------------
// SqliteParameterCollection::SetParameter (protected)
//
// Replaces an existing parameter in the collection.  The parameter cannot
// already exist in another collection slot
//
// Arguments:
//
//	index			- Index of the parameter to be set
//	param			- Reference to the SqliteParameter to be set

void SqliteParameterCollection::SetParameter(int index, DbParameter^ param)
{
	SqliteParameter^			zparam;			// param as a SqliteParameter
	int						existing;		// Index of existing parameter

	if((index < 0) || (index >= m_col->Count)) throw gcnew ArgumentOutOfRangeException();
	if(m_locked) throw gcnew ParameterCollectionLockedException();

	if(param == nullptr) return Remove(index);		// Remove on NULL
	zparam = ToParameter(param);					// Cast into SqliteParameter

	existing = m_col->IndexOf(zparam);				// See if this already exists	
	if(index == existing) return;					// Already at this index .. later

	// If the parameter is part of another collection, it cannot be added in here

	if(zparam->Parent != nullptr) throw gcnew ParameterParentException(zparam->ParameterName);

	// If the parameter has a name, check to see if a parameter with that name
	// already exists in the collection.  It's OK to replace a named parameter
	// as long as the new one has the same name.

	if(!zparam->IsUnnamed) {

		existing = IndexOf(zparam->ParameterName);
		if((existing != -1) && (existing != index)) throw gcnew ParameterExistsException(zparam->ParameterName);
	}

	m_col[index]->Parent = nullptr;				// Remove parent reference
	m_col[index] = zparam;						// Set the new parameter
	zparam->Parent = this;						// Set the new parameter's parent
}

//---------------------------------------------------------------------------
// SqliteParameterCollection::SetParameter
//
// Replaces an existing named parameter in the collection.  The parameter 
// cannot already exist in another collection slot
//
// Arguments:
//
//	name			- Named parameter to be set
//	param			- Reference to the SqliteParameter to be set

void SqliteParameterCollection::SetParameter(String^ name, DbParameter^ param)
{
	SqliteParameter^				zparam;			// param as a SqliteParameter
	int							existing;		// Index of an existing parameter

	if(name == nullptr) throw gcnew ArgumentNullException();
	if(name->Length == 0) throw gcnew ArgumentException();
	if(m_locked) throw gcnew ParameterCollectionLockedException();

	// The key must always match the .ParameterName value of the parameter

	if(String::Compare(name, param->ParameterName, true) != 0)
		throw gcnew ParameterNameMismatchException(name, param);

	if(param == nullptr) return Remove(name);	// Remove on NULL
	zparam = ToParameter(param);				// Cast into SqliteParameter

	existing = IndexOf(name);					// See if this already exists
	if(existing == -1) Add(zparam);				// New: add it as such
	else SetParameter(existing, zparam);		// Existing: replace it
}

//---------------------------------------------------------------------------
// SqliteParameterCollection::TestAddParameter (private)
//
// Tests to see if a parameter can be added to this collection, and throws a
// generally appropriate exception if it can't
//
// Arguments:
//
//	param		- SqliteParameter object to be tested

void SqliteParameterCollection::TestAddParameter(SqliteParameter^ param)
{
	// NULL Reference
	if(param == nullptr) throw gcnew ArgumentNullException();

	// Locked collection
	if(m_locked) throw gcnew ParameterCollectionLockedException();

	// Part of another collection already
	if(param->Parent != nullptr) throw gcnew ParameterParentException(param->ParameterName);

	// Duplicate object exists
	if(m_col->Contains(param)) throw gcnew ParameterExistsException(param->ParameterName);

	// Duplicate name exists
	if((!param->IsUnnamed) && (Contains(param->ParameterName))) throw gcnew ParameterExistsException(param->ParameterName);
}

//---------------------------------------------------------------------------
// SqliteParameterCollection::ToParameter (private)
//
// Converts the specified object into a SqliteParameter, if possible
//
// Arguments:
//
//	object		- Object to be converted

SqliteParameter^ SqliteParameterCollection::ToParameter(Object^ object)
{
	if(object == nullptr) throw gcnew ArgumentNullException();
	return safe_cast<SqliteParameter^>(object);
}

//---------------------------------------------------------------------------
// SqliteParameterCollection::ToParameterArray (private)
//
// Converts the specified object in a SqliteParameter array, if possible
//
// Arguments:
//
//	object		- Object to be converted

array<SqliteParameter^>^ SqliteParameterCollection::ToParameterArray(Array^ object)
{
	if(object == nullptr) throw gcnew ArgumentNullException();
	return safe_cast<array<SqliteParameter^>^>(object);
}

//---------------------------------------------------------------------------
// SqliteParameterCollection::Unlock (internal)
//
// Unlocks the collection and all of the contained parameters
//
// Arguments:
//
//	NONE

void SqliteParameterCollection::Unlock(void)
{
	Debug::Assert(m_locked == true);

	for each(SqliteParameter^ param in m_col) param->Locked = false;
	m_locked = false;
}

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)
