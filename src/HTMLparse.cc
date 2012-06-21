//
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

// $Id: HTMLparse.cc,v 1.23 2002/09/30 13:29:45 tujikawa Exp $

#include "HTMLparse.h"

bool isTag(const string& line, const string& tag)
{
  if(startwith(line, "base ")
     || startwith(line, "base\t")
     || startwith(line, "base\r")
     || startwith(line, "base\n")) {
    return(true);
  } else {
    return(false);
  }
}

string HTMLparse::erase_protocol(string line, int length, int prot_pos)
{
  line.erase(prot_pos, length);
  line.insert(prot_pos, prefix);
  unsigned int colon_pos = line.find_first_of(":/\"'>", prot_pos);
  if(colon_pos != string::npos) {
    if(line.at(colon_pos) == ':') {
      unsigned int slash_pos = line.find_first_of("/\"'>", colon_pos);
      if(slash_pos != string::npos) {
	line.erase(colon_pos, slash_pos-colon_pos);
      } else {
	line.erase(colon_pos);
      }
    }
  }
  return line;
}

URLcontainer HTMLparse::find_href(string line, Options& options)
{
  unsigned int href_pos = 0;
  unsigned int eq_pos = 0;
  bool flag = true;;
  URLcontainer urlcon;
  bool fsavefile = (options.ret_with_hostname_dir() && options.ret_abs2rel_url() || options.ret_delete_comment() || options.ret_delete_javascript() || options.ret_convert_tilde()) && !outfile_bad;
  bool abs2rel = options.ret_with_hostname_dir() && options.ret_abs2rel_url() && !outfile_bad;
  bool fcv_flag = false;
  try {
    /*
    list<string> targetElement;
    targetElement.push_back("href");
    targetElement.push_back("src");
    targetElement.push_back("background");
    targetElement.push_back("codebase");

    for(list<string>::const_iterator itr = targetElement.begin();
	itr != targetElement.end() && flag; ++itr) {
      href_pos = casefind(line, *itr);
      if(href_pos != string::npos) {
	eq_pos = line.find_first_not_of(" \t\r\n", href_pos+itr->size());
	if(eq_pos == string::npos) {
	  throw HTMLPARSE_NOHREF;
	}
	if(line.at(eq_pos) == '=') {
	  flag = false;
	  if(!options.ret_use_tag_href() &&
	     options.ret_abs2rel_url() && options.ret_with_hostname_dir()) {
	    fcv_flag = true;
	  }
	}
      }
    }
    */

    if(//options.ret_use_tag_href() &&
       (href_pos = casefind(line, "href")) != string::npos) {
      //eq_pos = line.find("=", href_pos);
      eq_pos = line.find_first_not_of(" \t\r\n", href_pos+4);

      if(eq_pos == string::npos) {
	throw HTMLPARSE_NOHREF;
      }
      //if(Remove_white(line.substr(href_pos+4, eq_pos-href_pos-4)).empty()) {
      if(line.at(eq_pos) == '=') {
	flag = false;
	if(!options.ret_use_tag_href() &&
	   options.ret_abs2rel_url() && options.ret_with_hostname_dir()) {
	  fcv_flag = true;
	}
      }
    }
    if(flag &&// options.ret_use_tag_background() &&
       (href_pos = casefind(line, "background")) != string::npos) {
      //eq_pos = line.find("=", href_pos);
      eq_pos = line.find_first_not_of(" \t\r\n", href_pos+10);
	
      if(eq_pos == string::npos) {
	throw HTMLPARSE_NOHREF;
      }
      //if(Remove_white(line.substr(href_pos+10, eq_pos-href_pos-10)).empty()) {
      if(line.at(eq_pos) == '=') {
	flag = false;
	if(!options.ret_use_tag_background() &&
	   options.ret_abs2rel_url() && options.ret_with_hostname_dir()) {
	  fcv_flag = true;
	}
      }	
    }
    if(flag &&// options.ret_use_tag_src() &&
       (href_pos = casefind(line, "src")) != string::npos) {
      //eq_pos = line.find("=", href_pos);
      eq_pos = line.find_first_not_of(" \t\r\n", href_pos+3);

      if(eq_pos == string::npos) {
	throw HTMLPARSE_NOHREF;
      }
      //if(Remove_white(line.substr(href_pos+3, eq_pos-href_pos-3)).empty()) {
      if(line.at(eq_pos) == '=') {
	flag = false;
	if(!options.ret_use_tag_src() &&
	   options.ret_abs2rel_url() && options.ret_with_hostname_dir()) {
	  fcv_flag = true;
	}
      }
    }
    if(flag &&// options.ret_use_tag_code() &&
       (href_pos = casefind(line, "codebase")) != string::npos) {
      //eq_pos = line.find("=", href_pos);
      eq_pos = line.find_first_not_of(" \t\r\n", href_pos+4);

      if(eq_pos == string::npos) {
	throw HTMLPARSE_NOHREF;
      }
      //if(Remove_white(line.substr(href_pos+8, eq_pos-href_pos-8)).empty()) {
      if(line.at(eq_pos) == '=') {
	flag = false;
	if(!options.ret_use_tag_code() &&
	   options.ret_abs2rel_url() && options.ret_with_hostname_dir()) {
	  fcv_flag = true;
	}
      }
    }

    if(!flag) {
      unsigned int url_start = line.find_first_not_of(" \t\r\n", eq_pos+1);
      if(url_start == string::npos) {
	throw HTMLPARSE_NOHREF;
      }
      bool quoted_flag;
      if(line.at(url_start) == '\'' ||
	 line.at(url_start) == '"') {
	quoted_flag = true;
	++url_start;
      } else {
	quoted_flag = false;
      }

      //cerr << line.substr(url_start) << endl;
      /*
      unsigned int url_start = line.find_first_not_of(" \t'\"", eq_pos+1);
      bool quoted_flag = false;
      if(url_start == string::npos) {
	throw HTMLPARSE_NOHREF;
      } else {
	if(line.substr(eq_pos+1, url_start-eq_pos-1).find("\"\"") != string::npos || line.substr(eq_pos+1, url_start-eq_pos-1).find("''") != string::npos) {
	  throw HTMLPARSE_NOHREF;
	}
	if(line.find_first_not_of("'\"", eq_pos+1) < url_start) {
	  quoted_flag = true;
	  //if(line.at(url_start-1) == '\'' || line.at(url_start-1) == '"') {
	  //quoted_flag = true;
	}
      }
      */
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
      href = URLcontainer::URL_Decode(href);

      unsigned int slash_pos = href.find('#');
      if(slash_pos != string::npos) {
	href.erase(slash_pos);
      }
      string url;

      // add baseHref prior to href
      if(!startwith(href, "http:")
#ifdef HAVE_OPENSSL
	 && !startwith(href, "https:")
#endif // HAVE_OPENSSL
	 && !startwith(href, "ftp:")
	 && !startwith(href, "news:")
	 && !startwith(href, "mailto:")
	 && !startwith(href, "telnet:")) {
	if(startwith(href, "/")) {
	  href = baseHref+href.substr(1, href.size()-1);
	} else {
	  href = baseHref+href;
	}
      }

      if(startwith(href, "http:")
#ifdef HAVE_OPENSSL
	 || startwith(href, "https:")
#endif // HAVE_OPENSSL
	 ) {
	if(!isTag(line, "base")
	   && options.ret_only_relative_links() || !options.ret_with_hostname_dir()) {
	  if(options.ret_force_convert() && options.ret_with_hostname_dir()) {
	    fcv_flag = true;
	  } else {
	    throw HTMLPARSE_NOHREF;
	  }
	}
	url = href;
	if(!urlcon.Parse_URL(url)) throw HTMLPARSE_NOHREF;
	if(!isTag(line, "base")
	   && get_hostname(base_url) == urlcon.ret_Hostname()) { // same site
	} else {
	  // different site
	  if(options.ret_no_other_host() || options.ret_no_ascend()) {
	    if(options.ret_force_convert()) {
	      fcv_flag = true;
	    } else {
	      throw HTMLPARSE_NOHREF;
	    }
	  }
	}
      }	else if(startwith(href, "ftp:")) {
	if(options.ret_no_other_host() ||
	   options.ret_only_relative_links() ||
	   !options.ret_with_hostname_dir() ||
	   !options.ret_Follow_ftp_link()) {
	  if(options.ret_force_convert() && options.ret_with_hostname_dir()) {
	    fcv_flag = true;
	  } else {
	    throw HTMLPARSE_NOHREF;
	  }
	}
	url = href;
	if(!urlcon.Parse_URL(url)) throw HTMLPARSE_NOHREF;
      } else if(startwith(href, "news:")) {
	throw HTMLPARSE_NOHREF;
      } else if(startwith(href, "mailto:")) {
	throw HTMLPARSE_NOHREF;
      } else if(startwith(href, "telnet:")) {
	throw HTMLPARSE_NOHREF;
      } else {
	try {
	  url = get_abs_url(base_url, href);
	} catch (int err) {
	  throw HTMLPARSE_NOHREF;
	}
	if(!urlcon.Parse_URL(url)) throw HTMLPARSE_NOHREF;
      }

      if(!isTag(line, "base")) {
	// check Options "no ascend"
	if(options.ret_no_ascend() && url.find(root_url.c_str(), 0, root_url.size()) == string::npos) {
	  if(options.ret_force_convert() && options.ret_with_hostname_dir()) {
	    fcv_flag = true;
	  } else {
	    throw HTMLPARSE_NOHREF;
	  }
	}
	// filter by domain here
	if(options.Is_in_activated_ign_domain_list(urlcon.ret_Hostname())) {
	  if(options.ret_force_convert() && options.ret_with_hostname_dir()) {
	    fcv_flag = true;
	  } else {
	    throw HTMLPARSE_NOHREF;
	  }
	}
	
	// filter by file extension here
	if(options.ret_use_down_filter() &&
	   (!options.Is_in_activated_filter_down_target_list(urlcon.ret_Filename()) && !options.Is_in_activated_parse_target_list(urlcon.ret_Filename()))
	   || !options.ret_use_down_filter() &&
	   options.Is_in_activated_filter_nodown_target_list(urlcon.ret_Filename())) {
	  if(options.ret_force_convert() && options.ret_with_hostname_dir()) {
	    fcv_flag = true;
	  } else {
	    throw HTMLPARSE_NOHREF;
	  }
	}
      }

      // set save directory
      Set_save_directory(urlcon, href, options);

      if(options.ret_convert_tilde()) {
	unsigned int tilde_pos = line.find('~');
	if(tilde_pos >= url_start &&  tilde_pos < url_end) {
	  line.erase(tilde_pos, 1);
	  line.insert(tilde_pos, "%7E");
	  url_end += 2;
	}
      }
      // convert absolute URL to relative one
      if(abs2rel) {
	unsigned int prot_pos;
	string line_temp;
	if(line.size() > 7 && (prot_pos = line.find("http://", url_start, 7)) != string::npos && prot_pos < url_end) {
	  // start with "HTTP://"
	  line_temp = erase_protocol(line, 7, prot_pos);
	}
#ifdef HAVE_OPENSSL
	else if(line.size() > 8 && (prot_pos = line.find("https://", url_start, 8)) != string::npos && prot_pos < url_end) {
	  // start with "https://"
	  line_temp = erase_protocol(line, 8, prot_pos);
	}
#endif // HAVE_OPENSSL
	else if(line.size() > 6 && (prot_pos = line.find("ftp://", url_start, 6)) != string::npos && prot_pos < url_end) {
	  // start with "FTP://"
	  line_temp = erase_protocol(line, 6, prot_pos);
	} else if(line.size() > url_start && line.at(url_start) == '/') {
	  // start with "/"
	  line_temp = line;
	  line_temp.insert(url_start, prefix+urlcon.ret_Hostname());
	} else {
	  // otherwise
	  line_temp = line;
	}
	outfile << line_temp;
      } else {
	if(fsavefile) outfile << line;
      }
    } else {
      // no link
      if(fsavefile) outfile << line;
    }
    if(fcv_flag) {
      urlcon.force_bad();
    }
    //if(!urlcon.bad())
    //  cerr << line << endl;
    // insert here &amp; -> &
    return urlcon;
  } catch (HTMLparseExceptionType err) {
    if(fsavefile) outfile << line;
    urlcon.force_bad();// 強制的にbad状態へ
    return urlcon;
  }
}

// Set save directory for each item
void HTMLparse::Set_save_directory(const URLcontainer& urlcon,
				   const string& href,
				   Options& options)
{
  try {
    // set save directory
    if(options.ret_with_hostname_dir()) {// use hostname directory
      string fix_dir;
      if(options.ret_convert_tilde()) {
	fix_dir = convert_tilde(urlcon.ret_Dir());
      } else {
	fix_dir = urlcon.ret_Dir();
      }
      options.set_Store_Dir(documentroot_dir+urlcon.ret_Hostname()+fix_dir);
    } else {// otherwise
      string fix_href = href;
      string file = get_file(fix_href);
      if(options.ret_convert_tilde()) {
	fix_href = convert_tilde(fix_href);
      }
      if(fix_href.size() && fix_href.at(0) == '/') {
	options.set_Store_Dir(get_storedir(documentroot_dir, fix_href));
      } else {
	options.set_Store_Dir(get_storedir(options.ret_Store_Dir(), fix_href));
      }
    }
  } catch (int err) {
    throw HTMLPARSE_NOHREF;
  }
}

URLcontainer HTMLparse::find_css(string line, Options& options) {
  // expects line to be url(***.css) or url("***.css")
  //         or simply  "***.css" or ***.css
  bool fsavefile = (options.ret_with_hostname_dir() && options.ret_abs2rel_url() || options.ret_delete_comment() || options.ret_delete_javascript()) && !outfile_bad;
  bool abs2rel = options.ret_with_hostname_dir() && options.ret_abs2rel_url() && !outfile_bad;
  URLcontainer urlcon;
  try {
    unsigned int index;
    bool fcv_flag = false;
    string media;

    if((index = line.find("(")) == string::npos) {
      //throw HTMLPARSE_NOHREF;
      media = "file";
      //cerr << "this is file media type" << endl;
    } else {
      media = Remove_white(line.substr(0, index));
      //cerr << "this is url media type" << endl;
    }
    // needs following three variables
    string href;
    unsigned int url_start;
    unsigned int url_end;
    if(casecomp(media, "url")) {
      //cerr << "find keyword 'url'" << endl;
      unsigned int parenL = index;
      unsigned int parenR = line.find(")", index+1);
      if(parenR == string::npos) throw HTMLPARSE_NOHREF;
      unsigned int quoteL = line.find("\"", index+1);
      if(quoteL < parenR && quoteL != string::npos) {
	if((url_start = line.find_first_not_of(" \t", quoteL+1, parenR-quoteL-1)) == string::npos) {
	  throw HTMLPARSE_NOHREF;
	}
      } else {
	url_start = parenL+1;
      }
      unsigned int quoteR = line.find("\"", url_start+1);
      if(quoteR < parenR && quoteR != string::npos) {
	url_end = quoteR-1;
      } else {
	url_end = parenR-1;
      }
      href = Remove_white(line.substr(url_start, url_end-url_start+1));
      //cerr << "href found is " << href << endl;
      if(href.size() == 0) {
	throw HTMLPARSE_NOHREF;
      }
//        url_start = parenL+1;
//        url_end = parenR-1;
    } else if(casecomp(media, "file")) {
      if((url_start = line.find_first_not_of(" \t\"")) == string::npos) {
	throw HTMLPARSE_NOHREF;
      }
      url_end = line.find("\"", url_start+1);
      if(url_end == string::npos) {
	url_end = line.size()-1;
      } else {
	--url_end;
      }
      href = Remove_white(line.substr(url_start, url_end-url_start+1));
      if(href.size() == 0) {
	throw HTMLPARSE_NOHREF;
      }
    } else {
      throw HTMLPARSE_NOHREF;
    }
    if(href.size() > 0) {
      string url;
      // examine suburl is absolute URL or relative one
      if(startwith(href, "http:")
#ifdef HAVE_OPENSSL
	 || startwith(href, "https:")
#endif // HAVE_OPENSSL
	 ) {
	if(options.ret_only_relative_links() || !options.ret_with_hostname_dir()) {
	  if(options.ret_force_convert() && options.ret_with_hostname_dir()) {
	    fcv_flag = true;
	  } else {
	    throw HTMLPARSE_NOHREF;
	  }
	}
	url = href;
	if(!urlcon.Parse_URL(url)) throw HTMLPARSE_NOHREF;
	if(get_hostname(base_url) == urlcon.ret_Hostname()) { // same site
	} else {
	  // different site
	  if(options.ret_no_other_host() || options.ret_no_ascend()) {
	    if(options.ret_force_convert()) {
	      fcv_flag = true;
	    } else {
	      throw HTMLPARSE_NOHREF;
	    }
	  }
	}
      }	else if(startwith(href, "ftp:")) {
	if(options.ret_no_other_host() ||
	   options.ret_only_relative_links() ||
	   !options.ret_with_hostname_dir() ||
	   !options.ret_Follow_ftp_link()) {
	  if(options.ret_force_convert() && options.ret_with_hostname_dir()) {
	    fcv_flag = true;
	  } else {
	    throw HTMLPARSE_NOHREF;
	  }
	}
	url = href;
	if(!urlcon.Parse_URL(url)) throw HTMLPARSE_NOHREF;
      } else if(startwith(href, "news:")) {
	throw HTMLPARSE_NOHREF;
      } else if(startwith(href, "mailto:")) {
	throw HTMLPARSE_NOHREF;
      } else if(startwith(href, "telnet:")) {
	throw HTMLPARSE_NOHREF;
      } else {
	try {
	  url = get_abs_url(base_url, href);
	} catch (int err) {
	  throw HTMLPARSE_NOHREF;
	}
	if(!urlcon.Parse_URL(url)) throw HTMLPARSE_NOHREF;
      }

      // check Options "no ascend"
      if(options.ret_no_ascend() && url.find(root_url.c_str(), 0, root_url.size()) == string::npos) {
	if(options.ret_force_convert() && options.ret_with_hostname_dir()) {
	  fcv_flag = true;
	} else {
	  throw HTMLPARSE_NOHREF;
	}
      }
      // filter by domain here
      if(options.Is_in_activated_ign_domain_list(urlcon.ret_Hostname())) {
	if(options.ret_force_convert() && options.ret_with_hostname_dir()) {
	  fcv_flag = true;
	} else {
	  throw HTMLPARSE_NOHREF;
	}
      }

      // filter by file extension here
      if(options.ret_use_down_filter() &&
	 (!options.Is_in_activated_filter_down_target_list(urlcon.ret_Filename()) && !options.Is_in_activated_parse_target_list(urlcon.ret_Filename())) ||
	 !options.ret_use_down_filter() &&
	 options.Is_in_activated_filter_nodown_target_list(urlcon.ret_Filename())) {
	if(options.ret_force_convert() && options.ret_with_hostname_dir()) {
	  fcv_flag = true;
	} else {
	  throw HTMLPARSE_NOHREF;
	}
      }

      // set save directory
      Set_save_directory(urlcon, href, options);

      if(options.ret_convert_tilde()) {
	unsigned int tilde_pos = line.find('~');
	if(tilde_pos >= url_start &&  tilde_pos < url_end) {
	  line.erase(tilde_pos, 1);
	  line.insert(tilde_pos, "%7E");
	  url_end += 2;
	}
      }
      // convert absolute URL to relative one
      if(abs2rel) {
	unsigned int prot_pos;
	string line_temp;
	if(line.size() > 7 && (prot_pos = line.find("http://", url_start, 7)) != string::npos && prot_pos < url_end) {
	  // start with "HTTP://"
	  line_temp = erase_protocol(line, 7, prot_pos);
	}
#ifdef HAVE_OPENSSL
	else if(line.size() > 8 && (prot_pos = line.find("https://", url_start, 8)) != string::npos && prot_pos < url_end) {
	  // start with "https://"
	  line_temp = erase_protocol(line, 8, prot_pos);
	}
#endif // HAVE_OPENSSL
	else if(line.size() > 6 && (prot_pos = line.find("ftp://", url_start, 6)) != string::npos && prot_pos < url_end) {
	  // start with "FTP://"
	  line_temp = erase_protocol(line, 6, prot_pos);
	} else if(line.size() > url_start && line.at(url_start) == '/') {
	  // start with "/"
	  line_temp = line;
	  line_temp.insert(url_start, prefix+urlcon.ret_Hostname());
	} else {
	  // otherwise
	  line_temp = line;
	}
	outfile << line_temp;
      } else {
	if(fsavefile) outfile << line;
      }
    } else {
      // no link
      if(fsavefile) outfile << line;
    }
    if(fcv_flag) {
      urlcon.force_bad();
    }
    //if(!urlcon.bad())
    //  cerr << line << endl;
    // insert here &amp; -> &
    return urlcon;
  } catch (HTMLparseExceptionType err) {
    if(fsavefile) outfile << line;
    urlcon.force_bad();// 強制的にbad状態へ
    return urlcon;
  } 
}

URLcontainer HTMLparse::get_href(Options& options)
{
  bool fsavefile = (options.ret_with_hostname_dir() && options.ret_abs2rel_url() || options.ret_delete_comment() || options.ret_delete_javascript()) && !outfile_bad;
  bool abs2rel = options.ret_with_hostname_dir() && options.ret_abs2rel_url() && !outfile_bad;
  //bool keylink_flag = false;
  //if(keylink_list.size()) keylink_flag = true;
  if(endwith(infilename, ".css")) {
    //cerr << "css iteration in file " << infilename << endl;
    //bool commentFlag = false;
    while(1) {      
      string line;
      getline(infile, line, ';');

      if(line.empty() && infile.eof()) {
	throw HTMLPARSE_EOF;
      }
      unsigned int index;
      if((index = line.find("@import")) != string::npos) {
	index = line.find_first_not_of(" \t", index+7);// 7 is the length of string "@import"
	if(index != string::npos) {
	  URLcontainer urlcon;
	  if(fsavefile) outfile << line.substr(0, index);
	  urlcon = find_css(line.substr(index), options);
	  if(fsavefile) outfile << ';';
	  if(!urlcon.bad()) return urlcon;
	} else {
	  if(fsavefile) outfile << line << ';';
	}
      } else if((index = line.find("url(")) != string::npos) {
	URLcontainer urlcon;
	if(fsavefile) outfile << line.substr(0, index);
	urlcon = find_css(line.substr(index), options);
	if(fsavefile) outfile << ';';
	if(!urlcon.bad()) return urlcon;
      } else { 
	if(fsavefile) outfile << line << ';';
      }
    }
  } else {
  while(1) {
    string line;
    getline(infile, line, '<');
    if(fsavefile) outfile << line;
    if(line.empty() && infile.eof()) {
      throw HTMLPARSE_EOF;
    }
    //if(abs2rel) outfile << '<';

    getline(infile, line, '>');
    line = Remove_white(line);
    if(startwith(line, "script")
       && (casefind(line, "language=\"javascript\"") != string::npos
	   || casefind(line, "language=javascript") != string::npos
	   || casefind(line, "type=\"text/javascript\"") != string::npos)) {
      URLcontainer urlcon;
      if(fsavefile && !options.ret_delete_javascript()) {
	outfile << '<';
	urlcon = find_href(line, options);
	outfile << '>';
      }
      while(1) {
	line = "";
	getline(infile, line, '>');
	if(line.empty() && infile.eof()) throw HTMLPARSE_EOF;
	if(fsavefile && !options.ret_delete_javascript()) {
	  outfile << line << '>';
	}
	if(endwith(line, "/script")) {
	  break;
	}
      }
      if(!urlcon.bad()) {
	return urlcon;
      }
    } else if(casecomp(line, "style")) {
      if(fsavefile) {
	outfile << '<' << line << '>';
      }
      while(1) {
	line = "";
	getline(infile, line, '>');
	if(line.empty() && infile.eof()) throw HTMLPARSE_EOF;
	if(fsavefile) {
	  outfile << line << '>';
	}
	if(endwith(line, "/style")) {
	  break;
	}
      }
    } else if(startwith(line, "!--")) {
      if(abs2rel && !options.ret_delete_comment()) {
	outfile << '<' << line << '>';
      }
      if(line.size() <= 3 ||
	 !endwith(line, "--")) {
	while(1) {
	  line = "";
	  getline(infile, line, '>');
	  if(line.empty() && infile.eof()) throw HTMLPARSE_EOF;
	  if(abs2rel && !options.ret_delete_comment()) {
	    outfile << line << '>';
	  }
	  if(endwith(line, "--")) {
	    break;
	  }
	}
      }
    } else if(startwith(line, "pre")) {
      if(fsavefile) outfile << '<' << line << '>';
      while(1) {
	line = "";
	getline(infile, line, '>');
	if(line.empty() && infile.eof()) throw HTMLPARSE_EOF;
	if(fsavefile) outfile << line << '>';
	if(endwith(line, "/pre")) {
	  break;
	}
      }
    } else if(isTag(line, "base")) {
      // <base href="...">
      if(fsavefile) outfile << '<';
      URLcontainer urlcon = find_href(line, options);
      if(fsavefile) outfile << '>';
      if(!urlcon.bad()) {
	baseHref = urlcon.ret_URL();
	if(!endwith(baseHref, "/")) {
	  baseHref += '/';
	}
      }
    } else if(Remove_white(line).empty()) {
      if(fsavefile) outfile << line;
    } else {
      // remove "iframe" if requested
      // <iframe SRC="http://ad4.cool.ne.jp/html/?media=007" FRAMEBORDER="0" WIDTH="468" HEIGHT="60" MARGINWIDTH="0" MARGINHEIGHT="0" SCROLLING="no">
      // </iframe>
      
      if(options.ret_delete_iframe() && startwith(line, "iframe")) {
	while(1) {
	  line = "";
	  getline(infile, line, '>');
	  if(line.empty() && infile.eof()) throw HTMLPARSE_EOF;
	  if(endwith(line, "/iframe")) {
	    break;
	  }
	}
      } else {
	if(fsavefile) outfile << '<';
	URLcontainer urlcon = find_href(line, options);
	if(fsavefile) outfile << '>';
	if(!urlcon.bad()) {
	  return urlcon;
	}
      }
    }
  }
  }
}

bool HTMLparse::in_bad() const
{
  return infile_bad;
}

bool HTMLparse::out_bad() const
{
  return outfile_bad;
}

HTMLparse::HTMLparse(const string& base_url_in,
		     const string& root_url_in,
		     const string& documentroot_dir_in,
		     const Options& static_options_in,
		     const string& infilename_in)

{
  infilename = infilename_in;
  base_url = base_url_in;
  root_url = root_url_in;
  documentroot_dir = documentroot_dir_in;
  baseHref = "";

  if(static_options_in.ret_with_hostname_dir() &&
     static_options_in.ret_abs2rel_url() ||
     static_options_in.ret_delete_comment() ||
     static_options_in.ret_delete_javascript()) {
    unsigned int pos = documentroot_dir.size();
    while((pos = static_options_in.ret_Store_Dir().find('/', pos+1)) != string::npos) {
      prefix += "../";
    }
    outfilename = infilename+"."+itos(time(NULL))+itos((int)((float)100*random()/(RAND_MAX+1.0)));
  }

  infile.open(infilename.c_str(), ios::in);
  if(infile.bad()) {
    infile_bad = true;
  } else {
    infile_bad = false;
  }

  if(outfilename.size()) {
    outfile.open(outfilename.c_str(), ios::out);
    if(outfile.bad() || outfile.fail()) {
      outfile_bad = true;
    } else {
      outfile_bad = false;
    }
  } else {
    outfile_bad = true;
  }
}

HTMLparse::~HTMLparse()
{
  if(!outfile_bad)
    outfile.close();
  if(!infile_bad)
    infile.close();

  if(!outfile_bad) {
    if(unlink(infilename.c_str()) < 0) {
      throw HTMLPARSE_EIO;
    }
    if(rename(outfilename.c_str(), infilename.c_str()) < 0) {
      throw HTMLPARSE_EIO;
    }
  }
}
