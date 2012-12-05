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

// $Id: CookieList.h,v 1.3 2002/04/03 13:33:51 tujikawa Exp $

#ifndef _COOKIELIST_H_
#define _COOKIELIST_H_
#include "Cookie.h"

class CookieList
{
private:
    list<Cookie> cookie_list;
public:
    CookieList();
    ~CookieList();

    void add_cookie(const Cookie &cookie);
    void add_cookie(string cookie_string);
    list<Cookie>::iterator search_by_domain_path(const string &domain, const string &path);
    string ret_valid_cookie_string(const string &domain, const string &path) const;
    void all_clear();
};

#endif //_COOKIELIST_H_
