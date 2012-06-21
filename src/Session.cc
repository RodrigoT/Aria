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

// $Id: Session.cc,v 1.16 2002/04/03 13:33:52 tujikawa Exp $
#include "Session.h"

Session::Session(const string& label_in,
		 const string& get_string,
		 const string& referer_string,
		 const string& keylink_string,
		 const string& command_string,
		 const string& command_status_string,
		 const string& cond_in,
		 const string& cond_succ_in,
		 const string& cond_fail_in,
		 int post_offset_size_in,
		 const string& post_offset_string_in,
		 const string& option_string)
{
  label = label_in;

  if((cond_type = Process_cond_string(cond_in)) > 0) {
    cond_succ = cond_succ_in;
    cond_fail = cond_fail_in;
  }
  nodown = false;
  nocookie = false;
  deletecookie = false;
  getkeylink = false;
  getkeylinkf = false;
  addhref = false;
  readconfig = false;
  noresume = false;
  bad_flag = false;
  try {
    Process_get_string(get_string);
    Process_referer_string(referer_string);
    Process_option_string(option_string);
    Process_keylink_string(keylink_string);

    if(Remove_white(command_string).size()) {
      Command command_temp(command_string, command_status_string);
      if(command_temp.bad()) {
	throw EPARSE;
      } else {
	command = command_temp;
	command.set_valid(true);
      }
    }

    // offset
    if(post_offset_size_in < 0) post_offset_size = 0;
    else
      post_offset_size = post_offset_size_in;
    
    post_offset_string = post_offset_string_in;
  } catch (ErrorType err) {
    cerr << "error occurred in <session> tag" << endl;
    bad_flag = true;
  }
}
Session::Session()
{
}

bool Session::bad() const
{
  return bad_flag;
}

bool Session::Is_program_avaiable() const
{
  return command.Is_valid();
}

string Session::ret_program_line(const string& filename, const string& save_dir, const URLcontainer& urlcon) const
{
  return command.Create_commandline(filename, save_dir, urlcon);
}

Session::SessionExecStatus Session::Execute_program(const string& filename, const string& save_dir, const URLcontainer& urlcon) const
{
  int status = command.Execute_commandline(filename, save_dir, urlcon);
  int ex_stat =  WIFEXITED(status);
  int return_stat = 0;
  if(ex_stat) {
    return_stat = WEXITSTATUS(status);
  }

  if(command.Is_ignore_return_status()) {
    if(ex_stat) {
      return SESSION_EXEC_IGN;
    } else {
      return SESSION_EXEC_IGN_FATAL;
    }
  } else if(!ex_stat) {
    return SESSION_EXEC_IGN_FATAL;
  } else if(command.Is_in_succ_status_list(return_stat)) {
    return SESSION_EXEC_SUCC;
  } else {
    return SESSION_EXEC_FAIL;
  }
}

string Session::MyToken_splitter(string& line)
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
      throw EPARSE;
    }
    token = line.substr(start_pos, end_pos-start_pos+1);

    line.erase(0, end_pos+1);
  } else if(line.at(start_pos) == '"') { //phrase
    ++start_pos;

    unsigned int end_pos = line.find('"', start_pos);
    if(end_pos == string::npos) {
      cerr << "error: unterminated phrase '" << line.substr(start_pos) << "'" << endl;
      throw EPARSE;
    }
    token = line.substr(start_pos, end_pos-start_pos);
    line.erase(0, end_pos+1);
  } else { // unreserved word
    bool flag = true;
    unsigned int end_pos = start_pos;
    while(flag) {
      unsigned int lstart_pos = end_pos;
      end_pos = line.find_first_of(" \t\"$", lstart_pos);
      if(end_pos == string::npos) {
	end_pos = line.size();
	flag = false;
      } else {
	unsigned int bkslash_pos;
	// '\\' must be converted to '\'
	bkslash_pos = lstart_pos;
	while((bkslash_pos = line.find("\\\\", bkslash_pos)) < end_pos) {
	  line.erase(bkslash_pos, 1);
	  ++bkslash_pos;
	  --end_pos;
	}
	if(line.at(end_pos) == '"' && end_pos != 0 && line.at(end_pos-1) == '\\') {
	  // '\"' is '"'
	  line.erase(end_pos-1, 1);
	  //end_pos;
	} else {
	  flag = false;
	}
      }
    }
    token = line.substr(start_pos, end_pos-start_pos);
    line.erase(0, end_pos);
  }
  //cerr << token << endl;
  return token;
}

int Session::Process_cond_string(string cond_string)
{
  if(cond_string.empty()) return -1;
  if(startwith(cond_string, "status_branch(")) {
    Token_splitter(cond_string, "( ");
    string status = Token_splitter(cond_string, ")");
    if(status == "HTTP_OK") {
      cond_type = Session::COND_HTTP_OK;
    } else if(status == "HTTP_ERROR") {
      cond_type = Session::COND_HTTP_ERROR;
    } else if(status == "HTTP_FORBIDDEN") {
      cond_type = Session::COND_HTTP_FORBIDDEN;
    } else if(status == "HTTP_NOTFOUND") {
      cond_type = Session::COND_HTTP_NOTFOUND;
    } else if(status == "HTTP_MOVED") {
      cond_type = Session::COND_HTTP_MOVED;
    } else {
      throw EPARSE;
    }
  } else if(startwith(cond_string, "find_string(")) {
    Token_splitter(cond_string, "( ");
    string str_find = Token_splitter(cond_string, ")");
    if(str_find.empty()) throw EPARSE;
    cond_type = Session::COND_FIND_STRING;
    cond_str_find = str_find;
  }
  return cond_type;
}

void Session::Process_option_string(string option_string)
{
  while(option_string.size()) {
    string token = Token_splitter(option_string);
    
    if(token == "nodown") {
      nodown = true;
    } else if(token == SOPTION_NOCOOKIE) {
      nocookie = true;
    } else if(token == SOPTION_DELETECOOKIE) {
      deletecookie = true;
    } else if(token == SOPTION_GETKEYLINK) {
      getkeylink = true;
    } else if(token == SOPTION_GETKEYLINKF) {
      getkeylinkf = true;
    } else if(token == SOPTION_ADDHREF) {
      addhref = true;
    } else if(token == SOPTION_READCONFIG) {
      readconfig = true;
    } else if(token == SOPTION_NORESUME) {
      noresume = true;
    } else {
      cerr << "error: undefined option: '" << token << "' in <option>" << endl;
      throw EPARSE;
    }
  }
}

void Session::Process_get_string(string get_string)
{
  while(get_string.size()) {
    string token = MyToken_splitter(get_string);
    if(token.size()) {
      if(token.at(0) == '$' && !Is_reserved(token)) {
	cerr << "error: undefined macro: '" << token << "' in <get>" << endl;
	throw EPARSE;
      }
      alt_get.push_back(token);
    }
  }
}

void Session::Process_referer_string(string referer_string)
{
  while(referer_string.size()) {
    string token = MyToken_splitter(referer_string);
    if(token.size()) {
      if(token.at(0) == '$' && !Is_reserved(token)) {
	cerr << "error: undefined macro: '" << token << "' in <referer>" << endl;
	throw EPARSE;
      }
      alt_referer.push_back(token);
    }
  }
}

void Session::Process_keylink_string(string keylink_string)
{
  while(keylink_string.size()) {
    vector<string> keylink;
    while(1) {
      string token = MyToken_splitter(keylink_string);
      if(token.size()) {
	if(token.at(0) == '$' && !Is_reserved(token)) {
	  cerr << "error: undefined macro: '" << token << "' in <keylink>" << endl;
	  throw EPARSE;
	}
	keylink.push_back(token);
      }

      if(keylink_string.empty() || keylink_string.at(0) == ' ' || keylink_string.at(0) == '\t') {
	if(keylink.size()) keylink_list.push_back(keylink);
	break;
      }
    }
  }
}

bool Session::Is_reserved(const string& token)
{
  if(startwith(token, "$(host") ||
     startwith(token, "$(dir")  ||
     token == "$(file)" ||
     token == "$(protocol)" ||
     token == "$(query)" ||
     token == "$(url)" ||
     startwith(token, "$(retrieved_host") ||
     startwith(token, "$(retrieved_dir") ||
     token == "$(retrieved_file)" ||
     token == "$(retrieved_protocol)" ||
     token == "$(retrieved_query)" ||
     token == "$(retrieved_url)" ||
     startwith(token, "$(previous_host") ||
     startwith(token, "$(previous_dir") ||
     token == "$(previous_file)" ||
     token == "$(previous_protocol)" ||
     token == "$(previous_query)" ||
     token == "$(previous_url)") {
    return true;
  } else {
    return false;
  }
}

const list< vector<string> >& Session::ret_keylink_list() const
{
  return keylink_list;
}

const string& Session::ret_label() const
{
  return label;
}

const string& Session::ret_cond_succ() const
{
  return cond_succ;
}

const string& Session::ret_cond_fail() const
{
  return cond_fail;
}

void Session::set_cond_session_succ(int session_counter)
{
  cond_session_succ = session_counter;
}

void Session::set_cond_session_fail(int session_counter)
{
  cond_session_fail = session_counter;
}

int Session::ret_cond_session_succ() const
{
  return cond_session_succ;
}

int Session::ret_cond_session_fail() const
{
  return cond_session_fail;
}

int Session::ret_post_offset_size() const
{
  return post_offset_size;
}

const string& Session::ret_post_offset_string() const
{
  return post_offset_string;
}

int Session::ret_referer_type() const
{
  return referer_type;
}

const vector<string>& Session::ret_referer_vector() const
{
  return alt_referer;
}

const vector<string>& Session::ret_get_vector() const
{
  return alt_get;
}

bool Session::Is_noresume() const
{
  return noresume;
}

bool Session::Is_getkeylink() const
{
  return getkeylink;
}

bool Session::Is_getkeylink_force() const
{
  return getkeylinkf;
}

bool Session::Is_nodown() const
{
  return nodown;
}

bool Session::Is_nocookie() const
{
  return nocookie;
}

bool Session::Is_deletecookie() const
{
  return deletecookie;
}

bool Session::Is_addhref() const
{
  return addhref;
}

bool Session::Is_readconfig() const
{
  return readconfig;
}

int Session::interpret(const string& word)
{
  if(word == "$(url)") {
    return VAR_URL;
  } else if(word == "$(host)") {
    return VAR_HOST;
  } else if(word == "$(dir)") {
    return VAR_DIR;
  } else if(startwith(word, "$(dir")) {
    return VAR_PARTIAL_DIR;
  } else if(startwith(word, "$(host")) {
    return VAR_PARTIAL_HOST;
  } else if(word == "$(file)") {
    return VAR_FILE;
  } else if(word == "$(query)") {
    return VAR_QUERY;
  } else if(word == "$(protocol)") {
    return VAR_PROTOCOL;
  } else if(word == "$(retrieved_url)") { //start retrieved url macros
    return VAR_RETRIEVED_URL;
  } else if(word == "$(retrieved_host)") {
    return VAR_RETRIEVED_HOST;
  } else if(word == "$(retrieved_dir)") {
    return VAR_RETRIEVED_DIR;
  } else if(startwith(word, "$(retrieved_host")) {
    return VAR_RETRIEVED_PARTIAL_HOST;
  } else if(startwith(word, "$(retrieved_dir")) {
    return VAR_RETRIEVED_PARTIAL_DIR;
  } else if(word == "$(retrieved_file)") {
    return VAR_RETRIEVED_FILE;
  } else if(word == "$(retrieved_query)") {
    return VAR_RETRIEVED_QUERY;
  } else if(word == "$(retrieved_protocol)") {
    return VAR_RETRIEVED_PROTOCOL;
  } else if(word == "$(previous_url)") { //start previous url macros
    return VAR_SESSION_URL;
  } else if(word == "$(previous_host)") {
    return VAR_SESSION_HOST;
  } else if(word == "$(previous_dir)") {
    return VAR_SESSION_DIR;
  } else if(startwith(word, "$(previous_host")) {
    return VAR_SESSION_PARTIAL_HOST;
  } else if(startwith(word, "$(previous_dir")) {
    return VAR_SESSION_PARTIAL_DIR;
  } else if(word == "$(previous_file)") {
    return VAR_SESSION_FILE;
  } else if(word == "$(previous_query)") {
    return VAR_SESSION_QUERY;
  } else if(word == "$(previous_protocol)") {
    return VAR_SESSION_PROTOCOL;
  } else {
    return UNRESERVED;
  }
}

void Session::set_session_referer(const URLcontainer& urlcon)
{
  session_urlcon = urlcon;
}

bool Session::set_session_referer(const string& url)
{
  return session_urlcon.Parse_URL(url);
}

string Session::ret_session_referer() const
{
  return session_urlcon.ret_URL();
}


string Session::Create_URL_from_get_vector(const URLcontainer& orig_urlcon, const URLcontainer& retrieved_urlcon) {
  return Create_URL(orig_urlcon, retrieved_urlcon, alt_get);
}

string Session::Create_URL_from_referer_vector(const URLcontainer& orig_urlcon, const URLcontainer& retrieved_urlcon) {
  return Create_URL(orig_urlcon, retrieved_urlcon, alt_referer);

}

string Session::Create_URL(const URLcontainer& orig_urlcon, const URLcontainer& retrieved_urlcon, const vector<string>& string_vector)
{
  string url;
  //URLcontainer orig_urlcon;
  //orig_urlcon.Parse_URL(ret_URL());
  
  for(vector<string>::const_iterator itr = string_vector.begin();
      itr != string_vector.end(); ++itr) {
    int dval = Session::interpret(*itr);
    URLcontainer urlcon;
    if(dval < VAR_RETRIEVED_URL) {
      urlcon = orig_urlcon;
    } else if(dval < VAR_SESSION_URL) {
      urlcon = retrieved_urlcon;
    } else {
      urlcon = session_urlcon;
    }
    switch(dval) {
    case Session::VAR_URL:
    case Session::VAR_RETRIEVED_URL:
    case Session::VAR_SESSION_URL:
      url += urlcon.ret_URL();
      break;
    case Session::VAR_HOST:
    case Session::VAR_RETRIEVED_HOST:
    case Session::VAR_SESSION_HOST:
      url += urlcon.ret_Hostname();
      break;
    case Session::VAR_DIR:
    case Session::VAR_RETRIEVED_DIR:
    case Session::VAR_SESSION_DIR:
      url += urlcon.ret_Dir();
      break;
    case Session::VAR_PARTIAL_DIR:
    case Session::VAR_RETRIEVED_PARTIAL_DIR:
    case Session::VAR_SESSION_PARTIAL_DIR:
      {
	if(urlcon.ret_Dir().empty()) break;
	string word = *itr;
	Token_splitter(word, " ,\t");
	int start = stoi(Token_splitter(word, " ,)\t"));
	int end = stoi(Token_splitter(word, " ,)\t"));
  
	unsigned int start_index = 0;
	for(int i = 0; i < start; ++i) {
	  start_index = urlcon.ret_Dir().find("/", start_index+1);
	  if(start_index == string::npos) {
	    start_index = urlcon.ret_Dir().size();
	    break;
	  }
	}
	unsigned int end_index;
	if(end < 0) {
	  end_index = urlcon.ret_Dir().size();
	} else {
	  if(start > end) end = start;
	  end_index = start_index;
	  for(int i = 0; i < end-start+1; ++i) {
	    end_index = urlcon.ret_Dir().find("/", end_index+1);
	    if(end_index == string::npos) {
	      end_index = urlcon.ret_Dir().size();
	      break;
	    }
	  }
	}
	int chop_s = stoi(Token_splitter(word, " ,)\t"));
	if(chop_s > 0)
	  start_index += chop_s;
	int chop_e = stoi(Token_splitter(word, " ,)\t"));
	if(chop_e > 0)
	  end_index -= chop_e;
	if(start_index <= end_index) {
	  url += urlcon.ret_Dir().substr(start_index, end_index-start_index);
	}
      }
      break;
    case Session::VAR_PARTIAL_HOST:
    case Session::VAR_RETRIEVED_PARTIAL_HOST:
    case Session::VAR_SESSION_PARTIAL_HOST:
      
      {
	if(urlcon.ret_Hostname().empty()) break;

	string word = *itr;
	Token_splitter(word, " ,\t");
	int start = stoi(Token_splitter(word, " ,)\t"));
	int end = stoi(Token_splitter(word, " ,)\t"));
    
	unsigned int start_index = 0;

	for(int i = 0; i < start; ++i) {
	  start_index = urlcon.ret_Hostname().find(".", start_index);
	  if(start_index == string::npos) {
	    start_index = urlcon.ret_Hostname().size();
	    break;
	  } else {
	    ++start_index;
	  }
	}
	unsigned int end_index;
	if(end < 0) {
	  end_index = urlcon.ret_Hostname().size();
	} else {
	  if(start > end) end = start;
	  end_index = start_index;
	  for(int i = 0; i < end-start+1; ++i) {
	    end_index = urlcon.ret_Hostname().find(".", end_index+1);
	    if(end_index == string::npos) {
	      end_index = urlcon.ret_Hostname().size();
	      break;
	    }
	  }
	}
	int chop_s = stoi(Token_splitter(word, " ,)\t"));
	if(chop_s > 0)
	  start_index += chop_s;
	int chop_e = stoi(Token_splitter(word, " ,)\t"));
	if(chop_e > 0)
	  end_index -= chop_e;
	if(start_index <= end_index) {
	  url += urlcon.ret_Hostname().substr(start_index, end_index-start_index);
	}
      }
      break;
    case Session::VAR_FILE:
    case Session::VAR_RETRIEVED_FILE:
    case Session::VAR_SESSION_FILE:
      url += urlcon.ret_File();
      break;
    case Session::VAR_QUERY:
    case Session::VAR_RETRIEVED_QUERY:
    case Session::VAR_SESSION_QUERY:
      url += urlcon.ret_Query();
      break;
    case Session::VAR_PROTOCOL:
    case Session::VAR_RETRIEVED_PROTOCOL:
    case Session::VAR_SESSION_PROTOCOL:
      url += urlcon.ret_Protocol();
      break;
    default:
      url += *itr;
      break;
    } 
  }
  return url;
}
