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

// $Id: ItemStatusRec.cc,v 1.11 2001/10/17 13:06:34 tujikawa Exp $

#include "ItemStatusRec.h"

extern void Add_new_item_to_downloadlist(ItemCell *itemcell, ListEntry *listentry);

extern ItemCell *g_consoleItem;
extern ListManager *g_listManager;
extern PasteWindow *g_pasteWindow;

ItemStatusRec::ItemStatusRec(ItemCell *itemcell_in,
                             const list<ItemCell *> &item_list_in
                            ) : ItemStatus(itemcell_in)
{
    item_list = item_list_in;
    addPasteEnabled = false;
}

ItemStatusRec::ItemStatusRec(ItemCell *itemcell_in,
                             const list<ItemCell *> &item_list_in,
                             bool addPasteEnabled_in
                            ) : ItemStatus(itemcell_in)
{
    item_list = item_list_in;
    addPasteEnabled = addPasteEnabled_in;
}

ItemStatusRec::~ItemStatusRec()
{
}

void ItemStatusRec::Update()
{
    int count = 0;
    if (!g_listManager->Search(listentry)) {
        // delete all itemcell
        for (list<ItemCell *>::iterator itemcell_ptr = item_list.begin(); itemcell_ptr != item_list.end(); ++itemcell_ptr) {
            ItemCell *itemcell = *itemcell_ptr;
            delete itemcell;
        }
        return;
    }

    listentry->get_Dl_clist_lock();
    for (list<ItemCell *>::iterator itemcell_ptr = item_list.begin(); itemcell_ptr != item_list.end(); ++itemcell_ptr) {
        ItemCell *itemcell = *itemcell_ptr;
        if (!listentry->getItemManager()->search_by_url(itemcell->ret_URL_Container().ret_URL())) {
            ++count;
            if (addPasteEnabled) {
                // change the status of item
                // this is needed because default status is ItemCell::ITEM_READY
                itemcell->set_Status(ItemCell::ITEM_STOP);
                g_pasteWindow->addItem(itemcell);
            } else {
                Add_new_item_to_downloadlist(itemcell, listentry);
            }
        } else {
            delete itemcell;
        }
    }
    if (count) {
        if (addPasteEnabled) {
            g_pasteWindow->show();
        } else {
            g_consoleItem->Send_message_to_gui(itos(count) + _(" item(s) added"), MSG_SYS_INFO);
        }
    }
    listentry->Send_start_signal();
    listentry->release_Dl_clist_lock();
}
