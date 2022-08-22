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
#include "zDBDataSourceEnumerator.h"	// Include zDBDataSourceEnumerator decls
#include "zDBFactory.h"					// Include zDBFactory declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// zDBDataSourceEnumerator Constructor
//
// Arguments:
//
//	NONE

zDBDataSourceEnumerator::zDBDataSourceEnumerator()
{
	Folder = Environment::CurrentDirectory;		// Use the working directory
}

//---------------------------------------------------------------------------
// zDBDataSourceEnumerator Constructor
//
// Arguments:
//
//	path		- Path to be used for the data source enumeration

zDBDataSourceEnumerator::zDBDataSourceEnumerator(String^ path)
{
	Folder = path;			// Use the specified directory for scanning
}

//---------------------------------------------------------------------------
// zDBDataSourceEnumerator::Folder::set
//
// Changes the contained path that will be searched for SQLite databases

void zDBDataSourceEnumerator::Folder::set(String^ value)
{
	// Check for invalid characters in the path before converting it
	// into a full path string and applying it to the member variable

	if(value->IndexOfAny(Path::GetInvalidPathChars()) != -1)
		throw gcnew ArgumentException("Illegal characters in path");

	m_path = Path::GetFullPath(value);
}

//---------------------------------------------------------------------------
// zDBDataSourceEnumerator::GetDataSources
//
// Returns a DataTable containing information about all enumerated data
// sources in the directory currently set up in the .Path property
//
// Arguments:
//
//	NONE

DataTable^ zDBDataSourceEnumerator::GetDataSources(void)
{
	DataTable^			sources;		// DataTable to return to caller
	System::Byte		readVer;		// File format read version
	System::Byte		writeVer;		// File format write version

	sources = s_template->Clone();		// Clone the template data table

	// Iterate over each file in the specified directory, and use the 
	// helper function to determine if the file is a SQLite file or not

	for each(String^ filename in Directory::GetFiles(m_path)) {

		if(IsDatabase(filename, writeVer, readVer)) {

			sources->Rows->Add(gcnew array<Object^>{ 
				
				filename,											// ServerName
				IO::Path::GetFileNameWithoutExtension(filename),	// InstanceName
				"False",											// IsClustered
				String::Format("W{0};R{1}", writeVer, readVer),		// Version
				zDBFactory::typeid->AssemblyQualifiedName			// FactoryName
			});
		}
	}

	return sources;						// Return the generated data table
}

//---------------------------------------------------------------------------
// zDBDataSourceEnumerator::IsDatabase (private, static)
//
// Determines if the specified file is a SQLite database or not.  Requires
// that the file header be the same for the platform this code was compiled
// against. At initial implementation --> "SQLite format 3"
//
// Arguments:
//
//	filename		- File name to be tested
//	writeVer		- On TRUE, contains the file write version
//	readVer			- On TRUE, contains the file read version

bool zDBDataSourceEnumerator::IsDatabase(String^ filename, System::Byte% writeVer, System::Byte% readVer)
{
	HANDLE					hFile;			// File handle
	PinnedStringPtr			pinFilename;	// Pinned file name string
	BYTE					rgBuffer[20];	// 20 byte input buffer
	DWORD					cbRead;			// Bytes read from the file

	if(filename == nullptr) return false;

	// Attempt to open the file using as optimal of a set of flags as
	// we can since we only look at the first 16 bytes of the thing.  I
	// didn't use managed code here since I have no idea how efficient it is

	pinFilename = PtrToStringChars(filename);
	hFile = CreateFile(pinFilename, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if(hFile == INVALID_HANDLE_VALUE) return false;

	memset(rgBuffer, 0, 20);							// Initialize buffer
	ReadFile(hFile, rgBuffer, 20, &cbRead, NULL);		// Try to read the data
	CloseHandle(hFile);									// Close the file handle
	
	// If the data we got back doesn't match the SQLite header, we're done

	if(cbRead != 20) return false;
	if(strncmp(reinterpret_cast<char*>(rgBuffer), "SQLite format 3", 16) != 0) return false;

	writeVer = rgBuffer[18];				// Set the write format version
	readVer = rgBuffer[19];					// Set the read format version

	return true;
}

//---------------------------------------------------------------------------
// zDBDataSourceEnumerator::StaticConstruct (private, static)
//
// Acts on behalf of the static constructor to initialize all static data
//
// Arguments:
//
//	NONE

void zDBDataSourceEnumerator::StaticConstruct(void)
{
	// Construct and initialize the static DataTable that serves as the
	// template for all tables generated by GetDataSources

	s_template = gcnew DataTable("DataSources");
	s_template->Columns->Add("ServerName",		String::typeid);
	s_template->Columns->Add("InstanceName",	String::typeid);
	s_template->Columns->Add("IsClustered",		String::typeid);
	s_template->Columns->Add("Version",			String::typeid);
	s_template->Columns->Add("FactoryName",		String::typeid);
}

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)
