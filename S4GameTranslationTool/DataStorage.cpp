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

#include "DataStorage.h"
#ifdef _DEBUG
#include <iostream>
#endif

CDataStorage* CDataStorage::thisObj = nullptr;
bool CDataStorage::isExisting = false;


void CDataStorage::PushText(char* text, unsigned position)
{
	text_storage.push_back(text);
	position_storage.push_back(position);
}

bool CDataStorage::GetText(char* &text, unsigned position)
{
	for (unsigned i = 0; i < position_storage.size(); ++i)
	{
		if (position_storage[i] == position)
		{
			text = text_storage[i];
			position = position_storage[i];
			return true;
		}
	}

	return false;
}

bool CDataStorage::IsTextPosStored(unsigned position)
{
	for (unsigned i = 0; i < position_storage.size(); ++i)
	{
		if (position_storage[i] == position)
		{
			return true;
		}
	}

	return false;
}

void CDataStorage::InitIgnoreSave(unsigned size)
{
	if (ignoreSave_storage)
		delete[] ignoreSave_storage; // delete old one

	ignoreSave_storage = new bool[size];

#ifdef _DEBUG
	ignoreSaveSize = size;
#endif

	for (unsigned i = 0; i < size; ++i)
		ignoreSave_storage[i] = false;

	ignoreSaveReady = true;
}

void CDataStorage::ExtendIgnoreSave(unsigned size) // new size
{
#ifdef _DEBUG
	if (!ignoreSaveReady)
	{
		std::cout << std::endl << "__ERRORR__ CDataStorage::ExtendIgnoreSave: ignoreSave_storage not initialized!!!" << std::endl;
		return;
	}
#endif

	if (ignoreSaveSize >= size)
	{
#ifdef _DEBUG
		std::cout << std::endl << "__ERRORR__ CDataStorage::ExtendIgnoreSave: size is not increasing!" << std::endl;
#endif
		return;
	}

	bool* temp_storage = new bool[size];

	unsigned i;
	for (i = 0; i < ignoreSaveSize; ++i)
		temp_storage[i] = ignoreSave_storage[i];

	for (; i < size; ++i)
		temp_storage[i] = false;

	delete[] ignoreSave_storage;
	ignoreSave_storage = temp_storage;

	ignoreSaveSize = size;
}

void CDataStorage::SetIgnoreSave(unsigned position, bool ignore)
{
#ifdef _DEBUG
	if (!ignoreSaveReady)
	{
		std::cout << std::endl << "__ERRORR__ CDataStorage::SetIgnoreSave: ignoreSave_storage not initialized!!!" << std::endl;
		return;
	}

	if (position >= ignoreSaveSize) // safety check
		std::cout << std::endl << "__ERRORR__ CDataStorage::SetIgnoreSave: saving out of array!" << std::endl;
#endif

	ignoreSave_storage[position] = ignore;
}

bool CDataStorage::GetIgnoreSave(unsigned position)
{
#ifdef _DEBUG
	if (!ignoreSaveReady)
	{
		std::cout << std::endl << "__ERRORR__ CDataStorage::GetIgnoreSave: ignoreSave_storage not initialized!!!" << std::endl;
		return false;
	}

	if (position >= ignoreSaveSize) // safety check
		std::cout << std::endl << "__ERRORR__ CDataStorage::GetIgnoreSave: accessing out of array!" << std::endl;
#endif

	return ignoreSave_storage[position];
}

CDataStorage* CDataStorage::Get()
{
	if (!isExisting)
		new CDataStorage();

	return thisObj;
}