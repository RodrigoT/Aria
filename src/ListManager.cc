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

// $Id: ListManager.cc,v 1.12 2001/11/04 10:18:07 tujikawa Exp $

#include "ListManager.h"

extern void Set_sensitive__list_empty();
extern void Set_sensitive__list_not_empty();
extern void Set_sensitive__items_selected();
extern void Set_sensitive__no_item_selected();
extern void Toolbar_set_thread_spin(int n_thread);
extern void Set_speed_scale(ItemCell* itemcell);
extern void Show_download_log(ItemCell* itemcell);

//  static gboolean ListPopup_cb(GtkWidget *widget,
//  			     GdkEventButton *event,
//  			     gpointer user_data) {
//    cerr << "button pressed on notebook" << endl;

//    return TRUE;
//  }

static void Page_switched(GtkNotebook *notebook,
			  GtkWidget *page,
			  int page_num,
			  gpointer user_data) {
  ListEntry *listentry = (ListEntry *)gtk_object_get_user_data(GTK_OBJECT(page));

  Toolbar_set_thread_spin(listentry->getThreadLimit());

  if( listentry->getRowCount() == 0) {
    // no item
    Set_sensitive__list_empty();
    Set_sensitive__no_item_selected();
  } else {
    // at least one item exists
    Set_sensitive__list_not_empty();
    if(listentry->getSelectedRowCount() != 0) {
      // some items selected
      Set_sensitive__items_selected();
      
      int rowindex = listentry->getLastSelectedRow();
      Set_speed_scale(listentry->getItemCellByRow(rowindex));
      Show_download_log(listentry->getItemCellByRow(rowindex));
    } else {
      // no items selected
      Set_sensitive__no_item_selected();
    }
  }
}

ListManager::ListManager()
{
  baseNotebook = gtk_notebook_new();
  gtk_notebook_set_tab_pos(GTK_NOTEBOOK(baseNotebook), GTK_POS_TOP);
  gtk_notebook_set_show_border(GTK_NOTEBOOK(baseNotebook), true);
  gtk_notebook_set_scrollable(GTK_NOTEBOOK(baseNotebook), true);
  gtk_notebook_popup_enable(GTK_NOTEBOOK(baseNotebook));

  // call back function
  g_signal_connect(GTK_OBJECT(baseNotebook), "switch-page",
		     GTK_SIGNAL_FUNC(Page_switched),
		     NULL);

  gtk_widget_show(baseNotebook);
//    gtk_notebook_popup_disable(GTK_NOTEBOOK(notebook));
//    g_signal_connect(GTK_OBJECT(notebook), "button-release-event",
//  		     GTK_SIGNAL_FUNC(ListPopup_cb),
//  		     NULL);

  for(int index = 0; index < ICON_TOTAL; ++index) {
    statusIcon[index] = NULL;
    statusIconMask[index] = NULL;
  }
}

ListManager::~ListManager()
{
}

GtkWidget *ListManager::ret_baseNotebook() {
  return baseNotebook;
}

void ListManager::Register(ListEntry *listentry)
{
  GtkWidget *scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show(scrolled_window);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW (scrolled_window),
				 GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
  gtk_object_set_user_data(GTK_OBJECT(scrolled_window), (void *)listentry);
  gtk_container_add(GTK_CONTAINER(scrolled_window), listentry->ret_Dl_clist());
  gtk_widget_show(listentry->ret_Dl_clist());
  GtkWidget *label = gtk_label_new(listentry->getName().c_str());
  gtk_notebook_append_page(GTK_NOTEBOOK(baseNotebook),
			   scrolled_window,
			   label);
  gtk_notebook_set_show_border(GTK_NOTEBOOK(baseNotebook),
			     true);
  gtk_notebook_set_tab_hborder(GTK_NOTEBOOK(baseNotebook),
			      2);
  
  listentry->setStatusIcon(statusIcon, statusIconMask);
  listentry_list.push_back(listentry);
}

void ListManager::DeleteCurrentList()
{
  Delete(gtk_notebook_get_current_page(GTK_NOTEBOOK(baseNotebook)));
}

void ListManager::Delete(int page_num)
{
  if(listentry_list.size() == 1) return;
  if(page_num < 0) {
    page_num = gtk_notebook_get_current_page(GTK_NOTEBOOK(baseNotebook));
  }
  GtkWidget *w = gtk_notebook_get_nth_page(GTK_NOTEBOOK(baseNotebook), page_num);
  ListEntry *listentry = (ListEntry *)gtk_object_get_user_data(GTK_OBJECT(w));
  listentry_list.remove(listentry);
  delete listentry;
  gtk_notebook_remove_page(GTK_NOTEBOOK(baseNotebook), page_num);
}

// fix this
bool ListManager::Set_active_page()
{
  gtk_notebook_set_page(GTK_NOTEBOOK(baseNotebook), -1);

  return true;
}

bool ListManager::Search(ListEntry *listentry)
{
  for(list<ListEntry*>::const_iterator itr = listentry_list.begin();
      itr != listentry_list.end(); ++itr) {
    if(listentry == *itr) {
      return true;
    }
  }
  return false;
}

ListEntry *ListManager::ret_Current_listentry()
{
  int page_num = gtk_notebook_get_current_page(GTK_NOTEBOOK(baseNotebook));
  if(page_num < 0) page_num = 0;
  GtkWidget *w = gtk_notebook_get_nth_page(GTK_NOTEBOOK(baseNotebook), page_num);
  if(w == NULL) return NULL;
  else return (ListEntry *)gtk_object_get_user_data(GTK_OBJECT(w));
}

ListEntry *ListManager::ret_nth_listentry(int page_num)
{
  GtkWidget *w = gtk_notebook_get_nth_page(GTK_NOTEBOOK(baseNotebook), page_num);
  return (ListEntry *)gtk_object_get_user_data(GTK_OBJECT(w));
}

int ListManager::ret_Length() const
{
  return listentry_list.size();
}

const list<ListEntry*>& ListManager::ret_Listentry_list() const
{
  return listentry_list;
}

void ListManager::Move_sub(ListMoveType mvtype)
{
  int page_num = gtk_notebook_get_current_page(GTK_NOTEBOOK(baseNotebook));

  if(page_num+mvtype > (int)listentry_list.size()-1 ||
     page_num+mvtype < 0) {
    return;
  }
  GtkWidget *child = gtk_notebook_get_nth_page(GTK_NOTEBOOK(baseNotebook), page_num);
  gtk_notebook_reorder_child(GTK_NOTEBOOK(baseNotebook),
			     child,
			     page_num+mvtype);
  Swap(page_num, page_num+mvtype);
}

void ListManager::Move_left()
{
  Move_sub(LIST_MOVE_LEFT);
}

void ListManager::Move_right()
{
  Move_sub(LIST_MOVE_RIGHT);
}

void ListManager::Swap(int page_num1, int page_num2)
{
  ListEntry *listentry1 = ret_nth_listentry(page_num1);
  ListEntry *listentry2 = ret_nth_listentry(page_num2);
  list<ListEntry *>::iterator itr1 = find(listentry_list.begin(), listentry_list.end(), listentry1);
  list<ListEntry *>::iterator itr2 = find(listentry_list.begin(), listentry_list.end(), listentry2);
  *itr1 = listentry2;
  *itr2 = listentry1;
}

int ListManager::getPage(ListEntry *listentry)
{
  int index = 0;
  for(list<ListEntry *>::iterator itr = listentry_list.begin(); itr != listentry_list.end(); ++itr, ++index) {
    if(*itr == listentry) {
      return index;
    }
  }
  return -1;
}

void ListManager::showPage(ListEntry *listentry)
{
  int page_num = getPage(listentry);
  if(page_num >= 0) {
    showPage(page_num);
  }
}

void ListManager::showPage(int page_num)
{
  if(page_num >= 0) {
    gtk_notebook_set_page(GTK_NOTEBOOK(baseNotebook), page_num);
  }
}

list<string> ListManager::getListNames() const {
  list<string> nameList;
  for(list<ListEntry *>::const_iterator itr = listentry_list.begin();
      itr != listentry_list.end(); ++itr) {
    nameList.push_back((*itr)->getName());
  }
  return nameList;
}

bool ListManager::checkDuplicatedName(string name) {
  if(getListEntryByName(name) == NULL) {
    return false;
  } else {
    return true;
  }
}

string ListManager::getNewDefaultListName() {
  int index = 1;
  string baseName = _("New list");
  while(1) {
    string name = baseName+itos(index);
    if(!checkDuplicatedName(name)) {
      return name;
    } else {
      name = baseName+itos(++index);
    }
  }
}

ListEntry *ListManager::getListEntryByName(string name) {
  for(list<ListEntry *>::const_iterator itr = listentry_list.begin();
      itr != listentry_list.end(); ++itr) {
    if((*itr)->getName() == name) {
      return *itr;
    }
  }
  return NULL;
}

void
ListManager::setStatusIcon(GdkPixbuf *pixmaps[], GdkBitmap *bitmaps[])
{
  for(int index = 0; index < ICON_TOTAL; ++index) {
    statusIcon[index] = pixmaps[index];
    statusIconMask[index] = bitmaps[index];
  }

  for(list<ListEntry*>::const_iterator itr = ret_Listentry_list().begin();
      itr != ret_Listentry_list().end(); ++itr) {
    (*itr)->setStatusIcon(pixmaps, bitmaps);
  }
}
