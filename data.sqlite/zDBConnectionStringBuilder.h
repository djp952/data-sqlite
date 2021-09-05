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

#ifndef __ZDBCONNECTIONSTRINGBUILDER_H_
#define __ZDBCONNECTIONSTRINGBUILDER_H_
#pragma once

#include "zDBConstants.h"				// Include zDB constant declarations
#include "zDBEnumerations.h"			// Include zDB enumeration declarations
#include "zDBUtil.h"					// Include zDBUtil declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

using namespace System;
using namespace System::Collections;
using namespace System::Collections::Generic;
using namespace System::Data;
using namespace System::Data::Common;
using namespace System::Security;

BEGIN_NAMESPACE(zuki)
BEGIN_NAMESPACE(data)
BEGIN_NAMESPACE(dbms)

//---------------------------------------------------------------------------
// Class zDBConnectionStringBuilder
//
// Provides a simple way to create and manage the contents of connection
// strings used by the zDBConnection class
//---------------------------------------------------------------------------

public ref class zDBConnectionStringBuilder sealed : public DbConnectionStringBuilder
{
public:

	zDBConnectionStringBuilder();
	zDBConnectionStringBuilder(String^ connectionString);

	//-----------------------------------------------------------------------
	// Member Functions

	// Clear (DbConnectionStringBuilder)
	//
	// Removes all key/value pairs and resets all properties to default
	virtual void Clear(void) override;

	// ContainsKey (DbConnectionStringBuilder)
	//
	// Determines if a specific keyword is valid or not
	virtual bool ContainsKey(String^ keyword) override;

	// Remove (DbConnectionStringBuilder)
	//
	// Removes a key/value pair by keyword and resets its property to default
	virtual bool Remove(String^ keyword) override;

	// ShouldSerialize (DbConnectionStringBuilder)
	//
	// Determines if a specific keyword is valid or not
	virtual bool ShouldSerialize(String^ keyword) override;

	// TryGetValue (DbConnectionStringBuilder)
	//
	// Attempts to access a specific value without throwing any exceptions
	virtual bool TryGetValue(String^ keyword, Object^% value) override;

	//-----------------------------------------------------------------------
	// Properties

	// AllowExtensions = { true | false }
	//
	// Determines if dynamically loaded extension support will be enabled
	// or disabled.  Note that disabling this will also prevent the custom
	// zukisoft extensions from being enabled on the database
	property bool AllowExtensions
	{
		bool get(void) { return m_allowExtensions; }
		void set(bool value);
	}

	// AutoVacuum = { true | false }
	//
	// Determines if the SQLite database files will be automatically
	// vacuumed when they are closed
	property bool AutoVacuum
	{
		bool get(void) { return m_autoVacuum; }
		void set(bool value);
	}

	// BooleanFormat = { OneZero | NegativeOneZero | TrueFalse }
	//
	// Determines what format boolean values should be stored with
	property zDBBooleanFormat BooleanFormat
	{
		zDBBooleanFormat get(void) { return m_booleanFormat; }
		void set(zDBBooleanFormat value);
	}

	// CacheSize = { n }
	//
	// Determines the number of disk pages SQLite should hold in memory
	property int CacheSize
	{
		int get(void) { return m_cacheSize; }
		void set(int value);
	}

	// CaseSensitiveLike = { true | false }
	//
	// Determines if a case-sensitive LIKE command should be used
	property bool CaseSensitiveLike
	{
		bool get(void) { return m_caseSensitiveLike; }
		void set(bool value);
	}

	// CompatibleFileFormat = { true | false }
	//
	// Determines if a compatible file format should be used or not
	property bool CompatibleFileFormat
	{
		bool get(void) { return m_compatibleFormat; }
		void set(bool value);
	}

	// DataSource = { path }
	//
	// Provides the location of the main database instance file
	property String^ DataSource
	{
		String^ get(void) { return m_dataSource; }
		void set(String^ value);
	}

	// DateTimeFormat = { ISO8601 | Sortable | UniversalSortable | RFC1123 | Ticks }
	//
	// Determines what format date/time values should be stored with
	property zDBDateTimeFormat DateTimeFormat
	{
		zDBDateTimeFormat get(void) { return m_dateTimeFormat; }
		void set(zDBDateTimeFormat value);
	}

	// Encoding = { UTF16 | UTF8 | UTF16LittleEndian | UTF16BigEndian }
	//
	// Determines what text encoding should be used to store strings with
	property zDBTextEncodingMode Encoding
	{
		zDBTextEncodingMode get(void) { return m_textEncodingMode; }
		void set(zDBTextEncodingMode value);
	}

	// Enlist = { true | false }
	//
	// Determines if the connection should automatically enlist in distributed
	// transactions or not
	property bool Enlist
	{
		bool get(void) { return m_enlist; }
		void set(bool value);
	}

	// FieldEncryptionPassword = { password }
	//
	// Sets the field-level encryption password for this connection
	property String^ FieldEncryptionPassword
	{
		void set(String^ value);
	}

	// GuidFormat = { Binary | HexString | Hyphenated | Bracketed | Parenthetic }
	//
	// Determines what format UUIDs should be stored with
	property zDBGuidFormat GuidFormat
	{
		zDBGuidFormat get(void) { return m_guidFormat; }
		void set(zDBGuidFormat value);
	}

	// IsFixedSize (DbConnectionStringBuilder)
	//
	// Determines if this collection is of a fixed size, which it is
	virtual property bool IsFixedSize
	{
		bool get(void) override { return true; }
	}

	// PageSize = { n }
	//
	// Determines the page size to use for a new SQLite database file
	property int PageSize
	{
		int get(void) { return m_pageSize; }
		void set(int value);
	}

	// SynchronousMode = { Normal | Full | Off }
	//
	// Determines the SQLite synchronous mode, which indicates how much
	// work it does to keep things synchronized vs. performance
	property zDBSynchronousMode SynchronousMode
	{
		zDBSynchronousMode get(void) { return m_syncMode; }
		void set(zDBSynchronousMode value);
	}

	// TemporaryStorageFolder = { path }
	//
	// Determines the path where temp database files will be stored
	property String^ TemporaryStorageFolder
	{
		String^ get(void) { return m_tempStorageFolder; }
		void set(String^ value);
	}

	// TemporaryStorageMode = { Default | File | Memory }
	//
	// Determines how temp tables will be stored
	property zDBTemporaryStorageMode TemporaryStorageMode
	{
		zDBTemporaryStorageMode get(void) { return m_tempStorageMode; }
		void set(zDBTemporaryStorageMode value);
	}

	// TransactionMode = { Single | SimulateNested }
	//
	// Determines if simulated nested transactions should be used or not
	property zDBTransactionMode TransactionMode
	{
		zDBTransactionMode get(void) { return m_transactionMode; }
		void set(zDBTransactionMode value);
	}

	//-----------------------------------------------------------------------
	// Indexers

	// via String^ (DbConnectionStringBuilder)
	//
	// Gets or sets a specific connection string property by keyword
	virtual property Object^ default[String^]
	{
		Object^ get(String^ keyword) override;
		void set(String^ keyword, Object^ value) override;
	}

internal:

	//-----------------------------------------------------------------------
	// Internal Member Functions

	// GetFieldEncryptionPassword
	//
	// Retrieves the SecureString instance containing the password
	SecureString^ GetFieldEncryptionPassword(void) { return m_fieldPassword; }

private:

	// STATIC CONSTRUCTOR
	static zDBConnectionStringBuilder() { StaticConstruct(); }

	//-----------------------------------------------------------------------
	// Private Constants

	// INVALID_PAGESIZE
	//
	// Constant string used to explain why a .PageSize value was out of range
	literal String^ INVALID_PAGESIZE = "The Page Size for a database must be a "
		"power of 2 and be in the range of 512 to 8192.";

	// KeywordCode
	//
	// Defines codes for each of the valid connection stringkeywords
	enum struct KeywordCode
	{
		AllowExtensions = 0,
		AutoVacuum,
		BooleanFormat,
		CacheSize,
		CaseSensitiveLike,
		CompatibleFileFormat,
		DataSource,
		DateTimeFormat,
		Encoding,
		Enlist,
		FieldEncryptionPassword,
		GuidFormat,
		PageSize,
		SynchronousMode,
		TemporaryStorageFolder,
		TemporaryStorageMode,
		TransactionMode,
	};

	//-----------------------------------------------------------------------
	// Private Member Functions

	Object^		GetAt(KeywordCode code);
	KeywordCode	GetIndex(String^ keyword);
	void		Reset(KeywordCode code);
	void		ResetAll(void);
	static void StaticConstruct(void);

	//-----------------------------------------------------------------------
	// Member Variables

	bool						m_allowExtensions;		// ALLOW EXTENSIONS=
	bool						m_autoVacuum;			// AUTO VACUUM=
	zDBBooleanFormat			m_booleanFormat;		// BOOLEAN FORMAT=
	int							m_cacheSize;			// CACHESIZE=
	bool						m_caseSensitiveLike;	// CASE SENSITIVE LIKE=
	bool						m_compatibleFormat;		// COMPATIBLE FILE FORMAT=
	String^						m_dataSource;			// DATA SOURCE=
	zDBDateTimeFormat			m_dateTimeFormat;		// DATETIME FORMAT=
	bool						m_enlist;				// ENLIST =
	SecureString^				m_fieldPassword;		// FIELD ENCRYPTION PASSWORD =
	zDBGuidFormat				m_guidFormat;			// GUID FORMAT=
	int							m_pageSize;				// PAGE SIZE=
	zDBSynchronousMode			m_syncMode;				// SYNCHRONOUS MODE=
	String^						m_tempStorageFolder;	// TEMPORARY STORAGE FOLDER=
	zDBTemporaryStorageMode		m_tempStorageMode;		// TEMPORARY STORAGE MODE=
	zDBTextEncodingMode			m_textEncodingMode;		// ENCODING=
	zDBTransactionMode			m_transactionMode;		// TRANSACTION MODE=

	// s_keywords
	//
	// Defines all of the connection string keywords available via an enumeration.
	// This used to be string constants with embedded spaces, but I switch it to 
	// mimick what the SqlConnectionStringBuilder does internally instead.
	static initonly array<String^>^ s_keywords = gcnew array<String^> 
	{
		"Allow Extensions",
		"Auto Vacuum",
		"Boolean Format",
		"Cache Size",
		"Case Sensitive Like",
		"Compatible File Format",
		"Data Source",
		"DateTime Format",
		"Encoding",
		"Enlist",
		"Field Encryption Password",
		"Guid Format",
		"Page Size",
		"Synchronous Mode",
		"Temporary Storage Folder",
		"Temporary Storage Mode",
		"Transaction Mode"
	};

	// s_keywordMap
	//
	// Defines a static string->code mapping for all of the keywords
	static initonly Generic::Dictionary<String^, KeywordCode>^ s_keywordMap =
		gcnew Generic::Dictionary<String^, KeywordCode>(StringComparer::OrdinalIgnoreCase);
};

//---------------------------------------------------------------------------

END_NAMESPACE(dbms)
END_NAMESPACE(data)
END_NAMESPACE(zuki)

#pragma warning(pop)

#endif		// __ZDBCONNECTIONSTRINGBUILDER_H_
