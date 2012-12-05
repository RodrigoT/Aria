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

// $Id: ProxyList.h,v 1.4 2001/10/01 12:45:26 tujikawa Exp $

#ifndef _PROXYLIST_H_
#define _PROXYLIST_H_

#include <iostream>
#include <list>
#include <fstream>
#include "Proxyserver.h"
#include "utils.h"

using namespace std;

class ProxyList
{
private:
    list<Proxyserver *> proxy_list;
    string filename;
public:
    ProxyList();
    ProxyList(list<Proxyserver> initial_proxy_list);
    ProxyList(const ProxyList &proxylist_src);
    ~ProxyList();

    bool search(const Proxyserver &entry);
    void clear();
    bool add(const Proxyserver &new_entry);
    bool remove(const Proxyserver &entry);
    bool set(const ProxyList &new_list);
    const list<Proxyserver *> &ret_list() const;

    bool Save_proxy_list(const string &filename);
    bool Read_proxy_list(const string &filename);
};

#endif // _PROXYLIST_H_
