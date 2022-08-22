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

#ifndef __SQLITEPARAMETERCOLLECTION_H_
#define __SQLITEPARAMETERCOLLECTION_H_
#pragma once

#include "SqliteParameter.h"				// Include SqliteParameter declarations
#include "SqliteExceptions.h"				// Include Sqlite exception decls
#include "SqliteType.h"					// Include SqliteType declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

using namespace System;
using namespace System::Collections;
using namespace System::Collections::Generic;
using namespace System::Data;
using namespace System::Data::Common;

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// Forward Class Declarations
//---------------------------------------------------------------------------

ref class SqliteConnection;				// SqliteConnection.h

//---------------------------------------------------------------------------
// Class SqliteParameterCollection
//
// Implements a collection of parameters for a parameterized query
//---------------------------------------------------------------------------

public ref class SqliteParameterCollection sealed : public DbParameterCollection
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
	SqliteParameter^ Add(SqliteParameter^ param);

	SqliteParameter^ Add(String^ name) 
		{ return Add(gcnew SqliteParameter(name)); }

	SqliteParameter^ Add(String^ name, SqliteType dataType) 
		{  return Add(gcnew SqliteParameter(name, dataType)); }

	SqliteParameter^ Add(String^ name, SqliteType dataType, String^ srcColumn) 
		{ return Add(gcnew SqliteParameter(name, dataType, 0, false, srcColumn)); }

	SqliteParameter^ Add(String^ name, SqliteType dataType, int size) 
		{  return Add(gcnew SqliteParameter(name, dataType, size)); }

	SqliteParameter^ Add(String^ name, SqliteType dataType, int size, String^ srcColumn) 
		{  return Add(gcnew SqliteParameter(name, dataType, size, false, srcColumn)); }

	SqliteParameter^ Add(SqliteType dataType) 
		{ return Add(gcnew SqliteParameter(String::Empty, dataType)); }

	SqliteParameter^ Add(SqliteType dataType, String^ srcColumn) 
		{ return Add(gcnew SqliteParameter(String::Empty, dataType, 0, false, srcColumn)); }

	SqliteParameter^ Add(SqliteType dataType, int size) 
		{ return Add(gcnew SqliteParameter(String::Empty, dataType, size)); }

	SqliteParameter^ Add(SqliteType dataType, int size, String^ srcColumn) 
		{ return Add(gcnew SqliteParameter(String::Empty, dataType, size, false, srcColumn)); }

	// AddRange (DbParameterCollection)
	//
	// Adds a range of unnamed parameter values to the collection
	virtual void AddRange(Array^ values) override { AddRange(ToParameterArray(values)); }

	// AddRange
	//
	// Adds an array of SqliteParameters to the collection
	void AddRange(array<SqliteParameter^>^ values);

	// AddWithValue
	//
	// Adds a parameter with a value to the end of the collection
	SqliteParameter^ AddWithValue(Object^ value) 
		{ return Add(gcnew SqliteParameter(value)); }

	SqliteParameter^ AddWithValue(String^ name, Object^ value) 
		{ return Add(gcnew SqliteParameter(name, value)); }

	SqliteParameter^ AddWithValue(String^ name, SqliteType dataType, Object^ value) 
		{ return Add(gcnew SqliteParameter(name, dataType, value)); }

	SqliteParameter^ AddWithValue(SqliteType dataType, Object^ value) 
		{ return Add(gcnew SqliteParameter(dataType, value)); }

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
	bool Contains(SqliteParameter^ parameter) { return m_col->Contains(parameter); }

	// CopyTo (DbParameterCollection)
	//
	// Copies the parameter values into an array
	virtual void CopyTo(Array^ array, int index) override { CopyTo(ToParameterArray(array), index); }

	// CopyTo
	//
	// Copies the parameter objects into an array
	void CopyTo(array<SqliteParameter^>^ array, int index) { return m_col->CopyTo(array, index); }

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
	virtual int IndexOf(SqliteParameter^ parameter) { return m_col->IndexOf(parameter); }

	// Insert (DbParameterCollection)
	//
	// Inserts the specified index of the DbParameter object with the specified 
	// name into the collection at the specified index. 
	virtual void Insert(int index, Object^ value) override { Insert(index, ToParameter(value)); }

	// Insert
	//
	// Inserts a SqliteParameter into the collection at the specified index
	void Insert(int index, SqliteParameter^ parameter);

	// Remove (DbParameterCollection)
	//
	// Removes the specified parameter object from the collection
	virtual void Remove(Object^ value) override { Remove(ToParameter(value)); }

	// Remove
	//
	// Removes the specified SqliteParameter object from the collection
	void Remove(SqliteParameter^ parameter);

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
	// Gets or sets the SqliteParameter at the specified index
	property SqliteParameter^ default[int]
	{
		SqliteParameter^ get(int index) new { return safe_cast<SqliteParameter^>(GetParameter(index)); }
		void set(int index, SqliteParameter^ value) { SetParameter(index, value); }
	}

	// default[String^]
	//
	// Gets or sets the SqliteParameter with the specified name
	property SqliteParameter^ default[String^]
	{
		SqliteParameter^ get(String^ name) new { return safe_cast<SqliteParameter^>(GetParameter(name)); }
		void set(String^ name, SqliteParameter^ value) { SetParameter(name, value); }
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
	SqliteParameterCollection() : m_col(gcnew List<SqliteParameter^>()) {}

	//-----------------------------------------------------------------------
	// Internal Member Functions

	// GetNamedParameter
	//
	// Gets a reference to a specific named parameter
	SqliteParameter^ GetNamedParameter(String^ name);

	// GetUnnamedParameter
	//
	// Gets a reference to a specific unnamed parameter
	SqliteParameter^ GetUnnamedParameter(int index);

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
	void TestAddParameter(SqliteParameter^ param);

	// ToParameter
	//
	// Converts a generic Object into a SqliteParameter, if possible
	SqliteParameter^ ToParameter(Object^ object);

	// ToParameterArray
	//
	// Converts a generic array into a SqliteParameter array, if possible
	array<SqliteParameter^>^ ToParameterArray(Array^ object);

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

	List<SqliteParameter^>^		m_col;			// Contained collection
	bool						m_locked;		// Locked collection flag
};

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)

#endif		// __SQLITEPARAMETERCOLLECTION_H_
