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

// $Id: CRCList.h,v 1.4 2001/03/09 06:49:09 tujikawa Exp $

#ifndef _CRCLIST_H_
#define _CRCLIST_H_

#include <string>
#include "aria.h"
using namespace std;

class CRCList
{
private:
    string filename;
    string crc_string;

public:
    CRCList(const string &file, const string &crc_str);
    ~CRCList();

    const string &ref_Filename() const;
    const string &ref_CRC_string() const;
};

#endif // _CRCLIST_H_
