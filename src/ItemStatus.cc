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

// $Id: ItemStatus.cc,v 1.8 2001/04/08 10:49:42 tujikawa Exp $

#include "ItemStatus.h"

ItemStatus::ItemStatus(ItemCell *itemcell_in)
{
    itemcell = itemcell_in;
}

ItemStatus::~ItemStatus()
{
}

ItemCell *ItemStatus::ret_ItemCell()
{
    return itemcell;
}

void ItemStatus::Update()
{
}

void ItemStatus::set_Listentry(ListEntry *listentry_in)
{
    listentry = listentry_in;
}

