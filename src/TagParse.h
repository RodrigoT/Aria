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

// $Id: TagParse.h,v 1.2 2001/09/11 13:13:32 tujikawa Exp $

#ifndef _TAGPARSE_H_
#define _TAGPARSE_H_
#include <iostream>
#include <fstream>
#include <string>
#include "utils.h"

using namespace std;

#define TAGPARSE_GETTAG_EOF 1000
#define TAGPARSE_GETVALUE_EOF 1001
#define TAGPARSE_UCOM_EOF 1002

string get_next_tag(ifstream &infile);
string get_value(ifstream &infile, string tag);

#endif // _TAGPARSE_H_

