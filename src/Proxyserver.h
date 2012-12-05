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

// $Id: Proxyserver.h,v 1.2 2001/02/22 09:36:51 tujikawa Exp $

#ifndef _PROXYSERVER_H_
#define _PROXYSERVER_H_

#include <string>
using namespace std;

// proxy server
class Proxyserver
{
private:
    string server;
    int port;
    friend class Options;
public:
    Proxyserver();
    Proxyserver(const string &server, int port);
    Proxyserver(const Proxyserver &entry);
    const string &ret_Server() const;
    int ret_Port() const;

    void set_Server(const string &server);
    void set_Port(int port);
};

#endif // _PROXYSERVER_H_
