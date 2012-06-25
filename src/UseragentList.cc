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

// $Id: UseragentList.cc,v 1.5 2001/10/01 12:45:26 tujikawa Exp $

#include "UseragentList.h"

UseragentList::UseragentList()
{
}

UseragentList::UseragentList(const UseragentList& useragentlist_src)
{
  useragent_list = useragentlist_src.ret_vector();
}

UseragentList::~UseragentList()
{
}

bool UseragentList::add(const string& useragent_in)
{
  string useragent = Remove_white(useragent_in);
  if(useragent.empty()) return false;
  else {
    useragent_list.push_back(useragent);
    return true;
  }
}

const vector<string>& UseragentList::ret_vector() const
{
  return useragent_list;
}

bool UseragentList::Read_useragent_list(const string& file_useragent_list)
{
  ifstream infile(file_useragent_list.c_str(), ios::in);//ios::skipws|ios::in);
  if(infile.bad() || infile.eof()) return false;

  while(infile.good()) {
    string line;
    getline(infile, line, '\n');
    if(infile.bad()) return false;
    if(line.empty() || line.at(0) == '#') continue;
    add(line);
  }
  return true;
}
