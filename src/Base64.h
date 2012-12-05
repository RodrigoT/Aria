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

// $Id: Base64.h,v 1.3 2001/06/09 08:52:58 tujikawa Exp $

#ifndef _BASE64_H_
#define _BASE64_H_
#include <string>
using namespace std;

class Base64
{
private:
    static string part_encode(const string &subplain);
    static string part_decode(const string &subCrypted);
    static char getValue(char ch);
public:
    Base64();
    static string encode(string plain);
    static string decode(string crypted);
};

#endif // _BASE64_H_
