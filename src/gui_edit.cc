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

// $Id: gui_edit.cc,v 1.48 2001/11/28 15:28:40 tujikawa Exp $

#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include "aria.h"
#include "ItemCell.h"
#include "ListManager.h"
#include "PasteWindow.h"
#include "CRCList.h"
#include "AppOption.h"
#include "utils.h"
#include "ItemCache.h"
#include "ShortCutKey.h"
#include "gui_utils.h"

#include "pixmaps/paste.xpm"

using namespace std;

// external functions
extern void Set_sensitive__list_empty();
extern void Set_sensitive__list_not_empty();
extern void Set_sensitive__items_selected();
extern void Set_sensitive__no_item_selected();
extern gboolean Edit_search(GtkWidget *w, gpointer data);
extern bool Download_clear_sub(ItemCell *itemcell, ListEntry *listentry);

extern gboolean Create_search_window();

// external variables
extern ItemCell *g_consoleItem;
extern AppOption *g_appOption;
extern ListManager *g_listManager;
extern PasteWindow *g_pasteWindow;
ItemCache *g_itemCache;

static GtkWidget *copyItem_item;
static GtkWidget *cutItem_item;
static GtkWidget *pasteItem_item;
static GtkWidget *menu, *root_item;
static GtkWidget *pasteURL_item;
static GtkWidget *pasteURLexp_item;
static GtkWidget *pasteCRC_item;
static GtkWidget *pasteMD5_item;
static GtkWidget *selectAll_item;
static GtkWidget *invert_item;
static GtkWidget *search_item;

void Edit_set_sensitive__no_item_selected()
{
  gtk_widget_set_sensitive(copyItem_item, FALSE);
  gtk_widget_set_sensitive(cutItem_item, FALSE);
}

void Edit_set_sensitive__items_selected()
{
  gtk_widget_set_sensitive(copyItem_item, TRUE);
  gtk_widget_set_sensitive(cutItem_item, TRUE);
}

// select all item in download list
static gboolean Edit_select_all(GtkWidget *w, gpointer data)
{
  ListEntry *listentry = g_listManager->ret_Current_listentry();
  listentry->get_Dl_clist_lock();

  /*gtk_clist_select_all(GTK_CLIST(listentry->ret_Dl_clist()));
  GList *node = GTK_CLIST(listentry->ret_Dl_clist())->selection;
  if(node != NULL) {
    Set_sensitive__items_selected(); // fix this
  }*/
  GtkTreeSelection* sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(listentry->ret_Dl_clist()));
  gtk_tree_selection_select_all(sel);
  listentry->release_Dl_clist_lock();
  return TRUE;
}

// invert selection
static gboolean Edit_invert_selection(GtkWidget *w, gpointer data)
{
  ListEntry *listentry = g_listManager->ret_Current_listentry();
  GtkWidget *clist = listentry->ret_Dl_clist();
  listentry->get_Dl_clist_lock();
  listentry->freezeDlCList();
  /*GList *selection = g_list_copy(GTK_CLIST(clist)->selection);

  // select all, then unselect row that is in node list
  gtk_clist_select_all(GTK_CLIST(clist));
  GList *node = selection;
  while(node) {
    int rowindex = GPOINTER_TO_INT(node->data);
    gtk_clist_unselect_row(GTK_CLIST(clist), rowindex, 0);
    node = g_list_next(node);
  }
  if(GTK_CLIST(clist)->selection != NULL) {
    Set_sensitive__items_selected();
  } else {
    Set_sensitive__no_item_selected();
  }
  g_list_free(selection);*/
  GtkTreeIter iter;
  GtkTreeSelection* sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(clist));
  if (gtk_tree_model_get_iter_first(gtk_tree_view_get_model(GTK_TREE_VIEW(clist)), &iter))
  {
	  do
	  {
		  if (gtk_tree_selection_iter_is_selected(sel, &iter))
		  {
			  gtk_tree_selection_unselect_iter(sel, &iter);
		  }
		  else
		  {
			  gtk_tree_selection_select_iter(sel, &iter);
		  }
	  } while (gtk_tree_model_iter_next(gtk_tree_view_get_model(GTK_TREE_VIEW(clist)), &iter));
  }
  listentry->thawDlCList();
  listentry->release_Dl_clist_lock();

  return TRUE;
}

static int gt_func(int x, int y)
{
  if(x > y) return 1;
  else return -1;
}

gboolean Edit_copy_item(GtkWidget *w, gpointer data)
{
  ListEntry *listentry = g_listManager->ret_Current_listentry();

  GList *node = GTK_CLIST(listentry->ret_Dl_clist())->selection;
  if(node == NULL) {
    return TRUE;
  }

  node = g_list_copy(GTK_CLIST(listentry->ret_Dl_clist())->selection);
  node = g_list_sort(node, (int (*)(const void *, const void *))gt_func);

  listentry->get_Dl_clist_lock();
  g_itemCache->clear_Cache();

  int num = g_list_length(node);

  while(node) {
    int rowindex = GPOINTER_TO_INT(node->data);

    ItemCell *itemcell = (ItemCell *)gtk_clist_get_row_data(GTK_CLIST(listentry->ret_Dl_clist()), rowindex);
    if(!itemcell->Is_Partial()) {
      g_itemCache->add_Cache_by_copy(itemcell);
    } else {
      --num;
    }
    node = g_list_next(node);
  }
  g_list_free(node);

  listentry->release_Dl_clist_lock();

  string line = itos(num)+_(" item(s) copied");
  g_consoleItem->Send_message_to_gui(line, MSG_SYS_INFO);

  gtk_widget_set_sensitive(pasteItem_item, TRUE);
  return TRUE;
}

gboolean Edit_cut_item(GtkWidget *w, gpointer data)
{
  ListEntry *listentry = g_listManager->ret_Current_listentry();
  
  g_itemCache->clear_Cache();

  GList *node = GTK_CLIST(listentry->ret_Dl_clist())->selection;  
  if(node == NULL) {
    return TRUE;
  }

  node = g_list_copy(GTK_CLIST(listentry->ret_Dl_clist())->selection);
  node = g_list_sort(node, (int (*)(const void *, const void *))gt_func);

  listentry->get_Dl_clist_lock();
  listentry->freezeDlCList();

  int num = g_list_length(node);

  while(node) {
    int rowindex = GPOINTER_TO_INT(node->data);
    ItemCell *itemcell = (ItemCell *)gtk_clist_get_row_data(GTK_CLIST(listentry->ret_Dl_clist()), rowindex);
    if(!itemcell->Is_Partial()) {
      g_itemCache->add_Cache_by_copy(itemcell);
      for(list<ItemCell*>::const_iterator item_ptr = itemcell->ret_Worker_list().begin(); item_ptr != itemcell->ret_Worker_list().end(); ++item_ptr) {
	Download_clear_sub((ItemCell*)*item_ptr, listentry);
      }
      Download_clear_sub(itemcell, listentry);
      node = GTK_CLIST(listentry->ret_Dl_clist())->selection;
    } else {
      --num;
      node = g_list_next(node);
    }

  }
  g_list_free(node);

  listentry->thawDlCList();
  listentry->release_Dl_clist_lock();

  string line = itos(num)+_(" item(s) cut");
  g_consoleItem->Send_message_to_gui(line, MSG_SYS_INFO);

  Set_sensitive__no_item_selected();
  if(GTK_CLIST(listentry->ret_Dl_clist())->rows == 0) {
    Set_sensitive__list_empty();
  }
  gtk_widget_set_sensitive(pasteItem_item, TRUE);

  return TRUE;
}

gboolean Edit_paste_item(GtkWidget *w, gpointer data)
{
  if(g_itemCache->ret_Cache_Total() == 0) return TRUE;
  ListEntry *listentry = g_listManager->ret_Current_listentry();
  listentry->get_Dl_clist_lock();
  g_itemCache->paste_Cache(listentry);

  gtk_clist_moveto(GTK_CLIST(listentry->ret_Dl_clist()),
		   GTK_CLIST(listentry->ret_Dl_clist())->rows-1,
		   0,
		   0.0, 0.0);
  // modified 2001/5/20
  if(g_appOption->Whether_use_automatic_start()) {
    listentry->Send_start_signal();      
  }

  listentry->release_Dl_clist_lock();

  string line = itos(g_itemCache->ret_Cache_Total())+_(" item(s) pasted");
  g_consoleItem->Send_message_to_gui(line, MSG_SYS_INFO);

  Set_sensitive__list_not_empty();

  return TRUE;
}

void Add_new_item_to_downloadlist(ItemCell *itemcell, ListEntry *listentry)
{
  char *clist_item[TOTALCOL];

  clist_item[COL_ICON] = NULL;
  if(itemcell->ret_Filename().empty()) {
    clist_item[COL_FILENAME] = g_strdup(_("<directory>"));
  } else {
    clist_item[COL_FILENAME] = g_strdup(itemcell->ret_Filename().c_str());
  }

  if(g_appOption->ret_use_size_human_readable()) {
    clist_item[COL_CURSIZE] = g_strdup(get_human_readable_size(itemcell->ret_Size_Current()).c_str());
  } else {
    clist_item[COL_CURSIZE] = g_strdup(itos(itemcell->ret_Size_Current(), true).c_str());
  }
  if(itemcell->ret_Size_Total() == 0) {
    clist_item[COL_TOTSIZE] = g_strdup(_("unknown"));
  } else {
    if(g_appOption->ret_use_size_human_readable()) {
      clist_item[COL_TOTSIZE] = g_strdup(get_human_readable_size(itemcell->ret_Size_Total()).c_str());
    } else {
      clist_item[COL_TOTSIZE] = g_strdup(itos(itemcell->ret_Size_Total(), true).c_str());
    }
  }
  clist_item[COL_PROGRESS] = NULL;//"";
  if(itemcell->ret_Options().ret_Retry() == -1) {
    // modified 2001/5/21
    clist_item[COL_RETRY] = g_strdup("0/-");
  } else {
    clist_item[COL_RETRY] = g_strdup_printf("0/%d", itemcell->ret_Options().ret_Retry());
  }
  int n_rec;
  if(itemcell->ret_URL_Container().ret_Protocol() == "http:"
#ifdef HAVE_OPENSSL
     || itemcell->ret_URL_Container().ret_Protocol() == "https:"
#endif // HAVE_OPENSSL
     ) {
    n_rec = itemcell->ret_Options().ret_recurse_count();
  } else {
    n_rec = itemcell->ret_Options().ret_FTP_recurse_count();
  }
  clist_item[COL_REC] = g_strdup_printf("%d", n_rec);
  clist_item[COL_SPEED] = g_strdup("");
  clist_item[COL_RTIME] = g_strdup("");
  string crc_string;
  switch(itemcell->ret_CRC_Type()) {
  case ItemCell::CRC_16:
    crc_string = itos_hex(itemcell->ret_CRC(), 4, '0');
    break;
  case ItemCell::CRC_32:
    crc_string = itos_hex(itemcell->ret_CRC(), 8, '0');
    break;
  default:
    break;
  }
  clist_item[COL_CRC] = g_strdup(crc_string.c_str());
  clist_item[COL_MD5] = g_strdup(itemcell->ret_md5string().c_str());
  clist_item[COL_STATUS] = _("READY");
  clist_item[COL_SAVE] = g_strdup(itemcell->ret_Options().ret_Store_Dir().c_str());
  clist_item[COL_URL] = g_strdup(itemcell->ret_URL().c_str());
  //unsigned int rowindex = gtk_clist_append(GTK_CLIST(itemlistwidget), clist_item);
  itemcell->set_id(-1);
  listentry->Append_dl_item(clist_item, itemcell);
  g_free(clist_item[COL_FILENAME]);
  g_free(clist_item[COL_RETRY]);
  g_free(clist_item[COL_REC]);
  g_free(clist_item[COL_CRC]);
  g_free(clist_item[COL_MD5]);
  g_free(clist_item[COL_SAVE]);
  g_free(clist_item[COL_URL]);
  g_free(clist_item[COL_CURSIZE]);
  g_free(clist_item[COL_TOTSIZE]);
  //listentry->Set_clist_column__icon(rowindex, itemcell->ret_Status());
  //listentry->Set_clist_column__progress(rowindex, 0);
}

static void Paste_md5_set(const string& md5Strings)
{
  list<CRCList*> md5List;
  unsigned int index = 0;

  ListEntry *listentry = g_listManager->ret_Current_listentry();

  while(index < md5Strings.size()) {
    string filenameString;
    string md5String;

    unsigned int lf_pos, top_pos;
    if((lf_pos = md5Strings.find('\n', index)) == string::npos) {
      lf_pos = md5Strings.size();
    }
    if((top_pos = md5Strings.find_first_not_of(" \t", index)) != string::npos && top_pos < lf_pos) {
      index = lf_pos+1;
      string md5EntryString = md5Strings.substr(top_pos, lf_pos);
      unsigned int start_pos, end_pos;
      if((end_pos = md5EntryString.find_first_of(" \t")) == string::npos) continue;
      md5String = md5EntryString.substr(0, end_pos);

      if((start_pos = md5EntryString.find_first_not_of(" \t", end_pos)) == string::npos) continue;
      if((end_pos = md5EntryString.find_first_of(" \t\n", start_pos)) == string::npos) {
	end_pos = lf_pos;//md5EntryString.size();
      }

      filenameString = md5EntryString.substr(start_pos, end_pos-start_pos);
      CRCList *md5Entry = new CRCList(filenameString, md5String);
      md5List.push_back(md5Entry);
    } else {
      index = lf_pos+1;
    }
  }

  const list<int>& download_list = listentry->getItemManager()->ret_id_list();
  for(list<int>::const_iterator id_itr = download_list.begin(); id_itr != download_list.end(); ++id_itr) {
    ItemCell *itemcell = listentry->getItemManager()->ret_itemaddr(*id_itr);
    for(list<CRCList*>::iterator md5List_itr = md5List.begin(); md5List_itr != md5List.end(); ++md5List_itr) {
      if(itemcell->ret_URL_Container().ret_Filename() == (*md5List_itr)->ref_Filename()) {
	itemcell->set_md5string((*md5List_itr)->ref_CRC_string());
	int rowindex = gtk_clist_find_row_from_data(GTK_CLIST(listentry->ret_Dl_clist()), itemcell);
	listentry->Set_clist_column__md5(rowindex, (*md5List_itr)->ref_CRC_string());
	md5List.remove(*md5List_itr);
	delete *md5List_itr;
	break;
      }
    }
  }
  for(list<CRCList*>::iterator md5List_itr = md5List.begin(); md5List_itr != md5List.end(); ++md5List_itr) {
    delete *md5List_itr;
  }
}

static void Paste_crc_set(const string& crc_strings)
{
  list<CRCList*> crc_list;
  unsigned int index = 0;

  ListEntry *listentry = g_listManager->ret_Current_listentry();

  while(index < crc_strings.size()) {
    string filename_entry;
    string crc_string;
    string size_string;
    unsigned int lf_pos, top_pos;
    if((lf_pos = crc_strings.find('\n', index)) == string::npos) {
      lf_pos = crc_strings.size();
    }
    if((top_pos = crc_strings.find_first_not_of(" \t", index)) != string::npos && top_pos < lf_pos) {
      index = lf_pos+1;
      string crc_entry_string = crc_strings.substr(top_pos, lf_pos);
      unsigned int start_pos, end_pos;
      if((end_pos = crc_entry_string.find_first_of(" \t")) == string::npos) continue;
      filename_entry = crc_entry_string.substr(0, end_pos);
      if((start_pos = crc_entry_string.find_first_not_of(" \t", end_pos)) == string::npos) continue;
      if((end_pos = crc_entry_string.find_first_of(" \t", start_pos)) == string::npos) end_pos = crc_entry_string.size();
      crc_string = crc_entry_string.substr(start_pos, end_pos-start_pos);
            
      CRCList *crc_entry = new CRCList(filename_entry, crc_string);
      crc_list.push_back(crc_entry);
    } else {
      index = lf_pos+1;
    }
  }
  const list<int>& download_list = listentry->getItemManager()->ret_id_list();
  for(list<int>::const_iterator id_itr = download_list.begin(); id_itr != download_list.end(); ++id_itr) {
    ItemCell *itemcell = listentry->getItemManager()->ret_itemaddr(*id_itr);
    for(list<CRCList*>::iterator crc_list_itr = crc_list.begin(); crc_list_itr != crc_list.end(); ++crc_list_itr) {
      if(itemcell->ret_URL_Container().ret_Filename() == (*crc_list_itr)->ref_Filename()) {
	int rowindex;
	switch((*crc_list_itr)->ref_CRC_string().size()) {
	case 8:
	  itemcell->set_CRC(stoui((*crc_list_itr)->ref_CRC_string(), 16));
	  itemcell->set_CRC_Type(ItemCell::CRC_32);
	  rowindex = gtk_clist_find_row_from_data(GTK_CLIST(listentry->ret_Dl_clist()), itemcell);
	  listentry->Set_clist_column__crc(rowindex, (*crc_list_itr)->ref_CRC_string());
	  crc_list.remove(*crc_list_itr);
	  delete *crc_list_itr;
	  break;
	case 4:
	  itemcell->set_CRC(stoui((*crc_list_itr)->ref_CRC_string(), 16));
	  itemcell->set_CRC_Type(ItemCell::CRC_16);
	  rowindex = gtk_clist_find_row_from_data(GTK_CLIST(listentry->ret_Dl_clist()), itemcell);
	  listentry->Set_clist_column__crc(rowindex, (*crc_list_itr)->ref_CRC_string());
	  crc_list.remove(*crc_list_itr);
	  delete *crc_list_itr;
	  break;
	default:
	  cerr << "WARNING: unsupported CRC format" << endl;
	}
	break;
      }
    }
  }
  for(list<CRCList*>::iterator crc_list_itr = crc_list.begin(); crc_list_itr != crc_list.end(); ++crc_list_itr) {
    delete *crc_list_itr;
  }
}

//XのクリップボードからURLを貼付け
gboolean Paste_get_clipboard(GtkWidget *widget, gpointer data)
{
  static GdkAtom targets_atom = GDK_NONE;

  // 文字列 "STRING" に対応するアトムを取得
  if (targets_atom == GDK_NONE)
    targets_atom = gdk_atom_intern ("STRING", FALSE);

  // プライマリセレクションの "STRING" ターゲットを要求
  gtk_selection_convert (widget, GDK_SELECTION_PRIMARY, targets_atom,
			 GDK_CURRENT_TIME);
  return TRUE;
}
/*
gboolean Watch_clip_board(gpointer data)
{
  Paste_get_clipboard(pasteURL_item, NULL);

  return TRUE;
}
*/
// セレクションの所有者がデータを返すと呼ばれるシグナルハンドラ
gboolean Paste_md5_selection_received(GtkWidget* widget, GtkSelectionData* selection_data, gpointer data)
{
  char *clipboard_contents;

  // 検索が成功したかどうかを確認するためのチェック
  if (selection_data->length < 0) {
#ifdef DEBUG
      cout << "Selection retrieval failed\n";
#endif
      return(TRUE);
  }
  // 期待した形式でデータを取得したことを確認
  if (selection_data->type != GDK_SELECTION_TYPE_STRING) {
#ifdef DEBUG
    cout << "Selection \"TARGETS\" was not returned as atoms!\n";
#endif
    return(TRUE);
  }
       
  // 受け取ったアトムを出力
  clipboard_contents = (char *)selection_data->data;
  //cerr << "clipboard_contents:\n" << clipboard_contents << "\n" << flush;

  Paste_md5_set(clipboard_contents);
  return TRUE;
}

// セレクションの所有者がデータを返すと呼ばれるシグナルハンドラ
gboolean Paste_crc_selection_received(GtkWidget* widget, GtkSelectionData* selection_data, gpointer data)
{
  char *clipboard_contents;

  // 検索が成功したかどうかを確認するためのチェック
  if (selection_data->length < 0) {
#ifdef DEBUG
      cout << "Selection retrieval failed\n";
#endif
      return(TRUE);
  }
  // 期待した形式でデータを取得したことを確認
  if (selection_data->type != GDK_SELECTION_TYPE_STRING) {
#ifdef DEBUG
    cout << "Selection \"TARGETS\" was not returned as atoms!\n";
#endif
    return(TRUE);
  }
       
  // 受け取ったアトムを出力
  clipboard_contents = (char *)selection_data->data;
  //cerr << "clipboard_contents:\n" << clipboard_contents << "\n" << flush;

  Paste_crc_set(clipboard_contents);
  return(TRUE);
}


// セレクションの所有者がデータを返すと呼ばれるシグナルハンドラ
gboolean Paste_selection_received (GtkWidget *widget, GtkSelectionData *selection_data, gpointer data)
{
  char *clipboard_contents;

  // 検索が成功したかどうかを確認するためのチェック
  if (selection_data->length < 0) {
      return TRUE;
  }
  // 期待した形式でデータを取得したことを確認
  if (selection_data->type != GDK_SELECTION_TYPE_STRING) {
    return TRUE;
  }       
  // 受け取ったアトムを出力
  clipboard_contents = (char*)selection_data->data;

  //gtk_clist_clear(GTK_CLIST(additem_clist));

  if(g_pasteWindow->addURL(clipboard_contents)) {
    g_pasteWindow->show();
  }

  return TRUE;
}

// URLを展開してから貼付け
gboolean Paste_url_unfold_selection_received (GtkWidget *widget, GtkSelectionData *selection_data, gpointer data)
{
  char *clipboard_contents;

  // 検索が成功したかどうかを確認するためのチェック
  if (selection_data->length < 0) {
      return TRUE;
  }
  // 期待した形式でデータを取得したことを確認
  if (selection_data->type != GDK_SELECTION_TYPE_STRING) {
    return TRUE;
  }
       
  // 受け取ったアトムを出力
  clipboard_contents = (char*)selection_data->data;

  if(g_pasteWindow->addURLByNumericalExpansion(clipboard_contents)) {
    g_pasteWindow->show();
  }

  return TRUE;
}

void Create_edit_menu(GtkWidget *toplevel, GtkWidget *menu_bar, GtkAccelGroup *accel_group)
{
  //create Edit menu
  menu = gtk_menu_new();

  // create each menu item
  selectAll_item = GTK_create_menu_item_with_icon(menu,
						  _("Select all"),
						  GTK_SIGNAL_FUNC(Edit_select_all),
						  NULL,
						  
						  accel_group,
						  SC_SELECT_ALL,
						  SCM_SELECT_ALL);


  invert_item = GTK_create_menu_item_with_icon(menu,
				     _("Invert selection"),
				     GTK_SIGNAL_FUNC(Edit_invert_selection),
				     NULL);

  GTK_create_menu_separator(menu);

  
  pasteURL_item = GTK_create_menu_item_with_icon(menu,
						 _("Paste URL list"),
						 GTK_SIGNAL_FUNC(Paste_get_clipboard),
						 NULL,
						 paste_xpm,
						 toplevel,
						 accel_group,
						 SC_PASTEURL,
						 SCM_PASTEURL);

  gtk_signal_connect(GTK_OBJECT(pasteURL_item), "selection_received",
		     GTK_SIGNAL_FUNC(Paste_selection_received),
		     NULL);

  pasteURLexp_item = GTK_create_menu_item_with_icon(menu,
					  _("Paste URL list with numerical expansion"),
					  GTK_SIGNAL_FUNC(Paste_get_clipboard),
					  NULL,
					  accel_group,
					  SC_PASTEURLEXP,
					  SCM_PASTEURLEXP);

  gtk_signal_connect(GTK_OBJECT(pasteURLexp_item), "selection_received",
		     GTK_SIGNAL_FUNC(Paste_url_unfold_selection_received),
		     NULL);



  pasteCRC_item = GTK_create_menu_item_with_icon(menu,
				       _("Paste CRC list"),
				       GTK_SIGNAL_FUNC(Paste_get_clipboard),
				       NULL,
				       accel_group,
				       SC_PASTECRC,
				       SCM_PASTECRC);

  
  gtk_signal_connect(GTK_OBJECT(pasteCRC_item), "selection_received",
		     GTK_SIGNAL_FUNC(Paste_crc_selection_received),
		     NULL);

  pasteMD5_item = GTK_create_menu_item_with_icon(menu,
				       _("Paste MD5 list"),
				       GTK_SIGNAL_FUNC(Paste_get_clipboard),
				       NULL);
  /*
				       accel_group,
				       SC_PASTEMD5,
				       SCM_PASTEMD5);
  */
  
  gtk_signal_connect(GTK_OBJECT(pasteMD5_item), "selection_received",
		     GTK_SIGNAL_FUNC(Paste_md5_selection_received),
		     NULL);


  GTK_create_menu_separator(menu);

  copyItem_item = GTK_create_menu_item_with_icon(menu,
				       _("Copy selected items"),
				       GTK_SIGNAL_FUNC(Edit_copy_item),
				       NULL,
				       accel_group,
				       SC_COPYITEM,
				       SCM_COPYITEM);
  cutItem_item = GTK_create_menu_item_with_icon(menu,
				      _("Cut selected items"),
				      GTK_SIGNAL_FUNC(Edit_cut_item),
				      NULL,
				      accel_group,
				      SC_CUTITEM,
				      SCM_CUTITEM);
  pasteItem_item = GTK_create_menu_item_with_icon(menu,
					_("Paste items"),
					GTK_SIGNAL_FUNC(Edit_paste_item),
					NULL,
					accel_group,
					SC_PASTEITEM,
					SCM_PASTEITEM);
  GTK_create_menu_separator(menu);
  search_item = GTK_create_menu_item_with_icon(menu,
				     _("Search in current list"),
				     GTK_SIGNAL_FUNC(Edit_search),
				     NULL,
				     accel_group,
				     SC_SEARCH,
				     SCM_SEARCH);
  // create search panel window
  Create_search_window();

  root_item = gtk_menu_item_new_with_label(_("Edit"));
  gtk_widget_show(root_item);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(root_item), menu);
  gtk_menu_bar_append(GTK_MENU_BAR(menu_bar), root_item);
  // create paste window
  g_pasteWindow = new PasteWindow(GTK_WINDOW(toplevel));
  g_itemCache = new ItemCache();
  gtk_widget_set_sensitive(pasteItem_item, FALSE);
}
