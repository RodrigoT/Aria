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

// $Id: FTPcontainer.h,v 1.6 2001/03/09 06:49:09 tujikawa Exp $

#ifndef _FTPCONTAINER_H_
#define _FTPCONTAINER_H_
#include <string>
#include "aria.h"
#include "utils.h"
using namespace std;

class FTPcontainer {
private:
  string server_addr;
  int port;
  unsigned int filesize;
public:
  FTPcontainer(const string& resp);
  ~FTPcontainer();
  
  void set_Filesize(unsigned int size);
  int ret_Port() const;
  unsigned int ret_Filesize() const;
};

#endif // _FTPCONTAINER_H_
