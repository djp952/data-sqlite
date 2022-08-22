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
#include "zDBMetaData.h"			// Include zDBMetaData declarations
#include "zDBConnection.h"			// Include zDBConnection declarations

#pragma warning(push, 4)			// Enable maximum compiler warnings

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// zDBMetaData::Generate (static)
//
// Generates the requested database metadata for a zDBConnection
//
// Arguments:
//
//	conn		- zDBConnection requesting a metadata generation
//	schema		- Name of the schema to generate metadata for
//	args		- Arguments (restrictions) for the specified schema

DataTable^ zDBMetaData::Generate(zDBConnection^ conn, String^ schema, array<String^>^ args)
{
	if(conn == nullptr) throw gcnew ArgumentException();
	zDBUtil::CheckConnectionReady(conn);

	// The default schema set is the "MetaDataCollection", and the default arguments,
	// or restrictions if you prefer, is an empty array of strings

	if(schema == nullptr) schema = DbMetaDataCollectionNames::MetaDataCollections;
	if(args == nullptr) args = gcnew array<String^>(0);

	// Common: MetaDataCollections
	if(String::Compare(schema, DbMetaDataCollectionNames::MetaDataCollections, true) == 0)
		return s_metadata->Tables[DbMetaDataCollectionNames::MetaDataCollections]->Copy();

	// Common: DataSourceInformation
	else if(String::Compare(schema, DbMetaDataCollectionNames::DataSourceInformation, true) == 0)
		return s_metadata->Tables[DbMetaDataCollectionNames::DataSourceInformation]->Copy();

	// Common: DataTypes
	else if(String::Compare(schema, DbMetaDataCollectionNames::DataTypes, true) == 0)
		return s_metadata->Tables[DbMetaDataCollectionNames::DataTypes]->Copy();

	// Common: Restrictions
	else if(String::Compare(schema, DbMetaDataCollectionNames::Restrictions, true) == 0)
		return s_metadata->Tables[DbMetaDataCollectionNames::Restrictions]->Copy();

	// Common: ReservedWords
	else if(String::Compare(schema, DbMetaDataCollectionNames::ReservedWords, true) == 0)
		return s_metadata->Tables[DbMetaDataCollectionNames::ReservedWords]->Copy();

	// Anything else we don't understand generates an ArgumentException ...

	else throw gcnew ArgumentException(String::Format("The requested collection ({0}) "
		"is not defined.", schema));
}

//---------------------------------------------------------------------------
// zDBMetaData::LoadEmbeddedMetaData (private, static)
//
// Loads the embedded DataSet that contains all of the static metadata
//
// Arguments:
//
//	NONE

DataSet^ zDBMetaData::LoadEmbeddedMetaData(void)
{
	Assembly^			assembly;				// This assembly reference
	Stream^				schema = nullptr;		// The DataSet schema
	Stream^				data = nullptr;			// The DataSet data
	DataSet^			metadata;				// The generated DataSet

	assembly = zDBMetaData::typeid->Assembly;	// Grab the assembly reference
	
	try {
		
		// Acquire references to both the schema as well as the data resources.  They
		// were broken up so the VS XML editor could be used to work on the data as needed

		schema = assembly->GetManifestResourceStream(zDBMetaData::typeid, METADATA_XSD_RESOURCE);
		if(schema == nullptr) throw gcnew Exception("INTERNAL FAILURE: Cannot load embedded resource");

		data = assembly->GetManifestResourceStream(zDBMetaData::typeid, METADATA_XML_RESOURCE);
		if(data == nullptr) throw gcnew Exception("INTERNAL FAILURE: Cannot load embedded resource");

		metadata = gcnew DataSet();				// Create the new data set
		metadata->ReadXmlSchema(schema);		// Apply the embedded XML schema
		metadata->ReadXml(data);				// Apply the embedded XML data		
		return metadata;						// Return the completed data set
	}

	finally { 

		if(data != nullptr) delete data;		// Dispose of the stream
		if(schema != nullptr) delete schema;	// Dispose of the stream
	}
}

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)
