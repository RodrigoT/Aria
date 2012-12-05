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

// $Id: HTTP_Header.h,v 1.8 2001/10/17 13:06:34 tujikawa Exp $

#ifndef _HTTP_HEADER_H_
#define _HTTP_HEADER_H_

#include <string>
#include "aria.h"
using namespace std;

// server Status-Code
#define OK 200
#define PartialContent 206
#define MultipleChoices 300
#define MovedPermanently 301
#define Found 302
#define SeeOther 303
#define NotModified 304
#define UseProxy 305
#define Forbidden 403
#define NotFound 404
#define RequestedRangeNotSatisfiable 416
#define ServiceUnavailable 503
#define AuthorizationRequired 401

class HTTP_Header
{
private:
    string item;
    string arg;
public:
    HTTP_Header(const string &item, const string &arg);
    ~HTTP_Header();
    const string &ret_Arg() const;
    const string &ret_Item() const;
};

#endif // _HTTP_HEADER_
