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

// $Id: UseragentList.h,v 1.4 2001/09/11 13:13:32 tujikawa Exp $

#ifndef _USERAGENTLIST_H_
#define _USERAGENTLIST_H_

#include <vector>
#include <fstream>
#include "aria.h"
#include "utils.h"
using namespace std;

class UseragentList
{
private:
    vector<string> useragent_list;
public:
    UseragentList();
    UseragentList(const UseragentList &useragentlist_src);
    ~UseragentList();

    bool add(const string &useragent_in);
    const vector<string> &ret_vector() const;
    bool Read_useragent_list(const string &filename);
};

#endif // _USERAGENTLIST_H_
