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

// $Id: ThreadManager.cc,v 1.19 2002/09/30 13:29:46 tujikawa Exp $

// implemantation of class ThreadManager

#include "ThreadManager.h"
#include "ListEntry.h"

extern void* Download_thread_main(ListEntry *listentry);

// コンストラクタ
ThreadManager::ThreadManager(int maxthread_in, void *boss_in)
{
  maxthread = maxthread_in;
  halt_flag = false;
  n_thread_to_retire = 0;
  boss = boss_in;

  autostart_flag = false;
  //token_thread = NULL;
  pthread_mutex_init(&tm_lock, NULL);
  //pthread_mutex_init(&token_lock, NULL);
//    token_gettime.tv_sec = 0;
//    token_gettime.tv_usec = 0;
}

void ThreadManager::setBoss(void *boss_in)
{
  boss = boss_in;
}

// Change thread state
//
// if thread is downloading any item, its state is THREAD_ACTIVE.
// if thread is waiting the signal(wait state), its state is THREAD_WAIT.
void ThreadManager::setThreadState(pthread_t thread_id,
				   ItemCell *itemcell,
				   ThreadStatusType thread_status)
{
  pthread_mutex_lock(&tm_lock);
  ThreadList::iterator tl_itr;
  for(tl_itr = thread_list.begin(); tl_itr != thread_list.end(); ++tl_itr) {
    if((*tl_itr)->Is_equal_thread(thread_id)) {
      (*tl_itr)->set_Status(thread_status);
      (*tl_itr)->setItemCell(itemcell);
      break;
    }
  }
  pthread_mutex_unlock(&tm_lock);
}

ThreadSlot *ThreadManager::getThreadSlot(pthread_t thread)
{
  ThreadSlot *retval = NULL;
  pthread_mutex_lock(&tm_lock);
  for(ThreadList::const_iterator tl_itr = thread_list.begin(); tl_itr != thread_list.end(); ++tl_itr) {
    if((*tl_itr)->ret_thread_id() == thread) {
      retval = *tl_itr;
    }
  }
  pthread_mutex_unlock(&tm_lock);
  return retval;
}

list<ItemCell *> ThreadManager::getActiveItemCell() {
  pthread_mutex_lock(&tm_lock);
  list<ItemCell *> itemCellList;
  for(ThreadList::const_iterator tl_itr = thread_list.begin(); tl_itr != thread_list.end(); ++tl_itr) {
    itemCellList.push_back((*tl_itr)->getItemCell());
  }  
  pthread_mutex_unlock(&tm_lock);

  return itemCellList;
}

/*
bool ThreadManager::get_token(pthread_t thread)
{
  ThreadSlot *threadslot = get_threadslot(thread);
  threadslot->set_Status(THREAD_WAITTOKEN);
  //cerr << "thread " << threadslot->ret_thread_id() << " change state to WAITTOKEN" << endl;
  pthread_mutex_lock(&token_lock);
  while(1) {
    struct timeval curtime;
    gettimeofday(&curtime, &tz_dummy);
    if(token_thread == threadslot) {
      //token_gettime = time(NULL);
      gettimeofday(&token_gettime, &tz_dummy);
      //cerr << "thread " << threadslot->ret_thread_id() << " get token" << endl;
      return true;
      //} else if(time(NULL)-token_gettime > 1) {
    } else if(curtime.tv_sec-token_gettime.tv_sec > 1 ||
	      curtime.tv_usec-token_gettime.tv_usec > 8000000) {
      gettimeofday(&token_gettime, &tz_dummy);
      token_thread = threadslot;
      //cerr << "timedout" << endl;
      //cerr << "thread " << threadslot->ret_thread_id() << " get token" << endl;
      return true;
    } else {
      //cerr << "wait token" << endl;
      //cerr << "thread " << threadslot->ret_thread_id() << " waiting signal" << endl;
      pthread_cond_wait(threadslot->ret_token_cond(), &token_lock);
      //cerr << "thread " << threadslot->ret_thread_id() << " received signal" << endl;
    }
  }
}

bool ThreadManager::release_token(pthread_t thread)
{
  ThreadSlot *threadslot = get_threadslot(thread);

  if(threadslot != token_thread) return false;

  token_thread = get_next_token_thread(token_thread);
  //cerr << "thread " << threadslot->ret_thread_id() << " release token, next is " << token_thread->ret_thread_id() << endl;
  pthread_cond_signal(token_thread->ret_token_cond());
  pthread_mutex_unlock(&token_lock);
  threadslot->set_Status(THREAD_ACTIVE);
  return true;
}

ThreadSlot *ThreadManager::get_next_token_thread(ThreadSlot *threadslot)
{
  ThreadSlot *retval = NULL;
  pthread_mutex_lock(&tm_lock);

  ThreadList::const_iterator base_itr = find(thread_list.begin(), thread_list.end(), threadslot);

  for(ThreadList::const_iterator tl_itr = base_itr; tl_itr != thread_list.end(); ++tl_itr) {
    if((*tl_itr)->ret_Status() == THREAD_WAITTOKEN && tl_itr != base_itr) {
      retval = *tl_itr;
    }
  }
  if(retval == NULL) {
    for(ThreadList::const_iterator tl_itr = thread_list.begin(); tl_itr != base_itr; ++tl_itr) {
      if((*tl_itr)->ret_Status() == THREAD_WAITTOKEN) {
	retval = *tl_itr;
      }
    }
  }
  if(retval == NULL) {
    retval = threadslot;
  }

  pthread_mutex_unlock(&tm_lock);
  return retval;
}
*/

bool ThreadManager::isNoActiveThread() const
{
  for(ThreadList::const_iterator tl_itr = thread_list.begin(); tl_itr != thread_list.end(); ++tl_itr) {
    if((*tl_itr)->ret_Status() == THREAD_ACTIVE || (*tl_itr)->ret_Status() == THREAD_WAITTOKEN) {
      return false;
    }
  }
  return true;
}

void ThreadManager::ManageThread()
{
  ManageThread(maxthread);
}

// スレッドの数を動的に増減する
void ThreadManager::ManageThread(int maxthread_in)
{  
  pthread_mutex_lock(&tm_lock);
  int diff = maxthread_in-thread_list.size();

  //cerr << maxthread_in << " " << n_thread_to_retire << " " << thread_list.size() << endl;
  autostart_flag = !isNoActiveThread();
  maxthread = maxthread_in;
  if(diff > 0) {
    n_thread_to_retire = 0;
    // increase the total number of thread by diff
    // diff個だけスレッドを増加させる
    //cerr << "create" << diff << endl;
    for(int i = 0; i < diff; i++) {
      pthread_t* thread_ptr = new pthread_t;
      pthread_create(thread_ptr, (pthread_attr_t*)NULL,
		     (void*(*)(void*))Download_thread_main,
		     (void*)boss);
      //pthread_detach(*thread_ptr);
      // create thread entry
      ThreadSlot* thread_slot_ptr = new ThreadSlot(thread_ptr);
      // end register it to thread management list
      thread_list.push_back(thread_slot_ptr);
    }
  } else if(diff < 0) {
    //cerr << "delete" << -diff << endl;
    // decrease the total number of thread by abs(diff)
    // diff個だけスレッドを減少させる
    //set_retire_number(-diff);
    n_thread_to_retire = -diff;
  } else {
    n_thread_to_retire = 0;
  }
  pthread_mutex_unlock(&tm_lock);
}

// Wait the other threads terminate
// その他のすべてのスレッドが終了するのを待つ
void ThreadManager::waitThreadTermination()
{
  for(ThreadList::iterator tl_itr = thread_list.begin(); tl_itr != thread_list.end(); ++tl_itr) {
    //pthread_cancel((*tl_itr)->ret_thread_id() );
    pthread_join((*tl_itr)->ret_thread_id(), NULL);
  }
}

// Return the number of currently running thread
// 現在のスレッドの数を返す
int ThreadManager::getTotalThread() const
{
  return thread_list.size();
}

// delete thread entry whose id is thread_id from thread_list
// thread_list is thread management list
// 引数thread_idのスレッドエントリをスレッド管理リストから削除する
void ThreadManager::retireThread(pthread_t thread_id)
{
  pthread_mutex_lock(&tm_lock);
  --n_thread_to_retire;
  //cerr << "retire" << endl;
  ThreadList::iterator tl_itr;
  for(tl_itr = thread_list.begin(); tl_itr != thread_list.end(); ++tl_itr) {
    if((*tl_itr)->Is_equal_thread(thread_id)) {
      // unresiter ThreadSlot *tl_itr
      thread_list.remove(*tl_itr);
      // and delete it
      delete *tl_itr;
      pthread_detach(thread_id);
      break;
    }
  }
  pthread_mutex_unlock(&tm_lock);
}

bool ThreadManager::retireThreadByRequest(pthread_t thread_id)
{
  bool retval;
  pthread_mutex_lock(&tm_lock);
  if(n_thread_to_retire > 0) {
    retval = true;
    --n_thread_to_retire;
    ThreadList::iterator tl_itr;
    for(tl_itr = thread_list.begin(); tl_itr != thread_list.end(); ++tl_itr) {
      if((*tl_itr)->Is_equal_thread(thread_id)) {
	// unresiter ThreadSlot *tl_itr
	thread_list.remove(*tl_itr);
	// and delete it
	delete *tl_itr;
	pthread_detach(thread_id);
	break;
      }
    }
  } else {
    // no retire requested
    retval = false;
  }
  pthread_mutex_unlock(&tm_lock);
  return retval;
}

bool ThreadManager::getHaltFlag() const
{
  return halt_flag;
}

void ThreadManager::setHaltFlag()
{
  halt_flag = true;
}

//  bool ThreadManager::Whether_retire_is_required()
//  {
//    bool retval;
//    pthread_mutex_lock(&tm_lock);
//    if(n_thread_to_retire > 0) {
//      retval = true;
//    } else {
//      retval = false;
//    }
//    pthread_mutex_unlock(&tm_lock);
//    return retval;
//  }

void ThreadManager::setRetireNumber(int num)
{
  //cerr << num << endl;
  n_thread_to_retire = num;
}

bool ThreadManager::getAutostartFlag() const
{
  return autostart_flag;
}

void ThreadManager::setAutostartFlag(bool flag)
{
  autostart_flag = flag;
}

