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

// $Id: StringHash.h,v 1.2 2002/04/03 13:33:52 tujikawa Exp $

#ifndef _STRINGHASH_H_
#define _STRINGHASH_H_
#include <glib.h>
#include <string>
#include <iostream>

using namespace std;

class StringHash
{
private:
    GHashTable *hashTable;

    void destory();
public:
    StringHash();

    ~StringHash();

    // adds the key-value pair to the hash table
    void add(string key, string value);

    // returns the size of the hash table
    int size();

    // gets the value associated with the key
    string get(string key);

    // clears all the key-value pair in the hash table
    void clear();

    // returns true if the value associated with the key is found,
    // returns false otherwise.
    bool find(string key);
};
#endif // _STRINGHASH_H_
