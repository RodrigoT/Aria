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

// $Id: PasteWindow.cc,v 1.30 2002/10/01 15:32:00 tujikawa Exp $

#include "PasteWindow.h"
#include "ItemCell.h"
#include "ItemOption.h"
#include "Dialog.h"

extern void Set_sensitive__list_not_empty();
extern void Add_new_item_to_downloadlist(ItemCell *itemcell, ListEntry *listentry);

extern ListManager *g_listManager;
extern ItemManager *g_itemManagerPaste;
extern ItemOption *g_itemOption;
extern AppOption *g_appOption;
extern ItemCell *g_consoleItem;
extern Dialog *g_cDialog;

static GtkWidget *selectAllItem;
static GtkWidget *invertSelectionItem;

static GtkWidget *item_individual_item;
static GtkWidget *reset_to_default_item;
static GtkWidget *reset_to_default_nosd_item;
//static GtkWidget *reset_to_default_norec_item;
static GtkWidget *item_default_item;
static GtkWidget *reset_to_default_all_item;
static GtkWidget *reset_to_default_all_nosd_item;
//static GtkWidget *reset_to_default_all_norec_item;

static void Paste_set_sensitive__no_item_selected()
{
  gtk_widget_set_sensitive(item_individual_item, FALSE);
  gtk_widget_set_sensitive(reset_to_default_item, FALSE);
  gtk_widget_set_sensitive(reset_to_default_nosd_item, FALSE);
}

static void Paste_set_sensitive__items_selected()
{
  gtk_widget_set_sensitive(item_individual_item, TRUE);
  gtk_widget_set_sensitive(reset_to_default_item, TRUE);
  gtk_widget_set_sensitive(reset_to_default_nosd_item, TRUE);
}

static void Paste_set_sensitive__list_empty()
{
  gtk_widget_set_sensitive(reset_to_default_all_item, FALSE);
  gtk_widget_set_sensitive(reset_to_default_all_nosd_item, FALSE);
}

static void Paste_set_sensitive__list_not_empty()
{
  gtk_widget_set_sensitive(reset_to_default_all_item, TRUE);
  gtk_widget_set_sensitive(reset_to_default_all_nosd_item, TRUE);  
}

static void Set_clist_column__save(GtkWidget *clist, int rowindex, const string& save_string)
{
  gtk_clist_set_text(GTK_CLIST(clist), rowindex, 1, save_string.c_str());
}

// paste_windowのOKボタンをおすと呼び出される
static gboolean Paste_OK(GtkWidget *w, PasteWindow *pw)
{
  pw->paste();

  return TRUE;
}

void PasteWindow::paste() {
  string line;

  ListEntry *listentry = g_listManager->getListEntryByName(getTargetListName());
  if(listentry == NULL) {
    g_cDialog->setup(_("Error"),
		     _("The specified list no longer exists."));
    g_cDialog->set_cancel_button_visible(false);
    g_cDialog->set_transient(GTK_WINDOW(window));
    g_cDialog->show();
    updateTargetList();
    return;
  }

  hide();
  listentry->get_Dl_clist_lock();

  if(GTK_CLIST(paste_list)->rows == 0) {
    listentry->release_Dl_clist_lock();
    return;
  }

  int maxrow = GTK_CLIST(listentry->ret_Dl_clist())->rows;

  int count = 0;// added 2001/3/21
  // add to main download list
  ItemCell *itemcell = NULL;
  for(unsigned int i = 0;(itemcell = (ItemCell *)gtk_clist_get_row_data(GTK_CLIST(paste_list), i)) != NULL; ++i) {
    // modified
    //if(!listentry->getItemManager()->search_by_url(itemcell->ret_URL())) {
    if(!listentry->getItemManager()->search_by_url_with_local_path(itemcell->ret_URL_Container_opt().ret_URL(), itemcell->ret_Options_opt().ret_Store_Dir()+itemcell->ret_Filename_opt())) {
      itemcell->set_Options(itemcell->ret_Options_opt());
      itemcell->set_URL_Container(itemcell->ret_URL_Container_opt());
      itemcell->set_URL(itemcell->ret_URL_opt());
      itemcell->set_Filename(itemcell->ret_Filename_opt());// added 2001/5/6
      // added 2001/5/20
      if(isAutostartEnabled()) {
	itemcell->set_Status(ItemCell::ITEM_READY);
      }
      Add_new_item_to_downloadlist(itemcell, listentry);
      ++count;// added 2001/3/21
    } else {
      delete itemcell;
    }
  }

  if(isAutostartEnabled()) {
    listentry->Send_start_signal();
  }

  gtk_clist_moveto(GTK_CLIST(listentry->ret_Dl_clist()),
		   maxrow,
		   0,
		   0.0, 0.0);

  listentry->release_Dl_clist_lock();

  if(count) {// added 2001/3/21
    line = itos(count)+_(" item(s) pasted");
    g_consoleItem->Send_message_to_gui(line, MSG_SYS_INFO);
    Set_sensitive__list_not_empty();
  }

  gtk_clist_clear(GTK_CLIST(paste_list));

  // clear paste item buffer
  g_itemManagerPaste->all_clear();

  return;
}

static gboolean Option_item_individual(GtkWidget *w, PasteWindow *pw)
{
  GtkWidget *paste_list = pw->getPasteList();
  GList* node = GTK_CLIST(paste_list)->selection;
  if(node == NULL) return(TRUE);
  ItemCell* itemcell = (ItemCell*)gtk_clist_get_row_data(GTK_CLIST(paste_list), GPOINTER_TO_UINT(node->data));

  g_itemOption->setOptionValues(itemcell, itemcell->ret_Options_opt(),
				g_listManager->getListEntryByName(pw->getTargetListName()));
  g_itemOption->show();

  return TRUE;
}

static gboolean Option_reset_to_default(GtkWidget *w, PasteWindow *pw)
{
  //ListEntry *listentry = g_listManager->ret_Current_listentry();
   ListEntry *listentry = g_listManager->getListEntryByName(pw->getTargetListName());
  if(listentry == NULL) {
    g_cDialog->setup(_("Error"),
		     _("The specified list no longer exists."));
    g_cDialog->set_cancel_button_visible(false);
    g_cDialog->set_transient(GTK_WINDOW(pw->getWindow()));
    g_cDialog->show();
    pw->updateTargetList();
    return TRUE;
  }
  GtkWidget *paste_list = pw->getPasteList();
  GList *node = GTK_CLIST(paste_list)->selection;
  
  const Options& console_option = listentry->ret_Options();
  while(node) {
    unsigned int rowindex = GPOINTER_TO_UINT(node->data);
    ItemCell* itemcell = (ItemCell*)gtk_clist_get_row_data(GTK_CLIST(paste_list), rowindex);
    // acquire lock;
    itemcell->get_Options_Lock();
    itemcell->ret_Options_opt() = console_option;

    Set_clist_column__save(paste_list, rowindex, itemcell->ret_Options_opt().ret_Store_Dir());
    //itemcell->Raise_option_update_flag();
    // release lock
    itemcell->release_Options_Lock();
    node = g_list_next(node);
  }
  return TRUE;
}

static gboolean Option_reset_to_default_nosd(GtkWidget *w, PasteWindow *pw)
{
   ListEntry *listentry = g_listManager->getListEntryByName(pw->getTargetListName());
  if(listentry == NULL) {
    g_cDialog->setup(_("Error"),
		     _("The specified list no longer exists."));
    g_cDialog->set_cancel_button_visible(false);
    g_cDialog->set_transient(GTK_WINDOW(pw->getWindow()));
    g_cDialog->show();
    pw->updateTargetList();
    return TRUE;
  }
  GtkWidget *paste_list = pw->getPasteList();
  GList *node = GTK_CLIST(paste_list)->selection;
  //ListEntry *listentry = g_listManager->ret_Current_listentry();
  const Options& console_option = listentry->ret_Options();
  while(node) {
    unsigned int rowindex = GPOINTER_TO_UINT(node->data);
    ItemCell* itemcell = (ItemCell*)gtk_clist_get_row_data(GTK_CLIST(paste_list), rowindex);
    // acquire lock
    itemcell->get_Options_Lock();
    string storedir_temp = itemcell->ret_Options_opt().ret_Store_Dir();
    itemcell->ret_Options_opt() = console_option;
    itemcell->ret_Options_opt().set_Store_Dir(storedir_temp);

    //Set_clist_column__save(paste_list, rowindex, itemcell->ret_Options_opt().ret_Store_Dir());
    //itemcell->Raise_option_update_flag();
    // release
    itemcell->release_Options_Lock();
    node = g_list_next(node);
  }
  return TRUE;
}

static gboolean Option_item_default(GtkWidget *w, PasteWindow *pw)
{
   ListEntry *listentry = g_listManager->getListEntryByName(pw->getTargetListName());
  if(listentry == NULL) {
    g_cDialog->setup(_("Error"),
		     _("The specified list no longer exists."));
    g_cDialog->set_cancel_button_visible(false);
    g_cDialog->set_transient(GTK_WINDOW(pw->getWindow()));
    g_cDialog->show();
    pw->updateTargetList();
    return TRUE;
  }
  //ListEntry *listentry = g_listManager->ret_Current_listentry();
  g_itemOption->setOptionValues(listentry->ret_Default_item(),
				listentry->ret_Default_item()->ret_Options_opt(),
				g_listManager->getListEntryByName(pw->getTargetListName()));
  g_itemOption->show();

  return TRUE;
}

static gboolean Option_reset_to_default_all(GtkWidget *w, PasteWindow *pw)
{
   ListEntry *listentry = g_listManager->getListEntryByName(pw->getTargetListName());
  if(listentry == NULL) {
    g_cDialog->setup(_("Error"),
		     _("The specified list no longer exists."));
    g_cDialog->set_cancel_button_visible(false);
    g_cDialog->set_transient(GTK_WINDOW(pw->getWindow()));
    g_cDialog->show();
    pw->updateTargetList();
    return TRUE;
  }
  GtkWidget *paste_list = pw->getPasteList();
  //ListEntry *listentry = g_listManager->ret_Current_listentry();

  const Options& console_option = listentry->ret_Options();
  for(int rowindex = 0; rowindex < GTK_CLIST(paste_list)->rows; ++rowindex) {
    ItemCell *itemcell = (ItemCell*)gtk_clist_get_row_data(GTK_CLIST(paste_list), rowindex);
    // acquire lock
    itemcell->get_Options_Lock();
    itemcell->ret_Options_opt() = console_option;

    Set_clist_column__save(paste_list, rowindex, itemcell->ret_Options_opt().ret_Store_Dir());
    //itemcell->Raise_option_update_flag();
    // release lock
    itemcell->release_Options_Lock();
  }
  return TRUE;
}

static gboolean Option_reset_to_default_all_nosd(GtkWidget *w, PasteWindow *pw)
{
   ListEntry *listentry = g_listManager->getListEntryByName(pw->getTargetListName());
  if(listentry == NULL) {
    g_cDialog->setup(_("Error"),
		     _("The specified list no longer exists."));
    g_cDialog->set_cancel_button_visible(false);
    g_cDialog->set_transient(GTK_WINDOW(pw->getWindow()));
    g_cDialog->show();
    pw->updateTargetList();
    return TRUE;
  }
  GtkWidget *paste_list = pw->getPasteList();
  //ListEntry *listentry = g_listManager->ret_Current_listentry();
  const Options& console_option = listentry->ret_Options();
  for(int rowindex = 0; rowindex < GTK_CLIST(paste_list)->rows; ++rowindex) {
    ItemCell *itemcell = (ItemCell*)gtk_clist_get_row_data(GTK_CLIST(paste_list), rowindex);
    // acquire lock
    itemcell->get_Options_Lock();
    string storedir_temp = itemcell->ret_Options_opt().ret_Store_Dir();
    itemcell->ret_Options_opt() = console_option;
    itemcell->ret_Options_opt().set_Store_Dir(storedir_temp);

    Set_clist_column__save(paste_list, rowindex, itemcell->ret_Options_opt().ret_Store_Dir());
    //itemcell->Raise_option_update_flag();
    // release lock
    itemcell->release_Options_Lock();
  }
  return TRUE;
}

// paste window上のdeleteボタンが押されたときの処理。
// CLIST additem_list上で選択されているアイテムがあればそれを削除する。
static gboolean Paste_delete(GtkWidget *w, PasteWindow *pw)
{
  pw->deleteItem();

  return TRUE;
}

// paste_windowを隠す
static gboolean Paste_cancel(GtkObject *obj) //GtkWidget *w, PasteWindow *pw)
{
  PasteWindow *pw = (PasteWindow *)obj;

  pw->hide();
  pw->deleteAllItem();

  return TRUE;
}

static void PasteWindow_selectRow_cb(GtkWidget *clist,
				     int row,
				     int column,
				     GdkEventButton *event,
				     PasteWindow *pw) {
  static bool enterflag = false;
  if(enterflag == true || event == NULL) return;
  enterflag = true;
  
  pw->selectRow(row, column, event);

  enterflag = false;

  return;
}

void PasteWindow::selectRow(int row, int column, GdkEventButton *event) {
  if(event->type == GDK_2BUTTON_PRESS) {
    ItemCell* itemcell = (ItemCell*)gtk_clist_get_row_data(GTK_CLIST(paste_list), row);
    g_itemOption->setOptionValues(itemcell, itemcell->ret_Options_opt(),
				  g_listManager->getListEntryByName(getTargetListName()));
    g_itemOption->show();
  } else {
    switch(event->button) {
    case 1:
    case 3:
      if(event->state & GDK_CONTROL_MASK) {
      // do nothing, just select the row
      } else if(event->state & GDK_SHIFT_MASK) {
	int nearestSelectedRow = findNearestSelectedRow(paste_list, row);
	if(nearestSelectedRow < row) {
	  for(int index = nearestSelectedRow+1; index < row; ++index) {
	    gtk_clist_select_row(GTK_CLIST(paste_list), index, 0);
	  }
	} else {
	  for(int index = row+1; index < nearestSelectedRow; ++index) {
	    gtk_clist_select_row(GTK_CLIST(paste_list), index, 0);
	  }
	}
      } else {// with no mask
	gtk_clist_unselect_all(GTK_CLIST(paste_list));
	gtk_clist_select_row(GTK_CLIST(paste_list), row, 0);
      }
      Paste_set_sensitive__items_selected();
    break;
    }
  }

  return;
}

static void PasteWindow_unselectRow_cb(GtkWidget *clist,
				       int row,
				       int column,
				       GdkEventButton *event,
				       PasteWindow *pw) {
  static bool enterflag = false;
  if(enterflag == true || event == NULL) return;
  enterflag = true;
  
  pw->unselectRow(row, column, event);

  enterflag = false;

  return;
}

void PasteWindow::unselectRow(int row, int column, GdkEventButton *event) {
  switch(event->button) {
  case 1:
  case 3:
    if(event->state & (GDK_CONTROL_MASK | GDK_SHIFT_MASK)) {
      GList *node = GTK_CLIST(paste_list)->selection;
      if(node == NULL) {
	Paste_set_sensitive__no_item_selected();
      } else {
	gtk_clist_unselect_row(GTK_CLIST(paste_list), row, 0);
	GList *node = GTK_CLIST(paste_list)->selection;
	if(node == NULL) {
	  Paste_set_sensitive__no_item_selected();
	}
      }
    } else {
      GList *node = GTK_CLIST(paste_list)->selection;
      if(node != NULL) {
	gtk_clist_unselect_all(GTK_CLIST(paste_list));
	gtk_clist_select_row(GTK_CLIST(paste_list), row, 0);
	Paste_set_sensitive__items_selected();
      } else {
	Paste_set_sensitive__no_item_selected();
      }
    }
    break;
  }

  return;
}

static void add_url_on_dnd(GtkWidget  *paste_list,
			   GdkDragContext *context,
			   int x,
			   int y,
			   GtkSelectionData *data,
			   unsigned int info,
			   unsigned int time)
{
  string url_string = (char *)data->data;
  bool ws_quark;
  if(info == MIME_TEXT_PLAIN) {
    ws_quark = true;
  } else {
    ws_quark = false;
  }

  while(url_string.size()) {
    string url = URLcontainer::Find_URL(url_string, ws_quark);
    URLcontainer urlcon;
    if(urlcon.Parse_URL(url) &&
       //!item_manager->search_by_url(urlcon.ret_URL()) &&
       !g_itemManagerPaste->search_by_url(urlcon.ret_URL())) {
      ListEntry *listentry = g_listManager->ret_Current_listentry();

      const Options& options = listentry->ret_Options();

      ItemCell *itemcell = new ItemCell(url, urlcon, options, _("Created"));
      /*
      if(urlcon.ret_Protocol() == "http:") {
	itemcell = new ItemCell_HTTP(url, urlcon, options, _("Created"));
      } else if(urlcon.ret_Protocol() == "ftp:") {
	itemcell = new ItemCell_FTP(url, urlcon, options, _("Created"));
      } else {
	// unsupported protocols
	continue;
      }
      */
      char *clist_item[2];
      clist_item[0] = strdup(urlcon.ret_URL().c_str());
      clist_item[1] = strdup(itemcell->ret_Options_opt().ret_Store_Dir().c_str());
      int rowindex = gtk_clist_append(GTK_CLIST(paste_list), clist_item);
      //GdkColor color = {0, 0, 0};
      //gtk_clist_set_foreground(GTK_CLIST(paste_list), rowindex, &color);

      delete [] clist_item[0];
      delete [] clist_item[1];
      gtk_clist_set_row_data(GTK_CLIST(paste_list), rowindex, itemcell);
      g_itemManagerPaste->regist_item_back(itemcell);
    }
  }
}

// select all item in download list
static void Edit_select_all(GtkWidget *w, PasteWindow *pw)
{
  pw->selectAll();

  return;
}

void PasteWindow::selectAll() {
  gtk_clist_select_all(GTK_CLIST(paste_list));

  GList *node = GTK_CLIST(paste_list)->selection;
  if(node != NULL) {
    Paste_set_sensitive__items_selected(); // fix this
  }
}

// invert selection
static void Edit_invert_selection(GtkWidget *w, PasteWindow *pw)
{
  pw->invertSelection();
  return;
}

void PasteWindow::invertSelection() {
  pthread_mutex_lock(&listLock);
  gtk_clist_freeze(GTK_CLIST(paste_list));
  GList *selection = g_list_copy(GTK_CLIST(paste_list)->selection);

  // select all, then unselect row that is in node list
  gtk_clist_select_all(GTK_CLIST(paste_list));
  GList *node = selection;
  while(node) {
    int rowindex = GPOINTER_TO_INT(node->data);
    gtk_clist_unselect_row(GTK_CLIST(paste_list), rowindex, 0);
    node = g_list_next(node);
  }

  if(GTK_CLIST(paste_list)->selection != NULL) {
    Paste_set_sensitive__items_selected();
  } else {
    Paste_set_sensitive__no_item_selected();
  }

  g_list_free(selection);
  gtk_clist_thaw(GTK_CLIST(paste_list));
  pthread_mutex_unlock(&listLock);
}

void PasteWindow::Create_edit_menu(GtkWidget *window, GtkWidget *menu_bar, GtkAccelGroup *accel_group) {
  GtkWidget *menu;
  GtkWidget *root_item;

  menu = gtk_menu_new();

  // Select all items
  selectAllItem = GTK_create_menu_item_with_icon(menu,
						 _("Select all"),
						 GTK_SIGNAL_FUNC(Edit_select_all),
						 (GtkObject *)this,
						 accel_group,
						 SC_SELECT_ALL,
						 SCM_SELECT_ALL);
  
  // Invert selection
  invertSelectionItem = GTK_create_menu_item_with_icon(menu,
						       _("Invert selection"),
						       GTK_SIGNAL_FUNC(Edit_invert_selection),
						       (GtkObject *)this
						       );
  
  // Create root menu item
  root_item = gtk_menu_item_new_with_label(_("Edit"));
  gtk_widget_show(root_item);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(root_item), menu);
  gtk_menu_bar_append(GTK_MENU_BAR(menu_bar), root_item);
}

void PasteWindow::Create_option_menu(GtkWidget *window, GtkWidget *menu_bar, GtkAccelGroup *accel_group) {
  GtkWidget* menu;
  GtkWidget* root_item;

  //create option menu
  menu = gtk_menu_new();


  item_individual_item = GTK_create_menu_item_with_icon(menu,
							_("Selected item option"),
							GTK_SIGNAL_FUNC(Option_item_individual),
							(void *)this,
							accel_group,
							SC_ITEMOPTION,
							SCM_ITEMOPTION);
  
  item_default_item = GTK_create_menu_item_with_icon(menu,
						     _("Default item option for current list"),
						     GTK_SIGNAL_FUNC(Option_item_default),
						     (void *)this,
						     accel_group,
						     SC_ITEMOPTION_DEFAULT_LIST,
						     SCM_ITEMOPTION_DEFAULT_LIST);
  
  GtkWidget *apply_item = GTK_create_menu_item_with_icon(menu,
							 _("Apply default item option"),
							 NULL,
							 NULL);

  GtkWidget *apply_menu = gtk_menu_new();
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(apply_item), apply_menu);
  gtk_widget_show(apply_item);
  GtkWidget *selected_item = GTK_create_menu_item_with_icon(apply_menu,
							    _("To selected items"),
							    NULL,
							    NULL);

  GtkWidget *all_item = GTK_create_menu_item_with_icon(apply_menu,
						       _("To all items in current list"),
						       NULL,
						       NULL);

  GtkWidget *selected_menu = gtk_menu_new();
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(selected_item), selected_menu);

  reset_to_default_item = GTK_create_menu_item_with_icon(selected_menu,
							 _("All settings"),
							 GTK_SIGNAL_FUNC(Option_reset_to_default),
							 (void *)this,
							 accel_group,
							 SC_SETDEFAULT,
							 SCM_SETDEFAULT);
  
  reset_to_default_nosd_item = GTK_create_menu_item_with_icon(selected_menu,
						    _("Ignore save directory"),
						    GTK_SIGNAL_FUNC(Option_reset_to_default_nosd),
						    (void *)this,
						    accel_group,
						    SC_SETDEFAULT_NOSAVEDIR,
						    SCM_SETDEFAULT_NOSAVEDIR);
  /*
  reset_to_default_norec_item = GTK_create_menu_item_with_icon(selected_menu,
						     _("Ignore save directory and recursive settings"),
						     GTK_SIGNAL_FUNC(Option_reset_to_default_norec),
						     NULL);
  */

  GtkWidget *all_menu = gtk_menu_new();
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(all_item), all_menu);

  reset_to_default_all_item = GTK_create_menu_item_with_icon(all_menu,
						   _("All settings"),
						   GTK_SIGNAL_FUNC(Option_reset_to_default_all),
						   (void *)this,
						   accel_group,
						   SC_SETDEFAULT_ALL,
						   SCM_SETDEFAULT_ALL);

  reset_to_default_all_nosd_item = GTK_create_menu_item_with_icon(all_menu,
							_("Ignore save directory"),
							GTK_SIGNAL_FUNC(Option_reset_to_default_all_nosd),
							(void *)this,
							accel_group,
							SC_SETDEFAULT_ALL_NOSAVEDIR,
							SCM_SETDEFAULT_ALL_NOSAVEDIR);
  
  /*
    item_individual_item = gtk_menu_item_new_with_label(_("Selected item option"));
  reset_to_default_item = gtk_menu_item_new_with_label(_("Apply default item option to selected items"));
  reset_to_default_nosd_item = gtk_menu_item_new_with_label(_("Apply default item option to selected items(except for save dir)"));
  separator1_item = gtk_menu_item_new();
  item_default_item = gtk_menu_item_new_with_label(_("Default item option for current tab"));

  reset_to_default_all_item = gtk_menu_item_new_with_label(_("Apply default item option to all items"));
  reset_to_default_all_nosd_item = gtk_menu_item_new_with_label(_("Apply default item option to all items(except for save dir)"));
  
  gtk_menu_append(GTK_MENU(option_menu), item_individual_item);
  gtk_menu_append(GTK_MENU(option_menu), reset_to_default_item);
  gtk_menu_append(GTK_MENU(option_menu), reset_to_default_nosd_item);
  gtk_menu_append(GTK_MENU(option_menu), separator1_item);
  gtk_menu_append(GTK_MENU(option_menu), item_default_item);
  gtk_menu_append(GTK_MENU(option_menu), reset_to_default_all_item);
  gtk_menu_append(GTK_MENU(option_menu), reset_to_default_all_nosd_item);

  // signal handling
  gtk_signal_connect(GTK_OBJECT(item_individual_item), "activate",
		     GTK_SIGNAL_FUNC(Option_item_individual), GTK_OBJECT(paste_list));
  gtk_signal_connect(GTK_OBJECT(reset_to_default_item), "activate",
		     GTK_SIGNAL_FUNC(Option_reset_to_default), GTK_OBJECT(paste_list));
  gtk_signal_connect(GTK_OBJECT(reset_to_default_nosd_item), "activate",
		     GTK_SIGNAL_FUNC(Option_reset_to_default_nosd), GTK_OBJECT(paste_list));
  gtk_signal_connect(GTK_OBJECT(item_default_item), "activate",
		     GTK_SIGNAL_FUNC(Option_item_default), NULL);
  gtk_signal_connect(GTK_OBJECT(reset_to_default_all_item), "activate",
		     GTK_SIGNAL_FUNC(Option_reset_to_default_all), GTK_OBJECT(paste_list));
  gtk_signal_connect(GTK_OBJECT(reset_to_default_all_nosd_item), "activate",
		     GTK_SIGNAL_FUNC(Option_reset_to_default_all_nosd), GTK_OBJECT(paste_list));

  // accelerator key
  gtk_widget_add_accelerator(item_individual_item,
			     "activate",
			     accel_group,
			     'O',
			     GDK_MOD1_MASK,
			     GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator(item_default_item,
			     "activate",
			     accel_group,
			     'Z',
			     GDK_MOD1_MASK,
			     GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator(reset_to_default_item,
			     "activate",
			     accel_group,
			     'J',
			     GDK_MOD1_MASK,
			     GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator(reset_to_default_nosd_item,
			     "activate",
			     accel_group,
			     'K',
			     GDK_MOD1_MASK,
			     GTK_ACCEL_VISIBLE);

  gtk_widget_add_accelerator(reset_to_default_all_item,
			     "activate",
			     accel_group,
			     'G',
			     GDK_MOD1_MASK,
			     GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator(reset_to_default_all_nosd_item,
			     "activate",
			     accel_group,
			     'H',
			     GDK_MOD1_MASK,
			     GTK_ACCEL_VISIBLE);

  gtk_widget_show(item_individual_item);
  gtk_widget_show(reset_to_default_item);
  gtk_widget_show(reset_to_default_nosd_item);
  gtk_widget_show(separator1_item);
  gtk_widget_show(item_default_item);
  gtk_widget_show(reset_to_default_all_item);
  gtk_widget_show(reset_to_default_all_nosd_item);
  */
  Paste_set_sensitive__no_item_selected();
  Paste_set_sensitive__list_empty();

  root_item = gtk_menu_item_new_with_label(_("Option"));
  gtk_widget_show(root_item);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(root_item), menu);
  gtk_menu_bar_append(GTK_MENU_BAR(menu_bar), root_item);
}

GtkWidget *PasteWindow::Create_menu_bar(GtkWidget *window)
{
  GtkAccelGroup *accel_group;

  GtkWidget *menu_bar = gtk_menu_bar_new();
  gtk_menu_bar_set_shadow_type(GTK_MENU_BAR(menu_bar), GTK_SHADOW_ETCHED_OUT);
  gtk_widget_show(menu_bar);

  accel_group = gtk_accel_group_new();
  gtk_accel_group_attach(accel_group, GTK_OBJECT(window));

  //Create_file_menu(window, menu_bar, accel_group);
  Create_edit_menu(window, menu_bar, accel_group);
  //Create_download_menu(menu_bar, accel_group);
  Create_option_menu(window, menu_bar, accel_group);
  //Create_help_menu(window, menu_bar, accel_group);
  return menu_bar;
}

PasteWindow::PasteWindow(GtkWindow *toplevel)
{
  char *title[2];

  pthread_mutex_init(&listLock, NULL);

  //URL貼付け用のダイアログの作成
  window = gtk_dialog_new();
  gtk_window_set_title(GTK_WINDOW(window), _("Paste URL list"));
  gtk_widget_set_usize(GTK_WIDGET(window), 680, 300);
  gtk_signal_connect_object(GTK_OBJECT(window),
			    "delete_event",
			    GTK_SIGNAL_FUNC(Paste_cancel),
			    (GtkObject *)this);

  // create hbox which holds a combobox and a toggle button
  GtkWidget *toolHBox = gtk_hbox_new(FALSE, 5);
  gtk_widget_show(toolHBox);
  
  // create a combobox to allow users to choose the list to drop items
  GtkWidget *targetListLabel = gtk_label_new(_("Paste to "));
  gtk_box_pack_start(GTK_BOX(toolHBox), targetListLabel, FALSE, FALSE, 5);
  gtk_widget_show(targetListLabel);

  targetListCombo = gtk_combo_new();
  gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(targetListCombo)->entry), FALSE);
  gtk_box_pack_start(GTK_BOX(toolHBox), targetListCombo, FALSE, FALSE, 0);
  gtk_widget_show(targetListCombo);
  
  // create a toggle button to toggle whether or not downloads should be
  // started automatically.
  autostartToggle = gtk_check_button_new_with_label(_("Start downloading when pasted"));
  gtk_widget_show(autostartToggle);
  gtk_box_pack_start(GTK_BOX(toolHBox), autostartToggle, FALSE, FALSE, 0);
    

  // create CList
  title[0] = _("URL");
  title[1] = _("Save");
  paste_list = gtk_clist_new_with_titles(2, title);
  Setup_dnd(paste_list, add_url_on_dnd);

  // create menu bar
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(window)->vbox), Create_menu_bar(window), FALSE, TRUE, 0);

  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(window)->vbox),
		     toolHBox, FALSE, FALSE, 0);    

  // CList ウィジェットをパックするスクロールドウィンドウを作成する
  GtkWidget *scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
				 GTK_POLICY_ALWAYS, GTK_POLICY_ALWAYS);
  
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(window)->vbox),
		     scrolled_window, TRUE, TRUE, 0);
  gtk_widget_show(scrolled_window);  
  
  // CList を作成する。この例では 2 列を使う
  gtk_widget_set_name(paste_list, "clist");

  gtk_object_set_user_data(GTK_OBJECT(window), paste_list);
//    gtk_clist_set_column_auto_resize(GTK_CLIST(paste_list),
//  				   0,
//  				   TRUE);
  gtk_clist_set_selection_mode(GTK_CLIST(paste_list), GTK_SELECTION_MULTIPLE);
  
  gtk_signal_connect(GTK_OBJECT(paste_list), "select-row",
		     GTK_SIGNAL_FUNC(PasteWindow_selectRow_cb),
		     (void *)this);
  gtk_signal_connect(GTK_OBJECT(paste_list), "unselect-row",
		     GTK_SIGNAL_FUNC(PasteWindow_unselectRow_cb),
		     (void *)this);
  // 境界に影を付ける必要などないが、そうすれば見栄えが良くなる :)
  gtk_clist_set_shadow_type(GTK_CLIST(paste_list), GTK_SHADOW_ETCHED_OUT);

  // set column width
  gtk_clist_set_column_width(GTK_CLIST(paste_list), 0, 360);
  gtk_clist_set_column_width(GTK_CLIST(paste_list), 1, 360);

  // CList ウィジェットを垂直ボックスに加え、それを表示する
  gtk_container_add(GTK_CONTAINER(scrolled_window), paste_list);
  gtk_widget_show(paste_list);
  gtk_clist_column_titles_passive(GTK_CLIST(paste_list));

  //// action area
  GtkWidget *vbox = gtk_vbox_new(FALSE, 5);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(window)->action_area),
		     vbox, FALSE, FALSE, 0);
  gtk_widget_show(vbox);

  {
    GtkWidget *bbox = gtk_hbutton_box_new();
    gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_SPREAD);
    gtk_button_box_set_spacing(GTK_BUTTON_BOX(bbox), 5);
    gtk_box_pack_start(GTK_BOX(vbox),
		       bbox, FALSE, FALSE, 0);
    gtk_widget_show(bbox);
    
    // Paste button
    GtkWidget *Paste_button = gtk_button_new_with_label(_("Paste"));
    GTK_WIDGET_SET_FLAGS(Paste_button, GTK_CAN_DEFAULT);
    gtk_window_set_default(GTK_WINDOW(window), Paste_button);
    gtk_signal_connect(GTK_OBJECT(Paste_button),
		       "clicked",
		       GTK_SIGNAL_FUNC(Paste_OK),
		       (void *)this);
    gtk_box_pack_start(GTK_BOX(bbox),
		       Paste_button, TRUE, TRUE, 0);
    gtk_widget_show(Paste_button);
    gtk_object_set_user_data(GTK_OBJECT(Paste_button), window);

    // item Option button
    GtkWidget *Option_button = gtk_button_new_with_label(_("Option"));
    GTK_WIDGET_SET_FLAGS(Option_button, GTK_CAN_DEFAULT);
    gtk_signal_connect(GTK_OBJECT(Option_button),
		       "clicked",
		       GTK_SIGNAL_FUNC(Option_item_individual),
		       (void *)this);
    gtk_box_pack_start(GTK_BOX(bbox),
		       Option_button, TRUE, TRUE, 0);
    gtk_widget_show(Option_button);
    
    // Delete button
    GtkWidget *Delete_button = gtk_button_new_with_label(_("Delete"));
    GTK_WIDGET_SET_FLAGS(Delete_button, GTK_CAN_DEFAULT);
    gtk_signal_connect(GTK_OBJECT(Delete_button),
		       "clicked",
		       GTK_SIGNAL_FUNC(Paste_delete),
		       (void *)this);
    gtk_box_pack_start(GTK_BOX(bbox),
		       Delete_button, TRUE, TRUE, 0);
    gtk_widget_show(Delete_button);

    // Cancel button
    GtkWidget *Cancel_button = gtk_button_new_with_label(_("Cancel"));
    GTK_WIDGET_SET_FLAGS(Cancel_button, GTK_CAN_DEFAULT);
    gtk_signal_connect_object(GTK_OBJECT(Cancel_button),
			      "clicked",
			      GTK_SIGNAL_FUNC(Paste_cancel),
			      (GtkObject *)this);
    gtk_box_pack_start(GTK_BOX(bbox),
		       Cancel_button, TRUE, TRUE, 0);
    gtk_widget_show(Cancel_button);
  }

  // setup dnd

  //Setup_dnd(paste_list, add_url_on_dnd);
  /*
  static GtkTargetEntry target_table[] = {
    { "x-url/http", 0, 0 },
    { "x-url/ftp", 0, 0 },
    { "_NETSCAPE_URL", 0, 0 }
  };

  gtk_drag_dest_set(paste_list,
		    (GtkDestDefaults)(GTK_DEST_DEFAULT_MOTION |
				      GTK_DEST_DEFAULT_HIGHLIGHT |
				      GTK_DEST_DEFAULT_DROP),
		    target_table, 3,
		    GDK_ACTION_COPY);
  gtk_signal_connect(GTK_OBJECT(paste_list), "drag_data_received",
		     GTK_SIGNAL_FUNC(add_url_on_dnd),
		     NULL);
  */
  //// window property
  // modified
  visibleFlag = false;
  gtk_widget_realize(window);
  gtk_window_set_modal(GTK_WINDOW(window), FALSE);
  if(toplevel != NULL)
    gtk_window_set_transient_for(GTK_WINDOW(window),
				 GTK_WINDOW(toplevel));
}

GtkWidget *PasteWindow::getPasteList()
{
  return paste_list;
}

// clipboard_contentsにはクリップボードの内容がnull-terminatedの
// 文字列で格納されている。それをCLIST paste_listにURLと思えるもの
// だけ追加する。またここでclass ItemCellのインスタンスも作成する。
bool PasteWindow::addURL(string url_string, int mime_info)
{
  bool retval = false;
  bool ws_quark;

  if(mime_info == MIME_TEXT_PLAIN) {
    ws_quark = true;
  } else {
    ws_quark = false;
  }
  while(url_string.size()) {
    string url = URLcontainer::Find_URL(url_string, ws_quark);
    if(addItem(url)) retval = true;
  }
  return retval;
}

bool PasteWindow::addURLByNumericalExpansion(string url_string)
{
  bool retval = false;
  while(url_string.size()) {
    list<string> url_list = URLcontainer::Unfold_URL(URLcontainer::Find_URL(url_string));
    //while(unfolded_urls.size()) {
    //string url = URLcontainer::Find_URL(unfolded_urls);
    for(list<string>::const_iterator itr = url_list.begin(); itr != url_list.end(); ++itr) {
      if(addItem(*itr)) retval = true;
    }
  }
  return retval;
}

bool PasteWindow::addItem(ItemCell *itemcell)
{
  char *clist_item[2];
  clist_item[0] = strdup(itemcell->ret_URL().c_str());
  clist_item[1] = strdup(itemcell->ret_Options_opt().ret_Store_Dir().c_str());
  int rowindex = gtk_clist_append(GTK_CLIST(paste_list), clist_item);
  //GdkColor color = {0, 0, 0};
  //gtk_clist_set_foreground(GTK_CLIST(paste_list), rowindex, &color);

  delete [] clist_item[0];
  delete [] clist_item[1];
  gtk_clist_set_row_data(GTK_CLIST(paste_list), rowindex, itemcell);
  g_itemManagerPaste->regist_item_back(itemcell);
  return true;
}

bool PasteWindow::addItem(const string& url)
{
  URLcontainer urlcon;

  //urlcon.Parse_URL(url);
  if(urlcon.Parse_URL(url) &&
     //!item_manager->search_by_url(urlcon.ret_URL()) &&
     !g_itemManagerPaste->search_by_url(urlcon.ret_URL())) {

    ListEntry *listentry = g_listManager->ret_Current_listentry();
    const Options& options = listentry->ret_Options();
    ItemCell *itemcell = new ItemCell(url, urlcon, options, _("Created"));
    /*
    if(urlcon.ret_Protocol() == "http:") {
      itemcell = new ItemCell_HTTP(url, urlcon, options, _("Created"));
    } else if(urlcon.ret_Protocol() == "ftp:") {
      itemcell = new ItemCell_FTP(url, urlcon, options, _("Created"));
    } else {
      // unsupported protocols
      return false;
    }
    */
    char *clist_item[2];
    clist_item[0] = strdup(urlcon.ret_URL().c_str());
    clist_item[1] = strdup(itemcell->ret_Options_opt().ret_Store_Dir().c_str());
    int rowindex = gtk_clist_append(GTK_CLIST(paste_list), clist_item);
    //GdkColor color = {0, 0, 0};
    //gtk_clist_set_foreground(GTK_CLIST(paste_list), rowindex, &color);

    delete [] clist_item[0];
    delete [] clist_item[1];
    gtk_clist_set_row_data(GTK_CLIST(paste_list), rowindex, itemcell);
    g_itemManagerPaste->regist_item_back(itemcell);
    return true;
  } else {
    return false;
  }
}

void PasteWindow::show()
{
  visibleFlag = true;
  updateTargetListWithCurrentList();
  updateAutostart();
  Paste_set_sensitive__list_not_empty();
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_MOUSE);
  gtk_widget_show(window);
}

void PasteWindow::show(const string& listName)
{
  visibleFlag = true;
  gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(targetListCombo)->entry), listName.c_str());    
  updateTargetList();
  updateAutostart();
  Paste_set_sensitive__list_not_empty();
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_MOUSE);
  gtk_widget_show(window);
}

void PasteWindow::hide()
{
  visibleFlag = false;

  gtk_widget_hide(window);
}

static GList *list2glist(const list<string>& stringList)
{
  GList *glist = NULL;

  for(list<string>::const_iterator itr = stringList.begin(); itr != stringList.end(); ++itr) {
    char *entry_string = g_strdup(itr->c_str());
    glist = g_list_append(glist, entry_string);
  }
  return glist;
}

static void freeGList(GList *glist) {
  GList *node = glist;
  while(node) {
    //delete [] (char *)node->data;
    g_free(node->data);
    node = g_list_next(node);
  }
  g_list_free(glist);
}

void PasteWindow::updateTargetList() {
  string currentName = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(targetListCombo)->entry));
  GList *nameList = list2glist(g_listManager->getListNames());
  gtk_combo_set_popdown_strings(GTK_COMBO(targetListCombo), nameList);
  //gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(targetListCombo)->entry), g_listManager->ret_Current_listentry()->getName().c_str());
  if(g_listManager->checkDuplicatedName(currentName)) {
    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(targetListCombo)->entry), currentName.c_str());    
  } else {
    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(targetListCombo)->entry), g_listManager->ret_Current_listentry()->getName().c_str());
  }
  freeGList(nameList);
}

void PasteWindow::updateTargetListWithCurrentList() {
  GList *nameList = list2glist(g_listManager->getListNames());
  gtk_combo_set_popdown_strings(GTK_COMBO(targetListCombo), nameList);
  gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(targetListCombo)->entry), g_listManager->ret_Current_listentry()->getName().c_str());
  freeGList(nameList);
}

void PasteWindow::updateAutostart() {
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(autostartToggle),
			       g_appOption->Whether_use_automatic_start());
}

GtkWidget *PasteWindow::getWindow() {
  return window;
}

string PasteWindow::getTargetListName() const {
  return gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(targetListCombo)->entry));
}

bool PasteWindow::isAutostartEnabled() const {
  if(GTK_TOGGLE_BUTTON(autostartToggle)->active) {
    return true;
  } else {
    return false;
  }
}

bool PasteWindow::isVisible() const {
  return visibleFlag;
}

int PasteWindow::getSelectedItemNum() const {
  return g_list_length(GTK_CLIST(paste_list)->selection);
}

void PasteWindow::deleteItem() {
  ItemCell *itemcell;
  GList *node = GTK_CLIST(paste_list)->selection;
  while(node) {
    unsigned int rowindex = GPOINTER_TO_UINT(node->data);
    itemcell = (ItemCell*)gtk_clist_get_row_data(GTK_CLIST(paste_list), rowindex);
    g_itemManagerPaste->unregist_item(itemcell);
    gtk_clist_remove(GTK_CLIST(paste_list), rowindex);
    delete itemcell;
    itemcell = NULL;
    node = GTK_CLIST(paste_list)->selection;
  }
}

void PasteWindow::deleteAllItem() {
  pthread_mutex_lock(&listLock);
  ItemCell *itemcell;
  for(unsigned int i = 0;(itemcell = (ItemCell *)gtk_clist_get_row_data(GTK_CLIST(paste_list), i)) != NULL; ++i) {
    delete itemcell;
  }
  gtk_clist_clear(GTK_CLIST(paste_list));
  g_itemManagerPaste->all_clear();  
  pthread_mutex_unlock(&listLock);
}
