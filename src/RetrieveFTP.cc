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

// $Id: RetrieveFTP.cc,v 1.17 2002/10/01 15:32:00 tujikawa Exp $

#include "RetrieveFTP.h"
#include "LockList.h"
extern LockList *g_lockList;

RetrieveFTP::RetrieveFTP(ItemCell *itemcell)
    : Retrieve(itemcell)
{
    // set retrPrefix
    // only when the option "no CWD" is enabled
    // and directory is not null string
    // directory takes the form like:"/dir1/dir2", "/dir1" or ""
    if (itemcell->ret_Options().isFtpNoCwdEnabled()
            && !itemcell->ret_URL_Container().ret_Dir().empty()) {
        retrPrefix = itemcell->ret_URL_Container().ret_Dir() + '/';
    }
}

RetrieveFTP::~RetrieveFTP()
{
}

// sock_commandはコマンド用ソケット
// sock_waitはFTPサーバーからの接続受け付け用ソケット
void RetrieveFTP::Get_PORT_command(const Socket &sock_command,
                                   const Socket &sock_wait,
                                   string &port_command)
{
    sockaddr_in addr_in;
    // sock_commandから,ローカルマシンのIPアドレスを取得
    if (sock_command.Getsockname(addr_in) < 0) {
        throw ItemCell::ITEM_ESOCKET;
    }
    char *ip_string = inet_ntoa(addr_in.sin_addr);
    // sock_waitから,ポート番号を取得
    if (sock_wait.Getsockname(addr_in) < 0) {
        throw ItemCell::ITEM_ESOCKET;
    }
    int port = ntohs(addr_in.sin_port);
    int ipaddr[4];
    sscanf(ip_string, "%d.%d.%d.%d", &ipaddr[0], &ipaddr[1], &ipaddr[2], &ipaddr[3]);
    //PORTコマンドを作成
    port_command = "PORT " + itos(ipaddr[0]) + "," +
                   itos(ipaddr[1]) + "," +
                   itos(ipaddr[2]) + "," +
                   itos(ipaddr[3]) + "," +
                   itos(port / 256) + "," +
                   itos(port % 256) + "\r\n";
}

// send PORT command
void RetrieveFTP::Send_port_command(const Socket &sock_command,
                                    const Socket &sock_wait)
{
    // PORTコマンドの送信
    string command; // PORTコマンドの文字列
    string retbuf;

    Get_PORT_command(sock_command, sock_wait, command);
    Send_command(command, sock_command);

    if (Get_response(sock_command, retbuf) < 0) { // failed to send port command
        itemcell->set_Command(ItemCell::DLERROR);
        throw ItemCell::ITEM_EPROT;
    }
}


//FTP activeモード
//sock_command-- コマンド用のソケット
//sock_wait-- FTPサーバーからの接続受け付け用のソケットへのポインタ
//startingbyte-- 転送開始バイト
void RetrieveFTP::Handle_ftp_active_mode(const Socket &sock_command,
                                         Socket &sock_data,
                                         unsigned int &startingbyte)
{
    string command;
    string retbuf;

    Socket sock_wait(-1, Socket::DEFAULT);
    connect_from(sock_command, sock_wait);
    command = "REST " + itos(startingbyte) + "\r\n";
    Send_command(command, sock_command);
    if (Get_response(sock_command, retbuf) < 0) {
        itemcell->Send_message_to_gui(_("Sorry, cannot resume"), MSG_DOWNLOAD_ERROR);
        if (startingbyte > 0 && itemcell->ret_Options().ret_use_no_redownload()) {
            itemcell->set_Command(ItemCell::DLERRORSTOP);
            throw ItemCell::ITEM_EINTER;
        }
        startingbyte = 0;
    }

    command = "RETR " + retrPrefix + itemcell->ret_URL_Container().ret_Filename() + "\r\n";
    Send_command(command, sock_command);

    if (Get_response(sock_command, retbuf) < 0) { //failed to RETR
        itemcell->set_Command(ItemCell::DLERROR);
        throw ItemCell::ITEM_EPROT;
    }
    try {
        sock_data = sock_wait.Accept();
        itemcell->Send_message_to_gui(_("Data connection established"), MSG_DOWNLOAD_INFO);
        //return sock_data;
    } catch (SocketErrorType err) {
        itemcell->set_Command(ItemCell::DLERROR);
        throw ItemCell::ITEM_EPROT;
    }
}

//FTP passiveモード
//sock_command-- コマンド用のソケット
//startingbyte-- 転送開始バイト
void RetrieveFTP::Handle_ftp_passive_mode(const Socket &sock_command,
                                          Socket &sock_data,
                                          unsigned int &startingbyte)
{
    string report;
    string command;
    string retbuf;

    FTPcontainer ftpcon = connect_to(sock_command);
    if (ftpcon.ret_Port() == 0) {
        itemcell->set_Command(ItemCell::DLERROR);
        throw ItemCell::ITEM_EPROT;
    }

    if (itemcell->ret_Options().ret_use_ftp_proxy() &&
            !itemcell->ret_Options().ret_ftp_proxy().ret_Server().empty()) {
        Make_TCP_connection(sock_data,
                            itemcell->ret_Options().ret_ftp_proxy().ret_Server(),
                            ftpcon.ret_Port());
    } else {
        Make_TCP_connection(sock_data,
                            itemcell->ret_URL_Container().ret_Hostname(),
                            ftpcon.ret_Port());
    }

    //Make_TCP_connection(sock_data, ftpcon.ret_Port());

    command = "REST " + itos(startingbyte) + "\r\n";
    Send_command(command, sock_command);

    if (Get_response(sock_command, retbuf) < 0) {
        itemcell->Send_message_to_gui(_("Sorry, cannot resume"), MSG_DOWNLOAD_ERROR);
        if (startingbyte > 0 && itemcell->ret_Options().ret_use_no_redownload()) {
            itemcell->set_Command(ItemCell::DLERRORSTOP);
            throw ItemCell::ITEM_EINTER;
        }
        startingbyte = 0;
    }

    itemcell->set_Size_Current(startingbyte);
    command = "RETR " + retrPrefix + itemcell->ret_URL_Container().ret_Filename() + "\r\n";
    Send_command(command, sock_command);

    if (Get_response(sock_command, retbuf) < 0) { // failed to RETR
        itemcell->set_Command(ItemCell::DLERROR);
        throw ItemCell::ITEM_EPROT;
    }
}

//
// FTPサーバーから離脱
//
void RetrieveFTP::Leave_ftp_server(Socket &sock_command)
{
    string retbuf;

    if (!itemcell->ret_Options().ret_FTP_nosend_quit()) {
        Send_command("QUIT\r\n", sock_command);
        Get_response(sock_command, retbuf);
        sock_command.Shutdown(2);
    }
}

//
// 分割ダウンロードのための準備
//
// divide個の部分アイテムを作成
// return:
//     false : エラー(分割不可)
//     true : 分割成功
//
#if 0
bool RetrieveFTP::Create_partial_item_entry(unsigned int divide,
                                            unsigned int total_size)
{
    /*unsigned int downloaded_size = Get_starting_byte();
    if(downloaded_size > 0) {
      string old_filename = ret_Options().ret_Store_Dir()+ret_Filename();
      string new_filename = old_filename+".-1";
      if(rename(old_filename.c_str(), new_filename.c_str()) < 0) {
        downloaded_size = 0;
      }
      } else {
      struct stat filestat;
      string filename = ret_Options().ret_Store_Dir()+ret_Filename()+".-1";
      if(stat(filename.c_str(), &filestat) >= 0) {
        downloaded_size = filestat.st_size;
      }
      }*/
    unsigned int downloaded_size = 0;//Get_starting_byte();
    unsigned int unit, unit_reminder;
    while (1) {
        unit = (total_size - downloaded_size) / divide;
        if (unit > 0) {
            unit_reminder = (total_size - downloaded_size) % divide;
            break;
        }
        if (--divide == 1) return false;
    }
    begin_split();
    ret_Options().set_Divide(divide);
    for (unsigned int i = 0; i < divide; i++) {
        unsigned int start_range = unit * i + downloaded_size;
        unsigned int end_range;
        if (i == divide - 1) {
            end_range = unit * (i + 1) + downloaded_size + unit_reminder;
        } else {
            end_range = unit * (i + 1) + downloaded_size;
        }
        URLcontainer urlcon_partial = ret_URL_Container();
        Options options_partial = ret_Options();
        options_partial.set_Divide(1);
        ItemCell_FTP_p *itemcell_partial = new ItemCell_FTP_p(ret_URL(),
                                                              urlcon_partial,
                                                              options_partial,
                                                              _("Created"),
                                                              this,
                                                              i,
                                                              start_range,
                                                              end_range);
        itemcell_partial->set_Status(ItemCell::ITEM_READY);
        Send_partial(itemcell_partial);
        Send_message_to_gui(_("Partial item added"), MSG_DOWNLOAD_INFO);
    }
    end_split();
    return true;
}
#endif

FTPcontainer RetrieveFTP::connect_to(const Socket &sock_command)
{
    string report;
    string command;
    string retbuf;

    Send_command("PASV\r\n", sock_command);
    if (Get_response(sock_command, retbuf) < 0) {
        itemcell->Send_message_to_gui(_("Cannot enter PASV mode"), MSG_DOWNLOAD_ERROR);
        itemcell->set_Command(ItemCell::DLERRORSTOP);
        throw ItemCell::ITEM_EPROT;
    }
    FTPcontainer ftpcon(retbuf);

    return ftpcon;
}

void RetrieveFTP::connect_from(const Socket &sock_command, Socket &sock_wait)
{
#ifndef INET6
    sock_wait.create(AF_UNSPEC, SOCK_STREAM, PF_UNSPEC);
    sockaddr_in serv_addr;// server(local machine)

    memset((char *)&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(0);

    if (sock_wait.bad()) throw ItemCell::ITEM_EIO;
    if (sock_wait.Bind(serv_addr) < 0) {
        throw ItemCell::ITEM_EBIND;
    }

#else
    // IPv6
    sock_wait.create(AF_INET, SOCK_STREAM, 0);
    struct addrinfo hints;
    struct addrinfo *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    char *service = "0";
    int error = getaddrinfo(NULL, service, &hints, &res);
    if (error) {
        throw ItemCell::ITEM_EIO;
    }

    if (sock_wait.bad()) throw ItemCell::ITEM_EIO;
    if (sock_wait.Bind(res) < 0) {
        throw ItemCell::ITEM_EBIND;
    }
#endif
    if (sock_wait.Listen() < 0) {
        throw ItemCell::ITEM_ELISTEN;
    }

    Send_port_command(sock_command, sock_wait);
}

bool Is_dot_dir(const string &file)
{
    if (file.empty() || file == "." || file == ".." || file == "./" || file == "../") return true;
    else return false;
}

list<ItemCell *>::const_iterator Is_in(const list<ItemCell *> &target_list, const string &target)
{
    for (list<ItemCell *>::const_iterator itr = target_list.begin(); itr != target_list.end(); ++itr) {
        if ((*itr)->ret_URL() == target) return itr;
    }
    return target_list.end();
}

ItemCell *RetrieveFTP::Make_itemcell(const string &url, const string &save_dir)
{
    ItemCell *itemcellNew = NULL;
    URLcontainer urlcon;
    if (urlcon.Parse_URL(url)) {
        Options options_temp = itemcell->ret_Options();
        options_temp.set_Store_Dir(save_dir);
        options_temp.set_FTP_recurse_count(itemcell->ret_Options().ret_FTP_recurse_count() - 1);
        itemcellNew = new ItemCell(url, urlcon, options_temp, _("Created"));
        //itemcell->set_documentroot_dir(documentroot_dir);
        itemcellNew->set_root_url(itemcell->ret_root_url());
        itemcellNew->set_Status(ItemCell::ITEM_READY);
    }
    return itemcellNew;
}

list<ItemCell *> RetrieveFTP::Make_filelist(string lsdata)
{
    list<ItemCell *> filelist;
    list<ItemCell *> dirlist;
    while (lsdata.size()) {
        string line = Token_splitter(lsdata, "\n");
        if (line.empty()) break;
        string mode = Token_splitter(line, " \t");
        if (casecomp(mode, "total")) continue;
        string dsize = Token_splitter(line, " \t");
        string user = Token_splitter(line, " \t");
        string group = Token_splitter(line, " \t");
        string size = Token_splitter(line, " \t");
        string month = Token_splitter(line, " \t");
        string day = Token_splitter(line, " \t");
        string year_hm = Token_splitter(line, " \t");
        string fullpath;
        string savedir;
        unsigned int arrow_pos;
        bool dir_flag = false;

        try {
            if ((arrow_pos = line.find("->")) != string::npos) {
                //convert link to real path
                string file;
                // ret_FTP_convet_link_to_realpath ->
                // ret_FTP_get_symlink_as_realfile
                if (!itemcell->ret_Options().ret_FTP_get_symlink_as_realfile()) {
                    file = Remove_white(line.substr(arrow_pos + 2));
                    string file_link = Remove_white(line.substr(0, arrow_pos));
                    // create symlink
                    if (symlink(file.c_str(), (itemcell->ret_Options().ret_Store_Dir() + file_link).c_str()) < 0) {
                        string line = _("Failed to create symbolic link. Ignored") + ':';
                        itemcell->Send_message_to_gui(line + strerror(errno), MSG_DOWNLOAD_INFO);
                    }
                } else {
                    file = Remove_white(line.substr(0, arrow_pos));
                }
                if (Is_dot_dir(file)) continue;

                fullpath = get_abs_url(itemcell->ret_URL_Container().ret_URL(), file);
                if (itemcell->ret_Options().ret_FTP_no_ascend() &&
                        !itemcell->ret_Options().ret_FTP_get_symlink_as_realfile() &&
                        !startwith(fullpath, itemcell->ret_root_url())) {
                    continue;
                }
                get_file(file);
                savedir = get_storedir(itemcell->ret_Options().ret_Store_Dir(), file);
                if (fullpath.at(fullpath.size() - 1) == '/') {
                    dir_flag = true;
                }
            } else {
                string file = Remove_white(line);
                if (Is_dot_dir(file)) continue;
                if (mode.at(0) == 'd') {
                    //if no_subdir then continue
                    if (file.at(file.size() - 1) != '/') {
                        file += '/';
                        dir_flag = true;
                    }
                }
                //file extension filter
                if (!dir_flag &&
                        itemcell->ret_Options().ret_FTP_use_filter() &&
                        !itemcell->ret_Options().Is_in_FTP_filter_target_list(file)) {
                    continue;
                }
                fullpath = get_abs_url(itemcell->ret_URL_Container().ret_URL(), file);

                if (itemcell->ret_Options().ret_FTP_no_ascend() &&
                        !startwith(fullpath, itemcell->ret_root_url())) {
                    continue;
                }
                get_file(file);
                savedir = get_storedir(itemcell->ret_Options().ret_Store_Dir(), file);
            }
            if (!itemcell->ret_Options().ret_FTP_get_symlink_as_realfile()) {
                list<ItemCell *>::const_iterator itr;
                string fix;
                if (dir_flag) {
                    fix = fullpath.substr(0, fullpath.size() - 1);
                } else {
                    fix = fullpath;
                }
                if ((itr = Is_in(filelist, fix)) != filelist.end()) {
                    filelist.remove(*itr);
                } else {
                    if (dir_flag) {
                        fix = fullpath;
                    } else {
                        fix = fullpath + '/';
                    }
                    if ((itr = Is_in(dirlist, fix)) != dirlist.end()) {
                        continue;
                    }
                }
            }

            if (dir_flag && itemcell->ret_Options().ret_FTP_recurse_count() <= 1) {
                continue;
            }
            ItemCell *itemcellNew = Make_itemcell(fullpath, savedir);
            if (itemcell != NULL) {
                if (dir_flag) {
                    dirlist.push_back(itemcellNew);
                } else {
                    filelist.push_back(itemcellNew);
                }
            }

        } catch (int err) {
            // dir stack underrun
            continue;
        }
    }
    filelist.insert(filelist.end(), dirlist.begin(), dirlist.end());

    return filelist;
}

string RetrieveFTP::Get_fileinfo_by_ls(const Socket &sock_command, const string &filename)
{
    string command;
    string retbuf;

    try {
        Socket sock_data(-1, Socket::DEFAULT);
        switch (itemcell->ret_Options().ret_FTP_Mode()) {
            case Options::FTP_ACTIVE_MODE: {
                Socket sock_wait(-1, Socket::DEFAULT);
                connect_from(sock_command, sock_wait);
                command = "LIST -l " + retrPrefix;
                if (filename.size()) {
                    command += filename;
                }
                command += "\r\n";
                Send_command(command, sock_command);
                if (Get_response(sock_command, retbuf) < 0) {
                    itemcell->Send_message_to_gui(_("Failed to list files"), MSG_DOWNLOAD_ERROR);
                    itemcell->set_Command(ItemCell::DLERRORSTOP);// fix this
                    throw ItemCell::ITEM_EPROT;
                }
                sock_data = sock_wait.Accept();
                itemcell->Send_message_to_gui(_("Data connection established"), MSG_DOWNLOAD_INFO);
                break;
            }
            case Options::FTP_PASSIVE_MODE:
            default: {
                FTPcontainer ftpcon = connect_to(sock_command);
                if (ftpcon.ret_Port() == 0) {
                    itemcell->set_Command(ItemCell::DLERROR);
                    throw ItemCell::ITEM_EPROT;
                }

                if (itemcell->ret_Options().ret_use_ftp_proxy() &&
                        !itemcell->ret_Options().ret_ftp_proxy().ret_Server().empty()) {
                    Make_TCP_connection(sock_data,
                                        itemcell->ret_Options().ret_ftp_proxy().ret_Server(),
                                        //itemcell->ret_URL_Container().ret_Hostname(),
                                        ftpcon.ret_Port());
                } else {
                    Make_TCP_connection(sock_data,
                                        itemcell->ret_URL_Container().ret_Hostname(),
                                        ftpcon.ret_Port());
                }

                itemcell->Send_message_to_gui(_("Data connection established"), MSG_DOWNLOAD_INFO);
                command = "LIST -l " + retrPrefix;
                if (filename.size()) {
                    command += filename;
                }
                command += "\r\n";
                Send_command(command, sock_command);
                if (Get_response(sock_command, retbuf) < 0) {
                    itemcell->Send_message_to_gui(_("Failed to list files"), MSG_DOWNLOAD_ERROR);
                    itemcell->set_Command(ItemCell::DLERRORSTOP);// fix this
                    throw ItemCell::ITEM_EPROT;
                }

                break;
            }
        }

        string lsdata;
        list<int> fd_list;
        //unsigned int timedout = itemcell->ret_Options().ret_Timed_Out();
        struct timeval tv;
        tv.tv_sec = itemcell->ret_Options().ret_Timed_Out();
        tv.tv_usec = 0;

        while (1) {
            fd_list.clear();
            fd_list.push_back(itemcell->ret_Desc_r());

            int retval = sock_data.is_readready(&tv, fd_list);
            if (retval && sock_data.is_set(fd_list)) {
                // reset the interval of time out
                tv.tv_sec = itemcell->ret_Options().ret_Timed_Out();
                tv.tv_usec = 0;

                char databuf[4096];
                int size = sock_data.Recv(databuf, sizeof(databuf - 1));
                if (size == 0) break;
                databuf[size] = '\0';
                lsdata += databuf;
            } else if (retval && Socket::is_set(itemcell->ret_Desc_r(), fd_list)) {
                ItemCommand itemcommand;
                read(itemcell->ret_Desc_r(), &itemcommand, sizeof(ItemCommand));
                itemcell->set_Command(itemcommand);
                if (itemcell->ret_Dl_status() == ItemCell::DLCHANGE) {
                    itemcell->Process_command(itemcommand);
                    updateInterval(tv, itemcell->ret_Options().ret_Timed_Out());
                } else {
                    sock_data.Shutdown(2);
                    throw ItemCell::ITEM_EINTER;
                }
            } else {
                // timed out
                sock_data.Shutdown(2);
                throw ItemCell::ITEM_ETIMEDOUT;
            }
        }

        for (string::iterator itr = lsdata.begin(); itr != lsdata.end(); ++itr) {
            if (*itr == '\r') {
                *itr = ' ';
            }
        }
        sock_data.Shutdown(2);
        return lsdata;
    } catch (SocketErrorType err) {
        itemcell->set_Command(ItemCell::DLERROR);
        throw ItemCell::ITEM_EPROT;
    }
}

list<ItemCell *> RetrieveFTP::Get_filelist(const Socket &sock_command)
{
    string retbuf;

    string lsdata = Get_fileinfo_by_ls(sock_command);

    itemcell->Send_message_to_gui('\n' + lsdata, MSG_DOWNLOAD_RECV);

    Get_response(sock_command, retbuf);

    list<ItemCell *> file_list = Make_filelist(lsdata);

    return file_list;
}

void RetrieveFTP::Make_Authentication(const Socket &sock_command)
{
    //Send_username_password(const Socket& sock);
    //Send_open(const Socket& sock);
    //Send_open2(const Socket& sock);
    //Send_site(const Socket& sock);
    //Send_user_password(const Socket& sock);
    //Send_proxy_username_password(const Socket& sock);
    //Send_proxy_user_password(const Socket& sock);

    if (itemcell->ret_Options().ret_use_ftp_proxy() &&
            !itemcell->ret_Options().ret_ftp_proxy().ret_Server().empty()) {
        switch (itemcell->ret_Options().ret_ftp_proxy_login_proc()) {
            case Options::FTPPROXY_PL:
                Send_proxy_user_password(sock_command);
                //Send_username_password(sock_command);
                break;
            case Options::FTPPROXY_PL_OPEN:
                Send_proxy_username_password(sock_command);
                Send_open(sock_command);
                Send_username_password(sock_command);
                break;
            case Options::FTPPROXY_PL_OPEN2:
                Send_proxy_username_password(sock_command);
                Send_open2(sock_command);
                Send_username_password(sock_command);
                break;
            case Options::FTPPROXY_PL_SITE:
                Send_proxy_username_password(sock_command);
                Send_site(sock_command);
                Send_username_password(sock_command);
                break;
            case Options::FTPPROXY_PL_USER:
                Send_proxy_username_password(sock_command);
                Send_user_password(sock_command);
                break;
            case Options::FTPPROXY_USER:
                Send_user_password(sock_command);
                break;
            case Options::FTPPROXY_OPEN:
                Send_open(sock_command);
                Send_username_password(sock_command);
                break;
            default:
                itemcell->Send_message_to_gui(_("Failed to login"), MSG_DOWNLOAD_ERROR);
                itemcell->set_Command(ItemCell::DLERROR);
                throw ItemCell::ITEM_EPROT;
        }
    } else {
        Send_username_password(sock_command);
    }
}

void RetrieveFTP::Get_username_password(string &username, string &password)
{
    string def_username = "anonymous";
    string def_password = "IE40user@";

    if (itemcell->ret_Options().Whether_use_authentication() &&
            itemcell->ret_Options().ret_User() != "") {
        username = itemcell->ret_Options().ret_User();
        password = itemcell->ret_Options().ret_Password();
    } else {
        username = def_username;
        password = def_password;
    }
}

void RetrieveFTP::Get_proxy_username_password(string &username, string &password)
{
    string def_username = "anonymous";
    string def_password = "IE40user@";

    if (itemcell->ret_Options().ret_use_ftp_proxy_authentication() &&
            itemcell->ret_Options().ret_ftp_proxy_User() != "") {
        username = itemcell->ret_Options().ret_ftp_proxy_User();
        password = itemcell->ret_Options().ret_ftp_proxy_Password();
    } else {
        username = def_username;
        password = def_password;
    }
}

void RetrieveFTP::Send_username_password_sub(const Socket &sock_command,
                                             const string &username,
                                             const string &password)
{
    string command;
    string retbuf;
    // send USER
    command = "USER " + username + "\r\n";
    Send_command(command, sock_command);
    if (Get_response(sock_command, retbuf) < 0) {
        itemcell->set_Command(ItemCell::DLERROR);
        throw ItemCell::ITEM_EPROT;
    }

    // send PASS
    command = "PASS " + password + "\r\n";
    Send_command_pass(command, sock_command);

    if (Get_response(sock_command, retbuf) < 0) { // maybe full?
        itemcell->Send_message_to_gui(_("Failed to login"), MSG_DOWNLOAD_ERROR);
        itemcell->set_Command(ItemCell::DLERROR);
        throw ItemCell::ITEM_EPROT;
    }
}

void RetrieveFTP::Send_username_password(const Socket &sock_command)
{
    string username;
    string password;
    Get_username_password(username, password);

    Send_username_password_sub(sock_command, username, password);
}

void RetrieveFTP::Send_proxy_username_password(const Socket &sock_command)
{
    string username;
    string password;
    Get_proxy_username_password(username, password);

    Send_username_password_sub(sock_command, username, password);
}

void RetrieveFTP::Send_user_password(const Socket &sock_command)
{
    string userhost;
    string password;
    Get_username_password(userhost, password);

    userhost += "@" + itemcell->ret_URL_Container().ret_Hostname();
    if (itemcell->ret_URL_Container().ret_Port() != 21) {
        userhost += ":" + itos(itemcell->ret_URL_Container().ret_Port());
    }
    Send_username_password_sub(sock_command, userhost, password);
}

void RetrieveFTP::Send_proxy_user_password(const Socket &sock_command)
{
    string userhost;
    string password;
    Get_proxy_username_password(userhost, password);

    userhost += "@" + itemcell->ret_URL_Container().ret_Hostname();
    if (itemcell->ret_URL_Container().ret_Port() != 21) {
        userhost += ":" + itos(itemcell->ret_URL_Container().ret_Port());
    }
    Send_username_password_sub(sock_command, userhost, password);
}

void RetrieveFTP::Send_open(const Socket &sock_command)
{
    string command = "OPEN " + itemcell->ret_URL_Container().ret_Hostname();

    Send_open_site_sub(sock_command, command);
}

void RetrieveFTP::Send_open2(const Socket &sock_command)
{
    string command = "open " + itemcell->ret_URL_Container().ret_Hostname();

    Send_open_site_sub(sock_command, command);
}

void RetrieveFTP::Send_site(const Socket &sock_command)
{
    string command = "open " + itemcell->ret_URL_Container().ret_Hostname();

    Send_open_site_sub(sock_command, command);
}

void RetrieveFTP::Send_open_site_sub(const Socket &sock_command, string command)
{
    if (itemcell->ret_URL_Container().ret_Port() != 21) {
        command += ":" + itos(itemcell->ret_URL_Container().ret_Port());
    }
    command += "\r\n";

    string retbuf;
    Send_command(command, sock_command);
    if (Get_response(sock_command, retbuf) < 0) {
        itemcell->Send_message_to_gui(_("Failed to login"), MSG_DOWNLOAD_ERROR);
        itemcell->set_Command(ItemCell::DLERROR);
        throw ItemCell::ITEM_EPROT;
    }
}

ItemCell::DownloadStatusType RetrieveFTP::Download_Main()
{
    unsigned int startingbyte = 0;
    string report;
    string command;
    string retbuf;

    if (itemcell->ret_root_url().size()) {
        sleep(2);
    }
    //Socket sock_command;
    Socket sock_command(-1, Socket::DEFAULT);

    try {
        // added 2001/5/13
        // from here
        if (itemcell->ret_Session_counter() > 1 &&
                itemcell->Is_current_session_valid()) {
            itemcell->Send_message_to_gui("Using Server Template '" +
                                          itemcell->ret_svt().ret_template_name() +
                                          "'", MSG_DOWNLOAD_INFO);
            itemcell->Send_message_to_gui("Entering Session " +
                                          itos(itemcell->ret_Session_counter()),
                                          MSG_DOWNLOAD_INFO);
            URLcontainer orig_urlcon;
            orig_urlcon.Parse_URL(itemcell->ret_URL());
            // create URL
            if (itemcell->ret_current_session().ret_get_vector().size() &&
                    !itemcell->ret_URL_Container().Parse_URL(itemcell->ret_current_session().Create_URL_from_get_vector(orig_urlcon, itemcell->ret_Retrieved_urlcon()))) {
                throw ItemCell::ITEM_ESERVERCONFIG;
            }
            // fix this
            // set correct filename
            // これはユーザのファイル名設定を無視している
            itemcell->set_Filename(itemcell->ret_URL_Container().ret_Filename());

            //itemcell->set_Filename(itemcell->ret_URL_Container().ret_Filename());
            // create Referer
            if (itemcell->ret_current_session().ret_referer_vector().size()) {
                itemcell->ret_Options().set_Referer_Type(Options::REFERER_USER_DEFINED);
                itemcell->ret_Options().set_Referer(itemcell->ret_current_session().Create_URL_from_referer_vector(orig_urlcon, itemcell->ret_Retrieved_urlcon()));
            }
        }
        // to here

        // get the file size if the file exists
        if (itemcell->ret_Status() != ItemCell::ITEM_CRCERROR &&
                itemcell->ret_Status() != ItemCell::ITEM_EXECERROR &&
                itemcell->ret_Status() != ItemCell::ITEM_DOWNLOAD_AGAIN &&
                itemcell->ret_Options().ret_downm_type() != Options::DOWNM_NORESUME) {
            startingbyte = Get_starting_byte();
        }

        // set item's status
        itemcell->set_Status(ItemCell::ITEM_DOWNLOAD);
        itemcell->Send_status();

        bool use_proxy = false;
        if (itemcell->ret_Options().ret_use_ftp_proxy() &&
                !itemcell->ret_Options().ret_ftp_proxy().ret_Server().empty()) {
            use_proxy = true;
        }

        if (use_proxy) {
            Make_TCP_connection(sock_command,
                                itemcell->ret_Options().ret_ftp_proxy().ret_Server(),
                                itemcell->ret_Options().ret_ftp_proxy().ret_Port(),
                                itemcell->ret_URL_Container().ret_Hostname(),
                                itemcell->ret_URL_Container().ret_Port());
        } else {
            Make_TCP_connection(sock_command,
                                itemcell->ret_URL_Container().ret_Hostname(),
                                itemcell->ret_URL_Container().ret_Port());
        }

        if (!sock_command.isPooledSocket()) {
            if (Get_response(sock_command, retbuf) < 0) { //cannot connect
                itemcell->set_Command(ItemCell::DLERROR);
                throw ItemCell::ITEM_EPROT;
            }

            Make_Authentication(sock_command);
        }
        // send TYPE command
        switch (itemcell->ret_Options().ret_FTP_ret_mode()) {
            case Options::FTP_BINARY:
                Send_command("TYPE I\r\n", sock_command);
                break;
            case Options::FTP_ASCII:
            default:
                Send_command("TYPE A\r\n", sock_command);
                break;
        }
        if (Get_response(sock_command, retbuf) < 0) {
            itemcell->set_Command(ItemCell::DLERROR);
            throw ItemCell::ITEM_EPROT;
        }
        // send CWD command
        if (!itemcell->ret_URL_Container().ret_Dir().empty()
                && !itemcell->ret_Options().isFtpNoCwdEnabled()) {
            command = "CWD " + itemcell->ret_URL_Container().ret_Dir() + "\r\n";
            Send_command(command, sock_command);

            if (Get_response(sock_command, retbuf) < 0) {
                itemcell->Send_message_to_gui(_("Specified directory was not found"), MSG_DOWNLOAD_ERROR);

                itemcell->set_Command(ItemCell::DLERRORSTOP);
                throw ItemCell::ITEM_EPROT;

            }
        }
        // send LS -l %s command
        bool dirbrowse_flag = false;
        // send SIZE command
        if (itemcell->ret_URL_Container().ret_Filename().empty()) {
            dirbrowse_flag = true;
        } else {
            if (startingbyte > 0 &&
                    itemcell->ret_Options().ret_downm_type() == Options::DOWNM_IFMODSINCE) {
                command = "MDTM " + retrPrefix + itemcell->ret_URL_Container().ret_Filename() + "\r\n";
                Send_command(command, sock_command);
                if (Get_response(sock_command, retbuf) < 0) {
                    itemcell->Send_message_to_gui(_("MDTM command failed. Anyway, try resuming"), MSG_DOWNLOAD_INFO);
                } else {
                    time_t modtime = get_mod_time(retbuf);
                    if (Is_older_than_remote(modtime)) {
                        itemcell->Send_message_to_gui(_("Modification time of remote file is newer than local file's one. Resume disabled"), MSG_DOWNLOAD_INFO);
                        startingbyte = 0;
                    }
                }
            }

            command = "SIZE " + retrPrefix + itemcell->ret_URL_Container().ret_Filename() + "\r\n";

            Send_command(command, sock_command);
            int retstat;
            if (Get_response(sock_command, retstat, retbuf) < 0) {
                // SIZE command failed
                // some ftp servers in SunOS can not understand SIZE command
                // They return:
                //   500 'SIZE ppp-2.4.1.tar.gz': command not understood.
                // So, in this case, try "LIST -l $(retrPrefix)$(filename)".
                bool doCWDFlag = true;
                if (retstat == 500) {
                    string line = Get_fileinfo_by_ls(sock_command, itemcell->ret_URL_Container().ret_Filename());
                    if (line.empty()) {
                        itemcell->Send_message_to_gui(_("Failed to list files"), MSG_DOWNLOAD_ERROR);
                        itemcell->set_Command(ItemCell::DLERRORSTOP);// fix this
                        throw ItemCell::ITEM_EPROT;
                    }
                    itemcell->Send_message_to_gui('\n' + line, MSG_DOWNLOAD_RECV);
                    Get_response(sock_command, retbuf);

                    string mode = Token_splitter(line, " \t");
                    string dsize = Token_splitter(line, " \t");
                    string user = Token_splitter(line, " \t");
                    string group = Token_splitter(line, " \t");
                    string size = Token_splitter(line, " \t");
                    string month = Token_splitter(line, " \t");
                    string day = Token_splitter(line, " \t");
                    string year_hm = Token_splitter(line, " \t");
                    if (mode.at(0) != 'd') {
                        doCWDFlag = false;
                        retbuf = size;
                    }
                }

                if (doCWDFlag) {
                    URLcontainer urlcon_temp = itemcell->ret_URL_Container();
                    itemcell->ret_URL_Container().set_Dir(itemcell->ret_URL_Container().ret_Dir() + itemcell->ret_URL_Container().ret_File());
                    itemcell->ret_URL_Container().set_File("");

                    if (itemcell->ret_root_url().size()
                            && itemcell->ret_root_url() != itemcell->ret_URL_Container().ret_Protocol() + "//" +
                            itemcell->ret_URL_Container().ret_Hostname() +
                            itemcell->ret_URL_Container().ret_Dir() + '/'
                            && itemcell->ret_Options().ret_FTP_recurse_count() == 1) {
                        // recursive download limit exceeded
                        itemcell->Send_message_to_gui(_("Depth of recursion exceeded. This message can be ignored safely"), MSG_DOWNLOAD_INFO);
                        itemcell->get_Options_Lock();
                        itemcell->ret_Options_opt().set_Delete_When_Finish(true);
                        itemcell->ret_Options_opt().set_Dont_Delete_Without_CRC(false);
                        itemcell->release_Options_Lock();
                        throw ItemCell::ITEM_ESUCCESS;
                    } else {
                        itemcell->Send_message_to_gui(_("Try again assuming as directory"), MSG_DOWNLOAD_INFO);
                        if (!itemcell->ret_URL_Container().ret_Dir().empty()
                                && !itemcell->ret_Options().isFtpNoCwdEnabled()) {
                            command = "CWD " + itemcell->ret_URL_Container().ret_Dir() + "\r\n";
                            Send_command(command, sock_command);

                            if (Get_response(sock_command, retbuf) < 0) {
                                itemcell->Send_message_to_gui(_("Specified directory or file was not found"), MSG_DOWNLOAD_ERROR);
                                itemcell->set_URL_Container(urlcon_temp);
                                itemcell->set_Command(ItemCell::DLERRORSTOP);
                                throw ItemCell::ITEM_EPROT;
                            } else {
                                dirbrowse_flag = true;
                                //set_Command(ItemCell::DLSUCCESS);
                                //throw ItemCell::ITEM_ESUCCESS;
                            }
                        } else {
                            // fix this
                            dirbrowse_flag = true;
                        }
                    }
                }
            }
            if (!dirbrowse_flag &&
                    itemcell->ret_root_url().size() &&
                    itemcell->ret_Options().ret_FTP_use_filter() &&
                    !itemcell->ret_Options().Is_in_FTP_filter_target_list(itemcell->ret_URL_Container().ret_Filename())) {
                itemcell->Send_message_to_gui(_("Download abort due to extension. This message can be ignored safely"), MSG_DOWNLOAD_INFO);
                itemcell->get_Options_Lock();
                itemcell->ret_Options_opt().set_Delete_When_Finish(true);
                itemcell->ret_Options_opt().set_Dont_Delete_Without_CRC(false);
                itemcell->release_Options_Lock();
                throw ItemCell::ITEM_ESUCCESS;
            }
        }

        if (dirbrowse_flag) {
            itemcell->set_root_url(itemcell->ret_URL_Container().ret_Protocol() + "//" +
                                   itemcell->ret_URL_Container().ret_Hostname() +
                                   itemcell->ret_URL_Container().ret_Dir() + '/');

            list<ItemCell *> file_dir_list = Get_filelist(sock_command);

            itemcell->Send_status_recursive(file_dir_list);
            throw ItemCell::ITEM_ESUCCESS;
        }

        // now we know the actual size of file..
        itemcell->set_Size_Total((unsigned int)atoi(retbuf.c_str()));
        // limit file size
        if ( //itemcell->ret_Size_Total() != 0 &&
            itemcell->ret_Options().ret_use_size_lower_limit() &&
            (unsigned int)itemcell->ret_Options().ret_size_lower_limit() > itemcell->ret_Size_Total()) {
            itemcell->Send_message_to_gui(_("Download aborted due to the file size limitation"), MSG_DOWNLOAD_INFO);
            itemcell->set_Command(ItemCell::DLSTOP);
            throw ItemCell::ITEM_EINTER;
        }

        if (itemcell->ret_Size_Total() == startingbyte) {
            report = "'" + itemcell->ret_Filename() + "'" + _(" is already downloaded");
            itemcell->Send_message_to_gui(report, MSG_DOWNLOAD_INFO);

            itemcell->set_Size_Current(itemcell->ret_Size_Total());
            itemcell->Send_status();
            Leave_ftp_server(sock_command);
            //Send_message_to_gui(_("connection closed"), MSG_DOWNLOAD_INFO);

            throw ItemCell::ITEM_ESUCCESS;
        } else if (itemcell->ret_Size_Total() < startingbyte) {
            itemcell->Send_message_to_gui(_("Size of local file is larger than remote file's one. Download again"), MSG_DOWNLOAD_ERROR);
            startingbyte = 0;
        }
        if (itemcell->ret_Options().ret_Divide() > 1) {
            // send REST command
            itemcell->Send_message_to_gui(_("Checking whether ftp server supports resuming"), MSG_DOWNLOAD_INFO);

            command = "REST " + itos(itemcell->ret_Size_Total() / 2) + "\r\n";
            Send_command(command, sock_command);

            if (Get_response(sock_command, retbuf) < 0) {
                itemcell->Send_message_to_gui(_("Sorry, cannot resume"), MSG_DOWNLOAD_ERROR);
                itemcell->Send_message_to_gui(_("Starting normal download instead"), MSG_DOWNLOAD_INFO);
            } else {
                //Leave_ftp_server(sock_command);
                itemcell->Send_message_to_gui(_("Splitting file..."), MSG_DOWNLOAD_INFO);
                if (Create_partial_item_entry(itemcell->ret_Options().ret_Divide(), itemcell->ret_Size_Total())) {
                    Leave_ftp_server(sock_command);
                    itemcell->set_Status(ItemCell::ITEM_DOWNLOAD_PARTIAL);
                    itemcell->Send_status();
                    itemcell->set_Command(ItemCell::DLPARTIALSUCCESS);
                    throw ItemCell::ITEM_EINTER;
                } else {
                    itemcell->Send_message_to_gui(_("File is too small to be divided"), MSG_DOWNLOAD_ERROR);
                    itemcell->Send_message_to_gui(_("Starting normal download instead"), MSG_DOWNLOAD_INFO);
                }
            }
        }

        Options::FTP_Mode ftp_mode = itemcell->ret_Options().ret_FTP_Mode();

        Socket sock_data(-1, Socket::DEFAULT);
        switch (ftp_mode) {
            case Options::FTP_PASSIVE_MODE: { // PASV mode
                Handle_ftp_passive_mode(sock_command, sock_data, startingbyte);

                break;
            }
            case Options::FTP_ACTIVE_MODE:
            default: { //ACTIVE(NORMAL) mode
                Handle_ftp_active_mode(sock_command, sock_data, startingbyte);
                break;
            }
        }
        Start_Download(sock_data, startingbyte);
        sock_data.Shutdown(2);

        try {
            // expected reply: "226 Transfer complete."
            Get_response(sock_command, retbuf);

            if (!itemcell->ret_Options().ret_FTP_nosend_quit()) {
                Send_command("QUIT\r\n", sock_command);
                // expected reply: "221 Goodbye." or something like this
                Get_response(sock_command, retbuf);
                sock_command.Shutdown(2);
            }
        } catch (ItemCell::ItemErrorType err) {
            itemcell->PERROR(err);
            itemcell->Send_message_to_gui(_("An error occurred, but this error can be ignored safely"), MSG_DOWNLOAD_INFO);
            throw ItemCell::ITEM_ESUCCESS;
        }

        throw ItemCell::ITEM_ESUCCESS;
    } catch (ItemCell::ItemErrorType err) {
        //sock_command.Shutdown(2);
        //itemcell->Send_message_to_gui(_("Connection closed"), MSG_DOWNLOAD_INFO);
        if (itemcell->ret_Session_counter() > 1) {
            itemcell->Raise_option_update_flag();
        }
        itemcell->Reset_Session_counter();
        switch (err) {
            case ItemCell::ITEM_ESUCCESS:
                itemcell->set_Command(ItemCell::DLSUCCESS);
                return ItemCell::DLSUCCESS;
            case ItemCell::ITEM_ESUCCESSALR:
                itemcell->set_Command(ItemCell::DLSUCCESSALR);
                return ItemCell::DLSUCCESSALR;
            case ItemCell::ITEM_EINTER:
                switch (itemcell->ret_Dl_status()) {
                    case ItemCell::DLAGAIN:
                    case ItemCell::DLSTOP:
                    case ItemCell::DLERRORSTOP:
                    case ItemCell::DLDELETEITEM:
                    case ItemCell::DLDELETEITEMFILE:
                    case ItemCell::DLPARTIALSUCCESS:
                        sock_command.Shutdown(2);
                        itemcell->Send_message_to_gui(_("Connection closed"), MSG_DOWNLOAD_INFO);
                        break;
                    case ItemCell::DLHALT:
                    default:
                        sock_command.Shutdown(2);
                        break;
                }
                return itemcell->ret_Dl_status();
            case ItemCell::ITEM_EIOFILE:
                sock_command.Shutdown(2);
                itemcell->Send_message_to_gui(_("Connection closed"), MSG_DOWNLOAD_INFO);
                itemcell->set_Command(ItemCell::DLERRORSTOP);
                itemcell->PERROR(err);
                return ItemCell::DLERRORSTOP;
            case ItemCell::ITEM_EPROT://modified
                sock_command.Shutdown(2);
                itemcell->Send_message_to_gui(_("Connection closed"), MSG_DOWNLOAD_INFO);
                itemcell->PERROR(err);
                return itemcell->ret_Dl_status();
            case ItemCell::ITEM_ETIMEDOUT:
                sock_command.Shutdown(2);
                itemcell->Send_message_to_gui(_("Connection closed"), MSG_DOWNLOAD_INFO);
                itemcell->PERROR(err);
                itemcell->set_Command(ItemCell::DLERROR);
                return ItemCell::DLERROR;
            default:
                itemcell->PERROR(err);
                itemcell->set_Command(ItemCell::DLERROR);
                return ItemCell::DLERROR;
        }
    }
}

//
// FTPサーバーにコマンドを送る
//
void RetrieveFTP::Send_command(const string &command, const Socket &sock_command)
{
    itemcell->Send_message_to_gui(command, MSG_DOWNLOAD_SEND);

    SEND(command, sock_command);
}

void RetrieveFTP::Send_command_cwdpass(const string &command, const Socket &sock_command)
{
    string ast = command;
    //string::iterator itr = ast.begin()+ast.find(':')+1;
    for (string::iterator itr = ast.begin() + ast.find(':') + 1; itr != ast.end() && *itr != '@'; ++itr) {
        *itr = '*';
    }
    itemcell->Send_message_to_gui(ast, MSG_DOWNLOAD_SEND);
    SEND(command, sock_command);
}

//
// FTPサーバーにコマンドを送る
//
// Send_commandと一緒だが, コマンドの引数部分を*にして表示
void RetrieveFTP::Send_command_pass(const string &command, const Socket &sock_command)
{
    string ast = "PASS ";
    for (unsigned int i = 0; i < command.size() - 7; i++) { //fixed 2001/3/14
        ast += "*";
    }
    itemcell->Send_message_to_gui(ast, MSG_DOWNLOAD_SEND);

    SEND(command, sock_command);
}

//
// FTPサーバーからの応答を受け取る
//
// *retbufがNULLでないとき, 応答メッセージの最後の行がそこに格納される
// return:
//     -1: エラー
//      0: ユーザー操作による割り込み
//      1: 成功
//
int RetrieveFTP::Get_response(const Socket &sock_command, string &retbuf)
{
    int retstat;

    return Get_response(sock_command, retstat, retbuf);
}

/*
int RetrieveFTP::Get_response(const Socket& sock_command, int& retstat, string& retbuf)
{
  int size;
  int retval;
  retstat = 0;

  retbuf = "";

  list<int> fd_list;
  unsigned int timedout = itemcell->ret_Options().ret_Timed_Out();
//    struct timeval tv;
//    tv.tv_sec = itemcell->ret_Options().ret_Timed_Out();
//    tv.tv_usec = 0;

  string header_string;
  while(1) {
    char ch[1024];
    char *ch_tail = ch;
    fd_list.clear();
    fd_list.push_back(itemcell->ret_Desc_r());

    retval = sock_command.is_readready(timedout, fd_list);
    //retval = sock_command.is_readready(tv, fd_list);
    if(retval && sock_command.is_set(fd_list)) {
      while(1) {
	// reset time out value
	//tv.tv_sec = itemcell->ret_Options().ret_Timed_Out();
	//tv.tv_usec = 0;

	size = sock_command.Recv(ch, sizeof(ch)-(ch_tail-ch), MSG_PEEK);

	if(size < 0) {
	  throw ItemCell::ITEM_ERECV;
	} else if(size == 0) {

	  if(header_string.size() < 5) {
	    itemcell->set_Command(ItemCell::DLERRORSTOP);
	    throw ItemCell::ITEM_EPROT;
	  }

	  retstat = stoi(header_string.substr(0, 3));
	  retbuf = header_string.substr(4);
	  return 1;
	}

	int crlf_pos = 0;
	while(1) {
	  if(crlf_pos == size) break;
	  if(ch[crlf_pos] == '\r') {
	    break;
	  }
	  ++crlf_pos;
	}
	if(crlf_pos == 0) {
	  size = sock_command.Recv(ch, 2, 0);
	  header_string = "";
	  break;
	} else if(crlf_pos < size) {
	  size = sock_command.Recv(ch, crlf_pos+2, 0);
	  ch[size-2] = '\0';
	  header_string = ch;
	  break;
	} else {
	  //ch[size] = '\0';// dangerous??
	  ch_tail = ch+size;
	}
      }
    } else if(retval && Socket::is_set(itemcell->ret_Desc_r(), fd_list)) {
      ItemCommand itemcommand;
      read(itemcell->ret_Desc_r(), &itemcommand, sizeof(ItemCommand));
      itemcell->set_Command(itemcommand);
      if(itemcell->ret_Dl_status() == ItemCell::DLCHANGE) {
	itemcell->Process_command(itemcommand);
	//updateInterval(tv, itemcell->ret_Options().ret_Timed_Out());
	continue;
      } else {
	throw ItemCell::ITEM_EINTER;
      }
    } else {
      // timed out
      throw ItemCell::ITEM_ETIMEDOUT;
    }
    itemcell->Send_message_to_gui(header_string, MSG_DOWNLOAD_RECV);
    if(isdigit(header_string[0]) && header_string[3] == ' ') {
      if(header_string[0] == '4' || header_string[0] == '5') {
	retstat = stoi(header_string.substr(0, 3));
	return -1;
      } else {
	break;
      }
    }
  }
  retstat = stoi(header_string.substr(0, 3));
  retbuf = header_string.substr(4);

  return 1;
}
*/

int RetrieveFTP::Get_response(const Socket &sock_command, int &retstat, string &retbuf)
{
    retstat = 0;
    retbuf = "";

    struct timeval tv;
    tv.tv_sec = itemcell->ret_Options().ret_Timed_Out();
    tv.tv_usec = 0;

    string tempLine;
    while (1) {
        char sockBuf[1024];
        list<int> fd_list;
        fd_list.clear();
        fd_list.push_back(itemcell->ret_Desc_r());

        int retval = sock_command.is_readready(&tv, fd_list);
        if (retval && sock_command.is_set(fd_list)) {
            // reset time out value
            tv.tv_sec = itemcell->ret_Options().ret_Timed_Out();
            tv.tv_usec = 0;

            // read bytes from socket
            int size = sock_command.Recv(sockBuf, sizeof(sockBuf) - 1);

            // error handling
            if (size < 0) {
                throw ItemCell::ITEM_ERECV;
            } else if (size == 0) {
                if (tempLine.size() < 5) {
                    itemcell->set_Command(ItemCell::DLERRORSTOP);
                    throw ItemCell::ITEM_EPROT;
                }
                retstat = stoi(tempLine.substr(0, 3));
                retbuf = tempLine.substr(4);
                return 1;
            }

            // put terminate character
            sockBuf[size] = '\0';

            // add the header lines to previous line
            string tempSockBuf = sockBuf;
            tempLine += tempSockBuf;
            unsigned int lineEnd;
            while ((lineEnd = tempLine.find('\r')) != string::npos &&
                    tempLine.find('\n') != string::npos) {
                string headerLine = tempLine.substr(0, lineEnd);
                itemcell->Send_message_to_gui(headerLine, MSG_DOWNLOAD_RECV);
                if (headerLine.size() >= 4
                        && isdigit(headerLine.at(0))
                        && headerLine.at(3) == ' ') {
                    if (headerLine.at(0) == '4' || headerLine.at(0) == '5') {
                        retstat = stoi(headerLine.substr(0, 3));
                        return(-1);
                    } else {
                        retstat = stoi(headerLine.substr(0, 3));
                        retbuf = headerLine.substr(4);
                        return(1);
                    }
                } else {
                    tempLine.erase(0, lineEnd + 2); // erase \r\n
                }
            }
        } else if (retval && Socket::is_set(itemcell->ret_Desc_r(), fd_list)) {
            ItemCommand itemcommand;
            read(itemcell->ret_Desc_r(), &itemcommand, sizeof(ItemCommand));
            itemcell->set_Command(itemcommand);
            if (itemcell->ret_Dl_status() == ItemCell::DLCHANGE) {
                itemcell->Process_command(itemcommand);
                updateInterval(tv, itemcell->ret_Options().ret_Timed_Out());
                continue;
            } else {
                throw ItemCell::ITEM_EINTER;
            }
        } else {
            // timed out
            throw ItemCell::ITEM_ETIMEDOUT;
        }
    }
}

//
// ファイルを落す処理
//
// return:
//     -1: エラー
//      0: ユーザー操作による割り込み
//      1: 成功
//
void RetrieveFTP::Start_Download(const Socket  &sock_data, unsigned int startingbyte)
{
    ofstream outfile;
    string line;
    bool flag_trylock = false;
    Make_directory_if_needed();
    string filename;
    try {
        filename = itemcell->ret_Options().ret_Store_Dir() +
                   //itemcell->ret_URL_Container().ret_File();
                   itemcell->ret_Filename();
        if (!g_lockList->Try_lock(filename)) {
            itemcell->Send_message_to_gui(_("This file is locked. Aborting download"), MSG_DOWNLOAD_ERROR);
            itemcell->set_Command(ItemCell::DLERRORSTOP);
            throw ItemCell::ITEM_EINTER;
        } else {
            flag_trylock = true;
        }

        line = _("Starting download at ") + itos(startingbyte, true) + _(" bytes");

        itemcell->Send_message_to_gui(line, MSG_DOWNLOAD_INFO);

        if (startingbyte == 0) { // 新規にダウンロード
            outfile.open(filename.c_str(), ios::out | ios::trunc | ios::binary);
        } else { // レジュームする
            outfile.open(filename.c_str(), ios::out | ios::app | ios::binary);
        }
        if (outfile.bad()) {
            throw ItemCell::ITEM_EIOFILE;
        }
        itemcell->set_Size_Current(startingbyte);
        // modified 2001/5/20
        itemcell->set_previous_dl_size(Download_data(outfile, sock_data));
        outfile.close();

        if (itemcell->ret_Size_Current() != itemcell->ret_Size_Total()
                && itemcell->ret_Size_Total() != 0) {
            itemcell->Send_message_to_gui(_("No match size"), MSG_DOWNLOAD_ERROR);
            itemcell->set_Command(ItemCell::DLERROR);
            throw ItemCell::ITEM_EPROT;
        }
    } catch (ItemCell::ItemErrorType err) {
        if (flag_trylock) g_lockList->Unlock(filename);
        throw err;
    }
    if (flag_trylock) g_lockList->Unlock(filename);
    // download successful
}
