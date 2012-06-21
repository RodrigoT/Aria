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

// $Id: RetrieveHTTP.h,v 1.6 2002/02/13 12:09:24 tujikawa Exp $

#ifndef _RETRIEVEHTTP_H_
#define _RETRIEVEHTTP_H_
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <fstream>
#include <algorithm>
#include <vector>
#include "aria.h"
#ifdef HAVE_ZLIB
#include <zlib.h>
#endif // HAVE_ZLIB
#include "Base64.h"
#include "md5check.h"
#include "ItemCell.h"
#include "ItemList.h"
#include "UseragentList.h"
#include "HTTPcontainer.h"
#include "CookieList.h"
#include "ServerTemplateList.h"
#include "Retrieve.h"
using namespace std;

class RetrieveHTTP : public Retrieve
{
private:
  bool compressedFlag;
  bool chunkedFlag;
public:
  RetrieveHTTP(ItemCell *itemcell);
  ~RetrieveHTTP();

  ItemCell::DownloadStatusType Download_Main();
protected:
  // download mode
  enum DownloadMode {
    NORMAL_MODE,
    EMBEDED_URL_MODE,
    ADD_HREF_MODE
  };

  // ファイルの格納
  void Start_Download(const Socket& socket, unsigned int startingbyte);

  // download chunked encoding data
  int Download_data_chunked(ofstream& outfile, const Socket& socket);

  void validateHTTPStatus(int httpStatus);

  // GETなどの送信
  void Send_Request(const Socket& socket, unsigned int startingbyte);
  // ヘッダーを取得
  void Get_HTTP_header(const Socket& socket, HTTPHeaderList& http_header_list);
  
  string create_url(const vector<string>& string_vector);
#ifdef HAVE_ZLIB
  void uncompressFile(const string& filename);
#endif // HAVE_ZLIB
  //ItemCell::DownloadStatusType Post_process();
  //bool Execute_program();
  virtual void establishConnection(Socket& sock);
};
#endif // _RETRIEVEHTTP_H_
