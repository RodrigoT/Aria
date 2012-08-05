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

// $Id: RetrieveHTTP.cc,v 1.30 2002/12/18 15:41:05 tujikawa Exp $

// class ItemListCell implementation
#include "RetrieveHTTP.h"

#define DEBUG 1
#undef DEBUG
extern LockList *g_lockList;
extern ServerTemplateList g_servTempList;
extern UseragentList *g_userAgentList;
extern ItemList *g_itemList;
extern AppOption *g_appOption;

RetrieveHTTP::RetrieveHTTP(ItemCell *itemcell) : Retrieve(itemcell)
{
}

RetrieveHTTP::~RetrieveHTTP()
{
}

//
// Web サーバーに要求メッセージを送る
//
void RetrieveHTTP::Send_Request(const Socket& sock, unsigned int startingbyte)
{
  string command, command_line;

  // GET command
  bool proxy_in_use = itemcell->ret_Options().ret_use_http_proxy() &&
    !itemcell->ret_Options().ret_http_proxy().ret_Server().empty();
  bool ftp_proxy_in_use = itemcell->ret_URL_Container().ret_Protocol() == "ftp:";
  if(ftp_proxy_in_use || proxy_in_use) {
    command_line = "GET "+
      itemcell->ret_URL_Container().ret_Protocol()+
      "//"+
      URLcontainer::URL_Encode(itemcell->ret_URL_Container().ret_Hostname()+
			       itemcell->ret_URL_Container().ret_Dir()+
			       itemcell->ret_URL_Container().ret_File())+
      itemcell->ret_URL_Container().ret_Query()+
      " HTTP/"+itemcell->ret_Options().ret_HTTP_version()+"\r\n"; // modified 2001/3/20
  } else {
    command_line = "GET "+
      URLcontainer::URL_Encode(itemcell->ret_URL_Container().ret_Dir()+
			       itemcell->ret_URL_Container().ret_File())+
      itemcell->ret_URL_Container().ret_Query()+
      " HTTP/"+itemcell->ret_Options().ret_HTTP_version()+"\r\n"; // modified 2001/3/20
  }
  itemcell->Send_message_to_gui(command_line, MSG_DOWNLOAD_SEND);
  command = command_line;

  // Referer
  Options::RefererType referer_type;
  string referer_string;

  referer_type = itemcell->ret_Options().ret_Referer_Type();
  referer_string = itemcell->ret_Options().ret_Referer();

  switch(referer_type) {
  case Options::REFERER_NONE:
    command_line = "Referer:\r\n";
    break;
  case Options::REFERER_USER_DEFINED:
    command_line = "Referer: "+URLcontainer::URL_Encode(referer_string)+"\r\n";
    break;
  case Options::REFERER_URL:
    command_line = "Referer: "+URLcontainer::URL_Encode(itemcell->ret_URL())+"\r\n";
    break;
  case Options::REFERER_NOSEND:
    command_line = "";
    break;
  case Options::REFERER_INDEX:
  default:
    command_line = "Referer: "+
      itemcell->ret_URL_Container().ret_Protocol()+
      "//"+
      URLcontainer::URL_Encode(itemcell->ret_URL_Container().ret_Hostname()+
			       itemcell->ret_URL_Container().ret_Dir())+
      "/index.html\r\n";
    break;
  }
  itemcell->Send_message_to_gui(command_line, MSG_DOWNLOAD_SEND);
  command = command+command_line;

  // User-Agent
  if(0 && itemcell->ret_Options().ret_Random_useragent()) {
    unsigned int list_size = g_userAgentList->ret_vector().size();
    const vector<string>& useragent_vector = g_userAgentList->ret_vector();
    command_line = "User-Agent: "+useragent_vector[(int)((float)list_size*random()/(RAND_MAX+1.0))]+"\r\n";
  } else {
    command_line = "User-Agent: "+itemcell->ret_Options().ret_Useragent()+"\r\n";
  }
  itemcell->Send_message_to_gui(command_line, MSG_DOWNLOAD_SEND);
  command = command+command_line;

  // authentication
  // currently, basic authentication only
  if(ftp_proxy_in_use) {
    if(itemcell->ret_Options().ret_use_ftp_proxy_authentication() &&
       !itemcell->ret_Options().ret_ftp_proxy_User().empty()) {
      string user_pass = itemcell->ret_Options().ret_ftp_proxy_User()+":"+
	itemcell->ret_Options().ret_ftp_proxy_Password();
      string encoded_user_pass = Base64::encode(user_pass);
      command_line = "Proxy-Authorization: Basic "+encoded_user_pass+"\r\n";
      itemcell->Send_message_to_gui(command_line, MSG_DOWNLOAD_SEND);
      command = command+command_line;
    }
  } else if(proxy_in_use) {
    if(itemcell->ret_Options().ret_use_http_proxy_authentication() &&
       !itemcell->ret_Options().ret_http_proxy_User().empty()) {
      string user_pass = itemcell->ret_Options().ret_http_proxy_User()+":"+
	itemcell->ret_Options().ret_http_proxy_Password();
      string encoded_user_pass = Base64::encode(user_pass);
      command_line = "Proxy-Authorization: Basic "+encoded_user_pass+"\r\n";
      itemcell->Send_message_to_gui(command_line, MSG_DOWNLOAD_SEND);
      command = command+command_line;
    }
  } else if(itemcell->ret_Options().Whether_use_authentication() &&
	    !itemcell->ret_Options().ret_User().empty()) {
    string user_pass = itemcell->ret_Options().ret_User()+":"+
      itemcell->ret_Options().ret_Password();
    string encoded_user_pass = Base64::encode(user_pass);
    command_line = "Authorization: Basic "+encoded_user_pass+"\r\n";
    //command_line = "Authorization: Digest "+md5CheckString(user_pass)+"\r\n";
    itemcell->Send_message_to_gui(command_line, MSG_DOWNLOAD_SEND);
    command = command+command_line;
  }

  // Cookie: 適当ですみません
  // I apologize for lazy implementation of cookie
  if((!itemcell->Is_current_session_valid() ||
      !itemcell->ret_current_session().Is_nocookie()) &&
     !itemcell->ret_Options().ret_Cookie_nosend()) {
    string key_value;
    if(itemcell->ret_Options().getCookieUserDefined() &&
       itemcell->ret_Options().getCookieUserDefinedString().size()) {
      key_value = itemcell->ret_Options().getCookieUserDefinedString();
    } else {
      key_value = itemcell->ret_Cookie_list().ret_valid_cookie_string(itemcell->ret_URL_Container().ret_Hostname(), itemcell->ret_URL_Container().ret_Dir());
    }
    if(key_value.size()) {
      command_line = "Cookie: "+key_value+"\r\n";
      itemcell->Send_message_to_gui(command_line, MSG_DOWNLOAD_SEND);
      command = command+command_line;
    }
  }
  if(itemcell->Is_current_session_valid() &&
     itemcell->ret_current_session().Is_deletecookie()) {
    itemcell->ret_Cookie_list().all_clear();
  }

  // Connection: close
  // In HTTP/1.0, connection is closed per request-response by default,
  // in HTTP/1.1, connection is keep alive by default.
  if(proxy_in_use || ftp_proxy_in_use) {
    command_line = "Connection: close\r\n";
    itemcell->Send_message_to_gui(command_line, MSG_DOWNLOAD_SEND);
    command = command+command_line;
  } else {//if("1.0" == itemcell->ret_Options().ret_HTTP_version()) {
    command_line = "Connection: Keep-Alive\r\n";
    itemcell->Send_message_to_gui(command_line, MSG_DOWNLOAD_SEND);
    command = command+command_line;
  }

  // Accept
  command_line = "Accept: */*\r\n";/* accept */
  itemcell->Send_message_to_gui(command_line, MSG_DOWNLOAD_SEND);
  command = command+command_line;

  if(itemcell->ret_Options().ret_HTTP_accept_lang_enabled()
     && itemcell->ret_Options().ret_HTTP_accept_lang_string().size()) {
    command_line = "Accept-Language: "+itemcell->ret_Options().ret_HTTP_accept_lang_string()+"\r\n";
    itemcell->Send_message_to_gui(command_line, MSG_DOWNLOAD_SEND);
    command = command+command_line;
  }

#ifdef HAVE_ZLIB
  if(!itemcell->Is_Partial() && itemcell->ret_Options().ret_HTTP_accept_compression()) {
    command_line = "Accept-Encoding: gzip, deflate\r\n";
    itemcell->Send_message_to_gui(command_line, MSG_DOWNLOAD_SEND);
    command = command+command_line;
  }
#endif // HAVE_ZLIB

  // Host
  // HTTP/1.1 needs Host field.
  command_line = "Host: "+itemcell->ret_URL_Container().ret_Hostname();
  if(itemcell->ret_URL_Container().ret_Port() != 80) {
    command_line += ':'+itos(itemcell->ret_URL_Container().ret_Port())+"\r\n";
  } else {
    command_line += "\r\n";
  }
  itemcell->Send_message_to_gui(command_line, MSG_DOWNLOAD_SEND);
  command += command_line;

  //Cache control
  if((itemcell->ret_Options().ret_use_http_proxy() &&
     !itemcell->ret_Options().ret_http_proxy().ret_Server().empty() &&
     !itemcell->ret_Options().ret_use_http_cache()) ||
     (itemcell->ret_URL_Container().ret_Protocol() == "ftp:" &&
     itemcell->ret_Options().ret_use_ftp_cache())) {
    command_line = "Pragma: no-cache\r\n";
    itemcell->Send_message_to_gui(command_line, MSG_DOWNLOAD_SEND);
    command += command_line;
    command_line = "Cache-Control: no-cache\r\n";
    itemcell->Send_message_to_gui(command_line, MSG_DOWNLOAD_SEND);
    command += command_line;
  }    

  // Range
  if(startingbyte != 0) {
    command_line = "Range: bytes="+itos(startingbyte)+"-\r\n";
    itemcell->Send_message_to_gui(command_line, MSG_DOWNLOAD_SEND);
    command = command+command_line;

    // if-mod-since
    if(itemcell->ret_Options().ret_downm_type() == Options::DOWNM_IFMODSINCE) {
      command_line = "If-Modified-Since: "+get_file_mod_date(itemcell->ret_Options().ret_Store_Dir()+itemcell->ret_Filename())+"\r\n";
      itemcell->Send_message_to_gui(command_line, MSG_DOWNLOAD_SEND);
      command = command+command_line;
    }
  }
  command += "\r\n";
  SEND(command, sock);
}

//
// wwwサーバーからの応答メッセージのヘッダーをよんで**httpheaerに格納
//
void RetrieveHTTP::Get_HTTP_header(const Socket& socket, HTTPHeaderList& http_header_list)
{

  string report;
  struct timeval tv;
  tv.tv_sec = itemcell->ret_Options().ret_Timed_Out();
  tv.tv_usec = 0;

  bool first_line = true;
  list<int> fd_list;

  while(1) {
    string header_string;
    char ch[1024];
    char *ch_tail = ch;
    while(1) {
      fd_list.clear();
      fd_list.push_back(itemcell->ret_Desc_r());

      int retval = socket.is_readready(&tv, fd_list);
      if(retval && Socket::is_set(itemcell->ret_Desc_r(), fd_list)) {
	ItemCommand itemcommand;
	read(itemcell->ret_Desc_r(), &itemcommand, sizeof(ItemCommand));
	itemcell->set_Command(itemcommand);
	if(itemcell->ret_Dl_status() == ItemCell::DLCHANGE) {
	  itemcell->Process_command(itemcommand);

	  updateInterval(tv, itemcell->ret_Options().ret_Timed_Out());
	} else {
	  throw ItemCell::ITEM_EINTER;
	}
      } else if(retval && socket.is_set(fd_list)) {
	// reset time out value
	tv.tv_sec = itemcell->ret_Options().ret_Timed_Out();
	tv.tv_usec = 0;
  
	int size = socket.Recv(ch_tail, sizeof(ch)-(ch_tail-ch), MSG_PEEK);
	if(size <= 0) {
	  throw ItemCell::ITEM_ERECV;
	}
	int crlf_pos = 0;
	while(1) {
	  if(crlf_pos == size) break;
	  if(ch[crlf_pos] == '\r' || ch[crlf_pos] == '\n') {
	    break;
	  }
	  ++crlf_pos;
	}
	if(crlf_pos == 0) {
	  if(ch[crlf_pos] == '\r') {
	    size = socket.Recv(ch, 2, 0);
	  } else {
	    size = socket.Recv(ch, 1, 0);
	  }
	  header_string = "";
	  break;
	} else if(crlf_pos < size) {
	  if(ch[crlf_pos] == '\r') {
	    size = socket.Recv(ch, crlf_pos+2, 0);
	    ch[size-2] = '\0';
	  } else {
	    size = socket.Recv(ch, crlf_pos+1, 0);
	    ch[size-1] = '\0';
	  }
	  header_string = ch;
	  break;
	} else {
	  //ch[size] = '\0';// dangerous??
	  ch_tail = ch+size;
	}
      } else {
	throw ItemCell::ITEM_ETIMEDOUT;
      }
    }
    if(header_string.empty()) break;
    string header_string_temp = header_string;
    if(first_line) {
      first_line = false;
      //if(header_vector.size() < 2) throw ItemCell::ITEM_EPROT;
      string HTTP_version = Token_splitter(header_string_temp);
      int HTTP_status = stoi(Token_splitter(header_string_temp));
      string arg;
      //vector<string> arg_v;
      //copy(istream_iterator<string>(msgbuf), istream_iterator<string>(),
      //   back_inserter(arg_v));
      //cerr << header_string << endl;
      itemcell->Send_message_to_gui(header_string, MSG_DOWNLOAD_RECV);


      HTTP_Header httpheader("status", itos(HTTP_status));
      http_header_list.push_back(httpheader);
    } else {
      string header = Token_splitter(header_string_temp, ":");
      string arg = Remove_white(header_string_temp);
      itemcell->Send_message_to_gui(header_string, MSG_DOWNLOAD_RECV);
      HTTP_Header httpheader(header, arg);
      http_header_list.push_back(httpheader);
    }
  }
}

void RetrieveHTTP::validateHTTPStatus(int httpStatus)
{
      switch(httpStatus) {
      case OK:
      case MultipleChoices:
      case MovedPermanently:
      case Found:
      case NotModified:
      case SeeOther:
	break;
      case UseProxy:
	itemcell->Send_message_to_gui(_("UseProxy is not supported yet"), MSG_DOWNLOAD_ERROR);
	itemcell->set_Command(ItemCell::DLERRORSTOP);
	throw ItemCell::ITEM_EPROT;
      case PartialContent:
	break;
      case Forbidden:
	itemcell->Send_message_to_gui(_("Content forbidden"), MSG_DOWNLOAD_ERROR);
	itemcell->set_Command(ItemCell::DLERROR);
	throw ItemCell::ITEM_EPROT;
      case NotFound:
	itemcell->Send_message_to_gui(_("Content not found"), MSG_DOWNLOAD_ERROR);
	if(itemcell->ret_Options().ret_force_retry_404()) {
	  itemcell->set_Command(ItemCell::DLERROR);
	} else {
	  // don't retry
	  itemcell->set_Command(ItemCell::DLERRORSTOP);
	}
	throw ItemCell::ITEM_EINTER;
      case ServiceUnavailable:
	if(itemcell->ret_Options().ret_force_retry_503()) {
	  itemcell->set_Command(ItemCell::DLERROR);
	} else {
	  // don't retry
	  itemcell->set_Command(ItemCell::DLERRORSTOP);
	}
	throw ItemCell::ITEM_EINTER;
      case RequestedRangeNotSatisfiable:
	switch(itemcell->ret_Options().ret_status_416_handling()) {
	case Options::S416SUCC:
	  itemcell->Send_message_to_gui(_("Server returned 416 status. Assume download was completed"), MSG_DOWNLOAD_INFO);
	  throw ItemCell::ITEM_ESUCCESSALR;
	case Options::S416REDOWN:
	  itemcell->Send_message_to_gui(_("Download again"), MSG_DOWNLOAD_INFO);
	  itemcell->set_Command(ItemCell::DLAGAIN);
	  throw ItemCell::ITEM_EINTER;
	case Options::S416ERR:
	default:
	  itemcell->Send_message_to_gui(_("Server returned 416 status. This file may be already downloaded"), MSG_DOWNLOAD_ERROR);
	  itemcell->set_Command(ItemCell::DLERRORSTOP);
	  throw ItemCell::ITEM_EINTER;
	}
      case AuthorizationRequired:
	itemcell->Send_message_to_gui(_("Access denied"), MSG_DOWNLOAD_ERROR);
	itemcell->set_Command(ItemCell::DLERRORSTOP);
	throw ItemCell::ITEM_EINTER;
      default:
	itemcell->Send_message_to_gui(_("Unexpected error occurred"), MSG_DOWNLOAD_ERROR);
	itemcell->set_Command(ItemCell::DLERRORSTOP);
	throw ItemCell::ITEM_EPROT;
      }
}

ItemCell::DownloadStatusType RetrieveHTTP::Download_Main()
{
  unsigned int startingbyte = 0;
  string report;
  ItemCell::ItemStatusType initialstatus = ItemCell::ITEM_READY;
  URLcontainer sessionURL;
  compressedFlag = false;

  // ITEM_DOWNLOAD_AGAIN : download from scratch
  // ITEM_DOWNLOAD_INTERNAL_AGAIN : loop caused by server template
  if(itemcell->ret_Session_counter() == 1 &&
     itemcell->ret_Status() != ItemCell::ITEM_DOWNLOAD_INTERNAL_AGAIN) {
    itemcell->ret_URL_Container().Parse_URL(itemcell->ret_URL());
    itemcell->ret_Retrieved_urlcon().clear();
    if(itemcell->ret_URL_Container().ret_Filename().empty()) {
      itemcell->set_Filename("");
    }

    if(itemcell->ret_Options().ret_Cookie_delete_on_restart()) itemcell->ret_Cookie_list().all_clear();
  }

  // 再帰回数 > 1でドキュメントルートが空ならドキュメントルートを現在の
  // 保存ディレクトリに設定する
  if(itemcell->ret_Options().ret_recurse_count() > 1 &&
     itemcell->ret_documentroot_dir().empty()) {
    itemcell->set_documentroot_dir(itemcell->ret_Options().ret_Store_Dir());
    if(itemcell->ret_Options().ret_with_hostname_dir()) {
      string fix_dir;
      fix_dir = itemcell->ret_URL_Container().ret_Dir();
      itemcell->ret_Options().set_Store_Dir(itemcell->ret_documentroot_dir()+itemcell->ret_URL_Container().ret_Hostname()+fix_dir);
    }
  } else if(itemcell->ret_Options().ret_recurse_count() > 1 &&
	    itemcell->ret_Options().ret_with_hostname_dir() &&
	    itemcell->ret_Options().ret_Store_Dir().find(itemcell->ret_URL_Container().ret_Hostname()+itemcell->ret_URL_Container().ret_Dir()) == string::npos) {
    string fix_dir;
    fix_dir = itemcell->ret_URL_Container().ret_Dir();
    itemcell->ret_Options().set_Store_Dir(itemcell->ret_documentroot_dir()+itemcell->ret_URL_Container().ret_Hostname()+fix_dir);
  }

  // socket
  Socket sock(-1, Socket::DEFAULT);
  try {
    if(!g_appOption->ret_use_servertemplate()) {
      itemcell->ret_svt().set_valid(false);
    } else if(itemcell->ret_Session_counter() == 1 ||
	      !itemcell->Is_current_session_valid() ||
	      itemcell->ret_current_session().Is_readconfig()) {
      itemcell->set_svt(g_servTempList.search(itemcell->ret_URL_Container().ret_Hostname(), itemcell->ret_URL_Container().ret_Filename()));
      itemcell->Reset_Session_counter();
    }
    if(itemcell->Is_current_session_valid()){
      itemcell->Send_message_to_gui("Using Server Template '"+
				    itemcell->ret_svt().ret_template_name()+
				    "'", MSG_DOWNLOAD_INFO);
      itemcell->Send_message_to_gui("Entering Session "+
				    itos(itemcell->ret_Session_counter()),
				    MSG_DOWNLOAD_INFO);
      // modified 2001/3/9
      URLcontainer orig_urlcon;
      orig_urlcon.Parse_URL(itemcell->ret_URL());
      if(itemcell->ret_current_session().ret_get_vector().size() &&
	 !itemcell->ret_URL_Container().Parse_URL(itemcell->ret_current_session().Create_URL_from_get_vector(orig_urlcon, itemcell->ret_Retrieved_urlcon()))) {
	throw ItemCell::ITEM_ESERVERCONFIG;
      }
      // added 2001/5/27
      // ここで今回 get する URL を退避しておく
      // 次の Session で利用できるようにする $(previous_***) ファミリが使う
      sessionURL = itemcell->ret_URL_Container();
      // fix this
      // ユーザのファイル名設定を反映していない

      itemcell->set_Filename(itemcell->ret_URL_Container().ret_Filename());


      if(itemcell->ret_current_session().ret_referer_vector().size()) {
	itemcell->ret_Options().set_Referer_Type(Options::REFERER_USER_DEFINED);
	itemcell->ret_Options().set_Referer(itemcell->ret_current_session().Create_URL_from_referer_vector(orig_urlcon, itemcell->ret_Retrieved_urlcon()));
      }
    }

    if((itemcell->ret_Status() != ItemCell::ITEM_CRCERROR &&
       itemcell->ret_Status() != ItemCell::ITEM_EXECERROR &&
       itemcell->ret_Status() != ItemCell::ITEM_INUSE_AGAIN &&
       itemcell->ret_Status() != ItemCell::ITEM_DOWNLOAD_AGAIN &&
       itemcell->ret_Options().ret_downm_type() != Options::DOWNM_NORESUME &&
       itemcell->ret_Filename().size() &&
       (!itemcell->Is_current_session_valid() ||
	(!itemcell->ret_current_session().Is_noresume() &&
	!itemcell->ret_current_session().Is_nodown() &&
	!itemcell->ret_current_session().Is_getkeylink() &&
	!itemcell->ret_current_session().Is_getkeylink_force())))) {
      startingbyte = Get_starting_byte();
    }
    initialstatus = itemcell->ret_Status();
    itemcell->set_Status(ItemCell::ITEM_DOWNLOAD);
    itemcell->Send_status();

    // connect to www server
    //Socket sock(-1, Socket::DEFAULT);
    //      if(sock.bad()) {
    //        throw ItemCell::ITEM_EIO;
    //      }

    establishConnection(sock);
    /*
    if(itemcell->ret_URL_Container().ret_Protocol() == "ftp:") {
      Make_TCP_connection(sock,
			  itemcell->ret_Options().ret_ftp_proxy().ret_Server(),
			  itemcell->ret_Options().ret_ftp_proxy().ret_Port());
    } else if(itemcell->ret_Options().ret_use_http_proxy() &&
       !itemcell->ret_Options().ret_http_proxy().ret_Server().empty()) {      
      Make_TCP_connection(sock,
			  itemcell->ret_Options().ret_http_proxy().ret_Server(),
			  itemcell->ret_Options().ret_http_proxy().ret_Port());
    } else {
      Make_TCP_connection(sock, 
			  itemcell->ret_URL_Container().ret_Hostname(),
			  itemcell->ret_URL_Container().ret_Port());
    }
    */
    // send GET request to www server]
    Send_Request(sock, startingbyte);

    // get HTTP header
    HTTPHeaderList httpheaderlist;

    Get_HTTP_header(sock, httpheaderlist);

    // analyze HTTP header and store it to class HTTPcontainer
    HTTPcontainer httpcon;

    httpcon.Parse_HTTP_header(httpheaderlist);
    validateHTTPStatus(httpcon.ret_HTTP_Status());
    switch(httpcon.ret_HTTP_Status()) {
    case MultipleChoices:
    case MovedPermanently:
    case Found:
    case SeeOther:
      {
	if(itemcell->ret_Options().ret_use_no_redirection()) {
	  itemcell->Send_message_to_gui(_("Redirection found"),
					MSG_DOWNLOAD_ERROR);
	  itemcell->set_Command(ItemCell::DLERROR);
	  throw ItemCell::ITEM_EINTER;
	}
	URLcontainer urlcon_new;
	string location = URLcontainer::URL_Decode(httpcon.ret_Location());
	if(!startwith(location, "http://") &&
#ifdef HAVE_OPENSSL
	   !startwith(location, "https://") &&
#endif // HAVE_OPENSSL
	   !startwith(location, "ftp://")) {
	  // relative URL
	  try {
	    location = get_abs_url(itemcell->ret_URL_Container().ret_Protocol()+
				   "//"+
				   itemcell->ret_URL_Container().ret_Hostname()+
				   itemcell->ret_URL_Container().ret_Dir(),
				   location);
	  } catch (int err) {
	    // stack underflow
	    // server side problem?
	  }
	}
	if(!urlcon_new.Parse_URL(location)) {
	  location += '/';
	  if(!urlcon_new.Parse_URL(location)) {
	    //Send_message_to_gui(_("Location retrieval failed"), MSG_DOWNLOAD_ERROR);
	    throw ItemCell::ITEM_ELOCATION;
	  }
	}
	// set Cookie
	for(list<string>::const_iterator itr = httpcon.ret_Cookie_list().begin();
	    itr != httpcon.ret_Cookie_list().end(); ++itr) {
	  itemcell->ret_Cookie_list().add_cookie(*itr);
	}
	itemcell->ret_Options().set_Referer_Type(Options::REFERER_URL);

	if(urlcon_new.ret_Protocol() != itemcell->ret_URL_Container().ret_Protocol()) {
	  // the following block was modified on 2001/5/10
	  
	  itemcell->ret_Options().set_use_authentication(false);
	}
	itemcell->set_Retrieved_urlcon(urlcon_new);
	itemcell->set_URL_Container(urlcon_new);
	// fixed 2001/11/13
	if(itemcell->ret_Options().ret_sync_with_URL()) {
	  itemcell->set_Filename(urlcon_new.ret_Filename());
	}

	report = _("Redirecting to ")+location;
	itemcell->Send_message_to_gui(report, MSG_DOWNLOAD_INFO);
	sock.Shutdown(2);
	itemcell->Send_message_to_gui(_("Connection closed"), MSG_DOWNLOAD_INFO);
	if(itemcell->Is_current_session_valid()) {
	  throw ItemCell::ITEM_ENEXTSTAGE;
	} else {
	  return ItemCell::DLINTERNALAGAIN;
	}
      }
    case NotModified:
      itemcell->Send_message_to_gui("'"+itemcell->ret_Filename()+"'"+_(" is already downloaded"), MSG_DOWNLOAD_INFO);
      itemcell->set_Size_Current(startingbyte);
      itemcell->set_Size_Total(startingbyte);
      itemcell->Send_status();
      throw ItemCell::ITEM_ESUCCESSALR;
    }

    // set md5 from response header
    // fix this
    itemcell->get_Options_Lock();
    if(itemcell->ret_Options_opt().ret_use_content_md5() && httpcon.ret_MD5().size()) {
      itemcell->set_md5string(httpcon.ret_MD5());
    }
    itemcell->release_Options_Lock();


    // set Cookie
    for(list<string>::const_iterator itr = httpcon.ret_Cookie_list().begin();
	itr != httpcon.ret_Cookie_list().end(); ++itr) {
      itemcell->ret_Cookie_list().add_cookie(*itr);
    }

    // use content-location field if it exists
    if(httpcon.ret_contentLocation().size()) {
      URLcontainer urlconTemp;
      if(urlconTemp.Parse_URL(httpcon.ret_contentLocation())) {
	// absolute URL form
	itemcell->set_URL_Container(urlconTemp);
      } else {
	// relative URL form
	string newURL = get_abs_url(itemcell->ret_URL_Container().ret_URL(), httpcon.ret_contentLocation());
	if(newURL.empty()) {
	  itemcell->set_Command(ItemCell::DLERRORSTOP);
	  throw ItemCell::ITEM_EPROT;
	}
	if(urlconTemp.Parse_URL(newURL)) {
	  itemcell->set_URL_Container(urlconTemp);
	} else {
	  itemcell->set_Command(ItemCell::DLERRORSTOP);
	  throw ItemCell::ITEM_EPROT;
	}
      }
      //itemcell->get_Options_Lock();
      if(itemcell->ret_Options().ret_sync_with_URL()) {
	itemcell->set_Filename(itemcell->ret_URL_Container().ret_Filename());
      }
      //itemcell->release_Options_Lock();
      /*  
	  itemcell->set_Filename(httpcon.ret_contentLocation());
	  //itemcell->set_Filename_opt(httpcon.ret_contentLocation());
	  itemcell->ret_URL_Container().set_File('/'+httpcon.ret_contentLocation());
      */
    }

    if(itemcell->ret_Filename().empty()) {
      // when get directory, for example, http://hostname/~tujikawa/,
      // I have to choose the default file name.
      // In IIS servers, "Default.htm(l)" or "Default.asp" is commonly used,
      // and apache servers, "index.html" is probably used.
      // Aria can specify this name as prewrittenHTMLName

      string prewrittenHTMLName;
      if(itemcell->ret_Options().getPrewrittenHTMLType() == Options::PREWRITTEN_HTML_INDEX ||
	 itemcell->ret_Options().getPrewrittenHTMLName().empty()) {
	prewrittenHTMLName = "index.html";
      } else {
	prewrittenHTMLName = itemcell->ret_Options().getPrewrittenHTMLName();
      }

      itemcell->ret_URL_Container().set_File("/"+prewrittenHTMLName);
      itemcell->set_Filename(prewrittenHTMLName);
    }
    
    // check "Content-Encoding" field
    if(httpcon.isCompressEnabled()) {
      compressedFlag = true;
    }

    // check "Transfer-Encoding" field
    chunkedFlag = false;
    if(casefind(httpcon.getTransferEncoding(), "chunked") != string::npos) {
#ifdef DEBUG
      cerr << "chunked encoding" << endl;
#endif // DEBUG
      chunkedFlag = true;
    }
    if(httpcon.ret_ContentLength() == 0) {
      if(httpcon.ret_ContentLength() < startingbyte) {
	itemcell->Send_message_to_gui(_("Resume disabled"), MSG_DOWNLOAD_INFO);

	if(!itemcell->ret_documentroot_dir().empty() &&
	   itemcell->ret_Options().ret_no_redownload_HTTP_recurse()) {
	  itemcell->Send_message_to_gui(_("Assume download was completed"), MSG_DOWNLOAD_INFO);
	  throw ItemCell::ITEM_ESUCCESSALR;
	}
	if(itemcell->ret_Options().ret_use_no_redownload()) {
	  itemcell->set_Command(ItemCell::DLERRORSTOP);
	  throw ItemCell::ITEM_EINTER;
	}
      }
      if(itemcell->ret_Options().ret_use_size_lower_limit() &&
	 (unsigned int)itemcell->ret_Options().ret_size_lower_limit() > itemcell->ret_Size_Total()) {
	itemcell->Send_message_to_gui(_("Download aborted due to the file size limitation"), MSG_DOWNLOAD_INFO);
	itemcell->set_Command(ItemCell::DLSTOP);
	throw ItemCell::ITEM_EINTER;
      }
      itemcell->set_Size_Total(0);
      startingbyte = 0; // added
    } else if(httpcon.ret_ContentLength() == startingbyte && startingbyte != 0) {
      //すでにダウンロード済
      // download has been competed
      report = "'"+itemcell->ret_Filename()+"'"+_(" is already downloaded");
      itemcell->Send_message_to_gui(report, MSG_DOWNLOAD_INFO);
      
      itemcell->set_Size_Total(startingbyte);
      itemcell->set_Size_Current(startingbyte);
      itemcell->Send_status();
      //Send_message_to_gui(_("connection closed"), MSG_DOWNLOAD_INFO);
      //session_counter = 1;
      //return ItemCell::DLSUCCESSALR;
      throw ItemCell::ITEM_ESUCCESSALR;
    } else {
      bool resume_allowed = true;
      if(httpcon.ret_ContentLength() < startingbyte &&
	 httpcon.ret_HTTP_Status() != PartialContent) {
	// ファイルサイズが異なる
	// size mismatch
	itemcell->Send_message_to_gui(_("Size of local file is larger than remote file's one. Download again"), MSG_DOWNLOAD_ERROR);
	startingbyte = 0;
	itemcell->set_Size_Total(httpcon.ret_ContentLength());
      } else if(startingbyte > 0 && httpcon.ret_HTTP_Status() == OK) {
	// レジューム不可
	// www server does not support resume
	resume_allowed = false;
	itemcell->Send_message_to_gui(_("Resume disabled"), MSG_DOWNLOAD_INFO);
	if(!itemcell->ret_documentroot_dir().empty() &&
	   itemcell->ret_Options().ret_no_redownload_HTTP_recurse()) {
	  itemcell->Send_message_to_gui(_("Assume download was completed"), MSG_DOWNLOAD_INFO);
	  throw ItemCell::ITEM_ESUCCESSALR;
	}
	if(itemcell->ret_Options().ret_use_no_redownload()) {
	  itemcell->set_Command(ItemCell::DLERRORSTOP);
	  throw ItemCell::ITEM_EINTER;
	}
	startingbyte = 0;
	itemcell->set_Size_Total(httpcon.ret_ContentLength());
      } else {// modified 2001/5/20
	itemcell->set_Size_Total(httpcon.ret_ContentLength());
      }
      // limit file size
      if(itemcell->ret_Size_Total() != 0 &&
	 itemcell->ret_Options().ret_use_size_lower_limit() &&
	 (unsigned int)itemcell->ret_Options().ret_size_lower_limit() > itemcell->ret_Size_Total()) {
	itemcell->Send_message_to_gui(_("Download aborted due to the file size limitation"), MSG_DOWNLOAD_INFO);
	itemcell->set_Command(ItemCell::DLSTOP);
	throw ItemCell::ITEM_EINTER;
      }
	//} else {
      //itemcell->set_Size_Total(httpcon.ret_ContentLength()+startingbyte);
      //}
      if (itemcell->ret_Options().ret_Divide() > 1 && resume_allowed &&
	 (!itemcell->Is_current_session_valid() ||
	  (!itemcell->ret_current_session().Is_nodown() &&
	  !itemcell->ret_current_session().Is_getkeylink() &&
	  !itemcell->ret_current_session().Is_getkeylink_force()))) {
	itemcell->set_Size_Current(0);
	// check whether or not remote server suports resume
	itemcell->Send_message_to_gui(_("Checking whether server supports resuming"), MSG_DOWNLOAD_INFO);
	
	//sock.reopen();
	sock.Shutdown(2);

	// connect to www server
	establishConnection(sock);
	/*
	if(itemcell->ret_URL_Container().ret_Protocol() == "ftp:") {
	  Make_TCP_connection(sock,
			      itemcell->ret_Options().ret_ftp_proxy().ret_Server(),
			      itemcell->ret_Options().ret_ftp_proxy().ret_Port());
	} else if(itemcell->ret_Options().ret_use_http_proxy() &&
	   !itemcell->ret_Options().ret_http_proxy().ret_Server().empty()) {      
	  Make_TCP_connection(sock,
			      itemcell->ret_Options().ret_http_proxy().ret_Server(),
			      itemcell->ret_Options().ret_http_proxy().ret_Port());
	} else {
	  Make_TCP_connection(sock, 
			      itemcell->ret_URL_Container().ret_Hostname(),
			      itemcell->ret_URL_Container().ret_Port());
	}
	*/
	// send GET request to www server]
	////backup downm
	Options::DownloadMethodType downm_temp = itemcell->ret_Options().ret_downm_type();
      	itemcell->ret_Options().set_downm_type(Options::DOWNM_ALWAYSRESUME);
	Send_Request(sock, 1);
	itemcell->ret_Options().set_downm_type(downm_temp);
	// get HTTP header
	Get_HTTP_header(sock, httpheaderlist);
	// analyze HTTP header and store it to class HTTPcontainer
	HTTPcontainer httpcon_tmp;
	httpcon_tmp.Parse_HTTP_header(httpheaderlist);
	int http_status = httpcon_tmp.ret_HTTP_Status();
	
	if(http_status !=  PartialContent) {
	  // split download is not allowed
	  itemcell->Send_message_to_gui(_("Resume disabled"), MSG_DOWNLOAD_ERROR);
	  itemcell->Send_message_to_gui(_("Starting normal download instead"), MSG_DOWNLOAD_INFO);
	} else {
	  // OK, ready to split downloading!
	  itemcell->Send_message_to_gui(_("Splitting file..."), MSG_DOWNLOAD_INFO);
	  if(Create_partial_item_entry(itemcell->ret_Options().ret_Divide(), itemcell->ret_Size_Total())) {
	    itemcell->set_Status(ItemCell::ITEM_DOWNLOAD_PARTIAL);
	    itemcell->Send_status();
	    itemcell->set_Command(ItemCell::DLPARTIALSUCCESS);
	    throw ItemCell::ITEM_EINTER;
	  } else {
	    itemcell->Send_message_to_gui(_("File is too small to be divided"), MSG_DOWNLOAD_ERROR);
	    itemcell->Send_message_to_gui(_("Starting normal download instead"), MSG_DOWNLOAD_INFO);
	  }
	}
      }
    }

    // start downloading
    Start_Download(sock, startingbyte);

    // fix this

    if(chunkedFlag) {
      httpheaderlist.clear();
      list<int> dfd;
      int retval = sock.is_readready(0, dfd);
      if(retval && sock.is_set(dfd)) {
	Get_HTTP_header(sock, httpheaderlist);
      }
    }

    throw ItemCell::ITEM_ESUCCESS;
  } catch (ItemCell::ItemErrorType err) {
    switch(err) {
    case ItemCell::ITEM_ESUCCESS:
      itemcell->set_Command(ItemCell::DLSUCCESS);
#ifdef HAVE_ZLIB
      if(compressedFlag
	 && !endwith(itemcell->ret_URL_Container().ret_File(), ".gz")
	 && !endwith(itemcell->ret_URL_Container().ret_File(), ".tgz")
	 && !endwith(itemcell->ret_URL_Container().ret_File(), ".gzip")
	 && !endwith(itemcell->ret_URL_Container().ret_File(), ".z")
	 && !endwith(itemcell->ret_URL_Container().ret_File(), ".Z")) {
	uncompressFile(itemcell->ret_Filename());
      }
#endif // HAVE_ZLIB
      return ItemCell::DLSUCCESS;
    case ItemCell::ITEM_ESUCCESSALR:
      sock.Shutdown(2);
      itemcell->Send_message_to_gui(_("Connection closed"), MSG_DOWNLOAD_INFO);
      itemcell->set_Command(ItemCell::DLSUCCESSALR);
      return ItemCell::DLSUCCESSALR;
    case ItemCell::ITEM_ENEXTSTAGE:
      if(itemcell->Is_current_session_valid() &&
	 itemcell->ret_current_session().Is_nodown()) {
	sock.Shutdown(2);
	itemcell->Send_message_to_gui(_("Connection closed"), MSG_DOWNLOAD_INFO);
      }
      if(!itemcell->Execute_program()) {
	sock.Shutdown(2);
	itemcell->Send_message_to_gui(_("Connection closed"), MSG_DOWNLOAD_INFO);
	itemcell->Reset_Session_counter();// session_counter = 1
	itemcell->set_Command(ItemCell::DLERROR);
	return itemcell->ret_Dl_status();
      }
      if((itemcell->ret_current_session().Is_getkeylink() ||
	  itemcell->ret_current_session().Is_getkeylink_force()) &&
	 itemcell->ret_Retrieved_urlcon().ret_Protocol() == "ftp:") {
	// redirection to FTP protocol
	itemcell->set_URL_Container(itemcell->ret_Retrieved_urlcon());
	itemcell->ret_Options().set_use_authentication(false);//disable authentication for safety
      } else {
	if(itemcell->ret_svt().Is_valid(itemcell->ret_Session_counter()+1)) {
	  // forward currest URL to the next session
	  // so that it can be used as referer in the next session
	  itemcell->ret_svt().ret_session(itemcell->ret_Session_counter()+1).set_session_referer(sessionURL);
	}
	itemcell->ret_Options().set_Referer_Type(Options::REFERER_USER_DEFINED);
	itemcell->ret_Options().set_Referer(sessionURL.ret_URL());
      }
      itemcell->Inc_Session_counter();
      itemcell->set_Size_Total(0);
      itemcell->set_Size_Current(0);

      if(initialstatus == ItemCell::ITEM_CRCERROR ||
	 initialstatus == ItemCell::ITEM_EXECERROR ||
	 initialstatus == ItemCell::ITEM_INUSE_AGAIN ||
	 initialstatus == ItemCell::ITEM_DOWNLOAD_AGAIN) {
	itemcell->set_Command(ItemCell::DLAGAIN);
      } else {
	itemcell->set_Command(ItemCell::DLINTERNALAGAIN);
      }

      return itemcell->ret_Dl_status();
    case ItemCell::ITEM_EINTER:
      //session_counter = 1;
      itemcell->Reset_Session_counter();
      switch(itemcell->ret_Dl_status()) {
      case ItemCell::DLAGAIN:
      case ItemCell::DLSTOP:
      case ItemCell::DLERRORSTOP:
      case ItemCell::DLDELETEITEM:
      case ItemCell::DLDELETEITEMFILE:
      case ItemCell::DLPARTIALSUCCESS:
	sock.Shutdown(2);
	itemcell->Send_message_to_gui(_("Connection closed"), MSG_DOWNLOAD_INFO);
	break;
      case ItemCell::DLHALT:
      default:
	sock.Shutdown(2);
	break;
      }
      return itemcell->ret_Dl_status();
    case ItemCell::ITEM_ELOCATION:// fixed
    case ItemCell::ITEM_ESERVERCONFIG://fixed
    case ItemCell::ITEM_EIOFILE:
      itemcell->Reset_Session_counter();//session_counter = 1;
      itemcell->set_Command(ItemCell::DLERRORSTOP);
      itemcell->PERROR(err);
      sock.Shutdown(2);
      itemcell->Send_message_to_gui(_("Connection closed"), MSG_DOWNLOAD_INFO);
      return ItemCell::DLERRORSTOP;
    case ItemCell::ITEM_EPROT:// protocl error // modified
      itemcell->Reset_Session_counter();
      itemcell->PERROR(err);
      sock.Shutdown(2);
      itemcell->Send_message_to_gui(_("Connection closed"), MSG_DOWNLOAD_INFO);
      return itemcell->ret_Dl_status();
    default:
      itemcell->Reset_Session_counter();
      itemcell->set_Command(ItemCell::DLERROR);
      itemcell->PERROR(err);
      sock.Shutdown(2);
      itemcell->Send_message_to_gui(_("Connection closed"), MSG_DOWNLOAD_INFO);
      return itemcell->ret_Dl_status();
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
void RetrieveHTTP::Start_Download(const Socket& socket, unsigned int startingbyte)
{
  ofstream outfile;
  DownloadMode downloadmode = NORMAL_MODE;
  string line;

  Make_directory_if_needed();

  //ret_URL_Container().Parse_URL(ret_URL());
  string filename = itemcell->ret_Options().ret_Store_Dir()+itemcell->ret_Filename();
  string tempfilename;
  itemcell->Send_status();
  bool flag_trylock = false;

  try {
    if(itemcell->Is_current_session_valid() &&
       itemcell->ret_current_session().Is_nodown()) {
      throw ItemCell::ITEM_ENEXTSTAGE;
    }
    if(((itemcell->ret_Size_Total() == 0 || (itemcell->ret_Options().ret_use_http_proxy() && !itemcell->ret_Options().ret_http_proxy().ret_Server().empty()) &&
	itemcell->Is_current_session_valid() &&
	itemcell->ret_current_session().Is_getkeylink())) ||
       (itemcell->Is_current_session_valid() &&
       itemcell->ret_current_session().Is_getkeylink_force())) {
      tempfilename = filename+"."+itos(time(NULL))+itos((int)((float)100*random()/(RAND_MAX+1.0)));
      downloadmode = EMBEDED_URL_MODE;
      outfile.open(tempfilename.c_str(), ios::out|ios::trunc|ios::binary);
      startingbyte = 0;
    } else if(itemcell->Is_current_session_valid() &&
	      itemcell->ret_current_session().Is_addhref()) {
      tempfilename = filename+"."+itos(time(NULL))+itos((int)((float)100*random()/(RAND_MAX+1.0)));

      downloadmode = ADD_HREF_MODE;
      outfile.open(tempfilename.c_str(), ios::out|ios::trunc|ios::binary);
      startingbyte = 0;
    } else {
      if(!g_lockList->Try_lock(filename)) {
	itemcell->Send_message_to_gui(_("This file is locked. Aborting download"), MSG_DOWNLOAD_ERROR);
	itemcell->set_Command(ItemCell::DLERRORSTOP);
	throw ItemCell::ITEM_EINTER;
      } else {
	flag_trylock = true;
      }
	
      line = _("Starting download at ")+itos(startingbyte, true)+_(" bytes");
      itemcell->Send_message_to_gui(line, MSG_DOWNLOAD_INFO);
    
      if(startingbyte == 0) {
	// 新規にダウンロード開始
	outfile.open(filename.c_str(), ios::out|ios::trunc|ios::binary);
      } else {
	// レジュームする
	outfile.open(filename.c_str(), ios::out|ios::app|ios::binary);
      }
    }
    if(outfile.bad()) {
      throw ItemCell::ITEM_EIOFILE;
    }    
    itemcell->set_Size_Current(startingbyte);
    // modified 2001/5/20
    if(chunkedFlag) {
      // chunked encoding
      itemcell->set_previous_dl_size(Download_data_chunked(outfile, socket));
    } else {
      int flag = false;
      if(compressedFlag
	 && !endwith(itemcell->ret_URL_Container().ret_File(), ".gz")
	 && !endwith(itemcell->ret_URL_Container().ret_File(), ".tgz")
	 && !endwith(itemcell->ret_URL_Container().ret_File(), ".gzip")
	 && !endwith(itemcell->ret_URL_Container().ret_File(), ".z")
	 && !endwith(itemcell->ret_URL_Container().ret_File(), ".Z")) {
	flag = true;
      }
      itemcell->set_previous_dl_size(Download_data(outfile, socket, flag));
    }
    outfile.close();

    switch(downloadmode) {
    case EMBEDED_URL_MODE:
      {
	itemcell->Send_message_to_gui(_("Retrieving embedded URL"), MSG_DOWNLOAD_INFO);
	URLcontainer trueurlcon;
	list<string> keylink_list;
	URLcontainer orig_urlcon;
	orig_urlcon.Parse_URL(itemcell->ret_URL());
	for(list< vector<string> >::const_iterator itr = itemcell->ret_current_session().ret_keylink_list().begin();
	    itr != itemcell->ret_current_session().ret_keylink_list().end(); ++itr) {
	  string keylink = itemcell->ret_current_session().Create_URL(orig_urlcon, itemcell->ret_Retrieved_urlcon(), *itr);
	  if(keylink.size())
	    keylink_list.push_back(keylink);
	}
#ifdef HAVE_ZLIB
      if(compressedFlag
	 && !endwith(itemcell->ret_URL_Container().ret_File(), ".gz")
	 && !endwith(itemcell->ret_URL_Container().ret_File(), ".tgz")
	 && !endwith(itemcell->ret_URL_Container().ret_File(), ".gzip")
	 && !endwith(itemcell->ret_URL_Container().ret_File(), ".z")
	 && !endwith(itemcell->ret_URL_Container().ret_File(), ".Z")) {
	uncompressFile(tempfilename);
      }	
#endif // HAVE_ZLIB
	if(!trueurlcon.Retrieve_embedded_URL(keylink_list, tempfilename, itemcell->ret_URL_Container())) {
	  unlink(tempfilename.c_str());
	  throw ItemCell::ITEM_ECANTFINDURL;
	}
	unlink(tempfilename.c_str());
	itemcell->set_Retrieved_urlcon(trueurlcon);
	throw ItemCell::ITEM_ENEXTSTAGE;
      }
    case ADD_HREF_MODE:
      {
	itemcell->Send_message_to_gui(_("Finding hyperlink"), MSG_DOWNLOAD_INFO);
#ifdef HAVE_ZLIB
	if(compressedFlag
	   && !endwith(itemcell->ret_URL_Container().ret_File(), ".gz")
	   && !endwith(itemcell->ret_URL_Container().ret_File(), ".tgz")
	   && !endwith(itemcell->ret_URL_Container().ret_File(), ".gzip")
	   && !endwith(itemcell->ret_URL_Container().ret_File(), ".z")
	   && !endwith(itemcell->ret_URL_Container().ret_File(), ".Z")) {
	  uncompressFile(tempfilename);
	}
#endif // HAVE_ZLIB
	g_itemList->Find_Hyperlink_from_file(tempfilename, itemcell->ret_URL_Container().ret_Protocol()+"//"+itemcell->ret_URL_Container().ret_Hostname()+itemcell->ret_URL_Container().ret_Dir(), ItemList::FINDHREF_ADD);
	itemcell->Send_message_to_gui(_("Hyperlink retrieval complete"), MSG_DOWNLOAD_INFO);
	unlink(tempfilename.c_str());
	throw ItemCell::ITEM_ENEXTSTAGE;
      }
    case NORMAL_MODE:
    default:
      {
	if(itemcell->ret_Size_Current() != itemcell->ret_Size_Total() && itemcell->ret_Size_Total() != 0) {
	  bool flag = true;
	  if(flag) {
	    itemcell->Send_message_to_gui(_("No match size"), MSG_DOWNLOAD_ERROR);
	    itemcell->set_Command(ItemCell::DLERROR);
	    throw ItemCell::ITEM_EPROT;
	  }
	} else {
	  itemcell->set_Size_Total(itemcell->ret_Size_Current());
	}
      }
    }
  } catch (ItemCell::ItemErrorType err) {
    outfile.close();
    if(downloadmode == EMBEDED_URL_MODE || downloadmode == ADD_HREF_MODE) {
      unlink(tempfilename.c_str());
    }
#ifdef HAVE_ZLIB
    if(compressedFlag
       && !endwith(itemcell->ret_URL_Container().ret_File(), ".gz")
       && !endwith(itemcell->ret_URL_Container().ret_File(), ".tgz")
       && !endwith(itemcell->ret_URL_Container().ret_File(), ".gzip")
       //&& !endwith(itemcell->ret_URL_Container().ret_File(), ".z")
       && !endwith(itemcell->ret_URL_Container().ret_File(), ".Z")) {
      unlink(filename.c_str());
    }
#endif // HAVE_ZLIB
    if(flag_trylock) g_lockList->Unlock(filename);

    throw err;
  }
  if(flag_trylock) g_lockList->Unlock(filename);
}

#ifdef HAVE_ZLIB
void RetrieveHTTP::uncompressFile(const string& filename)
{
  string dir = itemcell->ret_Options().ret_Store_Dir();
  //string filename = itemcell->ret_Filename();
  string tempFilename = filename+"."+itos(time(NULL))+itos((int)((float)100*random()/(RAND_MAX+1.0)));

  // check error
  if(rename((dir+filename).c_str(), (dir+tempFilename).c_str()) < 0) {
    itemcell->Send_message_to_gui(_("Failed to uncompress data"), MSG_DOWNLOAD_ERROR);
    return;
  }

  // open compressed file
  gzFile *infile = (gzFile *)gzopen((dir+tempFilename).c_str(), "r");

  FILE *outfile = fopen((dir+filename).c_str(), "w");

  while(1) {
    char buffer[4096];
    int size = gzread(infile, buffer, sizeof(buffer));

    if(size == 0) break;
    // check error
    if(size == -1) {
      itemcell->Send_message_to_gui(_("Failed to uncompress data"), MSG_DOWNLOAD_ERROR);
      break;
    }
    int wsize = fwrite(buffer, size, 1, outfile);
    if(wsize < size) {
      if(ferror(outfile)) {
	itemcell->Send_message_to_gui(_("Failed to uncompress data"), MSG_DOWNLOAD_ERROR);
	break;
      }
    }
  }
  fclose(outfile);
  gzclose(infile);

  // check error
  if(unlink((dir+tempFilename).c_str()) < 0) {
    itemcell->Send_message_to_gui(_("Failed to uncompress data"), MSG_DOWNLOAD_ERROR);
    return;
  }
}
#endif // HAVE_ZLIB

// support "Transfer-Encoding: chunked"
#define PCSPLIT 10
int
RetrieveHTTP::Download_data_chunked(ofstream& outfile, const Socket& socket)
{
  const int databuf_size_def = 4096;
  int databuf_size;
  itemcell->get_Options_Lock();
  if(databuf_size_def < itemcell->ret_Options_opt().ret_speed_limit()*1024/PCSPLIT || itemcell->ret_Options_opt().ret_speed_limit() < 0.1) {
    databuf_size = databuf_size_def;
  } else {
    databuf_size = (int)(itemcell->ret_Options_opt().ret_speed_limit()*1024)/PCSPLIT;
  }
  itemcell->release_Options_Lock();
  char databuf[databuf_size_def];
  const int writebuf_size = 12288;
  char *writebuf = new char[writebuf_size];
  char *writebuf_tail = writebuf;
  int size = 0;
  string line;
  struct timeval start_time, end_time, initial_time;
  struct timezone tz_dummy;
  unsigned int timedout = itemcell->ret_Options().ret_Timed_Out();
  unsigned int elapsed_size = 0;
  float speed = 0.0;
  float avgSpeed = 0.0;
  list<int> fd_list;
  unsigned int startsize = itemcell->ret_Size_Current();

  gettimeofday(&initial_time, &tz_dummy);
  start_time = initial_time;
  itemcell->Send_status();

  try {
    while(1) {
      fd_list.clear();
      fd_list.push_back(itemcell->ret_Desc_r());
      int retval = socket.is_readready(timedout, fd_list);
      if(retval && Socket::is_set(itemcell->ret_Desc_r(), fd_list)) {
	ItemCommand itemcommand;

	read(itemcell->ret_Desc_r(), &itemcommand, sizeof(ItemCommand));

	//set_Command(Process_command(itemcommand));
	itemcell->set_Command(itemcommand);
	if(itemcell->ret_Dl_status() != ItemCell::DLCHANGE) {
	  outfile.write(writebuf, writebuf_tail-writebuf);
	  throw ItemCell::ITEM_EINTER;
	} else {
	  // change buffer size;
	  if(databuf_size_def < itemcell->ret_Options_opt().ret_speed_limit()*1024/PCSPLIT || itemcell->ret_Options_opt().ret_speed_limit() < 0.1 ) {
	    databuf_size = databuf_size_def;
	  } else {
	    //cerr << "bufsize" << (int)(options_opt.ret_speed_limit()*1024)/PCSPLIT << endl;
	    databuf_size = (int)(itemcell->ret_Options_opt().ret_speed_limit()*1024)/PCSPLIT;
	  }
	}
      } else if(retval && socket.is_set(fd_list)) {
	string chunkSizeStr;
	while(1) {
	  char ch;
	  int retval = socket.Recv(&ch, 1);
	  if(retval <= 0) {
	    throw ItemCell::ITEM_EIO;
	  }
	  if(ch != '\r') {
	    chunkSizeStr += ch;
	  } else {
	    // read '\n'
	    socket.Recv(&ch, 1);
	    break;
	  }
	}
	// now chunk-size is chunkSizeStr as string
	int chunkSize = strtol(chunkSizeStr.c_str(), NULL, 16);
#ifdef DEBUG
	cerr << "ChunkSize: " << chunkSizeStr << endl;
	cerr << "           " << chunkSize << endl;
#endif // DEBUG
	if(chunkSize == 0) {
	  // download finished
	  if(itemcell->Is_Partial()) {
	    unsigned int diff = itemcell->ret_Size_Current()-itemcell->ret_Size_Total();
	    memcpy(writebuf_tail, databuf, size-diff);
	    writebuf_tail += size-diff;
	    itemcell->set_Size_Current(itemcell->ret_Size_Total());
	  } else {
	    memcpy(writebuf_tail, databuf, size);
	    writebuf_tail += size;
	  }
	  itemcell->Send_status(avgSpeed, avgSpeed);
	  
	  outfile.write(writebuf, writebuf_tail-writebuf);
	    
	  writebuf_tail = writebuf;
	  
	  if(outfile.bad() || outfile.fail()) {
	    throw ItemCell::ITEM_EIOFILE;
	  }	  
	  break;
	}

	// read chuckSize bytes
	while(chunkSize) {
	  int chunkBufferSize;
	  if(databuf_size < chunkSize) {
	    chunkBufferSize = databuf_size;
	  } else {
	    chunkBufferSize = chunkSize;
	  }
	  size = socket.Recv(databuf, chunkBufferSize);
	  if(size < 0) {
	    throw ItemCell::ITEM_EIO;
	  }
	  // decrease chunkSize by size
	  chunkSize -= size;

	  itemcell->set_Size_Current(itemcell->ret_Size_Current()+size);
	  gettimeofday(&end_time, &tz_dummy);
	  elapsed_size += size;
	  unsigned long elapsed = (end_time.tv_sec-start_time.tv_sec)*1000000+end_time.tv_usec-start_time.tv_usec;
	  
	  if(elapsed > 1000000) {
	    speed = ((elapsed_size/1024.0)/(elapsed/1000000.0)+speed)/2;
	    
	    unsigned long elapsedTotal = (end_time.tv_sec-initial_time.tv_sec)*1000000+end_time.tv_usec-initial_time.tv_usec;
	    avgSpeed = ((itemcell->ret_Size_Current()-startsize)/1024.0/(elapsedTotal/1000000.0));
	    itemcell->Send_status(speed, avgSpeed);
	    if(avgSpeed < 0) avgSpeed = speed;// fix this
	    gettimeofday(&start_time, &tz_dummy);
	    elapsed_size = 0;
	  } else {
	    //cerr << elapsed_size << endl;
	    if(itemcell->ret_Options_opt().ret_speed_limit() > 0.0 &&
	       elapsed_size >= (unsigned int)(itemcell->ret_Options_opt().ret_speed_limit()*1024*0.8)) {  	       
	      struct timeval tv;
	      tv.tv_sec = 0;
	      tv.tv_usec = 1000000-elapsed;  
	      select(itemcell->ret_Desc_r()+1, NULL, NULL, NULL, &tv);
	    }
	  }
	  memcpy(writebuf_tail, databuf, size);
	  writebuf_tail += size;
	  if(writebuf_size-(writebuf_tail-writebuf) < databuf_size+databuf_size_def) {
	    outfile.write(writebuf, writebuf_tail-writebuf);
	    writebuf_tail = writebuf;
	  }
	  if(outfile.bad() || outfile.fail()) {
	    throw ItemCell::ITEM_EIOFILE;
	  }
	}
	// Read "\r\n";
	char crlf[2];
	int retval = socket.Recv(crlf, sizeof(crlf));
	if(retval <= 0) {
	  throw ItemCell::ITEM_EIO;
	}
      } else {
	// timed out
	outfile.write(writebuf, writebuf_tail-writebuf);// buffer flush
	throw ItemCell::ITEM_ETIMEDOUT;
      }
    }
  } catch (ItemCell::ItemErrorType err) {
    delete [] writebuf;
    // added 2001/5/20
    itemcell->set_previous_dl_size(itemcell->ret_Size_Current()-startsize);
    throw err;
  }
  delete [] writebuf;
  return itemcell->ret_Size_Current()-startsize;
}

void RetrieveHTTP::establishConnection(Socket& sock)
{
  if(itemcell->ret_URL_Container().ret_Protocol() == "ftp:") {
    Make_TCP_connection(sock,
			itemcell->ret_Options().ret_ftp_proxy().ret_Server(),
			itemcell->ret_Options().ret_ftp_proxy().ret_Port());
  } else if(itemcell->ret_Options().ret_use_http_proxy() &&
	    !itemcell->ret_Options().ret_http_proxy().ret_Server().empty()) {      
    Make_TCP_connection(sock,
			itemcell->ret_Options().ret_http_proxy().ret_Server(),
			itemcell->ret_Options().ret_http_proxy().ret_Port());
  } else {
    Make_TCP_connection(sock, 
			itemcell->ret_URL_Container().ret_Hostname(),
			itemcell->ret_URL_Container().ret_Port());
  }
}
