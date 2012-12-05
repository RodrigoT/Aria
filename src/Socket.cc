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

// $Id: Socket.cc,v 1.14 2002/10/01 15:32:00 tujikawa Exp $

#include "Socket.h"
#include "aria.h"

#define DEBUG 1
#undef DEBUG

// static member variables
SocketPool Socket::socketPool;

Socket::Socket()
{
    // make a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd >= 0) {
        socklen_t sockopt = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(int)) < 0) {
            close(sockfd);
            sockfd = -1;
        }
    }
    pooledFlag = false;
#ifdef HAVE_OPENSSL
    ctx = NULL;
    ssl = NULL;
    serverCert = NULL;
#endif // HAVE_OPENSSL
}

Socket::Socket(const Socket &src)
{
    // fix this
    pooledFlag = false;
    sockfd = dup(src.ret_Desc());

#ifdef HAVE_OPENSSL
    ctx = NULL;
    ssl = NULL;
    serverCert = NULL;
#endif // HAVE_OPENSSL
}

Socket::Socket(int sockfd_in, SocketModeType mode)
{
    // fix this
    pooledFlag = false;
    switch (mode) {
        case DUPE:
            sockfd = dup(sockfd_in);
            break;
        case DEFAULT:
        default:
            sockfd = sockfd_in;
            break;
    }
#ifdef HAVE_OPENSSL
    ctx = NULL;
    ssl = NULL;
    serverCert = NULL;
#endif // HAVE_OPENSSL
}

Socket::~Socket()
{
    if (sockfd >= 0) {
        //shutdown(sockfd, 2);
        if (pooledFlag) {
            if (host.size()) {
                socketPool.returnPooledSocket(sockfd);
#ifdef DEBUG
                cerr << "socket returned: " << host << ':' << port << ", socket: " << sockfd << endl;
#endif // DEBUG
            } else {
#ifdef DEBUG
                cerr << "delete no name socket: " << host << ':' << port << ", socket: " << sockfd << endl;
#endif // DEBUG
                socketPool.deleteSocketPoolCell(sockfd);
            }
            //pooledFlag = false;
        } else if (host.size()) {
            fd_set rfds;
            fd_set wfds;
            struct timeval tv;
            int retval;

            FD_ZERO(&rfds);
            FD_ZERO(&wfds);
            FD_SET(sockfd, &rfds);
            FD_SET(sockfd, &wfds);
            tv.tv_sec = 0;
            tv.tv_usec = 0;
            retval = select(sockfd + 1, &rfds, &wfds, NULL, &tv);
            if (retval
                    && !FD_ISSET(sockfd, &rfds)
                    && FD_ISSET(sockfd, &wfds)) {

                socketPool.addSocketPoolCell(sockfd, host, port,
                                             redirectedHost,
                                             redirectedPort
#ifdef HAVE_OPENSSL
                                             , ctx, ssl
#endif // HAVE_OPENSSL
                                            );

#ifdef DEBUG
                cerr << "socket pooled: " << host << ':' << port << ", socket: " << sockfd << endl;
#endif // DEBUG
            } else {
#ifdef HAVE_OPENSSL
                if (ssl != NULL) {
                    SSL_shutdown(ssl);
                }
#endif
                close(sockfd);
#ifdef HAVE_OPENSSL
                if (ssl != NULL) {
                    SSL_free(ssl);
                    SSL_CTX_free(ctx);
                }
#endif
            }
        } else {
#ifdef HAVE_OPENSSL
            if (ssl != NULL) {
                SSL_shutdown(ssl);
            }
#endif
            close(sockfd);
#ifdef HAVE_OPENSSL
            if (ssl != NULL) {
                SSL_free(ssl);
                SSL_CTX_free(ctx);
            }
#endif
        }
    }
}

#ifdef HAVE_OPENSSL
int Socket::enableSSL()
{
    if (ssl == NULL) {
        ctx = SSL_CTX_new(SSLv23_client_method());
        ssl = SSL_new(ctx);
        SSL_set_fd(ssl, sockfd);
        return SSL_connect(ssl);
    } else {
        return 1;
    }
}
#endif

bool Socket::isPooledSocket()
{
    return pooledFlag;
}

void Socket::setHostPort(const string &ht, int pt)
{
    host = ht;
    port = pt;
}

void Socket::setRedirectedHostPort(const string &ht, int pt)
{
    redirectedHost = ht;
    redirectedPort = pt;
}

SocketPoolCell *Socket::tryGetPooledSocket(const string &host_in, int port_in, const string &redirectedHost_in, int redirectedPort_in)
{
#ifdef DEBUG
    cerr << "in tryGetPooledSocket()" << endl;
#endif // DEBUG
    SocketPoolCell *socketPoolCell = socketPool.getPooledSocket(host_in, port_in, redirectedHost_in, redirectedPort_in);
    if (socketPoolCell != NULL && socketPoolCell->getSocket() > 0) {
#ifdef DEBUG
        cerr << "socketTemp is " << socketPoolCell->getSocket() << endl;
#endif // DEBUG
        if (sockfd >= 0) {
#ifdef HAVE_OPENSSL
            if (ssl != NULL) {
                SSL_shutdown(ssl);
            }
#endif // HAVE_OPENSSL
            close(sockfd);
#ifdef HAVE_OPENSSL
            if (ssl != NULL) {
                SSL_free(ssl);
                SSL_CTX_free(ctx);
            }
#endif // HAVE_OPENSSL
        }
        pooledFlag = true;
        sockfd = socketPoolCell->getSocket();
        host = host_in;
        port = port_in;
        redirectedHost = redirectedHost_in;
        redirectedPort = redirectedPort_in;
#ifdef HAVE_OPENSSL
        ctx = socketPoolCell->getSSLCTX();
        ssl = socketPoolCell->getSSL();
#endif // HAVE_OPENSSL
    }
    return socketPoolCell;
}

void Socket::refreshSocketPool()
{
    socketPool.refresh();
}

int Socket::create(int domain, int type, int protocol)
{
    if (sockfd >= 0) {
        close(sockfd);
    }
    sockfd = socket(domain, type, protocol);
    if (sockfd >= 0) {
        socklen_t sockopt = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(int)) < 0) {
            close(sockfd);
            sockfd = -1;
        }
    }
    return sockfd;
}

Socket &Socket::operator=(const Socket &src)
{
    if (&src != this) {
        if (sockfd >= 0) {
            close(sockfd);
        }
        sockfd = dup(src.sockfd);
        del_flag = src.del_flag;
#ifdef HAVE_OPENSSL
        ctx = src.ctx;
        ssl = src.ssl;
#endif // HAVE_OPENSSL
    }
    return *this;
}

bool Socket::bad() const
{
    if (sockfd < 0) return true;
    else return false;
}

int Socket::ret_Desc() const
{
    return sockfd;
}

bool Socket::reopen()
{
    if (sockfd >= 0) {
        close(sockfd);
    }
    // make a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd >= 0) {
        socklen_t sockopt = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(int)) < 0) {
            close(sockfd);
            sockfd = -1;
            return false;
        }
    }
    return true;
}

int Socket::Getflags() const
{
    return fcntl(sockfd, F_GETFL, 0);
}

int Socket::Setflags(int flags) const
{
    return fcntl(sockfd, F_SETFL, flags);
}

void Socket::Shutdown(int mode)
{
#ifdef HAVE_OPENSSL
    if (ssl != NULL) {
        SSL_shutdown(ssl);
    }
#endif
    shutdown(sockfd, mode);
    if (pooledFlag) {
        socketPool.deleteSocketPoolCell(sockfd);
    } else {
        close(sockfd);
    }
    sockfd = -1;
#ifdef HAVE_OPENSSL
    if (ssl != NULL) {
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        ssl = NULL;
        ctx = NULL;
    }
#endif // HAVE_OPENSSL
}

int Socket::Connect(const struct sockaddr_in &serv_addr) const
{
    return connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
}

int Socket::Connect(struct addrinfo *res) const
{
    return connect(sockfd, res->ai_addr, res->ai_addrlen);
}

// if there is an error, return true;
bool Socket::check_error() const
{
    int retval;
    int arglen = sizeof(int);
    if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &retval, (socklen_t *)&arglen) < 0) {
        return true;
    }
    if (retval > 0) {
        errno = retval;
        return true;
    } else {
        return false;
    }
}

// wrapper function for is_readready
// this function ignores microsecond part of time out.
int Socket::is_readready(int timeout, list<int> &rfds_list) const
{
    struct timeval tv;
    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    return is_readready(&tv, rfds_list);
}


int Socket::is_readready(struct timeval *tv_ptr, list<int> &rfds_list) const
{
    fd_set rfds;
    int maxfd = sockfd;
    FD_ZERO(&rfds);
    FD_SET(sockfd, &rfds);

    for (list<int>::iterator itr = rfds_list.begin(); itr != rfds_list.end(); ++itr) {

        FD_SET(*itr, &rfds);
        if (maxfd < *itr) maxfd = *itr;
    }

    struct timeval tvStart;
    struct timeval tvEnd;
    struct timezone tz;
    gettimeofday(&tvStart, &tz);

    int retval = select(maxfd + 1, &rfds, NULL, NULL, tv_ptr);

    list<int> rfds_list_temp = rfds_list;
    rfds_list.clear();
    if (retval > 0) {
        for (list<int>::iterator itr = rfds_list_temp.begin(); itr != rfds_list_temp.end(); ++itr) {
            if (FD_ISSET(*itr, &rfds)) {
                rfds_list.push_back(*itr);
            }
        }
        if (FD_ISSET(sockfd, &rfds)) {
            rfds_list.push_back(sockfd);
        }
    }

    gettimeofday(&tvEnd, &tz);
    if (tv_ptr != NULL) {
        tv_ptr->tv_sec = tvEnd.tv_sec - tvStart.tv_sec;
        if (tvEnd.tv_usec < tvStart.tv_usec) {
            --(tv_ptr->tv_sec);
            tv_ptr->tv_usec = tvEnd.tv_usec + 1000000 - tvStart.tv_usec;
        } else {
            tv_ptr->tv_usec = tvEnd.tv_usec - tvStart.tv_usec;
        }

    }

    return retval;
}

bool Socket::is_set(const list<int> &fds_list) const
{
    if (fds_list.end() != find(fds_list.begin(), fds_list.end(), sockfd)) {
        return true;
    } else {
        return false;
    }
}

bool Socket::is_set(int fd, const list<int> &fds_list)
{
    if (fds_list.end() != find(fds_list.begin(), fds_list.end(), fd)) {
        return true;
    } else {
        return false;
    }
}

int Socket::is_readwriteready(int timeout, list<int> &rfds_list, list<int> &wfds_list) const
{
    struct timeval tv;
    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    return is_readwriteready(&tv, rfds_list, wfds_list);
}

int Socket::is_readwriteready(struct timeval *tv_ptr, list<int> &rfds_list, list<int> &wfds_list) const
{
    int maxfd = sockfd;
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(sockfd, &rfds);
    for (list<int>::iterator itr = rfds_list.begin(); itr != rfds_list.end(); ++itr) {
        FD_SET(*itr, &rfds);
        if (maxfd < *itr) maxfd = *itr;
    }
    fd_set wfds;
    FD_ZERO(&wfds);
    FD_SET(sockfd, &wfds);
    for (list<int>::iterator itr = wfds_list.begin(); itr != wfds_list.end(); ++itr) {
        FD_SET(*itr, &wfds);
        if (maxfd < *itr) maxfd = *itr;
    }

    struct timeval tvStart;
    struct timeval tvEnd;
    struct timezone tz;
    gettimeofday(&tvStart, &tz);

    int retval = select(maxfd + 1, &rfds, &wfds, NULL, tv_ptr);

    list<int> rfds_list_temp = rfds_list;
    list<int> wfds_list_temp = wfds_list;
    rfds_list.clear();
    wfds_list.clear();
    if (retval > 0) {
        for (list<int>::iterator itr = rfds_list_temp.begin(); itr != rfds_list_temp.end(); ++itr) {
            if (FD_ISSET(*itr, &rfds)) {
                rfds_list.push_back(*itr);
            }
        }
        for (list<int>::iterator itr = wfds_list_temp.begin(); itr != wfds_list_temp.end(); ++itr) {
            if (FD_ISSET(*itr, &wfds)) {
                wfds_list.push_back(*itr);
            }
        }
        if (FD_ISSET(sockfd, &rfds)) {
            rfds_list.push_back(sockfd);
        }
        if (FD_ISSET(sockfd, &wfds)) {
            wfds_list.push_back(sockfd);
        }
    }

    gettimeofday(&tvEnd, &tz);
    if (tv_ptr != NULL) {
        tv_ptr->tv_sec = tvEnd.tv_sec - tvStart.tv_sec;
        if (tvEnd.tv_usec < tvStart.tv_usec) {
            --(tv_ptr->tv_sec);
            tv_ptr->tv_usec = tvEnd.tv_usec + 1000000 - tvStart.tv_usec;
        } else {
            tv_ptr->tv_usec = tvEnd.tv_usec - tvStart.tv_usec;
        }

    }

    return retval;
}

int Socket::is_writeready(int timeout, list<int> &wfds_list) const
{
    struct timeval tv;
    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    return is_writeready(&tv, wfds_list);
}

int Socket::is_writeready(struct timeval *tv_ptr, list<int> &wfds_list) const
{
    fd_set wfds;
    FD_ZERO(&wfds);
    FD_SET(sockfd, &wfds);

    int maxfd = sockfd;
    for (list<int>::iterator itr = wfds_list.begin(); itr != wfds_list.end(); ++itr) {
        FD_SET(*itr, &wfds);
        if (maxfd < *itr) maxfd = *itr;
    }

    struct timeval tvStart;
    struct timeval tvEnd;
    struct timezone tz;
    gettimeofday(&tvStart, &tz);

    int retval = select(maxfd + 1, NULL, &wfds, NULL, tv_ptr);

    list<int> wfds_list_temp = wfds_list;
    wfds_list.clear();
    if (retval > 0) {
        for (list<int>::iterator itr = wfds_list_temp.begin(); itr != wfds_list_temp.end(); ++itr) {
            if (FD_ISSET(*itr, &wfds)) {
                wfds_list.push_back(*itr);
            }
        }
        if (FD_ISSET(sockfd, &wfds)) {
            wfds_list.push_back(sockfd);
        }
    }

    gettimeofday(&tvEnd, &tz);
    if (tv_ptr != NULL) {
        tv_ptr->tv_sec = tvEnd.tv_sec - tvStart.tv_sec;
        if (tvEnd.tv_usec < tvStart.tv_usec) {
            --(tv_ptr->tv_sec);
            tv_ptr->tv_usec = tvEnd.tv_usec + 1000000 - tvStart.tv_usec;
        } else {
            tv_ptr->tv_usec = tvEnd.tv_usec - tvStart.tv_usec;
        }

    }

    return retval;
}

int Socket::Send(const string &command, int flags) const
{
#ifdef HAVE_OPENSSL
    if (ssl != NULL) {
        return SSL_write(ssl, command.c_str(), command.length());
    } else
#endif // HAVE_OPENSSL
        return send(sockfd, command.c_str(), command.length(), flags);
}

int Socket::Send(const string &command) const
{
    int flags = 0;//MSG_NOSIGNAL;
    return Send(command, flags);
}

int Socket::Recv(void *buf, size_t len, int flags) const
{
    flags = flags;//|MSG_NOSIGNAL;
#ifdef HAVE_OPENSSL
    if (ssl != NULL) {
        if (flags & MSG_PEEK) {
            return SSL_peek(ssl, (char *)buf, len);
        } else {
            return SSL_read(ssl, (char *)buf, len);
        }
    } else
#endif // HAVE_OPENSSL
        return recv(sockfd, buf, len, flags);
}

int Socket::Recvfrom(void *buf, size_t len, int from) const
{
    struct sockaddr_in addr_in;
    if (Getsockname(addr_in) < 0) {
        return -1;
    }
    int addrlen = sizeof(addr_in);
    int flags = 0;//MSG_NOSIGNAL;

    return recvfrom(sockfd, buf, len, flags, (struct sockaddr *)&addr_in, (socklen_t *)&addrlen);
}


int Socket::Getsockname(struct sockaddr_in &addr) const
{
    int len = sizeof(struct sockaddr);
    return getsockname(sockfd, (struct sockaddr *)&addr, (socklen_t *)&len);
}

int Socket::Bind(struct sockaddr_in &addr) const
{
    return bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));
}

int Socket::Bind(struct addrinfo *res) const
{
    /*
    switch(res->ai_family) {
    case AF_INET:
      sa_len = sizeof(struct sockaddr_in);
      break;
    case AF_INET6:
      sa_len = sizeof(struct sockaddr_in6);
      break;
    default:
      return -1;
    }
    */
    return bind(sockfd, res->ai_addr, res->ai_addrlen);
}

int Socket::Listen(int i) const
{
    return listen(sockfd, 1);
}

int Socket::Listen() const
{
    return listen(sockfd, 1);
}

Socket Socket::Accept(struct sockaddr_in &addr) const
{
    int len = sizeof(struct sockaddr);
    int fd = accept(sockfd, (struct sockaddr *)&addr, (socklen_t *)&len);
    if (fd < 0) {
        throw SOCKET_EACCEPT;
    }
    Socket socket(fd, Socket::DEFAULT);
    return socket;
}

Socket Socket::Accept() const
{
    struct sockaddr addr;
    int len = sizeof(struct sockaddr_in);
    int fd = accept(sockfd, (struct sockaddr *)&addr, (socklen_t *)&len);
    // exception handling rquired here
    if (fd < 0) {
        throw SOCKET_EACCEPT;
    }
    Socket socket(fd, Socket::DEFAULT);
    return socket;
}
