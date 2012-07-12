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

// $Id: gui_download.cc,v 1.54 2002/10/01 15:32:00 tujikawa Exp $

#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <utility>
#include <list>
#include <algorithm>
#include "aria.h"
#include "ListManager.h"
#include "ItemCell.h"
#include "ItemList.h"
#include "utils.h"
#include "Dialog.h"
#include "HistoryWindow.h"
#include "ItemStatusSum.h"
#include "SumInfo.h"
#include "ShortCutKey.h"
#include "gui_utils.h"

#include "pixmaps/start.xpm"
#include "pixmaps/start_all.xpm"
#include "pixmaps/pause.xpm"
#include "pixmaps/pause_all.xpm"
#include "pixmaps/trash.xpm"
#include "pixmaps/restart.xpm"

using namespace std;

extern void Set_sensitive__no_item_selected();
extern void Set_sensitive__list_empty();
extern void Send_report(MessageType reporttype, ItemStatus *itemstatus);
extern gboolean Download_check_crc(GtkWidget *w, gpointer data);
extern gboolean Download_check_crc_downloaded(GtkWidget *w, gpointer data);
extern gboolean Download_check_md5(GtkWidget *w, gpointer data);
extern gboolean Download_check_md5_downloaded(GtkWidget *w, gpointer data);
extern void Download_check_end(void *w,
			       int sock_main,
			       GdkInputCondition dummy_cond);
extern ItemCell::DownloadStatusType Execute_command_in_option(ItemCell *itemcell);

extern ItemCell *g_consoleItem;
extern ListManager *g_listManager;
extern AppOption *g_appOption;
extern Dialog *g_cDialog;
extern HistoryWindow *g_historyWindow;
extern SumInfo g_summaryInfo;

static GtkWidget *start_item, *stop_item, *clear_item, *downloadAgain_item;
static GtkWidget *clear_with_file_item;
static GtkWidget *clearCRC_item, *checkCRC_item;
static GtkWidget *clearMD5Item, *checkMD5Item;
static GtkWidget *startAll_item, *stopAll_item, *clearAll_item;
static GtkWidget *startAllList_item, *stopAllList_item, *clearAllList_item;
static GtkWidget *clearCRCall_item, *clearMD5AllItem;
static GtkWidget *checkCRCdownloaded_item, *checkMD5DownloadedItem;
int g_checkSockPair[2];

void Download_stop_sub(ItemCell *itemcell, int rowindex, ListEntry *listentry, bool all_flag = false, ItemCommand::EventCause eventtype = ItemCommand::EV_USERINTER);

void Download_set_sensitive__no_item_selected()
{
  gtk_widget_set_sensitive(start_item, FALSE);
  gtk_widget_set_sensitive(stop_item, FALSE);
  gtk_widget_set_sensitive(clear_item, FALSE);
  gtk_widget_set_sensitive(clear_with_file_item, FALSE);
  gtk_widget_set_sensitive(checkCRC_item, FALSE);
  gtk_widget_set_sensitive(clearCRC_item, FALSE);
  gtk_widget_set_sensitive(checkMD5Item, FALSE);
  gtk_widget_set_sensitive(clearMD5Item, FALSE);
  gtk_widget_set_sensitive(downloadAgain_item, FALSE);
}

void Download_set_sensitive__items_selected()
{
  gtk_widget_set_sensitive(start_item, TRUE);
  gtk_widget_set_sensitive(stop_item, TRUE);
  gtk_widget_set_sensitive(clear_item, TRUE);
  gtk_widget_set_sensitive(clear_with_file_item, TRUE);
  gtk_widget_set_sensitive(checkCRC_item, TRUE);
  gtk_widget_set_sensitive(clearCRC_item, TRUE);
  gtk_widget_set_sensitive(checkMD5Item, TRUE);
  gtk_widget_set_sensitive(clearMD5Item, TRUE);
  gtk_widget_set_sensitive(downloadAgain_item, TRUE);
}

void Download_set_sensitive__list_empty()
{
  gtk_widget_set_sensitive(startAll_item, FALSE);
  gtk_widget_set_sensitive(stopAll_item, FALSE);
  gtk_widget_set_sensitive(clearAll_item, FALSE);
  gtk_widget_set_sensitive(clearCRCall_item, FALSE);
  gtk_widget_set_sensitive(checkCRCdownloaded_item, FALSE);
  gtk_widget_set_sensitive(clearMD5AllItem, FALSE);
  gtk_widget_set_sensitive(checkMD5DownloadedItem, FALSE);
}

void Download_set_sensitive__list_not_empty()
{
  gtk_widget_set_sensitive(startAll_item, TRUE);
  gtk_widget_set_sensitive(stopAll_item, TRUE);
  gtk_widget_set_sensitive(clearAll_item, TRUE);
  gtk_widget_set_sensitive(clearCRCall_item, TRUE);
  gtk_widget_set_sensitive(checkCRCdownloaded_item, TRUE);
  gtk_widget_set_sensitive(clearMD5AllItem, TRUE);
  gtk_widget_set_sensitive(checkMD5DownloadedItem, TRUE);
}

// backup current selection list
static void Backup_selection_list(GtkWidget *clist, list<GtkTreePath*>& selection_temp)
{
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(clist));
  for(GList* node = gtk_tree_selection_get_selected_rows(gtk_tree_view_get_selection(GTK_TREE_VIEW(clist)), &model); 
  	node != NULL; 
	node = g_list_next(node)) {
    selection_temp.push_back((GtkTreePath*)node->data);
  }
}

// restore selection list
static void Restore_selection_list(GtkWidget *clist, const list<GtkTreePath*>& selection_temp)
{
  list<GtkTreePath*>::const_iterator sl_itr;
  for(sl_itr = selection_temp.begin(); sl_itr != selection_temp.end(); ++sl_itr) {
    gtk_tree_selection_select_path(gtk_tree_view_get_selection(GTK_TREE_VIEW(clist)), *sl_itr);
  }
}

unsigned int find_min_value(GList *node)
{
  unsigned int min = GPOINTER_TO_UINT(node->data);
  node = g_list_next(node);
  while(node) {
    if(min > GPOINTER_TO_UINT(node->data)) {
      min = GPOINTER_TO_UINT(node->data);
    }
    node = g_list_next(node);
  }
  return min;
}

unsigned int find_max_value(GList *node)
{
  unsigned int max = GPOINTER_TO_UINT(node->data);
  node = g_list_next(node);
  while(node) {
    if(max < GPOINTER_TO_UINT(node->data)) {
      max = GPOINTER_TO_UINT(node->data);
    }
    node = g_list_next(node);
  }
  return max;
}

void Download_change_speed_sub(ItemCell *itemcell, float fspeed)
{
  switch(itemcell->ret_Status()) {
  case ItemCell::ITEM_DOWNLOAD:
    //case ItemCell::ITEM_DOWNLOAD_PARTIAL:
  case ItemCell::ITEM_INUSE:
  case ItemCell::ITEM_INUSE_CONCAT:
  case ItemCell::ITEM_INUSE_AGAIN:
  case ItemCell::ITEM_DOWNLOAD_AGAIN:
  case ItemCell::ITEM_DOWNLOAD_INTERNAL_AGAIN:
  case ItemCell::ITEM_CRCERROR:
  case ItemCell::ITEM_EXECERROR:
    {
      ItemCommand itemcommand;
      itemcommand.commandtype = ItemCommand::COMMAND_CHANGE_SPEED;
      itemcommand.eventtype = ItemCommand::EV_USERINTER;
      itemcommand.value = fspeed;
      write(itemcell->ret_Desc_w(), &itemcommand, sizeof(ItemCommand));
      break;
    }
  case ItemCell::ITEM_DOWNLOAD_PARTIAL:
    itemcell->get_Options_Lock();
    itemcell->ret_Options_opt().set_speed_limit(fspeed);
    itemcell->ret_Options().set_speed_limit(fspeed);
    itemcell->release_Options_Lock();    
    for(list<ItemCell*>::const_iterator item_ptr = itemcell->ret_Worker_list().begin(); item_ptr != itemcell->ret_Worker_list().end(); ++item_ptr) {
      Download_change_speed_sub((ItemCell*)*item_ptr, fspeed);
    }
    break;
  default:
    itemcell->get_Options_Lock();
    itemcell->ret_Options_opt().set_speed_limit(fspeed);
    itemcell->ret_Options().set_speed_limit(fspeed);
    itemcell->release_Options_Lock();
    break;
  }
}

void Download_change_speed(float fspeed, ListEntry *listentry)
{

  GList *node = GTK_CLIST(listentry->ret_Dl_clist())->selection;
  if(node == NULL) {
    return;
  }
  listentry->get_Dl_clist_lock();
  while(node) {
    int rowindex = GPOINTER_TO_INT(node->data);
    ItemCell *itemcell = (ItemCell *)gtk_clist_get_row_data(GTK_CLIST(listentry->ret_Dl_clist()), rowindex);
    Download_change_speed_sub(itemcell, fspeed);

    node = g_list_next(node);
  }
  listentry->release_Dl_clist_lock();
}

//
// 選択されたアイテムのダウンロードを開始
//
static bool Download_start_sub(ItemCell *itemcell, int rowindex, ListEntry *listentry, bool all_flag = false)
{
  bool retval = false;
  bool er_flag = false;
  switch(itemcell->ret_Status()) {
  case ItemCell::ITEM_ERROR:
    if(all_flag && g_appOption->ret_use_ignore_error_item()) {
      break;
    }
    er_flag = true;
  case ItemCell::ITEM_STOP:
    {
      //suminfo.inc_ready();
      if(er_flag) {
	g_summaryInfo.dec_error();
      } else {
	//suminfo.dec_stop();
      }

      itemcell->set_Status(ItemCell::ITEM_READY);
      listentry->Set_clist_column__icon(rowindex, ItemCell::ITEM_READY);

      retval = true;
      break;
    }
  case ItemCell::ITEM_DOWNLOAD_PARTIAL:
    {
      for(list<ItemCell*>::const_iterator item_ptr = itemcell->ret_Worker_list().begin(); item_ptr != itemcell->ret_Worker_list().end(); ++item_ptr) {
	int rowindex = gtk_clist_find_row_from_data(GTK_CLIST(listentry->ret_Dl_clist()), *item_ptr);
	if(Download_start_sub((ItemCell*)*item_ptr, rowindex, listentry)) {
	  retval = true;
	}
      }
      break;
    }
  case ItemCell::ITEM_READY://added 2001/3/21
  case ItemCell::ITEM_READY_CONCAT://added 2001/3/21
    retval = true;
    break;
  case ItemCell::ITEM_READY_AGAIN://added 2001/3/21
    //itemcell->set_Status(ItemCell::ITEM_READY);
    retval = true;
  default:
    break;
  }
  return retval;
}

gboolean Download_start(GtkWidget *w, gpointer data)
{
  ListEntry *listentry = g_listManager->ret_Current_listentry();

  GtkWidget *clist = listentry->ret_Dl_clist();

  GList *node = GTK_CLIST(clist)->selection;
  if(node == NULL) {
    return TRUE;
  }

  listentry->get_Dl_clist_lock();
  listentry->freezeDlCList();

  // int count added 2001/3/21
  int count = 0;
  while(node) {
    int rowindex = GPOINTER_TO_UINT(node->data);
    ItemCell *itemcell = (ItemCell *)gtk_clist_get_row_data(GTK_CLIST(clist), rowindex);
    if(Download_start_sub(itemcell, rowindex, listentry)) {
      ++count;
    }
    node = g_list_next(node);
  }
  if(g_appOption->isForceDownloadNowEnabled()) {
    int diff = 0;
    diff = count+listentry->getThreadManager()->getTotalThread()-listentry->getThreadLimit();
    
    if(diff > 0) {
      list<int> activeRowList = listentry->getActiveRowList();
      
      list<int>::iterator itr = activeRowList.begin();
      while(itr != activeRowList.end() && (int)activeRowList.size() > diff) {
	++itr;
	++diff;
      }
      while(itr != activeRowList.end()) {
	int rowindex = (*itr);
	ItemCell *itemcell = listentry->getItemCellByRow(rowindex);
	Download_stop_sub(itemcell, rowindex, listentry);
	++itr;
      }
    }
  }
  listentry->thawDlCList();
  // added 2001/3/21
  if(count) {
    listentry->Send_start_signal();
    // update sumup informatiaon
    ItemStatusSum *itemstatus = new ItemStatusSum(g_consoleItem);
    Send_report(MSG_SYS_INFO, itemstatus);
  }
  listentry->release_Dl_clist_lock();

  return TRUE;
}

void Update_sumup_info(ItemCell::ItemStatusType status)
{
  switch(status) {
  case ItemCell::ITEM_ERROR:
    g_summaryInfo.dec_error();
    break;
    /*
  case ItemCell::ITEM_READY:
  case ItemCell::ITEM_READY_AGAIN:
  case ItemCell::ITEM_READY_CONCAT:
    itemstatus->dec_ready();
    break;
  case ItemCell::ITEM_STOP:
    itemstatus->dec_stop();
    break;
  case ItemCell::ITEM_LOCK:
    itemstatus->dec_locked();
    break;
  case ItemCell::ITEM_ERROR:
    itemstatus->dec_error();
    break;
  case ItemCell::ITEM_COMPLETE:
    itemstatus->dec_complete();
    break;
    */
  default:
    break;
  }
}

//
// CLIST itemlistwidget上の選択されたアイテムの削除
//
// 実際の削除を実行する。削除しようとするアイテムがitemcellである。
// 削除しようとしているアイテムがスレッドによって操作されている場合、
// スレッドにメッセージを送りスレッドのほうで削除してもらう。
bool Download_clear_sub(ItemCell *itemcell, ListEntry *listentry)
{
  if(listentry->getItemManager()->search_item(itemcell)) {
    int rowindex = gtk_clist_find_row_from_data(GTK_CLIST(listentry->ret_Dl_clist()), itemcell);
    gtk_clist_remove(GTK_CLIST(listentry->ret_Dl_clist()), rowindex); // remove from clist
    listentry->getItemManager()->unregist_item(itemcell);
    switch(itemcell->ret_Status()) {
    case ItemCell::ITEM_DOWNLOAD:
    case ItemCell::ITEM_DOWNLOAD_AGAIN:
    case ItemCell::ITEM_DOWNLOAD_INTERNAL_AGAIN:
    case ItemCell::ITEM_CRCERROR:
    case ItemCell::ITEM_EXECERROR://added 2001/3/16
      //case ItemCell::ITEM_ERROR://modified 2001/4/21
    case ItemCell::ITEM_INUSE:
    case ItemCell::ITEM_INUSE_AGAIN:
      //case ITEM_INUSE_CONCAT:
      {
	ItemCommand itemcommand;
	itemcommand.commandtype = ItemCommand::COMMAND_DELETE_ITEM;
	itemcommand.eventtype = ItemCommand::EV_USERINTER;//hmmm, may be EV_APPINTER?
	if(write(itemcell->ret_Desc_w(), &itemcommand, sizeof(ItemCommand)) < 0) {
	  Update_sumup_info(itemcell->ret_Status());
	  delete itemcell;// i'm not sure this line is whether safe or unsafe
	}
	break;
      }
    default:
      {
	Update_sumup_info(itemcell->ret_Status());

	delete itemcell;
	break;
      }
    }
    return true;
  } else {
    return false;
  }
}

void Download_clear()
{
  int count = 0;

  ListEntry *listentry = g_listManager->ret_Current_listentry();

  listentry->get_Dl_clist_lock();
  listentry->freezeDlCList();

  GList *node = GTK_CLIST(listentry->ret_Dl_clist())->selection;
  while (node) {
    int rowindex = GPOINTER_TO_UINT(node->data);
    ItemCell* itemcell = (ItemCell*)gtk_clist_get_row_data(GTK_CLIST(listentry->ret_Dl_clist()), rowindex);
    for(list<ItemCell*>::const_iterator item_ptr = itemcell->ret_Worker_list().begin(); item_ptr != itemcell->ret_Worker_list().end(); ++item_ptr) {
      Download_clear_sub((ItemCell*)*item_ptr, listentry);
    }
    if(itemcell->Is_Partial()) {
      node = g_list_next(node);
    } else {
      if(Download_clear_sub(itemcell, listentry)) count++;
      node = GTK_CLIST(listentry->ret_Dl_clist())->selection;
    }
  }

  listentry->thawDlCList();

  if(count > 0) {
    Set_sensitive__no_item_selected();// fix this
    if(GTK_CLIST(listentry->ret_Dl_clist())->rows == 0) {
      Set_sensitive__list_empty(); // fix this
    }
  }
  listentry->release_Dl_clist_lock();
  
  if(count > 0) {
    string line = itos(count)+_(" item(s) cleared");
    g_consoleItem->Send_message_to_gui(line, MSG_SYS_INFO);
    // update sumup informatiaon
    ItemStatusSum *itemstatus = new ItemStatusSum(g_consoleItem);
    Send_report(MSG_SYS_INFO, itemstatus);
  }
}

bool Download_clear_with_file_sub(ItemCell *itemcell, ListEntry *listentry)
{
  try {
    if(listentry->getItemManager()->search_item(itemcell)) {
      int rowindex = gtk_clist_find_row_from_data(GTK_CLIST(listentry->ret_Dl_clist()), itemcell);
      gtk_clist_remove(GTK_CLIST(listentry->ret_Dl_clist()), rowindex); // remove from clist
      listentry->getItemManager()->unregist_item(itemcell);
      switch(itemcell->ret_Status()) {
      case ItemCell::ITEM_DOWNLOAD:
      case ItemCell::ITEM_DOWNLOAD_AGAIN:
      case ItemCell::ITEM_DOWNLOAD_INTERNAL_AGAIN:
      case ItemCell::ITEM_CRCERROR:
      case ItemCell::ITEM_EXECERROR://added 2001/3/16
	//case ItemCell::ITEM_ERROR://modified 2001/4/21
      case ItemCell::ITEM_INUSE:
      case ItemCell::ITEM_INUSE_AGAIN:
	//case ITEM_INUSE_CONCAT:
	{
	  ItemCommand itemcommand;
	  itemcommand.commandtype = ItemCommand::COMMAND_DELETE_ITEM_FILE;
	  itemcommand.eventtype = ItemCommand::EV_USERINTER;//hmmm, may be EV_APPINTER?
	  if(write(itemcell->ret_Desc_w(), &itemcommand, sizeof(ItemCommand)) < 0) {
	    Update_sumup_info(itemcell->ret_Status());
	    if(itemcell->ret_Options_opt().ret_Divide() > 1) {
	      for(int i = 0; i < (int)itemcell->ret_Options_opt().ret_Divide(); ++i) {
		string filename = itemcell->ret_Options_opt().ret_Store_Dir()+itemcell->ret_Filename()+"."+itos(i);
		if(unlink(filename.c_str()) < 0) {
		  throw 0;
		}
		filename = filename+".index";
		if(unlink(filename.c_str()) < 0) {
		  throw 0;
		}
	      }
	    }
	    if(unlink((itemcell->ret_Options_opt().ret_Store_Dir()+itemcell->ret_Filename()).c_str()) < 0) {
	      throw 0;
	    }
	    delete itemcell;// i'm not sure this line is whether safe or unsafe
	  }
	  break;
	}
      default:
	{
	  Update_sumup_info(itemcell->ret_Status());
	  if(itemcell->ret_Options_opt().ret_Divide() > 1) {
	    for(int i = 0; i < (int)itemcell->ret_Options_opt().ret_Divide(); ++i) {
	      string filename = itemcell->ret_Options_opt().ret_Store_Dir()+itemcell->ret_Filename()+"."+itos(i);
	      if(unlink(filename.c_str()) < 0) {
		throw 0;
	      }
	      filename = filename+".index";
	      if(unlink(filename.c_str()) < 0) {
		throw 0;
	      }
	    }
	  }
	  if(unlink((itemcell->ret_Options_opt().ret_Store_Dir()+itemcell->ret_Filename()).c_str()) < 0) {
	    throw 0;
	  }
	  delete itemcell;
	  break;
	}
      }
      return true;
    } else {
      return false;
    }
  } catch (int err) {
    //g_consoleItem->Send_message_to_gui(_("Error occurred while deleting files")+(":"+itemcell->ret_Options_opt().ret_Store_Dir())+itemcell->ret_Filename(),
    //			       MSG_SYS_ERROR);
    delete itemcell;
    return true;
  }
}

void Download_clear_with_file()
{
  int count = 0;

  ListEntry *listentry = g_listManager->ret_Current_listentry();

  listentry->get_Dl_clist_lock();
  listentry->freezeDlCList();

  GList *node = GTK_CLIST(listentry->ret_Dl_clist())->selection;
  while (node) {
    int rowindex = GPOINTER_TO_UINT(node->data);
    ItemCell* itemcell = (ItemCell*)gtk_clist_get_row_data(GTK_CLIST(listentry->ret_Dl_clist()), rowindex);
    for(list<ItemCell*>::const_iterator item_ptr = itemcell->ret_Worker_list().begin(); item_ptr != itemcell->ret_Worker_list().end(); ++item_ptr) {
      Download_clear_with_file_sub((ItemCell*)*item_ptr, listentry);
    }
    if(itemcell->Is_Partial()) {
      node = g_list_next(node);
    } else {
      if(Download_clear_with_file_sub(itemcell, listentry)) count++;
      node = GTK_CLIST(listentry->ret_Dl_clist())->selection;
    }
  }

  listentry->thawDlCList();

  if(count > 0) {
    Set_sensitive__no_item_selected();// fix this
    if(GTK_CLIST(listentry->ret_Dl_clist())->rows == 0) {
      Set_sensitive__list_empty(); // fix this
    }
  }
  listentry->release_Dl_clist_lock();
  if(count > 0) {
    string line = itos(count)+_(" item(s) cleared, and its file deleted");
    g_consoleItem->Send_message_to_gui(line, MSG_SYS_INFO);
    // update sumup informatiaon
    ItemStatusSum *itemstatus = new ItemStatusSum(g_consoleItem);
    Send_report(MSG_SYS_INFO, itemstatus);
  }    
}

gboolean Download_clear_ok(GtkWidget *w, GtkWidget *window)
{
  g_cDialog->hide();
  Download_clear();

  return TRUE;
}

gboolean Download_clear_c(GtkWidget *w, gpointer data)
{
  if(g_appOption->ret_confirm_clear()) {
    g_cDialog->setup(_("Clear items"),
		   _("Are you sure to delete these items?"),
		   Download_clear_ok);
    g_cDialog->set_cancel_button_visible(false);
    g_cDialog->show();
  } else {
    Download_clear();
  }
  return TRUE;
}

gboolean Download_clear_with_file_ok(GtkWidget *w, GtkWidget *window)
{
  g_cDialog->hide();
  Download_clear_with_file();

  return TRUE;
}

gboolean Download_clear_with_file_c(GtkWidget *w, gpointer data)
{
  if(g_appOption->ret_confirm_clear()) {
    g_cDialog->setup(_("Clear items and delete their files"),
		   _("Are you sure to delete these items and their files?"),
		   Download_clear_with_file_ok);
    g_cDialog->set_cancel_button_visible(false);
    g_cDialog->show();
  } else {
    Download_clear_with_file();
  }
  return TRUE;
}

//
// 選択されたアイテムを再度ダウンロード
//
void Download_download_again_sub(ItemCell *itemcell, int rowindex, ListEntry *listentry)
{
  ItemCommand itemcommand;
  
  switch(itemcell->ret_Status()) {
  case ItemCell::ITEM_ERROR:
    g_summaryInfo.dec_error();
  case ItemCell::ITEM_STOP:
  case ItemCell::ITEM_COMPLETE:
  case ItemCell::ITEM_READY:
  case ItemCell::ITEM_READY_AGAIN:
    {
      // delete file
      // acquire lock
      itemcell->get_Options_Lock();
      string filename = itemcell->ret_Options_opt().ret_Store_Dir()+itemcell->ret_Filename();
      unlink(filename.c_str());

      if(itemcell->ret_Options_opt().ret_Divide() > 1) {
	for(int i = 0; i < (int)itemcell->ret_Options_opt().ret_Divide(); ++i) {
	  string filename = itemcell->ret_Options_opt().ret_Store_Dir()+itemcell->ret_Filename()+"."+itos(i);
	  unlink(filename.c_str());
	  filename = filename+".index";
	  unlink(filename.c_str());
	}
      }
      // release lock
      itemcell->release_Options_Lock();
      itemcell->set_Status(ItemCell::ITEM_READY_AGAIN);
      itemcell->set_Size_Current(0);
      itemcell->set_Size_Total(0);

      ListEntry *listentry = g_listManager->ret_Current_listentry();
      listentry->Set_clist_column__icon(rowindex, itemcell->ret_Status());
      listentry->Set_clist_column__cursize(rowindex, itos(itemcell->ret_Size_Current()));
      listentry->Set_clist_column__totsize(rowindex, itos(itemcell->ret_Size_Total()));
      listentry->Set_clist_column__progress(rowindex, 0);

      break;
    }
  case ItemCell::ITEM_INUSE_AGAIN:
  case ItemCell::ITEM_CRCERROR: // added 2001/3/16
  case ItemCell::ITEM_EXECERROR:// added 2001/3/16
  case ItemCell::ITEM_DOWNLOAD:
  case ItemCell::ITEM_INUSE:
  case ItemCell::ITEM_DOWNLOAD_AGAIN:
  case ItemCell::ITEM_DOWNLOAD_INTERNAL_AGAIN:
    {
      itemcommand.commandtype = ItemCommand::COMMAND_DOWNLOAD_AGAIN;
      itemcommand.eventtype = ItemCommand::EV_USERINTER;
      write(itemcell->ret_Desc_w(), &itemcommand, sizeof(ItemCommand));
      break;
    }
  case ItemCell::ITEM_DOWNLOAD_PARTIAL:
    {
      //ちょいと制約を...
      //分割ダウンロードをしている場合, 新たに設定した分割数でダウンロード
      //しなおすには, 派生したすべての分割ダウンロードアイテムが停止状態
      //(ITEM_READY, ITEM_STOP, ITEM_ERROR, ITEM_COMPLETE)の時にダウンロード
      //メニューから「ダウンロードをやり直す」を選択する方法しかありません.
      //もし, 一つでも派生したアイテムが停止状態にない場合, 分割数を変更し,
      //「ダウンロードをやり直す」を選択しても, 変更前の分割数でダウンロード
      //をやり直すことになります.
      //すみません..
      if(itemcell->ret_Worker_list().size() == 0) {
	break;
      }
      //check whether all splited items are halt state
      bool allStopFlag = true;
      for(list<ItemCell*>::const_iterator item_ptr = itemcell->ret_Worker_list().begin(); item_ptr != itemcell->ret_Worker_list().end(); ++item_ptr) {
	ItemCell *itemcell_worker = (ItemCell*)*item_ptr;
	switch(itemcell_worker->ret_Status()) {
	case ItemCell::ITEM_READY:
	case ItemCell::ITEM_READY_AGAIN:
	case ItemCell::ITEM_STOP:
	case ItemCell::ITEM_ERROR:
	  //case ItemCell::ITEM_COMPLETE:
	  break;
	default:
	  {
	    allStopFlag = false;
	    break;
	  }
	}
	if(!allStopFlag) break;
      }
      ListEntry *listentry = g_listManager->ret_Current_listentry();
      if(!allStopFlag) {
	// if any is not in halt state,...
	for(list<ItemCell*>::const_iterator item_ptr = itemcell->ret_Worker_list().begin(); item_ptr != itemcell->ret_Worker_list().end(); ++item_ptr) {
	  int rowindex = gtk_clist_find_row_from_data(GTK_CLIST(listentry->ret_Dl_clist()), (ItemCell*)*item_ptr);
	  Download_download_again_sub((ItemCell*)*item_ptr, rowindex, listentry);
	}
	break;
      } else {
	// all items are in halt state..
	// delete file
	for(int i = 0; i < (int)itemcell->ret_Options().ret_Divide(); ++i) {
	  string filename = itemcell->ret_Options().ret_Store_Dir()+itemcell->ret_Filename()+"."+itos(i);
	  unlink(filename.c_str());
	  string indexfilename = filename+".index";
	  unlink(indexfilename.c_str());
	}
	// delete worker
	for(list<ItemCell*>::const_iterator item_ptr = itemcell->ret_Worker_list().begin(); item_ptr != itemcell->ret_Worker_list().end(); ++item_ptr) {
	  Download_clear_sub(*item_ptr, listentry);
	}
	itemcell->Clear_worker();

	// update sumup info
	itemcell->set_Status(ItemCell::ITEM_READY_AGAIN);
	itemcell->set_Size_Current(0);
	itemcell->set_Size_Total(0);

	listentry->Set_clist_column__icon(rowindex, itemcell->ret_Status());
	listentry->Set_clist_column__cursize(rowindex, itos(itemcell->ret_Size_Current()));
	listentry->Set_clist_column__totsize(rowindex, itos(itemcell->ret_Size_Total()));
	listentry->Set_clist_column__progress(rowindex, 0);

	
	if(GTK_CLIST(listentry->ret_Dl_clist())->selection <= 0) {
	  Set_sensitive__no_item_selected(); // fix this
	}
	if(GTK_CLIST(listentry->ret_Dl_clist())->rows == 0) {
	  Set_sensitive__list_empty(); // fix this
	}
	break;
      }
      break;
    }
  default:
    break;
  }
}

gboolean Download_download_again(GtkWidget *w, gpointer data)
{
  ListEntry *listentry = g_listManager->ret_Current_listentry();
  listentry->get_Dl_clist_lock();

  GList *node = GTK_CLIST(listentry->ret_Dl_clist())->selection;

  while(node) {
    unsigned int rowindex = GPOINTER_TO_UINT(node->data);
    ItemCell *itemcell = (ItemCell*)gtk_clist_get_row_data(GTK_CLIST(listentry->ret_Dl_clist()), rowindex);

    Download_download_again_sub(itemcell, rowindex, listentry);
    node = g_list_next(node);   
  }
  listentry->Send_start_signal();
  listentry->release_Dl_clist_lock();

  ItemStatusSum *itemstatus = new ItemStatusSum(g_consoleItem);
  Send_report(MSG_SYS_INFO, itemstatus);

  return TRUE;
}

//
// 選択されたアイテムのダウンロードを停止
//
//void Download_stop_sub(ItemCell *itemcell, int rowindex, ListEntry *listentry, bool all_flag = false, ItemCommand::EventCause eventtype = ItemCommand::EV_USERINTER)
void Download_stop_sub(ItemCell *itemcell, int rowindex, ListEntry *listentry, bool all_flag, ItemCommand::EventCause eventtype)
{
  switch(itemcell->ret_Status()) {
    /* //modified 2001/3/2
  case ItemCell::ITEM_ERROR:
    if(all_flag && g_appOption->ret_use_ignore_error_item()) {
      break;
    }
    */
  case ItemCell::ITEM_DOWNLOAD:
  case ItemCell::ITEM_DOWNLOAD_AGAIN:
  case ItemCell::ITEM_DOWNLOAD_INTERNAL_AGAIN:
  case ItemCell::ITEM_CRCERROR:
  case ItemCell::ITEM_EXECERROR:
  case ItemCell::ITEM_INUSE:
  case ItemCell::ITEM_INUSE_AGAIN:
    {
      ItemCommand itemcommand;
      itemcommand.commandtype = ItemCommand::COMMAND_STOP;
      itemcommand.eventtype = eventtype;
      if(write(itemcell->ret_Desc_w(), &itemcommand, sizeof(ItemCommand)) < 0) {
	// may be item is not driven by download thread or,
	// item status is ITEM_CRCERROR or ITEM_ERROR and not are driven by
	// download thread
      } else {//modified 2001/3/2
	listentry->Set_clist_column__icon(rowindex, ItemCell::ITEM_STOP);
      }
      break;
    }
  case ItemCell::ITEM_READY:
  case ItemCell::ITEM_READY_AGAIN:
    {
      itemcell->set_Status(ItemCell::ITEM_STOP);
      listentry->Set_clist_column__icon(rowindex, ItemCell::ITEM_STOP);
      break;
    }
  case ItemCell::ITEM_DOWNLOAD_PARTIAL:
    {
      for(list<ItemCell*>::const_iterator item_ptr = itemcell->ret_Worker_list().begin(); item_ptr != itemcell->ret_Worker_list().end(); ++item_ptr) {
	int rowindex = gtk_clist_find_row_from_data(GTK_CLIST(listentry->ret_Dl_clist()), *item_ptr);
	Download_stop_sub(*item_ptr, rowindex, listentry);
      }
      break;
    }
  default:
    break;
  }
}

gboolean Download_stop(GtkWidget *w, gpointer data)
{
  ListEntry *listentry = g_listManager->ret_Current_listentry();
  listentry->get_Dl_clist_lock();

  GList *node = GTK_CLIST(listentry->ret_Dl_clist())->selection;
  while(node) {
    int rowindex = GPOINTER_TO_UINT(node->data);
    ItemCell* itemcell = (ItemCell *)gtk_clist_get_row_data(GTK_CLIST(listentry->ret_Dl_clist()), rowindex);
    Download_stop_sub(itemcell, rowindex, listentry);
    node = g_list_next(node);
  }
  listentry->release_Dl_clist_lock();
  // update sumup informatiaon
  ItemStatusSum *itemstatus = new ItemStatusSum(g_consoleItem);
  Send_report(MSG_SYS_INFO, itemstatus);

  return TRUE;
}


//
// 選択されたアイテムのCRCを削除
//
void Download_clear_crc_sub(GtkWidget *clist, int rowindex)
{
  ItemCell* itemcell = (ItemCell*)gtk_clist_get_row_data(GTK_CLIST(clist), rowindex);
  itemcell->set_CRC_Type(ItemCell::CRC_NONE);
}

gboolean Download_clear_crc(GtkWidget *w, gpointer data)
{
  ListEntry *listentry = g_listManager->ret_Current_listentry();
  listentry->get_Dl_clist_lock();

  GList *node = GTK_CLIST(listentry->ret_Dl_clist())->selection;
  while(node) {
    int rowindex = GPOINTER_TO_UINT(node->data);
    Download_clear_crc_sub(listentry->ret_Dl_clist(), rowindex);
    listentry->Set_clist_column__crc(rowindex, "");

    node = g_list_next(node);
  }
  listentry->release_Dl_clist_lock();
  return TRUE;
}

//
// 選択されたアイテムの MD5 digest message を削除
//
void Download_clear_md5_sub(GtkWidget *clist, int rowindex)
{
  ItemCell *itemcell = (ItemCell *)gtk_clist_get_row_data(GTK_CLIST(clist), rowindex);
  itemcell->set_md5string("");
}

gboolean Download_clear_md5(GtkWidget* w, gpointer data)
{
  ListEntry *listentry = g_listManager->ret_Current_listentry();
  listentry->get_Dl_clist_lock();
  GList *node = GTK_CLIST(listentry->ret_Dl_clist())->selection;
  while(node) {
    int rowindex = GPOINTER_TO_UINT(node->data);
    Download_clear_md5_sub(listentry->ret_Dl_clist(), rowindex);
    listentry->Set_clist_column__md5(rowindex, "");

    node = g_list_next(node);
  }
  listentry->release_Dl_clist_lock();
  return TRUE;
}

// execute command of selected items.
// command is specified in item option

gboolean Download_executeCommand(GtkWidget* w, gpointer data)
{
  ListEntry *listentry = g_listManager->ret_Current_listentry();
  listentry->get_Dl_clist_lock();
  GList *node = GTK_CLIST(listentry->ret_Dl_clist())->selection;
  while(node) {
    int rowindex = GPOINTER_TO_UINT(node->data);
    ItemCell* itemcell = (ItemCell*)gtk_clist_get_row_data(GTK_CLIST(listentry->ret_Dl_clist()), rowindex);
    Execute_command_in_option(itemcell);

    node = g_list_next(node);

  }
  listentry->release_Dl_clist_lock();
  return TRUE;
}

//
// すべてのアイテムのCRCを削除
//
static gboolean Download_clear_crc_all(GtkWidget *w, gpointer data)
{
  ListEntry *listentry = g_listManager->ret_Current_listentry();

  listentry->get_Dl_clist_lock();
  listentry->freezeDlCList();

  for(int rowindex = 0; rowindex < GTK_CLIST(listentry->ret_Dl_clist())->rows; ++rowindex) {
    Download_clear_crc_sub(listentry->ret_Dl_clist(), rowindex);
    listentry->Set_clist_column__crc(rowindex, "");
  }  

  listentry->thawDlCList();
  listentry->release_Dl_clist_lock();

  return TRUE;
}

//
// すべてのアイテムの MD5 digest message を削除
//
static gboolean Download_clear_md5_all(GtkWidget *w, gpointer data)
{
  ListEntry *listentry = g_listManager->ret_Current_listentry();

  listentry->get_Dl_clist_lock();
  listentry->freezeDlCList();

  for(int rowindex = 0; rowindex < GTK_CLIST(listentry->ret_Dl_clist())->rows; ++rowindex) {
    Download_clear_md5_sub(listentry->ret_Dl_clist(), rowindex);
    listentry->Set_clist_column__md5(rowindex, "");
  }  

  listentry->thawDlCList();
  listentry->release_Dl_clist_lock();

  return TRUE;
}

//
// subroutine of starting downloads
//
static void Download_start_all_sub(ListEntry *listentry)
{
  list<GtkTreePath*> selection_temp;
  Backup_selection_list(listentry->ret_Dl_clist(), selection_temp);

  /*gtk_clist_select_all(GTK_CLIST(listentry->ret_Dl_clist()));
  GList *node = GTK_CLIST(listentry->ret_Dl_clist())->selection;
  if(node == NULL) {
    return;
  }
  while(node) {
    int rowindex = GPOINTER_TO_UINT(node->data);
    ItemCell* itemcell = (ItemCell*)gtk_clist_get_row_data(GTK_CLIST(listentry->ret_Dl_clist()), rowindex);
    Download_start_sub(itemcell, rowindex, listentry, true);
    node = g_list_next(node);
  }*/
  for (size_t i=0; i < listentry->getRowCount(); ++i)
  {
    ItemCell* itemcell = listentry->getItemCellByRow(i);
    Download_start_sub(itemcell, i, listentry, true);
  }
  //gtk_clist_unselect_all(GTK_CLIST(listentry->ret_Dl_clist())); // fix this

  Restore_selection_list(listentry->ret_Dl_clist(), selection_temp);

  listentry->Send_start_signal();
}

//
// すべて停止状態のアイテムを待機状態(READY)にし, ダウンロードを開始
//

gboolean Download_start_all(GtkWidget *w, gpointer data)
{
  ListEntry *listentry = g_listManager->ret_Current_listentry();

  listentry->get_Dl_clist_lock();
  listentry->freezeDlCList();

  Download_start_all_sub(listentry);

  listentry->thawDlCList();
  listentry->release_Dl_clist_lock();
  
  return TRUE;
}

gboolean Download_start_all_by_listentry(ListEntry *listentry) {
  Download_start_all_sub(listentry);  
  return TRUE;
}
//
// すべて停止状態のアイテムを待機状態(READY)にし, ダウンロードを開始
// すべてのタブで
//
gboolean Download_start_all_list(GtkWidget *w, gpointer data)
{
  for(list<ListEntry*>::const_iterator itr = g_listManager->ret_Listentry_list().begin(); itr != g_listManager->ret_Listentry_list().end(); ++itr) {
    ListEntry *listentry = *itr;

    listentry->get_Dl_clist_lock();
    listentry->freezeDlCList();

    Download_start_all_sub(listentry);

    listentry->thawDlCList();
    listentry->release_Dl_clist_lock();
  }
  return TRUE;
}

//
// すべてのアイテムのダウンロードをやり直す の実体
//
// Subroutine for "Download again" menu
//
static void Download_download_again_all_sub(ListEntry *listentry)
{
  //gtk_clist_freeze(GTK_CLIST(listentry->ret_Dl_clist()));
  list<GtkTreePath*> selection_temp;
  Backup_selection_list(listentry->ret_Dl_clist(), selection_temp);

  gtk_clist_select_all(GTK_CLIST(listentry->ret_Dl_clist()));
  GList *node = GTK_CLIST(listentry->ret_Dl_clist())->selection;
  if(node == NULL) {
    return;
  }
  while(node) {
    int rowindex = GPOINTER_TO_UINT(node->data);
    ItemCell* itemcell = (ItemCell*)gtk_clist_get_row_data(GTK_CLIST(listentry->ret_Dl_clist()), rowindex);
    Download_download_again_sub(itemcell, rowindex, listentry);
    node = g_list_next(node);
  }
  gtk_clist_unselect_all(GTK_CLIST(listentry->ret_Dl_clist()));

  Restore_selection_list(listentry->ret_Dl_clist(), selection_temp);

  //gtk_clist_thaw(GTK_CLIST(listentry->ret_Dl_clist()));

  listentry->Send_start_signal();
}

//
// すべてのアイテムのダウンロードをやり直す
//
// Callback function for "Download again" menu
gboolean Download_download_again_all(GtkWidget* w, gpointer data)
{
  ListEntry *listentry = g_listManager->ret_Current_listentry();

  listentry->get_Dl_clist_lock();
  listentry->freezeDlCList();

  Download_download_again_all_sub(listentry);

  listentry->thawDlCList();
  listentry->release_Dl_clist_lock();

  return TRUE;
}

//
// すべてのアイテムのダウンロードをやり直す (すべてのタブで)
//
gboolean Download_download_again_all_list(GtkWidget* w, gpointer data)
{
  for(list<ListEntry*>::const_iterator itr = g_listManager->ret_Listentry_list().begin(); itr != g_listManager->ret_Listentry_list().end(); ++itr) {
    ListEntry *listentry = *itr;
    listentry->get_Dl_clist_lock();
    listentry->freezeDlCList();

    Download_download_again_all_sub(listentry);
    
    listentry->thawDlCList();
    listentry->release_Dl_clist_lock();
  }
  return TRUE;
}

//
// subroutine of stopping downloads
//
void Download_stop_all_sub(ListEntry *listentry, bool timerFlag)
{
  //gtk_clist_freeze(GTK_CLIST(listentry->ret_Dl_clist()));
  list<GtkTreePath*> selection_temp;
  Backup_selection_list(listentry->ret_Dl_clist(), selection_temp);

  gtk_clist_select_all(GTK_CLIST(listentry->ret_Dl_clist()));

  GList *node = GTK_CLIST(listentry->ret_Dl_clist())->selection;
  while(node) {
    int rowindex = GPOINTER_TO_UINT(node->data);
    ItemCell* itemcell = (ItemCell*)gtk_clist_get_row_data(GTK_CLIST(listentry->ret_Dl_clist()), rowindex);
    if(timerFlag) {
      Download_stop_sub(itemcell, rowindex, listentry, true, ItemCommand::EV_TIMERINTER);
    } else {
      Download_stop_sub(itemcell, rowindex, listentry, true);
    }
    node = g_list_next(node);
  }

  gtk_clist_unselect_all(GTK_CLIST(listentry->ret_Dl_clist()));

  Restore_selection_list(listentry->ret_Dl_clist(), selection_temp);

  //gtk_clist_thaw(GTK_CLIST(listentry->ret_Dl_clist()));
}

//
// すべてのアイテムを停止状態にする
//
gboolean Download_stop_all(GtkWidget *w, gpointer data)
{
  ListEntry *listentry = g_listManager->ret_Current_listentry();

  listentry->get_Dl_clist_lock();
  listentry->freezeDlCList();

  Download_stop_all_sub(listentry, false);

  listentry->thawDlCList();
  listentry->release_Dl_clist_lock();

  return TRUE;
}

//
// すべてのアイテムを停止状態にする (すべてのタブで)
//
gboolean Download_stop_all_list(GtkWidget *w, gpointer data)
{
  for(list<ListEntry*>::const_iterator itr = g_listManager->ret_Listentry_list().begin(); itr != g_listManager->ret_Listentry_list().end(); ++itr) {
    ListEntry *listentry = *itr;
    listentry->get_Dl_clist_lock();
    Download_stop_all_sub(listentry, false);
    listentry->release_Dl_clist_lock();
  }
  return TRUE;
}

gboolean Download_stop_all_list_on_timer(GtkWidget*w, gpointer data)
{
  for(list<ListEntry*>::const_iterator itr = g_listManager->ret_Listentry_list().begin(); itr != g_listManager->ret_Listentry_list().end(); ++itr) {
    ListEntry *listentry = *itr;
    listentry->get_Dl_clist_lock();
    Download_stop_all_sub(listentry, true);
    listentry->release_Dl_clist_lock();
  }
  return TRUE;  
}

static void Download_clear_all_sub(ListEntry *listentry)
{
  int count = 0;

  //gtk_clist_freeze(GTK_CLIST(listentry->ret_Dl_clist()));
  gtk_clist_select_all(GTK_CLIST(listentry->ret_Dl_clist()));

  GList *node = GTK_CLIST(listentry->ret_Dl_clist())->selection;

  while (node) {
    int rowindex = GPOINTER_TO_UINT(node->data);
    ItemCell *itemcell = (ItemCell*)gtk_clist_get_row_data(GTK_CLIST(listentry->ret_Dl_clist()), rowindex);
    for(list<ItemCell*>::const_iterator item_ptr = itemcell->ret_Worker_list().begin(); item_ptr != itemcell->ret_Worker_list().end(); ++item_ptr) {
      Download_clear_sub((ItemCell*)*item_ptr, listentry);
    }
    if(itemcell->Is_Partial()) {
      node = g_list_next(node);
    } else {
      if(Download_clear_sub(itemcell, listentry)) count++;
      node = GTK_CLIST(listentry->ret_Dl_clist())->selection;
    }
  }
  //gtk_clist_thaw(GTK_CLIST(listentry->ret_Dl_clist()));
  
  if(count > 0) {
    string line = itos(count)+_(" item(s) deleted");
    g_consoleItem->Send_message_to_gui(line, MSG_SYS_INFO);
    Set_sensitive__no_item_selected();// fix this
    if(GTK_CLIST(listentry->ret_Dl_clist())->rows == 0) {
      Set_sensitive__list_empty(); // fix this
    }
    // update sumup informatiaon
    ItemStatusSum *itemstatus = new ItemStatusSum(g_consoleItem);
    Send_report(MSG_SYS_INFO, itemstatus);
  }
}

//
// すべてのアイテムを削除する
//
void Download_clear_all()
{
  ListEntry *listentry = g_listManager->ret_Current_listentry();
  
  listentry->get_Dl_clist_lock();
  listentry->freezeDlCList();

  Download_clear_all_sub(listentry);

  listentry->thawDlCList();
  listentry->release_Dl_clist_lock();
}

//
// すべてのアイテムを削除する (すべてのタブで)
//
gboolean Download_clear_all_list(GtkWidget* w, gpointer data)
{
  for(list<ListEntry*>::const_iterator itr = g_listManager->ret_Listentry_list().begin(); itr != g_listManager->ret_Listentry_list().end(); ++itr) {
    ListEntry *listentry = *itr;
    listentry->get_Dl_clist_lock();
    Download_clear_all_sub(listentry);
    listentry->release_Dl_clist_lock();
  }
  return TRUE;
}

static gboolean Download_clear_all_ok(GtkWidget *w, GtkWidget *window)
{
  g_cDialog->hide();
  Download_clear_all();

  return TRUE;
}

static gboolean Download_clear_all_list_ok(GtkWidget *w, GtkWidget *window)
{
  g_cDialog->hide();
  Download_clear_all_list(w, NULL);

  return TRUE;
}

// callback "Clear all"
gboolean Download_clear_all_c(GtkWidget *w,
			       gboolean (*OkFunc)(GtkWidget *w, GtkWidget *window))
{
  if(g_appOption->ret_confirm_clear()) {
    g_cDialog->setup(_("Clear items"),
		   _("Are you sure to delete these items?"),
		   OkFunc);
    g_cDialog->set_cancel_button_visible(false);
    g_cDialog->show();
  } else {
    OkFunc(w, NULL);
  }
  return TRUE;
}

//
// ダウンロードメニューを作成
//
void Create_download_menu(GtkWidget *toplevel, GtkWidget* menu_bar, GtkAccelGroup* accel_group)
{
  GtkWidget *menu;
  GtkWidget *root_item;
  
  menu = gtk_menu_new();
  // menu for selected items
  start_item = GTK_create_menu_item_with_icon(menu,
					      _("Start"),
					      GTK_SIGNAL_FUNC(Download_start),
					      NULL,
					      start_xpm,
					      toplevel,
					      accel_group,
					      SC_START,
					      SCM_START);

  stop_item = GTK_create_menu_item_with_icon(menu,
					     _("Stop"),
					     GTK_SIGNAL_FUNC(Download_stop),
					     NULL,
					     pause_xpm,
					     toplevel,
					     accel_group,
					     SC_STOP,
					     SCM_STOP);

  clear_item = GTK_create_menu_item_with_icon(menu,
					      _("Clear"),
					      GTK_SIGNAL_FUNC(Download_clear_c),
					      NULL,
					      trash_xpm,
					      toplevel,
					      accel_group,
					      SC_CLEAR,
					      SCM_CLEAR);

  clear_with_file_item = GTK_create_menu_item_with_icon(menu,
							_("Clear and delete files"),
							GTK_SIGNAL_FUNC(Download_clear_with_file_c),
							NULL,
							accel_group,
							SC_CLEARWITHFILE,
							SCM_CLEARWITHFILE);
							  
  clearCRC_item = GTK_create_menu_item_with_icon(menu,
						 _("Clear CRC"),
						 GTK_SIGNAL_FUNC(Download_clear_crc),
						 NULL);

  clearMD5Item = GTK_create_menu_item_with_icon(menu,
						_("Clear MD5"),
						GTK_SIGNAL_FUNC(Download_clear_md5),
						NULL);

  checkCRC_item = GTK_create_menu_item_with_icon(menu,
						 _("Check CRC"),
						 GTK_SIGNAL_FUNC(Download_check_crc),
						 NULL);

  checkMD5Item = GTK_create_menu_item_with_icon(menu,
						_("Check MD5"),
						GTK_SIGNAL_FUNC(Download_check_md5),
						NULL);

  downloadAgain_item = GTK_create_menu_item_with_icon(menu,
						      _("Download again"),
						      GTK_SIGNAL_FUNC(Download_download_again),
						      NULL,
						      restart_xpm,
						      toplevel,
						      accel_group,
						      SC_DOWNLOADAGAIN,
						      SCM_DOWNLOADAGAIN);

  GTK_create_menu_separator(menu);

  // menu for all items
  startAll_item = GTK_create_menu_item_with_icon(menu,
						 _("Start all"),
						 GTK_SIGNAL_FUNC(Download_start_all),
						 NULL,
						 start_all_xpm,
						 toplevel,
						 accel_group,
						 SC_START_ALL,
						 SCM_START_ALL);

  stopAll_item = GTK_create_menu_item_with_icon(menu,
						_("Stop all"),
						GTK_SIGNAL_FUNC(Download_stop_all),
						NULL,
						pause_all_xpm,
						toplevel,
						accel_group,
						SC_STOP_ALL,
						SCM_STOP_ALL);
 
  clearAll_item = GTK_create_menu_item_with_icon(menu,
				       _("Clear all"),
				       GTK_SIGNAL_FUNC(Download_clear_all_c),
				       (void *)Download_clear_all_ok,
				       accel_group,
				       SC_CLEAR_ALL,
				       SCM_CLEAR_ALL);

  clearCRCall_item = GTK_create_menu_item_with_icon(menu,
						    _("Clear all CRC"),
						    GTK_SIGNAL_FUNC(Download_clear_crc_all),
						    NULL);
  clearMD5AllItem = GTK_create_menu_item_with_icon(menu,
						   _("Clear all MD5"),
						   GTK_SIGNAL_FUNC(Download_clear_md5_all),
						   NULL);

  GTK_create_menu_separator(menu);

  // menu for all lists
  startAllList_item = GTK_create_menu_item_with_icon(menu,
						     _("Start all lists"),
						     GTK_SIGNAL_FUNC(Download_start_all_list),
						     NULL,
						     accel_group,
						     SC_START_ALL_LIST,
						     SCM_START_ALL_LIST);

  stopAllList_item = GTK_create_menu_item_with_icon(menu,
						    _("Stop all lists"),
						    GTK_SIGNAL_FUNC(Download_stop_all_list),
						    NULL,
						    accel_group,
						    SC_STOP_ALL_LIST,
						    SCM_STOP_ALL_LIST);

  clearAllList_item = GTK_create_menu_item_with_icon(menu,
						     _("Clear all lists"),
						     GTK_SIGNAL_FUNC(Download_clear_all_c),
						     (void *)Download_clear_all_list_ok,
						     accel_group,
						     SC_CLEAR_ALL_LIST,
						     SCM_CLEAR_ALL_LIST);

  GTK_create_menu_separator(menu);

  checkCRCdownloaded_item = GTK_create_menu_item_with_icon(menu,
						 _("Check downloaded items' CRC"),
						 GTK_SIGNAL_FUNC(Download_check_crc_downloaded),
						 NULL);

  checkMD5DownloadedItem = GTK_create_menu_item_with_icon(menu,
							  _("Check downloaded items' MD5"),
							  GTK_SIGNAL_FUNC(Download_check_md5_downloaded),
							  NULL);
  
  // set sensitivity
  Download_set_sensitive__no_item_selected();
  Download_set_sensitive__list_empty();

  root_item = gtk_menu_item_new_with_label(_("Download"));
  gtk_widget_show(root_item);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(root_item), menu);
  gtk_menu_bar_append(GTK_MENU_BAR(menu_bar), root_item);

  if(socketpair(AF_UNIX, SOCK_STREAM, 0, g_checkSockPair) < 0) {
    // need some error handling ??
  }
  gdk_input_add(g_checkSockPair[0], GDK_INPUT_READ, Download_check_end, NULL);
}
