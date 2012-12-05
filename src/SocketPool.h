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

// $Id: SocketPool.h,v 1.2 2001/11/19 16:26:21 tujikawa Exp $

#ifndef _SOCKETPOOL_H_
#define _SOCKETPOOL_H_
#include <iostream>
#include <vector>
#include <string>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include "SocketPoolCell.h"

using namespace std;

class SocketPool
{
private:
    vector<SocketPoolCell *> socketPoolCellList;
    pthread_mutex_t socketPoolCellListLock;

    // search a socket associated to host:port
    vector<SocketPoolCell *>::iterator searchPooledSocket(const string &host, int port);
    vector<SocketPoolCell *>::iterator searchPooledSocket(int socket);
public:
    SocketPool();
    ~SocketPool();

    void getSocketPoolLock();
    void releaseSocketPoolLock();
    // add a socket to the list
    void addSocketPoolCell(int socket, const string &host, int port);
    void addSocketPoolCell(int socket, const string &host, int port,
                           const string &redirectedHost, int redirectedPort
#ifdef HAVE_OPENSSL
                           , SSL_CTX *ctx,
                           SSL *ssl
#endif // HAVE_OPENSSL
                          );
    // delete specified socket form the list
    bool deleteSocketPoolCell(int socket, const string &host, int port);
    bool deleteSocketPoolCell(int socket);
    // delete all sockets from the list
    void deleteAllSocketPoolCell();
    // get the socket associated to host:port
    SocketPoolCell *getPooledSocket(const string &host, int port);
    SocketPoolCell *getPooledSocket(const string &host, int port,
                                    const string &redirectedHost,
                                    int redirectedPort);
    void returnPooledSocket(int socket);
    void refresh();
};
#endif // _SOCKETPOOL_H_
