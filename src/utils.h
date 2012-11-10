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

// $Id: utils.h,v 1.23 2002/02/13 12:09:24 tujikawa Exp $
#ifndef _UTILS_H_
#define _UTILS_H_

#include <string>
#include <list>
#include <iomanip>
#include <fstream>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
//#include "aria.h"

using namespace std;

string Remove_white(string nword);
string Token_splitter(string& line, const char *delimitors=" \t");
string get_human_readable_size(int size);
// convert an integer to a string
string itos(int value, bool comma = false);
string itos(int vlaue, unsigned int width, char ch);

string itos_hex(unsigned int value);
string itos_hex(unsigned int value, unsigned int width, char ch);

string ftos(float value, unsigned int precision = 2);

int power(int base, int power);
string insert_comma(const string& val_str);

int stoi(const string& src_string, int base = 10);
unsigned int stoui(const string& src_string, int base = 10);

bool isalpha_all(const string& src_string);
bool isdigit_all(const string& src_string);

#define GETABSURL_EUSTACK 0 // stack under run
string get_abs_url(string base_url, string target);
#define GETSTOREDIR_EUSTACK 0 // stack under run
string get_storedir(string store_dir, string target);
string get_hostname(string url);
string get_file(string& target);
string get_file_mod_date(const string& filename);
size_t casefind(const string& string1, const string& string2);
bool startwith(const string& string1, const string& string2);
bool casecomp(const string& string1, const string& string2);
bool endwith(const string& string1, const string& string2);
time_t get_mod_time(const string& mdtm_string);
string convert_tilde(const string& src_str);
bool copy_file(const string& srcfile, const string& destfile);
string replaceSubstring(const string& srcStr, const string& oldSubstr, const string& newSubstr);
string removeCtrlChar(const string& srcStr);
bool patternMatch(string str, string pattern);
void updateInterval(struct timeval &dst, int timeout);
void updateInterval(struct timeval &dst, struct timeval src);
void updateInteverl(struct timeval &dst,
		    const struct timeval &start,
		    const struct timeval &end);
string get_protocol_from_url(const string& base_url);
#endif //_UTILS_H_
