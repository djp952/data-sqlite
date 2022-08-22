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

#ifndef __ZDBVIRTUALTABLECONSTRUCTORARGS_H_
#define __ZDBVIRTUALTABLECONSTRUCTORARGS_H_
#pragma once

#include "zDBUtil.h"					// Include zDBUtil declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Collections::ObjectModel;
using namespace System::Threading;

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// Forward Class Declarations
//---------------------------------------------------------------------------

ref class zDBConnection;				// zDBConnection.h

//---------------------------------------------------------------------------
// Class zDBVirtualTableConstructorArgs (internal)
//
// In order to make the derivations of virtual tables a little easier, I've
// changed this around so it no longer gets passed into zDBVirtualTable's
// constructor.  Rather than add a method that would called post-ctor, which
// would prevent the application from accessing this data from it's ctor,
// I hacked in a simple static collection that uses a push/pop mentality.
// The virtual table module creates and pushes an instance of this in there,
// and zDBVirtualTable will pop it back out when it's ready.  Only the
// thread id is used as a key, that should be sufficient since you won't be
// creating more than one vtable on any given thread at any given time.
//---------------------------------------------------------------------------

ref class zDBVirtualTableConstructorArgs sealed
{
public:

	//-----------------------------------------------------------------------
	// Member Functions

	// Pop (static)
	//
	// Called by zDBVirtualTable to "pop" it's constructor arguments out of 
	// the static cache during construction
	static zDBVirtualTableConstructorArgs^ Pop(void)
	{
		zDBVirtualTableConstructorArgs^		instance;		// Instance to return

		if(s_col->TryGetValue(Thread::CurrentThread->ManagedThreadId, instance)) return instance;
		else throw gcnew Exception("INTERNAL FAILURE: Missing virtual table constructor data");
	}

	// Push (static)
	//
	// Creates a new instance of the arguments class and "pushes" it into the
	// static cache so zDBVirtualTable can find it later on (it's not really
	// a stack of course, but it still seemed like a good name to use)
	static void Push(zDBConnection^ conn, int argc, const char* const* argv)
	{
		// The object does not need to be disposed of or have any special 
		// conditions that would prevent us from just overwriting an existing
		// one.  Technically that should never happen, but if it does it's fine

		int key = Thread::CurrentThread->ManagedThreadId;
		s_col[key] = gcnew zDBVirtualTableConstructorArgs(conn, argc, argv);;
	}
	
	//-----------------------------------------------------------------------
	// Properties

	// Arguments
	//
	// Gets a reference to the read-only module arguments collection
	property ReadOnlyCollection<String^>^ Arguments
	{
		ReadOnlyCollection<String^>^ get(void) { return m_args; }
	}

	// Connection
	//
	// Gets a reference to the connection that this virtual table
	// has been registered with
	property zDBConnection^ Connection
	{
		zDBConnection^ get(void) { return m_conn; }
	}

	// DatabaseName
	//
	// Gets the name of the database in which the table was created
	property String^ DatabaseName
	{
		String^ get(void) { return m_database; }
	}

	// ModuleName
	//
	// Gets the name of the module used to create the virtual table
	property String^ ModuleName
	{
		String^ get(void) { return m_module; }
	}

	// TableName
	//
	// Gets the name of the virtual table
	property String^ TableName
	{
		String^ get(void) { return m_table; }
	}

private:

	// STATIC CONSTRUCTOR
	static zDBVirtualTableConstructorArgs()
	{
		s_col = gcnew Dictionary<int, zDBVirtualTableConstructorArgs^>();
	}

	// PRIVATE CONSTRUCTOR
	zDBVirtualTableConstructorArgs(zDBConnection^ conn, int argc, const char* const* argv)
	{
		Debug::Assert(argc >= 3);		// Should always be at least 3 of them

		m_conn = conn;	
		m_module = zDBUtil::FastPtrToStringAnsi(argv[0]);
		m_database = zDBUtil::FastPtrToStringAnsi(argv[1]);
		m_table = zDBUtil::FastPtrToStringAnsi(argv[2]);

		// Process all of the remaining arguments into a read only collection
		// of strings that the class can access whenever it wants to

		array<String^>^ args = gcnew array<String^>(argc - 3);
		for(int index = 0; index < (argc - 3); index++) 
			args[index] = zDBUtil::FastPtrToStringAnsi(argv[index + 3]);

		m_args = Array::AsReadOnly(args);		// Convert to ReadOnlyCollection
	}

	//-----------------------------------------------------------------------
	// Member Variables

	zDBConnection^					m_conn;			// Parent connection object
	String^							m_module;		// Parent module name
	String^							m_database;		// Parent database name
	String^							m_table;		// Virtual table name
	ReadOnlyCollection<String^>^	m_args;			// Arguments passed to module

	// s_col
	//
	// Hack to allow zDBVirtualTable to get it's constructor arguments without
	// us having to actually pass them in.
	static Dictionary<int, zDBVirtualTableConstructorArgs^>^	s_col;
};

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)

#endif		// __ZDBVIRTUALTABLECONSTRUCTORARGS_H_
