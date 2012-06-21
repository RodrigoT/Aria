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

// $Id: ItemStatusDynamic.h,v 1.8 2001/06/03 07:25:51 tujikawa Exp $

#ifndef _ITEMSTATUSDYNAMIC_H_
#define _ITEMSTATUSDYNAMIC_H_
#include "aria.h"
#include "ItemStatus.h"
#include "AppOption.h"

// 各スレッドがGUIスレッドにファイルのダウンロード状況を知らせるときに
// 送るデータのフォーマット
class ItemStatusDynamic : public ItemStatus {
private:
  ItemCell::ItemStatusType status;
  unsigned int retry_count;
  unsigned int size_current;
  unsigned int size_total;
  float speed;
  float avgSpeed;
  bool delete_flag;
  unsigned int update_flag;
public:
  ItemStatusDynamic(ItemCell *itemcell, ItemCell::ItemStatusType status, unsigned int retry_count, unsigned int size_current, unsigned int size_total, float speed, float avgSpeed);
  ItemStatusDynamic(ItemCell *itemcell, ItemCell::ItemStatusType status, unsigned int retry_count, unsigned int size_current, unsigned int size_total);

  enum {
    name_col = 1,
    progress_col = 1 << 1,
    crc_col = 1 << 2,
    speed_col = 1 << 3,
    static_col = 1 << 4,
    all_col = name_col | progress_col | crc_col | speed_col | static_col
  };
  ~ItemStatusDynamic();
  void Update();
  void set_UpdateFlag(unsigned int flag);
  void set_DeleteFlag(bool flag);
};

#endif // _ITEMSTATUSDYNAMIC_H_
