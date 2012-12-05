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

// $Id: ThreadSlot.h,v 1.6 2001/09/11 13:13:32 tujikawa Exp $

#ifndef _THREADSLOT_H_
#define _THREADSLOT_H_

#include <pthread.h>
#include "aria.h"
#include "ItemCell.h"

enum ThreadStatusType {
    THREAD_ACTIVE,
    THREAD_WAITTOKEN,
    THREAD_WAIT
};

class ThreadSlot
{
private:
    pthread_t *thread_id_ptr;
    ThreadStatusType status;
    pthread_cond_t token_cond;
    ItemCell *itemcell;
public:
    ThreadSlot(pthread_t *thread_id_ptr_in);
    ~ThreadSlot();

    void             set_Status(ThreadStatusType status_new);
    ThreadStatusType ret_Status() const;

    void             setItemCell(ItemCell *itemcell);
    ItemCell         *getItemCell() const;

    bool             Is_equal_thread(pthread_t thread_id_in);
    pthread_t        ret_thread_id();
    pthread_cond_t *ret_token_cond();
};

#endif // _THREADSLOT_H_
