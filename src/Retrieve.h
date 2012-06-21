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

// $Id: Retrieve.h,v 1.7 2002/04/06 10:02:10 tujikawa Exp $

#ifndef _RETRIEVE_H_
#define _RETRIEVE_H_
#include <iostream>
#include "Socket.h"
#include "ItemCell.h"
#include "LockList.h"

#if defined(AF_INET6) && defined(HAVE_GETADDRINFO) && defined(HAVE_GETNAMEINFO) && defined(HAVE_FREEADDRINFO)
#define INET6 1
#endif

//
// ファイルダウンロードを担うクラス
//
class Retrieve
{
protected:
  ItemCell *itemcell;
  
public:
  Retrieve(ItemCell *itemcell);
  virtual ~Retrieve();

  virtual ItemCell::DownloadStatusType Download_Main();

protected:
  int inet_hostaddr(const string& hostname, struct in_addr *addr);

  // リモートホストにTCP/IPで接続する
  // connect remote host via TCP/IP
  void Make_TCP_connection(Socket& socket, int port);
  void Make_TCP_connection(Socket& socket, const string& server, int port);
  void Make_TCP_connection(Socket& socket, const string& server, int port, const string& redirectedServer, int redirectedPort);

  // 文字列 message を socket に書き込む
  // write message to socket
  void SEND(const string& message, const Socket& socket);

  // ローカルファイルのサイズを取得. ロールバック処理も行う
  unsigned int Get_starting_byte();

  // make a directory if it doesnot exists
  void Make_directory_if_needed();

  // if the modification time of the file is older than the time specified in
  // modtime, then return true. Otherwise return false
  bool Is_older_than_remote(time_t modtime);

  // 分割ダウンロードの準備
  bool Create_partial_item_entry(unsigned int divide, unsigned int total_size);

  unsigned int Download_data(ofstream& outfile, const Socket& socket, bool compressedFlag = false);
};
#endif //_RETRIEVE_H_

