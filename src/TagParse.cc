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

// $Id: TagParse.cc,v 1.2 2001/02/26 10:10:55 tujikawa Exp $

#include "TagParse.h"

string get_next_tag(ifstream& infile)
{
  string tag;
  while(1) {
    getline(infile, tag, '>');
    tag = Remove_white(tag);
    if(tag.size()) {
      if(startwith(tag, "<!--")) {
	if(!endwith(tag, "--")) {
	  while(1) {
	    getline(infile, tag, '>');
	    if(endwith(tag, "--")) {
	      break;
	    }
	    if(!infile.good()) {
	      throw TAGPARSE_UCOM_EOF;
	    }
	  }
	}
      } else {
	break;
      }
    }
    if(!infile.good()) {
      throw TAGPARSE_GETTAG_EOF;
    }

  }
  tag += '>';
  return tag;
}

string get_value(ifstream& infile, string tag)
{
  string line;

  tag.insert(tag.begin()+1, '/'); // end of tag

  while(1) {
    string line_temp = line;
    getline(infile, line, '>');
    line = line_temp+line+'>';

    if(line.find(tag) != string::npos) {
      line.erase(line.size()-tag.size());
      break;
    }
    if(!infile.good()) {
      throw TAGPARSE_GETVALUE_EOF;
    }
  }
  line = Remove_white(line);
  return line;
}
