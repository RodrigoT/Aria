//  Aria - yet another download tool
//  Copyright (C) 2000, 2002 Tatsuhiro Tsujikawa
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

// $Id: ServerTemplateList.h,v 1.9 2002/03/16 14:13:00 tujikawa Exp $

#ifndef _SERVERTEMPLATELIST_H_
#define _SERVERTEMPLATELIST_H_
#include <gtk/gtk.h>
#include "aria.h"
#include "StringHash.h"
#include "ServerTemplate.h"
#include "TagParse.h"

using namespace std;

#define GET "<get>"
#define REFERER "<referer>"
#define OPTION "<option>"
#define KEYLINK "<keylink>"
#define SESSION "<session>"
#define ESESSION "</session>"
#define SEQUENCE "<sequence>"
#define ESEQUENCE "</sequence>"
#define POST_OFFSET_SIZE "<post-offset-size>"
#define POST_OFFSET_STRING "<post-offset-string>"
#define IDENTIFIER "<identifier>"
#define TARGET_SERVERS "<target-servers>"
#define IGNORE_SERVERS "<ignore-servers>"
#define IGNORE_EXTENSIONS "<ignore-extensions>"
#define COMMENT "<comment>"
#define ECOMMENT "</comment>"
#define SVCOMMAND "<execute>"
#define ESVCOMMAND "</execute>"
#define SVCOMMANDSTAT "<execute-status>"
#define ESVCOMMANDSTAT "</execute-status>"
#define SERVER "<server>"
#define ESERVER "</server>"

class ServerTemplateList
{
private:
  vector<ServerTemplate> svt_list;
  pthread_mutex_t svt_list_lock;
public:
  ServerTemplateList();
  ~ServerTemplateList();

  void push_front(const ServerTemplate& svt_in);
  void set_valid(bool flag);
  bool Is_valid() const;

  const ServerTemplate& search(const string& server_name, const string& filename);
  vector<ServerTemplate>& ret_server_template_list();

  void set_valid_safely(vector<ServerTemplate>::iterator svt_itr, bool flag);

  bool find(const string& server_name);
  bool find_with_keylink(const string& server_name);

  bool Is_reserved(const string& tag);
  bool Is_reserved_in_session(const string& tag);
  bool Read_config_file(const string& filename);
  void update_validity(GList *selection);
  int length() const;
  enum ErrorType {
    EOPEN
  };
};
#endif //_SERVERTEMPLATELIST_H_
