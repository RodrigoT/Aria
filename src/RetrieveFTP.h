//
//  Aria - yet another download tool
//  Copyright (C) 2000, 2001, 2002 Tatsuhiro Tsujikawa
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

// $Id: RetrieveFTP.h,v 1.5 2002/02/13 12:09:24 tujikawa Exp $
#ifndef _RETRIEVEFTP_H_
#define _RETRIEVEFTP_H_
#include <sys/time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fstream>
#include <utility>
#include "aria.h"
#include "FTPcontainer.h"
#include "ItemCell.h"
#include "utils.h"
#include "Retrieve.h"

using namespace std;

class RetrieveFTP : public Retrieve
{
public:
  // constructor
  RetrieveFTP(ItemCell *itemcell);
  // destructor
  virtual ~RetrieveFTP();

  ItemCell::DownloadStatusType Download_Main();
protected:
  // string put before file name in RETR, SIZE command when no CWD option
  // is enabled
  string retrPrefix;

  // FTPサーバーからの返答を取得
  // return:
  //     -1: エラー
  //      1: 成功
  int Get_response(const Socket& socket, string& retbuf);
  int Get_response(const Socket& socket, int& retstat, string& retbuf);

  // FTPサーバーにコマンドcommandを送る
  void Send_command(const string& command, const Socket& socket);

  // FTPサーバーにコマンドcommandを送る(表示は*に置き換えられる)
  void Send_command_pass(const string& command, const Socket& socket);

  void Send_command_cwdpass(const string& command, const Socket& sock_command);

  // PORTコマンドを作る
  void Get_PORT_command(const Socket& sock_command, const Socket& sock_wait, string& portcommand);

  // 認証など
  void Make_Authentication(const Socket& sock_command);
  // sub routines for authentication
  void Get_username_password(string& username, string& password);
  void Get_proxy_username_password(string& username, string& password);

  void Send_username_password_sub(const Socket& sock_command,
				  const string& username,
				  const string& password);

  void Send_username_password(const Socket& sock_command);
  void Send_proxy_username_password(const Socket& sock_command);
  
  void Send_user_password(const Socket& sock_command);
  void Send_proxy_user_password(const Socket& sock_command);

  void Send_open_site_sub(const Socket& sock_command, string command);
  void Send_open(const Socket& sock_command);
  void Send_open2(const Socket& sock_command);
  void Send_site(const Socket& sock_command);

  // FTPサーバーから離脱
  void Leave_ftp_server(Socket& sock_command);

  // PASVモード
  void Handle_ftp_passive_mode(const Socket& sock_command, Socket& sock_data, unsigned int& startingbyte);
  void connect_from(const Socket& sock_command, Socket& sock_wait);

  // ACTIVEモード
  void Handle_ftp_active_mode(const Socket& sock_command, Socket& sock_data, unsigned int& startingbyte);
  FTPcontainer connect_to(const Socket& sock_command);

  // start downloading
  void Start_Download(const Socket& sock_data, unsigned int startingbyte);

  // send PORT command
  void Send_port_command(const Socket& sock_command, const Socket& sock_wait);

  string Get_fileinfo_by_ls(const Socket& sock_command, const string& filename = "");

  list<ItemCell*> Get_filelist(const Socket& sock_command);
  list<ItemCell*> Make_filelist(string lsdata);
  ItemCell* Make_itemcell(const string& url, const string& save_dir);
};

#endif // _RETRIEVEFTP_H_
