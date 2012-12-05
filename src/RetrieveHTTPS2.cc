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

// $Id: RetrieveHTTPS2.cc,v 1.1 2001/11/20 14:14:23 tujikawa Exp $

#include "RetrieveHTTPS2.h"

RetrieveHTTPS2::RetrieveHTTPS2(ItemCellPartial *itemcellPartial_in)
    : RetrieveHTTP2(itemcellPartial_in)
{
}

RetrieveHTTPS2::~RetrieveHTTPS2()
{
}

#ifdef HAVE_OPENSSL
void RetrieveHTTPS2::establishConnection(Socket &sock)
{
    RetrieveHTTP2::establishConnection(sock);
    if (sock.enableSSL() <= 0) {
        itemcell->Send_message_to_gui(_("Error while establishing SSL connection"), MSG_DOWNLOAD_ERROR);
        itemcell->set_Command(ItemCell::DLERRORSTOP);
        throw ItemCell::ITEM_EPROT;
    }
}
#endif // HAVE_OPENSSL
