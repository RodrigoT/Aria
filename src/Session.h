//  Aria - yet another download tool
//  Copyright (C) 2000, 2002 Tatsuhiro Tsujikawa
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

// $Id: Session.h,v 1.10 2002/04/03 13:33:52 tujikawa Exp $

#ifndef _SESSION_H_
#define _SESSION_H_
#include <string>
#include <vector>
#include <list>
#include <iostream>
#include "utils.h"
#include "URLcontainer.h"
#include "Command.h"

using namespace std;

#define SOPTION_NODOWN "nodown"
#define SOPTION_NOCOOKIE "nocookie"
#define SOPTION_DELETECOOKIE "deletecookie"
#define SOPTION_GETKEYLINK "getkeylink"
#define SOPTION_GETKEYLINKF "getkeylinkf"
#define SOPTION_ADDHREF "addhref"
#define SOPTION_READCONFIG "readconfig"
#define SOPTION_NORESUME "noresume"

class Session
{
private:
  string label;
  vector<string> alt_get;
  vector<string> alt_referer;
  URLcontainer session_urlcon;//added 2001/3/9
  int referer_type;
  list< vector<string> > keylink_list;
  Command command;

  int cond_type; // type of condition
  string cond_str_find; // string to find
  string cond_succ; // destination label when condition is true
  string cond_fail; // destination laben when condition is false
  int cond_session_succ;
  int cond_session_fail;

  int post_offset_size;
  string post_offset_string;

  bool nodown;
  bool nocookie;
  bool noresume;
  bool deletecookie;
  bool getkeylink;
  bool getkeylinkf;
  bool addhref;
  bool readconfig;
  bool bad_flag;
public:

  Session(const string& label,
	  const string& get_string,
	  const string& referer_string,
	  const string& keylink_string,
	  const string& command_string,
	  const string& command_status_string,
	  const string& cond,
	  const string& cond_succ,
	  const string& cond_fail,
	  int post_offset_size,
	  const string& post_offset_string,
	  const string& options);
  Session();

  void Process_get_string(string get_string);
  void Process_referer_string(string referer_string);
  void Process_option_string(string option_string);
  void Process_keylink_string(string keylink_string);
  int Process_cond_string(string cond); // return cond_type
  bool bad() const;
  static int interpret(const string& word);
  bool Is_reserved(const string& token);
  string MyToken_splitter(string& line);
  const list< vector<string> >& ret_keylink_list() const;
  const string& ret_label() const;
  const string& ret_cond_succ() const;
  const string& ret_cond_fail() const;
  void set_cond_session_succ(int session_counter);
  void set_cond_session_fail(int session_counter);
  int ret_cond_session_succ() const;
  int ret_cond_session_fail() const;

  int ret_pre_offset_size() const;
  const string& ret_pre_offset_string() const;
  int ret_post_offset_size() const;
  const string& ret_post_offset_string() const;

  int ret_referer_type() const;
  const vector<string>& ret_referer_vector() const;
  const vector<string>& ret_get_vector() const;

  void set_session_referer(const URLcontainer& urlcon);

  bool set_session_referer(const string& url);

  string ret_session_referer() const;

  string Create_URL_from_referer_vector(const URLcontainer& orig_urlcon, const URLcontainer& retrieved_urlcon);

  string Create_URL_from_get_vector(const URLcontainer& org_urlcon, const URLcontainer& retrieved_urlcon);

  string Create_URL(const URLcontainer& orig_urlcon, const URLcontainer& retrieved_urlcon, const vector<string>& string_vector);

  enum SessionExecStatus {
    SESSION_EXEC_SUCC,
    SESSION_EXEC_FAIL,
    SESSION_EXEC_IGN,
    SESSION_EXEC_IGN_FATAL
  };
  bool Is_program_avaiable() const;
  string ret_program_line(const string& filename, const string& save_dir, const URLcontainer& urlcon) const;
  SessionExecStatus Execute_program(const string& filename, const string& save_dir, const URLcontainer& urlcon) const;

  bool Is_noresume() const;
  bool Is_getkeylink() const;
  bool Is_getkeylink_force() const;
  bool Is_nodown() const;
  bool Is_nocookie() const;
  bool Is_addhref() const;
  bool Is_deletecookie() const;
  bool Is_readconfig() const;

  enum {
    REF_URL,
    REF_STRING,
    REF_NONE,
    REF_NOSEND,
    REF_NOTSPECIFIED
  };
  
  enum ErrorType {
    EPARSE,
    TOKEN_ERROR
  };

  enum MacroAtomType {
    VAR_URL = 0,
    VAR_HOST = 1,
    VAR_PARTIAL_HOST = 2,
    VAR_DIR = 3,
    VAR_PARTIAL_DIR = 4,
    VAR_FILE = 5,
    VAR_QUERY = 6,
    VAR_PROTOCOL = 7,
    VAR_RETRIEVED_URL = 100,
    VAR_RETRIEVED_HOST = 101,
    VAR_RETRIEVED_PARTIAL_HOST = 102,
    VAR_RETRIEVED_DIR = 103,
    VAR_RETRIEVED_PARTIAL_DIR = 104,
    VAR_RETRIEVED_FILE = 105,
    VAR_RETRIEVED_QUERY = 106,
    VAR_RETRIEVED_PROTOCOL = 107,
    VAR_SESSION_URL = 200,
    VAR_SESSION_HOST = 201,
    VAR_SESSION_PARTIAL_HOST = 202,
    VAR_SESSION_DIR = 203,
    VAR_SESSION_PARTIAL_DIR = 204,
    VAR_SESSION_FILE = 205,
    VAR_SESSION_QUERY = 206,
    VAR_SESSION_PROTOCOL = 207,
    UNRESERVED = 999
  };

  enum {
    COND_HTTP_OK = 0,
    COND_HTTP_ERROR = 1,
    COND_HTTP_NOTFOUND = 2,
    COND_HTTP_FORBIDDEN = 3,
    COND_HTTP_MOVED = 4,
    COND_FIND_STRING = 10
  };
};
#endif //_SESSION_H_
