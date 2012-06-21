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

// $Id: ItemStatus.h,v 1.7 2001/05/22 15:32:49 tujikawa Exp $

#ifndef _ITEMSTATUS_H_
#define _ITEMSTATUS_H_
#include "aria.h"
#include "ItemCell.h"
#include "ListManager.h"

// 各スレッドがGUIスレッドにファイルのダウンロード状況を知らせるときに
// 送るデータのフォーマット
class ItemStatus {
private:
  ItemCell *itemcell;
protected:
  ListEntry *listentry;
public:
  ItemStatus(ItemCell *itemcell);
  virtual ~ItemStatus();

  ItemCell *ret_ItemCell();
  virtual void Update();
  void set_Listentry(ListEntry *listentry);
};

#endif // _ITEMSTATUS_H_
