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

// $Id: ItemManager.cc,v 1.14 2001/11/28 15:28:39 tujikawa Exp $

#include "ItemManager.h"

ItemManager::ItemManager()
{
    maxid = 1;
    pthread_mutex_init(&im_lock, NULL);
}

ItemManager::~ItemManager()
{
    pthread_mutex_destroy(&im_lock);
}

bool ItemManager::regist_item_back(ItemCell *itemcell)
{
    int id;
    if (search_item(itemcell)) return false;
    pthread_mutex_lock(&im_lock);
    if (reusable_id_list.size()) {
        id = reusable_id_list.front();
        reusable_id_list.pop_front();
    } else {
        id = maxid;
        ++maxid;
    }
    itemcell->set_id(id);
    used_id_list.push_back(id);
    item_id_map[id] = itemcell;
    reverse_item_id_map[itemcell] = id;
    pthread_mutex_unlock(&im_lock);

    return true;
}
/*
bool ItemManager::regist_item_next_to(ItemCell *itemcell_base, ItemCell *itemcell)
{
  int id;
  if(search_item(itemcell)) return false;
  pthread_mutex_lock(&im_lock);
  if(reusable_id_list.size()) {
    id = reusable_id_list.front();
    reusable_id_list.pop_front();
  } else {
    id = maxid;
    ++maxid;
  }
  itemcell->set_id(id);
  int id_base = reverse_item_id_map[itemcell_base];
  if(id_base == 0) {
    used_id_list.push_front(id);
  } else {
    bool flag = false;
    for(list<int>::iterator id_itr = used_id_list.begin();
	id_itr != used_id_list.end(); ++id_itr) {
      if(*id_itr == id_base) {
	++id_itr;
	used_id_list.insert(id_itr, id);
	flag = true;
	break;
      }
    }
    if(!flag) {
      used_id_list.push_front(id);
    }
  }
  item_id_map[id] = itemcell;
  reverse_item_id_map[itemcell] = id;
  pthread_mutex_unlock(&im_lock);

  return true;
}
*/
/*
bool ItemManager::regist_item_front(ItemCell *itemcell)
{

  int id;
  if(search_item(itemcell)) return false;
  pthread_mutex_lock(&im_lock);
  if(reusable_id_list.size()) {
    id = reusable_id_list.front();
    reusable_id_list.pop_front();
  } else {
    id = maxid;
    ++maxid;
  }
  itemcell->set_id(id);
  used_id_list.push_front(id);
  item_id_map[id] = itemcell;
  reverse_item_id_map[itemcell] = id;
  pthread_mutex_unlock(&im_lock);

  return true;
}
*/
bool ItemManager::unregist_item(ItemCell *itemcell)
{

    if (!search_item(itemcell)) return false;
    pthread_mutex_lock(&im_lock);
    int id = itemcell->ret_id();
    itemcell->set_id(-1);
    used_id_list.remove(id);
    reusable_id_list.push_back(id);
    //reverse_item_id_map[itemcell] = 0;
    reverse_item_id_map.erase(itemcell);//modified 2001/6/2
    item_id_map.erase(id);// added 2001/6/2
    pthread_mutex_unlock(&im_lock);

    return true;
}

bool ItemManager::search_item(ItemCell *itemcell)
{
    bool retval;
    pthread_mutex_lock(&im_lock);
    /*
    int id = itemcell->ret_id();
    if(used_id_list.end() == find(used_id_list.begin(), used_id_list.end(), id))
      retval =  false;
    else retval = true;
    */
    if (reverse_item_id_map[itemcell] == 0) {
        retval = false;
    } else {
        retval = true;
    }

    pthread_mutex_unlock(&im_lock);

    return retval;
}

bool ItemManager::search_by_url(const string &url)
{
    bool retval = false;
    pthread_mutex_lock(&im_lock);
    for (list<int>::iterator itr = used_id_list.begin(); itr != used_id_list.end(); ++itr) {
        if (item_id_map[*itr]->ret_URL_Container_opt().ret_URL() == url) {
            retval = true;
            break;
        }
    }
    pthread_mutex_unlock(&im_lock);

    return retval;
}

bool ItemManager::search_by_url_with_local_path(const string &url, const string &localPath)
{
    bool retval = false;
    pthread_mutex_lock(&im_lock);
    for (list<int>::iterator itr = used_id_list.begin(); itr != used_id_list.end(); ++itr) {
        string localPathItr = item_id_map[*itr]->ret_Options_opt().ret_Store_Dir()
                              + item_id_map[*itr]->ret_Filename_opt();
        if (localPathItr == localPath &&
                item_id_map[*itr]->ret_URL_Container_opt().ret_URL() == url) {
            retval = true;
            break;
        }
    }
    pthread_mutex_unlock(&im_lock);

    return retval;
}

const list<int> &ItemManager::ret_id_list() const
{
    return used_id_list;
}

ItemCell *ItemManager::ret_itemaddr(int id)
{
    return item_id_map[id];
}

/*
bool ItemManager::move_to_front(ItemCell *itemcell)
{
  if(!search_item(itemcell)) return false;
  pthread_mutex_lock(&im_lock);
  int id = itemcell->ret_id();
  used_id_list.remove(id);
  used_id_list.push_front(id);
  pthread_mutex_unlock(&im_lock);

  return true;
}
*/
/*
bool ItemManager::move_to(int index, ItemCell *itemcell)
{
  pthread_mutex_lock(&im_lock);

  int id = itemcell->ret_id();
  used_id_list.remove(id);
  list<int>::iterator id_itr = used_id_list.begin();
  for(int i = 0; i < index; ++i) ++id_itr;
  //cerr << index << endl;
  used_id_list.insert(id_itr, id);

  pthread_mutex_unlock(&im_lock);

  return true;
}
*/
/*
bool ItemManager::swap_item(ItemCell *itemcell_new, ItemCell *itemcell_old)
{
  if(!search_item(itemcell_old)) return false;
  pthread_mutex_lock(&im_lock);
  int id = itemcell_old->ret_id();
  itemcell_new->set_id(id);
  item_id_map[id] = itemcell_new;
  reverse_item_id_map[itemcell_old] = 0;
  reverse_item_id_map[itemcell_new] = id;
  pthread_mutex_unlock(&im_lock);
  //cerr << "in swap_item" << endl;
  return true;
}
*/
void ItemManager::all_clear()
{
    used_id_list.clear();
    item_id_map.clear();
    reverse_item_id_map.clear();
    reusable_id_list.clear();
    maxid = 1;
}
/*
ItemCell *ItemManager::Get_next_item()
{
  ItemCell* itemcell_next = NULL;
  //cerr << "in get next item" << endl;
  pthread_mutex_lock(&im_lock);
  const list<int>& download_list = ret_id_list();
  for(list<int>::const_iterator id_itr = download_list.begin(); id_itr != download_list.end(); ++id_itr) {
    ItemCell *itemcell = ret_itemaddr(*id_itr);
    if(itemcell->ret_Status() == ItemCell::ITEM_READY) {
      itemcell->Open_Desc();
      itemcell->set_Status(ItemCell::ITEM_INUSE);
      itemcell_next = itemcell;
      break;
    } else if(itemcell->ret_Status() == ItemCell::ITEM_READY_AGAIN) {
      itemcell->Open_Desc();
      itemcell->set_Status(ItemCell::ITEM_INUSE_AGAIN);
      itemcell_next = itemcell;
      break;
    } else if(itemcell->ret_Status() == ItemCell::ITEM_READY_CONCAT) {
      //cerr << "Item ready concat" << endl;
      itemcell->Open_Desc();
      itemcell->set_Status(ItemCell::ITEM_INUSE_CONCAT);
      itemcell_next = itemcell;
      break;
    }
  }
  pthread_mutex_unlock(&im_lock);
  return itemcell_next;
}
*/
