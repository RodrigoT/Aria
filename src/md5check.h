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

// $Id: md5check.h,v 1.4 2001/10/20 08:56:18 tujikawa Exp $

#ifndef _MD5CHECK_H_
#define _MD5CHECK_H_

#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include "md5.h"
#ifdef __cplusplus
}
#endif /* __cplusplus */

//prototype declaration
enum md5ExceptionType {
  MD5_IOERR
};

string md5_check(const string& filename);
string md5CheckString(const string& srcString);

#endif // _MD5CHECK_H_
