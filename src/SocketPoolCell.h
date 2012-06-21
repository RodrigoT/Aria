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

// $Id: SocketPoolCell.h,v 1.3 2001/11/19 16:26:21 tujikawa Exp $

#ifndef _SOCKETPOOLCELL_H_
#define _SOCKETPOOLCELL_H_
#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/time.h>

#include "aria.h"
#ifdef HAVE_OPENSSL
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif // HAVE_OPENSSL

using namespace std;

class SocketPoolCell
{
private:
  int socket;
  string host;
  int port;
  string redirectedHost;
  int redirectedPort;
  bool enabledFlag;
  time_t lastUsedTime;
#ifdef HAVE_OPENSSL
  SSL_CTX *ctx;
  SSL *ssl;
#endif // HAVE_OPENSSL
  void setLastUsedTime();
public:
  SocketPoolCell();
  SocketPoolCell(int socket, const string& host, int port);
  SocketPoolCell(int socket, const string& host, int port,
		 const string& redirectedHost, int redirectedPort
#ifdef HAVE_OPENSSL
		 , SSL_CTX *ctx, SSL *ssl
#endif // HAVE_OPENSSL
		 );
  ~SocketPoolCell();

  bool isSocketAlive();
  bool isEqual(const string& host, int port, const string& redirectedHost, int redirectedPort);
  bool isEqual(const string& host, int port);
  bool isEqual(int socket);
  bool isTimeOut();
  time_t getLastUsedTime();
  // access functions
  bool isEnabled();
  void setEnabled(bool toggle);
  int getSocket();
  const string& getHost();
  int getPort();
  const string& getRedirectedHost();
  int getRedirectedPort();
  void setSocket(int socket);
  void setHost(const string& host);
  void setPort(int port);
  void setRedirectedHost(const string& host);
  void setRedirectedPort(int port);
#ifdef HAVE_OPENSSL
  SSL_CTX *getSSLCTX();
  SSL *getSSL();
  void setSSLCTX(SSL_CTX *ctx);
  void setSSL(SSL *ssl);
#endif // HAVE_OPENSSL
};
#endif // SOCKETPOOLCELL_H_
