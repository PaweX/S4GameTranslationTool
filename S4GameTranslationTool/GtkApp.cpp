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
#include "SettingsFile.h"
#include "DataStorage.h"
#include "CharacterSetConverter.h"
extern "C"
{
#include <gtk/gtk.h>
#include "GtkApp.h"
#include "unlibs4-dll.h"
#include <memory>

	extern void MarkTextWithColor(GtkListStore *liststore, GtkTreeIter *iter, GtkTreePath *path, change_mark mark, bool showmsg = true);
	extern void cb_SaveFileAs(/*GtkWidget *caller*/);

	//--------------------------------
	//------- Shared variables -------
	namespace GtkApp
	{
		//short numLang = -1; // Number of language of loaded file, -1 means no language
		std::string openedLangFile = ""; // Name of a lang file currently loaded or file that is being loaded
		std::string openedProjFile = ""; // Name of a project file currently loaded or file that is being loaded
		bool changesWithoutSave = false;

		gint colorCellHeight;

		// tool tips for change marks
		/*namespace MarkTip
		{
			GtkTooltip changeTipImport;
			GtkTooltip changeTipNew;
			GtkTooltip changeTipChange;
			GtkTooltip changeTipDelete;
		}*/

		// objects
		namespace Wid
		{
			GtkStatusbar *statusbar; // The statusbar
			GtkProgressBar *progressbar; // The progress bar
			GtkToolItem *flagbutton; // The button with a country flag

			GtkWidget *window; // The main window
			GtkWidget *toolbar; // The left toolbar
			GtkWidget *toolbar2; // The right toolbar
			GtkWidget *menubar; // The menubar
			GtkWidget *treeview; // The treeview in the scrolled window (displays game texts)
		}

		namespace Set
		{
			bool darkTheme = false;
			bool remWhitespaces = true;
			int defaultSearchCol = COLUMN_TEXT;
			bool libiconvEncoding = true;
		}

		//***********************
		namespace Priv
		{
			GtkListStore *listModel; // Data model to store texts from a language file. P: To be used only when loading by CLangFileLoader
			GtkTreeIter iter;
		}
		//***********************

		// states
		bool bBusy = 0; // True when the program is saving or opening a file
		unsigned uTxtPos = 0;

		//------------------------------------------
		//------------ Driver Functions ------------
		// Functions used by non Gtk functions to send data to the Gtk App
		void SetProgressBar(float fProg)
		{
			gtk_progress_bar_set_fraction(Wid::progressbar, fProg);
		}

		void SetStatusbar(const char *msg)
		{
			// remove last msg
			gtk_statusbar_pop(GTK_STATUSBAR(Wid::statusbar), NULL);

			// add new msg
			gtk_statusbar_push(GTK_STATUSBAR(Wid::statusbar), NULL, msg);
		}

		void SetLanguageFlag(int langNum)
		{
			// P: couldn't find better solution as the gtk_tool_button_set_icon_name() seems to be not working here
			gtk_widget_destroy(GTK_WIDGET(Wid::flagbutton));

			switch (langNum)
			{
				case LANG_ENGLISH:
				{
					//gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(Wid::flagbutton), "flag-england");
					Wid::flagbutton = gtk_tool_button_new(gtk_image_new_from_icon_name("flag-uk", GTK_ICON_SIZE_DND), "Language flag");
					gtk_tool_item_set_tooltip_text(Wid::flagbutton, "Currently loaded language is English");
					break;
				}

				case LANG_GERMAN:
				{
					//gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(Wid::flagbutton), "flag-germany");
					Wid::flagbutton = gtk_tool_button_new(gtk_image_new_from_icon_name("flag-germany", GTK_ICON_SIZE_DND), "Language flag");
					gtk_tool_item_set_tooltip_text(Wid::flagbutton, "Currently loaded language is German");
					break;
				}

				case LANG_FRENCH:
				{
					//gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(Wid::flagbutton), "flag-france");
					Wid::flagbutton = gtk_tool_button_new(gtk_image_new_from_icon_name("flag-france", GTK_ICON_SIZE_DND), "Language flag");
					gtk_tool_item_set_tooltip_text(Wid::flagbutton, "Currently loaded language is French");
					break;
				}

				case LANG_SPANISH:
				{
					//gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(Wid::flagbutton), "flag-spain");
					Wid::flagbutton = gtk_tool_button_new(gtk_image_new_from_icon_name("flag-spain", GTK_ICON_SIZE_DND), "Language flag");
					gtk_tool_item_set_tooltip_text(Wid::flagbutton, "Currently loaded language is Spanish");
					break;
				}

				case LANG_ITALIAN:
				{
					//gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(Wid::flagbutton), "flag-italy");
					Wid::flagbutton = gtk_tool_button_new(gtk_image_new_from_icon_name("flag-italy", GTK_ICON_SIZE_DND), "Language flag");
					gtk_tool_item_set_tooltip_text(Wid::flagbutton, "Currently loaded language is Italian");
					break;
				}

				case LANG_POLISH:
				{
					//gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(Wid::flagbutton), "flag-poland");
					Wid::flagbutton = gtk_tool_button_new(gtk_image_new_from_icon_name("flag-poland", GTK_ICON_SIZE_DND), "Language flag");
					gtk_tool_item_set_tooltip_text(Wid::flagbutton, "Currently loaded language is Polish");
					break;
				}

				case LANG_KOREAN:
				{
					//gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(Wid::flagbutton), "flag-korea");
					Wid::flagbutton = gtk_tool_button_new(gtk_image_new_from_icon_name("flag-korea", GTK_ICON_SIZE_DND), "Language flag");
					gtk_tool_item_set_tooltip_text(Wid::flagbutton, "Currently loaded language is Korean");
					break;
				}

				case LANG_CHINESE:
				{
					//gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(Wid::flagbutton), "flag-china");
					Wid::flagbutton = gtk_tool_button_new(gtk_image_new_from_icon_name("flag-china", GTK_ICON_SIZE_DND), "Language flag");
					gtk_tool_item_set_tooltip_text(Wid::flagbutton, "Currently loaded language is Chinese");
					break;
				}

				case LANG_SWEDISH:
				{
					//gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(Wid::flagbutton), "flag-sweden");
					Wid::flagbutton = gtk_tool_button_new(gtk_image_new_from_icon_name("flag-sweden", GTK_ICON_SIZE_DND), "Language flag");
					gtk_tool_item_set_tooltip_text(Wid::flagbutton, "Currently loaded language is Swedish");
					break;
				}

				case LANG_DANISH:
				{
					//gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(Wid::flagbutton), "flag-denmark");
					Wid::flagbutton = gtk_tool_button_new(gtk_image_new_from_icon_name("flag-denmark", GTK_ICON_SIZE_DND), "Language flag");
					gtk_tool_item_set_tooltip_text(Wid::flagbutton, "Currently loaded language is Danish");
					break;
				}

				case LANG_NORWEGIAN:
				{
					//gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(Wid::flagbutton), "flag-norway");
					Wid::flagbutton = gtk_tool_button_new(gtk_image_new_from_icon_name("flag-norway", GTK_ICON_SIZE_DND), "Language flag");
					gtk_tool_item_set_tooltip_text(Wid::flagbutton, "Currently loaded language is Norwegian");
					break;
				}

				case LANG_HUNGARIAN:
				{
					//gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(Wid::flagbutton), "flag-hungary");
					Wid::flagbutton = gtk_tool_button_new(gtk_image_new_from_icon_name("flag-hungary", GTK_ICON_SIZE_DND), "Language flag");
					gtk_tool_item_set_tooltip_text(Wid::flagbutton, "Currently loaded language is Hungarian");
					break;
				}

				case LANG_HEBREW:
				{
					//gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(Wid::flagbutton), "flag-israel");
					Wid::flagbutton = gtk_tool_button_new(gtk_image_new_from_icon_name("flag-israel", GTK_ICON_SIZE_DND), "Language flag");
					gtk_tool_item_set_tooltip_text(Wid::flagbutton, "Currently loaded language is Hebrew");
					break;
				}

				case LANG_CZECH:
				{
					//gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(Wid::flagbutton), "flag-czech");
					Wid::flagbutton = gtk_tool_button_new(gtk_image_new_from_icon_name("flag-czech", GTK_ICON_SIZE_DND), "Language flag");
					gtk_tool_item_set_tooltip_text(Wid::flagbutton, "Currently loaded language is Czech");
					break;
				}

				case LANG_FINNISH:
				{
					//gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(Wid::flagbutton), "flag-finland");
					Wid::flagbutton = gtk_tool_button_new(gtk_image_new_from_icon_name("flag-finland", GTK_ICON_SIZE_DND), "Language flag");
					gtk_tool_item_set_tooltip_text(Wid::flagbutton, "Currently loaded language is Finnish");
					break;
				}

				case LANG_RUSSIAN:
				{
					//gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(Wid::flagbutton), "flag-russia");
					Wid::flagbutton = gtk_tool_button_new(gtk_image_new_from_icon_name("flag-russia", GTK_ICON_SIZE_DND), "Language flag");
					gtk_tool_item_set_tooltip_text(Wid::flagbutton, "Currently loaded language is Russian");
					break;
				}

				case LANG_THAI:
				{
					//gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(Wid::flagbutton), "flag-thailand");
					Wid::flagbutton = gtk_tool_button_new(gtk_image_new_from_icon_name("flag-thailand", GTK_ICON_SIZE_DND), "Language flag");
					gtk_tool_item_set_tooltip_text(Wid::flagbutton, "Currently loaded language is Thai");
					break;
				}

				case LANG_JAPANESE:
				{
					//gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(Wid::flagbutton), "flag-japan");
					Wid::flagbutton = gtk_tool_button_new(gtk_image_new_from_icon_name("flag-japan", GTK_ICON_SIZE_DND), "Language flag");
					gtk_tool_item_set_tooltip_text(Wid::flagbutton, "Currently loaded language is Japanese");
					break;
				}

				default:
				{
					//gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(Wid::flagbutton), "flag-none");
					Wid::flagbutton = gtk_tool_button_new(gtk_image_new_from_icon_name("flag-none", GTK_ICON_SIZE_DND), "Language flag");
					gtk_tool_item_set_tooltip_text(Wid::flagbutton, "Currently loaded language is unknown");
					break;
				}
			}

			gtk_toolbar_insert(GTK_TOOLBAR(Wid::toolbar2), Wid::flagbutton, -1);
			gtk_widget_show_all(GTK_WIDGET(Wid::flagbutton));

			// save new language name
			if (CLangFileLoader::IsExisting())
			{
				CLangFileLoader::GetObject()->langName = CLangFileLoader::GetLanguageNameByLangNum(langNum);
			}
		}

		bool AskUserAQuestion(const char *question)
		{
			GtkWindow *parent_window = GTK_WINDOW(GtkApp::Wid::window);
			GtkWidget *dialog;

			gint res;
			GtkDialogFlags flags = (GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT);
			dialog = gtk_message_dialog_new(parent_window,
											flags,
											GTK_MESSAGE_QUESTION,
											GTK_BUTTONS_YES_NO,
											question);

			res = gtk_dialog_run(GTK_DIALOG(dialog));

			if (res == GTK_RESPONSE_YES)
			{
				gtk_widget_destroy(dialog);
				return true;
			}

			gtk_widget_destroy(dialog);
			return false;
		}

		void ShowUserAMessage(const char* message, bool markup = false)
		{
			GtkWindow *parent_window = GTK_WINDOW(GtkApp::Wid::window);
			GtkWidget *dialog;

			GtkDialogFlags flags = (GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT);
			dialog = gtk_message_dialog_new(parent_window,
											flags,
											GTK_MESSAGE_INFO,
											GTK_BUTTONS_OK,
											message);

			if (markup)
			{
				gtk_message_dialog_set_markup(GTK_MESSAGE_DIALOG(dialog), message);
			}

			gtk_dialog_run(GTK_DIALOG(dialog));


			gtk_widget_destroy(dialog);
		}

		gchar *posStr;
		// ----------------------
		// Load and Save drivers
		void LangDataAddToArray(const char *text // without converting to UTF-8 format
#ifdef USE_HEX_COLUMN
								, const char* hexHeader
#endif
		) // Accessed by CLangFileLoader
		{

			if (uTxtPos == 0)
			{
				// Create new list store
				g_object_unref(GtkApp::Priv::listModel);
				GtkApp::Priv::listModel = gtk_list_store_new(NUM_COLUMNS, G_TYPE_STRING, G_TYPE_OBJECT, G_TYPE_STRING
#ifdef USE_HEX_COLUMN
															 , G_TYPE_STRING
#endif
				);

				posStr = g_strdup("START");
			}
			else
				posStr = g_strdup_printf("%d", uTxtPos); //convert i to str

			gtk_list_store_append(GtkApp::Priv::listModel, &GtkApp::Priv::iter);

			gtk_list_store_set(GtkApp::Priv::listModel, &GtkApp::Priv::iter,
#ifdef USE_HEX_COLUMN
							   COLUMN_HEX, hexHeader,
#endif
							   COLUMN_POSITION, posStr,
							   //COLUMN_CHANGE, nullptr,
							   COLUMN_TEXT, text,
							   -1);

			g_free(posStr);
			//++uTxtPos; // P: moved to LangFileLoader.cpp
		}

		void LangDataAddToArray_utf8conv(const char *text, unsigned textLength
#ifdef USE_HEX_COLUMN
							  ,const char* hexHeader
#endif
							) // Accessed by CLangFileLoader
		{

			if (uTxtPos == 0) 
			{
				// Create new list store
				g_object_unref(GtkApp::Priv::listModel);
				GtkApp::Priv::listModel = gtk_list_store_new(NUM_COLUMNS, G_TYPE_STRING, G_TYPE_OBJECT, G_TYPE_STRING
#ifdef USE_HEX_COLUMN
															 ,G_TYPE_STRING
#endif
																);

				posStr = g_strdup("START");
			}
			else
				posStr = g_strdup_printf("%d", uTxtPos); //convert i to str


			std::string strText;
			if (!GtkApp::Set::libiconvEncoding)
			{
				//try // check if the conversion doesn't fail
				//{
					strText = g_locale_to_utf8(text, textLength, NULL, NULL, NULL); // convert to utf-8
				//}
				/*catch (...)
				{
					const char *message = "Exception: Function LangDataAddToArray_utf8conv:\nText conversion failed.";
					GtkApp::ShowUserAMessage(message);
					CLangFileLoader::SendMessage(message);
				}*/
			}
			else
			{
				CCharacterSetConverter *pTextConv = CCharacterSetConverter::GetConverter();
				if (!pTextConv)
				{
					CLangFileLoader::SendMessage("Character set converter is not created!");
					strText = g_locale_to_utf8(text, textLength, NULL, NULL, NULL); // convert to utf-8
				}
				else
				{
					if (!pTextConv->ConvertText(text, textLength, &strText))
					{
						const char *message = "Error: Function LangDataAddToArray_utf8conv:\nText conversion failed.\nDid you use the right encoding?";
						GtkApp::ShowUserAMessage(message);
						CLangFileLoader::SendMessage(message);
						return;
					}
				}
			}

			gtk_list_store_append(GtkApp::Priv::listModel, &GtkApp::Priv::iter);

			gtk_list_store_set(GtkApp::Priv::listModel, &GtkApp::Priv::iter,
#ifdef USE_HEX_COLUMN
								COLUMN_HEX, hexHeader,
#endif
								COLUMN_POSITION, posStr,
								//COLUMN_CHANGE, nullptr,
								COLUMN_TEXT, strText.c_str(),
								-1);

			g_free(posStr);
			//++uTxtPos; // P: moved to LangFileLoader.cpp
		}

		// Swap text in a row
		void LangDataSwapText(const char *text)
		{
			gchar *old_text;
			gchar *original_text;
			gtk_tree_model_get(GTK_TREE_MODEL(GtkApp::Priv::listModel), &GtkApp::Priv::iter, COLUMN_TEXT, &old_text, -1);

			gtk_list_store_set(GtkApp::Priv::listModel, &GtkApp::Priv::iter,
							   COLUMN_TEXT, text,
							   -1);

			GtkTreePath *path = gtk_tree_model_get_path(GTK_TREE_MODEL(GtkApp::Priv::listModel), &GtkApp::Priv::iter);

			if (!CDataStorage::Get()->GetText(original_text, gtk_tree_path_get_indices(path)[0])) // Get the original text to compare with the new one
			{
				original_text = old_text;
				CDataStorage::Get()->PushText(original_text, gtk_tree_path_get_indices(path)[0]);
			}
			else
				g_free(old_text);
		}

		// Swap text in a row - converts text to utf-8 format
		void LangDataSwapText_utf8conv(const char *text, unsigned textLength)
		{
			std::string strText;
			if (!GtkApp::Set::libiconvEncoding)
			{
				//try // check if the conversion doesn't fail
				//{
					strText = g_locale_to_utf8(text, textLength, NULL, NULL, NULL); // convert to utf-8
				//}
				/*catch (...)
				{
					const char *message = "Exception: Function LangDataSwapText_utf8conv:\nText conversion failed.";
					GtkApp::ShowUserAMessage(message);
					CLangFileLoader::SendMessage(message);
				}*/
			}
			else
			{
				CCharacterSetConverter *pTextConv = CCharacterSetConverter::GetConverter();
				if (!pTextConv)
				{
					CLangFileLoader::SendMessage("Character set converter is not created!");
					strText = g_locale_to_utf8(text, textLength, NULL, NULL, NULL); // convert to utf-8
				}
				else
				{
					if(!pTextConv->ConvertText(text, textLength, &strText))
					{
						const char *message = "Error: Function LangDataSwapText_utf8conv:\nText conversion failed.\nDid you use the right encoding?";
						GtkApp::ShowUserAMessage(message);
						CLangFileLoader::SendMessage(message);
						return;
					}
				}
			}

			gchar *old_text;
			gchar *original_text;
			gtk_tree_model_get(GTK_TREE_MODEL(GtkApp::Priv::listModel), &GtkApp::Priv::iter, COLUMN_TEXT, &old_text, -1);

			gtk_list_store_set(GtkApp::Priv::listModel, &GtkApp::Priv::iter,
							   COLUMN_TEXT, strText.c_str(),
							   -1);

			GtkTreePath *path = gtk_tree_model_get_path(GTK_TREE_MODEL(GtkApp::Priv::listModel), &GtkApp::Priv::iter);

			if (!CDataStorage::Get()->GetText(original_text, gtk_tree_path_get_indices(path)[0])) // Get the original text to compare with the new one
			{
				original_text = old_text;
				CDataStorage::Get()->PushText(original_text, gtk_tree_path_get_indices(path)[0]);
			}
			else
				g_free(old_text);
		}

		// Get data from list model
		bool LangDataGetFromArray(std::string &text, unsigned &numChar) // Accessed by CLangFileLoader
		{
			//++uTxtPos; // P: moved to LangFileLoader.cpp

			gchar *utf8Text; // UTF-8 Text

			gtk_tree_model_get(GTK_TREE_MODEL(GtkApp::Priv::listModel), &GtkApp::Priv::iter,
							   COLUMN_TEXT, &utf8Text,
							   -1);

			//if (!CDataStorage::Get()->GetIgnoreSave(uTxtPos)) // P: moved to LangFileLoader.cpp
			
			if (!GtkApp::Set::libiconvEncoding)
			{
				//try // check if the conversion doesn't fail
				//{
					text = g_locale_from_utf8(utf8Text, -1, NULL, &numChar, NULL); // convert from utf-8
				//}
				/*catch (...)
				{
					const char *message = "Exception: Function LangDataGetFromArray:\nText conversion failed.\n"
											"Maybe in some texts you have UNICODE special characters that can't be converted?\n"
											"Try to use 'ICONV Encoding' with option 'ICONV Ignore'.";
					GtkApp::ShowUserAMessage(message);
					CLangFileLoader::SendMessage(message);
				}*/
			}
			else
			{
				CCharacterSetConverter *pTextConv = CCharacterSetConverter::GetConverter();
				if (!pTextConv)
				{
					CLangFileLoader::SendMessage("Character set converter is not created!");
					text = g_locale_from_utf8(utf8Text, -1, NULL, &numChar, NULL); // convert from utf-8
				}
				else
				{
					if(!pTextConv->ConvertText(utf8Text, strlen(utf8Text), &text, &numChar))
					{
						const char *message = "Error: Function LangDataGetFromArray:\nText conversion failed.\nDid you use the right encoding?\n"
												"Maybe in some texts you have special characters that can't be converted?\n"
												"Try to use 'ICONV Encoding' with option 'ICONV Ignore'.";
						GtkApp::ShowUserAMessage(message);
						CLangFileLoader::SendMessage(message);
						return false;
					}
				}
			}
			//text = g_locale_from_utf8(utf8Text, -1, NULL, &numChar, NULL); // convert from utf-8

			g_free(utf8Text);

			// return false if it is end of the array
			if (gtk_tree_model_iter_next(GTK_TREE_MODEL(GtkApp::Priv::listModel), &GtkApp::Priv::iter))
				return true;
			else
				return false;
		}

		// Get data from list model - utf-8 version
		bool LangDataGetFromArray_utf8conv(char* &utf8Text) // Accessed by CLangFileLoader
		{
			//++uTxtPos; // P: moved to LangFileLoader.cpp

			gtk_tree_model_get(GTK_TREE_MODEL(GtkApp::Priv::listModel), &GtkApp::Priv::iter,
							   COLUMN_TEXT, &utf8Text,
							   -1);

			//if (!CDataStorage::Get()->GetIgnoreSave(uTxtPos)) // P: moved to LangFileLoader.cpp

			// return false if it is end of the array
			if (gtk_tree_model_iter_next(GTK_TREE_MODEL(GtkApp::Priv::listModel), &GtkApp::Priv::iter))
				return true;
			else
				return false;
		}

		bool LangDataNextRow()
		{
			// return false if it is end of the array
			if (gtk_tree_model_iter_next(GTK_TREE_MODEL(GtkApp::Priv::listModel), &GtkApp::Priv::iter))
				return true;
			else
				return false;
		}

		bool LangDataCheckRowIfEmpty()
		{
			gchar* text;
			gtk_tree_model_get(GTK_TREE_MODEL(GtkApp::Priv::listModel), &GtkApp::Priv::iter, COLUMN_TEXT, &text, -1);

			if (strcmp(text, "") == 0)
			{
				g_free(text);
				return true;
			}
			else
			{
				g_free(text);
				return false;
			}
		}

		void LangDataMarkRow(change_mark mark)
		{
			GtkTreePath *path = gtk_tree_model_get_path(GTK_TREE_MODEL(GtkApp::Priv::listModel), &GtkApp::Priv::iter);

			MarkTextWithColor(GtkApp::Priv::listModel, &GtkApp::Priv::iter, path, mark);

			gtk_tree_path_free(path);
		}

		void LangDataMarkRowWithNum(change_mark mark, unsigned rowPosition)
		{
			GtkTreePath *path = gtk_tree_path_new_from_indices(rowPosition, -1);
			GtkTreeIter iter;
			gtk_tree_model_get_iter(GTK_TREE_MODEL(GtkApp::Priv::listModel), &iter, path);

			MarkTextWithColor(GtkApp::Priv::listModel, &iter, path, mark);

			gtk_tree_path_free(path);
		}
		//------------------------------------------
	}
	//--------------------------------


	void SetBusy(bool busy) // Activates or deactivates some widgets while opening or saving a file
	{	
		// On opening/saving a file set bBusy to true, when finished set to false
		GtkApp::bBusy = busy;

		//Deactivate or activate menu, toolbar and treeview
		gtk_widget_set_sensitive(GtkApp::Wid::menubar, !busy);
		gtk_widget_set_sensitive(GtkApp::Wid::toolbar, !busy);
		gtk_widget_set_sensitive(GtkApp::Wid::toolbar2, !busy);
		gtk_widget_set_sensitive(GtkApp::Wid::treeview, !busy);
	}

	// This function will be called, as long as it returns true, whenever
	// there are no higher priority events pending to the default main loop.
	static gboolean Do_LoadFile(gpointer data)
	{
		// File loading process
		if (CLangFileLoader::DoLoadFile())
			return true;
		else
		{
			// if succeeded
			if (CLangFileLoader::IsExisting() && CLangFileLoader::GetObject()->IsFileLoaded())
			{
				// set new tree view model
				gtk_tree_view_set_model(GTK_TREE_VIEW(GtkApp::Wid::treeview), GTK_TREE_MODEL(GtkApp::Priv::listModel));

				// set searching
				gtk_tree_view_set_search_column(GTK_TREE_VIEW(GtkApp::Wid::treeview), GtkApp::Set::defaultSearchCol);

				GtkApp::openedProjFile = ""; // reset project name

				// init data storage
				CDataStorage::Get()->InitIgnoreSave(GtkApp::uTxtPos);
				CDataStorage::Get()->ClearTextPosStorage();
			}
			else // if failed
			{
				GtkApp::Priv::listModel = gtk_list_store_new(NUM_COLUMNS, G_TYPE_STRING, G_TYPE_OBJECT, G_TYPE_STRING
#ifdef USE_HEX_COLUMN
															 , G_TYPE_STRING
#endif
				);

				// set new tree view model
				gtk_tree_view_set_model(GTK_TREE_VIEW(GtkApp::Wid::treeview), GTK_TREE_MODEL(GtkApp::Priv::listModel));
			}

			// Activate widgets
			SetBusy(false);

			GtkApp::changesWithoutSave = false; // reset
			
			return false;
		}
	}

	// This function will be called, as long as it returns true, whenever
	// there are no higher priority events pending to the default main loop.
	static gboolean Do_SaveFile(gpointer data)
	{
		// File saving process
		if (CLangFileLoader::DoSaveFile())
			return true;
		else
		{
			// if succeeded
			GtkApp::SetProgressBar(1);

			// Activate widgets
			SetBusy(false);
			GtkApp::changesWithoutSave = false; // reset
			return false;
		}
	}

	// This function will be called, as long as it returns true, whenever
	// there are no higher priority events pending to the default main loop.
	static gboolean Do_CompareS4LangFile(gpointer data)
	{
		// File loading process
		if (CLangFileLoader::DoCompareS4LangFile())
			return true;
		else
		{
			SetBusy(false);
			return false;
		}
	}

	// This function will be called, as long as it returns true, whenever
// there are no higher priority events pending to the default main loop.
	static gboolean Do_CompareProjectFile(gpointer data)
	{
		// File loading process
		if (CLangFileLoader::DoCompareProjectFile())
			return true;
		else
		{
			SetBusy(false);
			return false;
		}
	}

	// This function will be called, as long as it returns true, whenever
	// there are no higher priority events pending to the default main loop.
	static gboolean Do_LoadProject(gpointer data)
	{
		// File loading process
		if (CLangFileLoader::DoLoadProject())
			return true;
		else
		{
			// if succeeded
			if (CLangFileLoader::IsExisting() && CLangFileLoader::GetObject()->IsFileLoaded())
			{
				// set new tree view model
				gtk_tree_view_set_model(GTK_TREE_VIEW(GtkApp::Wid::treeview), GTK_TREE_MODEL(GtkApp::Priv::listModel));

				// set searching
				gtk_tree_view_set_search_column(GTK_TREE_VIEW(GtkApp::Wid::treeview), GtkApp::Set::defaultSearchCol);

				GtkApp::openedLangFile = ""; // reset lang file name

				// init data storage
				CDataStorage::Get()->InitIgnoreSave(GtkApp::uTxtPos);
				CDataStorage::Get()->ClearTextPosStorage();
			}
			else // if failed
			{
				GtkApp::Priv::listModel = gtk_list_store_new(NUM_COLUMNS, G_TYPE_STRING, G_TYPE_OBJECT, G_TYPE_STRING
#ifdef USE_HEX_COLUMN
															 , G_TYPE_STRING
#endif
				);

				// set new tree view model
				gtk_tree_view_set_model(GTK_TREE_VIEW(GtkApp::Wid::treeview), GTK_TREE_MODEL(GtkApp::Priv::listModel));
			}

			// Activate widgets
			SetBusy(false);

			GtkApp::changesWithoutSave = false; // reset
			
			return false;
		}
	}

	// This function will be called, as long as it returns true, whenever
	// there are no higher priority events pending to the default main loop.
	static gboolean Do_SaveProject(gpointer data)
	{
		// File saving process
		if (CLangFileLoader::DoSaveProject())
			return true;
		else
		{
			// if succeeded
			GtkApp::SetProgressBar(1);

			// Activate widgets
			SetBusy(false);
			GtkApp::changesWithoutSave = false; // reset
			return false;
		}
	}

	void OpenFile()
	{
		// Set window to busy mode
		SetBusy(true); 

		// Init the loader
		if (CLangFileLoader::CheckLangFileNameValidity(GtkApp::openedLangFile) && CLangFileLoader::SetFileLoad(GtkApp::openedLangFile))
		{
			// Reset texts positions
			GtkApp::uTxtPos = 0;

			// Pass Do_LoadFile() function to be freely called from the main loop 
			g_idle_add(Do_LoadFile, NULL);

			// Set progress bar to 0
			GtkApp::SetProgressBar(0);
		}
		else
		{
			SetBusy(false);
			GtkApp::openedLangFile = "";
		}
	}

	void SaveFile(const char* newName) // newName: yes or no
	{
		// Set window to busy mode
		SetBusy(true);

		// Init the saver
		if (CLangFileLoader::CheckLangFileNameValidity(newName) && CLangFileLoader::SetFileSave(newName))
		{
			// Set iter to the beginning of the file
			GtkTreePath *path = gtk_tree_path_new_from_string("1"); // start from the second row (it is position 1)
			gtk_tree_model_get_iter(GTK_TREE_MODEL(GtkApp::Priv::listModel), &GtkApp::Priv::iter, path);
			gtk_tree_path_free(path);

			// save the new file name
			GtkApp::openedLangFile = const_cast<char*>(newName);

			// set language flag
			GtkApp::SetLanguageFlag(CLangFileLoader::GetLangNumFromFileName(newName));

			// Pass Do_SaveFile() function to be freely called from the main loop 
			g_idle_add(Do_SaveFile, NULL);

			// Set progress bar to 0
			GtkApp::SetProgressBar(0);
		}
		else
			SetBusy(false);
	}

	void CompareMissingTexts(const char* fileName)
	{
		// Check if the file name is the same as name of the file currently loaded
		/*if (strcmp(fileName, GtkApp::openedFile) == 0)
		{
			GtkApp::SetStatusbar("File chosen for comparison has the same name as the loaded file");
			return;
		}*/				// P: But there might be a case when we want to compare 2 files with the same name

		// Set window to busy mode
		SetBusy(true);

		if (CLangFileLoader::SetFileCompare(fileName))
		{
			GtkTreePath *path = gtk_tree_path_new_from_string("1"); // start from the second row (it is position 1)
			gtk_tree_model_get_iter(GTK_TREE_MODEL(GtkApp::Priv::listModel), &GtkApp::Priv::iter, path);
			gtk_tree_path_free(path);

			// Reset texts positions to 1
			GtkApp::uTxtPos = 1;

			// Pass Do_Compare...() function to be freely called from the main loop 
			if (CLangFileLoader::IsCompareFileAProj())
				g_idle_add(Do_CompareProjectFile, NULL);
			else
				g_idle_add(Do_CompareS4LangFile, NULL);

			// Set progress bar to 0
			GtkApp::SetProgressBar(0);
		}
		else
			SetBusy(false);
	}

	void OpenProject()
	{
		// Set window to busy mode
		SetBusy(true);

		// Init the loader
		if (CLangFileLoader::CheckProjectFileNameValidity(GtkApp::openedProjFile) && CLangFileLoader::SetFileLoad(GtkApp::openedProjFile))
		{
			// Reset texts positions
			GtkApp::uTxtPos = 0;

			// Pass Do_LoadFile() function to be freely called from the main loop 
			g_idle_add(Do_LoadProject, NULL);

			// Set progress bar to 0
			GtkApp::SetProgressBar(0);
		}
		else
		{
			SetBusy(false);
			GtkApp::openedProjFile == "";
		}
	}

	void SaveProject(const char* newName) // newName: yes or no
	{
		// Set window to busy mode
		SetBusy(true);

		// Init the saver
		if (CLangFileLoader::CheckProjectFileNameValidity(newName) && CLangFileLoader::SetFileSave(newName)) // if file was loaded successfully
		{
			// Set iter to the beginning of the file
			GtkTreePath *path = gtk_tree_path_new_from_string("1"); // start from the second row (it is position 1)
			gtk_tree_model_get_iter(GTK_TREE_MODEL(GtkApp::Priv::listModel), &GtkApp::Priv::iter, path);
			gtk_tree_path_free(path);

			// Pass Do_SaveProject() function to be freely called from the main loop 
			g_idle_add(Do_SaveProject, NULL);

			// Set progress bar to 0
			GtkApp::SetProgressBar(0);
		}
		else
		{
			SetBusy(false);
			//GtkApp::openedProjFile == "";
		}
	}

	void UnpackLibFile(const char* fileName)
	{
		int error;
		ExtractFile(fileName, &error);
		
		switch (error)
		{
			case 0:
				GtkApp::ShowUserAMessage("<span size=\"xx-large\">Unpacking done!</span>\n\n<i>The Settlers IV .LIB unpacker originally made by CTPAX-X Team</i>\n\n<span size=\"small\">The Settlers IV .LIB unpacker v1.0 (c) CTPAX-X Team 2018\n<span color=\"blue\">http://www.CTPAX-X.org/</span></span>", true); break;

			case 2:
				GtkApp::ShowUserAMessage("<span size=\"large\">Error: can't open the file</span>", true); break;

			case 3:
				GtkApp::ShowUserAMessage("<span size=\"large\">Error: invalid/unknown file format.</span>", true); break;

			case 1:
				GtkApp::ShowUserAMessage("Error: empty file name"); break;

			default:
				GtkApp::ShowUserAMessage("Unknown error number " + error); break;
		}
	}

	static void MarkTextWithColor(GtkListStore *liststore, GtkTreeIter *iter, GtkTreePath *path, change_mark mark, bool showmsg)
	{
		unsigned textPos = (gtk_tree_path_get_indices(path)[0]);

		if (mark != CHANGE_MARK_IMPOTRED)
			CDataStorage::Get()->SetIgnoreSave(textPos, false);

		if (mark == CHANGE_MARK_NONE)
		{
			gtk_list_store_set(liststore, iter,
							   COLUMN_CHANGE, nullptr,
							   -1);

			if (showmsg)
				GtkApp::SetStatusbar("Last action: A text has been restored to its original state");

			return;
		}

		GdkRGBA color;
		guint32 pixel = 0;
		GdkPixbuf *pixbuf = nullptr;
		const char *colorName;

		// Set tip and color
		switch (mark)
		{
			case CHANGE_MARK_IMPOTRED:
			{
				//gtk_tree_view_set_tooltip_row(GTK_TREE_VIEW(GtkApp::Wid::treeview), &GtkApp::MarkTip::changeTipImport, path);
				colorName = CHANGE_MARK_COLOR_IMPOTRED;
				CDataStorage::Get()->SetIgnoreSave(textPos, true); // ignore imported texts while saving

				/*if (!CDataStorage::Get()->IsTextPosStored(textPos)) // check if its already stored
				{
					char* empty = const_cast<char*>("");
					CDataStorage::Get()->PushText(empty, textPos); // set original text as empty
				}*/ // P: its already done in LangDataSwapText

				break;
			}

			case CHANGE_MARK_NEW:
			{
				//gtk_tree_view_set_tooltip_row(GTK_TREE_VIEW(GtkApp::Wid::treeview), &GtkApp::MarkTip::changeTipNew, path);
				colorName = CHANGE_MARK_COLOR_NEW;
				if (showmsg)
					GtkApp::SetStatusbar("Last action: A new text has been added");

				break;
			}

			case CHANGE_MARK_CHANGED:
			{
				//gtk_tree_view_set_tooltip_row(GTK_TREE_VIEW(GtkApp::Wid::treeview), &GtkApp::MarkTip::changeTipChange, path);
				colorName = CHANGE_MARK_COLOR_CHANGED;
				if (showmsg)
					GtkApp::SetStatusbar("Last action: A text has been replaced with a new one");

				break;
			}

			case CHANGE_MARK_DELETED:
			{
				//gtk_tree_view_set_tooltip_row(GTK_TREE_VIEW(GtkApp::Wid::treeview), &GtkApp::MarkTip::changeTipDelete, path);
				colorName = CHANGE_MARK_COLOR_DELETED;
				if (showmsg)
					GtkApp::SetStatusbar("Last action: A text has been deleted");

				break;
			}

			default:
				colorName = CHANGE_MARK_COLOR_NONE; break;
		}

		if (gdk_rgba_parse(&color, colorName))
			pixel =
			((gint)(color.red * 255)) << 24 |
			((gint)(color.green * 255)) << 16 |
			((gint)(color.blue * 255)) << 8 |
			((gint)(color.alpha * 255));
		

		pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, GtkApp::colorCellHeight, GtkApp::colorCellHeight);
		gdk_pixbuf_fill(pixbuf, pixel);

		gtk_list_store_set(liststore, iter,
						   COLUMN_CHANGE, pixbuf,
						   -1);

		g_object_unref(pixbuf);
	}

	//------------------------------------------
	//----------- callback Functions -----------
	static void cb_OpenFile()
	{
		// Ask user about loading a new file when changes were made
		if (GtkApp::changesWithoutSave)
		{
			if (!GtkApp::AskUserAQuestion("Are you sure you want to load a new file without saving the changes in the current one?"))
			{
				return;
			}
		}

		GtkWindow *parent_window = GTK_WINDOW(GtkApp::Wid::window);
#ifdef USE_GTK_FILE_CHOOSER
		GtkWidget *OF_dialog;

		GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
		gint res;

		OF_dialog = gtk_file_chooser_dialog_new("Open File",
											 parent_window,
											 action,
											 ("_Cancel"),
											 GTK_RESPONSE_CANCEL,
											 ("_Open"),
											 GTK_RESPONSE_ACCEPT,
											 NULL);

		res = gtk_dialog_run(GTK_DIALOG(OF_dialog));
		if (res == GTK_RESPONSE_ACCEPT)
		{
			char *filename;
			GtkFileChooser *chooser = GTK_FILE_CHOOSER(OF_dialog);
			filename = gtk_file_chooser_get_filename(chooser);

			// open the file
			GtkApp::openFile = filename;
			OpenFile();

			g_free(filename);
		}

		gtk_widget_destroy(OF_dialog);
		g_object_unref(OF_dialog);
#else // else use native one
		GtkFileChooserNative *native;
		GtkFileFilter *filter; // For file types
		GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
		gint res;

		native = gtk_file_chooser_native_new("Open S4 Language File",
											 parent_window,
											 action,
											 "_Open",
											 "_Cancel");

		// Filters
		std::string filetype = S4_LANG_FILE_NAME;
		filetype.append("*");
		filter = gtk_file_filter_new(); // filter 1
		gtk_file_filter_set_name(filter, "S4 Language file");
		gtk_file_filter_add_pattern(filter, filetype.c_str());
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(native), filter);
		filter = gtk_file_filter_new(); // filter 2
		gtk_file_filter_set_name(filter, "All files");
		gtk_file_filter_add_pattern(filter, "*");
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(native), filter);


		res = gtk_native_dialog_run(GTK_NATIVE_DIALOG(native));
		if (res == GTK_RESPONSE_ACCEPT)
		{
			GtkFileChooser *chooser = GTK_FILE_CHOOSER(native);
			GtkApp::openedLangFile = gtk_file_chooser_get_filename(chooser);

			// open the file
			OpenFile();
		}

		g_object_unref(native);
#endif
	}

	static void cb_SaveFile()
	{
		// Check if we have anything to save (no loaded file means nothing to save)
		// P: Because there is no 'new file' feature, we work only on files which were previously loaded.
		if (!CLangFileLoader::IsExisting() || !CLangFileLoader::GetObject()->IsFileLoaded())
		{											
			GtkApp::SetStatusbar("No file to be saved.");
			return;
		}


		//filename = g_filename_to_utf8(filename, sizeof(filename), NULL, NULL, NULL);

		// Check if it's the first time we save this lang file
		if (GtkApp::openedLangFile == "")
		{
			cb_SaveFileAs(); 
		}
		else if (CLangFileLoader::IsFileExisting(GtkApp::openedLangFile.c_str())) // Check if the file already exists
		{
			GtkWindow *parent_window = GTK_WINDOW(GtkApp::Wid::window);
			GtkWidget *dialog;

			gint res;
			GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
			dialog = gtk_message_dialog_new(parent_window,
											flags,
											GTK_MESSAGE_QUESTION,
											GTK_BUTTONS_YES_NO,
											"Do you want to overwrite the existing file: \"%s\" ?",
											GtkApp::openedLangFile.c_str());
			res = gtk_dialog_run(GTK_DIALOG(dialog));

			if (res == GTK_RESPONSE_YES)
			{
				// Save file
				SaveFile(GtkApp::openedLangFile.c_str());
			}

			gtk_widget_destroy(dialog);
		}
		else
		{
			SaveFile(GtkApp::openedLangFile.c_str());
		}
	}

	static void cb_SaveFileAs(/*GtkWidget *caller*/)
	{
		// Check if we have anything to save (no loaded file means nothing to save)
		// P: Because there is no 'new file' feature, we work only on files which were previously loaded.
		if (!CLangFileLoader::IsExisting() || !CLangFileLoader::GetObject()->IsFileLoaded())
		{
			GtkApp::SetStatusbar("No file to be saved.");
			return;
		}


		GtkWindow *parent_window = GTK_WINDOW(GtkApp::Wid::window);
#ifdef USE_GTK_FILE_CHOOSER

#else // else use native one
		GtkFileChooserNative *native;
		GtkFileFilter *filter; // For file types
		GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SAVE;
		gint res;

		native = gtk_file_chooser_native_new("Save S4 Language File",
											 parent_window,
											 action,
											 "_Save",
											 "_Cancel");

		// Filters
		std::string filetype = S4_LANG_FILE_NAME;
		filetype.append("*");
		filter = gtk_file_filter_new(); // filter 1
		gtk_file_filter_set_name(filter, "S4 Language file");
		gtk_file_filter_add_pattern(filter, filetype.c_str());
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(native), filter);
		filter = gtk_file_filter_new(); // filter 2
		gtk_file_filter_set_name(filter, "All files");
		gtk_file_filter_add_pattern(filter, "*");
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(native), filter);

		// default file name
		filetype = S4_LANG_FILE_NAME;
		if (CLangFileLoader::IsExisting()) { filetype.append(std::to_string(CLangFileLoader::GetObject()->GetLangNumber())); }
		gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(native), filetype.c_str());


		res = gtk_native_dialog_run(GTK_NATIVE_DIALOG(native));
		if (res == GTK_RESPONSE_ACCEPT)
		{
			char *filename;
			GtkFileChooser *chooser = GTK_FILE_CHOOSER(native);
			filename = gtk_file_chooser_get_filename(chooser);

			// save the file
			if (CLangFileLoader::IsFileExisting(filename)) // Check if the file already exists
			{
				GtkWidget *dialog;

				gint res;
				GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
				dialog = gtk_message_dialog_new(parent_window,
												flags,
												GTK_MESSAGE_QUESTION,
												GTK_BUTTONS_YES_NO,
												"Do you want to overwrite the existing file: \"%s\" ?",
												filename);

				res = gtk_dialog_run(GTK_DIALOG(dialog));

				if (res == GTK_RESPONSE_YES)
				{
					// Save file
					SaveFile(filename);
				}

				gtk_widget_destroy(dialog);
			}
			else
			{
				// Save file
				SaveFile(filename);
			}
		}

		g_object_unref(native);
#endif
	}

	static void cb_OpenProject()
	{
		// Ask user about loading a new file when changes were made
		if (GtkApp::changesWithoutSave)
		{
			if (!GtkApp::AskUserAQuestion("Are you sure you want to load a new file without saving the changes in the current one?"))
			{
				return;
			}
		}


		GtkWindow *parent_window = GTK_WINDOW(GtkApp::Wid::window);
#ifdef USE_GTK_FILE_CHOOSER

#else // else use native one
		GtkFileChooserNative *native;
		GtkFileFilter *filter; // For file types
		GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
		gint res;

		native = gtk_file_chooser_native_new("Open S4GTT Project",
											 parent_window,
											 action,
											 "_Open",
											 "_Cancel");

		// Filters
		std::string filetype = "*";
		filetype.append(S4TT_PROJECT_FILE_EXTENSION);
		filter = gtk_file_filter_new(); // filter 1
		gtk_file_filter_set_name(filter, "S4GTT project file");
		gtk_file_filter_add_pattern(filter, filetype.c_str());
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(native), filter);
		filter = gtk_file_filter_new(); // filter 2
		gtk_file_filter_set_name(filter, "All files");
		gtk_file_filter_add_pattern(filter, "*");
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(native), filter);


		res = gtk_native_dialog_run(GTK_NATIVE_DIALOG(native));
		if (res == GTK_RESPONSE_ACCEPT)
		{
			GtkFileChooser *chooser = GTK_FILE_CHOOSER(native);
			GtkApp::openedProjFile = gtk_file_chooser_get_filename(chooser);

			// open the file
			OpenProject();
		}

		g_object_unref(native);
#endif
	}

	static void cb_SaveProject()
	{
		// Check if we have anything to save (no loaded file means nothing to save)
		// P: Because there is no 'new file' feature, we work only on files which were previously loaded.
		if (!CLangFileLoader::IsExisting() || !CLangFileLoader::GetObject()->IsFileLoaded())
		{
			GtkApp::SetStatusbar("There is no file to be saved");
			return;
		}


		//filename = g_filename_to_utf8(filename, sizeof(filename), NULL, NULL, NULL);

		if (GtkApp::openedProjFile == "")
		{
			GtkApp::openedProjFile = (S4TT_PROJECTS_FOLDER_PATH + CLangFileLoader::GetObject()->GetProjectName());
		}

		if (CLangFileLoader::IsFileExisting(GtkApp::openedProjFile)) // Check if the file already exists
		{
			GtkWindow *parent_window = GTK_WINDOW(GtkApp::Wid::window);
			GtkWidget *dialog;

			gint res;
			GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
			dialog = gtk_message_dialog_new(parent_window,
											flags,
											GTK_MESSAGE_QUESTION,
											GTK_BUTTONS_YES_NO,
											"Do you want to overwrite the existing file: \"%s\" ?",
											GtkApp::openedProjFile.c_str());
			res = gtk_dialog_run(GTK_DIALOG(dialog));

			if (res == GTK_RESPONSE_YES)
			{
				// Save project
				SaveProject(GtkApp::openedProjFile.c_str());
			}

			gtk_widget_destroy(dialog);
		}
		else
		{
			// Save project
			SaveProject(GtkApp::openedProjFile.c_str());
		}
	}

	static void cb_SaveProjectAs(/*GtkWidget *caller*/)
	{
		// Check if we have anything to save (no loaded file means nothing to save)
		// P: Because there is no 'new file' feature, we work only on files which were previously loaded.
		if (!CLangFileLoader::IsExisting() || !CLangFileLoader::GetObject()->IsFileLoaded())
		{
			GtkApp::SetStatusbar("There is no file to be saved");
			return;
		}


		GtkWindow *parent_window = GTK_WINDOW(GtkApp::Wid::window);
#ifdef USE_GTK_FILE_CHOOSER

#else // else use native one
		GtkFileChooserNative *native;
		GtkFileFilter *filter; // For file types
		GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SAVE;
		gint res;

		native = gtk_file_chooser_native_new("Save S4GTT project",
											 parent_window,
											 action,
											 "_Save",
											 "_Cancel");

		// Filters
		std::string filetype = "*";
		filetype.append(S4TT_PROJECT_FILE_EXTENSION);
		filter = gtk_file_filter_new(); // filter 1
		gtk_file_filter_set_name(filter, "S4GTT project file");
		gtk_file_filter_add_pattern(filter, filetype.c_str());
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(native), filter);
		filter = gtk_file_filter_new(); // filter 2
		gtk_file_filter_set_name(filter, "All files");
		gtk_file_filter_add_pattern(filter, "*");
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(native), filter);

		// default file name
		if (CLangFileLoader::IsExisting()) { filetype = CLangFileLoader::GetObject()->langName; } else { filetype = "LANGUAGE_NAME"; }
		filetype.append(S4TT_PROJECT_FILE_EXTENSION);
		gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(native), filetype.c_str());


		res = gtk_native_dialog_run(GTK_NATIVE_DIALOG(native));
		if (res == GTK_RESPONSE_ACCEPT)
		{
			char *filename;
			GtkFileChooser *chooser = GTK_FILE_CHOOSER(native);
			filename = gtk_file_chooser_get_filename(chooser);

			// save the file
			if (CLangFileLoader::IsFileExisting(filename)) // Check if the file already exists
			{
				GtkWidget *dialog;

				gint res;
				GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
				dialog = gtk_message_dialog_new(parent_window,
												flags,
												GTK_MESSAGE_QUESTION,
												GTK_BUTTONS_YES_NO,
												"Do you want to overwrite the existing file: \"%s\" ?",
												filename);

				res = gtk_dialog_run(GTK_DIALOG(dialog));

				if (res == GTK_RESPONSE_YES)
				{
					// Save file
					SaveProject(filename);
				}

				gtk_widget_destroy(dialog);
			}
			else
			{
				// Save file
				SaveProject(filename);
			}
		}

		g_object_unref(native);
#endif
	}

	static void cb_CellEdited(GtkCellRendererText	*cell,
								const gchar         *path_string,
								const gchar         *new_text)
	{
		GtkTreeModel *model = GTK_TREE_MODEL(GtkApp::Priv::listModel);
		GtkTreePath *path = gtk_tree_path_new_from_string(path_string);
		GtkTreeIter iter;

		gtk_tree_model_get_iter(model, &iter, path);

		gchar *old_text;
		gchar *original_text;

		gtk_tree_model_get(model, &iter, COLUMN_TEXT, &old_text, -1);

		gtk_list_store_set(GTK_LIST_STORE(model), &iter,
							COLUMN_TEXT, (GtkApp::Set::remWhitespaces ? g_strstrip(const_cast<gchar*>(new_text)) : new_text),
							-1);

		if (!CDataStorage::Get()->GetText(original_text, gtk_tree_path_get_indices(path)[0])) // Get the original text to compare with the new one
		{
			original_text = old_text;
			CDataStorage::Get()->PushText(original_text, gtk_tree_path_get_indices(path)[0]);
		}
		else
			g_free(old_text);

		if (strcmp(original_text, new_text) != 0)
		{
			GtkApp::changesWithoutSave = true;

			if (strcmp(original_text, "") == 0)
				MarkTextWithColor(GTK_LIST_STORE(model), &iter, path, CHANGE_MARK_NEW);
			else if (strcmp(new_text, "") == 0)
				MarkTextWithColor(GTK_LIST_STORE(model), &iter, path, CHANGE_MARK_DELETED);
			else
				MarkTextWithColor(GTK_LIST_STORE(model), &iter, path, CHANGE_MARK_CHANGED);
		}
		else
			MarkTextWithColor(GTK_LIST_STORE(model), &iter, path, CHANGE_MARK_NONE);

		/*g_object_set(cell,
					 "cell-background", "Blue",
					 "cell-background-set", TRUE,
					 NULL);*/

		gtk_tree_path_free(path);
	}

	static void cb_EditCopy(GtkWidget *caller)
	{
		if (!CLangFileLoader::IsExisting() || !CLangFileLoader::GetObject()->IsFileLoaded())
			return;

		GtkTreeIter iter;
		GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(GtkApp::Wid::treeview));

		if (gtk_tree_selection_get_selected(selection, NULL, &iter))
		{
			GtkTreeModel *model = GTK_TREE_MODEL(GtkApp::Priv::listModel);
			GtkTreePath *path = gtk_tree_model_get_path(model, &iter);
			GtkClipboard *clipboard = gtk_widget_get_clipboard(caller, GDK_SELECTION_CLIPBOARD);

			gchar *text;

			gtk_tree_model_get(model, &iter, COLUMN_TEXT, &text, -1);

			// to clipboard
			gtk_clipboard_set_text(clipboard, text, -1);

			g_free(text);

			gtk_tree_path_free(path);
		}
	}

	static void cb_EditCut(GtkWidget *caller)
	{
		if (!CLangFileLoader::IsExisting() || !CLangFileLoader::GetObject()->IsFileLoaded())
			return;

		GtkTreeIter iter;
		GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(GtkApp::Wid::treeview));

		if (gtk_tree_selection_get_selected(selection, NULL, &iter))
		{
			GtkTreeModel *model = GTK_TREE_MODEL(GtkApp::Priv::listModel);
			GtkTreePath *path = gtk_tree_model_get_path(model, &iter);
			GtkClipboard *clipboard = gtk_widget_get_clipboard(caller, GDK_SELECTION_CLIPBOARD);

			gchar *old_text;
			gchar *original_text;

			gtk_tree_model_get(model, &iter, COLUMN_TEXT, &old_text, -1);

			// to clipboard
			gtk_clipboard_set_text(clipboard, old_text, -1);

			gtk_list_store_set(GTK_LIST_STORE(model), &iter,
							   COLUMN_TEXT, "",
							   -1);

			if (!CDataStorage::Get()->GetText(original_text, gtk_tree_path_get_indices(path)[0])) // Get the original text to compare with the new one
			{
				original_text = old_text;
				CDataStorage::Get()->PushText(original_text, gtk_tree_path_get_indices(path)[0]);
			}
			else
				g_free(old_text);

			if (strcmp(original_text, "") != 0)
			{
				MarkTextWithColor(GTK_LIST_STORE(model), &iter, path, CHANGE_MARK_DELETED);
				GtkApp::changesWithoutSave = true;
			}
			else
				MarkTextWithColor(GTK_LIST_STORE(model), &iter, path, CHANGE_MARK_NONE);

			gtk_tree_path_free(path);
			GtkApp::changesWithoutSave = true;
		}
	}

	void cb_PasteReceived(GtkClipboard *clipboard,
					   const gchar  *text2,
					   gpointer      user_data)
	{
		GtkTreeIter iter;
		GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(GtkApp::Wid::treeview));

		if (gtk_tree_selection_get_selected(selection, NULL, &iter))
		{
			GtkTreeModel *model = GTK_TREE_MODEL(GtkApp::Priv::listModel);
			GtkTreePath *path = gtk_tree_model_get_path(model, &iter);

			gchar *old_text;
			gchar *original_text;

			gtk_tree_model_get(model, &iter, COLUMN_TEXT, &old_text, -1);

			// Place new text into the selected cell
			const gchar *new_text = (GtkApp::Set::remWhitespaces ? g_strstrip(const_cast<gchar*>(text2)) : text2);

			gtk_list_store_set(GTK_LIST_STORE(model), &iter,
							   COLUMN_TEXT, new_text,
							   -1);

			if (!CDataStorage::Get()->GetText(original_text, gtk_tree_path_get_indices(path)[0])) // Get the original text to compare with the new one
			{
				original_text = old_text;
				CDataStorage::Get()->PushText(original_text, gtk_tree_path_get_indices(path)[0]);
			}
			else
				g_free(old_text);

			if (strcmp(original_text, new_text) != 0)
			{
				GtkApp::changesWithoutSave = true;

				if (strcmp(original_text, "") == 0)
					MarkTextWithColor(GTK_LIST_STORE(model), &iter, path, CHANGE_MARK_NEW);
				//else if (strcmp(new_text, "") == 0)
				//	MarkTextWithColor(GTK_LIST_STORE(model), &iter, path, CHANGE_MARK_DELETED);
				else
					MarkTextWithColor(GTK_LIST_STORE(model), &iter, path, CHANGE_MARK_CHANGED);
			}
			else
				MarkTextWithColor(GTK_LIST_STORE(model), &iter, path, CHANGE_MARK_NONE);


			gtk_tree_path_free(path);
		}
	}


	static void cb_EditPaste(GtkWidget *caller)
	{
		if (!CLangFileLoader::IsExisting() || !CLangFileLoader::GetObject()->IsFileLoaded())
			return;

		GtkClipboard *clipboard = gtk_widget_get_clipboard(caller, GDK_SELECTION_CLIPBOARD);

		// Request the contents of the clipboard, contents_received will be
		// called when we do get the contents.
		gtk_clipboard_request_text(clipboard, cb_PasteReceived, NULL);
	}

	static void cb_EditRemove()
	{
		if (!CLangFileLoader::IsExisting() || !CLangFileLoader::GetObject()->IsFileLoaded())
			return;

		GtkTreeIter iter;
		GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(GtkApp::Wid::treeview));

		if (gtk_tree_selection_get_selected(selection, NULL, &iter))
		{
			GtkTreeModel *model = GTK_TREE_MODEL(GtkApp::Priv::listModel);
			GtkTreePath *path = gtk_tree_model_get_path(model, &iter);

			gchar *old_text;
			gchar *original_text;

			gtk_tree_model_get(model, &iter, COLUMN_TEXT, &old_text, -1);

			gtk_list_store_set(GTK_LIST_STORE(model), &iter,
							   COLUMN_TEXT, "",
							   -1);

			if (!CDataStorage::Get()->GetText(original_text, gtk_tree_path_get_indices(path)[0])) // Get the original text to compare with the new one
			{
				original_text = old_text;
				CDataStorage::Get()->PushText(original_text, gtk_tree_path_get_indices(path)[0]);
			}
			else
				g_free(old_text);

			if (strcmp(original_text, "") != 0)
			{
				MarkTextWithColor(GTK_LIST_STORE(model), &iter, path, CHANGE_MARK_DELETED);
				GtkApp::changesWithoutSave = true;
			}
			else
				MarkTextWithColor(GTK_LIST_STORE(model), &iter, path, CHANGE_MARK_NONE);


			gtk_tree_path_free(path);
		}
	}

	static void cb_EditRestore()
	{
		if (!CLangFileLoader::IsExisting() || !CLangFileLoader::GetObject()->IsFileLoaded())
			return;

		GtkTreeIter iter;
		GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(GtkApp::Wid::treeview));

		if (gtk_tree_selection_get_selected(selection, NULL, &iter))
		{
			GtkTreeModel *model = GTK_TREE_MODEL(GtkApp::Priv::listModel);
			GtkTreePath *path = gtk_tree_model_get_path(model, &iter);

			gchar *original_text;

			if (!CDataStorage::Get()->GetText(original_text, gtk_tree_path_get_indices(path)[0])) // Get the original text to compare with the new one
			{
				gtk_tree_path_free(path);
				return;
			}

			gchar *old_text;
			gtk_tree_model_get(model, &iter, COLUMN_TEXT, &old_text, -1);

			if (strcmp(original_text, old_text) != 0)
			{
				gtk_list_store_set(GTK_LIST_STORE(model), &iter,
								   COLUMN_TEXT, original_text,
								   -1);

				MarkTextWithColor(GTK_LIST_STORE(model), &iter, path, CHANGE_MARK_NONE);
				g_free(old_text);
			}

			gtk_tree_path_free(path);
		}
	}

	static void cb_EditConfirmImported()
	{
		if (!CLangFileLoader::IsExisting() || !CLangFileLoader::GetObject()->IsFileLoaded())
			return;

		GtkTreeIter iter;
		GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(GtkApp::Wid::treeview));

		if (gtk_tree_selection_get_selected(selection, NULL, &iter))
		{
			GtkTreeModel *model = GTK_TREE_MODEL(GtkApp::Priv::listModel);
			GtkTreePath *path = gtk_tree_model_get_path(model, &iter);

			if (!CDataStorage::Get()->GetIgnoreSave((gtk_tree_path_get_indices(path)[0])))
				return;

			MarkTextWithColor(GTK_LIST_STORE(model), &iter, path, CHANGE_MARK_NEW, false);
			GtkApp::SetStatusbar("Last action: An imported text has been accepted");
		}
	}

	static void cb_OpenInternalTextEditor()
	{
		if (!CLangFileLoader::IsExisting() || !CLangFileLoader::GetObject()->IsFileLoaded())
			return;

		GtkTreeIter iter;
		GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(GtkApp::Wid::treeview));

		if (gtk_tree_selection_get_selected(selection, NULL, &iter))
		{
			GtkTreeModel *model = GTK_TREE_MODEL(GtkApp::Priv::listModel);
			GtkTreePath *path = gtk_tree_model_get_path(model, &iter);

			gchar *existing_text;
			gchar *new_text = nullptr;
			gtk_tree_model_get(model, &iter, COLUMN_TEXT, &existing_text, -1);

			// --Create mini text editor--
			GtkWindow *parent_window = GTK_WINDOW(GtkApp::Wid::window);
			GtkWidget *dialog, *scrolled, *view, *gtkbox;

			GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
			dialog = gtk_dialog_new_with_buttons("Built-in Text Editor",
												 parent_window,
												 flags,
												 ("_Save"),
												 GTK_RESPONSE_ACCEPT,
												 ("_Cancel"),
												 GTK_RESPONSE_REJECT,
												 NULL);

			gtk_window_set_type_hint(GTK_WINDOW(dialog), GDK_WINDOW_TYPE_HINT_NORMAL);
			gtk_window_set_default_size(GTK_WINDOW(dialog), 640, 400);

			gtkbox = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
			scrolled = gtk_scrolled_window_new(NULL, NULL);
			gtk_widget_set_hexpand(scrolled, FALSE);
			gtk_widget_set_vexpand(scrolled, TRUE);
			gtk_box_pack_start(GTK_BOX(gtkbox), scrolled, TRUE, TRUE, 0);

			view = gtk_text_view_new();
			gtk_text_view_set_editable(GTK_TEXT_VIEW(view), TRUE);
			gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(view), GTK_WRAP_WORD);

			gtk_container_add(GTK_CONTAINER(scrolled), view);

			gtk_widget_show_all(dialog);

			// send text to new window
			GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
			gtk_text_buffer_set_text(buffer, existing_text, -1);

			int result = gtk_dialog_run(GTK_DIALOG(dialog));
			switch (result)
			{
				case GTK_RESPONSE_ACCEPT:
				{
					GtkTextIter start, end;
					gtk_text_buffer_get_bounds(buffer, &start, &end);

					new_text = (GtkApp::Set::remWhitespaces ?
								g_strstrip(const_cast<gchar*>(gtk_text_buffer_get_text(buffer, &start, &end, true)))
								: gtk_text_buffer_get_text(buffer, &start, &end, true));

					gtk_list_store_set(GTK_LIST_STORE(model), &iter,
									   COLUMN_TEXT, new_text,
									   -1);

					gchar *original_text;
					if (!CDataStorage::Get()->GetText(original_text, gtk_tree_path_get_indices(path)[0])) // Get the original text to compare with the new one
					{
						original_text = existing_text;
						CDataStorage::Get()->PushText(original_text, gtk_tree_path_get_indices(path)[0]);
					}
					else
						g_free(existing_text);

					if (strcmp(original_text, new_text) != 0)
					{
						GtkApp::changesWithoutSave = true;

						if (strcmp(original_text, "") == 0)
							MarkTextWithColor(GTK_LIST_STORE(model), &iter, path, CHANGE_MARK_NEW);
						else if (strcmp(new_text, "") == 0)
							MarkTextWithColor(GTK_LIST_STORE(model), &iter, path, CHANGE_MARK_DELETED);
						else
							MarkTextWithColor(GTK_LIST_STORE(model), &iter, path, CHANGE_MARK_CHANGED);
					}
					else
						MarkTextWithColor(GTK_LIST_STORE(model), &iter, path, CHANGE_MARK_NONE);
					
					break;
				}
				default:
					// do_nothing_since_dialog_was_cancelled ();
					break;
			}
			gtk_widget_destroy(dialog);
			// ----

			g_free(new_text);
			gtk_tree_path_free(path);
		}
	}

	static void cb_ImportMissingT()
	{
		// Check if a file is loaded, otherwise we can't compare
		if (!CLangFileLoader::IsExisting() || !CLangFileLoader::GetObject()->IsFileLoaded())
		{
			GtkApp::SetStatusbar("Can't compare. No file is loaded.");
			return;
		}

		GtkWindow *parent_window = GTK_WINDOW(GtkApp::Wid::window);
#ifdef USE_GTK_FILE_CHOOSER

#else // else use native one
		GtkFileChooserNative *native;
		GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
		gint res;

		native = gtk_file_chooser_native_new("Open File",
											 parent_window,
											 action,
											 "_Compare",
											 "_Cancel");

		res = gtk_native_dialog_run(GTK_NATIVE_DIALOG(native));
		if (res == GTK_RESPONSE_ACCEPT)
		{
			char *filename;
			GtkFileChooser *chooser = GTK_FILE_CHOOSER(native);
			filename = gtk_file_chooser_get_filename(chooser);

			// open the file
			CompareMissingTexts(filename);
		}

		g_object_unref(native);
#endif
	}

	static void cb_UnpackLibFile()
	{
		GtkWindow *parent_window = GTK_WINDOW(GtkApp::Wid::window);
#ifdef USE_GTK_FILE_CHOOSER

#else // else use native one
		GtkFileChooserNative *native;
		GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
		gint res;

		native = gtk_file_chooser_native_new("Unpack .lib file",
											 parent_window,
											 action,
											 "_Unpack",
											 "_Cancel");

		res = gtk_native_dialog_run(GTK_NATIVE_DIALOG(native));
		if (res == GTK_RESPONSE_ACCEPT)
		{
			char *filename;
			GtkFileChooser *chooser = GTK_FILE_CHOOSER(native);
			filename = gtk_file_chooser_get_filename(chooser);

			// unpack the file
			UnpackLibFile(filename);
			//g_free(filename);
		}

		g_object_unref(native);
#endif
	}

	static void cb_ToggleDarkTheme(GtkCheckMenuItem *checkmenuitem)
	{
		GtkSettings *settings = gtk_settings_get_default();

		GtkApp::Set::darkTheme = gtk_check_menu_item_get_active(checkmenuitem);

		g_object_set(G_OBJECT(settings),
					 "gtk-application-prefer-dark-theme",
					 GtkApp::Set::darkTheme,
					 NULL);
	}

	static void cb_ToggleLibiconvEncoding(GtkCheckMenuItem *checkmenuitem)
	{
		GtkApp::Set::libiconvEncoding = gtk_check_menu_item_get_active(checkmenuitem);
	}

	static void cb_ToggleLibiconvTranslit(GtkCheckMenuItem *checkmenuitem)
	{
		CCharacterSetConverter::useTranslit = gtk_check_menu_item_get_active(checkmenuitem);
	}
	
	static void cb_ToggleLibiconvIgnore(GtkCheckMenuItem *checkmenuitem)
	{
		CCharacterSetConverter::useIgnore = gtk_check_menu_item_get_active(checkmenuitem);
	}

	static void cb_ToggleWhitespaces(GtkCheckMenuItem *checkmenuitem)
	{
		GtkApp::Set::remWhitespaces = gtk_check_menu_item_get_active(checkmenuitem);
	}

	static void cb_ToggleSearchInTextCol(GtkCheckMenuItem *checkmenuitem)
	{
		if (gtk_check_menu_item_get_active(checkmenuitem))
			GtkApp::Set::defaultSearchCol = COLUMN_TEXT;
		else
			GtkApp::Set::defaultSearchCol = COLUMN_POSITION;

		// set searching for text column
		gtk_tree_view_set_search_column(GTK_TREE_VIEW(GtkApp::Wid::treeview), GtkApp::Set::defaultSearchCol);
	}

	static void cb_HelpDialog()
	{
		GtkWindow *parent_window = GTK_WINDOW(GtkApp::Wid::window);
		GtkWidget *dialog;

		GtkDialogFlags flags = (GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT);
		dialog = gtk_message_dialog_new(parent_window,
										flags,
										GTK_MESSAGE_INFO,
										GTK_BUTTONS_CLOSE,
										"Help not available yet. If you need help write to me on my blog: https://pawex3.blogspot.com/2019/01/the-settlers-iv-game-translation-tool.html");

		gtk_dialog_run(GTK_DIALOG(dialog));


		gtk_widget_destroy(dialog);
	}

	static void cb_AboutS4GTT()
	{
		GtkWindow *parent_window = GTK_WINDOW(GtkApp::Wid::window);

		const gchar *authors[] = {
									"Pawel C. (PaweX3)", "https://github.com/pawex",
									NULL
								 };

		gtk_show_about_dialog(parent_window,
							  "program-name", g_strdup_printf("S4 Game Translation Tool\n by Pawel C. (PaweX3)\nver. %s", S4GTT_PROGRAM_VERSION),
							  "version", g_strdup_printf("Running against GTK+ %d.%d.%d",
														 gtk_get_major_version(),
														 gtk_get_minor_version(),
														 gtk_get_micro_version()),
							  "copyright", u8"(C) 2019 Pawe³ Czapliñski",
							  "license-type", GTK_LICENSE_MIT_X11,
							  "website", "https://pawex3.blogspot.com/2019/01/the-settlers-iv-game-translation-tool.html",
							  "comments", "Program for editing Settlers IV language files.",
							  "authors", authors,
							  "logo-icon-name", "s4gtt-img",
							  "title", "About S4 Game Translation Tool",
							  NULL);
	}

	static gboolean cb_DeleteMainWindow(GtkWidget *window)
	{
		// Ask user about exiting when changes were made
		if (GtkApp::changesWithoutSave)
		{
			if (!GtkApp::AskUserAQuestion("Are you sure you want to exit without saving the changes?"))
			{
				return true;
			}
		}

		gtk_widget_destroy(window);
		return false;
	}

	static gboolean cb_ShowContextMenu(GtkWidget *widget, GdkEvent *event)
	{
		const gint RIGHT_CLICK = 3;

		if (event->type == GDK_BUTTON_PRESS)
		{

			GdkEventButton *bevent = (GdkEventButton *)event;
			
			if (bevent->button == RIGHT_CLICK)
			{

				gtk_menu_popup(GTK_MENU(widget), NULL, NULL, NULL, NULL,
							   bevent->button, bevent->time);
			}

			//return false;
		}

		return false;
	}
	//------------------------------------------
	//------------------------------------------


	//------------------------------------------
	static void AddColumns(GtkTreeView  *treeview) // add columns for GtkTreeView object
	{
		GtkCellRenderer *renderer;
#ifdef USE_HEX_COLUMN
		// hex column
		renderer = gtk_cell_renderer_text_new();
		g_object_set(renderer,
					 "background", "khaki",
					 "editable", FALSE,
					 NULL);

		g_object_set_data(G_OBJECT(renderer), "column", GINT_TO_POINTER(COLUMN_HEX));

		gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(treeview),
													-1, "Header", renderer,
													"text", COLUMN_HEX,
													NULL);
#endif
		// position column
		renderer = gtk_cell_renderer_text_new();
		g_object_set(renderer,
					 "background", "dimgrey",
					 "editable", FALSE,
					 NULL);

		g_object_set_data(G_OBJECT(renderer), "column", GINT_TO_POINTER(COLUMN_POSITION));

		gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(treeview),
													-1, "Position", renderer,
													"text", COLUMN_POSITION,
													NULL);

		// change column
		renderer = gtk_cell_renderer_pixbuf_new();
		g_object_set(renderer,
					 NULL);

		g_object_set_data(G_OBJECT(renderer), "column", GINT_TO_POINTER(COLUMN_CHANGE));

		gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(treeview),
													-1, "Ch.", renderer,
													"pixbuf", COLUMN_CHANGE,
													NULL);

		// text column
		renderer = gtk_cell_renderer_text_new();
		g_object_set(renderer,
					 "editable", TRUE,
					 NULL);
		g_signal_connect(renderer, "edited",
						 G_CALLBACK(cb_CellEdited), NULL);

		g_object_set_data(G_OBJECT(renderer), "column", GINT_TO_POINTER(COLUMN_TEXT));

		gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(treeview),
													-1, "Game Text", renderer,
													"text", COLUMN_TEXT,
													NULL);

		gtk_cell_renderer_get_preferred_height(renderer, GTK_WIDGET(treeview), NULL, &GtkApp::colorCellHeight);
	}

	//------------------------------------------


	//----------------- Widgets ----------------
	//------------------------------------------

	void CreateMenuBar(GtkWidget* &window, GtkWidget* &gtkbox)
	{ // Creates menu bar

		GtkWidget *menubar; // main, highest object in menubar hierarchy
		GtkWidget *topMenuItem;
		GtkWidget *menu;
		GtkWidget *pos1_item;
		GtkWidget *pos2_item;
		GtkWidget *pos3_item;
		GtkWidget *pos4_item;
		GtkWidget *pos5_item;
		GtkWidget *pos6_item;
		GtkWidget *pos7_item;
		GtkWidget *pos8_item;
		GtkWidget *pos9_item;

		GtkAccelGroup *accel_group = NULL;
		accel_group = gtk_accel_group_new();
		gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);

		menubar = gtk_menu_bar_new();

		// -------- File menu --------
		GtkApp::Wid::menubar = menu = gtk_menu_new(); //P: AfteraWhile check: I'm not sure about it, is it correct?

		topMenuItem = gtk_menu_item_new_with_mnemonic("_File");

		pos1_item = gtk_menu_item_new_with_mnemonic("Open Project");
		pos2_item = gtk_menu_item_new_with_mnemonic("Save Project");
		pos3_item = gtk_menu_item_new_with_mnemonic("Save Project As");
		pos4_item = gtk_separator_menu_item_new();
		pos5_item = gtk_menu_item_new_with_mnemonic("Open S4 Language File");
		pos6_item = gtk_menu_item_new_with_label("Save S4 Language File");
		pos7_item = gtk_menu_item_new_with_label("Save S4 Language File As");
		pos8_item = gtk_separator_menu_item_new();
		pos9_item = gtk_menu_item_new_with_mnemonic("Quit");

		//gtk_widget_set_sensitive(GTK_WIDGET(pos2_item), false); // make save button disabled at the start of the program
		//gtk_widget_set_sensitive(GTK_WIDGET(pos3_item), false);

		// accel group
		gtk_widget_add_accelerator(pos1_item, "activate", accel_group,
								   GDK_KEY_o, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

		gtk_widget_add_accelerator(pos2_item, "activate", accel_group,
								   GDK_KEY_s, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

		gtk_widget_add_accelerator(pos6_item, "activate", accel_group,
								   GDK_KEY_s, (GdkModifierType)(GDK_CONTROL_MASK | GDK_SHIFT_MASK), GTK_ACCEL_VISIBLE);

		gtk_widget_add_accelerator(pos9_item, "activate", accel_group,
								   GDK_KEY_q, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

		gtk_menu_item_set_submenu(GTK_MENU_ITEM(topMenuItem), menu);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), pos1_item);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), pos2_item);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), pos3_item);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), pos4_item);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), pos5_item);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), pos6_item);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), pos7_item);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), pos8_item);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), pos9_item);
		gtk_menu_shell_append(GTK_MENU_SHELL(menubar), topMenuItem);

		// g_signals
		g_signal_connect(G_OBJECT(pos1_item), "activate", G_CALLBACK(cb_OpenProject), NULL);
		g_signal_connect(G_OBJECT(pos2_item), "activate", G_CALLBACK(cb_SaveProject), NULL);
		g_signal_connect(G_OBJECT(pos3_item), "activate", G_CALLBACK(cb_SaveProjectAs), NULL);
		g_signal_connect(G_OBJECT(pos5_item), "activate", G_CALLBACK(cb_OpenFile), NULL);
		g_signal_connect(G_OBJECT(pos6_item), "activate", G_CALLBACK(cb_SaveFile), NULL);
		g_signal_connect(G_OBJECT(pos7_item), "activate", G_CALLBACK(cb_SaveFileAs), NULL);
		g_signal_connect(G_OBJECT(pos9_item), "activate", G_CALLBACK(gtk_main_quit), NULL);
		//---------------------------------

		// -------- Edit menu --------
		menu = gtk_menu_new();

		topMenuItem = gtk_menu_item_new_with_mnemonic("_Edit");

		pos1_item = gtk_menu_item_new_with_label("Copy");
		pos2_item = gtk_menu_item_new_with_label("Cut");
		pos3_item = gtk_menu_item_new_with_label("Paste");
		pos4_item = gtk_menu_item_new_with_label("Remove");
		pos5_item = gtk_menu_item_new_with_label("Restore");
		pos6_item = gtk_menu_item_new_with_label("Confirm");
		pos7_item = gtk_separator_menu_item_new();
		pos8_item = gtk_menu_item_new_with_label("Open In Editor");

		// accel group
		gtk_widget_add_accelerator(pos1_item, "activate", accel_group,
								   GDK_KEY_c, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

		gtk_widget_add_accelerator(pos2_item, "activate", accel_group,
								   GDK_KEY_x, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

		gtk_widget_add_accelerator(pos3_item, "activate", accel_group,
								   GDK_KEY_v, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

		gtk_widget_add_accelerator(pos4_item, "activate", accel_group,
								   GDK_KEY_Delete, (GdkModifierType)(NULL), GTK_ACCEL_VISIBLE);

		gtk_widget_add_accelerator(pos5_item, "activate", accel_group,
								   GDK_KEY_r, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

		gtk_widget_add_accelerator(pos6_item, "activate", accel_group,
								   GDK_KEY_g, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

		gtk_widget_add_accelerator(pos8_item, "activate", accel_group,
								   GDK_KEY_e, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

		gtk_menu_item_set_submenu(GTK_MENU_ITEM(topMenuItem), menu);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), pos1_item);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), pos2_item);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), pos3_item);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), pos4_item);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), pos5_item);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), pos6_item);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), pos7_item);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), pos8_item);
		gtk_menu_shell_append(GTK_MENU_SHELL(menubar), topMenuItem);

		// g_signals
		g_signal_connect(G_OBJECT(pos1_item), "activate", G_CALLBACK(cb_EditCopy), NULL);
		g_signal_connect(G_OBJECT(pos2_item), "activate", G_CALLBACK(cb_EditCut), NULL);
		g_signal_connect(G_OBJECT(pos3_item), "activate", G_CALLBACK(cb_EditPaste), NULL);
		g_signal_connect(G_OBJECT(pos4_item), "activate", G_CALLBACK(cb_EditRemove), NULL);
		g_signal_connect(G_OBJECT(pos5_item), "activate", G_CALLBACK(cb_EditRestore), NULL);
		g_signal_connect(G_OBJECT(pos6_item), "activate", G_CALLBACK(cb_EditConfirmImported), NULL);
		g_signal_connect(G_OBJECT(pos8_item), "activate", G_CALLBACK(cb_OpenInternalTextEditor), NULL);
		//---------------------------------

		// -------- Tools menu --------
		menu = gtk_menu_new();

		topMenuItem = gtk_menu_item_new_with_mnemonic("_Tools");

		pos1_item = gtk_menu_item_new_with_label("Import Missing Texts");
		pos2_item = gtk_separator_menu_item_new();
		pos3_item = gtk_menu_item_new_with_label("Unpack .lib file");

		// accel group
		gtk_widget_add_accelerator(pos1_item, "activate", accel_group,
								   GDK_KEY_F9, (GdkModifierType)(NULL), GTK_ACCEL_VISIBLE);

		//gtk_widget_add_accelerator(pos3_item, "activate", accel_group,
		//						   GDK_KEY_F8, (GdkModifierType)(NULL), GTK_ACCEL_VISIBLE);

		gtk_menu_item_set_submenu(GTK_MENU_ITEM(topMenuItem), menu);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), pos1_item);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), pos2_item);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), pos3_item);
		gtk_menu_shell_append(GTK_MENU_SHELL(menubar), topMenuItem);

		// g_signals
		g_signal_connect(G_OBJECT(pos1_item), "activate", G_CALLBACK(cb_ImportMissingT), NULL);
		g_signal_connect(G_OBJECT(pos3_item), "activate", G_CALLBACK(cb_UnpackLibFile), NULL);
		//---------------------------------

		// -------- Settings menu --------
		menu = gtk_menu_new();
		pos9_item = gtk_menu_new();

		topMenuItem = gtk_menu_item_new_with_mnemonic("_Settings");
		pos8_item = gtk_menu_item_new_with_mnemonic("_ICONV");

		pos1_item = gtk_check_menu_item_new_with_label("Dark Theme");
		pos2_item = gtk_separator_menu_item_new();

		pos3_item = gtk_check_menu_item_new_with_label("Use ICONV Encoding");
		pos4_item = gtk_check_menu_item_new_with_label("(EXPERT) ICONV Transliteration");
		pos5_item = gtk_check_menu_item_new_with_label("(EXPERT) ICONV Ignore");

		pos6_item = gtk_check_menu_item_new_with_label("Remove Whitespaces");
		pos7_item = gtk_check_menu_item_new_with_label("Search In Text Column");

		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(pos1_item), GtkApp::Set::darkTheme);
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(pos3_item), GtkApp::Set::libiconvEncoding);
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(pos4_item), CCharacterSetConverter::useTranslit);
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(pos5_item), CCharacterSetConverter::useIgnore);
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(pos6_item), GtkApp::Set::remWhitespaces);
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(pos7_item), (GtkApp::Set::defaultSearchCol == COLUMN_TEXT));

		// accel group
		gtk_widget_add_accelerator(pos1_item, "activate", accel_group,
								   GDK_KEY_F12, (GdkModifierType)(NULL), GTK_ACCEL_VISIBLE);

		gtk_widget_add_accelerator(pos6_item, "activate", accel_group,
								   GDK_KEY_F11, (GdkModifierType)(NULL), GTK_ACCEL_VISIBLE);

		gtk_widget_add_accelerator(pos7_item, "activate", accel_group,
								   GDK_KEY_F10, (GdkModifierType)(NULL), GTK_ACCEL_VISIBLE);

		gtk_menu_item_set_submenu(GTK_MENU_ITEM(topMenuItem), menu);
		gtk_menu_item_set_submenu(GTK_MENU_ITEM(pos8_item), pos9_item);

		gtk_menu_shell_append(GTK_MENU_SHELL(menu), pos1_item);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), pos2_item);

		gtk_menu_shell_append(GTK_MENU_SHELL(pos9_item), pos3_item);
		gtk_menu_shell_append(GTK_MENU_SHELL(pos9_item), pos4_item);
		gtk_menu_shell_append(GTK_MENU_SHELL(pos9_item), pos5_item);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), pos8_item);

		gtk_menu_shell_append(GTK_MENU_SHELL(menu), pos6_item);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), pos7_item);
		gtk_menu_shell_append(GTK_MENU_SHELL(menubar), topMenuItem);

		// g_signals
		g_signal_connect(G_OBJECT(pos1_item), "toggled", G_CALLBACK(cb_ToggleDarkTheme), NULL);
		cb_ToggleDarkTheme(GTK_CHECK_MENU_ITEM(pos1_item));

		g_signal_connect(G_OBJECT(pos3_item), "toggled", G_CALLBACK(cb_ToggleLibiconvEncoding), NULL);
		g_signal_connect(G_OBJECT(pos4_item), "toggled", G_CALLBACK(cb_ToggleLibiconvTranslit), NULL);
		g_signal_connect(G_OBJECT(pos5_item), "toggled", G_CALLBACK(cb_ToggleLibiconvIgnore), NULL);
		g_signal_connect(G_OBJECT(pos6_item), "toggled", G_CALLBACK(cb_ToggleWhitespaces), NULL);
		g_signal_connect(G_OBJECT(pos7_item), "toggled", G_CALLBACK(cb_ToggleSearchInTextCol), NULL);
		//---------------------------------

		// -------- Help menu --------
		menu = gtk_menu_new();

		topMenuItem = gtk_menu_item_new_with_mnemonic("_Help");

		pos1_item = gtk_menu_item_new_with_label("Help");
		pos2_item = gtk_menu_item_new_with_label("About");

		// accel group
		gtk_widget_add_accelerator(pos1_item, "activate", accel_group,
								   GDK_KEY_F1, (GdkModifierType)(NULL), GTK_ACCEL_VISIBLE);

		gtk_widget_add_accelerator(pos2_item, "activate", accel_group,
								   GDK_KEY_F7, (GdkModifierType)(NULL), GTK_ACCEL_VISIBLE);

		gtk_menu_item_set_submenu(GTK_MENU_ITEM(topMenuItem), menu);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), pos1_item);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), pos2_item);
		gtk_menu_shell_append(GTK_MENU_SHELL(menubar), topMenuItem);

		// g_signals
		g_signal_connect(G_OBJECT(pos1_item), "activate", G_CALLBACK(cb_HelpDialog), NULL);
		g_signal_connect(G_OBJECT(pos2_item), "activate", G_CALLBACK(cb_AboutS4GTT), NULL);
		//---------------------------------


		gtk_box_pack_start(GTK_BOX(gtkbox), menubar, FALSE, FALSE, 0);
	}

	void CreateToolBar(GtkWidget* &gtkbox)
	{ // Creates tool bar
		GtkWidget *hbox;

		hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
		//gtk_box_set_homogeneous(GTK_BOX(hbox), TRUE);
		gtk_box_pack_start(GTK_BOX(gtkbox), hbox, FALSE, TRUE, 0);

		// toolbar 1
		GtkWidget *toolbar;
		GtkToolItem *button;
		GtkToolItem *separator;

		GtkApp::Wid::toolbar = toolbar = gtk_toolbar_new();
		gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);

		// BUTTONS ------------------------------------------------------------------------------------------

		// Open project
		button = gtk_tool_button_new(gtk_image_new_from_icon_name("s4gtt-project-open", GTK_ICON_SIZE_DND), "Open Project");
		gtk_toolbar_insert(GTK_TOOLBAR(toolbar), button, -1);

		gtk_tool_item_set_tooltip_text(button, "Open a S4GTT project file");

		g_signal_connect(G_OBJECT(button), "clicked",
						 G_CALLBACK(cb_OpenProject), NULL);

		// Save project
		button = gtk_tool_button_new(gtk_image_new_from_icon_name("s4gtt-project-save", GTK_ICON_SIZE_DND), "Save Project");
		gtk_toolbar_insert(GTK_TOOLBAR(toolbar), button, -1);
		//gtk_widget_set_sensitive(GTK_WIDGET(button), false); // make it disabled at the start of the program

		gtk_tool_item_set_tooltip_text(button, "Save the opened file as S4GTT project file");

		g_signal_connect(G_OBJECT(button), "clicked",
						 G_CALLBACK(cb_SaveProject), NULL);

		// -Separator-
		separator = gtk_separator_tool_item_new();
		gtk_toolbar_insert(GTK_TOOLBAR(toolbar), separator, -1);

		// Open - import
		button = gtk_tool_button_new(gtk_image_new_from_icon_name("document-open", GTK_ICON_SIZE_DND), "Open S4 Language File");
		gtk_toolbar_insert(GTK_TOOLBAR(toolbar), button, -1);

		gtk_tool_item_set_tooltip_text(button, "Open a S-IV language file: s4_texts.dat<language number>");

		g_signal_connect(G_OBJECT(button), "clicked",
						 G_CALLBACK(cb_OpenFile), NULL);

		// Save - export
		button = gtk_tool_button_new(gtk_image_new_from_icon_name("document-save", GTK_ICON_SIZE_DND), "Save S4 Language File");
		gtk_toolbar_insert(GTK_TOOLBAR(toolbar), button, -1);
		//gtk_widget_set_sensitive(GTK_WIDGET(button), false); // make it disabled at the start of the program

		gtk_tool_item_set_tooltip_text(button, "Save the opened file as S-IV language file");

		g_signal_connect(G_OBJECT(button), "clicked",
						 G_CALLBACK(cb_SaveFile), NULL);

		// -Separator-
		separator = gtk_separator_tool_item_new();
		gtk_toolbar_insert(GTK_TOOLBAR(toolbar), separator, -1);

		// Copy
		button = gtk_tool_button_new(gtk_image_new_from_icon_name("edit-copy", GTK_ICON_SIZE_DND), "Copy");
		gtk_toolbar_insert(GTK_TOOLBAR(toolbar), button, -1);

		gtk_tool_item_set_tooltip_text(button, "Copy selected text into the clipboard");

		g_signal_connect(G_OBJECT(button), "clicked",
						 G_CALLBACK(cb_EditCopy), NULL);

		// Cut
		button = gtk_tool_button_new(gtk_image_new_from_icon_name("edit-cut", GTK_ICON_SIZE_DND), "Cut");
		gtk_toolbar_insert(GTK_TOOLBAR(toolbar), button, -1);

		gtk_tool_item_set_tooltip_text(button, "Cut selected text into the clipboard");

		g_signal_connect(G_OBJECT(button), "clicked",
						 G_CALLBACK(cb_EditCut), NULL);

		// Paste
		button = gtk_tool_button_new(gtk_image_new_from_icon_name("edit-paste", GTK_ICON_SIZE_DND), "Paste");
		gtk_toolbar_insert(GTK_TOOLBAR(toolbar), button, -1);

		gtk_tool_item_set_tooltip_text(button, "Paste text from the clipboard");

		g_signal_connect(G_OBJECT(button), "clicked",
						 G_CALLBACK(cb_EditPaste), NULL);

		// Remove
		button = gtk_tool_button_new(gtk_image_new_from_icon_name("edit-delete", GTK_ICON_SIZE_DND), "Remove");
		gtk_toolbar_insert(GTK_TOOLBAR(toolbar), button, -1);

		gtk_tool_item_set_tooltip_text(button, "Remove text from selected row");

		g_signal_connect(G_OBJECT(button), "clicked",
						 G_CALLBACK(cb_EditRemove), NULL);

		// Restore
		button = gtk_tool_button_new(gtk_image_new_from_icon_name("edit-undo", GTK_ICON_SIZE_DND), "Restore");
		gtk_toolbar_insert(GTK_TOOLBAR(toolbar), button, -1);

		gtk_tool_item_set_tooltip_text(button, "Restore original text in selected row");

		g_signal_connect(G_OBJECT(button), "clicked",
						 G_CALLBACK(cb_EditRestore), NULL);

		// Confirm
		button = gtk_tool_button_new(gtk_image_new_from_icon_name("emblem-default", GTK_ICON_SIZE_DND), "Confirm");
		gtk_toolbar_insert(GTK_TOOLBAR(toolbar), button, -1);

		gtk_tool_item_set_tooltip_text(button, "Confirm imported text");

		g_signal_connect(G_OBJECT(button), "clicked",
						 G_CALLBACK(cb_EditConfirmImported), NULL);

		// -Separator-
		separator = gtk_separator_tool_item_new();
		gtk_toolbar_insert(GTK_TOOLBAR(toolbar), separator, -1);

		// Internal text editor
		button = gtk_tool_button_new(gtk_image_new_from_icon_name("accessories-text-editor", GTK_ICON_SIZE_DND), "Edit in text editor..");
		gtk_toolbar_insert(GTK_TOOLBAR(toolbar), button, -1);

		gtk_tool_item_set_tooltip_text(button, "Open selected text in the built-in text editor");

		g_signal_connect(G_OBJECT(button), "clicked",
						 G_CALLBACK(cb_OpenInternalTextEditor), NULL);

		// --------------------------------------------------------------------------------------------------
		// Packing
		gtk_box_pack_start(GTK_BOX(hbox), toolbar, TRUE, TRUE, 5);




		//toolbar 2
		GtkApp::Wid::toolbar2 = toolbar = gtk_toolbar_new();
		gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);

		// BUTTONS ------------------------------------------------------------------------------------------

		// Unpack .lib file
		button = gtk_tool_button_new(gtk_image_new_from_icon_name("package-x-generic", GTK_ICON_SIZE_DND), "Unpack .lib file");
		gtk_toolbar_insert(GTK_TOOLBAR(toolbar), button, -1);

		gtk_tool_item_set_tooltip_text(button, "Unpack Settlers IV .lib file");

		g_signal_connect(G_OBJECT(button), "clicked",
						 G_CALLBACK(cb_UnpackLibFile), NULL);

		// -Separator-
		separator = gtk_separator_tool_item_new();
		gtk_toolbar_insert(GTK_TOOLBAR(toolbar), separator, -1);

		// Language flag - Displays language of the currently loaded file as country flag
		GtkApp::Wid::flagbutton = button = gtk_tool_button_new(gtk_image_new_from_icon_name("flag-none", GTK_ICON_SIZE_DND), "Language flag");
		gtk_toolbar_insert(GTK_TOOLBAR(toolbar), button, -1);

		gtk_tool_item_set_tooltip_text(button, "Language of the currently loaded file");

		// --------------------------------------------------------------------------------------------------
		// Packing
		gtk_box_pack_start(GTK_BOX(hbox), toolbar, FALSE, TRUE, 5);

	}

	void CreateScrolledWindow(GtkWidget* &gtkbox)
	{ // Creates scrolled window

		GtkWidget *scrolled;
		GtkWidget *treeview;
		//GtkTreeModel *gametexts_model;
		//GtkTreeModel *textpos_model;

		scrolled = gtk_scrolled_window_new(NULL, NULL);

		gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled),
											GTK_SHADOW_ETCHED_IN);

		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
									   GTK_POLICY_AUTOMATIC,
									   GTK_POLICY_AUTOMATIC);

		gtk_box_pack_start(GTK_BOX(gtkbox), scrolled, TRUE, TRUE, 0);

		// create models
		GtkApp::Priv::listModel = gtk_list_store_new(NUM_COLUMNS, G_TYPE_STRING, G_TYPE_OBJECT, G_TYPE_STRING
#ifdef USE_HEX_COLUMN
													 ,G_TYPE_STRING
#endif
														);
		//textpos_model = CreatePositionsModel();

		// create tree view
		GtkApp::Wid::treeview = treeview = /*gtk_tree_view_new();*/gtk_tree_view_new_with_model(GTK_TREE_MODEL(GtkApp::Priv::listModel));
		gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview)),
									GTK_SELECTION_SINGLE);

		AddColumns(GTK_TREE_VIEW(treeview));

		//g_object_unref(textpos_model);
		//g_object_unref(gametexts_model);

		gtk_container_add(GTK_CONTAINER(scrolled), treeview);

	}

	void CreateStatusBar(GtkWidget* &gtkbox) // not just status bar but also progress bar
	{ // Creates status bar

#if 0
		GtkWidget *hbox;
		GtkWidget *statusbar;
		GtkWidget *progressbar;

		hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
		gtk_box_set_homogeneous(GTK_BOX(hbox), TRUE);
		gtk_box_pack_start(GTK_BOX(gtkbox), hbox, FALSE, FALSE, 0);

		// status bar
		GtkApp::statusbar = GTK_STATUSBAR(statusbar = gtk_statusbar_new());
		gtk_statusbar_push(GTK_STATUSBAR(statusbar), NULL, "Ready");

		gtk_box_pack_start(GTK_BOX(hbox), statusbar, TRUE, TRUE, 0);

		// progress bar
		GtkApp::progressbar = GTK_PROGRESS_BAR(progressbar = gtk_progress_bar_new());
		gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(progressbar), true);

		gtk_box_pack_start(GTK_BOX(hbox), progressbar, TRUE, TRUE, 0);

#else // P: a bit different layout
		GtkWidget *statusbar;
		GtkWidget *progressbar;

		// progress bar
		GtkApp::Wid::progressbar = GTK_PROGRESS_BAR(progressbar = gtk_progress_bar_new());

		gtk_box_pack_start(GTK_BOX(gtkbox), progressbar, FALSE, TRUE, 0);

		// status bar
		GtkApp::Wid::statusbar = GTK_STATUSBAR(statusbar = gtk_statusbar_new());
		gtk_statusbar_push(GTK_STATUSBAR(statusbar), NULL, "Ready. Please share your completed translations on: https://github.com/PaweX/Settlers_IV_Language_Translations");

		gtk_box_pack_start(GTK_BOX(gtkbox), statusbar, FALSE, TRUE, 0);
#endif
	}

	void CreateContextMenu()
	{
		GtkWidget *cmenu = gtk_menu_new();

		GtkWidget *pos1_item = gtk_menu_item_new_with_label("Copy");
		GtkWidget *pos2_item = gtk_menu_item_new_with_label("Cut");
		GtkWidget *pos3_item = gtk_menu_item_new_with_label("Paste");
		GtkWidget *pos4_item = gtk_menu_item_new_with_label("Remove");
		GtkWidget *pos5_item = gtk_menu_item_new_with_label("Restore");
		GtkWidget *pos6_item = gtk_separator_menu_item_new();
		GtkWidget *pos7_item = gtk_menu_item_new_with_label("Open In Editor");

		gtk_menu_shell_append(GTK_MENU_SHELL(cmenu), pos7_item);
		gtk_menu_shell_append(GTK_MENU_SHELL(cmenu), pos6_item);
		gtk_menu_shell_append(GTK_MENU_SHELL(cmenu), pos1_item);
		gtk_menu_shell_append(GTK_MENU_SHELL(cmenu), pos2_item);
		gtk_menu_shell_append(GTK_MENU_SHELL(cmenu), pos3_item);
		gtk_menu_shell_append(GTK_MENU_SHELL(cmenu), pos4_item);
		gtk_menu_shell_append(GTK_MENU_SHELL(cmenu), pos5_item);

		// g_signals
		g_signal_connect(G_OBJECT(pos1_item), "activate", G_CALLBACK(cb_EditCopy), NULL);
		g_signal_connect(G_OBJECT(pos2_item), "activate", G_CALLBACK(cb_EditCut), NULL);
		g_signal_connect(G_OBJECT(pos3_item), "activate", G_CALLBACK(cb_EditPaste), NULL);
		g_signal_connect(G_OBJECT(pos4_item), "activate", G_CALLBACK(cb_EditRemove), NULL);
		g_signal_connect(G_OBJECT(pos5_item), "activate", G_CALLBACK(cb_EditRestore), NULL);
		g_signal_connect(G_OBJECT(pos7_item), "activate", G_CALLBACK(cb_OpenInternalTextEditor), NULL);

		gtk_widget_show(pos7_item);
		gtk_widget_show(pos6_item);
		gtk_widget_show(pos1_item);
		gtk_widget_show(pos2_item);
		gtk_widget_show(pos3_item);
		gtk_widget_show(pos4_item);
		gtk_widget_show(pos5_item);

		// g_signal for the menu
		g_signal_connect_swapped(G_OBJECT(GtkApp::Wid::treeview), "button-press-event",
								 G_CALLBACK(cb_ShowContextMenu), cmenu);
	}


	// --- Activate App Function ---
	void CreateMainWindow()
	{
		GtkWidget *window;
		GtkWidget *gtkbox;

		// Create main window, set name and size
		GtkApp::Wid::window = window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
		gtk_window_set_title(GTK_WINDOW(window), "S4 Game Translation Tool by PawelC (PaweX3)");
		gtk_window_set_default_size(GTK_WINDOW(window), 960, 600);
		//--

		// Create gtkbox
		gtkbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
		gtk_container_add(GTK_CONTAINER(window), gtkbox);
		//--

		CreateMenuBar(window, gtkbox); // Create menu bar (file, edit, help etc.)
		CreateToolBar(gtkbox); // Create tool bar (a bar with image buttons below the menu bar)
		CreateScrolledWindow(gtkbox); //Create scrolled window with all sub-objects - text editor
		CreateStatusBar(gtkbox); // Create status bar
		CreateContextMenu(); // Create context menu - the one when we press right-mouse-button on the view tree

		// --Do other little things
		// Tips for change marks - color marks for marking changes in a text
		/*gtk_tooltip_set_text(&GtkApp::MarkTip::changeTipImport, "This text has been imported from other file and was originally missing");
		gtk_tooltip_set_text(&GtkApp::MarkTip::changeTipNew, "This text is new");
		gtk_tooltip_set_text(&GtkApp::MarkTip::changeTipChange, "This text has replaced the original text");
		gtk_tooltip_set_text(&GtkApp::MarkTip::changeTipDelete, "Original text in this place has been deleted");*/
		// --

		// Connect call back function to the window 
		g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(cb_DeleteMainWindow), NULL);
		g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);


		gtk_widget_show_all(window);
	}

	void GetVarNumbers()
	{
		int state;
		if (CSettingsFile::GetVarNumber("darkTheme", state))
			(state == 1) ? GtkApp::Set::darkTheme = true : GtkApp::Set::darkTheme = false;

		if (CSettingsFile::GetVarNumber("whitespaces", state))
			(state == 1) ? GtkApp::Set::remWhitespaces = true : GtkApp::Set::remWhitespaces = false;

		if (CSettingsFile::GetVarNumber("defaultSearchCol", state))
			GtkApp::Set::defaultSearchCol = state;

		if (CSettingsFile::GetVarNumber("libiconvEncoding", state))
			(state == 1) ? GtkApp::Set::libiconvEncoding = true : GtkApp::Set::libiconvEncoding = false;

		if (CSettingsFile::GetVarNumber("iconvTranslit", state))
			(state == 1) ? CCharacterSetConverter::useTranslit = true : CCharacterSetConverter::useTranslit = false;

		if (CSettingsFile::GetVarNumber("iconvIgnore", state))
			(state == 1) ? CCharacterSetConverter::useIgnore = true : CCharacterSetConverter::useIgnore = false;

		// Finish
		CSettingsFile::Done();
	}

	void SaveVarNumbers()
	{
		int state;
		(GtkApp::Set::darkTheme) ? state = 1 : state = 0;
		CSettingsFile::SaveVarNumber("darkTheme", state);

		(GtkApp::Set::remWhitespaces) ? state = 1 : state = 0;
		CSettingsFile::SaveVarNumber("whitespaces", state);

		state = GtkApp::Set::defaultSearchCol;
		CSettingsFile::SaveVarNumber("defaultSearchCol", state);

		(GtkApp::Set::libiconvEncoding) ? state = 1 : state = 0;
		CSettingsFile::SaveVarNumber("libiconvEncoding", state);

		(CCharacterSetConverter::useTranslit) ? state = 1 : state = 0;
		CSettingsFile::SaveVarNumber("iconvTranslit", state);

		(CCharacterSetConverter::useIgnore) ? state = 1 : state = 0;
		CSettingsFile::SaveVarNumber("iconvIgnore", state);

		// Finish
		CSettingsFile::Done();
	}

	int GtkAppMain(int argc, char **argv)
	{
		gtk_init(&argc, &argv);

		GetVarNumbers(); // Load user preferences

		// Create main window
		CreateMainWindow();

		gtk_main(); // GTK Main

		SaveVarNumbers(); // Save user preferences/settings like dark theme

		// Destroy CLangFileLoader and CDataStorage and other objects
		CLangFileLoader::DeleteObject();
		CDataStorage::Destroy();
		CCharacterSetConverter::DestroyConverter();

		// Destroy other things

		return 0;
	}
}