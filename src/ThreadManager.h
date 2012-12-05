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

// $Id: ThreadManager.h,v 1.13 2001/11/04 10:18:08 tujikawa Exp $

// definition of class ThreadManager

#ifndef _THREADMANAGER_H_
#define _THREADMANAGER_H_

#include <list>
#include "aria.h"
#include "ItemManager.h"
#include "ThreadSlot.h"
using namespace std;

typedef list<ThreadSlot *> ThreadList;

class ThreadManager
{
private:
    int maxthread;
    bool halt_flag;
    bool autostart_flag;
    int n_thread_to_retire;
    ThreadList thread_list;//スレッド管理リスト
    pthread_mutex_t tm_lock;
    //ThreadSlot *token_thread;
    //struct timeval token_gettime;
    //struct timezone tz_dummy;
    //pthread_mutex_t token_lock;
    void *boss;
public:
    ThreadManager(int maxthread, void *boss_in);
    void setBoss(void *boss_in);
    void *getBoss() const;
    void ManageThread(int maxthread);
    void ManageThread();

    void waitThreadTermination(); // すべてのスレッドの終了を待つ
    int getTotalThread() const; // リストのスレッドの数を返す
    void retireThread(pthread_t thread_id); // 引数のスレッドをスレッドリスト
    // から削除
    bool retireThreadByRequest(pthread_t thread_id); //Whether_retire_is_requested()+Retire_thread(...)
    bool isNoActiveThread() const;
    void setThreadState(pthread_t thread_id, ItemCell *itemcell, ThreadStatusType status);
    bool getHaltFlag() const;
    void setHaltFlag();

    list<ItemCell *> getActiveItemCell();

    void setRetireNumber(int num_of_thread_to_retire);

    ThreadSlot *getThreadSlot(pthread_t thread);
    bool getAutostartFlag() const;
    void setAutostartFlag(bool flag);

    //bool get_token(pthread_t thread);
    //bool release_token(pthread_t thread);
    //ThreadSlot *get_next_token_thread(ThreadSlot *threadslot);
};
#endif // _THREADMANAGER_H_
