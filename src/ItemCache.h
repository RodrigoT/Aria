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

// $Id: ItemCache.h,v 1.2 2001/09/11 13:13:31 tujikawa Exp $

#ifndef _ITEMCACHE_H_
#define _ITEMCACHE_H_

#include <list>
#include "ListEntry.h"

using namespace std;

class ItemCache
{
private:
    list<ItemCell *> item_list;
public:
    ItemCache();
    ~ItemCache();

    void add_Cache_by_copy(ItemCell *itemcell);
    void add_Cache_by_cut(ItemCell *itemcell);
    void clear_Cache();
    int ret_Cache_Total();
    void paste_Cache(ListEntry *listentry);
};
#endif //_ITEMCACHE_H_
