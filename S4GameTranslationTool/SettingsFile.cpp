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
#include <iostream>
#include <fstream>
#include <string>
#include "SettingsFile.h"

std::fstream* CSettingsFile::settingsFile; // settings file
bool CSettingsFile::bIsOpened;
SF_OPEN_MODE CSettingsFile::openMode;

bool CSettingsFile::OpenSettingsFile(SF_OPEN_MODE mode)
{
	if (mode == SF_LOAD)
	{
		settingsFile->open(SETTINGS_FILE_NAME, std::ios::in); // for loading
		openMode = SF_LOAD;
	}
	else if (mode == SF_ADD)
	{
		settingsFile->open(SETTINGS_FILE_NAME, std::ios::out | std::ios::app); // for saving
		openMode = SF_ADD;
	}
	else
	{
		settingsFile->open(SETTINGS_FILE_NAME, std::ios::out); // for saving
		openMode = SF_REBUILD;
	}

	if (!settingsFile->good())
	{
		CloseSettingsFile();
		return false;
	}

	bIsOpened = true;
	return true;
}

bool CSettingsFile::GetVarNumber(std::string setting, int &state)
{
	NewSettingsFile();
	if (!bIsOpened)
	{
		if (!OpenSettingsFile(SF_LOAD))
			return false;
	}
	else if (openMode != SF_LOAD)
	{
		CloseSettingsFile();
		if (!OpenSettingsFile(SF_LOAD))
			return false;
	}

	while (!settingsFile->eof())
	{
		std::string line;
		getline(*settingsFile, line);

		std::string settingName(line, 0, setting.length());

		if (settingName == setting)
		{
			int length = setting.length() + 1;
			settingName = std::string(line, length, line.length() - length);

			state = std::stoi(settingName);
			return true;
		}
	}

	return false;
}

bool CSettingsFile::AddVarNumber(std::string setting, int &state) // Adds a setting at the end of file
{
	NewSettingsFile();
	if (!bIsOpened)
	{
		if (!OpenSettingsFile(SF_ADD))
			return false;
	}
	else if (openMode == SF_LOAD)
	{
		CloseSettingsFile();
		if (!OpenSettingsFile(SF_ADD))
			return false;
	}

	*settingsFile << setting << " " << state << std::endl;

	return true;
}

bool CSettingsFile::SaveVarNumber(std::string setting, int &state) // Rebuilds whole file
{
	NewSettingsFile();
	if (!bIsOpened)
	{
		if (!OpenSettingsFile(SF_REBUILD))
			return false;
	}
	else if (openMode == SF_LOAD)
	{
		CloseSettingsFile();
		if (!OpenSettingsFile(SF_REBUILD))
			return false;
	}

	*settingsFile << setting << " " << state << std::endl;

	return true;
}

