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

// $Id: Socket.h,v 1.11 2001/11/19 16:26:21 tujikawa Exp $

#ifndef __SOCKET_H__
#define __SOCKET_H__
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <cerrno>
#include <algorithm>
#include <string>
#include <list>

#include "SocketPool.h"

using namespace std;

enum SocketErrorType {
    SOCKET_EACCEPT
};

class Socket
{
private:
    int sockfd;
    int del_flag;
    string host;
    int port;
    string redirectedHost;
    int redirectedPort;
    bool pooledFlag;

#ifdef HAVE_OPENSSL
    SSL_CTX *ctx;
    SSL *ssl;
    X509 *serverCert;
#endif // HAVE_OPENSSL
protected:
    static SocketPool socketPool;
public:
    enum SocketModeType {
        DEFAULT,
        DUPE
    };

    Socket();
    Socket(int fd, SocketModeType mode);
    Socket(const Socket &socket);

    virtual ~Socket();

    static void refreshSocketPool();
    void setHostPort(const string &host, int port);
    void setRedirectedHostPort(const string &host, int port);
    SocketPoolCell *tryGetPooledSocket(const string &host, int port,
                                       const string &redirectedHost, int redirectedPort);
    int getPooledSocket(const string &host, int port);
    void addPooledSocket(const string &host, int port);

    Socket &operator=(const Socket &src);

    int create(int domain, int type, int protocol);
#ifdef HAVE_OPENSSL
    int enableSSL();
#endif // HAVE_OPENSSL
    bool reopen();
    int ret_Desc() const;
    int Getflags() const;
    int Setflags(int flags) const;
    void Shutdown(int mode);
    bool isPooledSocket();

    int Connect(const struct sockaddr_in &serv_addr) const;
    int Connect(struct addrinfo *res) const;
    int is_readready(int timeout, list<int> &rfds_list) const;
    int is_readready(struct timeval *tv_ptr, list<int> &rfds_list) const;

    int is_writeready(int timeout, list<int> &wfds_list) const;
    int is_writeready(struct timeval *tv_ptr, list<int> &wfds_list) const;

    int is_readwriteready(int timeout, list<int> &rfds_list, list<int> &wfds_list) const;
    int is_readwriteready(struct timeval *tv_ptr, list<int> &rfds_list, list<int> &wfds_list) const;

    static bool is_set(int fd, const list<int> &fds_list);

    bool is_set(const list<int> &fds_list) const;

    int Send(const string &command, int flags) const;
    int Send(const string &command) const;
    int Recv(void *buf, size_t len, int flags = 0) const;
    int Recvfrom(void *buf, size_t len, int from) const;

    int Getsockname(struct sockaddr_in &addr) const;
    int Bind(struct sockaddr_in &serv_addr) const;
    int Bind(struct addrinfo *res) const;
    int Listen(int i) const;
    int Listen() const;
    Socket Accept(struct sockaddr_in &clnt_addr) const;
    Socket Accept() const;
    bool check_error() const;
    bool bad() const;
};

#endif // __SOCKET_H__
