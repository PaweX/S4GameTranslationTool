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
#ifndef H_SETTINGSFILE
#define H_SETTINGSFILE

#define SETTINGS_FILE_NAME "settings.cfg"

enum SF_OPEN_MODE
{
	SF_LOAD,
	SF_ADD,
	SF_REBUILD
};

//extern std::fstream* settingsFile; // settings file
//extern bool bIsOpened;
//extern SF_OPEN_MODE openMode;

class CSettingsFile
{
private:
	static std::fstream* settingsFile; // settings file
	static bool bIsOpened;
	static SF_OPEN_MODE openMode;

private:
	CSettingsFile() {} // constructor

	// Static methods
	static void NewSettingsFile()
	{
		if (settingsFile)
		{
			return;
		}

		// Create new settingsFile object
		settingsFile = new std::fstream();
	}
	static bool OpenSettingsFile(SF_OPEN_MODE mode);
	static void CloseSettingsFile()
	{
		if (settingsFile)
		{
			settingsFile->close();
			delete settingsFile;
			settingsFile = nullptr;
			bIsOpened = false;
		}
	}

public:
	// --Settings storage-- // - some settings are stored here

	// -------------------- //

	// Static methods
	static bool GetVarNumber(std::string setting, int &state);
	static bool AddVarNumber(std::string setting, int &state); // used for adding new settings
	static bool SaveVarNumber(std::string setting, int &state); // used for saving all settings
	static void Done() { CloseSettingsFile(); }
};

#endif // H_SETTINGSFILE