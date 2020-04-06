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
#ifndef H_LANGFILELOADER
#define H_LANGFILELOADER

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
//#include <codecvt>
//#include <locale>

#include "CharacterSetConverter.h"

#define S4_LANG_FILE_NAME "s4_texts.dat" // without the number at the end

// Language numbers
enum
{
	LANG_UNKNOWN = -1,
	LANG_ENGLISH = 0,
	LANG_GERMAN,
	LANG_FRENCH,
	LANG_SPANISH,
	LANG_ITALIAN,
	LANG_POLISH,
	LANG_KOREAN,
	LANG_CHINESE,
	LANG_SWEDISH,
	LANG_DANISH,
	LANG_NORWEGIAN,
	LANG_HUNGARIAN,
	LANG_HEBREW,
	LANG_CZECH,
	LANG_FINNISH,
	LANG_RUSSIAN = 16,
	LANG_THAI,
	LANG_JAPANESE,
	LANG_LAST_POSITION
};

#define LANGUAGE_NAME_BUFFER_SIZE 10

const std::vector<std::string> languagesArray
{
	"ENGLISH",
	"GERMAN",
	"FRENCH",
	"SPANISH",
	"ITALIAN",
	"POLISH",
	"KOREAN",
	"CHINESE",
	"SWEDISH",
	"DANISH",
	"NORWEGIAN",
	"HUNGARIAN",
	"HEBREW",
	"CZECH",
	"FINNISH",
	"",
	"RUSSIAN",
	"THAI",
	"JAPANESE"
};

enum LFL_OPEN_MODE : bool
{
	LFL_LOAD,
	LFL_SAVE
};

enum // File header version
{
	FHEADER_GOLD_EDITION = 0xF,
	FHEADER_HISTORY_EDITION = 0x15
};

class CLangFileLoader
{
private:
	static CLangFileLoader* thisObj;

	CCharacterSetConverter* textConvObj;

	//int textCounter = 0; // counts each text

	bool bOnLoading = false;	// true when file is being loaded
	bool bOnSaving = false;		// true when file is being saved
	bool bFileOpened = false;	// true after file has been opened
	bool bFileLoaded = false;	// true after file has been loaded into memory
	bool bCompareProj = false;	// true when we load a project file for comparison
	std::string sFileName = "";
	std::fstream* langFile;
	std::string tempLangName; // Only for temporary storage

	char textHeader[4]; // four bytes of text header (node)
	char firstFourB[4]; // first four bytes of loaded file
	long fileLength = 0;
	unsigned importedTexts = 0;

	int fileLangNum = -1; 

	bool bIgonreTextHeaderMsg = false; // Warning that appears when a text header was recognized as invalid.
	bool bLoadExtraTexts = false; // True when we have a file loaded and we import text from another file that has larger number of texts (Usually when we import History Edition texts)
	bool bCompareNoMoreSlots = false; // True when while comparing the program text array has reached the end.
	unsigned firstExtendedTextPos; // position of a first extended text (after importing extra text at the end of file)
	
private:
	// Constructors
	CLangFileLoader(std::string sName) { sFileName = sName; thisObj = this; }

	bool OpenFile(bool mode);
	void CloseFile()
	{
		bFileOpened = false;
		if (langFile)
		{
			langFile->close();
			delete langFile;
			langFile = nullptr;
		}
	}
	bool CheckLangFileHeaderValidity();
	bool CheckProjectFileValidity();

	void UpdateProgressBar(long current);

	// Static methods
	//static void SetFile(std::string &sName); // Sets a file to work with

public:
	// Destructor
	~CLangFileLoader() { CloseFile(); CCharacterSetConverter::DestroyConverter(); }

	std::string langName; // for global purposes

	// Methods
	bool IsOnLoadingFile() { return bOnLoading; }
	bool IsOnSavingFile() { return bOnSaving; }
	//bool IsFileOpened() { return bFileOpened; }
	bool IsFileLoaded() { return bFileLoaded; }
	std::string GetFileName() { return sFileName; }
	std::string GetProjectName();
	std::string GetTextHeaderStr();

	int GetLangNumber() { return fileLangNum; }


	// Static methods
	static bool IsExisting() { return (thisObj); }
	static CLangFileLoader* GetObject() { return thisObj; }
	static bool IsCompareFileAProj() { return thisObj->bCompareProj; }
	static bool SetFileLoad(std::string sName); // Prepares things to load file
	static bool SetFileSave(std::string sName); // Prepares things to save file
	static bool SetFileCompare(std::string sName); // Prepares things to load file and imoprt missing texts
	static bool DoLoadFile();
	static bool DoSaveFile();
	static bool DoCompareS4LangFile();
	static bool DoCompareProjectFile();
	static bool DoLoadProject();
	static bool DoSaveProject();
	static bool IsFileExisting(std::string sName);
	static void SendMessage(std::string msg);
	static bool CheckLangFileNameValidity(std::string filename, bool noMsg = false);
	static bool CheckProjectFileNameValidity(std::string filename, bool noMsg = false);
	static std::string GetLangNameFromFileName(const std::string &fileName, bool setLangNum);
	static int GetLangNumFromFileName(std::string sFileName);
	static int GetLanguageNumByLangName(std::string langName);
	static std::string GetLanguageNameByLangNum(int langNum);
	//static void GetFileName(const char* &fileName);

	static void DeleteObject() { if (thisObj) delete thisObj; } // Remove the object and free the memory
};

#endif // H_LANGFILELOADER