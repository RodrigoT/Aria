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

// $Id: initrc.cc,v 1.3 2001/10/12 16:47:39 tujikawa Exp $

#include <iostream>
#include <string>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdlib>
#include <string.h>
#include "aria.h"
#include "utils.h"

void createDir(const string &dirName, int permission)
{
    struct stat dirStat;
    if (stat(dirName.c_str(), &dirStat) < 0) {
        if (mkdir(dirName.c_str(), permission) < 0) {
            if (errno == EEXIST) {
                cerr << _("Non-directory file \"") << dirName << _("\" exists. Please rename this file in order to get Aria work correctly.") << '\n';
                exit(1);
            }
        }
    }
}

bool Initialize_resource_files(const string &rcdir, bool force_flag = false)
{
    struct stat dir_stat;

    if (stat(rcdir.c_str(), &dir_stat) < 0) {
        if (mkdir(rcdir.c_str(), 0700) < 0) {
            if (errno == EEXIST) {
                cerr << _("Non-directory file \"") << rcdir << _("\" exists. Please rename this file in order to get Aria work correctly.") << '\n';
                exit(1);
            }
        }
    }

    createDir(rcdir + "/statusIcon", 0700);
    createDir(rcdir + "/statusIcon/iconset", 0700);

    createDir(rcdir + "/basket", 0700);
    createDir(rcdir + "/basket/pixmap", 0700);

    string savedir = rcdir + "/save";
    if (stat(savedir.c_str(), &dir_stat) < 0) {
        if (mkdir(savedir.c_str(), 0700) < 0) {
            if (errno == EEXIST) {
                cerr << _("Non-directory file \"") << savedir << _("\" exists. Please rename this file in order to get Aria work correctly.") << '\n';
                exit(1);
            }
        }
    } else if (!force_flag) {
        return true;
    }

    string datadir = DATADIR;

    if (!copy_file(datadir + "/aria/rc.aria", rcdir + "/rc.aria") ||
            !copy_file(datadir + "/aria/command.aria", savedir + "/command.aria") ||
            !copy_file(datadir + "/aria/server.aria", savedir + "/server.aria") ||
            !copy_file(datadir + "/aria/useragent.aria", savedir + "/useragent.aria")) {
        cerr << _("Error occurred in resource file copy: ") << strerror(errno) << endl;
        return false;
    }

    return true;
}
