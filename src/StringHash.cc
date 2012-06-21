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

// $Id: StringHash.cc,v 1.2 2002/04/03 13:33:52 tujikawa Exp $

#include "StringHash.h"

StringHash::StringHash()
{
  hashTable = g_hash_table_new(g_str_hash,
			       g_str_equal);
}

StringHash::~StringHash()
{
  destory();
}

static void aGHFunc(gpointer key,
		    gpointer value,
		    gpointer userData)
{
  g_free(key);
  g_free(value);
}

void StringHash::destory()
{
  g_hash_table_foreach(hashTable,
		       aGHFunc,
		       0);
  g_hash_table_destroy(hashTable);
}

void StringHash::clear()
{
  destory();
  hashTable = g_hash_table_new(g_str_hash,
			       g_str_equal);
}

void StringHash::add(string key, string value)
{
  char* keyCStr = g_strdup(key.c_str());
  char* valueCStr = g_strdup(value.c_str());
  
  // Check the same key already exists in the hash table.
  // If such a key exists, then remove it from the hash table.
  char* origKey = 0;
  char* origValue = 0;
  if(g_hash_table_lookup_extended(hashTable,
				  keyCStr,
				  (gpointer*)&origKey,
				  (gpointer*)&origValue)) {
    // first, remove the original key-value pair
    g_hash_table_remove(hashTable,
			origKey);
    // then, free up its memory
    g_free(origKey);
    g_free(origValue);
  }

  // insert the key-value pair.
  g_hash_table_insert(hashTable,
		      keyCStr,
		      valueCStr);

}

string StringHash::get(string key)
{
  char* str  = (char*)g_hash_table_lookup(hashTable, key.c_str());

  if(str == 0) {// str is NULL
    // if the value associated with the key is not found,
    // returns ""
    return("");
  } else {
    return(str);
  }
}

bool StringHash::find(string key)
{
  if(g_hash_table_lookup(hashTable, key.c_str()) == 0) {
    return(false);
  } else {
    return(true);
  }
}

int StringHash::size()
{
  return((int)g_hash_table_size(hashTable));
}
