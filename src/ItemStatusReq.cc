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

// $Id: ItemStatusReq.cc,v 1.4 2001/11/19 16:26:20 tujikawa Exp $

#include "ItemStatusReq.h"
#include "ListManager.h"

extern void Save_files();
extern void Send_halt_message();
extern ListManager *g_listManager;

ItemStatusReq::ItemStatusReq(ItemCell *itemcell_in, RequestType req_in) : ItemStatus(itemcell_in)
{
  req = req_in;
}

ItemStatusReq::~ItemStatusReq()
{
}

void ItemStatusReq::Update()
{
  switch(req) {
  case REQ_QUIT:
    Send_halt_message();
    Save_files();
    exit(0);
  case REQ_MOVEVIEW:// move clist viewport to the specific row, added 2001/5/21
    {
      if(g_listManager->Search(listentry)) {
	int rowindex;
	GtkWidget *clist = listentry->ret_Dl_clist();
	if((rowindex = listentry->getRowForCell(ret_ItemCell())) > -1) {
	  /*if(gtk_clist_row_is_visible(GTK_CLIST(clist), rowindex) != GTK_VISIBILITY_FULL) {
	    gtk_clist_moveto(GTK_CLIST(clist), rowindex, 0, 0.5, 0.0);
	  } */
	  gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(clist), gtk_tree_path_new_from_indices(rowindex, -1), NULL, FALSE, 0.0, 0.0);
	}
      }
      break;
    }
  default:
    break;
  }
}
