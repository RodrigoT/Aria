//
//  aria - yet another download tool
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

// $Id: CookieList.cc,v 1.2 2001/02/22 09:36:50 tujikawa Exp $

#include "CookieList.h"


CookieList::CookieList()
{
}

CookieList::~CookieList()
{
}

void CookieList::all_clear()
{
  cookie_list.clear();
}

void CookieList::add_cookie(const Cookie& cookie)
{
  list<Cookie>::iterator itr = search_by_domain_path(cookie.ret_domain(), cookie.ret_path());

  if(itr != cookie_list.end()) {
    itr->merge(cookie);
  } else {
    cookie_list.push_back(cookie);
  }
}

void CookieList::add_cookie(string cookie_string)
{
  Cookie cookie;
  if(!cookie.Parse(cookie_string)) {
    return;
  }
  //if(cookie.ret_domain().empty()) cookie.set_domain(hostname);

  add_cookie(cookie);
}

list<Cookie>::iterator CookieList::search_by_domain_path(const string& domain, const string& path)
{
  for(list<Cookie>::iterator itr = cookie_list.begin(); itr != cookie_list.end(); ++itr) {
    if(itr->ret_domain() == domain && itr->ret_path() == path) return itr;
  }
  return cookie_list.end();
}

string CookieList::ret_valid_cookie_string(const string& domain, const string& path) const
{
  string key_value;
  for(list<Cookie>::const_iterator itr = cookie_list.begin(); itr != cookie_list.end(); ++itr) {
    key_value += itr->ret_names(domain, path);
  }
  if(key_value.size()) {
    key_value.erase(key_value.size()-2);
  }
  return key_value;
}


