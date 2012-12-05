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

// $Id: Command.cc,v 1.9 2002/04/03 13:33:51 tujikawa Exp $

#include "Command.h"
extern char **environ;

Command::Command(const string &name_in, const string &comment_in, const string &command_string, const string &succ_status_string, const string &ext_string)
{
    try {
        name = name_in;
        comment = comment_in;
        ignore_ret_status = false;

        bad_flag = false;
        Process_command_string(command_string);
        Process_command_succ_stat_string(succ_status_string);
        Process_extension_string(ext_string);
        valid = false;//true;
    } catch (int err) {
        cerr << "error occurred in <command> tag" << endl;
        bad_flag = true;
    }
}

Command::Command(const string &command_string, const string &succ_status_string)
{
    try {
        bad_flag = false;
        ignore_ret_status = false;

        Process_command_string(command_string);
        Process_command_succ_stat_string(succ_status_string);
        valid = false;//true;
    } catch (int err) {
        //cerr << "error occurred in <command> tag" << endl;
        bad_flag = true;
    }
}

Command::Command()
{
    valid = false;
}

Command::~Command()
{
}

bool Command::bad() const
{
    return bad_flag;
}

bool Command::Is_valid() const
{
    return valid;
}

void Command::set_valid(bool flag)
{
    valid = flag;
}

bool Command::Is_in_extensions(const string &filename) const
{
    for (list<string>::const_iterator itr = extensions.begin(); itr != extensions.end(); ++itr) {
        if (itr->size() < filename.size()) {
            if (filename.substr(filename.size() - itr->size()) == *itr) return true;
        }
    }
    return false;
}

const list<string> &Command::ret_command() const
{
    return command;
}

const string &Command::ret_command_name() const
{
    return name;
}

const string &Command::ret_command_comment() const
{
    return comment;
}

void Command::Process_extension_string(string ext_string)
{
    while (ext_string.size()) {
        string token = Token_splitter(ext_string);
        if (token.size()) {
            extensions.push_back(token);
        }
    }
    if (extensions.empty()) {
        cerr << "error: no target extension specified" << endl;
        throw 1;
    }
}

void Command::Process_command_string(string command_string)
{
    while (command_string.size()) {
        string token = MyToken_splitter(command_string);
        if (token.size()) {
            if (token.at(0) == '$' && !Is_reserved(token)) throw 0;
            command.push_back(token);
        }
    }
    if (command.empty()) {
        //cerr << "error: no command specified" << endl;
        throw 1;
    }
}

const list<int> &Command::ret_succ_status_list() const
{
    return succ_status_list;
}

void Command::Process_command_succ_stat_string(string stat_string)
{
    if (stat_string.empty()) {
        ignore_ret_status = true;
        return;
    }
    while (stat_string.size()) {
        string token = Token_splitter(stat_string, " ,\t");
        if (token == "ign") {
            ignore_ret_status = true;
            break;
        } else {
            succ_status_list.push_back(stoi(token));
        }
    }
}

bool Command::Is_in_succ_status_list(int status) const
{
    for (list<int>::const_iterator itr = succ_status_list.begin();
            itr != succ_status_list.end(); ++itr) {
        if (*itr == status) return true;
    }
    return false;
}

bool Command::Is_ignore_return_status() const
{
    return ignore_ret_status;
}

string Command::MyToken_splitter(string &line)
{
    unsigned int start_pos = line.find_first_not_of(" \t\n\r");

    if (start_pos == string::npos) {
        line.erase();
        return "";
    }

    string token;
    if (start_pos > 0) {
        token = line.substr(0, start_pos);
        line.erase(0, start_pos);
        return(token);
    }

    if (line.at(start_pos) == '$') { // reserved word
        // $(****)
        unsigned int end_pos = line.find(')', start_pos);
        if (end_pos == string::npos) {
            cerr << "error: paren mismatch: '" << line.substr(start_pos) << "'" << endl;
            throw 0;
        }
        token = line.substr(start_pos, end_pos - start_pos + 1);

        line.erase(0, end_pos + 1);
    } else { // unreserved word
        unsigned int end_pos = line.find_first_of("$", start_pos);
        if (end_pos == string::npos) {
            end_pos = line.size();
        }
        token = line.substr(start_pos, end_pos - start_pos);
        line.erase(0, end_pos);
    }

    return token;
}

bool Command::Is_reserved(const string &token)
{
    if (token == "$(filename)"
            || token == "$(filepath)"
            || token == "$(url)"
            || token == "$(dir)") {
        return true;
    } else {
        return false;
    }
}

int Command::interpret(const string &word)
{
    if (word == "$(filename)") {
        return VAR_FILENAME;
    } else if (word == "$(dir)") {
        return VAR_DIR;
    } else if (word == "$(filepath)") {
        return VAR_FILEPATH;
    } else if (word == "$(url)") {
        return VAR_URL;
    } else {
        return UNRESERVED;
    }
}

string Command::Create_commandline(const string &filename, const string &save_dir, const URLcontainer &urlcon) const
{
    string commandline = "cd " + save_dir + "; ";
    for (list<string>::const_iterator itr = command.begin(); itr != command.end();
            ++itr) {
        switch (Command::interpret(*itr)) {
            case Command::VAR_FILENAME:
                commandline += filename;//+" ";
                break;
            case Command::VAR_URL:
                commandline += urlcon.ret_URL();//+" ";
                break;
            case Command::VAR_FILEPATH:
                commandline += save_dir + filename; //+" ";
                break;
            case Command::VAR_DIR:
                commandline += save_dir;//+" ";
                break;
            case Command::UNRESERVED:
            default:
                commandline += *itr;//+ " ";
                break;
        }
    }
    return commandline;
}

int Command::Execute_commandline(const string &filename, const string &save_dir, const URLcontainer &urlcon) const
{
    int status;
    int pid = fork();
    if (pid == -1)
        return -1;
    if (pid == 0) {
        const char *argv[4];
        argv[0] = "sh";
        argv[1] = "-c";
        argv[2] = Create_commandline(filename, save_dir, urlcon).c_str();
        argv[3] = 0;
        execve("/bin/sh", (char **)argv, environ);
        exit(127);
    } else if (pid < 0) {
        return -1;
        //itemcell->Send_message_to_gui(_("cannot execute: ")+commandline, MSG_DOWNLOAD_ERROR);
    }
    while (1) {
        if (waitpid(pid, &status, 0) == -1) {
            if (errno != EINTR)
                return -1;
        } else {
            return status;
        }
    }
}
