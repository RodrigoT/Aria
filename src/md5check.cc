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

// $Id: md5check.cc,v 1.5 2001/10/20 08:56:18 tujikawa Exp $

#include "md5check.h"

// calculates md5 checksum of the file and returns hex string
string md5_check(const string& filename)
{
  MD5_CONTEXT md5ctx;

  md5_init(&md5ctx);

  unsigned char buf[8092];
  ifstream infile(filename.c_str(), ios::in);
  if(infile.bad()) {
    throw MD5_IOERR;
  }
  while(!infile.eof()) {
    infile.read((char *)buf, sizeof(buf));
    md5_write(&md5ctx, buf, infile.gcount());
  }

  md5_final(&md5ctx);

  unsigned char digest[16];  
  memcpy(digest, md5ctx.buf, 16);

  char hexDigest[32];
  for(int i = 0; i < 16; ++i) {
    sprintf(hexDigest+i*2, "%02x", digest[i]);
  }

  return hexDigest;
}

string
md5CheckString(const string& srcString)
{
  MD5_CONTEXT md5ctx;

  md5_init(&md5ctx);

  md5_write(&md5ctx, (byte *)srcString.c_str(), srcString.size());

  md5_final(&md5ctx);

  unsigned char digest[16];  
  memcpy(digest, md5ctx.buf, 16);

  char hexDigest[32];
  for(int i = 0; i < 16; ++i) {
    sprintf(hexDigest+i*2, "%02x", digest[i]);
  }

  return hexDigest;
}

