//
//  Aria - yet another download tool
//  Copyright (C) 2000, 2001 Tatsuhiro Tsujikawa
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

// $Id: gui_file_open_md5.cc,v 1.1 2001/05/19 18:38:59 tujikawa Exp $

#include "aria.h"
#include "ItemList.h"
#include "AppOption.h"
#include "ItemCell.h"
#include "FileBrowser.h"
#include "ListManager.h"

// external functions
extern void Set_sensitive__list_not_empty();
extern gboolean Hide_window(GtkWidget *window, gpointer unused);

// global variables
extern ItemList *g_itemList;
extern ItemCell *g_consoleItem;
extern AppOption *g_appOption;
extern FileBrowser *g_cFileBrowser;
extern ListManager *g_listManager;

//
// CRCファイルを読み込む
//
// CRCファイルセレクションウインドウでOKボタンを押したときの処理
void Open_md5_file(const string& filename)
{
  ListEntry *listEntry = g_listManager->ret_Current_listentry();

  bool retval = g_itemList->Read_md5_from_file(listEntry, filename);
  
  if(retval == false) {
    g_consoleItem->Send_message_to_gui(_("Error occurred while reading MD5 file"), MSG_SYS_ERROR);
  } else {
    g_consoleItem->Send_message_to_gui(_("MD5 list opened"), MSG_SYS_INFO);
  }
}

static gboolean File_ok_open_md5_list(GtkWidget *w, GtkWidget *fs)
{
  const char *filename;

  g_cFileBrowser->hide();
  if((filename = gtk_file_selection_get_filename(GTK_FILE_SELECTION(fs))) != NULL) {
    Open_md5_file(filename);
  }
  return TRUE;
}

gboolean File_open_md5_list(GtkWidget *w, gpointer data)
{
  g_cFileBrowser->setup(_("Open MD5 list"),
		      File_ok_open_md5_list);
  g_cFileBrowser->show();
  return TRUE;
}
