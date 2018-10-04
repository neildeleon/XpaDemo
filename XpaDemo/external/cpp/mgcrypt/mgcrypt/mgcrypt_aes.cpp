//
// Copyright (c) 2018 Andreas Sedlmeier (sedlmeier@hotmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// 
#include "stdafx.h"

#include <string>
using std::string;

#include "hex.h"
using CryptoPP::HexEncoder;
using CryptoPP::HexDecoder;

#include "cryptlib.h"
using CryptoPP::BufferedTransformation;
using CryptoPP::AuthenticatedSymmetricCipher;

#include "filters.h"
using CryptoPP::StringSink;
using CryptoPP::StringSource;
using CryptoPP::AuthenticatedEncryptionFilter;
using CryptoPP::AuthenticatedDecryptionFilter;

#include "aes.h"
using CryptoPP::AES;

#include "gcm.h"
using CryptoPP::GCM;
using CryptoPP::GCM_TablesOption;

#include "assert.h"

#include "mgcrypt_tls_objects.h"
#include "mgcrypt_aes.h"

using namespace std;
using namespace CryptoPP;

int AES_Encrypt (const char* lpszPlaintext, long lpdwPlaintextLength, char* lpszKey, char* lpszMode, char**lpszCiphertext, char* lpszIV) {

	string plaintext(lpszPlaintext, lpdwPlaintextLength);
	string ciphertext;

	//KEY 0000000000000000000000000000000000000000000000000000000000000000
	//IV  000000000000000000000000
	//HDR 00000000000000000000000000000000
	//PTX 00000000000000000000000000000000
	//CTX cea7403d4d606b6e074ec5d3baf39d18
	//TAG ae9b1771dba9cf62b39be017940330b4

	// Test Vector 003
	byte key[32];
	memset(key, 0, sizeof(key));
	byte iv[12];
	memset(iv, 0, sizeof(iv));

	string adata(16, (char)0x00);

	const int TAG_SIZE = 16;

	// Encrypted, with Tag
	string cipher, encoded;


	try
	{
		GCM< AES >::Encryption e;
		e.SetKeyWithIV(key, sizeof(key), iv, sizeof(iv));
		// Not required for GCM mode (but required for CCM mode)
		// e.SpecifyDataLengths( adata.size(), pdata.size(), 0 );

		StringSource ss1(plaintext, true,
			new AuthenticatedEncryptionFilter(e,
				new StringSink(cipher), false, TAG_SIZE
			) // AuthenticatedEncryptionFilter
		); // StringSource


		// Pretty print
		StringSource(cipher, true,
			new HexEncoder(new StringSink(encoded), true, 16, " "));
	}
	catch (CryptoPP::BufferedTransformation::NoChannelSupport& e)
	{
	}
	catch (CryptoPP::AuthenticatedSymmetricCipher::BadState& e)
	{
	}
	catch (CryptoPP::InvalidArgument& e)
	{
	}


	*lpszCiphertext = new char[encoded.length() + 1];
	copy(encoded.begin(), encoded.end(), stdext::checked_array_iterator<char*>(*lpszCiphertext, encoded.length()));
	(*lpszCiphertext)[encoded.length()] = 0;

	return 0;

}