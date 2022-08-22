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
#include "SqliteConnectionStringBuilder.h"	// Include class declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// SqliteConnectionStringBuilder Constructor
//
// Arguments:
//
//	NONE

SqliteConnectionStringBuilder::SqliteConnectionStringBuilder()
{
	m_fieldPassword = gcnew SecureString();		// Create the SecureString
	ResetAll();									// Initialize to all defaults
}

//---------------------------------------------------------------------------
// SqliteConnectionStringBuilder Constructor
//
// Arguments:
//
//	connectionString	- Existing connection string to be parsed

SqliteConnectionStringBuilder::SqliteConnectionStringBuilder(String^ connectionString)
{
	m_fieldPassword = gcnew SecureString();		// Create the SecureString
	ResetAll();									// Initialize to all defaults

	// If the provided connection string is not empty, try to set it up.
	// (The base class kindly calls our indexer for us on each item)

	if((connectionString != nullptr) && (connectionString->Length > 0))
		__super::ConnectionString = connectionString;
}

//---------------------------------------------------------------------------
// SqliteConnectionStringBuilder::AllowExtensions::set
//
// Sets the option for allowing dynamically loaded extensions or not

void SqliteConnectionStringBuilder::AllowExtensions::set(bool value)
{
	DbConnectionStringBuilder::default[s_keywords[static_cast<int>(KeywordCode::AllowExtensions)]] = value.ToString();
	m_autoVacuum = value;
}

//---------------------------------------------------------------------------
// SqliteConnectionStringBuilder::AutoVacuum::set
//
// Sets the option for automatically shrinking the database file on close

void SqliteConnectionStringBuilder::AutoVacuum::set(bool value)
{
	DbConnectionStringBuilder::default[s_keywords[static_cast<int>(KeywordCode::AutoVacuum)]] = value.ToString();
	m_autoVacuum = value;
}

//---------------------------------------------------------------------------
// SqliteConnectionStringBuilder::BooleanFormat::set
//
// Sets the option for formatting and storing boolean values

void SqliteConnectionStringBuilder::BooleanFormat::set(SqliteBooleanFormat value)
{
	if(!Enum::IsDefined(SqliteBooleanFormat::typeid, value)) throw gcnew ArgumentOutOfRangeException();

	DbConnectionStringBuilder::default[s_keywords[static_cast<int>(KeywordCode::BooleanFormat)]] = value.ToString();
	m_booleanFormat = value;
}

//---------------------------------------------------------------------------
// SqliteConnectionStringBuilder::CacheSize::set
//
// Sets the maximum number of disk pages to be held in memory

void SqliteConnectionStringBuilder::CacheSize::set(int value)
{
	if(value < 0) throw gcnew ArgumentOutOfRangeException();

	DbConnectionStringBuilder::default[s_keywords[static_cast<int>(KeywordCode::CacheSize)]] = value.ToString();
	m_cacheSize = value;
}

//---------------------------------------------------------------------------
// SqliteConnectionStringBuilder::CaseSensitiveLike::set
//
// Sets the option for a case-sensitive LIKE command behavior or not

void SqliteConnectionStringBuilder::CaseSensitiveLike::set(bool value)
{
	DbConnectionStringBuilder::default[s_keywords[static_cast<int>(KeywordCode::CaseSensitiveLike)]] = value.ToString();
	m_caseSensitiveLike = value;
}

//---------------------------------------------------------------------------
// SqliteConnectionStringBuilder::CompatibleFileFormat::set
//
// Sets the option to use a compatible (back to 3.0) file format or not

void SqliteConnectionStringBuilder::CompatibleFileFormat::set(bool value)
{
	DbConnectionStringBuilder::default[s_keywords[static_cast<int>(KeywordCode::CompatibleFileFormat)]] = value.ToString();
	m_compatibleFormat = value;
}

//---------------------------------------------------------------------------
// SqliteConnectionStringBuilder::Clear
//
// Removes all items from the collection and returns the settings back to
// their default states
//
// Arguments:
//
//	NONE

void SqliteConnectionStringBuilder::Clear(void)
{
	__super::Clear();			// Remove all collection items
	ResetAll();					// Reset all connection properties
}

//---------------------------------------------------------------------------
// SqliteConnectionStringBuilder::ContainsKey
//
// Determines if the specified keyword exists or not
//
// Arguments:
//
//	keyword		- Keyword to be tested

bool SqliteConnectionStringBuilder::ContainsKey(String^ keyword)
{
	if(keyword == nullptr) throw gcnew ArgumentNullException();
	return s_keywordMap->ContainsKey(keyword);
}

//---------------------------------------------------------------------------
// SqliteConnectionStringBuilder::DataSource::set
//
// Sets the path to the master database file

void SqliteConnectionStringBuilder::DataSource::set(String^ value)
{
	if(value == nullptr) throw gcnew ArgumentNullException();

	// The Data Source must conform to certain restrictions.  Failure to check this up
	// front can lead to SQLITE_MISUSE errors when opening the connection.

	if(!SqliteUtil::ValidateDataSource(value))
		throw gcnew FormatException(String::Format("'{0}' is not a valid Data Source", value));

	// Another problem I've found is that the :memory: data source is case-sensitive.
	// This probably isn't a bug per-se, but I don't see why it needs to be this way.

	ENGINE_ISSUE(3.3.8, ":memory: data source name is case-sensitive");
	if(String::Compare(value, SqliteDataSource::Memory, true) == 0) value = SqliteDataSource::Memory;
	
	DbConnectionStringBuilder::default[s_keywords[static_cast<int>(KeywordCode::DataSource)]] = value;
	m_dataSource = value;
}

//---------------------------------------------------------------------------
// SqliteConnectionStringBuilder::DateTimeFormat::set
//
// Sets the option for formatting and storing date/time values

void SqliteConnectionStringBuilder::DateTimeFormat::set(SqliteDateTimeFormat value)
{
	if(!Enum::IsDefined(SqliteDateTimeFormat::typeid, value)) throw gcnew ArgumentOutOfRangeException();

	DbConnectionStringBuilder::default[s_keywords[static_cast<int>(KeywordCode::DateTimeFormat)]] = value.ToString();
	m_dateTimeFormat = value;
}

//---------------------------------------------------------------------------
// SqliteConnectionStringBuilder::default::get
//
// Retrieves a connection string value by keyword
//
// Arguments:
//
//	keyword		- Keyword of the item to be retrieved

Object^ SqliteConnectionStringBuilder::default::get(String^ keyword)
{
	return GetAt(GetIndex(keyword));		// Not too difficult ...
}

//---------------------------------------------------------------------------
// SqliteConnectionStringBuilder::default::set
//
// Changes the value of a specific connection string item by name
//
// Arguments:
//
//	keyword		- Keyword of the item to be set
//	value		- Value to set the connection string item to

void SqliteConnectionStringBuilder::default::set(String^ keyword, Object^ value)
{
	if(value == nullptr) { Remove(keyword); return; }	// NULL: Remove and reset

	// This is pretty ugly stuff.  I've followed the model used to implement the .NET
	// SqlConnectionStringBuilder, but in retrospect I wish I did it my way instead.
	// Of course, the REALLY ugly stuff comes from my own use of enumerations :)

	switch(GetIndex(keyword)) {

		case KeywordCode::AllowExtensions:
			AllowExtensions = Convert::ToBoolean(value);
			return;

		case KeywordCode::AutoVacuum:			
			AutoVacuum = Convert::ToBoolean(value); 
			return;

		case KeywordCode::BooleanFormat:
			try { BooleanFormat = static_cast<SqliteBooleanFormat>(Enum::Parse(SqliteBooleanFormat::typeid, Convert::ToString(value), true)); }
			catch(Exception^) { throw gcnew FormatException(String::Format("'{0}' is not a valid SqliteBooleanFormat option", Convert::ToString(value))); }
			return;

		case KeywordCode::CacheSize:
			CacheSize = Convert::ToInt32(value);
			return;

		case KeywordCode::CaseSensitiveLike:
			CaseSensitiveLike = Convert::ToBoolean(value);
			return;

		case KeywordCode::CompatibleFileFormat:
			CompatibleFileFormat = Convert::ToBoolean(value);
			return;

		case KeywordCode::DataSource:
			DataSource = Convert::ToString(value);
			return;

		case KeywordCode::DateTimeFormat:
			try { DateTimeFormat = static_cast<SqliteDateTimeFormat>(Enum::Parse(SqliteDateTimeFormat::typeid, Convert::ToString(value), true)); }
			catch(Exception^) { throw gcnew FormatException(String::Format("'{0}' is not a valid SqliteDateTimeFormat option", Convert::ToString(value))); }
			return;

		case KeywordCode::Encoding:
			try { Encoding = static_cast<SqliteTextEncodingMode>(Enum::Parse(SqliteTextEncodingMode::typeid, Convert::ToString(value), true)); }
			catch(Exception^) { throw gcnew FormatException(String::Format("'{0}' is not a valid SqliteTextEncodingMode option", Convert::ToString(value))); }
			return;

		case KeywordCode::Enlist:
			Enlist = Convert::ToBoolean(value);
			return;

		// TODO: Result from Convert::ToString needs to be secured!!!
		case KeywordCode::FieldEncryptionPassword:
			FieldEncryptionPassword = Convert::ToString(value);
			return;

		case KeywordCode::GuidFormat:
			try { GuidFormat = static_cast<SqliteGuidFormat>(Enum::Parse(SqliteGuidFormat::typeid, Convert::ToString(value), true)); }
			catch(Exception^) { throw gcnew FormatException(String::Format("'{0}' is not a valid SqliteGuidFormat option", Convert::ToString(value))); }
			return;

		case KeywordCode::PageSize:
			PageSize = Convert::ToInt32(value);
			return;

		case KeywordCode::SynchronousMode:
			try { SynchronousMode = static_cast<SqliteSynchronousMode>(Enum::Parse(SqliteSynchronousMode::typeid, Convert::ToString(value), true)); }
			catch(Exception^) { throw gcnew FormatException(String::Format("'{0}' is not a valid SqliteSynchronousMode option", Convert::ToString(value))); }
			return;

		case KeywordCode::TemporaryStorageFolder:	
			TemporaryStorageFolder = Convert::ToString(value); 
			return;
		
		case KeywordCode::TemporaryStorageMode:
			try { TemporaryStorageMode = static_cast<SqliteTemporaryStorageMode>(Enum::Parse(SqliteTemporaryStorageMode::typeid, Convert::ToString(value), true)); }
			catch(Exception^) { throw gcnew FormatException(String::Format("'{0}' is not a valid SqliteTemporaryStorageMode option", Convert::ToString(value))); }
			return;

		case KeywordCode::TransactionMode:
			try { TransactionMode = static_cast<SqliteTransactionMode>(Enum::Parse(SqliteTransactionMode::typeid, Convert::ToString(value), true)); }
			catch(Exception^) { throw gcnew FormatException(String::Format("'{0}' is not a valid SqliteTransactionMode option", Convert::ToString(value))); }
			return;
	}

	// UNSUPPORTED ARGUMENT
	throw gcnew KeyNotFoundException(String::Format("Keyword not supported: [{0}]", keyword));
}

//---------------------------------------------------------------------------
// SqliteConnectionStringBuilder::Encoding::set
//
// Sets the option for the text encoding (ANSI/UNICODE)

void SqliteConnectionStringBuilder::Encoding::set(SqliteTextEncodingMode value)
{
	if(!Enum::IsDefined(SqliteTextEncodingMode::typeid, value)) throw gcnew ArgumentOutOfRangeException();

	DbConnectionStringBuilder::default[s_keywords[static_cast<int>(KeywordCode::Encoding)]] = value.ToString();
	m_textEncodingMode = value;
}

//---------------------------------------------------------------------------
// SqliteConnectionStringBuilder::Enlist::set
//
// Sets the option for automatic enlistment in distributed transactions

void SqliteConnectionStringBuilder::Enlist::set(bool value)
{
	DbConnectionStringBuilder::default[s_keywords[static_cast<int>(KeywordCode::Enlist)]] = value.ToString();
	m_autoVacuum = value;
}

//---------------------------------------------------------------------------
// SqliteConnectionStringBuilder::FieldEncryptionPassword::set
//
// Sets the password that will be used to encrypt/decrypt field data

void SqliteConnectionStringBuilder::FieldEncryptionPassword::set(String^ value)
{
	PinnedStringPtr			pinPassword;		// Pinned password data
	int						cch;				// Length of password data

	delete m_fieldPassword;						// Destroy old password
	m_fieldPassword = gcnew SecureString();		// Create a new password

	try {

		if((value == nullptr) || (value->Length == 0)) return;

		pinPassword = PtrToStringChars(value);			// Pin the string
		cch = static_cast<int>(wcslen(pinPassword));	// Get the length

		// Append all of the characters in the password to the string, and then
		// immediately make it read-only to prevent tampering

		for(int index = 0; index < cch; index++) m_fieldPassword->AppendChar(pinPassword[index]);
	}
	
	finally { m_fieldPassword->MakeReadOnly(); }	// Lock the string
}

//---------------------------------------------------------------------------
// SqliteConnectionStringBuilder::GetAt (private)
//
// Returns the property value at the specified code index
//
// Arguments:
//
//	code		- KeywordCode of the property to get at

Object^ SqliteConnectionStringBuilder::GetAt(KeywordCode code)
{
	// NOTE: FieldEncryptionPassword cannot be accessed this way

	switch(code) {

		case KeywordCode::AllowExtensions:			return m_allowExtensions;
		case KeywordCode::AutoVacuum:				return m_autoVacuum;
		case KeywordCode::BooleanFormat:			return m_booleanFormat;
		case KeywordCode::CacheSize:				return m_cacheSize;
		case KeywordCode::CaseSensitiveLike:		return m_caseSensitiveLike;
		case KeywordCode::CompatibleFileFormat:		return m_compatibleFormat;
		case KeywordCode::DataSource:				return m_dataSource;
		case KeywordCode::DateTimeFormat:			return m_dateTimeFormat;
		case KeywordCode::Encoding:					return m_textEncodingMode;
		case KeywordCode::Enlist:					return m_enlist;
		case KeywordCode::GuidFormat:				return m_guidFormat;
		case KeywordCode::PageSize:					return m_pageSize;
		case KeywordCode::SynchronousMode:			return m_syncMode;
		case KeywordCode::TemporaryStorageFolder:	return m_tempStorageFolder;
		case KeywordCode::TemporaryStorageMode:		return m_tempStorageMode;
		case KeywordCode::TransactionMode:			return m_transactionMode;
	}

	// UNSUPPORTED ARGUMENT
	throw gcnew ArgumentOutOfRangeException();		// Invalid code
}

//---------------------------------------------------------------------------
// SqliteConnectionStringBuilder::GetIndex (private)
//
// Returns the keyword code associated with a specific string keyword
//
// Arguments:
//
//	keyword		- Keyword to be mapped back into a code

SqliteConnectionStringBuilder::KeywordCode SqliteConnectionStringBuilder::GetIndex(String^ keyword)
{
	KeywordCode				code;			// Code to be returned to the caller

	if(keyword == nullptr) throw gcnew ArgumentNullException();

	if(s_keywordMap->TryGetValue(keyword, code)) return code;
	else throw gcnew KeyNotFoundException(String::Format("Keyword not supported: [{0}]", keyword));
}

//---------------------------------------------------------------------------
// SqliteConnectionStringBuilder::GuidFormat::set
//
// Sets the option for formatting and storing UUID values

void SqliteConnectionStringBuilder::GuidFormat::set(SqliteGuidFormat value)
{
	if(!Enum::IsDefined(SqliteGuidFormat::typeid, value)) throw gcnew ArgumentOutOfRangeException();

	DbConnectionStringBuilder::default[s_keywords[static_cast<int>(KeywordCode::GuidFormat)]] = value.ToString();
	m_guidFormat = value;
}

//---------------------------------------------------------------------------
// SqliteConnectionStringBuilder::PageSize::set
//
// Sets the maximum number of disk pages to be held in memory

void SqliteConnectionStringBuilder::PageSize::set(int value)
{
	// The page size must be a power of 2, no less than 512, and no greater than 8192
	// (as per the SQLite documentation).  Note the slick math  to check for a power 
	// of two .. I didn't know this one before.  Very nice.  Thank you Google Groups.

	bool powerOfTwo = ((value > 0) && ((value & (value - 1)) == 0));
	if((!powerOfTwo) || (value < 512) || (value > 8192)) throw gcnew ArgumentOutOfRangeException(INVALID_PAGESIZE);

	DbConnectionStringBuilder::default[s_keywords[static_cast<int>(KeywordCode::PageSize)]] = value.ToString();
	m_pageSize = value;
}

//---------------------------------------------------------------------------
// SqliteConnectionStringBuilder::Remove
//
// Removes a specific connection string property from the collection.  The
// underlying value will be reset to a default state, not really "removed"
//
// Arguments:
//
//	keyword		- Keyword to remove from the collection

bool SqliteConnectionStringBuilder::Remove(String^ keyword)
{
	KeywordCode			code;			// Keyword code from the dictionary
	bool				result;			// Result from this function call

	if(keyword == nullptr) throw gcnew ArgumentNullException();

	if(!s_keywordMap->TryGetValue(keyword, code)) return false;
	result = __super::Remove(s_keywords[static_cast<int>(code)]);

	Reset(code);						// Always reset the underlying value
	return result;						// Return result from base class
}

//---------------------------------------------------------------------------
// SqliteConnectionStringBuilder::Reset (private)
//
// Resets a specific connection string property to its default state
//
// Arguments:
//
//	code		- Code of the property value to be reset

void SqliteConnectionStringBuilder::Reset(KeywordCode code)
{
	switch(code) {

		case KeywordCode::AllowExtensions:			m_allowExtensions = true; return;
		case KeywordCode::AutoVacuum :				m_autoVacuum = false; return;
		case KeywordCode::BooleanFormat:			m_booleanFormat = SqliteBooleanFormat::OneZero; return;
		case KeywordCode::CacheSize:				m_cacheSize = 2000; return;
		case KeywordCode::CaseSensitiveLike:		m_caseSensitiveLike = false; return;
		case KeywordCode::CompatibleFileFormat:		m_compatibleFormat = true; return;
		case KeywordCode::DataSource:				m_dataSource = String::Empty; return;
		case KeywordCode::DateTimeFormat:			m_dateTimeFormat = SqliteDateTimeFormat::ISO8601; return;
		case KeywordCode::Enlist:					m_enlist = false; return;
		case KeywordCode::FieldEncryptionPassword:	FieldEncryptionPassword = nullptr; return;
		case KeywordCode::GuidFormat:				m_guidFormat = SqliteGuidFormat::Binary; return;
		case KeywordCode::PageSize:					m_pageSize = 4096; return;
		case KeywordCode::SynchronousMode:			m_syncMode = SqliteSynchronousMode::Normal; return;
		case KeywordCode::TemporaryStorageFolder:	m_tempStorageFolder = String::Empty; return;
		case KeywordCode::TemporaryStorageMode:		m_tempStorageMode = SqliteTemporaryStorageMode::Default; return;
		case KeywordCode::Encoding:					m_textEncodingMode = SqliteTextEncodingMode::UTF16; return;
		case KeywordCode::TransactionMode:			m_transactionMode = SqliteTransactionMode::SimulateNested; return;
	}
}

//---------------------------------------------------------------------------
// SqliteConnectionStringBuilder::ResetAll (private)
//
// Resets all connection properties to their default states
//
// Arguments:
//
//	NONE

void SqliteConnectionStringBuilder::ResetAll(void)
{
	for(int index = 0; index < s_keywords->Length; index++) Reset(static_cast<KeywordCode>(index));
}

//---------------------------------------------------------------------------
// SqliteConnectionStringBuilder::ShouldSerialize
//
// Determines if the specified key exists in this collection.  (No joke, that's
// what MSDN says this method is for ... look it up!)
//
// Arguments:
//
//	keyword		- Keyword to be tested for existance

bool SqliteConnectionStringBuilder::ShouldSerialize(String^ keyword)
{
	KeywordCode			code;			// Keyword code from dictionary

	if(!s_keywordMap->TryGetValue(keyword, code)) return false;

	if(code == KeywordCode::FieldEncryptionPassword) return false;
	return __super::ShouldSerialize(s_keywords[static_cast<int>(code)]);
}

//---------------------------------------------------------------------------
// SqliteConnectionStringBuilder::StaticConstruct (static, private)
//
// Implements the class static constructor so it doesn't have to be
// declared inline in the header file
//
// Arguments:
//
//	NONE

void SqliteConnectionStringBuilder::StaticConstruct(void)
{
	// The only thing we need to do in here is actually load up the keyword
	// dictionary with the appropriate entries.  I fail to see how this can
	// be done with initializers, although in theory you can?

	for(int index = 0; index < s_keywords->Length; index++)
		s_keywordMap->Add(s_keywords[index], static_cast<KeywordCode>(index));
}

//---------------------------------------------------------------------------
// SqliteConnectionStringBuilder::SynchronousMode::set
//
// Sets the option for the SQLite 'synchronous' method

void SqliteConnectionStringBuilder::SynchronousMode::set(SqliteSynchronousMode value)
{
	if(!Enum::IsDefined(SqliteSynchronousMode::typeid, value)) throw gcnew ArgumentOutOfRangeException();

	DbConnectionStringBuilder::default[s_keywords[static_cast<int>(KeywordCode::SynchronousMode)]] = value.ToString();
	m_syncMode = value;
}

//---------------------------------------------------------------------------
// SqliteConnectionStringBuilder::TemporaryStorageFolder::set
//
// Sets the location of the temporary data storage file(s)

void SqliteConnectionStringBuilder::TemporaryStorageFolder::set(String^ value)
{
	if(value == nullptr) throw gcnew ArgumentNullException();
	
	// The Temporary Storage Folder must conform to OS directory name restrictions

	if(!SqliteUtil::ValidateFolderName(value))
		throw gcnew FormatException(String::Format("'{0}' is not a valid folder path", value));

	DbConnectionStringBuilder::default[s_keywords[static_cast<int>(KeywordCode::TemporaryStorageFolder)]] = value;
	m_tempStorageFolder = value;
}

//---------------------------------------------------------------------------
// SqliteConnectionStringBuilder::TemporaryStorageMode::set
//
// Sets the option for the SQLite temporary data storage method

void SqliteConnectionStringBuilder::TemporaryStorageMode::set(SqliteTemporaryStorageMode value)
{
	if(!Enum::IsDefined(SqliteTemporaryStorageMode::typeid, value)) throw gcnew ArgumentOutOfRangeException();

	DbConnectionStringBuilder::default[s_keywords[static_cast<int>(KeywordCode::TemporaryStorageMode)]] = value.ToString();
	m_tempStorageMode = value;
}

//---------------------------------------------------------------------------
// SqliteConnectionStringBuilder::TransactionMode::set
//
// Sets the option to use pseudo-nested transaction support or not

void SqliteConnectionStringBuilder::TransactionMode::set(SqliteTransactionMode value)
{
	if(!Enum::IsDefined(SqliteTransactionMode::typeid, value)) throw gcnew ArgumentOutOfRangeException();

	DbConnectionStringBuilder::default[s_keywords[static_cast<int>(KeywordCode::TransactionMode)]] = value.ToString();
	m_transactionMode = value;
}

//---------------------------------------------------------------------------
// SqliteConnectionStringBuilder::TryGetValue
//
// Attempts to retrieve the specified value, but returns true/false
// instead of throwing an exception if it does not exist
//
// Arguments:
//
//	keyword		- Keyword to be looked up
//	value		- [out] On success, contains the requested value

bool SqliteConnectionStringBuilder::TryGetValue(String^ keyword, Object^% value)
{
	KeywordCode				code;		// Keyword code looked up in dictionary
	
	value = nullptr;					// Initialize [out] variable to NULL

	if(keyword == nullptr) throw gcnew ArgumentNullException();

	// If we can find the specified keyword, return the current value, even 
	// if it's still set to a default.  Otherwise, leave it NULL and bail 

	if(s_keywordMap->TryGetValue(keyword, code)) { value = GetAt(code); return true; }
	else return false;
}

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)
