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

// $Id: ItemStatusSum.h,v 1.1 2001/04/24 16:08:19 tujikawa Exp $

#ifndef _ITEMSTATUSSUM_H_
#define _ITEMSTATUSSUM_H_
#include "aria.h"
#include "ItemStatus.h"

// �ƥ���åɤ�GUI����åɤ˥ꥯ������
// ������Υե����ޥå�

// ľ�� SumInfo ���֥������Ȥ˥�������,
// �ܥ��֥������Ȥ� GUI �ι����ꥯ�����ȤΤߤ����Ȥ���
//   ��ͳ
//    case ʸ�� new �������Ȥ��ˤ���
//    ¿���Υ����ƥ�ι����ξ��, itemstatus ������ pipe �Хåե���
//    ���Ƥ��ޤ�
//    �ܥ��֥���������Ǥ⤦������ SumInfo ���Ф���������ɿ��ʤɤ�
//    �ѹ�����ԤäƤ������ټ�֤ȤʤäƤ���
class ItemStatusSum : public ItemStatus {
private:
  int n_download;
  int n_stop;
  int n_ready;
  int n_split;
  int n_locked;
  int n_complete;
  int n_error;
public:
  ItemStatusSum(ItemCell *itemcell);
  ~ItemStatusSum();
  void Update();

  void inc_download();
  void dec_download();

  void inc_stop();
  void dec_stop();

  void inc_ready();
  void dec_ready();

  void inc_split();
  void dec_split();

  void inc_locked();
  void dec_locked();

  void inc_complete();
  void dec_complete();

  void inc_error();
  void dec_error();
};

#endif // _ITEMSTATUSSUM_H_
