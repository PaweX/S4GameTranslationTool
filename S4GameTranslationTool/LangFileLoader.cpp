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

#include "LangFileLoader.h"
#include "DataStorage.h"
#include "GtkApp.h"
#include <filesystem>


CLangFileLoader* CLangFileLoader::thisObj = nullptr;


bool CLangFileLoader::OpenFile(bool mode)
{
	// Check if langFile object is existing
	if (langFile)
	{
		SendMessage("Error. Tryig to create a new langFile object, but old one still exists!");

#ifndef _DEBUG
		CloseFile();
#endif

		return false;
	}

	// Create new langFile object
	langFile = new std::fstream();

	if (mode == LFL_LOAD)
	{
		// Open one of the Txt/s4_texts.dat* files
		langFile->open(sFileName, std::ios::in | std::ios_base::binary); // for loading

		if (!langFile->good())
		{
			SendMessage(("Loading failed. Can't access file: " + sFileName));
			if (langFile)
				CloseFile();

			return false;
		}
		else
			bFileOpened = true;

		// Save length for the progress bar
		langFile->seekg(0, langFile->end);
		fileLength = static_cast<long>(langFile->tellg());
	}
	else
	{
		// Save to one of the Txt/s4_texts.dat* files, or to a new file
		langFile->open(sFileName, std::ios::out | std::ios_base::binary); // for saving

		if (!langFile->good())
		{
			SendMessage(("Saving failed. Can't access file " + sFileName));
			if (langFile)
				CloseFile();

			return false;
		}
		else
			bFileOpened = true;
	}

	return true;
}

bool CLangFileLoader::CheckLangFileHeaderValidity() // Checks if the loaded file is correct S4 lang file
{
	// Check header
	if (( (textHeader[0] != FHEADER_GOLD_EDITION && textHeader[0] != FHEADER_HISTORY_EDITION) || textHeader[1] != 0 || textHeader[2] != 0 || textHeader[3] != 0))
	{
		if (GtkApp::AskUserAQuestion("The header of the file was recognized as invalid.\nDo you want to load the file anyway (NOT RECOMMENDED! - unless you know what you are doing)?"))
			return true;
		else
		{
			SendMessage("The file had wrong header: " + std::to_string(textHeader[0]) + " " + std::to_string(textHeader[1]) + " " + std::to_string(textHeader[2]) + " " + std::to_string(textHeader[3]) + " in decimal.");
			return false;
		}
	}

	return true;
}

bool CLangFileLoader::CheckLangFileNameValidity(std::string filename, bool noMsg)
{
	// Check the name
	int i = filename.length() - 3;
	if (i > 0)
	{
		if ((filename.at(i) == 't') || (filename.at(++i) == 't'))
		{
			std::string justFileName(filename, i - 11, 12);
			if (justFileName == S4_LANG_FILE_NAME)
			{
				return true;
			}
		}

	}

	if (!noMsg)
	{
		if (GtkApp::AskUserAQuestion("The file name was not recognized as a valid S4 language file name. Do you want to proceed anyway?"))
			return true;

		SendMessage("The S4 language file name was incorrect. Expected 's4_texts.dat<number>' e.g. 's4_texts.dat0'.");
	}

	return false;
}

bool CLangFileLoader::CheckProjectFileNameValidity(std::string filename, bool noMsg)
{
	int a = filename.length();
	int b = sizeof(S4TT_PROJECT_FILE_EXTENSION)-1;

	// Check the file extension
	if (a > sizeof(S4TT_PROJECT_FILE_EXTENSION))
	{
		a -= b;
		if (std::string(filename, a, b) == S4TT_PROJECT_FILE_EXTENSION)
			return true;
	}

	if (!noMsg)
	{
		if (GtkApp::AskUserAQuestion("The project name was not recognized as a valid S4GTT project file name. Do you want to proceed anyway?"))
			return true;

		SendMessage("The S4GTT project name was incorrect. Expected '<LANGUAGE_NAME>.s4_translation_project'.");
	}

	return false;
}

bool CLangFileLoader::CheckProjectFileValidity()
{
	char data[4];

	thisObj->langFile->seekg(0, std::ios::beg);
	thisObj->langFile->read(data, 4);

	if (strncmp(data, "****", 4) == 0) // first check
	{
		bool twostars = false;
		while (!thisObj->langFile->eof())
		{
			thisObj->langFile->read(data, 1);
			if (data[0] == '*')
			{
				thisObj->langFile->seekg(-1, std::ios::cur);
				thisObj->langFile->read(data, 4);

				if (strncmp(data, "****", 4) == 0) // second check
				{
					return twostars;
				}
				else if (strncmp(data, "**", 2) == 0)
				{
					twostars = true;
				}
			}
		}
	}

	return false;
}

void CLangFileLoader::UpdateProgressBar(long current)
{
	double total = fileLength;
	GtkApp::SetProgressBar(static_cast<float>((current / total)));
}

std::string CLangFileLoader::GetProjectName()
{
	return (langName + S4TT_PROJECT_FILE_EXTENSION);
}

std::string CLangFileLoader::GetTextHeaderStr()
{
	std::string hexStr;
	
	char const hex_chars[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	
	for (int i = 0; i < 4; ++i)
	{
		hexStr.append(&hex_chars[(textHeader[i] & 0xF0) >> 4], 1);
		hexStr.append(&hex_chars[textHeader[i] & 0xF], 1);

		//if (i < 3)
			hexStr.append(" "); // add space for clarity
	}

	return hexStr;
}


// Static methods
bool CLangFileLoader::SetFileLoad(std::string sName) // Prepares things to load file
{
	if (sName == "") // check string
	{
		SendMessage("File name is empty!");
		return false;
	}
	else if (thisObj && (thisObj->bOnLoading || thisObj->bOnSaving))
	{
		SendMessage("Can't load now. A file is already being processed.");
		return false;
	}
	else
		SendMessage("Preparing to open " + sName);

	// If the object was not created yet, create it
	if (!CLangFileLoader::thisObj)
		CLangFileLoader::thisObj = new CLangFileLoader(sName);
	else if (thisObj->bFileOpened) // Check if a file is opened
	{
		thisObj->CloseFile(); // close file
		thisObj->bFileLoaded = false;
		thisObj->sFileName = sName; // set new file name
	}
	else
	{
		thisObj->bFileLoaded = false;
		thisObj->sFileName = sName; // set new file name
	}

	return thisObj->OpenFile(LFL_LOAD);
}

// Prepares things to save file
bool CLangFileLoader::SetFileSave(std::string sName) // This function assumes that thisObj is not null.
{
	// P: This first 'if' is already placed in GtkApp.cpp, so I commented it out.
	/*if (!CLangFileLoader::thisObj || !thisObj->bFileLoaded)	
	{														
		SendMessage("No file to be saved.");
		return false;
	}
	else*/ if (thisObj->bOnLoading || thisObj->bOnSaving)
	{
		SendMessage("Can't save now. A file is already being processed.");
		return false;
	}
#ifdef _DEBUG
	else if (sName == "") // If string is empty, use stored name instead
	{
		std::cout << std::endl << "__ERRORR__ CLangFileLoader::SetFileSave: empty file name!!!" << std::endl;
		sName = thisObj->sFileName;
	}
#endif

	// If the object was not created yet, create it
	/*if (!CLangFileLoader::thisObj)
		CLangFileLoader::thisObj = new CLangFileLoader(sName);
	else*/ if (thisObj->bFileOpened) // Check if a previous file is still opened
	{
		thisObj->CloseFile(); // close file
		thisObj->sFileName = sName; // set new file name
	}
	else
	{
		thisObj->sFileName = sName; // set new file name
	}

	return thisObj->OpenFile(LFL_SAVE);
}

bool CLangFileLoader::SetFileCompare(std::string sName) // This function assumes that thisObj is not null.
{
	if (!thisObj)
		return false;
	else if (sName == "") // check string
	{
		SendMessage("Can't compare. File name is empty!");
		return false;
	}
	else if ((thisObj->bOnLoading || thisObj->bOnSaving))
	{
		SendMessage("Can't compare now. A file is already being processed.");
		return false;
	}
#ifdef _DEBUG
	else
	{ 
		SendMessage("Preparing to import missing texts " + sName);
	}
#endif

	if (!CLangFileLoader::CheckLangFileNameValidity(sName, true))
	{
		if (!CLangFileLoader::CheckProjectFileNameValidity(sName, true))
		{
			if (!GtkApp::AskUserAQuestion("The file name was not recognised as a valid S4 Language File name or S4GTT project file name.\nDo you want to try to load the file as S4GTT project file?"))
				return false;
			else
				thisObj->bCompareProj = true;
		}
		else
			thisObj->bCompareProj = true;
	}
	else
		thisObj->bCompareProj = false;

	if (thisObj->bFileOpened) // Check if a file is opened
	{
		thisObj->CloseFile(); // close file
		thisObj->sFileName = sName; // set new file name
	}
	else
	{
		thisObj->sFileName = sName; // set new file name
	}

	return thisObj->OpenFile(LFL_LOAD);
}


bool CLangFileLoader::DoLoadFile()
{
	if (!thisObj)
	{
		SendMessage("Can't load the file. CLangFileLoader object was not created!");
		goto fail;
	}
	else if (!thisObj->bFileOpened)
	{
		//SendMessage("Loading failed. None file is opened!");
		goto fail;
	}
	/*else if (thisObj->bFileLoaded) // Check if the file has been loaded
	{
		thisObj->CloseFile();
		return false;
	}*/
	else if (!thisObj->bOnLoading)
	{
		SendMessage(("Loading file: " + thisObj->sFileName));


		// Read first four characters of the file
		thisObj->langFile->seekg(0, std::ios::beg);
		thisObj->langFile->read(thisObj->textHeader, 4); // for CheckLangFileValidity()

		if (!thisObj->CheckLangFileHeaderValidity())
		{
		fail:
			thisObj->bOnLoading = false;
			thisObj->bFileLoaded = false;
			thisObj->CloseFile(); // we can close the file

			return false;
		}

		thisObj->bOnLoading = true; // Set on-loading flag to true

		thisObj->langName = GetLangNameFromFileName(thisObj->sFileName, true);

		// Setup the converter
		if (GtkApp::Set::libiconvEncoding)
		{
			thisObj->textConvObj = CCharacterSetConverter::GetConverter(LANG_NUM_UTF8, GetObject()->fileLangNum);
		}

		std::string langName = ("**** " + thisObj->langName + " ****");

		// Send data to GtkApp
		GtkApp::LangDataAddToArray_utf8conv(langName.c_str(), langName.length()
#ifdef USE_HEX_COLUMN
								  ,thisObj->GetTextHeaderStr().c_str()
#endif
		);

		++GtkApp::uTxtPos;
		thisObj->bIgonreTextHeaderMsg = false;
	}


	// Read data

	// check if finished
	if (thisObj->langFile->tellg() >= thisObj->fileLength/*thisObj->langFile->eof()*/)
	{
		// save again first four characters of the file
		thisObj->langFile->seekg(0, std::ios::beg);
		thisObj->langFile->read(thisObj->firstFourB, 4);

		// do the rest
		thisObj->bOnLoading = false;
		thisObj->bFileLoaded = true;
		thisObj->CloseFile(); // we can close the file

		// Destroy the converter
		/*if (GtkApp::Set::libiconvEncoding)
		{
			CCharacterSetConverter::DestroyConverter();
		}*/

		GtkApp::SetProgressBar(1);
		SendMessage("Loading completed.");
		GtkApp::SetLanguageFlag(thisObj->fileLangNum);
		return false;
	}

	//char c;
	unsigned numChars;
	char* pchString;

	// Reads one text per cycle (one index, or in GtkApp one row)
	{
		thisObj->UpdateProgressBar(static_cast<long>(thisObj->langFile->tellg()));

		// Read text header
		thisObj->langFile->read(thisObj->textHeader, 4);
		//SendMessage(std::to_string(thisObj->langFile->tellg()));

		// security check: check if we have a correct header
		if ((thisObj->textHeader[2] != 0 || thisObj->textHeader[3] != 0) && !thisObj->bIgonreTextHeaderMsg)
		{
			SendMessage("Error! Wrong text header. Position of the text is " + std::to_string(GtkApp::uTxtPos));

			std::string msg = "A text header of a text that is currently being loaded was recognized as incorrect.\nDo you want to stop loading the file (RECOMENDED)?\nThe text position is " + std::to_string(GtkApp::uTxtPos);
			if(GtkApp::AskUserAQuestion(msg.c_str()))
			{
				thisObj->CloseFile(); // we can close the file
				thisObj->bOnLoading = false;
				thisObj->bFileLoaded = GtkApp::AskUserAQuestion("Do you want to view all texts that were loaded up to this point?\nRemember that the whole file has not been loaded and the last text is most likely damaged!");

				return false;
			}

			thisObj->bIgonreTextHeaderMsg = true;
		}

		// Read how many characters are in the next text
		numChars = static_cast<unsigned>(static_cast<unsigned char>(thisObj->textHeader[0]));
		if (thisObj->textHeader[1] != 0)
		{
			numChars += (thisObj->textHeader[1] * 256);
		}

		// security check: check the maximum size of numChars
		if (numChars > 256 * 256)
		{
			SendMessage("Error! Text array out of size! ");

			//LOADING_FAILED:

			goto fail;
		}

		if (numChars == 0)
		{
			GtkApp::LangDataAddToArray_utf8conv("", NULL
#ifdef USE_HEX_COLUMN
											, thisObj->GetTextHeaderStr().c_str()
#endif
			);

			++GtkApp::uTxtPos; // Increase the text position
			return true;
		}

		pchString = new char[numChars];

		thisObj->langFile->read(pchString, numChars);

		// Remove all \r from the text
		for (unsigned i = 0; i < numChars; ++i)
		{
			if (pchString[i] == '\r')
			{
				for (unsigned j = i+1; j < numChars; ++j)
				{
					if (pchString[j] != '\r')
					{
						pchString[i] = pchString[j];
						++i;
					}
				}

				numChars = i;
				break;
			}
		}

		//std::string str(pchString, numChars);

		// Send data to GtkApp
		GtkApp::LangDataAddToArray_utf8conv(pchString, numChars
#ifdef USE_HEX_COLUMN
								  ,thisObj->GetTextHeaderStr().c_str()
#endif
									);

		delete[] pchString;
	}

	++GtkApp::uTxtPos; // Increase the text position

	return true;
}

bool CLangFileLoader::DoSaveFile()
{
	if (!thisObj->bFileOpened)
	{
		SendMessage("Can't save the file as it was not initialized.");
	
		thisObj->bOnSaving = false;
		thisObj->CloseFile(); // we can close the file

		return false;

	}
	else if (!thisObj->bOnSaving)
	{
		// Save first four characters from our storage
		thisObj->langFile->seekg(0, std::ios::beg);
		thisObj->langFile->write(thisObj->firstFourB, 4);

		// Change flag and sent a msg
		thisObj->bOnSaving = true; // Set on-loading flag to true
		SendMessage(("Saving file: " + thisObj->sFileName));

		// Setup the converter
		if (GtkApp::Set::libiconvEncoding)
		{
			thisObj->textConvObj = CCharacterSetConverter::GetConverter(GetLangNumFromFileName(thisObj->sFileName), LANG_NUM_UTF8);
		}

		// Reset texts positions
		GtkApp::uTxtPos = 0;
	}


	// Save data
	++GtkApp::uTxtPos; // increase at the beginning so 0 is missed
	std::string textBuffer;
	char* newTextBuffer;
	unsigned amountOfChar = 0;
	unsigned newAmountOfChar = 0;
	bool newbuffer = false;

	bool finish;

	if (CDataStorage::Get()->GetIgnoreSave(GtkApp::uTxtPos))
		finish = !GtkApp::LangDataNextRow();
	else
	{
		finish = !GtkApp::LangDataGetFromArray(textBuffer, amountOfChar); // get text
	}

	// P: TODO: when loading an different char set than the local one, textBuffer is null here. I need to write my own utf8 to ansi function // UPDATE: P: Test again with the new function, code was changed
	if (textBuffer == "") // New code: if "" not if NULL and textBuffer is std::string here now
	{
		if (amountOfChar > 0)
		{
			SendMessage("Error: textBuffer == \"\" but amountOfChar is not 0! Conversion Error?");
			
		fail:
			thisObj->bOnSaving = false;
			thisObj->CloseFile(); // we can close the file

			return false;
		}

		thisObj->textHeader[0] = 0;
		thisObj->textHeader[1] = 0;
		thisObj->langFile->write(thisObj->textHeader, 4); // write text header

		goto end_process;
	}
	
	// Add \r before all \n
	{
		newAmountOfChar = amountOfChar;
		for (unsigned k = 0; k < amountOfChar; ++k) // count all /n first
		{
			// increase the char num for each \n (will be used for \r as they are together with \n)
			if (textBuffer.at(k) == '\n')
				++newAmountOfChar;
		}

		if (newAmountOfChar != amountOfChar)
		{
			newTextBuffer = new char[newAmountOfChar];
			newbuffer = true;

			int j = 0;
			for (unsigned i = 0; i < amountOfChar; ++i)
			{
				if (textBuffer.at(i) == '\n')
				{
					newTextBuffer[j] = '\r';
					j++;
				}

				newTextBuffer[j] = textBuffer.at(i);
				j++;
			}
		}
		else
			newTextBuffer = const_cast<char*>(textBuffer.c_str());
	}

	thisObj->textHeader[0] = (newAmountOfChar % 256); // rest
	thisObj->textHeader[1] = (newAmountOfChar / 256); // multiplier
	thisObj->langFile->write(thisObj->textHeader, 4); // write text header

	thisObj->langFile->write(newTextBuffer, newAmountOfChar); // write text

	if (newbuffer)
		delete[] newTextBuffer;

	end_process:

	// Update progress bar
	thisObj->UpdateProgressBar(static_cast<long>(thisObj->langFile->tellg()));

	// check if finished
	if (finish)
	{
		// do the rest
		thisObj->bOnSaving = false;
		thisObj->CloseFile(); // we can close the file

		/*if (GtkApp::Set::libiconvEncoding)
		{
			CCharacterSetConverter::DestroyConverter();
		}*/

		SendMessage("Saving completed.");
		return false;
	}

	return true;
}

bool CLangFileLoader::DoCompareS4LangFile()
{
	if (!thisObj->bOnLoading)
	{
		// reset flags
		thisObj->bIgonreTextHeaderMsg = false;
		thisObj->bCompareNoMoreSlots = false;
		thisObj->bLoadExtraTexts = false;
		thisObj->firstExtendedTextPos = 0;

		// Read first four characters of the file
		thisObj->langFile->seekg(0, std::ios::beg);
		thisObj->langFile->read(thisObj->textHeader, 4);

		if (!thisObj->CheckLangFileHeaderValidity())
		{
		fail:
			thisObj->bOnLoading = false;
			thisObj->CloseFile(); // we can close the file

			return false;
		}

		// Check if we are importing History Edition texts to a Gold Edition lang file - GE files are shorter
		if (thisObj->firstFourB[0] == FHEADER_GOLD_EDITION && thisObj->textHeader[0] == FHEADER_HISTORY_EDITION)
		{
			if (GtkApp::AskUserAQuestion("It seems you are importing texts from S4 History Edition. The language files in that S4 edition have more text positions than the files from the S4 Gold Edition.\nDo you want to change this file version to History Edition one ?"))
			{
				thisObj->firstFourB[0] = thisObj->textHeader[0];
				thisObj->bLoadExtraTexts = true;
			}
			//else
			//	thisObj->bLoadExtraTexts = GtkApp::AskUserAQuestion("Do you want to add extra text positions to this file?");
		}

		thisObj->bOnLoading = true; // Set on-loading flag to true
		
		thisObj->tempLangName = GetLangNameFromFileName(thisObj->sFileName, false);

		// Setup the converter
		if (GtkApp::Set::libiconvEncoding)
		{
			thisObj->textConvObj = CCharacterSetConverter::GetConverter(LANG_NUM_UTF8, GetLanguageNumByLangName(thisObj->tempLangName));
		}

		SendMessage(("Importing missing texts from " + thisObj->tempLangName + " language file."));
		thisObj->importedTexts = 0;
	}

	///////////////
	// Read data //
	///////////////

	// Check if finished
	if (thisObj->langFile->tellg() >= thisObj->fileLength/*thisObj->langFile->eof()*/)
	{
	comparing_done:

		if (thisObj->firstExtendedTextPos != 0) // If we have extra texts, mark them as imported
		{
			// first extend the DataStorage array (ignoreSave_storage)
			CDataStorage::Get()->ExtendIgnoreSave(GtkApp::uTxtPos);

			for (unsigned i = thisObj->firstExtendedTextPos; i < GtkApp::uTxtPos; ++i)
			{
				// Mark as imported
				GtkApp::LangDataMarkRowWithNum(CHANGE_MARK_IMPOTRED, i);
			}
		}

		SendMessage("Comparing completed. " + std::to_string(thisObj->importedTexts) + " texts imported from " + thisObj->tempLangName);
		GtkApp::SetProgressBar(1);

		thisObj->bOnLoading = false;
		thisObj->CloseFile(); // we can close the file

		/*if (GtkApp::Set::libiconvEncoding)
		{
			CCharacterSetConverter::DestroyConverter();
		}*/
		
		return false;
	}

	//char c;
	unsigned numChars;
	char* pchString;

	// Reads one text per cycle (one index, or in GtkApp one row)
	{
		thisObj->UpdateProgressBar(static_cast<long>(thisObj->langFile->tellg()));

		// Read text header
		thisObj->langFile->read(thisObj->textHeader, 4);

		// security check: check if we have a correct header
		if ((thisObj->textHeader[2] != 0 || thisObj->textHeader[3] != 0) && !thisObj->bIgonreTextHeaderMsg)
		{
			SendMessage("Error! Wrong text header. Position of the text is " + std::to_string(GtkApp::uTxtPos));

			std::string msg = "A text header of a text that is currently being loaded was recognized as incorrect.\nDo you want to stop loading the file (RECOMENDED)?\nThe text position is " + GtkApp::uTxtPos;
			msg.append("\n\nPlease check also the last loaded text as it is most likely damaged.");
			if (GtkApp::AskUserAQuestion(msg.c_str()))
				goto fail;

			thisObj->bIgonreTextHeaderMsg = true;
		}

		// Read how many characters are in the next text
		numChars = static_cast<unsigned>(static_cast<unsigned char>(thisObj->textHeader[0]));
		if (thisObj->textHeader[1] != 0)
		{
			numChars += (thisObj->textHeader[1] * 256);
		}

		// security check: check the maximum size of numChars
		if (numChars > 256 * 256)
		{
			// clean the mess
			thisObj->bOnLoading = false;
			thisObj->bFileLoaded = false;
			thisObj->CloseFile(); // we can close the file

			SendMessage("Error! Text array out of size! " + numChars);
			goto fail;
		}

		if (numChars == 0) // no text in the source file
		{
			if (!thisObj->bCompareNoMoreSlots)
			{
				if (!GtkApp::LangDataNextRow())
				{
					//goto comparing_done; // we finished
					thisObj->bCompareNoMoreSlots = true;
				}
			}
			else // if we are here, we assume that the bLoadExtraTexts == true
			{
				if (!thisObj->bLoadExtraTexts)
				{
					// Ask if we finish now
					if (GtkApp::AskUserAQuestion("The main file has reached the end but there are more texts that can be imported.\nDo you want to extend the number of texts in the main file and import the remaining texts?"))
					{
						thisObj->bLoadExtraTexts = true;
					}
					else
						goto comparing_done;
				}

				GtkApp::LangDataAddToArray_utf8conv("", NULL
#ifdef USE_HEX_COLUMN
													, thisObj->GetTextHeaderStr().c_str()
#endif
													);


			}
		}
		// Check if we need to skip this text. We skip when the loaded file is not missing it
		else if (!thisObj->bCompareNoMoreSlots && !GtkApp::LangDataCheckRowIfEmpty()) // check if not empty
		{
			if (!GtkApp::LangDataNextRow())
			{
				//goto comparing_done; // we finished
				thisObj->bCompareNoMoreSlots = true;
			}

			thisObj->langFile->seekg(numChars, std::ios::cur);
		}
		else
		{
			pchString = new char[numChars];

			thisObj->langFile->read(pchString, numChars);
			//std::string str(pchString, numChars);

			// Remove all \r from the text
			for (unsigned i = 0; i < numChars; ++i)
			{
				if (pchString[i] == '\r')
				{
					for (unsigned j = i + 1; j < numChars; ++j)
					{
						if (pchString[j] != '\r')
						{
							pchString[i] = pchString[j];
							++i;
						}
					}

					numChars = i;
					break;
				}
			}


			if (thisObj->bCompareNoMoreSlots)
			{
				if (thisObj->bLoadExtraTexts)
				{
				add_text:
					if (thisObj->firstExtendedTextPos == 0)
						thisObj->firstExtendedTextPos = GtkApp::uTxtPos; // save position number of the first extended text

					// Send data to GtkApp
					GtkApp::LangDataAddToArray_utf8conv(pchString, numChars
#ifdef USE_HEX_COLUMN
														, thisObj->GetTextHeaderStr().c_str()
#endif
														);

					// Mark as imported
					//GtkApp::LangDataMarkRow(CHANGE_MARK_IMPOTRED); // P: Moved from here, see comparing_done node
				}
				// Ask if we finish now
				else if (GtkApp::AskUserAQuestion("The main file has reached the end but there are more texts that can be imported.\nDo you want to extend the number of texts in the main file and import the remaining texts?"))
				{
					thisObj->bLoadExtraTexts = true;
					goto add_text;
				}
				else
					goto comparing_done;
			}
			else
			{
				// Send data to GtkApp
				GtkApp::LangDataSwapText_utf8conv(pchString, numChars);

				// Mark as imported
				GtkApp::LangDataMarkRow(CHANGE_MARK_IMPOTRED);

				if (!GtkApp::LangDataNextRow())
				{
					thisObj->bCompareNoMoreSlots = true;
				}

			}

			++thisObj->importedTexts;
			delete[] pchString;
		}

		++GtkApp::uTxtPos; // Increase the text position
	}

	return true;
}

bool CLangFileLoader::DoCompareProjectFile()
{
	if (!thisObj->bOnLoading)
	{
		if (!thisObj->CheckProjectFileValidity())
		{
			SendMessage(("Project file invalid! " + thisObj->sFileName));
			goto fail;

		project_damaged:
			SendMessage(("Project file damaged! " + thisObj->sFileName));

		fail:
			thisObj->bOnLoading = false;
			thisObj->CloseFile(); // we can close the file
			return false;
		}


		char data[4];
		// Ignore the file title
		thisObj->langFile->seekg(4, std::ios::beg); // we skip first four stars (*)
		for (int i = 0; i < 2; ++i)
		{
			while (true)
			{
				if (thisObj->langFile->eof())
					goto project_damaged;

				thisObj->langFile->read(data, 1);
				if (data[0] == '*')
				{
					thisObj->langFile->seekg(-1, std::ios::cur);
					thisObj->langFile->read(data, 4);

					if (strncmp(data, "****", 4) == 0) // second check
					{
						break;
					}
				}
			}
		}

		// Get the Language name
		while (data[0] != ':') // skip the " Language"
		{
			if (thisObj->langFile->eof())
				goto project_damaged;

			thisObj->langFile->read(data, 1);
		}
		thisObj->langFile->seekg(-1, std::ios::cur); // go back

		thisObj->langFile->read(data, 2); // do extra check
		if (strncmp(data, ": ", 2) != 0)
			goto project_damaged; // when extra check fails

		char langNameBuffer[LANGUAGE_NAME_BUFFER_SIZE];
		thisObj->langFile->read(langNameBuffer, LANGUAGE_NAME_BUFFER_SIZE);

		for (int j = 0; j < LANGUAGE_NAME_BUFFER_SIZE; ++j)
		{
			if (langNameBuffer[j] == ' ')
			{
				thisObj->tempLangName = std::string(langNameBuffer, j);
				thisObj->langFile->seekg(j - LANGUAGE_NAME_BUFFER_SIZE + 1, std::ios::cur);
				break;
			}
		}

		thisObj->bOnLoading = true; // Set on-loading flag to true

		SendMessage(("Importing missing texts from " + thisObj->tempLangName + " language file."));
		thisObj->importedTexts = 0;
	}

	// Read data

	// check if finished
	if (thisObj->langFile->tellg() >= thisObj->fileLength/*thisObj->langFile->eof()*/)
	{
	comparing_done:

		SendMessage("Comparing completed. " + std::to_string(thisObj->importedTexts) + " texts imported from " + thisObj->tempLangName);
		GtkApp::SetProgressBar(1);

		thisObj->bOnLoading = false;
		thisObj->CloseFile(); // we can close the file

		return false;
	}


	// Reads one text per cycle (one index, or in GtkApp one row)
	{
		thisObj->UpdateProgressBar(static_cast<long>(thisObj->langFile->tellg()));

		std::string line;
		std::string str = "";

		// Read how many characters are in the next text
		while (true)
		{
			if (thisObj->langFile->eof())
				goto project_damaged;

			getline(*thisObj->langFile, line);

			if (strncmp(line.c_str(), "## Text", 7) == 0)
				break;
		}

		getline(*thisObj->langFile, line);
		if (line != "####")
		{
			str.append(line);

			while (true)
			{
				if (thisObj->langFile->eof())
					goto project_damaged;

				getline(*thisObj->langFile, line);

				if (line != "####")
					str.append("\n" + line);
				else
					break;
			}
		}

		// Check if we need to skip this text. We skip when the loaded file is not missing it
		if (str == "" || !GtkApp::LangDataCheckRowIfEmpty()) // check if not empty
		{
			if (!GtkApp::LangDataNextRow())
			{
				goto comparing_done; // we finished
			}
		}
		else
		{
			// Send data to GtkApp
			GtkApp::LangDataSwapText(str.c_str());

			// Mark as imported
			GtkApp::LangDataMarkRow(CHANGE_MARK_IMPOTRED);

			++thisObj->importedTexts;

			if (!GtkApp::LangDataNextRow())
			{
				goto comparing_done; // we finished
			}
		}

		++GtkApp::uTxtPos; // Increase the text position
	}

	return true;
}

bool CLangFileLoader::DoLoadProject()
{
	if (!thisObj)
	{
		SendMessage("Can't load the project file. CLangFileLoader object was not created!");

	fail:
		thisObj->bOnLoading = false;
		thisObj->bFileLoaded = false;
		thisObj->CloseFile(); // we can close the file
		return false;
	}
	else if (!thisObj->bFileOpened)
	{
		//SendMessage("Loading failed. None file is opened!");
		goto fail;
	}
	/*else if (thisObj->bFileLoaded) // Check if the file has been loaded
	{
		thisObj->CloseFile();
		return false;
	}*/
	else if (!thisObj->bOnLoading)
	{
		SendMessage(("Loading project: " + thisObj->sFileName));


		if (!thisObj->CheckProjectFileValidity())
		{
			SendMessage(("Project file invalid! " + thisObj->sFileName));
			goto fail;

			project_damaged:
			SendMessage(("Project file damaged! " + thisObj->sFileName));
			goto fail;
		}


		char data[4];
		// Ignore the file title
		thisObj->langFile->seekg(4, std::ios::beg); // we skip first four stars (*)
		for (int i = 0; i < 2; ++i)
		{
			while (true)
			{
				if (thisObj->langFile->eof())
					goto project_damaged;

				thisObj->langFile->read(data, 1);
				if (data[0] == '*')
				{
					thisObj->langFile->seekg(-1, std::ios::cur);
					thisObj->langFile->read(data, 4);

					if (strncmp(data, "****", 4) == 0) // second check
					{
						break;
					}
				}
			}
		}

		// Get the Language name
		while (data[0] != ':') // skip the " Language"
		{
			if (thisObj->langFile->eof())
				goto project_damaged;

			thisObj->langFile->read(data, 1);
		}
		thisObj->langFile->seekg(-1, std::ios::cur); // go back

		thisObj->langFile->read(data, 2); // do extra check
		if (strncmp(data, ": ", 2) != 0)
			goto project_damaged; // when extra check fails

		char langNameBuffer[LANGUAGE_NAME_BUFFER_SIZE];
		thisObj->langFile->read(langNameBuffer, LANGUAGE_NAME_BUFFER_SIZE);

		for (int j = 0; j < LANGUAGE_NAME_BUFFER_SIZE; ++j)
		{
			if (langNameBuffer[j] == ' ')
			{
				thisObj->langName = std::string(langNameBuffer, j);
				thisObj->fileLangNum = GetLanguageNumByLangName(thisObj->langName);
				thisObj->langFile->seekg(j - LANGUAGE_NAME_BUFFER_SIZE+1, std::ios::cur);
				break;
			}
		}

		// Get the header
		{
			thisObj->langFile->read(data, 4);
			int length = static_cast<int>(thisObj->langFile->tellg());

			if (strncmp(data, "** @", 4) == 0)
			{
				//thisObj->langFile->read(thisObj->firstFourB, 4);
				do
				{
					if (thisObj->langFile->eof())
						goto project_damaged;

					thisObj->langFile->read(data, 1);
				} while (data[0] != '@');
				/*thisObj->langFile->read(data, 1);
				{
					if (data[0] != '@')
						goto project_damaged;
				}*/
			}
			else
				goto project_damaged;

			length = static_cast<int>(-length + (thisObj->langFile->tellg()));
			thisObj->langFile->seekg(-length, std::ios::cur);
			std::string strBuffer;
			char* bufferHeader = new char[length];
			thisObj->langFile->read(bufferHeader, length);
			strBuffer = bufferHeader;

			int index = 0, offset = 0;
			for (int k = 0; k < length;)
			{
				if (bufferHeader[k] == ' ' || bufferHeader[k] == '@')
				{
					thisObj->firstFourB[index++] = std::stoi(std::string(strBuffer, offset, (k - offset)));
					offset = ++k;
				}
				else
					++k;
			}
			delete[] bufferHeader;

			thisObj->langFile->seekg(5, std::ios::cur); // go after " ****"
		}

		// Set reading position at first '#'
		while (data[0] != '#')
		{
			if (thisObj->langFile->eof())
				goto project_damaged;

			thisObj->langFile->read(data, 1);
		}
		thisObj->langFile->seekg(-1, std::ios::cur); // go back 1


		// Other stuff
		thisObj->bOnLoading = true; // Set on-loading flag to true

		std::string langName = ("**** " + thisObj->langName + " ****");

		// Send data to GtkApp
		GtkApp::LangDataAddToArray_utf8conv(langName.c_str(), langName.length()
#ifdef USE_HEX_COLUMN
								   , thisObj->GetTextHeaderStr().c_str()
#endif
		);

		++GtkApp::uTxtPos;
	}


	// Read data

	// check if finished
	if (thisObj->langFile->tellg() >= thisObj->fileLength/*thisObj->langFile->eof()*/)
	{
		thisObj->bOnLoading = false;
		thisObj->bFileLoaded = true;
		thisObj->CloseFile(); // we can close the file

		GtkApp::SetProgressBar(1);
		SendMessage("Loading project completed.");
		GtkApp::SetLanguageFlag(thisObj->fileLangNum);
		return false;
	}


	// Reads one text per cycle (one index, or in GtkApp one row)
	{
		thisObj->UpdateProgressBar(static_cast<long>(thisObj->langFile->tellg()));


		std::string line;
		std::string str = "";

		/*char data[1];

		// Skip the text description
		thisObj->langFile->seekg(8, std::ios::cur); // skip "# Text 0" or "## Text "
		while (data[0] != '\n') // skip the " Language"
		{
			if (thisObj->langFile->eof())
				goto project_damaged;

			thisObj->langFile->read(data, 1);
		}*/

		while (true)
		{
			if (thisObj->langFile->eof())
				goto project_damaged;

			getline(*thisObj->langFile, line);

			if (strncmp(line.c_str(), "## Text", 7) == 0) // if true
			{
				do // check if the new text starts with 'enter' character and remove it if present
				{
					getline(*thisObj->langFile, line);
				} while (line == "" || line == "\r");

				break;
			}
		}

		if (strncmp(line.c_str(), "####", 4) != 0) // when the strings are different
		{
			str.append(line);

			while (true)
			{
				if (thisObj->langFile->eof())
					goto project_damaged;

				getline(*thisObj->langFile, line);

				if (strncmp(line.c_str(), "####", 4) != 0) // when the strings are different
					str.append("\n" + line);
				else
					break;
			}
		}


		// Send data to GtkApp
		GtkApp::LangDataAddToArray(str.c_str()
#ifdef USE_HEX_COLUMN // P: doesn't work here, needs to be done
								   , thisObj->GetTextHeaderStr().c_str() 
#endif
		);

		//delete[] textBuffer;
	}

	++GtkApp::uTxtPos; // Increase the text position

	return true;
}

bool CLangFileLoader::DoSaveProject()
{
	if (!thisObj->bFileOpened)
	{
		SendMessage("Can't save the file as it was not initialized.");

	fail:
		thisObj->bOnSaving = false;
		thisObj->CloseFile(); // we can close the file
		return false;
	}
	else if (!thisObj->bOnSaving)
	{
		//if (!thisObj->CheckLangFileValidity(true)) P: Don't check it for project names
		//	return false;

		// Write the first line (project description) to the file
		thisObj->langFile->seekg(0, std::ios::beg);

		// Title 1 - the main title and program version
		const char* projTitlePart1 = u8"**** Settlers IV Game Translation Tool Project File ** Version ";
		const char* projTitlePart2 = u8" ****\n";
		thisObj->langFile->write(projTitlePart1, strlen(projTitlePart1)); // write first part
		thisObj->langFile->write(S4GTT_PROGRAM_VERSION, strlen(S4GTT_PROGRAM_VERSION)); // write version
		thisObj->langFile->write(projTitlePart2, strlen(projTitlePart2)); // write second part

		// Title 2 - language and file header
		const char* projTitle2Part1 = u8"**** Language: ";
		thisObj->langFile->write(projTitle2Part1, strlen(projTitle2Part1)); // write first part
		thisObj->langFile->write(thisObj->langName.c_str(), thisObj->langName.length()); // write language name
		const char* projTitle2Part2 = u8" ** @";
		thisObj->langFile->write(projTitle2Part2, strlen(projTitle2Part2)); // write second part
		// write header
		{
			std::string header = "";
			for (int i = 0; i < 4; ++i)
			{
				if (i == 3)
					header.append(std::to_string(thisObj->firstFourB[i]));
				else
					header.append(std::to_string(thisObj->firstFourB[i]) + " ");
			}

			thisObj->langFile->write(header.c_str(), header.length());
		}
		const char* projTitle2Part3 = u8"@ ****\n";
		thisObj->langFile->write(projTitle2Part3, strlen(projTitle2Part3)); // write third part


		// Change flag and sent a msg
		thisObj->bOnSaving = true; // Set on-loading flag to true
		SendMessage(("Saving project: " + thisObj->sFileName));

		// Reset texts positions
		GtkApp::uTxtPos = 0;
	}


	// Save data
	++GtkApp::uTxtPos; // increase at the beginning so 0 is missed
	char* textBuffer = nullptr;

	bool finish;

	if (CDataStorage::Get()->GetIgnoreSave(GtkApp::uTxtPos))
		finish = !GtkApp::LangDataNextRow();
	else
	{
		finish = !GtkApp::LangDataGetFromArray_utf8conv(textBuffer); // get text
	}

	// write text description
	const char* textDescPart1 = u8"## Text ";
	thisObj->langFile->write(textDescPart1, strlen(textDescPart1));
	std::string sTextNumber = std::to_string(GtkApp::uTxtPos);
	thisObj->langFile->write(sTextNumber.c_str(), sTextNumber.length()); // text number
	const char* textDescPart2 = u8" ##\n";
	thisObj->langFile->write(textDescPart2, strlen(textDescPart2));

	// write text
	if (textBuffer)
		thisObj->langFile->write(textBuffer, strlen(textBuffer));

	// write text footer
	const char* textFooter = u8"\n####\n";
	thisObj->langFile->write(textFooter, strlen(textFooter));

	// Update progress bar
	thisObj->UpdateProgressBar(static_cast<long>(thisObj->langFile->tellg()));

	// check if finished
	if (finish)
	{
		// do the rest
		thisObj->bOnSaving = false;
		thisObj->CloseFile(); // we can close the file

		SendMessage("Saving completed.");
		return false;
	}

	return true;
}

// Check if a file with the given name exists
bool CLangFileLoader::IsFileExisting(std::string sName)
{
	return std::experimental::filesystem::exists(sName);
}

// Send a message
void CLangFileLoader::SendMessage(std::string msg)
{
	std::cout << msg << std::endl;

	//Send to GtkApp statusbar
	GtkApp::SetStatusbar(msg.c_str());
}


// Get language name
std::string CLangFileLoader::GetLangNameFromFileName(const std::string &fileName, bool setLangNum = false)
{
	std::string langName;

	char ch1 = fileName.at(fileName.length() - 2); if (ch1 == 't') ch1 = 0;
	char ch2 = fileName.at(fileName.length() - 1);

	switch ((ch1 + ch2))
	{
		/*0*/case 48: langName = "ENGLISH"; if (setLangNum && IsExisting()) { GetObject()->fileLangNum = LANG_ENGLISH; }; break;
		/*1*/case 49: langName = "GERMAN"; if (setLangNum && IsExisting()) { GetObject()->fileLangNum = LANG_GERMAN; }; break;
		/*2*/case 50: langName = "FRENCH"; if (setLangNum && IsExisting()) { GetObject()->fileLangNum = LANG_FRENCH; }; break;
		/*3*/case 51: langName = "SPANISH"; if (setLangNum && IsExisting()) { GetObject()->fileLangNum = LANG_SPANISH; }; break;
		/*4*/case 52: langName = "ITALIAN"; if (setLangNum && IsExisting()) { GetObject()->fileLangNum = LANG_ITALIAN; }; break;
		/*5*/case 53: langName = "POLISH"; if (setLangNum && IsExisting()) { GetObject()->fileLangNum = LANG_POLISH; }; break;
		/*6*/case 54: langName = "KOREAN"; if (setLangNum && IsExisting()) { GetObject()->fileLangNum = LANG_KOREAN; }; break;
		/*7*/case 55: langName = "CHINESE"; if (setLangNum && IsExisting()) { GetObject()->fileLangNum = LANG_CHINESE; }; break;
		/*8*/case 56: langName = "SWEDISH"; if (setLangNum && IsExisting()) { GetObject()->fileLangNum = LANG_SWEDISH; }; break;
		/*9*/case 57: langName = "DANISH"; if (setLangNum && IsExisting()) { GetObject()->fileLangNum = LANG_DANISH; }; break;
		/*10*/case 97: langName = "NORWEGIAN"; if (setLangNum && IsExisting()) { GetObject()->fileLangNum = LANG_NORWEGIAN; }; break;
		/*11*/case 98: langName = "HUNGARIAN"; if (setLangNum && IsExisting()) { GetObject()->fileLangNum = LANG_HUNGARIAN; }; break;
		/*12*/case 99: langName = "HEBREW"; if (setLangNum && IsExisting()) { GetObject()->fileLangNum = LANG_HEBREW; }; break;
		/*13*/case 100: langName = "CZECH"; if (setLangNum && IsExisting()) { GetObject()->fileLangNum = LANG_CZECH; }; break;
		/*14*/case 101: langName = "FINNISH"; if (setLangNum && IsExisting()) { GetObject()->fileLangNum = LANG_FINNISH; }; break;
		/*16*/case 103: langName = "RUSSIAN"; if (setLangNum && IsExisting()) { GetObject()->fileLangNum = LANG_RUSSIAN; }; break;
		/*17*/case 104: langName = "THAI"; if (setLangNum && IsExisting()) { GetObject()->fileLangNum = LANG_THAI; }; break;
		/*18*/case 105: langName = "JAPANESE"; if (setLangNum && IsExisting()) { GetObject()->fileLangNum = LANG_JAPANESE; }; break;

		default: langName = "UNKNOWN LANGUAGE"; if (setLangNum && IsExisting()) { GetObject()->fileLangNum = LANG_UNKNOWN; }; break;
	}

	return langName;
}

int CLangFileLoader::GetLangNumFromFileName(std::string sFileName)
{
	char ch1 = sFileName.at(sFileName.length() - 2); if (ch1 == 't') ch1 = 0;
	char ch2 = sFileName.at(sFileName.length() - 1);

	switch ((ch1 + ch2))
	{
		/*0*/case 48: return LANG_ENGLISH;
		/*1*/case 49: return LANG_GERMAN;
		/*2*/case 50: return LANG_FRENCH;
		/*3*/case 51: return LANG_SPANISH;
		/*4*/case 52: return LANG_ITALIAN;
		/*5*/case 53: return LANG_POLISH;
		/*6*/case 54: return LANG_KOREAN;
		/*7*/case 55: return LANG_CHINESE;
		/*8*/case 56: return LANG_SWEDISH;
		/*9*/case 57: return LANG_DANISH;
		/*10*/case 97: return LANG_NORWEGIAN;
		/*11*/case 98: return LANG_HUNGARIAN;
		/*12*/case 99: return LANG_HEBREW;
		/*13*/case 100: return LANG_CZECH;
		/*14*/case 101: return LANG_FINNISH;
		/*16*/case 103: return LANG_RUSSIAN;
		/*17*/case 104: return LANG_THAI;
		/*18*/case 105: return LANG_JAPANESE;

		default: return LANG_UNKNOWN;
	}
}

int CLangFileLoader::GetLanguageNumByLangName(std::string langName)
{
	for (int i = 0; i < LANG_LAST_POSITION; ++i)
	{
		if (langName == languagesArray[i])
			return i;
	}

	return -1;
}

std::string CLangFileLoader::GetLanguageNameByLangNum(int langNum)
{
	switch (langNum)
	{
		/*0*/case LANG_ENGLISH: return "ENGLISH";
		/*1*/case LANG_GERMAN: return "GERMAN";
		/*2*/case LANG_FRENCH: return "FRENCH";
		/*3*/case LANG_SPANISH: return "SPANISH";
		/*4*/case LANG_ITALIAN: return "ITALIAN";
		/*5*/case LANG_POLISH: return "POLISH";
		/*6*/case LANG_KOREAN: return "KOREAN";
		/*7*/case LANG_CHINESE: return "CHINESE";
		/*8*/case LANG_SWEDISH: return "SWEDISH";
		/*9*/case LANG_DANISH: return "DANISH";
		/*10*/case LANG_NORWEGIAN: return "NORWEGIAN";
		/*11*/case LANG_HUNGARIAN: return "HUNGARIAN";
		/*12*/case LANG_HEBREW: return "HEBREW";
		/*13*/case LANG_CZECH: return "CZECH";
		/*14*/case LANG_FINNISH: return "FINNISH";
		/*16*/case LANG_RUSSIAN: return "RUSSIAN";
		/*17*/case LANG_THAI: return "THAI";
		/*18*/case LANG_JAPANESE: return "JAPANESE";

		default: return "UNKNOWN LANGUAGE";
	}
}

/*void CLangFileLoader::GetFileName(const char* &fileName)
{
	if (IsExisting() && (GetObject()->GetFileName().length() > 0))
	{
		fileName = GetObject()->sFileName.c_str();
	}
	else
		fileName = "";
}*/