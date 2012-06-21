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

// $Id: SocketPool.cc,v 1.4 2002/03/16 14:13:00 tujikawa Exp $

#include "SocketPool.h"
#define DEBUG 1
#undef DEBUG

SocketPool::SocketPool()
{
  // initialize mutex
  pthread_mutex_init(&socketPoolCellListLock, NULL);
}

SocketPool::~SocketPool()
{
  // delete all the element of the list before destruction
  deleteAllSocketPoolCell();

  pthread_mutex_destroy(&socketPoolCellListLock);
}

void
SocketPool::getSocketPoolLock()
{
  pthread_mutex_lock(&socketPoolCellListLock);
}

void
SocketPool::releaseSocketPoolLock()
{
  pthread_mutex_unlock(&socketPoolCellListLock);
}

vector<SocketPoolCell *>::iterator
SocketPool::searchPooledSocket(const string& host, int port)
{
  for(vector<SocketPoolCell *>::iterator itr = socketPoolCellList.begin(); itr != socketPoolCellList.end(); ++itr) {
    if((*itr)->isEqual(host, port)) {
      return itr;
    }
  }
  return socketPoolCellList.end();
}

vector<SocketPoolCell *>::iterator
SocketPool::searchPooledSocket(int socket)
{
  for(vector<SocketPoolCell *>::iterator itr = socketPoolCellList.begin(); itr != socketPoolCellList.end(); ++itr) {
    if((*itr)->isEqual(socket)) {
      return itr;
    }
  }
  return socketPoolCellList.end();
}

/*
 * Add a socket to the list.
 * the socket should be connected to host:port
 */
void
SocketPool::addSocketPoolCell(int socket, const string& host, int port)
{
  addSocketPoolCell(socket, host, port, "", -1
#ifdef HAVE_OPENSSL
		    , NULL, NULL
#endif // HAVE_OPENSSL
		    );
}

void
SocketPool::addSocketPoolCell(int socket, const string& host, int port,
			      const string& redirectedHost, int redirectedPort
#ifdef HAVE_OPENSSL
			      , SSL_CTX *ctx, SSL *ssl
#endif // HAVE_OPENSSL
			      )
{
  getSocketPoolLock();
  SocketPoolCell *socketPoolCellPtr = new SocketPoolCell(socket,
							 host,
							 port,
							 redirectedHost,
							 redirectedPort
#ifdef HAVE_OPENSSL
							 , ctx, ssl
#endif // HAVE_OPENSSL
							 );
  socketPoolCellList.push_back(socketPoolCellPtr);
  releaseSocketPoolLock();
}

// delete specified socket form the list
bool
SocketPool::deleteSocketPoolCell(int socket)
{
  bool retval = false;
  getSocketPoolLock();
  vector<SocketPoolCell *>::iterator itr = searchPooledSocket(socket);
  if(itr != socketPoolCellList.end()) {
    delete *itr;
    socketPoolCellList.erase(itr);
    retval = true;
  }
  releaseSocketPoolLock();
  return retval;
}

// delete all sockets from the list
void
SocketPool::deleteAllSocketPoolCell()
{
  getSocketPoolLock();
  for(vector<SocketPoolCell *>::iterator itr = socketPoolCellList.begin();
      itr != socketPoolCellList.end(); ++itr) {
    delete *itr;
  }
  socketPoolCellList.clear();
  releaseSocketPoolLock();
}

// get the socket associated to host:port
SocketPoolCell *
SocketPool::getPooledSocket(const string& host, int port)
{
  return getPooledSocket(host, port, "", -1);
}

SocketPoolCell *
SocketPool::getPooledSocket(const string& host, int port,
			    const string& redirectedHost,
			    int redirectedPort)
{
  getSocketPoolLock();

  SocketPoolCell *socketPoolCell = NULL;
#ifdef DEBUG
  cerr << "Size of socket pool: " << socketPoolCellList.size() << endl;
#endif //DEBUG
  for(vector<SocketPoolCell *>::iterator itr = socketPoolCellList.begin(); itr != socketPoolCellList.end(); ++itr) {
#ifdef DEBUG
    cerr << "Examining entry '" << (*itr)->getHost() << ':' << (*itr)->getPort() << ", socket: " << (*itr)->getSocket();
#endif //DEBUG
    if(!(*itr)->isEnabled()
       && (*itr)->isEqual(host, port, redirectedHost, redirectedPort)) {
       if((*itr)->isSocketAlive()) {
	 socketPoolCell = *itr;
	 (*itr)->setEnabled(true);
	 break;
       } else {
#ifdef DEBUG
	 cerr << "delete this entry" << endl;
#endif //DEBUG
	 vector<SocketPoolCell *>::iterator itr2 = itr;
	 --itr;
	 delete *itr2;
	 socketPoolCellList.erase(itr2);
       }
       // continue
    } else {
#ifdef DEBUG
       cerr << "skipping" << endl;
#endif // DEBUG
    }
  }
  releaseSocketPoolLock();
  return socketPoolCell;
}

void
SocketPool::returnPooledSocket(int socket)
{
  getSocketPoolLock();
  vector<SocketPoolCell *>::iterator itr = searchPooledSocket(socket);
  if(itr != socketPoolCellList.end()) {
    (*itr)->setEnabled(false);
  }
  releaseSocketPoolLock();
}

/*
 * scan the vector and delete closed sockets
 */
void
SocketPool::refresh()
{
  getSocketPoolLock();
#ifdef DEBUG
  cerr << "periodic refresh for socketPool" << endl;
#endif // DEBUG
  for(vector<SocketPoolCell *>::iterator itr = socketPoolCellList.begin();
      itr != socketPoolCellList.end(); ++itr) {
    if(!(*itr)->isEnabled()
       && (!(*itr)->isSocketAlive()
	   || (*itr)->isTimeOut())) {
      // delete this entry
#ifdef DEBUG
      cerr << "delete entry " << (*itr)->getHost() << ':' << (*itr)->getPort() << ", Socket: " << (*itr)->getSocket() << endl;
#endif // DEBUG
      vector<SocketPoolCell *>::iterator itrTemp = itr;
      --itr;
      delete *itrTemp;
      socketPoolCellList.erase(itrTemp);
    }
#ifdef DEBUG
    else {
      cerr << "alive entry " << (*itr)->getHost() << ':' << (*itr)->getPort() << ", Socket: " << (*itr)->getSocket() << endl;      
    }
#endif
  }
  releaseSocketPoolLock();
}
