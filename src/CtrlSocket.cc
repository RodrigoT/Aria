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
// $Id: CtrlSocket.cc,v 1.17 2002/10/01 15:32:00 tujikawa Exp $

#include "CtrlSocket.h"
#include "ListEntry.h"
extern void Open_url_file(const string &filename);
extern bool Create_new_item(string url, bool onPaste = false, const string &dir = "", const string &referer = "");
extern void Open_crc_file(const string &filename);
extern gboolean Download_start_all(GtkWidget *w, gpointer data);
extern gboolean Download_stop_all(GtkWidget *w, gpointer data);
//extern gboolean Download_start_all_list(GtkWidget* w, gpointer data);
//extern gboolean Download_stop_all_list(GtkWidget* w, gpointer data);
extern gboolean File_quit(GtkWidget *w, GtkWidget *unused);
extern void Send_ls_item(int fd, int ls_flag);
extern void Item_clear_state_item(int state_flag);
extern gboolean Download_download_again_all(GtkWidget *w, gpointer data);
//extern gboolean Download_download_again_all_list(GtkWidget* w, gpointer data);
extern void Show_remote_current_listname(int fd, ListEntry *listentry = NULL);
extern gboolean List_previous_list(GtkWidget *w, gpointer data);
extern gboolean List_next_list(GtkWidget *w, gpointer data);
extern void List_delete_list();

CmdPacketHeader::CmdPacketHeader(CommandType cmd, int len)
{
    command = cmd;
    length = len;
}

CmdPacketHeader::CmdPacketHeader()
{
    command = CMD_PING;
    length = 0;
}

int CmdPacketHeader::get_length() const
{
    return length;
}

CmdPacketHeader::CommandType CmdPacketHeader::get_command() const
{
    return command;
}

CtrlSocket::CtrlSocket()
{
    struct sockaddr_un saddr;
    saddr.sun_family = AF_UNIX;
    int i = 0;
    g_snprintf(saddr.sun_path, 108, "%s/aria_%s.%d",
               g_get_tmp_dir(), g_get_user_name(), i);

    ctrl_fd = -1;
    if (Is_remote_running_internal()) {
        remote_running = true;
        // connect to remote instance
    } else {
        remote_running = false;
        if ((ctrl_fd = socket(AF_UNIX, SOCK_STREAM, 0)) != -1) {
            // bind
            if ((unlink(saddr.sun_path) == -1) && errno != ENOENT) {
                cerr << "can't unlink" << endl;
                exit(1);
            }
            if (bind(ctrl_fd, (struct sockaddr *) &saddr, sizeof (saddr)) != -1) {
                listen(ctrl_fd, 100);
                socket_name = saddr.sun_path;
            } else {
                cerr << "can't bind" << endl;
                exit(1);
            }
        } else {
            cerr << "can't create socket" << endl;
            exit(1);
        }
    }
}

CtrlSocket::~CtrlSocket()
{
    if (ctrl_fd < 0) {
        close(ctrl_fd);
    }
    if (socket_name.size())
        unlink(socket_name.c_str());
}

int CtrlSocket::ret_ctrl_fd() const
{
    return ctrl_fd;
}

bool CtrlSocket::Is_remote_running_internal()
{
    int fd;
    if ((fd = connect_to()) == -1)
        return false;
    Send_command(fd, CmdPacketHeader::CMD_PING);
    close(fd);

    return true;
}

int CtrlSocket::Send_command(int fd, CmdPacketHeader::CommandType cmd, void *data, int length)
{
    try {
        CmdPacketHeader cmdpkthdr(cmd, length);
        if (write(fd, &cmdpkthdr, sizeof(CmdPacketHeader)) < 0) throw 0;
        if (cmdpkthdr.get_length() > 0) {
            if (write(fd, data, cmdpkthdr.get_length()) < 0) throw 0;
        }
        return 1;
    } catch (int err) {
        return -1;
    }
}

int CtrlSocket::Send_command(CmdPacketHeader::CommandType cmd, void *data, int length)
{
    int fd;
    try {
        if ((fd = connect_to()) == -1) throw 0;

        CmdPacketHeader cmdpkthdr(cmd, length);

        if (write(fd, &cmdpkthdr, sizeof(CmdPacketHeader)) < 0) throw 0;
        if (cmdpkthdr.get_length() > 0) {
            if (write(fd, data, cmdpkthdr.get_length()) < 0) throw 0;
        }
        close(fd);
        return 1;
    } catch (int err) {
        return -1;
    }
}

int CtrlSocket::Recv_command(int fd, void **data_ptr)
{
    try {
        CmdPacketHeader cmdpkthdr;
        //*data_ptr = NULL;
        int size;
        if ((size = read(fd, &cmdpkthdr, sizeof(CmdPacketHeader))) < 0) throw 0;
        if (size == 0) return 0; // EOF
        if (cmdpkthdr.get_length()) {
            *data_ptr = new char[cmdpkthdr.get_length()];
            if (read(fd, *data_ptr, cmdpkthdr.get_length()) < 0) throw 0;
        }
        return 1;
    } catch (int err) {
        return -1;
    }
}

// return file descriptor
int CtrlSocket::connect_to()
{
    int fd;

    uid_t stored_uid, euid;
    struct sockaddr_un saddr;
    int session = 0;

    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) != -1) {
        saddr.sun_family = AF_UNIX;
        stored_uid = getuid();
        euid = geteuid();
        setuid(euid);
        sprintf(saddr.sun_path, "%s/aria_%s.%d", g_get_tmp_dir(), g_get_user_name(), session);
        setreuid(stored_uid, euid);
        if (connect(fd, (struct sockaddr *) &saddr, sizeof (saddr)) != -1)
            return fd;
    }
    close(fd);
    return -1;
}

bool CtrlSocket::Is_remote_running()
{
    return remote_running;
}

int CtrlSocket::Select2()
{
    while (1) {
        int len;
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(ctrl_fd, &fds);
        struct sockaddr_un saddr;
        struct timeval tv;
        int fd;
        tv.tv_sec = 0;
        tv.tv_usec = 100000;
        len = sizeof (saddr);
        if ((select(ctrl_fd + 1, &fds, NULL, NULL, NULL) <= 0) ||
                ((fd = accept(ctrl_fd, (struct sockaddr *) &saddr, (socklen_t *)&len)) == -1)) {
            continue;
        }
        CmdPacketHeader cmdpkthdr;
        char *data = NULL;
        read(fd, &cmdpkthdr, sizeof(CmdPacketHeader));
        //cerr << cmdpkthdr.get_length() << endl;
        if (cmdpkthdr.get_length()) {
            data = new char[cmdpkthdr.get_length()];
            read(fd, data, cmdpkthdr.get_length());
            //cerr << data << endl;
            //cerr << strlen(data) << endl;
        }

        if (data) delete [] data;
        close(fd);
    }
    return 0;
}

void CtrlSocket::Read_message()
{
    int fd = -1;

    try {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(ctrl_fd, &fds);
        struct sockaddr_un saddr;
        int len;
        struct timeval tv;
        int fd;
        tv.tv_sec = 0;
        tv.tv_usec = 100000;
        len = sizeof (saddr);
        if ((fd = accept(ctrl_fd, (struct sockaddr *) &saddr, (socklen_t *)&len)) == -1) {
            throw 1;
        }
        CmdPacketHeader cmdpkthdr;
        char *data = NULL;
        if (read(fd, &cmdpkthdr, sizeof(CmdPacketHeader)) < 0) throw 0;
        if (cmdpkthdr.get_length()) {
            data = new char[cmdpkthdr.get_length()];
            if (read(fd, data, cmdpkthdr.get_length()) < 0) throw 0;
        }
        bool passPasteWindowFlag = false;
        switch (cmdpkthdr.get_command()) {
            case CmdPacketHeader::CMD_URL2PW:
                passPasteWindowFlag = true;
            case CmdPacketHeader::CMD_URL: {
                int size;
                string dir;
                string ref;
                if ((size = read(fd, &cmdpkthdr, sizeof(CmdPacketHeader))) < 0) throw 0;
                if (cmdpkthdr.get_length()) {
                    char *dir_data;
                    dir_data = new char[cmdpkthdr.get_length()];
                    if (read(fd, dir_data, cmdpkthdr.get_length()) < 0) throw 0;
                    dir = dir_data;
                    delete [] dir_data;

                }
                if ((size = read(fd, &cmdpkthdr, sizeof(CmdPacketHeader))) < 0) throw 0;
                if (cmdpkthdr.get_length()) {
                    char *ref_data;
                    ref_data = new char[cmdpkthdr.get_length()];
                    if (read(fd, ref_data, cmdpkthdr.get_length()) < 0) throw 0;
                    ref = ref_data;
                    delete [] ref_data;
                }

                Create_new_item(data, passPasteWindowFlag, dir, ref);
                break;
            }
            case CmdPacketHeader::CMD_URLLIST:
                Open_url_file(data);
                break;
            case CmdPacketHeader::CMD_CRCLIST:
                Open_crc_file(data);
                break;
            case CmdPacketHeader::CMD_START:
                Download_start_all(NULL, NULL);
                break;
            case CmdPacketHeader::CMD_STOP:
                Download_stop_all(NULL, NULL);
                break;
            case CmdPacketHeader::CMD_AGAIN:
                Download_download_again_all(NULL, NULL);
                break;
            case CmdPacketHeader::CMD_DELCOMPLETE:
                Item_clear_state_item(1 << 3);
                break;
            case CmdPacketHeader::CMD_QUIT:
                File_quit(NULL, NULL);
                break;
            case CmdPacketHeader::CMD_LISTNAME:
                Show_remote_current_listname(fd);
                break;
            case CmdPacketHeader::CMD_PREVLIST:
                List_previous_list(NULL, NULL);
                break;
            case CmdPacketHeader::CMD_NEXTLIST:
                List_next_list(NULL, NULL);
                break;
            case CmdPacketHeader::CMD_LSCOMP: {
                int ls_flag = CTSOCK_LSCOMP;
                Send_ls_item(fd, ls_flag);
                break;
            }
            case CmdPacketHeader::CMD_LSREADY: {
                int ls_flag = CTSOCK_LSREADY;
                Send_ls_item(fd, ls_flag);
                break;
            }
            case CmdPacketHeader::CMD_LSFAILED: {
                int ls_flag = CTSOCK_LSFAILED;
                Send_ls_item(fd, ls_flag);
                break;
            }
            case CmdPacketHeader::CMD_LSSTOP: {
                int ls_flag = CTSOCK_LSSTOP;
                Send_ls_item(fd, ls_flag);
                break;
            }
            case CmdPacketHeader::CMD_LSGO: {
                int ls_flag = CTSOCK_LSGO;
                Send_ls_item(fd, ls_flag);
                break;
            }
            case CmdPacketHeader::CMD_LSLOCK: {
                int ls_flag = CTSOCK_LSLOCK;
                Send_ls_item(fd, ls_flag);
                break;
            }
            case CmdPacketHeader::CMD_LSALL: {
                int ls_flag = CTSOCK_LSALL;
                Send_ls_item(fd, ls_flag);
                break;
            }

            default:
                break;
        }
        if (data) delete [] data;
        close(fd);
    } catch (int err) {
        switch (err) {
            case 0:
                close(fd);
                break;
            case 1:
            default:
                break;
        }
    }
}
