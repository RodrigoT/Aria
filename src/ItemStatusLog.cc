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

// $Id: ItemStatusLog.cc,v 1.6 2001/05/19 18:38:59 tujikawa Exp $

#include "ItemStatusLog.h"

void Append_text(const ItemLogCell& itemlogcell, GtkWidget *text);

extern ListManager *g_listManager;
extern ItemCell *g_consoleItem;
extern GtkWidget *g_consoleText, *g_text;

ItemStatusLog::ItemStatusLog(ItemCell *itemcell_in, const ItemLogCell& itemlogcell_in) : ItemStatus(itemcell_in)
{
  item_log = itemlogcell_in;
}

ItemStatusLog::~ItemStatusLog()
{
}

void ItemStatusLog::Update()
{
  if(g_consoleItem == ret_ItemCell()) {
    Append_text(item_log, g_consoleText);
  } else {
    if(g_listManager->ret_Current_listentry() != listentry) return;
    GList *node = GTK_CLIST(listentry->ret_Dl_clist())->selection;
    if(node != NULL &&
       gtk_clist_get_row_data(GTK_CLIST(listentry->ret_Dl_clist()), GPOINTER_TO_UINT(node->data)) == ret_ItemCell()) {
      Append_text(item_log, g_text);
    }
  }
}
