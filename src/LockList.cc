//
//  aria - yet another download tool
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

// $Id: LockList.cc,v 1.1 2001/04/08 12:06:32 tujikawa Exp $

#include "LockList.h"

LockList::LockList()
{
  pthread_mutex_init(&ll_lock, NULL);
}

LockList::~LockList()
{
  //cerr << "size of lock_list " << lock_list.size() << endl;
}

bool LockList::Try_lock(const string& filename)
{
  pthread_mutex_lock(&ll_lock);
  for(list<string>::iterator itr = lock_list.begin();
      itr != lock_list.end(); ++itr) {
    if(*itr == filename) {
      pthread_mutex_unlock(&ll_lock);
      return false;
    }
  }
  lock_list.push_back(filename);
  pthread_mutex_unlock(&ll_lock);
  return true;
}

bool LockList::Unlock(const string& filename)
{
  pthread_mutex_lock(&ll_lock);
  for(list<string>::iterator itr = lock_list.begin();
      itr != lock_list.end(); ++itr) {
    if(*itr == filename) {
      lock_list.remove(filename);
      pthread_mutex_unlock(&ll_lock);
      return true;
    }
  }
  pthread_mutex_unlock(&ll_lock);
  return false;
}
  
