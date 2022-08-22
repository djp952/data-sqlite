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
#include "SqliteCryptoKey.h"			// Include SqliteCryptoKey declarations

#pragma warning(push, 4)			// Enable maximum compiler warnings

namespace zuki::data::sqlite {

//---------------------------------------------------------------------------
// SqliteCryptoKey Constructor
//
// Arguments:
//
//	password		- The password to be hashed into an encryption key

SqliteCryptoKey::SqliteCryptoKey(SecureString^ password)
{
	IntPtr					ptPassword;			// Pointer to decrypted password
	wchar_t*				pwszPassword;		// Pointer to decrypted password
	size_t					cchPassword;		// Password length, in characters
	array<System::Byte>^	rgPassword;			// Password yet again, as a byte array
	PinnedBytePtr			pinPassword;		// You guessed it .. the password
	array<System::Byte>^	rgHash;				// Hashed password (128 bits)
	PinnedBytePtr			pinHash;			// Pinned password hash
	HCRYPTPROV				hProv;				// CryptoAPI Context handle
	HCRYPTKEY				hPrivKey;			// CryptoAPI private key
	HCRYPTKEY				hKey;				// CryptoAPI public key (???)

	// The first thing we need to do is get the actual password from the SecureString.
	// I'm not entirely sure how this is supposed to help that much, since you can 
	// still get in here via reflection, but I guess it's better than nothing.  The
	// use of SecureString should at least give the impression that I tried :)

	ptPassword = Marshal::SecureStringToGlobalAllocUnicode(password);

	try {
		
		// Create an MD5 hash of the provided password, which generates us
		// a 128 bit BLOB of key data we can pass into CryptoAPI.  Note all of
		// the hoops to jump through since we need to take the unmanaged string
		// and put it back into a managed byte array for Encoding to use it.
		// (Again, this probably wasn't worth the aggravation involved)

		MD5CryptoServiceProvider^ md5 = gcnew MD5CryptoServiceProvider();
		
		pwszPassword = reinterpret_cast<wchar_t*>(ptPassword.ToPointer());
		cchPassword = wcslen(pwszPassword);
		rgPassword = gcnew array<System::Byte>(Encoding::Unicode->GetByteCount(pwszPassword, 
			static_cast<int>(cchPassword)));

		if(cchPassword) {
			
			pinPassword = &rgPassword[0];
			Encoding::Unicode->GetBytes(pwszPassword, static_cast<int>(cchPassword), pinPassword, 
				rgPassword->Length);
		}

		rgHash = md5->ComputeHash(rgPassword);		// Compute the MD5 hash
		pinHash = &rgHash[0];						// And pin it down in memory

		// If we had a password array pinned, clear out the data to ensure some
		// level of security in here

		if(cchPassword) memset(pinPassword, 0, rgPassword->Length);

		try {

			// Use the utility function to create ourselves an "exponent one key",
			// whatever on Earth that means.  Encryption isn't really my thing.

			if(!CreatePrivateExponentOneKey(MS_ENHANCED_PROV, PROV_RSA_FULL, NULL, 
				AT_KEYEXCHANGE, &hProv, &hPrivKey)) throw gcnew Win32Exception(GetLastError());

			// Now take the "exponent one key" information in conjunction with the
			// hashed password and create ourselves the magical encryption key through
			// whatever means of Voodoo actually goes on in here ...

			if(!ImportPlainSessionBlob(hProv, hPrivKey, CALG_3DES_112, pinHash,
				/*rgHash->Length*/ 16, &hKey)) {

				DWORD dwError = GetLastError();		// Save off the last error code
				CryptDestroyKey(hPrivKey);			// Destroy the private key
				CryptReleaseContext(hProv, 0);		// Release CryptoAPI context

				throw gcnew Win32Exception(dwError);
			}
		}

		finally { memset(pinHash, 0, rgHash->Length); }		// Clear out the MD5 hash bits

		// All went well .. store the handles and whatnot into the IntPtrs.  It
		// would probably be good to see if we need to really keep all of these
		// around or not, but that's AFTER it all works properly ...

		m_ptProv = IntPtr(reinterpret_cast<void*>(hProv));
		m_ptPrivKey = IntPtr(reinterpret_cast<void*>(hPrivKey));
		m_ptKey = IntPtr(reinterpret_cast<void*>(hKey));
	}
	
	finally { Marshal::ZeroFreeGlobalAllocUnicode(ptPassword); }
}

//---------------------------------------------------------------------------
// SqliteCryptoKey Finalizer

SqliteCryptoKey::!SqliteCryptoKey()
{
	// Destroy all of the stored key information and release the context

	if(m_ptKey != IntPtr::Zero) CryptDestroyKey(reinterpret_cast<HCRYPTKEY>(m_ptKey.ToPointer()));
	if(m_ptPrivKey != IntPtr::Zero)	CryptDestroyKey(reinterpret_cast<HCRYPTKEY>(m_ptPrivKey.ToPointer()));
	if(m_ptProv != IntPtr::Zero) CryptReleaseContext(reinterpret_cast<HCRYPTPROV>(m_ptProv.ToPointer()), 0);

	m_ptKey = IntPtr::Zero;			// In case of object ressurection
	m_ptPrivKey = IntPtr::Zero;		// In case of object ressurection
	m_ptProv = IntPtr::Zero;		// In case of object ressurection
}

//---------------------------------------------------------------------------
// SqliteCryptoKey::CreatePrivateExponentOneKey (static, private)
//
// Creates an RSA private key with an exponent of one.  Adapted from MSKB
// article q228786 "How to export and import plain text session keys by
// using CryptoAPI".  Taken from newsgroup microsoft.public.security.crypto
// in a thread by Simon Waterer, May 28 2005
//
// NOTE: Your guess is as good as mine as to how any of this works.  Perhaps
// someday I'll come back in here, clean this up, and learn what it all does
//
// Arguments:
//
//	<< See MSKB Q228786 for more information >>

BOOL SqliteCryptoKey::CreatePrivateExponentOneKey(LPCTSTR szProvider, DWORD dwProvType, 
	LPCTSTR szContainer, DWORD dwKeySpec, HCRYPTPROV *hProv, HCRYPTKEY *hPrivateKey) 
{ 

	BOOL fReturn = FALSE; 
	BOOL fResult; 
	ULONG n; 
	LPBYTE keyblob = NULL; 
	DWORD dwkeyblob; 
	DWORD dwBitLen; 
	BYTE *ptr; 

	*hProv = 0; 
	*hPrivateKey = 0; 

	if ((dwKeySpec != AT_KEYEXCHANGE) && (dwKeySpec != AT_SIGNATURE)) return FALSE; 

	// Try to create new container 
	fResult = CryptAcquireContext(hProv, szContainer, szProvider, dwProvType, CRYPT_NEWKEYSET); 
	if(!fResult) 
	{ 
		// If the container exists, open it 
		if(GetLastError() == NTE_EXISTS) 
		{ 
			fResult = CryptAcquireContext(hProv, szContainer, szProvider, dwProvType, 0);
		} 
		if(!fResult) 
		{ 
			// No good, leave 
			return FALSE; 
		} 
	} 

	// Generate the private key 
	fResult = CryptGenKey(*hProv, dwKeySpec, CRYPT_EXPORTABLE, hPrivateKey); 
	if(fResult) 
	{ 
		// Export the private key, we'll convert it to a private 
		// exponent of one key 
		fResult = CryptExportKey(*hPrivateKey, 0, PRIVATEKEYBLOB, 0, NULL, &dwkeyblob); 
		if(fResult) 
		{ 
			keyblob = new BYTE[dwkeyblob]; 
			if(keyblob) 
			{ 
				// needs zero fill otherwise doesn't work 
				memset(keyblob, 0, dwkeyblob); 
				fResult = CryptExportKey(*hPrivateKey, 0, PRIVATEKEYBLOB, 0, keyblob, &dwkeyblob); 
				if(fResult) 
				{ 
					CryptDestroyKey(*hPrivateKey); 
					*hPrivateKey = 0; 

					// Get the bit length of the key 
					memcpy(&dwBitLen, &keyblob[12], 4); 

					// Modify the Exponent in Key BLOB format 
					// Key BLOB format is documented in SDK 

					// Convert pubexp in rsapubkey to 1 
					ptr = &keyblob[16]; 
					for (n = 0; n < 4; n++) 
					{ 
						if (n == 0) ptr[n] = 1; 
						else ptr[n] = 0; 
					} 

					// Skip pubexp 
					ptr += 4; 
					// Skip modulus, prime1, prime2 
					ptr += (dwBitLen/8); 
					ptr += (dwBitLen/16); 
					ptr += (dwBitLen/16); 

					// Convert exponent1 to 1 
					for (n = 0; n < (dwBitLen/16); n++) 
					{ 
						if (n == 0) ptr[n] = 1; 
						else ptr[n] = 0; 
					} 

					// Skip exponent1 
					ptr += (dwBitLen/16); 

					// Convert exponent2 to 1 
					for (n = 0; n < (dwBitLen/16); n++) 
					{ 
						if (n == 0) ptr[n] = 1; 
						else ptr[n] = 0; 
					} 

					// Skip exponent2, coefficient 
					ptr += (dwBitLen/16); 
					ptr += (dwBitLen/16); 

					// Convert privateExponent to 1 
					for (n = 0; n < (dwBitLen/8); n++) 
					{ 
						if (n == 0) ptr[n] = 1; 
						else ptr[n] = 0; 
					} 

					// Import the exponent-of-one private key. 
					if(CryptImportKey(*hProv, keyblob, dwkeyblob, 0, CRYPT_NO_SALT, hPrivateKey)) 
					{ 
						fReturn = TRUE; 
					} 
				} 
			} 
		} 
	} 

	if(keyblob) delete[] keyblob; 

	if(!fReturn) 
	{ 
		if (*hPrivateKey) CryptDestroyKey(*hPrivateKey); 
		if (*hProv) CryptReleaseContext(*hProv, 0); 
	} 
	
	return fReturn; 
} 

//---------------------------------------------------------------------------
// SqliteCryptoKey::ImportPlainSessionBlob (static, private)
//
// Imports a plaintext session key into the CSP.  Adapted from MSKB article
// q228786 "How to export and import plain text session keys by using CryptoAPI".
// Taken from newsgroup microsoft.public.security.crypto in a thread by Simon 
// Waterer, May 28 2005
//
// NOTE: Your guess is as good as mine as to how any of this works.  Perhaps
// someday I'll come back in here, clean this up, and learn what it all does
//
// Arguments:
//
//	<< See MSKB Q228786 for more information >>

BOOL SqliteCryptoKey::ImportPlainSessionBlob(HCRYPTPROV hProv, HCRYPTKEY hPrivateKey, 
	ALG_ID dwAlgId, const BYTE* pbKeyMaterial, DWORD dwKeyMaterial, HCRYPTKEY *hSessionKey)
{ 
	BOOL fResult; 
	BOOL fReturn = FALSE; 
	LPBYTE pbSessionBlob = NULL; 
	DWORD dwSessionBlob, dwSize, n; 
	DWORD dwPublicKeySize; 
	DWORD dwProvSessionKeySize; 
	ALG_ID dwPrivKeyAlg; 
	LPBYTE pbPtr; 
	HCRYPTKEY hTempKey = 0; 

	// We have to get the key size(including padding) 
	// from an HCRYPTKEY handle.  PP_ENUMALGS_EX contains 
	// the key size without the padding so we can't use it. 
	fResult = CryptGenKey(hProv, dwAlgId, CRYPT_NO_SALT, &hTempKey); 
	if(fResult) 
	{ 
		dwSize = sizeof(DWORD); 
		fResult = CryptGetKeyParam(hTempKey, KP_KEYLEN, (LPBYTE)&dwProvSessionKeySize, &dwSize, 0); 
		if(fResult) 
		{ 
			CryptDestroyKey(hTempKey); 
			hTempKey = 0; 

			if ((dwKeyMaterial * 8) <= dwProvSessionKeySize) 
			{ 
				// Get private key's algorithm 
				dwSize = sizeof(ALG_ID); 
				fResult = CryptGetKeyParam(hPrivateKey, KP_ALGID, (LPBYTE)&dwPrivKeyAlg, &dwSize, 0); 
				if(fResult) 
				{ 
					// Get private key's length in bits 
					dwSize = sizeof(DWORD); 
					fResult = CryptGetKeyParam(hPrivateKey, KP_KEYLEN, (LPBYTE)&dwPublicKeySize, &dwSize, 0); 
					if(fResult) 
					{ 
						// calculate Simple blob's length 
						dwSessionBlob = (dwPublicKeySize/8) + sizeof(ALG_ID) + sizeof(BLOBHEADER); 

						// allocate simple blob buffer 
						pbSessionBlob = new BYTE[dwSessionBlob]; 
						if(pbSessionBlob) 
						{ 
							// needs zero fill otherwise doesn't work 
							memset(pbSessionBlob, 0, dwSessionBlob); 

							pbPtr = pbSessionBlob; 

							// SIMPLEBLOB Format is documented in SDK 
							// Copy header to buffer 
							((BLOBHEADER *)pbPtr)->bType = SIMPLEBLOB; 
							((BLOBHEADER *)pbPtr)->bVersion = 2; 
							((BLOBHEADER *)pbPtr)->reserved = 0; 
							((BLOBHEADER *)pbPtr)->aiKeyAlg = dwAlgId; 
							pbPtr += sizeof(BLOBHEADER); 

							// Copy private key algorithm to buffer 
							*((DWORD *)pbPtr) = dwPrivKeyAlg; 
							pbPtr += sizeof(ALG_ID); 

							// Place the key material in reverse order 
							for (n = 0; n < dwKeyMaterial; n++) 
							{ 
								pbPtr[n] = pbKeyMaterial[dwKeyMaterial-n-1]; 
							}

							// 3 is for the first reserved byte after the key material + the 2 reserved bytes at the end. 
							dwSize = dwSessionBlob - (sizeof(ALG_ID) + sizeof(BLOBHEADER) + dwKeyMaterial + 3); 
							pbPtr += (dwKeyMaterial+1); 

							// Generate random data for the rest of the buffer 
							// (except that last two bytes) 
							fResult = CryptGenRandom(hProv, dwSize, pbPtr); 
							if(fResult) 
							{ 
								for (n = 0; n < dwSize; n++) 
								{ 
									if (pbPtr[n] == 0) pbPtr[n] = 1; 
								} 
								pbSessionBlob[dwSessionBlob - 2] = 2; 

								fResult = CryptImportKey(hProv, pbSessionBlob , dwSessionBlob, hPrivateKey, 0, hSessionKey); 
								if(fResult) 
								{ 
									fReturn = TRUE; 
								} 
							} 
						} 
					} 
				} 
			} 
		} 
	} 

	if (hTempKey) CryptDestroyKey(hTempKey); 
	if (pbSessionBlob) delete[] pbSessionBlob; 
	
	return fReturn;
} 

//---------------------------------------------------------------------------

} // zuki::data::sqlite

#pragma warning(pop)
