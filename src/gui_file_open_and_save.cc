//
//  Aria - yet another download tool
//  Copyright (C) 2000, 2001, 2002 Tatsuhiro Tsujikawa
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

// $Id: gui_file_open_and_save.cc,v 1.12 2002/02/13 12:09:24 tujikawa Exp $

#include "aria.h"
#include "ItemList.h"
#include "AppOption.h"
#include "ThreadManager.h"
#include "ItemCell.h"
#include "FileBrowser.h"

// external functions
extern void Set_sensitive__list_not_empty();
extern gboolean Hide_window(GtkWidget *window, gpointer unused);
extern gboolean Download_delete_all(GtkWidget *w, gpointer data);

// prototype declaration
static gboolean File_ok_open_Saved_list(GtkWidget *w, GtkWidget *fs);

// global variables
extern ItemList *g_itemList;
extern ItemCell *g_consoleItem;
extern AppOption *g_appOption;
extern ListManager *g_listManager;
extern Dialog *g_cDialog;
extern FileBrowser *g_cFileBrowser;

//
// 現在のリストを保存
// save current lists
//
static gboolean File_ok_save_list(GtkWidget *w, GtkWidget *fs)
{
  const char *filename;

  g_cFileBrowser->hide();
  if((filename = gtk_file_selection_get_filename(GTK_FILE_SELECTION(fs))) != NULL) {
    
    bool retval = g_itemList->Save_current_list(filename);

    if(!retval) {
      g_consoleItem->Send_message_to_gui(_("Failed to save lists"), MSG_SYS_ERROR);
    } else {
      g_consoleItem->Send_message_to_gui(_("lists saved"), MSG_SYS_INFO);
    }
  }
  return TRUE;
}

gboolean File_save_list(GtkWidget *w, gpointer data)
{
  g_cFileBrowser->setup(_("Save list"),
			File_ok_save_list);
  g_cFileBrowser->show();

  return TRUE;
}

//
// 現在のリストの内容を保存するかどうか尋ねる
//
/*
static gboolean Save_yes(GtkWidget* w, GtkWidget *window)
{
  cdialog->hide();
  cfilebrowser->setup(_("Save list"),
		      File_ok_save_list);
  cfilebrowser->show();

  return TRUE;
}

static gboolean Save_cancel(GtkWidget *w, GtkWidget *window)
{
  cdialog->hide();
  return TRUE;
}

static gboolean Save_no(GtkWidget* w, GtkWidget *window)
{
  cdialog->hide();
  Download_delete_all(w, NULL);

  cfilebrowser->setup(_("Open Saved list"),
		      File_ok_open_Saved_list);
  cfilebrowser->show();
  
  return TRUE;
}

static void Prompt_save_or_not()
{
  cdialog->setup(_("Open saved list"),
		 _("save current list?"),
		 Save_yes,
		 Save_no,
		 Save_cancel);
  cdialog->show();
}
*/

//
// 保存されていたダウンロードリストを読み込む
//
static gboolean File_ok_open_Saved_list(GtkWidget* w, GtkWidget *fs)
{
  const char *filename;

  g_cFileBrowser->hide();
  ListEntry *listEntry = g_listManager->ret_Current_listentry();

  if((filename = gtk_file_selection_get_filename(GTK_FILE_SELECTION(fs))) != NULL) {

    bool retval = g_itemList->Restore_saved_list(filename);
    if(!retval) {
      g_consoleItem->Send_message_to_gui(_("Failed to load lists"), MSG_SYS_ERROR);
    } else {
      g_consoleItem->Send_message_to_gui(_("Saved list opened"), MSG_SYS_INFO);
      // modified 2001/5/20
      listEntry->get_Dl_clist_lock();
      if(g_appOption->Whether_use_automatic_start()) {
	listEntry->Send_start_signal();
      }
      listEntry->release_Dl_clist_lock();
    }
    listEntry->get_Dl_clist_lock();
    if(GTK_CLIST(listEntry->ret_Dl_clist())->rows > 0) {
      Set_sensitive__list_not_empty(); // fix this
    }
    listEntry->release_Dl_clist_lock();  
  }
  return TRUE;
}

gboolean File_open_Saved_list(GtkWidget *w, gpointer data)
{
  /*
  // 現在のアイテムリストを保存もしくは破棄
  if(GTK_CLIST(itemlistwidget)->rows) {// if clist contains any items
    // 現在のリストを保存するかどうか尋ねる
    AfterOpen = true;
    Prompt_save_or_not();
  } else {
  */
  // AfterOpen = false;
  g_cFileBrowser->setup(_("Open Saved list"),
			File_ok_open_Saved_list);
  g_cFileBrowser->show();
    /*
  }
    */
  return TRUE;
}
