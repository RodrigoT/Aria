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

// $Id: ServerTemplate.cc,v 1.8 2001/05/19 18:38:59 tujikawa Exp $

#include "ServerTemplate.h"
#include <assert.h>

ServerTemplate::ServerTemplate(const string& name_in,
			       const string& comment_in,
			       const list<string>& server_name_list_in,
			       const list<string>& ignore_server_name_list_in,
			       const list<string>& ignore_extension_list_in,
			       const string& option_string,
			       const vector<Session>& session_vector_in)
{

  name = name_in;
  comment = comment_in;
  valid = false;//true;
  ignorefileerr = false;
  bad_flag = false;

  try {
    server_name_list = server_name_list_in;

    ignore_server_name_list = ignore_server_name_list_in;
    ignore_extension_list = ignore_extension_list_in;
    session_vector = session_vector_in;
    //Process_session_vector(session_vector);
    Process_option_string(option_string);
  } catch (int err) {
    cerr << "error occurred in <server> tag" << endl;
    bad_flag = true;
  }
}

ServerTemplate::ServerTemplate()
{
  valid = false;
}

bool ServerTemplate::bad() const
{
  return bad_flag;
}

void ServerTemplate::Process_option_string(string option_string)
{
  while(option_string.size()) {
    string token = Token_splitter(option_string);
    if(token == "nofile") {
      ignorefileerr = true;
    }
  }
}

/*
void ServerTemplate::Process_session_vector(vector<Session>& session_vector)
{
  map<string, int> label_map;
  int session_counter = 1;
  for(vector<Session>::iterator itr = session_vector.begin();
      itr != session_vector.end(); ++itr) {
    label_map[itr->ret_label()] = session_counter;
    ++session_counter;
  }
  for(vector<Session>::iterator itr = session_vector.begin();
      itr != session_vector.end(); ++itr) {
    // undefined label is 0
    itr->set_cond_session_succ(label_map[itr->ret_cond_succ()]);
    itr->set_cond_session_fail(label_map[itr->ret_cond_fail()]);
  }
}
*/
/*
void ServerTemplate::Process_hostname(string hostname)
{
  host = Token_splitter(hostname, ":");
  port = stoi(hostname);
  if(port <= 0) {
    port = 80;
  }
}
*/

const string& ServerTemplate::ret_template_name() const
{
  return name;
}

const string& ServerTemplate::ret_template_comment() const
{
  return comment;
}
/*
string ServerTemplate::MyToken_splitter(string& line)
{
  unsigned int start_pos = line.find_first_not_of(" \t\n\r");
  if(start_pos == string::npos) {
    line.erase();
    return "";
  }
  string token;
  if(line.at(start_pos) == '$') { // reserved word
    // $(****)
    unsigned int end_pos = line.find(')', start_pos);
    if(end_pos == string::npos) {
      cerr << "error: paren mismatch: '" << line.substr(start_pos) << "'" << endl;
      throw 0;
    }
    token = line.substr(start_pos, end_pos-start_pos+1);
    line.erase(0, end_pos+1);
  } else { // unreserved word
    unsigned int end_pos = line.find_first_of("$", start_pos);
    if(end_pos == string::npos) {
      end_pos = line.size();
    }
    token = line.substr(start_pos, end_pos-start_pos);
    line.erase(0, end_pos);
  }
  return token;
}
*/

bool ServerTemplate::Is_in_entry(const string& server_name, const string& filename) const
{
  for(list<string>::const_iterator server_ptr = server_name_list.begin();
      server_ptr != server_name_list.end(); ++server_ptr) {
    if(server_name.find(*server_ptr) != string::npos) {
      //bool flag = true; ////modified 2001/3/1
      for(list<string>::const_iterator ign_server_ptr = ignore_server_name_list.begin(); ign_server_ptr != ignore_server_name_list.end(); ++ign_server_ptr) {
	if(server_name.find(*ign_server_ptr) != string::npos) {
	  // found in ignore server list
	  //flag = false;
	  //break;
	  return false;
	}
      }
      //if(!flag) return false;
      for(list<string>::const_iterator ign_ext_ptr = ignore_extension_list.begin(); ign_ext_ptr != ignore_extension_list.end(); ++ign_ext_ptr) {
	if(endwith(filename, *ign_ext_ptr)) {
	  //flag = false;
	  //break;
	  return false;
	}
	//if(jtr->size() < filename.size()) {
	//if(filename.substr(filename.size()-jtr->size()) == *jtr) flag = false;
	//break;
	//}
      }
      //if(flag) return true;
      return true;
    }
  }
  return false;
}

const list<string>& ServerTemplate::ret_server_name_list() const
{
  return server_name_list;
}

const list<string>& ServerTemplate::ret_ignore_server_name_list() const
{
  return ignore_server_name_list;
}

int ServerTemplate::ret_port() const
{
  return port;
}

bool ServerTemplate::Is_valid() const
{
  return valid;
}

bool ServerTemplate::Is_valid(std::size_t session_count) const
{
  return valid && (session_vector.size() >= session_count);
}
    
Session& ServerTemplate::ret_session(std::size_t session_count)
{
	assert(session_vector.size() > 0);
  if(session_count > session_vector.size()) return session_vector[0];
  else 
    return session_vector[session_count-1];
}

/*const vector<Session>& ServerTemplate::ret_session_vector() const
{
  return session_vector;
}*/

int ServerTemplate::ret_total_session() const
{
  return session_vector.size();
}

void ServerTemplate::set_valid(bool flag)
{
  valid = flag;
}

