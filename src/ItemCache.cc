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

// $Id: ItemCache.cc,v 1.6 2001/11/28 15:28:38 tujikawa Exp $

#include "ItemCache.h"
#include "AppOption.h"

extern AppOption *g_appOption;

void Add_new_item_to_downloadlist(ItemCell *itemcell, ListEntry *listentry);

ItemCache::ItemCache()
{
}
 
ItemCache::~ItemCache()
{
  clear_Cache();
}

ItemCell::ItemStatusType STATUS_ADJUST(ItemCell::ItemStatusType status) {
  ItemCell::ItemStatusType retval = status;
  switch(status) {
  case ItemCell::ITEM_COMPLETE:
  case ItemCell::ITEM_LOCK:
  case ItemCell::ITEM_ERROR:
    break;
  default:
    retval = ItemCell::ITEM_STOP;
    break;
  }

  return retval;
}

void ItemCache::add_Cache_by_copy(ItemCell *itemcell_in)
{
  ItemCell *itemcell = new ItemCell(itemcell_in->ret_URL_opt(),
				    itemcell_in->ret_URL_Container_opt(),
				    itemcell_in->ret_Options_opt(),
				    _("Created"));
  itemcell->set_Filename(itemcell_in->ret_Filename_opt());
  itemcell->set_Filename_opt(itemcell_in->ret_Filename_opt());

  itemcell->set_documentroot_dir(itemcell_in->ret_documentroot_dir());
  itemcell->set_root_url(itemcell_in->ret_root_url());
  itemcell->set_Status(STATUS_ADJUST(itemcell_in->ret_Status()));
  itemcell->set_Size_Current(itemcell_in->ret_Size_Current());
  itemcell->set_Size_Total(itemcell_in->ret_Size_Total());
  itemcell->set_CRC_Type(itemcell_in->ret_CRC_Type());
  itemcell->set_CRC(itemcell_in->ret_CRC());

  item_list.push_back(itemcell);
}

void ItemCache::add_Cache_by_cut(ItemCell *itemcell_in)
{
  item_list.push_back(itemcell_in);
}

void ItemCache::clear_Cache()
{
  for(list<ItemCell *>::iterator itemcell_itr = item_list.begin();
      itemcell_itr != item_list.end(); ++itemcell_itr) {
    delete *itemcell_itr;
  }
  item_list.clear();  
}

void ItemCache::paste_Cache(ListEntry *listentry)
{  
  for(list<ItemCell *>::iterator itemcell_itr = item_list.begin();
      itemcell_itr != item_list.end(); ++itemcell_itr) {

    ItemCell *itemcell_in = *itemcell_itr;

    ItemCell *itemcell = new ItemCell(itemcell_in->ret_URL_opt(),
				      itemcell_in->ret_URL_Container_opt(),
				      itemcell_in->ret_Options_opt(),
				      _("Copied"));
    itemcell->set_Filename(itemcell_in->ret_Filename_opt());
    itemcell->set_Filename_opt(itemcell_in->ret_Filename_opt());
    
    itemcell->set_documentroot_dir(itemcell_in->ret_documentroot_dir());
    itemcell->set_root_url(itemcell_in->ret_root_url());
    itemcell->set_Status(itemcell_in->ret_Status());// mod 2001/8/1
    itemcell->set_Size_Current(itemcell_in->ret_Size_Current());
    itemcell->set_Size_Total(itemcell_in->ret_Size_Total());
    itemcell->set_CRC_Type(itemcell_in->ret_CRC_Type());
    itemcell->set_CRC(itemcell_in->ret_CRC());
    // modified 2001/8/15
    if(g_appOption->Whether_use_automatic_start() &&
       STATUS_ADJUST(itemcell->ret_Status()) == ItemCell::ITEM_STOP) {
      itemcell->set_Status(ItemCell::ITEM_READY);
    }
    Add_new_item_to_downloadlist(itemcell, listentry);
  }
}

int ItemCache::ret_Cache_Total()
{
  return item_list.size();
}
