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

#ifndef __ZDBPARAMETERCOLLECTION_H_
#define __ZDBPARAMETERCOLLECTION_H_
#pragma once

#include "zDBParameter.h"				// Include zDBParameter declarations
#include "zDBExceptions.h"				// Include zDB exception decls
#include "zDBType.h"					// Include zDBType declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

using namespace System;
using namespace System::Collections;
using namespace System::Collections::Generic;
using namespace System::Data;
using namespace System::Data::Common;

BEGIN_NAMESPACE(zuki)
BEGIN_NAMESPACE(data)
BEGIN_NAMESPACE(dbms)

//---------------------------------------------------------------------------
// Forward Class Declarations
//---------------------------------------------------------------------------

ref class zDBConnection;				// zDBConnection.h

//---------------------------------------------------------------------------
// Class zDBParameterCollection
//
// Implements a collection of parameters for a parameterized query
//---------------------------------------------------------------------------

public ref class zDBParameterCollection sealed : public DbParameterCollection
{
public:

	//-----------------------------------------------------------------------
	// Member Functions

	// Add (DbParameterCollection)
	//
	// Adds an unnamed parameter with the specified value
	virtual int Add(Object^ value) override { return IndexOf(Add(ToParameter(value))); }

	// Add
	//
	// Various additional signatures specific to this provider
	zDBParameter^ Add(zDBParameter^ param);

	zDBParameter^ Add(String^ name) 
		{ return Add(gcnew zDBParameter(name)); }

	zDBParameter^ Add(String^ name, zDBType dataType) 
		{  return Add(gcnew zDBParameter(name, dataType)); }

	zDBParameter^ Add(String^ name, zDBType dataType, String^ srcColumn) 
		{ return Add(gcnew zDBParameter(name, dataType, 0, false, srcColumn)); }

	zDBParameter^ Add(String^ name, zDBType dataType, int size) 
		{  return Add(gcnew zDBParameter(name, dataType, size)); }

	zDBParameter^ Add(String^ name, zDBType dataType, int size, String^ srcColumn) 
		{  return Add(gcnew zDBParameter(name, dataType, size, false, srcColumn)); }

	zDBParameter^ Add(zDBType dataType) 
		{ return Add(gcnew zDBParameter(String::Empty, dataType)); }

	zDBParameter^ Add(zDBType dataType, String^ srcColumn) 
		{ return Add(gcnew zDBParameter(String::Empty, dataType, 0, false, srcColumn)); }

	zDBParameter^ Add(zDBType dataType, int size) 
		{ return Add(gcnew zDBParameter(String::Empty, dataType, size)); }

	zDBParameter^ Add(zDBType dataType, int size, String^ srcColumn) 
		{ return Add(gcnew zDBParameter(String::Empty, dataType, size, false, srcColumn)); }

	// AddRange (DbParameterCollection)
	//
	// Adds a range of unnamed parameter values to the collection
	virtual void AddRange(Array^ values) override { AddRange(ToParameterArray(values)); }

	// AddRange
	//
	// Adds an array of zDBParameters to the collection
	void AddRange(array<zDBParameter^>^ values);

	// AddWithValue
	//
	// Adds a parameter with a value to the end of the collection
	zDBParameter^ AddWithValue(Object^ value) 
		{ return Add(gcnew zDBParameter(value)); }

	zDBParameter^ AddWithValue(String^ name, Object^ value) 
		{ return Add(gcnew zDBParameter(name, value)); }

	zDBParameter^ AddWithValue(String^ name, zDBType dataType, Object^ value) 
		{ return Add(gcnew zDBParameter(name, dataType, value)); }

	zDBParameter^ AddWithValue(zDBType dataType, Object^ value) 
		{ return Add(gcnew zDBParameter(dataType, value)); }

	// Clear (DbParameterCollection)
	//
	// Removes all parameters from the collection
	virtual void Clear(void) override;

	// Contains (DbParameterCollection)
	//
	// Determines if a parameter with the specified value exists in the collection
	virtual bool Contains(Object^ value) override { return Contains(ToParameter(value)); }

	// Contains (DbParameterCollection)
	//
	// Determines if a parameter with the specified name exists in the collection
	virtual bool Contains(String^ name) override;

	// Contains
	//
	// Determines if the specified parameter exists in this collection
	bool Contains(zDBParameter^ parameter) { return m_col->Contains(parameter); }

	// CopyTo (DbParameterCollection)
	//
	// Copies the parameter values into an array
	virtual void CopyTo(Array^ array, int index) override { CopyTo(ToParameterArray(array), index); }

	// CopyTo
	//
	// Copies the parameter objects into an array
	void CopyTo(array<zDBParameter^>^ array, int index) { return m_col->CopyTo(array, index); }

	// GetEnumerator (DbParameterCollection)
	//
	// Gets an enumerator capable of iterating the collection elements
	virtual Collections::IEnumerator^ GetEnumerator(void) override { return m_col->GetEnumerator(); }

	// IndexOf (DbParameterCollection)
	//
	// Gets the index of the specified parameter object
	virtual int IndexOf(Object^ value) override { return IndexOf(ToParameter(value)); }

	// IndexOf (DbParameterCollection)
	//
	// Gets the index of the specified parameter name
	virtual int IndexOf(String^ name) override;

	// IndexOf
	//
	// Gets the index of the specified parameter object
	virtual int IndexOf(zDBParameter^ parameter) { return m_col->IndexOf(parameter); }

	// Insert (DbParameterCollection)
	//
	// Inserts the specified index of the DbParameter object with the specified 
	// name into the collection at the specified index. 
	virtual void Insert(int index, Object^ value) override { Insert(index, ToParameter(value)); }

	// Insert
	//
	// Inserts a zDBParameter into the collection at the specified index
	void Insert(int index, zDBParameter^ parameter);

	// Remove (DbParameterCollection)
	//
	// Removes the specified parameter object from the collection
	virtual void Remove(Object^ value) override { Remove(ToParameter(value)); }

	// Remove
	//
	// Removes the specified zDBParameter object from the collection
	void Remove(zDBParameter^ parameter);

	// RemoveAt (DbParameterCollection)
	//
	// Removes the parameter with the specified index from the collection
	virtual void RemoveAt(int index) override;

	// RemoveAt (DbParameterCollection)
	//
	// Removes the parameter with the specified name from the collection
	virtual void RemoveAt(String^ name) override { RemoveAt(IndexOf(name)); }

	//-----------------------------------------------------------------------
	// Properties

	// Count (DbParameterCollection)
	//
	// Gets the number of parameters in the collection
	virtual property int Count 
	{ 
		int get(void) override { return InnerCollection->Count; } 
	}

	// IsFixedSize (DbParameterCollection)
	//
	// Gets a flag if this is a fixed size collection or not
	virtual property bool IsFixedSize 
	{ 
		bool get(void) override { return InnerList->IsFixedSize; } 
	}

	// IsReadOnly (DbParameterCollection)
	//
	// Gets a flag if this is a read only collection or not
	virtual property bool IsReadOnly 
	{ 
		bool get(void) override { return InnerList->IsReadOnly; } 
	}

	// IsSynchronized (DbParameterCollection)
	//
	// Gets a flag if this collection is synchronized or not
	virtual property bool IsSynchronized 
	{ 
		bool get(void) override { return InnerCollection->IsSynchronized; } 
	}

	// SyncRoot (DbParameterCollection)
	//
	// Gets a reference to the object to be used to synchronize access
	virtual property Object^ SyncRoot 
	{ 
		Object^ get(void) override { return InnerCollection->SyncRoot; } 
	}

	//-----------------------------------------------------------------------
	// Indexers

	// default[int]
	//
	// Gets or sets the zDBParameter at the specified index
	property zDBParameter^ default[int]
	{
		zDBParameter^ get(int index) new { return safe_cast<zDBParameter^>(GetParameter(index)); }
		void set(int index, zDBParameter^ value) { SetParameter(index, value); }
	}

	// default[String^]
	//
	// Gets or sets the zDBParameter with the specified name
	property zDBParameter^ default[String^]
	{
		zDBParameter^ get(String^ name) new { return safe_cast<zDBParameter^>(GetParameter(name)); }
		void set(String^ name, zDBParameter^ value) { SetParameter(name, value); }
	}

protected:

	//-----------------------------------------------------------------------
	// Protected Member Functions

	// GetParameter (DbParameterCollection)
	//
	// Gets a reference to the parameter at the specified index
	virtual DbParameter^ GetParameter(int index) override { return m_col[index]; }

	// GetParameter (DbParameterCollection)
	//
	// Gets a reference to the parameter with the specified name
	virtual DbParameter^ GetParameter(String^ name) override { return GetParameter(IndexOf(name)); }

	// SetParameter (DbParameterCollection)
	//
	// Sets the parameter at the specified index
	virtual void SetParameter(int index, DbParameter^ parameter) override;

	// SetParameter (DbParameterCollection)
	//
	// Sets the parameter with the specified name
	virtual void SetParameter(String^ name, DbParameter^ parameter) override;

internal:

	// INTERNAL CONSTRUCTOR
	zDBParameterCollection() : m_col(gcnew List<zDBParameter^>()) {}

	//-----------------------------------------------------------------------
	// Internal Member Functions

	// GetNamedParameter
	//
	// Gets a reference to a specific named parameter
	zDBParameter^ GetNamedParameter(String^ name);

	// GetUnnamedParameter
	//
	// Gets a reference to a specific unnamed parameter
	zDBParameter^ GetUnnamedParameter(int index);

	// Lock
	//
	// Locks the collection; used when a command is executing
	void Lock(void);

	// Unlock
	//
	// Unlocks the collection; used when a command is no longer executing
	void Unlock(void);

private:

	//-----------------------------------------------------------------------
	// Private Member Functions

	// TestAddParameter
	//
	// Determines if a parameter can be added to this collection or not
	void TestAddParameter(zDBParameter^ param);

	// ToParameter
	//
	// Converts a generic Object into a zDBParameter, if possible
	zDBParameter^ ToParameter(Object^ object);

	// ToParameterArray
	//
	// Converts a generic array into a zDBParameter array, if possible
	array<zDBParameter^>^ ToParameterArray(Array^ object);

	//-----------------------------------------------------------------------
	// Private Properties

	// InnerCollection
	//
	// Helper property to cast the inner List<T> into a standard ICollection^
	property Collections::ICollection^ InnerCollection
	{
		Collections::ICollection^ get(void) { return static_cast<Collections::ICollection^>(m_col); }
	}

	// InnerList
	//
	// Helper property to cast the inner List<T> into a standard IList^
	property Collections::IList^ InnerList
	{
		Collections::IList^ get(void) { return static_cast<Collections::IList^>(m_col); }
	}

	//-----------------------------------------------------------------------
	// Member Variables

	List<zDBParameter^>^		m_col;			// Contained collection
	bool						m_locked;		// Locked collection flag
};

//---------------------------------------------------------------------------

END_NAMESPACE(dbms)
END_NAMESPACE(data)
END_NAMESPACE(zuki)

#pragma warning(pop)

#endif		// __ZDBPARAMETERCOLLECTION_H_
