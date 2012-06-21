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

// $Id: gui_download_check.cc,v 1.7 2001/12/28 16:05:48 tujikawa Exp $

#include "Dialog.h"
#include "ListManager.h"
#include "HistoryWindow.h"
#include "md5check.h"
#include "crc.h"

extern void Set_sensitive__no_item_selected();
extern void Set_sensitive__list_empty();

extern int g_checkSockPair[2];
extern Dialog *g_cDialog;
extern ListManager *g_listManager;
extern ItemCell *g_consoleItem;
extern HistoryWindow *g_historyWindow;

static bool Do_crc_check(int rowindex, ItemCell *itemcell, ListEntry *listentry)
{
  bool retval = false;
//    switch(itemcell->ret_Status()) {
//    case ItemCell::ITEM_READY:
//    case ItemCell::ITEM_READY_AGAIN:
//    case ItemCell::ITEM_COMPLETE:
//    case ItemCell::ITEM_STOP:
//    case ItemCell::ITEM_ERROR:
    //case ItemCell::ITEM_CRCERROR:
    try {
      if(itemcell->ret_CRC_Type() == ItemCell::CRC_NONE) {
	// do nothing
      } else if(CRC_check_main(itemcell)) {
	string line = _("Checking CRC of '")+itemcell->ret_Filename()+_("' successful");
	g_consoleItem->Send_message_to_gui(line, MSG_SYS_SUCCESS);
	itemcell->set_Status(ItemCell::ITEM_COMPLETE);
	listentry->Set_clist_column__icon(rowindex, ItemCell::ITEM_COMPLETE);
	g_historyWindow->addItem(itemcell);
	itemcell->Send_status_complete();
	retval = true;
      } else {
	string line = _("Checking CRC of '")+itemcell->ret_Filename()+_("' failed");
	g_consoleItem->Send_message_to_gui(line, MSG_SYS_ERROR);

	itemcell->get_Options_Lock();
	if(!itemcell->ret_Options_opt().ret_ignore_crc_error()) {
	  itemcell->set_Status(ItemCell::ITEM_ERROR);
	  listentry->Set_clist_column__icon(rowindex, itemcell->ret_Status());
	}
	itemcell->release_Options_Lock();

      }
    }
    catch (ItemCell::ItemErrorType err) {
      itemcell->PERROR(err);
    }
//      break;
//    default:
//      break;
//    }
  return retval;
}

static bool Do_md5_check(int rowindex, ItemCell *itemcell, ListEntry *listentry)
{
  bool retval = false;
//    switch(itemcell->ret_Status()) {
//    case ItemCell::ITEM_READY:
//    case ItemCell::ITEM_READY_AGAIN:
//    case ItemCell::ITEM_COMPLETE:
//    case ItemCell::ITEM_STOP:
//    case ItemCell::ITEM_ERROR:
    //case ItemCell::ITEM_CRCERROR:
    try {
      if(itemcell->ret_md5string().size()) {
	string md5spec = itemcell->ret_md5string();
	try {
	  string md5comp = md5_check(itemcell->ret_Options().ret_Store_Dir()+itemcell->ret_Filename());

	  if(casecomp(md5spec, md5comp)) {
	    g_consoleItem->Send_message_to_gui(_("Checking MD5 of '")+itemcell->ret_Filename()+_("' successful"), MSG_DOWNLOAD_SUCCESS);
	    itemcell->set_Status(ItemCell::ITEM_COMPLETE);
	    listentry->Set_clist_column__icon(rowindex, ItemCell::ITEM_COMPLETE);
	    g_historyWindow->addItem(itemcell);
	    itemcell->Send_status_complete();
	    retval = true;
	  } else {
	    string line = _("Checking MD5 of '")+itemcell->ret_Filename()+_("' failed");
	    g_consoleItem->Send_message_to_gui(line, MSG_SYS_ERROR);

	    itemcell->get_Options_Lock();
	    if(!itemcell->ret_Options_opt().ret_ignore_crc_error()) {
	      itemcell->set_Status(ItemCell::ITEM_ERROR);
	      listentry->Set_clist_column__icon(rowindex, itemcell->ret_Status());
	    }
	    itemcell->release_Options_Lock();

	  }
	} catch (md5ExceptionType err) {
	  itemcell->Send_message_to_gui(_("IO error occurred"), MSG_DOWNLOAD_ERROR);
	}
      }
    }
    catch (ItemCell::ItemErrorType err) {
      itemcell->PERROR(err);
    }
//      break;
//    default:
//      break;
//    }
  return retval;
}

//
// 選択されたアイテムのCRCをチェック
//

// need pthread lock here
static void Download_check_sub(bool (*check_func)(int rowindex, ItemCell *itemcell, ListEntry *listentry))
{
  ListEntry *listentry = g_listManager->ret_Current_listentry();

  Socket socket(g_checkSockPair[1], Socket::DUPE);

  GList *node = GTK_CLIST(listentry->ret_Dl_clist())->selection;
  while(node) {
    list<int> rfds_list;
    if(socket.is_readready(0, rfds_list) > 0) {
      char buffer[7];
      socket.Recv(buffer, sizeof(buffer));
      socket.Send("QUIT", 0);
      pthread_exit(NULL);
    }

    int rowindex = GPOINTER_TO_UINT(node->data);
    ItemCell *itemcell = (ItemCell*)gtk_clist_get_row_data(GTK_CLIST(listentry->ret_Dl_clist()), rowindex);
    node = g_list_next(node);

    switch(itemcell->ret_Status()) {
    case ItemCell::ITEM_READY:
    case ItemCell::ITEM_READY_AGAIN:
    case ItemCell::ITEM_COMPLETE:
    case ItemCell::ITEM_STOP:
    case ItemCell::ITEM_ERROR:
      check_func(rowindex, itemcell, listentry);// only differs this line
      break;
    default:
      break;
    }
  }
  socket.Send("QUIT", 0);

  pthread_exit(NULL);
}

// need pthread lock here
static void Download_check_downloaded_sub(bool (*check_func)(int rowindex, ItemCell *itemcell, ListEntry *listentry))
{
  ListEntry *listentry = g_listManager->ret_Current_listentry();
  
  Socket socket(g_checkSockPair[1], Socket::DUPE);
  int rowindex = 0;
  //string temp_url;
  int id = -1;
  while(1) {
    list<int> rfds_list;
    if(socket.is_readready(0, rfds_list) > 0) {
      char buffer[7];
      socket.Recv(buffer, sizeof(buffer));
      socket.Send("QUIT", 0);
      pthread_exit(NULL);
    }
    listentry->get_Dl_clist_lock();
    if(rowindex == GTK_CLIST(listentry->ret_Dl_clist())->rows) {
      listentry->release_Dl_clist_lock();
      break;
    }
    ItemCell *itemcell = (ItemCell*)gtk_clist_get_row_data(GTK_CLIST(listentry->ret_Dl_clist()), rowindex);
    //if(temp_url == itemcell->ret_URL()) {
    if(id == itemcell->ret_id()) {
      ++rowindex;
      itemcell = (ItemCell*)gtk_clist_get_row_data(GTK_CLIST(listentry->ret_Dl_clist()), rowindex);
    }
    //temp_url = itemcell->ret_URL();
    id = itemcell->ret_id();

    listentry->release_Dl_clist_lock();

    switch(itemcell->ret_Status()) {
    case ItemCell::ITEM_COMPLETE:
      if(!check_func(rowindex, itemcell, listentry)) {// only differs this line
	++rowindex;
      }
      break;
    default:
      ++rowindex;
      break;
    }
  }
  socket.Send("QUIT", 0);

  pthread_exit(NULL);
}

//static void Download_check_crc_end(void *w,
//				   int sock_main,
//				   GdkInputCondition dummy_cond)
void Download_check_end(void *w,
			       int sock_main,
			       GdkInputCondition dummy_cond)
{
  char buffer[5];
  Socket socket(sock_main, Socket::DUPE);
  socket.Recv(buffer, sizeof(buffer));
  g_cDialog->hide();
  ListEntry *listentry = g_listManager->ret_Current_listentry();
  if(GTK_CLIST(listentry->ret_Dl_clist())->selection == NULL) {
    Set_sensitive__no_item_selected(); // fix this
  }
  if(GTK_CLIST(listentry->ret_Dl_clist())->rows == 0) {
    Set_sensitive__list_empty(); // fix this
  }
}

//static gboolean Download_check_crc_cancel(GtkWidget *w, GtkWidget *window)
static gboolean Download_check_cancel(GtkWidget *w, GtkWidget *window)
{
  Socket socket(g_checkSockPair[0], Socket::DUPE);
  socket.Send("Cancel", 0);

  return TRUE;
}

//gboolean Download_check_crc(GtkWidget* w, void *(*check_crc_func)())
void Download_check(void (*selectorFunc)(bool (*checkerFunc)(int rowindex, ItemCell *itemcell, ListEntry *listentry)),
		    bool (*checkerFunc)(int rowindex, ItemCell *itemcell, ListEntry *listentry))
{
  // cleaning up socket buffer
  Socket socket(g_checkSockPair[0], Socket::DUPE);
  list<int> fdlist;
  bool flag = true;
  while(flag) {
    fdlist.push_back(g_checkSockPair[1]);
    if(socket.is_readready(0, fdlist) > 0) {
      char buffer[7];
      if(Socket::is_set(g_checkSockPair[0], fdlist)) socket.Recv(buffer, sizeof(buffer));
      if(Socket::is_set(g_checkSockPair[1], fdlist)) recv(g_checkSockPair[1], buffer, sizeof(buffer), 0);
      fdlist.clear();
    } else {
      flag = false;
    }
  }
  g_cDialog->setup(_("Check"),
		 _("Checking..."),
		 NULL,
		 NULL,
		 Download_check_cancel);
  g_cDialog->set_yes_button_visible(false);
  g_cDialog->set_no_button_visible(false);
  g_cDialog->show();

  pthread_t crc_thread;
  pthread_create(&crc_thread, (pthread_attr_t *)NULL,
		 (void *(*)(void *))selectorFunc,
		 (void *)checkerFunc);
  pthread_detach(crc_thread);
}

gboolean Download_check_crc(GtkWidget *w, gpointer data) {
  Download_check(Download_check_sub, Do_crc_check);

  return TRUE;
}

gboolean Download_check_crc_downloaded(GtkWidget *w, gpointer data) {
  Download_check(Download_check_downloaded_sub, Do_crc_check);

  return TRUE;
}

gboolean Download_check_md5(GtkWidget *w, gpointer data) {
  Download_check(Download_check_sub, Do_md5_check);

  return TRUE;
}

gboolean Download_check_md5_downloaded(GtkWidget *w, gpointer data) {
  Download_check(Download_check_downloaded_sub, Do_md5_check);

  return TRUE;
}
