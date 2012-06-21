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

// $Id: URLcontainer.cc,v 1.45 2002/12/18 15:41:05 tujikawa Exp $

// implementation of class URLcontainer
#include "URLcontainer.h"

static ProtocolList supported_protocol_list[] = {
  {"http:", DEFAULT_WWW_PORT},
#ifdef HAVE_OPENSSL
  {"https:", DEFAULT_HTTPS_PORT},
#endif // HAVE_OPENSSL
  {"ftp:", DEFAULT_FTP_PORT},
  {"", 0}
};

//constructor
URLcontainer::URLcontainer()
{
  bad_flag = true;
}

//destructor
URLcontainer::~URLcontainer()
{
}

#define NONUM 0
list<string> URLcontainer::Unfold_URL(const string& src_url)
{
  URLcontainer urlcon;
  list<string> url_list;
  if(!urlcon.Parse_URL(src_url)) return url_list;
  unsigned int h_pos;
  unsigned int lbr_pos;
  string start, end;
  string body, tail;
  int rank_start, rank_end;

  string src_files = urlcon.ret_File();
  while(src_files.size()) {
    string src_file = Token_splitter(src_files, "+");
    if(src_file.size() && src_file.at(0) != '/') {
      src_file = '/'+src_file;
    }
    if(src_file.empty()) continue;
    bool zerofill_flag = true;
    try {
      if((lbr_pos = src_file.find('[')) == string::npos) {
	// bodySTART-END.tail
	if((h_pos = src_file.rfind('-')) == string::npos) {
	  //return src_url;//""
	  throw NONUM;
	}
	unsigned int var_end_pos = src_file.find('.', h_pos);
	if(var_end_pos == string::npos) {
	  var_end_pos = src_file.size();
	}
	
	rank_end = var_end_pos-h_pos-1;
	rank_start = rank_end;
	if(rank_end == 0 || h_pos-1 < (unsigned int)rank_end) throw NONUM;//return src_url;//""
	
	start = src_file.substr(h_pos-rank_start, rank_start);//cerr << start << endl;
	end = src_file.substr(h_pos+1, rank_end);//cerr << end << endl;
	tail = src_file.substr(h_pos+rank_end+1);
	body = src_file.substr(0, h_pos-rank_end);
      } else {
	// body[START-END]tail
	if((h_pos = src_file.find('-', lbr_pos)) == string::npos) {
	  throw NONUM;//return "";
	}
	unsigned int rbr_pos = src_file.rfind(']');
	if(rbr_pos == string::npos || rbr_pos < lbr_pos) throw NONUM;//return "";
	//rank = h_pos-lbr_pos-1;
	rank_end = rbr_pos-h_pos-1;
	rank_start = h_pos-lbr_pos-1;
	if(rank_end != rank_start) {
	  zerofill_flag = false;
	}
	start = src_file.substr(lbr_pos+1, rank_start);
	end = src_file.substr(h_pos+1, rank_end);
	tail = src_file.substr(rbr_pos+1);
	body = src_file.substr(0, lbr_pos);
      }
      int base;
      int zero;
      
      if(isdigit_all(start) && isdigit_all(end)) {
	zero = '0';
	base = 10;
      } else if(isalpha_all(start) && isalpha_all(end)) {
	zero = 'a';
	base = 26;
      } else {
	throw NONUM;//return "";
      }

      int s = 0;
      int e = 0;
      for(int i = 0; i < rank_start; ++i) {
	s += (start.at(i)-zero)*power(base, rank_start-i-1);
      }
      for(int i = 0; i < rank_end; ++i) {
	e += (end.at(i)-zero)*power(base, rank_end-i-1);
      }
      for(int i = s; i <= e; ++i) {
	string url;
	url = urlcon.ret_Protocol()+"//"+
	  urlcon.ret_Hostname();
	if(urlcon.ret_Port() != DEFAULT_WWW_PORT && urlcon.ret_Protocol() == "http:" ||
#ifdef HAVE_OPENSSL
	   urlcon.ret_Port() != DEFAULT_HTTPS_PORT && urlcon.ret_Protocol() == "https:" ||
#endif // HAVE_OPENSSL
	   urlcon.ret_Port() != DEFAULT_FTP_PORT && urlcon.ret_Protocol() == "ftp:") {
	  url += ":"+itos(urlcon.ret_Port());
	}
	url += urlcon.ret_Dir()+body;
	bool zeroseq_flag = true;
	for(int j = 0; j < rank_end; ++j) {
	  char val = i/power(base, rank_end-j-1) % base+zero;
	  if(val != '0') zeroseq_flag = false;
	  if(!zeroseq_flag || zerofill_flag || j == rank_end-1) {
	    url += val;
	  }
	}
	url += tail+urlcon.ret_Query();
	url_list.push_back(url);
      }
    } catch (int err) {
      url_list.push_back(urlcon.ret_Protocol()+"//"+
			 urlcon.ret_Hostname()+urlcon.ret_Dir()+src_file);
    }
  }

  return url_list;
}

string URLcontainer::Find_HREF(string& text, string base_url)
{
  // rule:
  //      if url is quoted by ' or ", then delimitor is one of \t \n ' " <.
  //      if url is not quoted, then delimitor is one of \t \n \ " < [spc].
  //

  // fix this. Only absolute URLs can be retrieved currently.
  // This feature must support relative URLs.
  string url;
  unsigned int p_pos;
  bool quote_flag = false;
  if((p_pos = text.find("http://")) != string::npos ||
#ifdef HAVE_OPENSSL
     (p_pos = text.find("https://")) != string::npos ||
#endif // HAVE_OPENSSL
     (p_pos = text.find("ftp://")) != string::npos) {
    if(p_pos > 0 && (text.at(p_pos-1) == '"' || text.at(p_pos-1) == '\'')) {
      quote_flag = true;
    }
    unsigned int delim_pos;
    if(quote_flag) {
      delim_pos = text.find_first_of("\t\n\"<", p_pos);
    } else {
      delim_pos = text.find_first_of(" \t\n\"<", p_pos);
    }
    if(delim_pos == string::npos) {
      delim_pos = text.size();
    }

    url = Remove_white(text.substr(p_pos, delim_pos-p_pos));

    //text.erase(0, delim_pos-p_pos+1);
    text.erase(0, delim_pos);
  } else {
    text.erase();
  }  

  if(url.size() && !startwith(url, "mailto:") & !startwith(url, "telnet:")) {
    if(url.at(0) == '/' && !base_url.empty() && base_url.at(base_url.size()-1) == '/') {
      unsigned int slash_pos = url.find_first_not_of("/");
      if(slash_pos == string::npos) slash_pos = url.size();
      url.erase(0, slash_pos);
    } else if(url.at(0) != '/' && !base_url.empty() && base_url.at(base_url.size()-1) != '/') {
      base_url += "/";
    }
    if(url.find("http://") != 0 &&
#ifdef HAVE_OPENSSL
       url.find("https://") != 0 &&
#endif // HAVE_OPENSSL
       url.find("ftp://") != 0) {
      url.insert(0, base_url);
    }
  } else {
    url.erase();
  }

  return url;
}
/*
string URLcontainer::Find_HREF_strict(string& text, string base_url)
{
  unsigned int href_pos;
  string url;
  if((href_pos = text.find("HREF=")) != string::npos ||
     (href_pos = text.find("href=")) != string::npos ||
     (href_pos = text.find("SRC=")) != string::npos ||
     (href_pos = text.find("src=")) != string::npos) {
    unsigned int lq_pos;
    if((lq_pos = text.find('"', href_pos)) == string::npos &&
       (lq_pos = text.find('\'', href_pos)) == string::npos) {
      text.erase();
      return "";
    }

    unsigned int rq_pos;
    if((rq_pos = text.find('"', lq_pos+1)) == string::npos &&
       (rq_pos = text.find('\'', lq_pos+1)) == string::npos) {
      text.erase();
      return "";
    }

    url = Remove_white(text.substr(lq_pos+1, rq_pos-lq_pos-1));

    text.erase(0, rq_pos+1);
    if(url.size() && url.substr(0, 7) != "mailto:") {
      if(url.at(0) == '/' && !base_url.empty() && base_url.at(base_url.size()-1) == '/') {
	unsigned int slash_pos = url.find_first_not_of("/");
	if(slash_pos == string::npos) slash_pos = url.size();
	url.erase(0, slash_pos);
      } else if(url.at(0) != '/' && !base_url.empty() && base_url.at(base_url.size()-1) != '/') {
	base_url += "/";
      }
      if(url.find("http://") != 0 && url.find("ftp://") != 0) {
	url.insert(0, base_url);
      }
    } else {
      url.erase();
    }
  } else {
    text.erase();
  }

  return url;
}
*/

string URLcontainer::Find_URL(string& text, bool ws_quark)
{
  unsigned int p_pos;
  string url;

  if((p_pos = text.find("http://")) != string::npos
#ifdef HAVE_OPENSSL
     || (p_pos = text.find("https://")) != string::npos
#endif // HAVE_OPENSSL
     ||(p_pos = text.find("ftp://")) != string::npos) {
    unsigned int delim_pos;
    if(!ws_quark) {
      // "'\"<" is removed,??
      if((delim_pos = text.find_first_of("\t\r\n", p_pos)) == string::npos) {
	delim_pos = text.size();
      }
      // space is not now recognized as a delimiter.
      /*
      unsigned int np_pos;
      if((np_pos = text.substr(p_pos+1, delim_pos-p_pos-1).find("ftp://")) != string::npos ||
#ifdef HAVE_OPENSSL
	 (np_pos = text.substr(p_pos+1, delim_pos-p_pos-1).find("https://")) != string::npos ||
#endif // HAVE_OPENSSL
	 (np_pos = text.substr(p_pos+1, delim_pos-p_pos-1).find("http://")) != string::npos
	 ) {//safe

	delim_pos = text.find_first_of(" \t\r\n", p_pos);
      }
      */
    } else {
      //cerr << "quark found" << endl;
      // space is not now recognized as a delimiter.
      //if((delim_pos = text.find_first_of(" \t\r\n", p_pos)) == string::npos) {
      if((delim_pos = text.find_first_of("\t\r\n", p_pos)) == string::npos) {
	delim_pos = text.size();
      }
    }

    url = Remove_white(text.substr(p_pos, delim_pos-p_pos));
    //text.erase(0, delim_pos-p_pos+1);
    text.erase(0, delim_pos);
  } else {
    text.erase();
  }  
  return  URL_Decode(url);
}

// may be backward compatibility?
string URLcontainer::Find_URL_ext(string& text)
{
  return Find_URL(text);
}

bool URLcontainer::Retrieve_embedded_URL(const list<string>& keylink_list, const string& filename, const URLcontainer& urlcon)
{
  ifstream infile(filename.c_str());
  //string line;

  try {
    while(1) {
      string line;
      getline(infile, line, '<');
      if(line.empty() && infile.eof()) {
	throw EMBEDPARSE_EOF;
      }
      
      getline(infile, line, '>');
      if(startwith(line, "SCRIPT LANGUAGE=\"JavaScript\"") ||
	 startwith(line, "SCRIPT LANGUAGE=JavaScript")) { // fixed 2001/3/18
	// this is temporal fix for homestead. Must be fixed in later release
	if(urlcon.ret_Hostname().find("homestead") == string::npos) {
	  // skip javascript if javainspect is not specified
	  while(1) {
	    line = "";
	    getline(infile, line, '>');
	    if(line.empty() && infile.eof()) throw EMBEDPARSE_EOF;
	    if(Examine_keylinks(line, keylink_list, urlcon)) {
	      throw EMBEDPARSE_FOUND;
	    }
	    if(endwith(line, "/SCRIPT")) {
	      break;
	    }
	  }
	} else
	if(Evaluate_javascript(infile, "document.write", keylink_list, urlcon)) {
	  throw EMBEDPARSE_FOUND;
	}
      } else if(casecomp(line, "style")) {
	while(1) {
	  line = "";
	  getline(infile, line, '>');
	  if(line.empty() && infile.eof()) throw EMBEDPARSE_EOF;
	  if(endwith(line, "/style")) {
	    break;
	  }
	}
      } else if(startwith(line, "!--")) {
	if(line.size() <= 3 ||
	   !endwith(line, "--")) {
	  while(1) {
	    line = "";
	    getline(infile, line, '>');
	    if(line.empty() && infile.eof()) throw EMBEDPARSE_EOF;
	    
	    if(endwith(line, "--")) {
	      break;
	    }
	  }
	}
      } else if(startwith(line, "PRE")) {
	while(1) {
	  line = "";
	  getline(infile, line, '>');
	  if(line.empty() && infile.eof()) throw EMBEDPARSE_EOF;
	  if(endwith(line, "/PRE")) {
	    break;
	  }
	}
      } else if(Remove_white(line).empty()) {
      } else {
	if(Examine_keylinks(line, keylink_list, urlcon)) {
	  throw EMBEDPARSE_FOUND;
	}
      }
    }
  } catch (EmbedParseStatusType err) {
    switch(err) {
    case EMBEDPARSE_FOUND:
      return true;
    case EMBEDPARSE_EOF:
    default:
      return false;
    }
  }
}

bool URLcontainer::Evaluate_javascript(ifstream& infile,
				       const string& target,
				       const list<string>& keylink_list,
				       const URLcontainer& urlcon)
{
  bool end_flag = false;
  string url;
  while(!end_flag) {
    string line = "";
    getline(infile, line, ';');
    unsigned int t_pos;
    if((t_pos = line.find(target)) != string::npos)  {
      unsigned f_pos = line.find('\'', t_pos)+1;
      unsigned e_pos = line.find('\'', f_pos);
      string url_piece = line.substr(f_pos, e_pos-f_pos);
      url += url_piece;
    }
    if(line.find("// -->") != string::npos) {
      end_flag = true;
    }
  }
  //cerr << url << endl;
  return Examine_keylinks(url,
			  keylink_list,
			  urlcon);
}

enum ExamineKeylinksExceptionType {
  URLCON_URLFOUND,
  URLCON_URLNOTFOUND
};

bool URLcontainer::Examine_keylinks(const string& line,
				    const list<string>& keylink_list,
				    const URLcontainer& urlcon)
{
  unsigned int href_pos;
  unsigned int eq_pos = 0;
  bool flag = true;
  string url;
  try {
    list<string> targetElement;
    targetElement.push_back("href");
    targetElement.push_back("src");
    targetElement.push_back("codebase");

    for(list<string>::const_iterator itr = targetElement.begin();
	itr != targetElement.end() && flag; ++itr) {
      href_pos = casefind(line, *itr);
      if(href_pos != string::npos) {
	eq_pos = line.find_first_not_of(" \t\r\n", href_pos+itr->size());
	if(eq_pos == string::npos) {
	  throw URLCON_URLNOTFOUND;
	}
	if(line.at(eq_pos) == '=') {
	  flag = false;
	  break;//added
	}
      }
    }
    if(flag) throw URLCON_URLNOTFOUND;

    unsigned int url_start = line.find_first_not_of(" \t\r\n", eq_pos+1);
    if(url_start == string::npos) {
      throw URLCON_URLNOTFOUND;
    }
    bool quoted_flag;
    if(line.at(url_start) == '\'' ||
       line.at(url_start) == '"') {
      quoted_flag = true;
      ++url_start;
    } else {
      quoted_flag = false;
    }

    if(line.at(url_start) == '\\') {
      url_start += 2;
    }
    unsigned int url_end;
    if(quoted_flag) {
      url_end = line.find_first_of("'\">", url_start);
    } else {
      url_end = line.find_first_of(" '\">", url_start);
    }
    
    if(url_end == string::npos) {
      url_end = line.size();
    }
    if(line.at(url_end-1) == '\\') {
      --url_end;
    }

    // fix this
    string href = replaceSubstring(removeCtrlChar(Remove_white(line.substr(url_start, url_end-url_start))), "&amp;", "&");
    unsigned int slash_pos = href.find('#');
    if(slash_pos != string::npos) {
      href.erase(slash_pos);
    }

    if(startwith(href, "http:")) {
      url = href;
      if(Parse_URL(url)) throw URLCON_URLFOUND;
    }
#ifdef HAVE_OPENSSL
    else if(startwith(href, "https:")) {
      url = href;
      if(Parse_URL(url)) throw URLCON_URLFOUND;
    }
#endif // HAVE_OPENSSL
    else if(startwith(href, "ftp:")) {
      url = href;
      if(Parse_URL(url)) throw URLCON_URLFOUND;
    } else if(startwith(href, "news:")) {
      throw URLCON_URLNOTFOUND;
    } else if(startwith(href, "mailto:")) {
      throw URLCON_URLNOTFOUND;
    } else if(startwith(href, "telnet:")) {
      throw URLCON_URLNOTFOUND;
    } else {
      try {
	url = get_abs_url(urlcon.ret_Protocol()+"//"+urlcon.ret_Hostname()+urlcon.ret_Dir(), href);
      } catch (int err) {
	throw URLCON_URLNOTFOUND;
      }
      if(Parse_URL(url)) throw URLCON_URLFOUND;
    }
    throw URLCON_URLNOTFOUND;
  } catch (ExamineKeylinksExceptionType err) {
    switch(err) {
    case URLCON_URLFOUND:
      for(list<string>::const_iterator itr = keylink_list.begin();
	  itr != keylink_list.end(); ++itr) {
  	unsigned int pointer = url.find(*itr);
  	if(pointer != string::npos) {
  	  return true;
  	}
	
      }
      return false;
    case URLCON_URLNOTFOUND:
    default:
      return false;
    }
  }
}
/*
bool URLcontainer::Examine_keylinks2(const string& line,
				     const list<string>& keylink_list,
				     const URLcontainer& urlcon)
{
  unsigned int pointer = 0;
  unsigned int from_pos = 0;
  while(1) {
    bool flag = true;
    for(list<string>::const_iterator itr = keylink_list.begin();
	itr != keylink_list.end(); ++itr) {
      if((pointer = line.find(*itr, from_pos)) != string::npos) {
	flag = false;
	break;
      }
    }
    if(flag) return false;
  
    unsigned int eq_pos;
    eq_pos = line.rfind('=', pointer);
    if(eq_pos == string::npos || eq_pos < from_pos) {
      if((from_pos = line.find(' ', pointer)) == string::npos) {
	return false;
      } else {
	continue;
      }
    }
    unsigned int url_start = line.find_first_not_of(" \t'\"", eq_pos+1);
    bool quoted_flag = false;
    if(url_start == string::npos) {
      if((from_pos = line.find(' ', pointer)) == string::npos) {
	return false;
      } else {
	continue;
      }
    } else {
      if(line.find_first_not_of("'\"", eq_pos+1) < url_start) {
	quoted_flag = true;
	//if(line.at(url_start-1) == '\'' || line.at(url_start-1) == '"') {
	//quoted_flag = true;
      }
    }
    if(line.at(url_start) == '\\') {
      url_start += 2;
    }
    unsigned int url_end;
    if(quoted_flag) {
      url_end = line.find_first_of("'\">", url_start+1);
    } else {
      url_end = line.find_first_of(" '\">", url_start+1);
    }
    
    if(url_end == string::npos) {
      url_end = line.size();
    }
    if(line.at(url_end-1) == '\\') {
      --url_end;
    }
    
    string href = Remove_white(line.substr(url_start, url_end-url_start));
    unsigned int slash_pos = href.find('#');
    if(slash_pos != string::npos) {
      href.erase(slash_pos);
    }
    string url;
    try {
      if(!startwith(href, "http:") && !startwith(href, "ftp:")) {
	url = get_abs_url(urlcon.ret_Protocol()+"//"+urlcon.ret_Hostname()+urlcon.ret_Dir(), href);
      } else {
	url = href;
      }
    } catch (int err) {
      if((from_pos = line.find(' ', pointer)) == string::npos) {
	return false;
      } else {
	continue;
      }
    }
    return Parse_URL(Remove_white(url));
  }
}
*/

void URLcontainer::clear()
{
  protocol = "";
  host = "";
  port = 0;
  dir = "";
  file = "";
  query = "";
  username = "";
  passwd = "";
  bad_flag = true;
}

bool URLcontainer::Parse_URL(string url)
{
  clear();
  try {
    Extract_protocol(url);
    Extract_userpasswd(url);// added by Matthias Babisch
    Extract_host(url);
    Extract_dir(url);
    Extract_file(url);
    bad_flag = false;
    if(file.empty()) file = '/';

    return true;
  }
  catch (URLconErrorType err) {
    bad_flag = true;
    return false;
  }
}

bool URLcontainer::Is_supported_protocol(const string& protocol_in)
{
  for(unsigned int index = 0; supported_protocol_list[index].protocol != ""; index ++) {
    if(supported_protocol_list[index].protocol == protocol_in) {
      port = supported_protocol_list[index].default_port;
      return true;
    }
  }
  return false;
}

void URLcontainer::Extract_protocol(string& url)
{
  unsigned int slash_index = url.find("//");
  if(slash_index == string::npos) throw URLCON_EINVALIDURL;
  protocol = url.substr(0, slash_index);
  url.erase(0, slash_index+2);
  if(!Is_supported_protocol(protocol)) throw URLCON_EUNSUPP;
}

// username and passwd extraction made by Matthias Babisch
void URLcontainer::Extract_userpasswd(string& url)
{
   unsigned int slash_index = url.find("/");
   unsigned int at_index = url.find("@");
   if(at_index == string::npos) {
     username="";
     passwd="";
   } else {
     if(slash_index == string::npos || slash_index > at_index) {
       // We do have a username-passwd combination
       unsigned int colon_index = url.find(":");
       if(colon_index < slash_index) {
	 // we have a passwd
	 passwd=url.substr(colon_index+1, at_index-colon_index-1);	
	 username=url.substr(0, colon_index);
       } else {
	 username=url.substr(0, at_index);
	 passwd="";
       }
       url.erase(0, at_index+1);
     }
   }  
}
 
void URLcontainer::Extract_host(string& url)
{
  unsigned int slash_index = url.find('/');
  if(slash_index == string::npos) {
    if(url.empty()) {
      throw URLCON_EINVALIDURL;
    } else {
      slash_index = url.size();
    }
  }
  if(slash_index == 0) throw URLCON_EINVALIDURL;
  host = url.substr(0, slash_index);
  url.erase(0, slash_index);
  unsigned int colon_index = host.rfind(':');
  if(colon_index != string::npos) {
    int port_temp = stoi(host.substr(colon_index+1), 10);
    if(port_temp != 0) port = port_temp;
    host.erase(colon_index);
  }
}

void URLcontainer::Extract_dir(string& url)
{
  unsigned int ques_index = url.find('?');
  if(ques_index != string::npos) {
    unsigned int slash_index = url.rfind('/', ques_index);
    if(slash_index == string::npos) throw URLCON_EINVALIDURL;
    dir = url.substr(0, slash_index);
    url.erase(0, slash_index);
  } else {
    unsigned int slash_index = url.rfind('/');
    if(slash_index == string::npos) {
      slash_index = url.size();
    }
    dir = url.substr(0, slash_index);
    url.erase(0, slash_index);
  }
  dir = URL_Decode(dir);
  if(dir == "/") {
    dir = "";
  }
}

void URLcontainer::Extract_file(string& url)
{
  //file = Remove_white(url);
  file = url;
  unsigned int sharp_index = file.find('#');
  if(sharp_index != string::npos) {
    file.erase(sharp_index);
  }
  unsigned int query_index = file.find('?');
  if(query_index != string::npos) {
    query = file.substr(query_index);
    file.erase(query_index);
  } else {
    query = "";
  }
  file = URL_Decode(file);
  /*
  if(file == "/" && protocol != "http:") {
    throw URLCON_EINVALIDURL;
  }
  */
}

const string& URLcontainer::ret_Protocol() const
{
  return (protocol);
}

const string& URLcontainer::ret_Hostname() const
{
  return (host);
}

// following two functions made by Matthias Babisch
const string& URLcontainer::ret_Username() const
{
  return (username);
}
 
const string& URLcontainer::ret_Password() const
{
  return (passwd);
}

const string& URLcontainer::ret_Dir() const
{
  return dir;
}

const string& URLcontainer::ret_File() const
{
  return file;
}

string URLcontainer::ret_Extension() const
{
  unsigned int comma_pos = file.rfind(".");
  if(comma_pos == string::npos) return "";
  else return file.substr(comma_pos+1);
}

string URLcontainer::URL_Decode(const string& src_string)
{
  // untouch strings after '?'
  string dst_string;
  string::const_iterator itr = src_string.begin();
  string::const_iterator itr_st = itr;
  bool q_flag = false;
  while(1) {
    if(itr == src_string.end()) break;
    if(!q_flag && *itr == '%') {
      if(itr+2 >= src_string.end()) return "";
      dst_string += stoui(src_string.substr(itr-itr_st+1, 2), 16);
      itr += 3;
    } else if(*itr == '?') {
      q_flag = true;
      dst_string += *itr;
      ++itr;      
    } else {
      dst_string += *itr;
      ++itr;
    }
  }
  return dst_string;
}

bool URLcontainer::Is_reserved(char ch)
{
  if(ch == 0x2d ||               // '-'
     ch >= 0x61 && ch <= 0x7a || // 'a'-'z'
     ch >= 0x41 && ch <= 0x5a || // 'A'-'Z'
     ch >= 0x30 && ch <= 0x39 || // '0'-'9'
     ch == 0x5f ||               // '_'
     ch == 0x5c ||               // '\'
     ch == 0x2a ||               // '*'
     ch == 0x2e ||               // '.'
     ch == 0x2f ||               // '/'
     ch == 0x3a ||               // ':'
     ch == 0x7e) {               // '~'
    return false;
  } else return true;
}

string URLcontainer::URL_Encode(const string& src_string)
{
  string dst_string;
  //bool flag = true;
  for(string::const_iterator itr = src_string.begin(); itr != src_string.end(); ++itr) {
    /*
    if(*itr == 0x3f && flag) {
      dst_string += *itr;
      flag = false;
      } else*/
    if(Is_reserved(*itr)) {
      dst_string += "%"+itos_hex(*itr&0x000000ff);
    } else {
      dst_string += *itr;
    }
  }
  return dst_string;
}

string URLcontainer::ret_Filename() const
{
  /*
  unsigned int ques_pos = file.find('?', 1);
  if(ques_pos != string::npos) {
    return file.substr(1, ques_pos-1);
  }
  */
  if(file.size()) {
    return file.substr(1);
  } else {
    return "";
  }
}

const string& URLcontainer::ret_Query() const
{
  return query;
}

unsigned int URLcontainer::ret_Port() const
{
  return(port);
}

void URLcontainer::set_Protocol(const string& protocol_in)
{
  protocol = protocol_in;
}

void URLcontainer::set_Hostname(const string& host_in)
{
  host = host_in;
}

void URLcontainer::set_Dir(const string& dir_in)
{
  dir = dir_in;
}

void URLcontainer::set_File(const string& file_in)
{
  file = file_in;
}

void URLcontainer::set_Port(unsigned int port_in)
{
  port = port_in;
}

bool URLcontainer::bad() const
{
  return bad_flag;
}

void URLcontainer::force_bad()
{
  bad_flag = true;
}

string URLcontainer::ret_URL() const
{
  if(protocol == "http:" && port != DEFAULT_WWW_PORT ||
#ifdef HAVE_OPENSSL
     protocol == "https:" && port != DEFAULT_HTTPS_PORT ||
#endif // HAVE_OPENSSL
     protocol == "ftp:" && port != DEFAULT_FTP_PORT) {
    return protocol+"//"+
      host+':'+itos(port)+dir+file+query;
  } else return protocol+"//"+
	   host+dir+file+query;
}
