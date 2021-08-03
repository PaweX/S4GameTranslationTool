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

#include "CharacterSetConverter.h"
#include "LangFileLoader.h"


CCharacterSetConverter* CCharacterSetConverter::thisObj = nullptr;
bool CCharacterSetConverter::useTranslit = false;
bool CCharacterSetConverter::useIgnore = false;

// Constructor
CCharacterSetConverter::CCharacterSetConverter(const char* tocode, const char* fromcode)
{
	strToCode = tocode;
	strFromCode = fromcode;

	if (CCharacterSetConverter::useTranslit)
		strToCode.append("//TRANSLIT");

	if (CCharacterSetConverter::useIgnore)
		strToCode.append("//IGNORE");

	descriptor = iconv_open(strToCode.c_str(), fromcode);
	if (descriptor < 0)
	{
		DestroyConverter();
		return;
	}

	thisObj = this;
}


/*bool CCharacterSetConverter::ConvertText(const char* * inbuf, size_t * inbytesleft,
											 char* * outbuf, size_t * outbytesleft)*/
bool CCharacterSetConverter::ConvertText(const char* inBuffer, size_t inbytesleft, std::string* outBuffer)
{
	if (inBuffer == "")
		return true; // don't do anything when the text is empty

	size_t outbytesleft = (inbytesleft * 3) + 1;

	char *out_str = (char*)calloc(outbytesleft, sizeof(char));
	char *out_str_start = out_str;

	int result = iconv(descriptor, &inBuffer, &inbytesleft, &out_str, &outbytesleft);

	if (result < 0) // try standard coding if the main one fails - P: usually happens when S4 authors names are loaded
	{
		if (strToCode == "UTF-8//TRANSLIT" || strToCode == "UTF-8")
			descriptor = iconv_open("UTF-8", "windows-1252");
		else if (strFromCode == "UTF-8")
			descriptor = iconv_open("windows-1252", "UTF-8");
		else
		{
			free(out_str_start);
			return false;
		}

		result = iconv(descriptor, &inBuffer, &inbytesleft, &out_str, &outbytesleft);

		descriptor = iconv_open(strToCode.c_str(), strFromCode.c_str()); // back to the main one
	}

	*outBuffer = std::string(out_str_start);

	free(out_str_start);

	if (result < 0)
		return false;
	else
		return true;
}

bool CCharacterSetConverter::ConvertText(const char* inBuffer, size_t inbytesleft, std::string* outBuffer, size_t* outbytes)
{
	if (inBuffer == "")
		return true; // don't do anything when the text is empty

	size_t outbytesleft = (inbytesleft * 3) + 1;
	*outbytes = outbytesleft; // Save out bytes

	char *out_str = (char*)calloc(outbytesleft, sizeof(char));
	char *out_str_start = out_str;

	int result = iconv(descriptor, &inBuffer, &inbytesleft, &out_str, &outbytesleft);

	if (result < 0) // try standard coding if the main one fails - P: usually happens when S4 authors names are loaded
	{
		if (strToCode == "UTF-8//TRANSLIT" || strToCode == "UTF-8")
			descriptor = iconv_open("UTF-8", "windows-1252");
		else if (strFromCode == "UTF-8")
			descriptor = iconv_open("windows-1252", "UTF-8");
		else
		{
			free(out_str_start);
			return false;
		}

		result = iconv(descriptor, &inBuffer, &inbytesleft, &out_str, &outbytesleft);

		descriptor = iconv_open(strToCode.c_str(), strFromCode.c_str()); // back to the main one
	}

	*outBuffer = std::string(out_str_start);

	*outbytes -= outbytesleft; // check how many bytes were written
	free(out_str_start);

	if (result < 0)
		return false;
	else
		return true;
}

// Static functions
const char* CCharacterSetConverter::GetLangCode(const int* langNum)
{
	switch (*langNum)
	{
		case LANG_NUM_UTF8: return "UTF-8";

		/*0*/case LANG_ENGLISH: return "windows-1252";
		/*1*/case LANG_GERMAN: return "windows-1252";
		/*2*/case LANG_FRENCH: return "windows-1252";
		/*3*/case LANG_SPANISH: return "windows-1252";
		/*4*/case LANG_ITALIAN: return "windows-1252";
		/*5*/case LANG_POLISH: return "windows-1250"; // TESTED
		/*6*/case LANG_KOREAN: return "CP949"; // P: not sure 
		/*7*/case LANG_CHINESE: return "BIG5-HKSCS";// P: very not sure 
		/*8*/case LANG_SWEDISH: return "windows-1252";
		/*9*/case LANG_DANISH: return "windows-1252";
		/*10*/case LANG_NORWEGIAN: return "windows-1252";
		/*11*/case LANG_HUNGARIAN: return "windows-1250";
		/*12*/case LANG_HEBREW: return "windows-1255"; // P: not sure but seems to be the right one
		/*13*/case LANG_CZECH: return "windows-1250";
		/*14*/case LANG_FINNISH: return "windows-1252";
		/*16*/case LANG_RUSSIAN: return "windows-1251";
		/*17*/case LANG_THAI: return "CP874"; // P: not sure, or maybe windows-28601?
		/*18*/case LANG_JAPANESE: return "CP932"; // P: not sure but seems to be the right one

		default: return "windows-1252";
	}
}

CCharacterSetConverter* CCharacterSetConverter::GetConverter(const char* tocode, const char* fromcode)
{
	if (thisObj)
	{
		if ((tocode == thisObj->strToCode.c_str()) && (fromcode == thisObj->strFromCode.c_str()))
			return thisObj;
		else
			DestroyConverter();
	}

	return thisObj = new CCharacterSetConverter(tocode, fromcode);
}

CCharacterSetConverter* CCharacterSetConverter::GetConverter(const int toLang, const int fromLang)
{
	return CCharacterSetConverter::GetConverter(GetLangCode(&toLang), GetLangCode(&fromLang));
}

CCharacterSetConverter* CCharacterSetConverter::GetConverter() // Only can be used when the converter object was already initialised 
{
	if (!thisObj)
		return nullptr;
	else
		return thisObj;
}

short CCharacterSetConverter::DestroyConverter()
{
	if (thisObj)
	{
		short err;
		err = iconv_close(thisObj->descriptor);
		delete thisObj;
		thisObj = nullptr;
		return err;
	}

	return 1;
}