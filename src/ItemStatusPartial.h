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

// $Id: ItemStatusPartial.h,v 1.3 2001/03/09 06:49:10 tujikawa Exp $

#ifndef _ITEMSTATUSPARTIAL_H_
#define _ITEMSTATUSPARTIAL_H_
#include "aria.h"
#include "ItemStatus.h"

// 各スレッドがGUIスレッドにファイルのダウンロード状況を知らせるときに
// 送るデータのフォーマット
class ItemStatusPartial : public ItemStatus {
private:
  ItemCell *itemcell_partial;
public:
  ItemStatusPartial(ItemCell *itemcell, ItemCell *itemcell_partial);
  ~ItemStatusPartial();
  void Update();
};

#endif // _ITEMSTATUSPARTIAL_H_
