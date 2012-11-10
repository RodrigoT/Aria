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

// $Id: utils.cc,v 1.26 2002/10/01 15:32:00 tujikawa Exp $
#include "utils.h"
#include <stdlib.h>
#include <strings.h>

string Remove_white(string nword)
{
  //string nword = word;
  std::size_t first_pos = nword.find_first_not_of(" \t\r\n");
  if(first_pos == string::npos) {
    first_pos = nword.size();
  }
  nword.erase(0, first_pos);

  std::size_t last_pos = nword.find_last_not_of(" \t\r\n");
  if(last_pos != string::npos) {
    nword.erase(last_pos+1);
  }
  
  /*
  std::size_t first_pos = nword.find_first_not_of(" \t");
  if(first_pos != string::npos) {
    nword.erase(0, first_pos);
  }
  std::size_t last_pos = nword.find_last_not_of(" \t\r");
  if(last_pos != string::npos) {
    nword.erase(last_pos+1);
  }
  */
  return nword;
}

string Token_splitter(string& line, const char *delimitors)
{
  string token;
  size_t start_pos = line.find_first_not_of(" \t");
  if(start_pos == string::npos) {
    line.erase();
    return token;
  }
  size_t end_pos = line.find_first_of(delimitors, start_pos);
  if(end_pos == string::npos) {
    end_pos = line.size();
  }
  token = line.substr(start_pos, end_pos-start_pos);
  size_t erase_pos = line.find_first_not_of(delimitors, end_pos+1);
  if(erase_pos == string::npos) {
    line.erase();
  } else {
    line.erase(0, erase_pos);
  }
  return token;
}

string get_human_readable_size(int size_int)
{
  float size = (float)size_int;
  const float blocksize = 1024.0;
  string s;
  if(size < blocksize) {
      s = ftos(size, 0);
  } else {
    size /= blocksize;
    if(size < blocksize) {
      if(size <= 10) {
	s = ftos(size, 1)+"K";
      } else {
	s = ftos(size, 0)+"K";
      }
    } else {
      size /= blocksize;
      if(size < blocksize) {
	if(size <= 10) {
	  s = ftos(size, 1)+"M";
	} else {
	  s = ftos(size, 0)+"M";
	}
      } else {
	size /= blocksize;
	if(size <= 10) {
	  s = ftos(size, 1)+"G";
	} else {
	  s = ftos(size, 0)+"G";
	}
      }
    }
  }
  return s;
}

int stoi(const string& src_string, int base)
{
 return strtol(src_string.c_str(), (char**)NULL, base);
}

unsigned int stoui(const string& src_string, int base)
{
 return strtoul(src_string.c_str(), (char**)NULL, base);
}

string itos(int value, bool comma)
{
  string str;
  bool flag = false;
  unsigned count = 0;
  if(value < 0) {
    flag = true;
    value = -value;
  } else if(value == 0) {
    str = "0";
  }
  while(value) {
    ++count;
    char digit = value%10+'0';
    str.insert(str.begin(), digit);
    value /= 10;
    if(comma && count > 3 && count%3 == 1) {
      str.insert(str.begin()+1, ',');
    }
  }
  if(flag) {
    str.insert(str.begin(), '-');
  }

  return str;
}

string itos(int value, unsigned int width, char fill)
{
  string str = itos(value);

  string::iterator itr;
  if(value < 0) {
    itr = str.begin()+1;
  } else {
    itr = str.begin();
  }

  if(width > (unsigned int)str.size()) {
    str.insert(itr, width-str.size(), fill);
  }

  return str;
}

string itos_hex(unsigned int value)
{
  string str;
  while(value) {
    char digit;
    if(value%16 > 9) {
      digit = (value-10)%16+'A';
    } else {
      digit = value%16+'0';
    }
   
    str.insert(str.begin(), digit);
    value /= 16;
  }
  return str;
}

string itos_hex(unsigned int value, unsigned int width, char fill)
{
  string str = itos_hex(value);
  str.insert(str.begin(), width-str.size(), fill);
  return str;
}

string ftos(float value, unsigned int precision)
{
  int value_int = (int)(value*power(10, precision));
  string str = itos(value_int);
  unsigned int offset;
  if(value > 0) offset = 0;
  else offset = 1;
  if(str.size() <= precision+offset) {
    str.insert(str.begin()+offset, precision+1-str.size(), '0');
  }
  //if(str.size()-offset-precision > 0) {
  //  str.erase(str.size()-precision);
  //  } else
  if(precision) {
    str.insert(str.size()-precision, ".");
  }

  return str;
}

int power(int base, int power)
{
  int result = 1;
  for(int i = 0; i < power; ++i) {
    result *= base;
  }

  return result;
}

bool isalpha_all(const string& src_string)
{
  for(string::const_iterator itr = src_string.begin(); itr != src_string.end(); ++itr) {
    if(!isalpha(*itr)) return false;
  }
  return true;
}

bool isdigit_all(const string& src_string)
{
  for(string::const_iterator itr = src_string.begin(); itr != src_string.end(); ++itr) {
    if(!isdigit(*itr)) return false;
  }
  return true;
}

string get_storedir(string store_dir, string target) 
{
  list<string> sd_list;
  while(store_dir.size()) {
    string dir = Token_splitter(store_dir, "/");
    if(dir.size()) {
      sd_list.push_back(dir);
    }
  }
  while(target.size()) {
    string dir = Token_splitter(target, "/");
    if(dir == "..") {
      if(sd_list.size()) {
	sd_list.pop_back();
      } else {
	// throw exception
	throw GETSTOREDIR_EUSTACK;
      }
    } else if(dir == ".") {
    } else if(dir.size()) {
      sd_list.push_back(dir);
    }
  }
  string retstr = "/";
  for(list<string>::iterator itr = sd_list.begin(); itr != sd_list.end(); ++itr) {
    retstr += *itr+"/";
  }
  return retstr;
}

string get_protocol_from_url(const string& base_url)
{
  std::size_t pos = base_url.find(':');
  if(pos != string::npos) {
    return base_url.substr(0, pos);
  } else {
    return "";
  }
}

string get_abs_url(string base_url, string target) 
{
  string retstr;
  if(target.size() && target.at(0) == '/') {
    return get_protocol_from_url(base_url)+"://"+
      get_hostname(base_url)+target;
  }
  std::size_t file_pos = target.find_last_of("/");
  string file;
  if(file_pos != string::npos) {
    file = target.substr(file_pos+1);
    target.erase(file_pos+1);

    list<string> sd_list;
    while(base_url.size()) {
      string dir = Token_splitter(base_url, "/");
      if(dir.size()) {
	sd_list.push_back(dir);
      }
    }
    while(target.size()) {
      string dir = Token_splitter(target, "/");
      if(dir == "..") {
	if(sd_list.size()) {
	  sd_list.pop_back();
	} else {
	  // throw exception
	  throw GETABSURL_EUSTACK;
	}
      } else if(dir == ".") {
      } else if(dir.size()) {
	sd_list.push_back(dir);
      }
    }
    for(list<string>::iterator itr = sd_list.begin(); itr != sd_list.end(); ++itr) {
      retstr += *itr+'/';
      if(*itr == "http:") retstr += '/';
#ifdef HAVE_OPENSSL
      else if(*itr == "https:") retstr += '/';
#endif // HAVE_OPENSSL
      else if(*itr == "ftp:") retstr += '/';
    }
    
  } else {
    if(target.size() && target.at(0) != '/' && base_url.at(base_url.size()-1) != '/') {
      target = '/'+target;
    }
    retstr = base_url+target;
  }

 retstr += file;
  return retstr;
}

void erase_protocol(string& url)
{
  int slash_index = url.find("//");
  if(slash_index <= 0) return;//throw URLCON_EINVALIDURL;
  string protocol = url.substr(0, slash_index);
  url.erase(0, slash_index+2);
  //if(!Is_supported_protocol(protocol)) return "";throw URLCON_EUNSUPP;
}

string get_hostname(string url)
{
  erase_protocol(url);
  std::size_t slash_index = url.find('/');
  if(slash_index == string::npos) {
    slash_index = url.size();
  }
  string host = url.substr(0, slash_index);
  //url.erase(0, slash_index);
  std::size_t colon_index = host.rfind(':');
  if(colon_index != string::npos) {
    //int port_temp = stoi(host.substr(colon_index+1), 10);
    //if(port_temp != 0) port = port_temp;
    host.erase(colon_index);
  }
  return host;
}

string get_file(string& target)
{
  std::size_t file_pos = target.find_last_of("/");
  if(file_pos == string::npos) {
    target.erase();
    return target;
  }
  string file = target.substr(file_pos+1);
  target.erase(file_pos+1);
  return file;
};

string get_wday(int wday)
{
  string wdaystr;
  switch(wday) {
  case 0:
    wdaystr = "Sun, ";
    break;
  case 1:
    wdaystr = "Mon, ";
    break;
  case 2:
    wdaystr = "Tue, ";
    break;
  case 3:
    wdaystr = "Wed, ";
    break;
  case 4:
    wdaystr = "Thu, ";
    break;
  case 5:
    wdaystr = "Fri, ";
    break;
  case 6:
    wdaystr = "Sat, ";
    break;
  default:
    wdaystr = "Sun, ";
    break;
  }
  return wdaystr;
}

string get_month(int mon)
{
  string monstr;
  switch(mon) {
  case 0:
    monstr = "Jan ";
    break;
  case 1:
    monstr = "Feb ";
    break;
  case 2:
    monstr = "Mar ";
    break;
  case 3:
    monstr = "Apr ";
    break;
  case 4:
    monstr = "May ";
    break;
  case 5:
    monstr = "Jun ";
    break;
  case 6:
    monstr = "Jul ";
    break;
  case 7:
    monstr = "Aug ";
    break;
  case 8:
    monstr = "Sep ";
    break;
  case 9:
    monstr = "Oct ";
    break;
  case 10:
    monstr = "Nov ";
    break;
  case 11:
    monstr = "Dec ";
    break;
  default:
    monstr = "Jan ";
    break;
  }
  return monstr;
}

string get_file_mod_date(const string& filename)
{
  struct stat file_stat;
  time_t modtime = 0;
  if(stat(filename.c_str(), &file_stat) == 0 && S_ISREG(file_stat.st_mode)){
    modtime = file_stat.st_mtime;
  }
  struct tm *timest;
  timest = gmtime(&modtime);
  string date;
  date += get_wday(timest->tm_wday);
  date += itos(timest->tm_mday, 2, '0')+' ';
  date += get_month(timest->tm_mon);
  date += itos(timest->tm_year+1900)+' ';
  date += itos(timest->tm_hour, 2, '0')+':'+
    itos(timest->tm_min, 2, '0')+':'+itos(timest->tm_sec, 2, '0')+' ';
  date += "GMT";

  return date;
}

size_t casefind(const string& string1, const string& string2)
{
  if(string1.size() < string2.size()) return string::npos;

  for(std::size_t index = 0; index < string1.size()-string2.size()+1; ++index) {
    if(!strncasecmp(string1.substr(index).c_str(), string2.c_str(), string2.size())) {
      return index;
    }
  }
  return string::npos;
}

bool casecomp(const string& string1, const string& string2)
{
  if(string1.size() != string2.size()) return false;
  if(!strcasecmp(string1.c_str(), string2.c_str())) {
    return true;
  } else {
    return false;
  }
}

bool startwith(const string& string1, const string& string2)
{
  if(string1.size() < string2.size()) return false;

  if(!strncasecmp(string1.c_str(), string2.c_str(), string2.size())) {
    return true;
  } else {
    return false;
  }
}

bool endwith(const string& string1, const string& string2)
{
  if(string1.size() < string2.size()) return false;

  if(!strncasecmp(string1.substr(string1.size()-string2.size()).c_str(), string2.c_str(), string2.size())) {
    return true;
  } else {
    return false;
  }
}

string insert_comma(const string& val_str)
{
  std::size_t pos = val_str.size();
  string ret_str;
  while(pos > 3) {
    ret_str = ','+val_str.substr(pos-3, 3)+ret_str;
    pos -= 3;
  }
  return val_str.substr(0, pos)+ret_str;
}

time_t get_mod_time(const string& mdtm_string)
{
  if(mdtm_string.size() < 14) return 0;
  struct tm time_st;
  time_st.tm_year = stoi(mdtm_string.substr(0, 4))-1900;
  time_st.tm_mon = stoi(mdtm_string.substr(4, 2))-1;
  time_st.tm_mday = stoi(mdtm_string.substr(6, 2));
  time_st.tm_hour = stoi(mdtm_string.substr(8, 2));
  time_st.tm_min = stoi(mdtm_string.substr(10, 2));
  time_st.tm_sec = stoi(mdtm_string.substr(12, 2));
  time_st.tm_wday = 0;

  time_t time_gmt = mktime(&time_st);

#ifdef HAVE_TIMEZONE
  time_gmt -= timezone;
#else
  time_t lt = time(NULL);
  struct tm *ltm = localtime(&lt);
  time_gmt += ltm->tm_gmtoff;
#endif
  //cerr << ctime(&time_gmt) << endl;
  return time_gmt;
}

string convert_tilde(const string& src_str)
{
  string mod_str = src_str;
  std::size_t tilde_pos = src_str.rfind('~');
  if(tilde_pos != string::npos) {
    mod_str.erase(tilde_pos, 1);
    mod_str.insert(tilde_pos, "%7E");
  }
  return mod_str;
}

bool copy_file(const string& srcfile, const string& destfile)
{
  ifstream infile(srcfile.c_str(), ios::in);
  if(infile.bad()) return false;
  ofstream outfile(destfile.c_str(), ios::out);
  if(outfile.bad()) return false;
  char buffer[8192];

  while(infile.good()) {
    infile.read(buffer, sizeof(buffer));
    if(infile.bad()) return false;
    outfile.write(buffer, infile.gcount());
    if(outfile.bad()) return false;
  }

  return true;
}

string replaceSubstring(const string& srcStr, const string& oldSubstr, const string& newSubstr) {
  std::size_t index = 0;
  string dstStr = srcStr;
  while(1) {
    if((index = dstStr.find(oldSubstr, index)) == string::npos) {
      return dstStr;
    }
    dstStr.erase(index, oldSubstr.size());
    dstStr.insert(index, newSubstr);
    index += newSubstr.size();
  }
}

// remove control chars from a given string
string removeCtrlChar(const string& srcStr) {
  string dstStr = srcStr;
  std::size_t index = 0;
  while(1) {
    if((index = dstStr.find_first_of("\r\n", index)) == string::npos) {
      return dstStr;
    }
    dstStr.erase(index);
  }
}


static string patternGetNextToken(string& pattern)
{
  string retStr;

  std::size_t wcIndex = pattern.find_first_of("*?");
  if(wcIndex == string::npos) {
    retStr = pattern;
    pattern.erase();
  } else {
    string subpattern = pattern.substr(0, wcIndex);
    if(subpattern.empty()) {
      retStr = pattern.at(wcIndex);
      pattern.erase(0, 1);
    } else {
      retStr = subpattern;
      pattern.erase(0, wcIndex);
    }
  }

  return retStr;
}

bool patternMatch(string str, string pattern)
{
  // wildcards: *, ?
  unsigned int sIndex = 0;
  char wc = 0;
  while(1) {
    string subpattern = patternGetNextToken(pattern);
    if(subpattern.empty() && pattern.empty()) {
      if(wc == '?') {
	return true;;
      }
      break;
    }
    if(subpattern == "?") {
      if(sIndex >= str.size()) {
	return false;
      }
      ++sIndex;
      wc = '?';
    } else if(subpattern == "*") {
      wc = '*';
    } else {
      unsigned int searchSize;
      if(wc) {
	if(str.size() < sIndex) {
	  return false;
	}
	//	cerr << "str.size() = " << str.size() << endl;
	searchSize = str.size()-sIndex;
	wc = 0;
      } else {
	searchSize = subpattern.size();
      }
//        cerr << sIndex << ", " << searchSize << endl;
//        cerr << str.substr(sIndex, searchSize) << endl;
      std::size_t index = casefind(str.substr(sIndex, searchSize), subpattern);
      if(index == string::npos) {
	return false;
      }
      sIndex = index+subpattern.size();
      //cerr << "sIndex = " << sIndex << endl;
    }
  }

  return true;
}

void updateInterval(struct timeval &dst, int timeout)
{
  if(timeout < dst.tv_sec ||
     (timeout == dst.tv_sec && dst.tv_usec > 0)) {
    dst.tv_sec = 0;
    dst.tv_usec = 0;
  }
  dst.tv_sec = timeout-dst.tv_sec;
  if(dst.tv_usec > 0) {
    --dst.tv_sec;
    dst.tv_usec = 1000000-dst.tv_usec;
  } 
}

void updateInterval(struct timeval &dst, struct timeval src)
{
  if(src.tv_sec < dst.tv_sec ||
     (src.tv_sec == dst.tv_sec && dst.tv_usec > 0)) {
    dst.tv_sec = 0;
    dst.tv_usec = 0;
  }
  dst.tv_sec = src.tv_sec-dst.tv_sec;
  if(src.tv_usec < dst.tv_usec) {
    --dst.tv_sec;
    dst.tv_usec = (1000000+src.tv_usec)-dst.tv_usec;
  } else {
    dst.tv_usec = src.tv_usec-dst.tv_usec;
  }
}

void updateInterval(struct timeval& dst,
		    const struct timeval& start,
		    const struct timeval& end)
{
  struct timeval split;
  split.tv_sec = end.tv_sec-start.tv_sec;
  if(end.tv_usec < start.tv_usec) {
    --split.tv_sec;
    split.tv_usec = (1000000+end.tv_usec)-start.tv_usec;
  } else {
    split.tv_usec = end.tv_usec-start.tv_usec;
  }

  if(dst.tv_sec < split.tv_sec
     || (dst.tv_sec == split.tv_sec
	 && dst.tv_usec < split.tv_usec)) {
    dst.tv_sec = 0;
    dst.tv_usec = 0;
  } else {
    dst.tv_sec = dst.tv_sec-split.tv_sec;
    if(dst.tv_usec < split.tv_usec) {
      --dst.tv_sec;
      dst.tv_usec = (1000000+dst.tv_usec)-split.tv_usec;
    } else {
      dst.tv_usec = dst.tv_usec-split.tv_usec;
    }
  }
}
