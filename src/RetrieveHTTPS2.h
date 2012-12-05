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

// $Id: RetrieveHTTPS2.h,v 1.1 2001/11/20 14:14:23 tujikawa Exp $

#ifndef _RETRIEVEHTTPS2_H_
#define _RETRIEVEHTTPS2_H_

#include "RetrieveHTTP2.h"

class RetrieveHTTPS2 : public RetrieveHTTP2
{
public:
    RetrieveHTTPS2(ItemCellPartial *itemcellPartial);
    ~RetrieveHTTPS2();
#ifdef HAVE_OPENSSL
    void establishConnection(Socket &sock);
#endif // HAVE_OPENSSL
};
#endif // _RETRIEVEHTTPS2_H_
