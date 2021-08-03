#pragma once
////////////////////////////////////////////////////////////
//////////// Settlers IV Game Translation Tool /////////////
//// By Pawe³ C. (PaweX3) - https://pawex3.blogspot.com ////
////////////////////////////////////////////////////////////
/*******************************************************************************
* Copyright (c) 2019 Pawe³ Czapliñski
*
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
* DEALINGS IN THE SOFTWARE.
*******************************************************************************/
#ifndef H_CHARSETCONV
#define H_CHARSETCONV

#include "iconv.h"
#include <string>

#define LANG_NUM_UTF8 -10


class CCharacterSetConverter
{
private:
	static CCharacterSetConverter* thisObj;
	iconv_t descriptor;

	std::string strToCode;
	std::string strFromCode;

	// Constructors
	CCharacterSetConverter(const char* tocode, const char* fromcode);
	~CCharacterSetConverter() {};

	// Methods

public:
	// Methods
	bool ConvertText(const char* inBuffer, size_t inbytesleft, std::string* outBuffer);
	bool ConvertText(const char* inBuffer, size_t inbytesleft, std::string* outBuffer, size_t* outbytesleft);

	// Static fields
	static bool useTranslit;
	static bool useIgnore;

	// Static methods
	static const char* GetLangCode(const int* langNum);
	static CCharacterSetConverter* GetConverter(const char* tocode, const char* fromcode);
	static CCharacterSetConverter* GetConverter(const int toLang, const int fromLang);
	static CCharacterSetConverter* GetConverter(); // used when the converter is already created
	static short DestroyConverter();

};


#endif // H_CHARSETCONV