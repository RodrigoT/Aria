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

// $Id: SocketPoolCell.cc,v 1.3 2001/11/19 16:26:21 tujikawa Exp $

#include "SocketPoolCell.h"

#define DEBUG 1
#undef DEBUG

SocketPoolCell::SocketPoolCell()
{
    socket = 0;
    port = 0;
    redirectedPort = -1;
    enabledFlag = false;
    lastUsedTime = 0;
#ifdef HAVE_OPENSSL
    ctx = NULL;
    ssl = NULL;
#endif // HAVE_OPENSSL
}

SocketPoolCell::SocketPoolCell(int socket_in, const string &host_in, int port_in)
{
    enabledFlag = false;
    socket = socket_in;
    host = host_in;
    port = port_in;
    redirectedHost = "";
    redirectedPort = -1;
    lastUsedTime = 0;
#ifdef HAVE_OPENSSL
    ctx = NULL;
    ssl = NULL;
#endif // HAVE_OPENSSL
}

SocketPoolCell::SocketPoolCell(int socket_in, const string &host_in, int port_in, const string &redirectedHost_in, int redirectedPort_in
#ifdef HAVE_OPENSSL
                               , SSL_CTX *ctx_in, SSL *ssl_in
#endif // HAVE_OPENSSL
                              )
{
    enabledFlag = false;
    socket = socket_in;
    host = host_in;
    port = port_in;
    redirectedHost = redirectedHost_in;
    redirectedPort = redirectedPort_in;
    setLastUsedTime();

#ifdef HAVE_OPENSSL
    ctx = ctx_in;
    ssl = ssl_in;
#endif // HAVE_OPENSSL
}

SocketPoolCell::~SocketPoolCell()
{
#ifdef DEBUG
    cerr << "closing socket: " << socket << endl;
#endif // DEBUG
    close(socket);
}

bool
SocketPoolCell::isTimeOut()
{
    int rightNowTime = time(NULL);
    if (rightNowTime - lastUsedTime > 600) {
        return true;
    } else {
        return false;
    }
}

/*
 * Check whether the socket is alive or not.
 * If either side of socket is closed, then this function returns false
 * Otherwise returns true
 */
bool
SocketPoolCell::isSocketAlive()
{
    fd_set rfds;
    fd_set wfds;
    struct timeval tv;
    int retval;

    FD_ZERO(&rfds);
    FD_ZERO(&wfds);
    FD_SET(socket, &rfds);
    FD_SET(socket, &wfds);
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    retval = select(socket + 1, &rfds, &wfds, NULL, &tv);
    if (retval
            && !FD_ISSET(socket, &rfds)
            && FD_ISSET(socket, &wfds)) {
        return true;
    } else {
        return false;
    }
}

bool
SocketPoolCell::isEqual(const string &host_in, int port_in)
{
    return isEqual(host_in, port_in, "", -1);
}

bool
SocketPoolCell::isEqual(const string &host_in, int port_in,
                        const string &redirectedHost_in, int redirectedPort_in)
{
    if (redirectedHost_in.empty()) {
        if (host_in == host && port_in == port) {
            return true;
        } else {
            return false;
        }
    } else {
        if (host_in == host && port_in == port
                && redirectedHost_in == redirectedHost
                && redirectedPort_in == redirectedPort) {
            return true;
        } else {
            return false;
        }
    }
}

bool
SocketPoolCell::isEqual(int socket_in)
{
    if (socket_in == socket) {
        return true;
    } else {
        return false;
    }
}

bool
SocketPoolCell::isEnabled()
{
    return enabledFlag;
}

void
SocketPoolCell::setEnabled(bool toggle)
{
    //cerr << "set enabled " << toggle << endl;
    enabledFlag = toggle;
    if (toggle == false) {
        setLastUsedTime();
    }
}

void
SocketPoolCell::setLastUsedTime()
{
#ifdef DEBUG
    cerr << "updating last used time" << endl;
#endif // DEBUG
    time(&lastUsedTime);
}

int
SocketPoolCell::getSocket()
{
    return socket;
}

const string &
SocketPoolCell::getHost()
{
    return host;
}

int
SocketPoolCell::getPort()
{
    return port;
}

void
SocketPoolCell::setSocket(int socket_in)
{
    socket = socket_in;
}

void
SocketPoolCell::setHost(const string &host_in)
{
    host = host_in;
}

void
SocketPoolCell::setPort(int port_in)
{
    port = port_in;
}

#ifdef HAVE_OPENSSL
SSL_CTX *
SocketPoolCell::getSSLCTX()
{
    return ctx;
}

SSL *
SocketPoolCell::getSSL()
{
    return ssl;
}

void
SocketPoolCell::setSSLCTX(SSL_CTX *ctx_in)
{
    ctx = ctx_in;
}

void
SocketPoolCell::setSSL(SSL *ssl_in)
{
    ssl = ssl_in;
}
#endif // HAVE_OPENSSL
