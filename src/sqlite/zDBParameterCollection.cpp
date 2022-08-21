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

#include "stdafx.h"						// Include project pre-compiled headers
#include "zDBParameterCollection.h"		// Include zDBParameterCollection decls
#include "zDBConnection.h"				// Include zDBConnection declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

BEGIN_NAMESPACE(zuki)
BEGIN_NAMESPACE(data)
BEGIN_NAMESPACE(dbms)

//---------------------------------------------------------------------------
// zDBParameterCollection::Add
//
// Inserts a new eDBParameter object into the collection
//
// Arguments:
//
//	param			- Reference to the zDBParameter object to be added

zDBParameter^ zDBParameterCollection::Add(zDBParameter^ param)
{
	TestAddParameter(param);			// Test add conditions

	m_col->Add(param);					// Insert into the collection
	param->Parent = this;				// Assign it's parent as us
	return param;						// Return back to the caller
}

//---------------------------------------------------------------------------
// zDBParameterCollection::AddRange
//
// Adds an array full of zDBParameter objects to the collection
//
// Arguments:
//
//	values		- Array of values to be added

void zDBParameterCollection::AddRange(array<zDBParameter^>^ values)
{
	// Validate that the contents of the array are going to work for this
	// collection first to avoid adding anything if it's just going to fail
	
	for each(zDBParameter^ param in values) TestAddParameter(param);
	for each(zDBParameter^ param in values) m_col->Add(param);
}

//---------------------------------------------------------------------------
// zDBParameterCollection::Clear
//
// Removes all items from the collection
//
// Arguments:
//
//	NONE

void zDBParameterCollection::Clear(void)
{
	if(m_locked) throw gcnew ParameterCollectionLockedException();

	for each(zDBParameter^ param in m_col) param->Parent = nullptr;
	m_col->Clear();
}

//---------------------------------------------------------------------------
// zDBParameterCollection::Contains
//
// Determines if a specific named parameter exists in this collection
//
// Arguments:
//
//	name			- zDBParameter.ParameterName to look for

bool zDBParameterCollection::Contains(String^ name)
{
	if(name == nullptr) throw gcnew ArgumentNullException();
	
	// Just walk the collection, looking for a zDBParameter with that name

	for each(zDBParameter^ param in m_col)
		if(String::Compare(param->ParameterName, name, true) == 0) return true;

	return false;				// The specified parameter name does not exist
}

//---------------------------------------------------------------------------
// zDBParameterCollection::GetNamedParameter (internal)
//
// Internal way of getting a specific named parameter, or NULL if there is
// no parameter with that name
//
// Arguments:
//
//	name		- Name of the parameter to locate in this collection

zDBParameter^ zDBParameterCollection::GetNamedParameter(String^ name)
{
	if(name == nullptr) throw gcnew ArgumentNullException();
	if(name->Length == 0) throw gcnew ArgumentException();

	// Just walk the collection, looking for a zDBParameter with that name

	for each(zDBParameter^ param in m_col)
		if(String::Compare(param->ParameterName, name, true) == 0) return param;

	return nullptr;					// The specified parameter does not exist
}

//---------------------------------------------------------------------------
// zDBParameterCollection::GetUnnamedParameter (internal)
//
// Easy way for the zDBStatement object to get an unnamed parameter object from
// this collection, or a NULL reference if none exists.  The unnamed parameters 
// are done by index.  The first unnamed parameter is index 0, the next index 1, etc.
//
// Arguments:
//
//	index		- Index of the unnamed parameter to locate in this collection
//	conn		- Reference to the connection object (for custom formats)

zDBParameter^ zDBParameterCollection::GetUnnamedParameter(int index)
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
// zDBParameterCollection::IndexOf
//
// Locates the index of a specific parameter in the collection, or -1
//
// Arguments:
//
//	name		- zDBParameter.ParameterName to locate in the collection

int zDBParameterCollection::IndexOf(String^ name)
{
	if(name == nullptr) throw gcnew ArgumentNullException();
	if(name->Length == 0) throw gcnew ArgumentException();

	// Just walk the collection to see if we can find this named parameter

	for(int index = 0; index < m_col->Count; index++)
		if(String::Compare(m_col[index]->ParameterName, name, true) == 0) return index;
				
	return -1;						// The requested object does not exist
}

//---------------------------------------------------------------------------
// zDBParameterCollection::Insert
//
// Inserts a new parameter object at the specified index
//
// Arguments:
//
//	index		- Collection index where the object gets inserted
//	param		- The eDBParameter object to be inserted

void zDBParameterCollection::Insert(int index, zDBParameter^ param)
{
	TestAddParameter(param);			// Test the parameter for inclusion
	m_col->Insert(index, param);		// Will throw if index out of range
	param->Parent = this;				// Assign the parent object property
}

//---------------------------------------------------------------------------
// zDBParameterCollection::Lock (internal)
//
// Locks the collection and all of the contained parameters so that nothing
// can be changed during the execution of a zDBCommand
//
// Arguments:
//
//	NONE

void zDBParameterCollection::Lock(void)
{
	Debug::Assert(m_locked == false);

	for each(zDBParameter^ param in m_col) param->Locked = true;
	m_locked = true;
}

//---------------------------------------------------------------------------
// zDBParameterCollection::Remove
//
// Removes a specific item from the collection
//
// Arguments:
//
//	param			- Reference to the object to be removed

void zDBParameterCollection::Remove(zDBParameter^ param)
{
	if(param == nullptr) throw gcnew ArgumentNullException();
	if(m_locked) throw gcnew ParameterCollectionLockedException();

	// Try to remove the parameter from the collection, and if we succeed
	// remove it's parent collection reference

	if(m_col->Remove(param)) param->Parent = nullptr;
}

//---------------------------------------------------------------------------
// zDBParameterCollection::RemoveAt
//
// Removes a specific item from the collection
//
// Arguments:
//
//	index		- Index of the item to be removed

void zDBParameterCollection::RemoveAt(int index)
{
	if((index < 0) || (index >= m_col->Count)) throw gcnew ArgumentOutOfRangeException();
	if(m_locked) throw gcnew ParameterCollectionLockedException();

	m_col[index]->Parent = nullptr;		// Will also throw if index is no good
	m_col->RemoveAt(index);
}

//---------------------------------------------------------------------------
// zDBParameterCollection::SetParameter (protected)
//
// Replaces an existing parameter in the collection.  The parameter cannot
// already exist in another collection slot
//
// Arguments:
//
//	index			- Index of the parameter to be set
//	param			- Reference to the zDBParameter to be set

void zDBParameterCollection::SetParameter(int index, DbParameter^ param)
{
	zDBParameter^			zparam;			// param as a zDBParameter
	int						existing;		// Index of existing parameter

	if((index < 0) || (index >= m_col->Count)) throw gcnew ArgumentOutOfRangeException();
	if(m_locked) throw gcnew ParameterCollectionLockedException();

	if(param == nullptr) return Remove(index);		// Remove on NULL
	zparam = ToParameter(param);					// Cast into zDBParameter

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
// zDBParameterCollection::SetParameter
//
// Replaces an existing named parameter in the collection.  The parameter 
// cannot already exist in another collection slot
//
// Arguments:
//
//	name			- Named parameter to be set
//	param			- Reference to the zDBParameter to be set

void zDBParameterCollection::SetParameter(String^ name, DbParameter^ param)
{
	zDBParameter^				zparam;			// param as a zDBParameter
	int							existing;		// Index of an existing parameter

	if(name == nullptr) throw gcnew ArgumentNullException();
	if(name->Length == 0) throw gcnew ArgumentException();
	if(m_locked) throw gcnew ParameterCollectionLockedException();

	// The key must always match the .ParameterName value of the parameter

	if(String::Compare(name, param->ParameterName, true) != 0)
		throw gcnew ParameterNameMismatchException(name, param);

	if(param == nullptr) return Remove(name);	// Remove on NULL
	zparam = ToParameter(param);				// Cast into zDBParameter

	existing = IndexOf(name);					// See if this already exists
	if(existing == -1) Add(zparam);				// New: add it as such
	else SetParameter(existing, zparam);		// Existing: replace it
}

//---------------------------------------------------------------------------
// zDBParameterCollection::TestAddParameter (private)
//
// Tests to see if a parameter can be added to this collection, and throws a
// generally appropriate exception if it can't
//
// Arguments:
//
//	param		- zDBParameter object to be tested

void zDBParameterCollection::TestAddParameter(zDBParameter^ param)
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
// zDBParameterCollection::ToParameter (private)
//
// Converts the specified object into a zDBParameter, if possible
//
// Arguments:
//
//	object		- Object to be converted

zDBParameter^ zDBParameterCollection::ToParameter(Object^ object)
{
	if(object == nullptr) throw gcnew ArgumentNullException();
	return safe_cast<zDBParameter^>(object);
}

//---------------------------------------------------------------------------
// zDBParameterCollection::ToParameterArray (private)
//
// Converts the specified object in a zDBParameter array, if possible
//
// Arguments:
//
//	object		- Object to be converted

array<zDBParameter^>^ zDBParameterCollection::ToParameterArray(Array^ object)
{
	if(object == nullptr) throw gcnew ArgumentNullException();
	return safe_cast<array<zDBParameter^>^>(object);
}

//---------------------------------------------------------------------------
// zDBParameterCollection::Unlock (internal)
//
// Unlocks the collection and all of the contained parameters
//
// Arguments:
//
//	NONE

void zDBParameterCollection::Unlock(void)
{
	Debug::Assert(m_locked == true);

	for each(zDBParameter^ param in m_col) param->Locked = false;
	m_locked = false;
}

//---------------------------------------------------------------------------

END_NAMESPACE(dbms)
END_NAMESPACE(data)
END_NAMESPACE(zuki)

#pragma warning(pop)
