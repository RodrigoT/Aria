//
//  Aria - yet another download tool
//  Copyright (C) 2000, 2001, 2002 Tatsuhiro Tsujikawa
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

// $Id: RetrieveHTTP2.h,v 1.3 2002/02/13 12:09:24 tujikawa Exp $

#ifndef _RETRIEVEHTTP2_H_
#define _RETRIEVEHTTP2_H_
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <fstream>
#include "aria.h"
#include "utils.h"
#include "RetrieveHTTP.h"
#include "ItemCellPartial.h"
using namespace std;

class RetrieveHTTP2 : public RetrieveHTTP {
protected:
  ItemCellPartial *itemcell;
public:
  RetrieveHTTP2(ItemCellPartial *itemcell);
  ~RetrieveHTTP2();

  ItemCell::DownloadStatusType Download_Main();
protected:
  void Start_Download(const Socket& socket, unsigned int startingbyte);
  //ItemCell::DownloadStatusType Post_process();
};
#endif // _RETRIEVEHTTP2_H_
