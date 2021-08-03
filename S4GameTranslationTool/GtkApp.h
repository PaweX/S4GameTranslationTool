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
#ifndef H_GTKAPP
#define H_GTKAPP

#include <string>

#define S4GTT_PROGRAM_VERSION "0.973 beta"


#define S4TT_PROJECT_FILE_EXTENSION ".s4_translation_project" // project file name
#define S4TT_PROJECTS_FOLDER_PATH "projects\\" // projects folder path

// -- Change Marks - for marking with a color, changes done to a text --
#define CHANGE_MARK_COLOR_NONE ""
#define CHANGE_MARK_COLOR_IMPOTRED "Blue"
#define CHANGE_MARK_COLOR_NEW "Green"
#define CHANGE_MARK_COLOR_CHANGED "Orange"
#define CHANGE_MARK_COLOR_DELETED "Red"

enum change_mark
{
	CHANGE_MARK_NONE,
	CHANGE_MARK_IMPOTRED,
	CHANGE_MARK_NEW,
	CHANGE_MARK_CHANGED,
	CHANGE_MARK_DELETED,
};
//

extern "C"
{
	// Definitions
#undef USE_GTK_FILE_CHOOSER // P: switched off as it caused memory leaks

	// -- Columns in our GtkTreeView (visible columns) --
	enum
	{
#ifdef USE_HEX_COLUMN
		COLUMN_HEX,
#endif
		COLUMN_POSITION,
		COLUMN_CHANGE,
		COLUMN_TEXT,
		NUM_COLUMNS
	};
	// -- --


	// EXTERN 
	extern int GtkAppMain(int argc, char **argv);

	namespace GtkApp
	{
		extern unsigned uTxtPos;

		namespace Set
		{
			extern bool libiconvEncoding;
		}

		extern void SetProgressBar(float fProg);
		extern void SetStatusbar(const char *msg);
		extern void SetLanguageFlag(int langNum);
		extern bool AskUserAQuestion(const char *question);
		extern void LangDataAddToArray(const char *text
#ifdef USE_HEX_COLUMN
									  ,const char* hexHeader
#endif
										);
		extern void LangDataAddToArray_utf8conv(const char *text, unsigned textLength // convert text to utf-8
#ifdef USE_HEX_COLUMN
									   , const char* hexHeader
#endif
		);
		extern void LangDataSwapText(const char *text);
		extern void LangDataSwapText_utf8conv(const char *text, unsigned textLength); // convert text to utf-8
		extern bool LangDataGetFromArray(std::string &text, unsigned &numChar);
		extern bool LangDataGetFromArray_utf8conv(char* &utf8Text); // get utf-8 text
		extern bool LangDataNextRow();
		extern bool LangDataCheckRowIfEmpty();
		extern void LangDataMarkRow(change_mark mark);
		extern void LangDataMarkRowWithNum(change_mark mark, unsigned rowPosition);
	}
}
#endif // H_GTKAPP