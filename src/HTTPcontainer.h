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

// $Id: HTTPcontainer.h,v 1.12 2001/10/17 13:06:34 tujikawa Exp $

//definition of class HTTPcontainer
#ifndef _HTTPCONTAINER_H_
#define _HTTPCONTAINER_H_
#include <string>
#include <list>
#include <stdlib.h>
#include "aria.h"
#include "utils.h"
#include "HTTP_Header.h"
#include "Base64.h"
using namespace std;

typedef list<HTTP_Header> HTTPHeaderList;

enum HTTPconErrorType {
  HTTPCON_EUNSUPP
};

class HTTPcontainer {
private:
  int status;
  string etag;
  string md5str;
  unsigned int contentlength;
  string contenttype;
  string location;
  list<string> cookie_list;
  string contentLocation;
  bool keepAliveFlag;
  bool compressedFlag;
  string transferEncoding;
public:
  friend class ItemCell_HTTP;

  HTTPcontainer();
  ~HTTPcontainer();
  int ret_HTTP_Status() const;
  unsigned int ret_ContentLength() const;
  const string& ret_MD5() const;
  const string& ret_Location() const;
  const list<string>& ret_Cookie_list() const;
  const string& ret_contentLocation() const;
  void set_ContentLength(const string& clength_str);
  void set_ETag(const string& etag);
  void set_MD5(const string& md5Crypted);
  void set_ContentType(const string& contenttype);
  void set_HTTP_Status(const string& status_str);
  void set_Location(const string& location);
  void add_Cookie_string(const string& cookie_string);
  void set_contentLocation(const string& content_string);
  void setKeepAliveEnabled(bool toggle);
  bool isKeepAliveEnabled();
  bool isCompressEnabled();
  void setCompressEnabled(bool toggle);
  void setTransferEncoding(const string& trasEnc);
  const string& getTransferEncoding();

  int Parse_HTTP_header(const HTTPHeaderList& http_header_list);
};

#endif // _HTTPCONTAINER_H_
