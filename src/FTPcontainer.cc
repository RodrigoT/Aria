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

// $Id: FTPcontainer.cc,v 1.9 2002/04/06 03:14:34 tujikawa Exp $

// implemantation of class FTPcontainer
#include "FTPcontainer.h"

//コンストラクタ
// 引数respはa,b,c,d,e,fのフォーマット
// ここからIPアドレスa.b.c.d, ポートe*256+fを構成し、
// server_addr, portにそれぞれ格納
FTPcontainer::FTPcontainer(const string &resp_string)
{
    int ip[4];
    int port_array[2];

    const char *resp = resp_string.c_str();

    port = 0;

    if (resp == NULL) return;
    unsigned int index = resp_string.find_first_of("0123456789");
    if (index != string::npos) {
        //cout << resp+i << "\n" << flush;
        sscanf(resp + index, "%d,%d,%d,%d,%d,%d", ip, ip + 1, ip + 2, ip + 3,
               port_array, port_array + 1);
        //cout << port_array[0] << "\n" << port_array[1] << "\n" << flush;
        server_addr = itos(ip[0]) + "." + itos(ip[1]) + "." + itos(ip[2]) + "." + itos(ip[3]);
        port = port_array[0] * 256 + port_array[1];
        //cout << port << "\n" << flush;
        //cout << server_addr << " " << port << flush;
    }
}

//デストラクタ
FTPcontainer::~FTPcontainer()
{
}

void FTPcontainer::set_Filesize(unsigned int size)
{
    filesize = size;
}

int FTPcontainer::ret_Port() const
{
    return port;
}

unsigned int FTPcontainer::ret_Filesize() const
{
    return filesize;
}

