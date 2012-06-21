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

// $Id: ThreadSlot.cc,v 1.5 2001/09/11 13:13:32 tujikawa Exp $

#include "ThreadSlot.h"

ThreadSlot::ThreadSlot(pthread_t* thread_id_ptr_in)
{
  thread_id_ptr = thread_id_ptr_in;
  status = THREAD_WAIT;
  itemcell = NULL;
  pthread_cond_init(&token_cond, NULL);
}

ThreadSlot::~ThreadSlot()
{
  delete thread_id_ptr;
  pthread_cond_destroy(&token_cond);
}

void ThreadSlot::set_Status(ThreadStatusType status_in)
{
  status = status_in;
}

pthread_cond_t *ThreadSlot::ret_token_cond()
{
  return &token_cond;
}

ThreadStatusType ThreadSlot::ret_Status() const {
  return status;
}

bool ThreadSlot::Is_equal_thread(pthread_t thread_id_in)
{
  if(pthread_equal(thread_id_in, *thread_id_ptr)) {
    return true;
  } else {
    return false;
  }
}

pthread_t ThreadSlot::ret_thread_id()
{
  return *thread_id_ptr;
}

void ThreadSlot::setItemCell(ItemCell *itemcell_in) {
  itemcell = itemcell_in;
}

ItemCell *ThreadSlot::getItemCell() const {
  return itemcell;
}
