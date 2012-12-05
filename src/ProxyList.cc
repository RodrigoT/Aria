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

// $Id: ProxyList.cc,v 1.6 2001/11/04 10:18:08 tujikawa Exp $

#include "ProxyList.h"

ProxyList::ProxyList()
{
}

ProxyList::ProxyList(list<Proxyserver> initial_proxy_list)
{
    for (list<Proxyserver>::iterator itr = initial_proxy_list.begin(); itr != initial_proxy_list.end(); ++itr) {
        Proxyserver *entry = new Proxyserver(itr->ret_Server(), itr->ret_Port());
        proxy_list.push_back(entry);
    }
}

ProxyList::ProxyList(const ProxyList &proxylist_src)
{
    for (list<Proxyserver *>::const_iterator itr = proxylist_src.ret_list().begin(); itr != proxylist_src.ret_list().end(); ++itr) {
        Proxyserver *entry = new Proxyserver((*itr)->ret_Server(), (*itr)->ret_Port());
        proxy_list.push_back(entry);
    }
}

ProxyList::~ProxyList()
{
    clear();
}

bool ProxyList::search(const Proxyserver &entry)
{
    for (list<Proxyserver *>::iterator itr = proxy_list.begin(); itr != proxy_list.end(); ++itr) {
        if ((*itr)->ret_Server() == entry.ret_Server() && (*itr)->ret_Port() == entry.ret_Port()) {
            return true;
        }
    }
    return false;
}

void ProxyList::clear()
{
    for (list<Proxyserver *>::iterator itr = proxy_list.begin(); itr != proxy_list.end(); ++itr) {
        delete *itr;
    }
    proxy_list.clear();
}

bool ProxyList::add(const Proxyserver &new_entry)
{
    if (new_entry.ret_Server() == "" || search(new_entry)) return false;

    Proxyserver *entry = new Proxyserver(new_entry);
    proxy_list.push_back(entry);
    return true;
}

bool ProxyList::remove(const Proxyserver &entry)
{
    for (list<Proxyserver *>::iterator itr = proxy_list.begin(); itr != proxy_list.end(); ++itr) {
        if ((*itr)->ret_Server() == entry.ret_Server() && (*itr)->ret_Port() == entry.ret_Port()) {
            proxy_list.remove(*itr);
            delete *itr;
            return true;
        }
    }
    return false;
}

bool ProxyList::set(const ProxyList &new_list)
{
    clear();
    for (list<Proxyserver *>::const_iterator itr = new_list.ret_list().begin(); itr != new_list.ret_list().end(); ++itr) {
        Proxyserver *entry = new Proxyserver((*itr)->ret_Server(), (*itr)->ret_Port());
        proxy_list.push_back(entry);
    }
    return true;
}

const list<Proxyserver *> &ProxyList::ret_list() const
{
    return proxy_list;
}

bool ProxyList::Save_proxy_list(const string &file_proxy_list)
{
    string filename = file_proxy_list;
    string filenameTemp = file_proxy_list + ".temporary.working";

    ofstream outfile(filenameTemp.c_str(), ios::out);//, 0600);
    if (outfile.bad()) {
        cerr << "ERROR:Unable to save proxy list" << endl;
        return false;
    }
    //HTTP Proxy List
    for (list<Proxyserver *>::const_iterator itr = proxy_list.begin(); itr != proxy_list.end(); ++itr) {
        outfile << (*itr)->ret_Server() << ":" << (*itr)->ret_Port() << endl;
        if (outfile.bad()) return false;
    }
    if (outfile.fail()) return false;

    if (rename(filenameTemp.c_str(), filename.c_str()) < 0) {
        return false;
    }

    // change the attributes
    chmod(filename.c_str(), S_IRUSR | S_IWUSR);

    return true;
}

bool ProxyList::Read_proxy_list(const string &file_proxy_list)
{
    ifstream infile(file_proxy_list.c_str(), ios::in);//ios::skipws|ios::in);
    if (infile.bad() || infile.eof()) return false;

    while (infile.good()) {
        string line;
        getline(infile, line, '\n');
        if (infile.bad()) return false;
        line = Remove_white(line);
        if (line.empty() || line.at(0) == '#') continue;
        string server_name = Token_splitter(line, ": \t");
        string server_port = Token_splitter(line);
        if (server_port.empty()) server_port = "8080";
        Proxyserver entry(server_name, stoi(server_port, 10));
        add(entry);
    }
    return true;
}
