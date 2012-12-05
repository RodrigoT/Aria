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

// $Id: HTMLparse.h,v 1.5 2002/04/03 13:33:51 tujikawa Exp $
#ifndef _HTMLPARSE_H_
#define _HTMLPARSE_H_

#include <string>
#include <fstream>
#include <iostream>
#include <list>
#include <stdio.h>
//#include "aria.h"
//#include "URLcontainer.h"
//#include "utils.h"
//#include "Options.h"

using namespace std;


enum HTMLparseExceptionType {
    HTMLPARSE_NOHREF,
    HTMLPARSE_EIO,
    HTMLPARSE_EOF,
};

class URLcontainer;
class Options;

class HTMLparse
{
private:
    string base_url;
    string root_url;
    string documentroot_dir;
    string prefix;
    string outfilename;
    string infilename;
    ofstream outfile;
    ifstream infile;
    bool outfile_bad;
    bool infile_bad;
    string baseHref;

    URLcontainer find_href(string line, Options &options);
    URLcontainer find_css(string line, Options &options);
    string erase_protocol(string line, int length, int prot_pos);
public:
    HTMLparse(const string &base_url_in,
              const string &root_url_in,
              const string &documentroot_dir_in,
              const Options &static_options,
              const string &infilename);
    ~HTMLparse();
    URLcontainer get_href(Options &options);


    void Set_save_directory(const URLcontainer &urlcon, const string &href, Options &options);

    bool out_bad() const;
    bool in_bad() const;
};
#endif
