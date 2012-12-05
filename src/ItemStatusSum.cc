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

// $Id: ItemStatusSum.cc,v 1.1 2001/04/24 16:08:19 tujikawa Exp $

#include "ItemStatusSum.h"

extern void Set_suminfo_label();

ItemStatusSum::ItemStatusSum(ItemCell *itemcell_in) : ItemStatus(itemcell_in)
{
    /*
    n_download = 0;
    n_stop = 0;
    n_ready = 0;
    n_split = 0;
    n_locked = 0;
    n_complete = 0;
    */
}

ItemStatusSum::~ItemStatusSum()
{
}

void ItemStatusSum::Update()
{
    /*
    suminfo.set_diff_download(n_download);
    suminfo.set_diff_stop(n_stop);
    suminfo.set_diff_ready(n_ready);
    suminfo.set_diff_split(n_split);
    suminfo.set_diff_locked(n_locked);
    suminfo.set_diff_complete(n_complete);
    */
    //suminfo.print_sum();
    Set_suminfo_label();
}
/*
void ItemStatusSum::inc_download()
{
  ++n_download;
}

void ItemStatusSum::dec_download()
{
  --n_download;
}

void ItemStatusSum::inc_stop()
{
  ++n_stop;
}

void ItemStatusSum::dec_stop()
{
  --n_stop;
}

void ItemStatusSum::inc_ready()
{
  ++n_ready;
}

void ItemStatusSum::dec_ready()
{
  --n_ready;
}

void ItemStatusSum::inc_split()
{
  ++n_split;
}

void ItemStatusSum::dec_split()
{
  --n_split;
}

void ItemStatusSum::inc_locked()
{
  ++n_locked;
}

void ItemStatusSum::dec_locked()
{
  --n_locked;
}

void ItemStatusSum::inc_complete()
{
  ++n_complete;
}

void ItemStatusSum::dec_complete()
{
  --n_complete;
}
*/
