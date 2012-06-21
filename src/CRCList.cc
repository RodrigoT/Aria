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

// $Id: CRCList.cc,v 1.5 2001/03/09 06:49:09 tujikawa Exp $

#include "CRCList.h"

CRCList::CRCList(const string& file_in, const string& crc_in)
{
  filename = file_in;
  crc_string = crc_in;
};

CRCList::~CRCList()
{
};

const string& CRCList::ref_Filename() const
{
  return filename;
}

const string& CRCList::ref_CRC_string() const
{
  return crc_string;
}
