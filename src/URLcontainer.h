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

// $Id: URLcontainer.h,v 1.27 2001/11/19 16:26:21 tujikawa Exp $

//definition of class URLcontainer

#ifndef _URLCONTAINER_H_
#define _URLCONTAINER_H_
#include <iostream>
#include <string>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <ctype.h>
#include <fstream>
#include "aria.h"
#include "utils.h"
//#include "ServerTemplateList.h"
using namespace std;

#define DEFAULT_WWW_PORT 80
#define DEFAULT_FTP_PORT 21
#define DEFAULT_HTTPS_PORT 443

class ProtocolList {
public:
  string protocol;
  int default_port;
};

class URLcontainer {
private:
  string protocol;
  string host;
  int port;
  string dir;
  string file;
  string query;

  // thanks to Matthias Babisch
  string username;
  string passwd;

  bool Is_supported_protocol(const string& protocol);
  void Extract_protocol(string& url);
  void Extract_userpasswd(string& url);
  void Extract_host(string& url);
  void Extract_dir(string& url);
  void Extract_file(string& url);

  bool bad_flag;
public:
  URLcontainer();
  ~URLcontainer();
  static string Find_URL(string& text, bool ws_quark = false);
  static string Find_URL_ext(string& text);
  static list<string> Unfold_URL(const string& src_url);
  static string Find_HREF(string& text, string base);
  static string Find_HREF_strict(string& text, string base_url);
  static string URL_Encode(const string& src_string);
  static string URL_Decode(const string& src_string);
  static bool Is_reserved(char ch);
  // read private member valiables
  string ret_URL() const;
  const string& ret_Protocol() const;
  const string& ret_Username() const;
  const string& ret_Password() const;
  const string& ret_Hostname() const;
  const string& ret_Dir() const;
  const string& ret_File() const;
  const string& ret_Query() const;
  string ret_Extension() const;
  string ret_Filename() const;
  unsigned int ret_Port() const;

  // write to private member valiables
  void set_Protocol(const string& protocol);
  void set_Hostname(const string& hostname);
  void set_Dir(const string& dir);
  void set_File(const string& file);
  void set_Port(unsigned int port);
  void force_bad();
  bool bad() const;
  void clear();
  bool Retrieve_embedded_URL(const list<string>& keylink_list, const string& filename, const URLcontainer& urlcon);
  bool Examine_keylinks(const string& line,
			const list<string>& keylink_list,
			const URLcontainer& urlcon);
  bool Evaluate_javascript(ifstream& infile,
			   const string& target,
			   const list<string>& keylink_list,
			   const URLcontainer& urlcon);

  bool Parse_URL(string url);

  enum EmbedParseStatusType {
    EMBEDPARSE_FOUND,
    EMBEDPARSE_EOF
  };
  enum URLconErrorType {
    URLCON_EINVALIDURL,
    URLCON_EUNSUPP
  };
};

#endif // _URLCONTAINER_H_
