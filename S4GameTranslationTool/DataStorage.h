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
#pragma once
#ifndef H_DATASTORAGE
#define H_DATASTORAGE

#include <vector>

class CDataStorage
{
private:
	static CDataStorage* thisObj;
	static bool isExisting;

	std::vector<char*> text_storage;
	std::vector<unsigned> position_storage;

	bool* ignoreSave_storage; // used for marking which texts should be ignored while saving a file (e.g. imported texts)
	bool ignoreSaveReady = false;

	unsigned ignoreSaveSize = 0;

private:
	CDataStorage() // constructor
	{
		if (!thisObj)
		{
			thisObj = this;
			isExisting = true;
		}
	}
	~CDataStorage()
	{
		text_storage.clear();
		position_storage.clear();
		if (ignoreSave_storage)
			delete[] ignoreSave_storage;
	}

public:
	void PushText(char* text, unsigned position);
	bool GetText(char* &text, unsigned position);
	bool IsTextPosStored(unsigned position);
	void ClearTextPosStorage() { text_storage.clear(); position_storage.clear(); }
	
	void InitIgnoreSave(unsigned size);
	void ExtendIgnoreSave(unsigned size);
	void SetIgnoreSave(unsigned position, bool ignore);
	bool GetIgnoreSave(unsigned position);

	// static methods
	static bool IsExisting() { return isExisting; }
	static CDataStorage* Get();
	static void Destroy() { if (thisObj) delete thisObj; }
};

#endif // H_DATASTORAGE