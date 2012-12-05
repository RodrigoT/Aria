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

// $Id: gui_file_open_url.cc,v 1.11 2002/02/13 12:09:24 tujikawa Exp $

#include "aria.h"
#include "ItemList.h"
#include "AppOption.h"
#include "ListManager.h"
#include "ItemCell.h"
#include "FileBrowser.h"

// external functions
extern void Set_sensitive__list_not_empty();
extern gboolean Hide_window(GtkWidget *window, gpointer unused);

// global variables
extern ItemList *g_itemList;
extern ItemCell *g_consoleItem;
extern AppOption *g_appOption;
extern ListManager *g_listManager;
extern FileBrowser *g_cFileBrowser;

//
// URLファイルを読み込む
//
// URLファイルセレクションウインドウでOKボタンを押したときの処理
void Open_url_file(const string &filename)
{
    ListEntry *listEntry = g_listManager->ret_Current_listentry();
    // another thread can access clist in download.cc,
    // so this mutex lock is necessary.
    listEntry->get_Dl_clist_lock();

    if (!g_itemList->Read_URL_from_file(listEntry, filename)) {
        listEntry->release_Dl_clist_lock();
        g_consoleItem->Send_message_to_gui(_("Error occurred while reading URL file"), MSG_SYS_ERROR);
    } else {
        bool crcFlag = false;
        struct stat fileStat;
        string crcFilename = filename + ".crc";
        if (stat(crcFilename.c_str(), &fileStat) == 0 && S_ISREG(fileStat.st_mode)) {
            crcFlag = true;
        } else {
            unsigned int ext_pos = filename.rfind('.');
            if (ext_pos != string::npos) {
                crcFilename = filename.substr(0, ext_pos) + ".crc";
                if (stat(crcFilename.c_str(), &fileStat) == 0 && S_ISREG(fileStat.st_mode)) {
                    crcFlag = true;
                }
            }
        }
        if (crcFlag) {
            g_itemList->Read_CRC_from_file(listEntry, crcFilename);
        }
        // modified 2001/5/20
        if (g_appOption->Whether_use_automatic_start()) {
            listEntry->Send_start_signal();
        }

        listEntry->release_Dl_clist_lock();
        g_consoleItem->Send_message_to_gui(_("URL list opened"), MSG_SYS_INFO);
    }
    if (GTK_CLIST(listEntry->ret_Dl_clist())->rows > 0) {
        Set_sensitive__list_not_empty();//fix this
    }
}

static gboolean File_ok_open_URL_list(GtkWidget *w, GtkWidget *fs)
{
    const char *filename;

    g_cFileBrowser->hide();
    if ((filename = gtk_file_selection_get_filename(GTK_FILE_SELECTION(fs))) != NULL) {
        Open_url_file(filename);
    }
    return TRUE;
}

gboolean File_open_URL_list(GtkWidget *w, gpointer data)
{
    g_cFileBrowser->setup(_("Open URL list"),
                          File_ok_open_URL_list);
    g_cFileBrowser->show();

    return TRUE;
}
