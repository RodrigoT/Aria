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
//  $Id: CtrlSocket.h,v 1.8 2001/11/04 10:18:07 tujikawa Exp $

#ifndef _CTRLSOCKET_H_
#define _CTRLSOCKET_H_
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <errno.h>
#include <sys/un.h>
#include <stdio.h>

using namespace std;

class CmdPacketHeader
{
public:
    enum CommandType {
        CMD_PING,
        CMD_URL,
        CMD_URL2PW,
        CMD_URLLIST,
        CMD_CRCLIST,
        CMD_START,
        CMD_STOP,
        CMD_AGAIN,
        CMD_LISTNAME,
        CMD_PREVLIST,
        CMD_NEXTLIST,
        CMD_DELLIST,
        CMD_DELCOMPLETE,
        CMD_QUIT,
        CMD_LSCOMP,
        CMD_LSREADY,
        CMD_LSSTOP,
        CMD_LSGO,
        CMD_LSFAILED,
        CMD_LSLOCK,
        CMD_LSALL,
        CMD_DATA
    };
private:
    CommandType command;
    int length;
    int option;
public:
    CmdPacketHeader(CommandType cmd, int length = 0);
    CmdPacketHeader();
    int get_length() const;
    CommandType get_command() const;
};

class CtrlSocket
{
private:
    bool remote_running;
    int ctrl_fd;
    string socket_name;
public:
    CtrlSocket();
    ~CtrlSocket();

    enum ListingType {
        CTSOCK_LSCOMP = 1,
        CTSOCK_LSREADY = 1 << 1,
        CTSOCK_LSSTOP = 1 << 2,
        CTSOCK_LSGO = 1 << 3,
        CTSOCK_LSFAILED = 1 << 4,
        CTSOCK_LSLOCK = 1 << 5,
        CTSOCK_LSALL = CTSOCK_LSCOMP | CTSOCK_LSREADY | CTSOCK_LSSTOP | CTSOCK_LSGO | CTSOCK_LSFAILED | CTSOCK_LSLOCK
    };

    bool Is_remote_running();
    int Send_command(int fd, CmdPacketHeader::CommandType command, void *data = NULL, int length = 0);
    int Send_command(CmdPacketHeader::CommandType cmd, void *data = NULL, int length = 0);
    bool Send_ack(int fd);
    int Recv_command(int fd, void **data_ptr);

    bool Recv_ack();
    bool Is_remote_running_internal();
    int connect_to();
    int Select2();
    void Read_message();
    int ret_ctrl_fd() const;
};
#endif // _CTRLSOCKET_H_
