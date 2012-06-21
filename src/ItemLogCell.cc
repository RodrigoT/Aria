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

// $Id: ItemLogCell.cc,v 1.8 2001/03/09 06:49:10 tujikawa Exp $

// implementation of ItemLogCell
#include "ItemLogCell.h"

ItemLogCell::ItemLogCell(const string& log_in, MessageType log_type)
{
  time_t timep = time(NULL);
  struct tm* tm = localtime(&timep);// the return address of localtime() is statically allocated
  char* prefix;
  switch(log_type) {
  case MSG_DOWNLOAD_SEND:
    prefix = _("SEND: ");
    break;
  case MSG_DOWNLOAD_RECV:
    prefix = _("RECV: ");
    break;
  case MSG_DOWNLOAD_INFO:
    prefix = _("INFO: ");
    break;
  case MSG_DOWNLOAD_SUCCESS:
    prefix = _("SUCC: ");
    break;
  case MSG_DOWNLOAD_ERROR:
    prefix = _("ERR: ");
    break;
  default:
    prefix = " ";
    break;
  }

  log = "["+itos(tm->tm_hour, 2, '0')+":"+
    itos(tm->tm_min, 2, '0')+":"+
    itos(tm->tm_sec, 2, '0')+"]"+
    prefix+" "+log_in;
  if(log.at(log.size()-2) == '\r') {
    log.erase(log.size()-2);
    log += '\n';
  } else if(log.at(log.size()-1) != '\n') {
    log += '\n';
  }
  logtype = log_type;
}

ItemLogCell::ItemLogCell()
{
}

ItemLogCell::~ItemLogCell()
{
}

MessageType ItemLogCell::ret_Logtype() const
{
  return(logtype);
}

const string& ItemLogCell::ret_Log() const
{
  return(log);
}
