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

// $Id: ItemStatusPartial.cc,v 1.17 2001/06/14 13:57:51 tujikawa Exp $

#include "ItemStatusPartial.h"

extern void Set_clist_column__progress(GtkWidget *clist, int rowindex, int progress);
extern void Set_clist_column__icon(GtkWidget *clist, int rowindex , ItemCell::ItemStatusType status);
extern ListManager *g_listManager;

ItemStatusPartial::ItemStatusPartial(ItemCell *itemcell_in, ItemCell *itemcell_partial_in) : ItemStatus(itemcell_in)
{
  itemcell_partial = itemcell_partial_in;
}

ItemStatusPartial::~ItemStatusPartial()
{
}

void ItemStatusPartial::Update()
{
  char *clist_item[TOTALCOL];

  if(!g_listManager->Search(listentry)) {
    // delete itemcell_partial
    delete itemcell_partial;
    return;
  }

  listentry->get_Dl_clist_lock();

  ItemCell *itemcell_worker = itemcell_partial;
  ItemCell *itemcell_boss = ret_ItemCell();
  clist_item[COL_ICON] = "";
  clist_item[COL_FILENAME] = g_strconcat("|->", itemcell_worker->ret_Filename().c_str(), NULL);
  clist_item[COL_CURSIZE] = "0";
  clist_item[COL_TOTSIZE] = _("unknown");
  clist_item[COL_PROGRESS] = "";//NULL;
  clist_item[COL_SPEED] = "";
  clist_item[COL_RTIME] = "";

  if(itemcell_worker->ret_Options().ret_Retry() == -1) {
    //modified 2001/5/21
    clist_item[COL_RETRY] = g_strdup("0/-");
  } else {
    clist_item[COL_RETRY] = g_strdup_printf("0/%d", itemcell_worker->ret_Options().ret_Retry());
  }
  clist_item[COL_REC] = g_strdup_printf("%d", itemcell_worker->ret_Options().ret_recurse_count());
  clist_item[COL_STATUS] = "";
  clist_item[COL_CRC] = "";
  clist_item[COL_MD5] = "";
  clist_item[COL_SAVE] = g_strdup(itemcell_worker->ret_Options().ret_Store_Dir().c_str());
  clist_item[COL_URL] = g_strdup(itemcell_worker->ret_URL().c_str());

  bool vadj_flag = false;
  float vadj = 0.0;
  GtkAdjustment *adj = NULL;
  if(gtk_clist_row_is_visible(GTK_CLIST(listentry->ret_Dl_clist()), GTK_CLIST(listentry->ret_Dl_clist())->rows-1) == GTK_VISIBILITY_NONE) {
    vadj_flag = true;
    adj = gtk_clist_get_vadjustment(GTK_CLIST(listentry->ret_Dl_clist()));
    vadj = adj->value;
  }

  int rowindex = listentry->Insert_dl_item(clist_item, itemcell_worker, itemcell_boss);

  //listentry->Set_clist_column__progress(rowindex, 0);
  //listentry->Set_clist_column__icon(rowindex, itemcell_worker->ret_Status());

  itemcell_boss->Append_worker(itemcell_worker);

  if(vadj_flag) {
    gtk_adjustment_set_value(adj, vadj);
  }

  g_free(clist_item[COL_FILENAME]);
  g_free(clist_item[COL_RETRY]);
  g_free(clist_item[COL_REC]);
  g_free(clist_item[COL_SAVE]);
  g_free(clist_item[COL_URL]);

  if(itemcell_boss->ret_Options().ret_Divide() == itemcell_boss->ret_Worker_list().size()) {
    listentry->Send_start_signal();
  }

  listentry->release_Dl_clist_lock();
}
