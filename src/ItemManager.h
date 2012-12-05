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

// $Id: ItemManager.h,v 1.11 2001/11/28 15:28:39 tujikawa Exp $

#ifndef _ITEMMANAGER_H_
#define _ITEMMANAGER_H_

#include <list>
#include <map>
#include <iostream>
#include "aria.h"
#include "ItemCell.h"

using namespace std;

class ItemManager
{
private:
    list<int> used_id_list;
    map<int, ItemCell *> item_id_map;
    map<ItemCell *, int> reverse_item_id_map;

    list<int> reusable_id_list;
    int maxid;
    pthread_mutex_t im_lock;
public:
    ItemManager();
    ~ItemManager();

    ItemCell *Get_next_item();
    bool regist_item_back(ItemCell *itemcell);
    bool regist_item_front(ItemCell *itemcell);
    bool regist_item_next_to(ItemCell *itemcell_base, ItemCell *itemcell);
    bool unregist_item(ItemCell *itemcell);
    bool search_item(ItemCell *itemcell);
    bool search_by_url(const string &url);
    bool search_by_url_with_local_path(const string &url, const string &localPath);
    ItemCell *ret_itemaddr(int id);
    bool move_to_front(ItemCell *itemcell);
    bool move_to(int index_dest, ItemCell *itemcell);
    const list<int> &ret_id_list() const;
    bool swap_item(ItemCell *itemcell_new, ItemCell *itemcell_old);
    void all_clear();
};

#endif // _ITEMMANAGER_H_
