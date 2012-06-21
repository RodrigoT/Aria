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

// $Id: gui_item.cc,v 1.10 2002/02/13 12:09:24 tujikawa Exp $
#include "ListManager.h"
#include "SumInfo.h"
#include "AppOption.h"
#include "Dialog.h"
#include "ShortCutKey.h"
#include "ItemStatusSum.h"
#include "gui_utils.h"

#include "pixmaps/moveup.xpm"
#include "pixmaps/movedown.xpm"
#include "pixmaps/lockon.xpm"
#include "pixmaps/unlock.xpm"

extern gboolean Item_new_item(GtkWidget *w, gpointer data);
extern void Create_new_item_window(GtkWidget *toplevel);
extern gboolean Download_clear_sub(ItemCell *itemcell, ListEntry *listentry);
extern void Set_sensitive__no_item_selected();
extern void Set_sensitive__list_empty();
extern void Send_report(MessageType reporttype, ItemStatus *itemstatus);

extern ListManager *g_listManager;
extern SumInfo g_summaryInfo;
extern AppOption *g_appOption;
extern ItemCell *g_consoleItem;
extern Dialog *g_cDialog;

static GtkWidget *lock_item, *unlock_item;
static GtkWidget *moveUp_item, *moveDown_item;
static GtkWidget *moveTop_item, *moveBottom_item;
static GtkWidget *lockError_item;
static GtkWidget *clearDownloaded_item;
static GtkWidget *clearLock_item;
static GtkWidget *clearError_item;
static GtkWidget *clearStop_item;
static GtkWidget *clearReady_item;
static GtkWidget *clearErrorStopReady_item;

enum ClearStateType {
  DOWNLOAD_DEL_READY = 1 << 0,
  DOWNLOAD_DEL_STOP = 1 << 1,
  DOWNLOAD_DEL_ERROR = 1 << 2,
  DOWNLOAD_DEL_COMPLETE = 1 << 3,
  DOWNLOAD_DEL_LOCK = 1 << 4,
};
static int state_flag;

enum DownloadListMoveType {
  DOWNLOAD_MOVE_TOP,
  DOWNLOAD_MOVE_BOTTOM,
  DOWNLOAD_MOVE_UP,
  DOWNLOAD_MOVE_DOWN
};

static int gt_func(int x, int y)
{
  if(x < y) return 1;
  else return -1;
}

static int lt_func(int x, int y)
{
  if(x > y) return 1;
  else return -1;
}

void Item_set_sensitive__no_item_selected()
{
  gtk_widget_set_sensitive(moveUp_item, FALSE);
  gtk_widget_set_sensitive(moveDown_item, FALSE);
  gtk_widget_set_sensitive(moveTop_item, FALSE);
  gtk_widget_set_sensitive(moveBottom_item, FALSE);
  gtk_widget_set_sensitive(lock_item, FALSE);
  gtk_widget_set_sensitive(unlock_item, FALSE);
}

void Item_set_sensitive__items_selected()
{
  gtk_widget_set_sensitive(moveUp_item, TRUE);
  gtk_widget_set_sensitive(moveDown_item, TRUE);
  gtk_widget_set_sensitive(moveTop_item, TRUE);
  gtk_widget_set_sensitive(moveBottom_item, TRUE);
  gtk_widget_set_sensitive(lock_item, TRUE);
  gtk_widget_set_sensitive(unlock_item, TRUE);
}

void Item_set_sensitive__list_empty()
{
  gtk_widget_set_sensitive(clearDownloaded_item, FALSE);
  gtk_widget_set_sensitive(clearLock_item, FALSE);
  gtk_widget_set_sensitive(clearError_item, FALSE);
  gtk_widget_set_sensitive(clearStop_item, FALSE);
  gtk_widget_set_sensitive(clearReady_item, FALSE);
  gtk_widget_set_sensitive(clearErrorStopReady_item, FALSE);
  gtk_widget_set_sensitive(lockError_item, FALSE);
}

void Item_set_sensitive__list_not_empty()
{
  gtk_widget_set_sensitive(clearDownloaded_item, TRUE);
  gtk_widget_set_sensitive(clearLock_item, TRUE);
  gtk_widget_set_sensitive(clearError_item, TRUE);
  gtk_widget_set_sensitive(clearStop_item, TRUE);
  gtk_widget_set_sensitive(clearReady_item, TRUE);
  gtk_widget_set_sensitive(clearErrorStopReady_item, TRUE);
  gtk_widget_set_sensitive(lockError_item, TRUE);
}

//// modified 2000/3/1
static void Item_move_sub(DownloadListMoveType mtype)
{
  ListEntry *listentry = g_listManager->ret_Current_listentry();
  GtkWidget *clist = listentry->ret_Dl_clist();

  GList *node = GTK_CLIST(clist)->selection;
  if(node == NULL || g_list_length(node) == (unsigned int)GTK_CLIST(clist)->rows) return;
  listentry->get_Dl_clist_lock();
  int diff = 0;

  GList *localcopy = g_list_copy(node);
  switch(mtype) {
  case DOWNLOAD_MOVE_TOP: {
    localcopy = g_list_sort(localcopy, (int (*)(const void *, const void *))lt_func);
    diff = -GPOINTER_TO_UINT(localcopy->data);
    //destrow = 0;

    break;
  }
  case DOWNLOAD_MOVE_BOTTOM:
    localcopy = g_list_sort(localcopy, (int (*)(const void *, const void *))gt_func);
    diff = GTK_CLIST(clist)->rows-1-GPOINTER_TO_UINT(localcopy->data);

    break;
  case DOWNLOAD_MOVE_UP: {
    localcopy = g_list_sort(localcopy, (int (*)(const void *, const void *))lt_func);
    if(GPOINTER_TO_UINT(localcopy->data) == 0) diff = 0;
    else diff = -1;

    //destrow = rowindex-1;
    break;
  }
  case DOWNLOAD_MOVE_DOWN:
    localcopy = g_list_sort(localcopy, (int (*)(const void *, const void *))gt_func);
    if(GPOINTER_TO_UINT(localcopy->data) == (unsigned int)GTK_CLIST(clist)->rows-1) diff = 0;
    else diff = 1;
    //destrow = rowindex+1;
    break;
  default:
    break;
  }
  //localcopy = g_list_sort(node, (int (*)(const void *, const void *))lt_func);
  node = localcopy;

  while(diff && node) {
    int rowindex = GPOINTER_TO_UINT(node->data);
    gtk_clist_row_move(GTK_CLIST(clist), rowindex, rowindex+diff);
    node = g_list_next(node);
  }


  int dstRowindex = GPOINTER_TO_UINT(localcopy->data)+diff;
  if(gtk_clist_row_is_visible(GTK_CLIST(clist), dstRowindex) != GTK_VISIBILITY_FULL) {
    if(mtype == DOWNLOAD_MOVE_UP) {
      gtk_clist_moveto(GTK_CLIST(clist), dstRowindex, 0, 0.0, 0.0);
    } else {
      gtk_clist_moveto(GTK_CLIST(clist), dstRowindex, 0, 1.0, 0.0);
    }
  }

  g_list_free(localcopy);
  listentry->release_Dl_clist_lock();
}


gboolean Item_move_up(GtkWidget *w, gpointer data)
{
  Item_move_sub(DOWNLOAD_MOVE_UP);
  return TRUE;
}

gboolean Item_move_down(GtkWidget *w, gpointer data)
{
  Item_move_sub(DOWNLOAD_MOVE_DOWN);
  return TRUE;
}

gboolean Item_move_top(GtkWidget *w, gpointer data)
{
  Item_move_sub(DOWNLOAD_MOVE_TOP);
  return TRUE;
}

gboolean Item_move_bottom(GtkWidget *w, gpointer data)
{
  Item_move_sub(DOWNLOAD_MOVE_BOTTOM);
  return TRUE;
}

gboolean Item_lock_item(GtkWidget *w, gpointer data)
{
  ListEntry *listentry = g_listManager->ret_Current_listentry();
  listentry->get_Dl_clist_lock();

  try{
    GList *node = GTK_CLIST(listentry->ret_Dl_clist())->selection;
    if(!node) throw 0;
    while(node) {
      int rowindex = GPOINTER_TO_UINT(node->data);
      ItemCell *itemcell = (ItemCell *)gtk_clist_get_row_data(GTK_CLIST(listentry->ret_Dl_clist()), rowindex);
      switch(itemcell->ret_Status()) {
      case ItemCell::ITEM_ERROR:
	g_summaryInfo.dec_error();
      case ItemCell::ITEM_READY:
      case ItemCell::ITEM_READY_AGAIN:
      case ItemCell::ITEM_READY_CONCAT:
      case ItemCell::ITEM_STOP:
	itemcell->set_Status(ItemCell::ITEM_LOCK);
	listentry->Set_clist_column__icon(rowindex, ItemCell::ITEM_LOCK);
	break;
      default:
	break;
      }
      node = g_list_next(node);
    }
    listentry->release_Dl_clist_lock();
  } catch (int err) {
    listentry->release_Dl_clist_lock();
  }
  ItemStatusSum *itemstatus = new ItemStatusSum(g_consoleItem);
  Send_report(MSG_SYS_INFO, itemstatus);

  return TRUE;
}


gboolean Item_unlock_item(GtkWidget *w, gpointer data)
{
  ListEntry *listentry = g_listManager->ret_Current_listentry();
  listentry->get_Dl_clist_lock();

  try{
    GList *node = GTK_CLIST(listentry->ret_Dl_clist())->selection;
    if(!node) throw 0;
    while(node) {
      int rowindex GPOINTER_TO_UINT(node->data);
      ItemCell *itemcell = (ItemCell *)gtk_clist_get_row_data(GTK_CLIST(listentry->ret_Dl_clist()), rowindex);
      switch(itemcell->ret_Status()) {
      case ItemCell::ITEM_LOCK:
	itemcell->set_Status(ItemCell::ITEM_STOP);// modified 2001/5/20
	listentry->Set_clist_column__icon(rowindex, ItemCell::ITEM_STOP);
	break;
      default:
	break;
      }
      node = g_list_next(node);
    }
    listentry->release_Dl_clist_lock();
  } catch (int err) {
    listentry->release_Dl_clist_lock();
  }
  return TRUE;
}

static gboolean Item_lock_error_item(GtkWidget *w, gpointer data)
{
  ListEntry *listentry = g_listManager->ret_Current_listentry();
  listentry->get_Dl_clist_lock();

  try{    
    for(int rowindex = 0; rowindex < GTK_CLIST(listentry->ret_Dl_clist())->rows; ++rowindex) {
      ItemCell *itemcell = (ItemCell *)gtk_clist_get_row_data(GTK_CLIST(listentry->ret_Dl_clist()), rowindex);
      switch(itemcell->ret_Status()) {
      case ItemCell::ITEM_ERROR:
	itemcell->set_Status(ItemCell::ITEM_LOCK);
	listentry->Set_clist_column__icon(rowindex, ItemCell::ITEM_LOCK);
	break;
      default:
	break;
      }
    }
    listentry->release_Dl_clist_lock();
  } catch (int err) {
    listentry->release_Dl_clist_lock();
  }
  return TRUE;
}

void Item_clear_state_item(int state_flag)
{ 
  ListEntry *listentry = g_listManager->ret_Current_listentry();
  listentry->get_Dl_clist_lock();
  listentry->freezeDlCList();
  int rowindex = 0;
  int count = 0;
  while(1) {
    ItemCell* itemcell = (ItemCell*)gtk_clist_get_row_data(GTK_CLIST(listentry->ret_Dl_clist()), rowindex);
    if(itemcell == NULL) break;
    bool del_flag = false;
    switch(itemcell->ret_Status()) {
    case ItemCell::ITEM_ERROR:
      //case ItemCell::ITEM_CRCERROR:
      if(state_flag & DOWNLOAD_DEL_ERROR) {
	del_flag = true;
	Download_clear_sub(itemcell, listentry);
      }
      break;
    case ItemCell::ITEM_READY:
    case ItemCell::ITEM_READY_AGAIN:
    case ItemCell::ITEM_READY_CONCAT:
      if(state_flag & DOWNLOAD_DEL_READY) {
	del_flag = true;
	Download_clear_sub(itemcell, listentry);
      }
      break;
    case ItemCell::ITEM_STOP:
      if(state_flag & DOWNLOAD_DEL_STOP) {
	del_flag = true;
	Download_clear_sub(itemcell, listentry);
      }
      break;
    case ItemCell::ITEM_LOCK:
      if(state_flag & DOWNLOAD_DEL_LOCK) {
	del_flag = true;
	Download_clear_sub(itemcell, listentry);
      }
      break;
    case ItemCell::ITEM_COMPLETE:
      if(state_flag & DOWNLOAD_DEL_COMPLETE) {
	del_flag = true;
	Download_clear_sub(itemcell, listentry);
      }
    default:
      break;
    }
    if(!del_flag) {
      ++rowindex;
    } else {
      ++count;
    }
  }
  if(count > 0) {
    if(GTK_CLIST(listentry->ret_Dl_clist())->selection == NULL) {
      Set_sensitive__no_item_selected(); // fix this
    }
    if(GTK_CLIST(listentry->ret_Dl_clist())->rows == 0) {
      Set_sensitive__list_empty(); // fix this
    }
  }
  listentry->thawDlCList();
  listentry->release_Dl_clist_lock();
  if(count > 0) {
    string line = itos(count)+_(" item(s) deleted");
    g_consoleItem->Send_message_to_gui(line, MSG_SYS_INFO);
  }
}

static gboolean Item_clear_state_item_ok(GtkWidget *w, GtkWidget *window)
{
  g_cDialog->hide();
  Item_clear_state_item(state_flag);

  return TRUE;
}

static void Item_clear_state_item_c()
{
  if(g_appOption->ret_confirm_clear()) {
    g_cDialog->setup(_("Clear items"),
		   _("Are you sure to delete these items?"),
		   Item_clear_state_item_ok);
    g_cDialog->set_cancel_button_visible(false);
    g_cDialog->show();
  } else {
    Item_clear_state_item(state_flag);
  }
}

static gboolean Item_clear_stopped_item(GtkWidget *w, gpointer data)
{
  state_flag = DOWNLOAD_DEL_STOP;
  Item_clear_state_item_c();

  return TRUE;
}

static gboolean Item_clear_error_item(GtkWidget *w, gpointer data)
{
  state_flag = DOWNLOAD_DEL_ERROR;
  Item_clear_state_item_c();

  return TRUE;
}

static gboolean Item_clear_ready_item(GtkWidget *w, gpointer data)
{
  state_flag = DOWNLOAD_DEL_READY;
  Item_clear_state_item_c();

  return TRUE;
}

static gboolean Item_clear_downloaded_item(GtkWidget *w, gpointer data)
{
  state_flag = DOWNLOAD_DEL_COMPLETE;
  Item_clear_state_item_c();

  return TRUE;
}

static gboolean Item_clear_locked_item(GtkWidget *w, gpointer data)
{
  state_flag = DOWNLOAD_DEL_LOCK;
  Item_clear_state_item_c();

  return TRUE;
}

static gboolean Item_clear_error_stopped_ready_item(GtkWidget *w, gpointer data)
{
  state_flag = DOWNLOAD_DEL_ERROR | DOWNLOAD_DEL_READY | DOWNLOAD_DEL_STOP;
  Item_clear_state_item_c();

  return TRUE;
}

// create "Item" menu
void Create_item_menu(GtkWidget *toplevel, GtkWidget *menu_bar, GtkAccelGroup *accel_group)
{
  GtkWidget *menu;
  GtkWidget *root_item;

  GtkWidget *newItem_item; // New item

  //create Item menu
  menu = gtk_menu_new();

  newItem_item = GTK_create_menu_item_with_icon(menu,
						_("New item"),
						GTK_SIGNAL_FUNC(Item_new_item),
						NULL,
						accel_group,
						SC_NEWITEM,
						SCM_NEWITEM);
  GTK_create_menu_separator(menu);

  lock_item = GTK_create_menu_item_with_icon(menu,
					     _("Lock"),
					     GTK_SIGNAL_FUNC(Item_lock_item),
					     NULL,
					     lockon_xpm,
					     toplevel,
					     accel_group,
					     SC_LOCK,
					     SCM_LOCK);

  unlock_item = GTK_create_menu_item_with_icon(menu,
					       _("Unlock"),
					       GTK_SIGNAL_FUNC(Item_unlock_item),
					       NULL,
					       unlock_xpm,
					       toplevel,
					       accel_group,
					       SC_UNLOCK,
					       SCM_UNLOCK);

  GTK_create_menu_separator(menu);

  moveUp_item = GTK_create_menu_item_with_icon(menu,
					       _("Move item up"),
					       GTK_SIGNAL_FUNC(Item_move_up),
					       NULL,
					       moveup_xpm,
					       toplevel,
					       accel_group,
					       SC_MOVEUP,
					       SCM_MOVEUP);

  moveDown_item = GTK_create_menu_item_with_icon(menu,
						 _("Move item down"),
						 GTK_SIGNAL_FUNC(Item_move_down),
						 NULL,
						 movedown_xpm,
						 toplevel,
						 accel_group,
						 SC_MOVEDOWN,
						 SCM_MOVEDOWN);
  
  moveTop_item = GTK_create_menu_item_with_icon(menu,
						_("Move item to top"),
						GTK_SIGNAL_FUNC(Item_move_top),
						NULL,
						accel_group,
						SC_MOVETOP,
						SCM_MOVETOP);

  moveBottom_item = GTK_create_menu_item_with_icon(menu,
						   _("Move item to bottom"),
						   GTK_SIGNAL_FUNC(Item_move_bottom),
						   NULL,
						   accel_group,
						   SC_MOVEBOTTOM,
						   SCM_MOVEBOTTOM);
 
  GTK_create_menu_separator(menu);

  lockError_item = GTK_create_menu_item_with_icon(menu,
						  _("Lock error items"),
						  GTK_SIGNAL_FUNC(Item_lock_error_item),
						  NULL,
						  accel_group,
						  SC_LOCKERROR,
						  SCM_LOCKERROR);

  GTK_create_menu_separator(menu);

  clearDownloaded_item = GTK_create_menu_item_with_icon(menu,
							_("Clear downloaded item"),
							GTK_SIGNAL_FUNC(Item_clear_downloaded_item),
							NULL,
							accel_group,
							SC_CLEARDOWNLOADED,
							SCM_CLEARDOWNLOADED);

  clearLock_item = GTK_create_menu_item_with_icon(menu,
						  _("Clear locked items"),
						  GTK_SIGNAL_FUNC(Item_clear_locked_item),
						  NULL);

  clearError_item = GTK_create_menu_item_with_icon(menu,
						   _("Clear error items"),
						   GTK_SIGNAL_FUNC(Item_clear_error_item),
						   NULL);

  clearStop_item = GTK_create_menu_item_with_icon(menu,
						  _("Clear stopped items"),
						  GTK_SIGNAL_FUNC(Item_clear_stopped_item),
						  NULL);

  clearReady_item = GTK_create_menu_item_with_icon(menu,
						   _("Clear ready items"),
						   GTK_SIGNAL_FUNC(Item_clear_ready_item),
						   NULL);

  clearErrorStopReady_item = GTK_create_menu_item_with_icon(menu,
							    _("Clear error, stopped and ready items"),
							    GTK_SIGNAL_FUNC(Item_clear_error_stopped_ready_item),
							    NULL);

  // create new item window
  Create_new_item_window(toplevel);

  root_item = gtk_menu_item_new_with_label(_("Item"));
  gtk_widget_show(root_item);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(root_item), menu);
  gtk_menu_bar_append(GTK_MENU_BAR(menu_bar), root_item);
}
