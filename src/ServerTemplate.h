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

// $Id: ServerTemplate.h,v 1.6 2002/03/16 14:13:00 tujikawa Exp $

#ifndef _SERVERTEMPLATE_H_
#define _SERVERTEMPLATE_H_
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <list>
#include "utils.h"
#include "Session.h"

using namespace std;

class ServerTemplate
{
private:
    string name;
    string comment;
    list<string> server_name_list;
    vector<Session> session_vector;
    list<string> ignore_server_name_list;
    list<string> ignore_extension_list;
    int port;
    bool valid;
    bool ignorefileerr;
    bool bad_flag;
public:
    ServerTemplate(const string &name_in,
                   const string &comment_in,
                   const list<string> &server_name_list,
                   const list<string> &ignore_server_name_list,
                   const list<string> &ignore_extension_list,
                   const string &option_in,
                   const vector<Session> &session_in);
    ServerTemplate();

    Session &ret_session(std::size_t session_count);
//  const vector<Session>& ret_session_vector() const;
    int ret_total_session() const;

    bool bad() const;
    void Process_hostname(string hostname);
    void Process_option_string(string option_string);
    void Process_session_vector(vector<Session> &session_vector);

    string MyToken_splitter(string &line);

    const list<string> &ret_server_name_list() const;
    const list<string> &ret_ignore_server_name_list() const;
    bool Is_in_entry(const string &server_name, const string &filename) const;
    const string &ret_template_name() const;
    const string &ret_template_comment() const;
    const string &ret_host() const;
    int ret_port() const;
    void set_valid(bool flag);
    bool Is_valid() const;
    bool Is_valid(std::size_t session_count) const;
};
#endif //_SERVERTEMPLATE_H_
