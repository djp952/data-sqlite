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

#ifndef __ZDBCRYPTOKEY_H_
#define __ZDBCRYPTOKEY_H_
#pragma once

#pragma warning(push, 4)				// Enable maximum compiler warnings

using namespace System;
using namespace System::ComponentModel;
using namespace System::Runtime::InteropServices;
using namespace System::Security;
using namespace System::Security::Cryptography;
using namespace System::Text;

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// Class zDBCryptoKey (internal)
//
// zDBCryptoKey is used to generate and store the cryptography key(s) used
// with the ENCRYPT() and DECRYPT() scalar functions built into the provider.
//
// Note that this is a carry-over from the 1.1 provider in order to maintain
// compatibility with both it and the Windows CE version of it.  It has been
// modified slightly to use SecureString, but otherwise it's pretty much an
// identical implementation. for better or worse.
//---------------------------------------------------------------------------

ref class zDBCryptoKey
{
public:

	//-----------------------------------------------------------------------
	// Constructor / Destructor

	zDBCryptoKey(SecureString^ password);

	//-----------------------------------------------------------------------
	// Properties

	// Key
	//
	// Returns the HCRYPTKEY handle
	property HCRYPTKEY Key
	{
		HCRYPTKEY get(void) { return reinterpret_cast<HCRYPTKEY>(m_ptKey.ToPointer()); }
	}

private:

	// DESTRUCTOR / FINALIZER
	~zDBCryptoKey() { this->!zDBCryptoKey(); m_disposed = true; }
	!zDBCryptoKey();

	//-----------------------------------------------------------------------
	// Private Member Functions

	// CreatePrivateExponentOneKey
	//
	// Used for key generation -- see code
	static BOOL CreatePrivateExponentOneKey(LPCTSTR szProvider, DWORD dwProvType, 
		LPCTSTR szContainer, DWORD dwKeySpec, HCRYPTPROV *hProv, HCRYPTKEY *hPrivateKey);
		
	// ImportPlainSessionBlob
	//
	// Used for key generation -- see code
	static BOOL ImportPlainSessionBlob(HCRYPTPROV hProv, HCRYPTKEY hPrivateKey, 
		ALG_ID dwAlgId, const BYTE* pbKeyMaterial, DWORD dwKeyMaterial, HCRYPTKEY *hSessionKey);

	//-----------------------------------------------------------------------
	// Member Variables

	bool					m_disposed;			// Object disposal flag
	IntPtr					m_ptProv;			// Really HCRYPTPROV
	IntPtr					m_ptPrivKey;		// Really HCRYPTKEY
	IntPtr					m_ptKey;			// Really HCRYPTKEY
};

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)

#endif	// __ZDBCRYPTOKEY_H_
