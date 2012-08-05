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

// $Id: ServerTemplateList.cc,v 1.10 2002/03/16 14:13:00 tujikawa Exp $

#include "ServerTemplateList.h"

ServerTemplateList::ServerTemplateList()
{
  ServerTemplate dummy;
  svt_list.push_back(dummy);
  pthread_mutex_init(&svt_list_lock, NULL);
}

ServerTemplateList::~ServerTemplateList()
{
}
/*
void ServerTemplateList::push_front(const ServerTemplate& svt_in)
{
  svt_list.push_front(svt_in);
}
*/

void ServerTemplateList::set_valid_safely(vector<ServerTemplate>::iterator svt_itr, bool flag)
{
  pthread_mutex_lock(&svt_list_lock);
  svt_itr->set_valid(flag);
  pthread_mutex_unlock(&svt_list_lock);
}

const ServerTemplate& ServerTemplateList::search(const string& server_name, const string& filename)
{
  pthread_mutex_lock(&svt_list_lock);
  for(vector<ServerTemplate>::const_iterator itr = svt_list.begin();
      itr != svt_list.end()-1; ++itr) {
    if(itr->Is_valid() && itr->Is_in_entry(server_name, filename)) {
      pthread_mutex_unlock(&svt_list_lock);
      return *itr;
    }
  }
  pthread_mutex_unlock(&svt_list_lock);
  return svt_list.back();
}

bool ServerTemplateList::Is_reserved_in_session(const string& tag)
{
  if(tag == "<get>" || tag == "</get>" ||
     tag == "<referer>" || tag == "</referer>" ||
     tag == "<option>" || tag == "</option>" ||
     tag == "<keylink>" || tag == "</keylink>" ||
     tag == "<execute>" || tag == "</execute>" ||
     tag == "<execute-status>" || tag == "</execute-status>" ||
     tag == "<session>" || tag == "</session>" ||
     tag == "<sequence>" || tag == "</sequence>" ||
     tag == "<post-offset-size>" || tag == "</post-offset-size>" ||
     tag == "<post-offset-string>" || tag == "</post-offset-string>") {
    return true;
  } else {
    return false;
  }
}

bool ServerTemplateList::Is_reserved(const string& tag)
{

  if(tag == "<identifier>" || tag == "</identifier>" ||
     tag == "<target-servers>" || tag == "</target-servers>" ||
     tag == "<ignore-servers>" || tag == "</ignore-servers>" ||
     tag == "<ignore-extensions>" || tag == "</ignore-extensions>" ||
     tag == "<sequence>" || tag == "</sequence>" ||
     tag == "<comment>" || tag == "</comment>" ||
     tag == "<server>" || tag == "</server>" || Is_reserved_in_session(tag)) {
    return true;
  } else {
    return false;
  }
}

bool ServerTemplateList::Read_config_file(const string& filename)
{
  ifstream infile;
  string line;
  string tag;
  StringHash server_map;

  try{
    pthread_mutex_lock(&svt_list_lock);
    svt_list.clear();
    infile.open(filename.c_str(), ios::in);//ios::skipws|ios::in);
    if(infile.bad()) throw EOPEN;
    //map<string, string> server_map;
    //vector<Session> session_vector;

    while(infile.good()) {
      try {
	tag = get_next_tag(infile);
	if(!Is_reserved(tag)) {
	  throw tag;
	}
      } catch (int err) {
	break;
      }

      if(tag == SERVER) {
	server_map.clear();
	vector<Session> session_vector;

	while(1) {
	  tag = get_next_tag(infile);
	  if(!Is_reserved(tag)) {
	    throw tag;
	  }
	  //cerr << tag << endl;
	  if(tag == ESERVER) {
	    string tempBuff = "";
	    list<string> server_name_list;
	    tempBuff = server_map.get(TARGET_SERVERS);
	    while(tempBuff.size()) {
	      string server_name = Token_splitter(tempBuff, " \t");
	      if(server_name.size()) server_name_list.push_back(server_name);
	    }
	    list <string> ignore_server_name_list;
	    tempBuff = server_map.get(IGNORE_SERVERS);
	    while(tempBuff.size()) {
	      string server_name = Token_splitter(tempBuff, " \t");
	      if(server_name.size()) ignore_server_name_list.push_back(server_name);
	    }
	    list <string> ignore_extension_list;
	    tempBuff = server_map.get(IGNORE_EXTENSIONS);
	    while(tempBuff.size()) {
	      string extension = Token_splitter(tempBuff, " \t");
	      if(extension.size()) ignore_extension_list.push_back(extension);
	    }
	    string option = server_map.get(OPTION);
	    string comment = server_map.get(COMMENT);
	    string name = server_map.get(IDENTIFIER);
	    ServerTemplate svt(name, comment, server_name_list, ignore_server_name_list, ignore_extension_list, option, session_vector);
	    if(!svt.bad()) {
	      svt_list.push_back(svt);
	    } else {
	      cerr << "error occurred in server template '" << server_map.get("<identifier>") << "' in file server.aria" << endl;
	      cerr << "discard this server entry" << endl;
	    }
	    //session_vector.clear();
	    break;
	  } else if(tag == SEQUENCE) { //cerr << tag << endl;
	    while(1) {
	      tag = get_next_tag(infile);
	      if(!Is_reserved_in_session(tag)) {
		throw tag;
	      }
	      if(!infile.good()) {
		cerr << "parse error: in getting tag in <sequence>" << endl;
		throw TAGPARSE_GETTAG_EOF;
	      }
	      //cerr << tag << endl;
	      if(tag == SESSION) {
		map<string, string> session_map;
		while(1) {
		  tag = get_next_tag(infile);
		  if(!Is_reserved_in_session(tag)) {
		    throw tag;
		  }
		  //cerr << tag << endl;
		  if(!infile.good()) {
		    cerr << "parse error: in getting tag in <session>" << endl;
		    throw TAGPARSE_GETTAG_EOF;
		  }
		  if(tag == ESESSION) {
		    string label;// = session_map[LABEL];
		    string get = session_map[GET];
		    string referer = session_map[REFERER];
		    string option = session_map[OPTION];
		    int post_offset_size = stoi(session_map[POST_OFFSET_SIZE], 10);
		    string post_offset_string = session_map[POST_OFFSET_STRING];
		    string key_link = session_map[KEYLINK];
		    string command_string = session_map[SVCOMMAND];
		    string command_status = session_map[SVCOMMANDSTAT];
		    string cond;// = session_map[COND];
		    string cond_succ;// = session_map[COND_SUCC];
		    string cond_fail;// = session_map[COND_FAIL];
		    
		    Session session(label, get, referer, key_link, 
				    command_string, command_status, cond,
				    cond_succ, cond_fail, post_offset_size,
				    post_offset_string, option);
		    if(!session.bad()) {
		      session_vector.push_back(session);
		    } else {
		      cerr << "error occurred in server template '" << server_map.get("<identifier>") << "' in file server.aria" << endl;
		      cerr << "discard this session entry" << endl;
		      cerr << "this template may not work correctly. Please disalbe this template until you fix" << endl;
		    }
		    
		    break;
		  } else {
		    string value = get_value(infile, tag);
		    session_map[tag] = value;
		  }
		}
	      } else if(tag == ESEQUENCE) {//cerr << "exit" << endl;
		break;
	      } else {
		throw tag;
	      }
	    }
	  } else {
	    server_map.add(tag, get_value(infile, tag));
	    //cerr << tag << endl;
	  }
	}
      } else {
	throw tag;
      }
    }
    ServerTemplate dummy;
    svt_list.push_back(dummy);
    pthread_mutex_unlock(&svt_list_lock);
    return true;
  } catch (int err) {
    switch(err) {
    case TAGPARSE_GETTAG_EOF:
      cerr << "in tag: " << tag << endl;
      break;
    case TAGPARSE_GETVALUE_EOF:
      cerr << "in getting value in tag: " << tag << endl;
      break;
    case TAGPARSE_UCOM_EOF:
      cerr << "unterminated comment tag" << endl;
      break;
    default:
      cerr << "io error; cannot open file server.aria" << endl;
      break;
    }
  } catch (ErrorType err) {
    cerr << "can't open file" << endl;
  } catch (string tag) {
    cerr << "invalid tag : " << tag << endl;
  }
  cerr << "error occurred in template '" << server_map.get("<identifier>") << "' in file server.aria" << endl;
  ServerTemplate dummy;
  svt_list.push_back(dummy);
  pthread_mutex_unlock(&svt_list_lock);
  return false;
}

vector<ServerTemplate>& ServerTemplateList::ret_server_template_list()
{
  return svt_list;
}

void ServerTemplateList::update_validity(GList *selection)
{
  pthread_mutex_lock(&svt_list_lock);
  for(vector<ServerTemplate>::iterator svt_itr = svt_list.begin();
      svt_itr != svt_list.end(); ++svt_itr) {
    svt_itr->set_valid(false);
  }
  while(selection) {
    int rowindex = GPOINTER_TO_UINT(selection->data);
    svt_list[rowindex].set_valid(true);
    selection = g_list_next(selection);
  }
  pthread_mutex_unlock(&svt_list_lock);
}
