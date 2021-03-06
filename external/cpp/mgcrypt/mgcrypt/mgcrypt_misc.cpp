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

#include <iostream>
#include <fstream>

#include <cstdio>
#include <cstdlib>
#include <filesystem>

#include "hex.h"
using CryptoPP::HexEncoder;
using CryptoPP::HexDecoder;

#include "cryptlib.h"

#include "filters.h"
using CryptoPP::StringSink;
using CryptoPP::StringSource;


#include "osrng.h"
#include "base64.h"
#include "hkdf.h"


#include "assert.h"

#include "mgcrypt_tls_objects.h"
#include "mgcrypt_misc.h"

using namespace std;
using namespace CryptoPP;


int GenerateRandom(char** pRandomResult, const unsigned int blocksize)
{
	// Generate random ...
	SecByteBlock scratch(blocksize);
	CryptoPP::AutoSeededRandomPool rng;
	rng.GenerateBlock(scratch, scratch.size());

	// Hexencode the random ...
	string hexencoded;
	HexEncoder hex(new StringSink(hexencoded));
	hex.Put(scratch, scratch.size());
	hex.MessageEnd();

	// Allocate memory for final result and copy hexencoded random to there
	*pRandomResult = new char[hexencoded.length() + 1];
	copy(hexencoded.begin(), hexencoded.end(), stdext::checked_array_iterator<char*>(*pRandomResult, hexencoded.length()));
	(*pRandomResult)[hexencoded.length()] = 0;

	return 0;
}

char* DeriveKey(char* lpszPassphrase, char* lpszIV, char* lpszKeyBuffer, size_t lKeybufferLength, size_t Keylength) {

	SecByteBlock key(Keylength);
	string password(lpszPassphrase), iv(lpszIV);

	try {
		HKDF<SHA256> hkdf;
		hkdf.DeriveKey(key, key.size(), (const byte*)password.data(), password.size(), (const byte*)iv.data(), iv.size(), NULL, 0);

		string encoded;
		CryptoPP::HexEncoder encoder;
		encoder.Attach(new CryptoPP::StringSink(encoded));
		encoder.Put(key, key.size());
		encoder.MessageEnd();

		size_t lencoded = encoded.size();
		if (lencoded + 1 <= lKeybufferLength) {
			copy(encoded.begin(), encoded.end(), stdext::checked_array_iterator<char*>(lpszKeyBuffer, lKeybufferLength));
			lpszKeyBuffer[lencoded] = 0;
		}
		else
			*lpszKeyBuffer = 0;
		return lpszKeyBuffer;

	}
	catch (const Exception& ex)
	{
		return 0;
	}
}


int Hexencode(string& rawdata, char** pEncodingResult) {

	string hexencoded;
	CryptoPP::HexEncoder encoder;
	encoder.Attach(new CryptoPP::StringSink(hexencoded));
	encoder.Put((const byte*)rawdata.data(), rawdata.size());
	encoder.MessageEnd();

	*pEncodingResult = new char[hexencoded.length() + 1];
	copy(hexencoded.begin(), hexencoded.end(), stdext::checked_array_iterator<char*>(*pEncodingResult, hexencoded.length()));
	(*pEncodingResult)[hexencoded.length()] = 0;

	return 0;
}

int Hexdecode(string& hexencoded, char** pEncodingResult, size_t& nBytesResult) {

	nBytesResult = 0;
	string hexdecoded;
	CryptoPP::HexDecoder decoder;
	decoder.Attach(new CryptoPP::StringSink(hexdecoded));
	decoder.Put((const byte*)hexencoded.data(), hexencoded.size());
	decoder.MessageEnd();

	*pEncodingResult = new char[hexdecoded.length() + 1];
	copy(hexdecoded.begin(), hexdecoded.end(), stdext::checked_array_iterator<char*>(*pEncodingResult, hexdecoded.length()));
	(*pEncodingResult)[hexdecoded.length()] = 0;
	nBytesResult = hexdecoded.length();

	return 0;
}

int GetTempfilename(wchar_t* szTempfilename) {
	wchar_t szTempPathBuffer[MAX_PATH];

	//  Gets the temp path env string (no guarantee it's a valid path).
	DWORD dwRetVal = GetTempPathW(MAX_PATH, szTempPathBuffer);
	if (dwRetVal > MAX_PATH || (dwRetVal == 0))
		return 0;
	
	//  Generates a temporary file name. 
	UINT uRetVal = GetTempFileNameW(szTempPathBuffer, L"mgcrypt_", 0, szTempfilename);
	if (uRetVal == 0)
		return 0;

	return uRetVal;
}

int WriteBlobToTempfile(const std::vector<BYTE> &buffer, wstring& filename) {
	wchar_t szTempFileName[MAX_PATH];

	int ndxTempfile = GetTempfilename(&szTempFileName[0]);
	if (ndxTempfile)
		filename = szTempFileName;
	else
		return 0;

	std::ofstream fout(szTempFileName, std::ios::out | std::ios::binary);
	fout.write((char *)buffer.data(), buffer.size() * sizeof(BYTE));
	fout.close();

	return 0;
}
