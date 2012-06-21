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

// $Id: RetrieveFTP2.cc,v 1.7 2002/04/06 22:13:06 tujikawa Exp $

// class ItemCell_ftp implementation

#include "RetrieveFTP2.h"
#include "LockList.h"
extern LockList *g_lockList;

RetrieveFTP2::RetrieveFTP2(ItemCellPartial *itemcell_in)
  : RetrieveFTP(itemcell_in)
{
  itemcell = itemcell_in;
}

RetrieveFTP2::~RetrieveFTP2()
{
}

ItemCell::DownloadStatusType RetrieveFTP2::Download_Main()
{
  unsigned int startingbyte = 0;
  string report;
  string command;
  string username;
  string password;
  string retbuf;

  Socket sock_command(-1, Socket::DEFAULT);
  try {
    // connection established
    if(itemcell->ret_Status() != ItemCell::ITEM_CRCERROR &&
       itemcell->ret_Status() != ItemCell::ITEM_EXECERROR &&
       itemcell->ret_Status() != ItemCell::ITEM_DOWNLOAD_AGAIN &&
       itemcell->ret_Options().ret_downm_type() != Options::DOWNM_NORESUME) {
      startingbyte = Get_starting_byte();
    }
    itemcell->set_Status(ItemCell::ITEM_DOWNLOAD);
    itemcell->Send_status();
    
    // connect to ftp server    
//      if(sock_command.bad()) {
//        throw ItemCell::ITEM_EIO;
//      }

    /*
    bool use_proxy = false;
    if(ret_Options().ret_use_ftp_proxy() &&
       !ret_Options().ret_ftp_proxy().ret_Server().empty()) {
      use_proxy = true;
    }
    */
    //Make_TCP_connection(sock_command,ret_URL_Container().ret_Port());
    if(itemcell->ret_Options().ret_use_ftp_proxy() &&
       !itemcell->ret_Options().ret_ftp_proxy().ret_Server().empty()) {      
      Make_TCP_connection(sock_command,
			  itemcell->ret_Options().ret_ftp_proxy().ret_Server(),
			  itemcell->ret_Options().ret_ftp_proxy().ret_Port(),
			  itemcell->ret_URL_Container().ret_Hostname(),
			  itemcell->ret_URL_Container().ret_Port());
    } else {
      Make_TCP_connection(sock_command, 
			  itemcell->ret_URL_Container().ret_Hostname(),
			  itemcell->ret_URL_Container().ret_Port());
    }

    if(!sock_command.isPooledSocket()) {
      if(Get_response(sock_command, retbuf) < 0) {
	itemcell->set_Command(ItemCell::DLERROR);
	throw ItemCell::ITEM_EPROT;
      }
      Make_Authentication(sock_command);
    }
    /*
    // authentication phase
    if(ret_Options().Whether_use_authentication() &&
       ret_Options().ret_User() != "") {
      username = ret_Options().ret_User();
      password = ret_Options().ret_Password();
    } else {
      username = "anonymous";
      password = "IE40user@";
    }
    // send USER
    command = "USER "+username+"\r\n";
    Send_command(command, sock_command);
    if(Get_response(sock_command, retbuf) < 0) {
      set_Command(ItemCell::DLERROR);
      throw ItemCell::ITEM_EPROT;
    }

    // send PASS
    command = "PASS "+password+"\r\n";
    Send_command_pass(command, sock_command);
    
    if(Get_response(sock_command, retbuf) < 0) {
      Send_message_to_gui(_("Failed to login"), MSG_DOWNLOAD_ERROR);
      set_Command(ItemCell::DLERROR);
      throw ItemCell::ITEM_EPROT;
    }
    */

    // send TYPE command
    switch(itemcell->ret_Options().ret_FTP_ret_mode()) {
    case Options::FTP_BINARY:
      Send_command("TYPE I\r\n", sock_command);
      break;
    case Options::FTP_ASCII:
    default:
      Send_command("TYPE A\r\n", sock_command);
      break;
    }
    if(Get_response(sock_command, retbuf) < 0) {
      itemcell->set_Command(ItemCell::DLERROR);
      throw ItemCell::ITEM_EPROT;
    }
    // send CWD command
    if(!itemcell->ret_URL_Container().ret_Dir().empty()
       && !itemcell->ret_Options().isFtpNoCwdEnabled()) {
      command = "CWD "+itemcell->ret_URL_Container().ret_Dir()+"\r\n";
    }

    Send_command(command, sock_command);
    
    if(Get_response(sock_command, retbuf) < 0) {
      itemcell->Send_message_to_gui(_("Specified directory is not found"), MSG_DOWNLOAD_ERROR);
      itemcell->set_Command(ItemCell::DLERRORSTOP);
      throw ItemCell::ITEM_EPROT;
    }
    
    if(startingbyte > 0 &&
       itemcell->ret_Options().ret_downm_type() == Options::DOWNM_IFMODSINCE) {
      command = "MDTM "+retrPrefix+itemcell->ret_URL_Container().ret_Filename()+"\r\n";
      Send_command(command, sock_command);
      if(Get_response(sock_command, retbuf) < 0) {
	itemcell->Send_message_to_gui(_("MDTM command failed. Anyway, try resuming"), MSG_DOWNLOAD_INFO);
      } else {
	  time_t modtime = get_mod_time(retbuf);
	  if(Is_older_than_remote(modtime)) {
	    itemcell->Send_message_to_gui(_("Modification time of remote file is newer than local file's one. Resume disabled"), MSG_DOWNLOAD_INFO);
	    startingbyte = 0;
	  }
      }
    }
    // we already know the size of partial file
    itemcell->set_Size_Total(itemcell->ret_End_range()-itemcell->ret_Start_range());
    
    if(itemcell->ret_Size_Total() == startingbyte) {
      //すでにダウンロード済
      // downloading has been competed
      report = "'"+itemcell->ret_Filename()+"'"+_(" is already downloaded");
      itemcell->Send_message_to_gui(report, MSG_DOWNLOAD_INFO);
      
      itemcell->set_Size_Current(itemcell->ret_Size_Total());
      itemcell->Send_status();
      Leave_ftp_server(sock_command);
      //Send_message_to_gui(_("connection closed"), MSG_DOWNLOAD_INFO);
      throw ItemCell::ITEM_ESUCCESSALR;
    } else if(itemcell->ret_Size_Total() < startingbyte) {
      // ファイルサイズが異なる
      // size mismatch
      itemcell->Send_message_to_gui(_("Size of local file is larger than remote file's one. Download again"), MSG_DOWNLOAD_ERROR);
      startingbyte = 0;
      itemcell->set_Size_Total(itemcell->ret_End_range()-itemcell->ret_Start_range());
    } else {
      itemcell->set_Size_Total(itemcell->ret_End_range()-itemcell->ret_Start_range());
    }

    Options::FTP_Mode ftp_mode = itemcell->ret_Options().ret_FTP_Mode();

    Socket sock_data(-1, Socket::DEFAULT);
    switch(ftp_mode) {
    case Options::FTP_PASSIVE_MODE: // PASV mode
      {
	unsigned int startingbyte_real = startingbyte+itemcell->ret_Start_range();
	//	Socket sock_data;
//  	if(sock_data.bad()) {
//  	  throw ItemCell::ITEM_EIO;
//  	}
	Handle_ftp_passive_mode(sock_command, sock_data, startingbyte_real);
	break;
      }
    case Options::FTP_ACTIVE_MODE:
    default: //ACTIVE(NORMAL) mode
      {
//  	Socket sock_wait;
//  	if(sock_wait.bad()) {
//  	  throw ItemCell::ITEM_EIO;
//  	}
	unsigned int startingbyte_real = startingbyte+itemcell->ret_Start_range();
	Handle_ftp_active_mode(sock_command, sock_data, startingbyte_real);
	break;
      }
    }
    Start_Download(sock_data, startingbyte);
    sock_data.Shutdown(2);

    // ftp server will return negative response:
    // "426 Connection reset by peer."
    // this message can be ignored safely
    try {
      Get_response(sock_command, retbuf);
      
      if(!itemcell->ret_Options().ret_FTP_nosend_quit()) {
	Send_command("QUIT\r\n", sock_command);
	// expected reply: "221 Goodbye." or something like this
	Get_response(sock_command, retbuf);
	sock_command.Shutdown(2);
      }
    } catch (ItemCell::ItemErrorType err) {
      itemcell->PERROR(err);
      itemcell->Send_message_to_gui(_("An error occurred, but this error can be ignored safely"), MSG_DOWNLOAD_INFO);
      throw ItemCell::ITEM_ESUCCESS;
    }

    throw ItemCell::ITEM_ESUCCESS;
  }
  catch(ItemCell::ItemErrorType err) {
    //itemcell->Send_message_to_gui(_("Connection closed"), MSG_DOWNLOAD_INFO);
    switch(err) {
    case ItemCell::ITEM_ESUCCESS:
      itemcell->set_Command(ItemCell::DLSUCCESS);
      return ItemCell::DLSUCCESS;
    case ItemCell::ITEM_ESUCCESSALR:
      itemcell->set_Command(ItemCell::DLSUCCESSALR);
      return ItemCell::DLSUCCESSALR;
    case ItemCell::ITEM_EINTER:
      switch(itemcell->ret_Dl_status()) {
      case ItemCell::DLAGAIN:
      case ItemCell::DLSTOP:
      case ItemCell::DLERRORSTOP:
      case ItemCell::DLDELETEITEM:
      case ItemCell::DLDELETEITEMFILE:
      case ItemCell::DLPARTIALSUCCESS:
	sock_command.Shutdown(2);
	itemcell->Send_message_to_gui(_("Connection closed"), MSG_DOWNLOAD_INFO);
	break;
      case ItemCell::DLHALT:
      default:
	sock_command.Shutdown(2);
	break;
      }
      return itemcell->ret_Command();
    case ItemCell::ITEM_EIOFILE:
      sock_command.Shutdown(2);
      itemcell->Send_message_to_gui(_("Connection closed"), MSG_DOWNLOAD_INFO);
      itemcell->set_Command(ItemCell::DLERRORSTOP);
      itemcell->PERROR(err);
      return ItemCell::DLERRORSTOP;
    case ItemCell::ITEM_EPROT:// modified
      sock_command.Shutdown(2);
      itemcell->Send_message_to_gui(_("Connection closed"), MSG_DOWNLOAD_INFO);
      itemcell->PERROR(err);
      return itemcell->ret_Dl_status();
    case ItemCell::ITEM_ETIMEDOUT:
      sock_command.Shutdown(2);
      itemcell->Send_message_to_gui(_("Connection closed"), MSG_DOWNLOAD_INFO);
      itemcell->PERROR(err);
      itemcell->set_Command(ItemCell::DLERROR);
      return ItemCell::DLERROR;
    default:
      itemcell->PERROR(err);
      itemcell->set_Command(ItemCell::DLERROR);
      return ItemCell::DLERROR;
    }
  }
}

//
// ファイルを落す処理
//
// return:
//     -1: エラー
//      0: ユーザー操作による割り込み
//      1: 成功
//
void RetrieveFTP2::Start_Download(const Socket& socket, unsigned int startingbyte)
{
  ofstream outfile;
  string line;
  bool flag_trylock = false;
  string filename;
  Make_directory_if_needed();

  try {
    filename =  itemcell->ret_Options().ret_Store_Dir()+itemcell->ret_Filename();
    if(!g_lockList->Try_lock(filename)) {
      itemcell->Send_message_to_gui(_("This file is locked. Aborting download"), MSG_DOWNLOAD_ERROR);
      itemcell->set_Command(ItemCell::DLERRORSTOP);
      throw ItemCell::ITEM_EINTER;
    } else {
      flag_trylock = true;
    }

    line = _("A part of '")+itemcell->ret_URL()+"'"+
      _(", its index is ")+
      itos(itemcell->ret_Order())+
      _(", its range is from ")+
      itos(itemcell->ret_Start_range(), true)+
      " to "+
      itos(itemcell->ret_End_range(), true);
    itemcell->Send_message_to_gui(line, MSG_DOWNLOAD_INFO);

    line = _("Starting download at ")+itos(itemcell->ret_Start_range()+
					   startingbyte, true)+
      _(" bytes");
    itemcell->Send_message_to_gui(line, MSG_DOWNLOAD_INFO);

    switch(itemcell->SplitNumberChanged(filename+".index")) {
    case ItemCellPartial::PARTIAL_NORMAL:
      break;
    case ItemCellPartial::PARTIAL_CHANGED:
      //Send_message_to_gui(_("split count has changed"), MSG_DOWNLOAD_INFO);
    default:
      startingbyte = 0;
    }

    if(startingbyte == 0) {
      // 新規にダウンロード
      outfile.open(filename.c_str(), ios::out|ios::trunc|ios::binary);
    } else {
      // レジュームする
      outfile.open(filename.c_str(), ios::out|ios::app|ios::binary);
    }
    if(outfile.bad()) {
      throw ItemCell::ITEM_EIOFILE;
    }

    itemcell->set_Size_Current(startingbyte);
    itemcell->set_previous_dl_size(Download_data(outfile, socket));
    outfile.close();
    if(itemcell->ret_Size_Current() != itemcell->ret_Size_Total()
       && itemcell->ret_Size_Total() != 0) {
      itemcell->Send_message_to_gui(_("No match size"), MSG_DOWNLOAD_ERROR);
      itemcell->set_Command(ItemCell::DLERROR);
      throw ItemCell::ITEM_EPROT;
    }
  } catch (ItemCell::ItemErrorType err) {
    if(flag_trylock) g_lockList->Unlock(filename);
    throw err;
  }
  if(flag_trylock) g_lockList->Unlock(filename);
}
