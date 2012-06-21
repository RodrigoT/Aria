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

// $Id: ItemLogCell.h,v 1.8 2001/04/12 14:00:14 tujikawa Exp $

//definition of class ItemLogCell

#ifndef _ITEMLOGCELL_H_
#define _ITEMLOGCELL_H_

#include <string>
#include <stdlib.h>
#include <time.h>
#include "aria.h"
#include "utils.h"
using namespace std;

enum MessageType {
  MSG_DOWNLOAD_STATUS,
  MSG_DOWNLOAD_SUCCESS,
  MSG_DOWNLOAD_PARTIAL_SUCCESS,
  MSG_DOWNLOAD_ABORTED,
  MSG_DOWNLOAD_ERROR,
  MSG_DOWNLOAD_INFO,
  MSG_DOWNLOAD_SEND,
  MSG_DOWNLOAD_RECV,
  MSG_DOWNLOAD_ADD_PARTIAL,
  MSG_SYS_REQ,
  MSG_SYS_INFO,
  MSG_SYS_ERROR,
  MSG_SYS_SUCCESS,
  MSG_INIT,
  MSG_GET_NEXT_ITEM
};

class ItemLogCell {
private:
  string log;
  MessageType logtype;
public:
  ItemLogCell(const string& log, MessageType reporttype);
  ItemLogCell();
  ~ItemLogCell();
  MessageType ret_Logtype() const;
  const string& ret_Log() const;
};

#endif // _ITEMLOGCELL_H_
