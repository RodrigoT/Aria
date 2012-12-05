//
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

// $Id: Command.h,v 1.9 2002/04/03 13:33:51 tujikawa Exp $

#ifndef _COMMAND_H_
#define _COMMAND_H_
#include <iostream>
#include <string>
#include <list>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include "utils.h"
#include "URLcontainer.h"

using namespace std;

class Command
{
private:
    string name;
    string comment;
    list<string> command;
    list<string> extensions;
    bool ignore_ret_status; // true: ignore return status of program
    list<int> succ_status_list; // assume success

    bool valid;
    bool bad_flag;
public:
    Command(const string &name, const string &comment, const string &command_string, const string &succ_status_string, const string &ext_string);
    Command(const string &command_string, const string &succ_status_string);
    Command();

    ~Command();

    bool bad() const;

    const list<string> &ret_command() const;
    bool Is_in_extensions(const string &fliename) const;
    bool Is_in_succ_status_list(int status) const;
    bool Is_ignore_return_status() const;
    const string &ret_command_name() const;
    const string &ret_command_comment() const;
    const list<int> &ret_succ_status_list() const;

    void Process_extension_string(string ext_string);
    void Process_command_string(string command_string);
    void Process_command_succ_stat_string(string stat_string);

    string MyToken_splitter(string &line);
    bool Is_reserved(const string &token);
    static int interpret(const string &word);
    bool Is_valid() const;
    void set_valid(bool flag);
    string Create_commandline(const string &filename, const string &save_dir, const URLcontainer &urlcon) const;
    int Execute_commandline(const string &filename, const string &save_dir, const URLcontainer &urlcon) const;
    enum {
        VAR_FILENAME = 0,
        VAR_DIR = 1,
        VAR_FILEPATH = 2,
        VAR_URL = 3,
        UNRESERVED = 99
    };
};
#endif //_COMMAND_H_
