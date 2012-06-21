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

// $Id: Cookie.h,v 1.5 2001/11/04 10:18:07 tujikawa Exp $

#ifndef _COOKIE_H_
#define _COOKIE_H_
#include <string>
#include <map>
#include <list>
#include <time.h>
#include "utils.h"

using namespace std;

class Cookie
{
private:
  map<string, pair<string, time_t> > key_value_map;
  string domain;
  string path;
  bool valid;
public:
  Cookie(const string& name, const string& value, const string& expires, const string& domain, const string& path);
  Cookie();
  ~Cookie();

  void all_clear();
  bool Parse(string cookie_string);
  void set_domain(const string& domain);
  time_t interpret_date(string date_gmt);
  bool Is_valid() const;
  bool Is_alive(const string& key, const string& host, const string& dir);
  bool Is_expired(const string& key);
  bool Is_expired(time_t expire_time) const;
  
  bool Is_in_path(const string& dir) const;
  bool Is_in_domain(const string& host) const;
  const map<string, pair<string, time_t> >& ret_key_value_map() const;
  string ret_names(const string& domain, const string& path) const;
  const string& ret_domain() const;
  const string& ret_path() const;
  void merge(const Cookie& cookie);
};

#endif //_COOKIE_H_
