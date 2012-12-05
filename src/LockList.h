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

// $Id: LockList.h,v 1.2 2001/09/11 13:13:31 tujikawa Exp $

#ifndef _LOCKLIST_H_
#define _LOCKLIST_H_

#include <string>
#include <list>
#include "aria.h"

using namespace std;

class LockList
{
private:
    list<string> lock_list;
    pthread_mutex_t ll_lock;
public:
    LockList();
    ~LockList();

    bool Try_lock(const string &filename);
    bool Unlock(const string &filename);
};
#endif // _LOCKLIST_H_
