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

// $Id: Retrieve.cc,v 1.20 2002/12/18 15:41:05 tujikawa Exp $

#include "Retrieve.h"
#include "ItemCellPartial.h"

#define DEBUG 1
#undef DEBUG

// プログラム全体で共通のロック
extern pthread_mutex_t g_appLock;

Retrieve::Retrieve(ItemCell *itemcell_in)
{
    itemcell = itemcell_in;
}

Retrieve::~Retrieve()
{
}

ItemCell::DownloadStatusType Download_Main()
{
    cerr << "in Retrieve::Download Main" << endl;

    return ItemCell::DLSUCCESS;
}

//
// 分割ダウンロードのための準備
//
// divide個の部分アイテムを作成
// return:
//     false : エラー(分割不可)
//     true : 分割成功
//
bool Retrieve::Create_partial_item_entry(unsigned int divide, unsigned int total_size)
{
    unsigned int downloaded_size = 0;//Get_starting_byte();
    /*
    if(downloaded_size > 0) {
      string old_filename = ret_Options().ret_Store_Dir()+ret_Filename();
      string new_filename = old_filename+".-1";
      if(rename(old_filename.c_str(), new_filename.c_str()) < 0) {
        downloaded_size = 0;
      }
    }
    */
    //else {
    //  struct stat filestat;
    //  string filename = ret_Options().ret_Store_Dir()+ret_Filename()+".-1";
    // if(stat(filename.c_str(), &filestat) >= 0) {
    //   downloaded_size = filestat.st_size;
    // }
    //
    unsigned int unit, unit_reminder;
    while (1) {
        unit = (total_size - downloaded_size) / divide;
        if (unit > 0) {
            unit_reminder = (total_size - downloaded_size) % divide;
            break;
        }
        if (--divide == 1) return false;
    }
    itemcell->begin_split();
    itemcell->ret_Options().set_Divide(divide);
    for (unsigned int i = 0; i < divide; i++) {
        unsigned int start_range = unit * i + downloaded_size;
        unsigned int end_range;
        if (i == divide - 1) {
            end_range = unit * (i + 1) + downloaded_size + unit_reminder;
        } else {
            end_range = unit * (i + 1) + downloaded_size;
        }
        URLcontainer urlcon_partial = itemcell->ret_URL_Container();
        Options options_partial = itemcell->ret_Options();
        options_partial.set_downm_type(Options::DOWNM_ALWAYSRESUME);
        options_partial.set_Divide(1);
        ItemCellPartial *itemcell_partial = new ItemCellPartial(itemcell->ret_URL(),
                                                                urlcon_partial,
                                                                options_partial,
                                                                _("Created"),
                                                                itemcell,
                                                                i,
                                                                start_range,
                                                                end_range);
        itemcell_partial->set_Filename(itemcell->ret_Filename());
        itemcell_partial->set_Filename_opt(itemcell->ret_Filename_opt());
        itemcell_partial->set_Cookie_list(itemcell->ret_Cookie_list());
        itemcell_partial->set_Status(ItemCell::ITEM_READY);

        itemcell->Send_partial(itemcell_partial);
        itemcell->Send_message_to_gui(_("Partial item added"), MSG_DOWNLOAD_INFO);
    }
    itemcell->end_split();
    return true;
}

// returns downloaded bytes
#define PCSPLIT 10
unsigned int Retrieve::Download_data(ofstream &outfile, const Socket &socket, bool compressedFlag)
{
    const int databuf_size_def = 4096;
    int databuf_size;
    itemcell->get_Options_Lock();
    if (databuf_size_def < itemcell->ret_Options_opt().ret_speed_limit() * 1024 / PCSPLIT || itemcell->ret_Options_opt().ret_speed_limit() < 0.1) {
        databuf_size = databuf_size_def;
    } else {
        databuf_size = (int)(itemcell->ret_Options_opt().ret_speed_limit() * 1024) / PCSPLIT;
    }
    itemcell->release_Options_Lock();
    char databuf[databuf_size_def];
    const int writebuf_size = 12288;
    char *writebuf = new char[writebuf_size];
    char *writebuf_tail = writebuf;
    int size;
    string line;
    struct timeval start_time, end_time, initial_time;
    struct timezone tz_dummy;
    unsigned int timedout = itemcell->ret_Options().ret_Timed_Out();
    unsigned int elapsed_size = 0;
    float speed = 0.0;
    float avgSpeed = 0.0;
    list<int> fd_list;
    unsigned int startsize = itemcell->ret_Size_Current();

    gettimeofday(&initial_time, &tz_dummy);
    start_time = initial_time;
    itemcell->Send_status();

    try {
        while (1) {
            fd_list.clear();
            fd_list.push_back(itemcell->ret_Desc_r());
            int retval = socket.is_readready(timedout, fd_list);
            if (retval && Socket::is_set(itemcell->ret_Desc_r(), fd_list)) {
                ItemCommand itemcommand;

                read(itemcell->ret_Desc_r(), &itemcommand, sizeof(ItemCommand));

                //set_Command(Process_command(itemcommand));
                itemcell->set_Command(itemcommand);
                if (itemcell->ret_Dl_status() != ItemCell::DLCHANGE) {
                    outfile.write(writebuf, writebuf_tail - writebuf);
                    throw ItemCell::ITEM_EINTER;
                } else {
                    // change buffer size;
                    if (databuf_size_def < itemcell->ret_Options_opt().ret_speed_limit() * 1024 / PCSPLIT || itemcell->ret_Options_opt().ret_speed_limit() < 0.1 ) {
                        databuf_size = databuf_size_def;
                    } else {
                        //cerr << "bufsize" << (int)(options_opt.ret_speed_limit()*1024)/PCSPLIT << endl;
                        databuf_size = (int)(itemcell->ret_Options_opt().ret_speed_limit() * 1024) / PCSPLIT;
                    }
                }
            } else if (retval && socket.is_set(fd_list)) {
                size = socket.Recv(databuf, databuf_size);//sizeof(databuf));
                if (size < 0) {
                    throw ItemCell::ITEM_EIO;
                }

                itemcell->set_Size_Current(itemcell->ret_Size_Current() + size);
                gettimeofday(&end_time, &tz_dummy);
                elapsed_size += size;
                unsigned long elapsed = (end_time.tv_sec - start_time.tv_sec) * 1000000 + end_time.tv_usec - start_time.tv_usec;

                if ((itemcell->ret_Size_Total() != 0 && itemcell->ret_Size_Current() >= itemcell->ret_Size_Total()) || size == 0) {
                    if (itemcell->Is_Partial()) {
                        unsigned int diff = itemcell->ret_Size_Current() - itemcell->ret_Size_Total();
                        memcpy(writebuf_tail, databuf, size - diff);
                        writebuf_tail += size - diff;
                        itemcell->set_Size_Current(itemcell->ret_Size_Total());
                    } else {
                        memcpy(writebuf_tail, databuf, size);
                        writebuf_tail += size;
                    }
                    itemcell->Send_status(avgSpeed, avgSpeed);
                    outfile.write(writebuf, writebuf_tail - writebuf);

                    writebuf_tail = writebuf;

                    if (outfile.bad() || outfile.fail()) {
                        throw ItemCell::ITEM_EIOFILE;
                    }

                    break;
                } else {

                    if (elapsed > 1000000) {
                        speed = ((elapsed_size / 1024.0) / (elapsed / 1000000.0) + speed) / 2;

                        unsigned long elapsedTotal = (end_time.tv_sec - initial_time.tv_sec) * 1000000 + end_time.tv_usec - initial_time.tv_usec;
                        avgSpeed = ((itemcell->ret_Size_Current() - startsize) / 1024.0 / (elapsedTotal / 1000000.0));
                        itemcell->Send_status(speed, avgSpeed);
                        if (avgSpeed < 0) avgSpeed = speed; // fix this
                        gettimeofday(&start_time, &tz_dummy);
                        elapsed_size = 0;
                    } else {
                        //cerr << elapsed_size << endl;
                        if (itemcell->ret_Options_opt().ret_speed_limit() > 0.0 &&
                                elapsed_size >= (unsigned int)(itemcell->ret_Options_opt().ret_speed_limit() * 1024 * 0.8)) {
                            struct timeval tv;
                            tv.tv_sec = 0;
                            tv.tv_usec = 1000000 - elapsed;
                            select(itemcell->ret_Desc_r() + 1, NULL, NULL, NULL, &tv);
                        }
                    }
                    memcpy(writebuf_tail, databuf, size);
                    writebuf_tail += size;
                    if (writebuf_size - (writebuf_tail - writebuf) < databuf_size + databuf_size_def) {
                        outfile.write(writebuf, writebuf_tail - writebuf);
                        writebuf_tail = writebuf;
                    }
                    if (outfile.bad() || outfile.fail()) {
                        throw ItemCell::ITEM_EIOFILE;
                    }
                }

            } else {
                // timed out
                outfile.write(writebuf, writebuf_tail - writebuf); // buffer flush
                throw ItemCell::ITEM_ETIMEDOUT;
            }
        }
    } catch (ItemCell::ItemErrorType err) {
        delete [] writebuf;
        // added 2001/5/20
        itemcell->set_previous_dl_size(itemcell->ret_Size_Current() - startsize);
        throw err;
    }
    delete [] writebuf;
    return itemcell->ret_Size_Current() - startsize;
}

// this function is originary appeared in SOCKET PROGRAMMING FAQ
int Retrieve::inet_hostaddr(const string &hostname, struct in_addr *addr)
{
    struct hostent *host;

    //#ifdef HAVE_INET_ATON
    if (inet_aton(hostname.c_str(), addr)) {
        //#else
        //if(inet_pton(AF_INET,hostname.c_str(),addr) > 0) {
        //#endif
        return 1;
    }

    host = gethostbyname(hostname.c_str());
    if (!host) {
        return 0;
    }
    *addr = *(struct in_addr *)(host->h_addr_list[0]);
    return 1;
}

void Retrieve::Make_TCP_connection(Socket &socket, int port_in)
{
    Make_TCP_connection(socket, itemcell->ret_URL_Container().ret_Hostname(), port_in);
}

void Retrieve::Make_TCP_connection(Socket &socket, const string &server, int port)
{
    Make_TCP_connection(socket, server, port, "", -1);
}

void Retrieve::Make_TCP_connection(Socket &socket, const string &server, int port, const string &redirectedServer, int redirectedPort)
{
    if (socket.tryGetPooledSocket(server, port, redirectedServer, redirectedPort) > 0) {
        return;
    }
    socket.setHostPort("", -1);
    socket.setRedirectedHostPort("", -1);

    string hostname = server;
    string report = _("Connecting to ") + hostname + ":" + itos(port);
    itemcell->Send_message_to_gui(report, MSG_DOWNLOAD_INFO);

#ifdef INET6
    /* IPv6 coding style */
    struct addrinfo hints;
    struct addrinfo *res;
    string portStr = itos(port);

    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_NUMERICHOST;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int error = getaddrinfo(hostname.c_str(), portStr.c_str(), &hints, &res);
    if (error) {
        hints.ai_flags = AI_CANONNAME;
        error = getaddrinfo(hostname.c_str(), portStr.c_str(), &hints, &res);
    }
    if (error != 0) {
        report = _("Can't resolve host: ") + hostname + _(", reason: ") + gai_strerror(error);
        itemcell->Send_message_to_gui(report, MSG_DOWNLOAD_ERROR);
        throw ItemCell::ITEM_ECANTRESOLVE;
    }
    if (hints.ai_flags == AI_CANONNAME) {
        report = _("Resolving complete");
        itemcell->Send_message_to_gui(report, MSG_DOWNLOAD_INFO);
    } else if (0) {

        // get canonical hostname
        char hostnameTemp[1024];
        getnameinfo(res->ai_addr, res->ai_addrlen, hostnameTemp,
                    sizeof(hostnameTemp) - 1, NULL, 0,
                    NI_NAMEREQD);
        hostnameTemp[sizeof(hostnameTemp) - 1] = '\0';
        report = _("Canonical name is ");
        report += hostnameTemp;
        itemcell->Send_message_to_gui(report, MSG_DOWNLOAD_INFO);
        hostname = hostnameTemp;
        itemcell->ret_URL_Container().set_Hostname(hostname);
    }
#else
    /* IPv4 coding style */
    sockaddr_in serv_addr;

    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_family = AF_INET;
    if (!inet_hostaddr(hostname, &serv_addr.sin_addr) ||
            !serv_addr.sin_port) {
        throw ItemCell::ITEM_ECANTRESOLVE;
    }
#endif

#ifdef INET6
    struct addrinfo *res0 = res;
    bool connected = false;
    int flags = 0;
    try {
        while (!connected && res) {
            if (socket.create(res->ai_family, res->ai_socktype, res->ai_protocol) < 0) {
                res = res->ai_next;
            }
            // set the socket nonblocking mode
            flags = socket.Getflags();
            socket.Setflags(flags | O_NONBLOCK);

            if (socket.bad()) {
                socket.Setflags(flags&~O_NONBLOCK);
                res = res->ai_next;
                if (res == NULL) break;
                else {
                    continue;
                }
            }
            int retval = socket.Connect(res);

            if (retval < 0 && errno != EINPROGRESS) {
                socket.Setflags(flags&~O_NONBLOCK);
                res = res->ai_next;
                if (res == NULL) break;
                else
                    continue;
            }

#else // not INET6
    // set the socket nonblocking mode
    socket.create(AF_INET, SOCK_STREAM, 0);
    int flags = socket.Getflags();
    socket.Setflags(flags | O_NONBLOCK);
    // connect to server
    int retval = socket.Connect(serv_addr);
    if (retval < 0 && errno != EINPROGRESS) {
        throw ItemCell::ITEM_ECONNREFUSED;
    }
#endif
            /*
                if(retval < 0 && errno != EINPROGRESS) {
            socket.Setflags(flags&~O_NONBLOCK);
            res = res->ai_next;
            if(res == NULL) break;
            else
              continue;
                }
            */
            struct timeval tv;
            tv.tv_sec = itemcell->ret_Options().ret_Timed_Out();
            tv.tv_usec = 0;

            bool loopFlag = true;
            while (loopFlag) {
                loopFlag = false;

                list<int> rfd_list;
                list<int> wfd_list;
                rfd_list.push_back(itemcell->ret_Desc_r());

                retval = socket.is_readwriteready(&tv, rfd_list, wfd_list);
                if (retval && Socket::is_set(itemcell->ret_Desc_r(), rfd_list)) {
                    ItemCommand itemcommand;
                    read(itemcell->ret_Desc_r(), &itemcommand, sizeof(ItemCommand));
                    itemcell->set_Command(itemcommand);
                    if (itemcell->ret_Dl_status() == ItemCell::DLCHANGE) {
                        loopFlag = true;
                        itemcell->Process_command(itemcommand);

                        updateInterval(tv, itemcell->ret_Options().ret_Timed_Out());
                    } else {
                        throw ItemCell::ITEM_EINTER;
                    }
                }
            }
#ifdef INET6
            if (retval > 0) {
            } else if (retval == 0) { // timed out
                throw ItemCell::ITEM_ETIMEDOUT;
            } else {     // Connection Refused
                res = res->ai_next;
                if (res == NULL) break;
                else continue;
                //throw ItemCell::ITEM_ECONNREFUSED;
            }

            if (socket.check_error()) {
                socket.Setflags(flags&~O_NONBLOCK);
                res = res->ai_next;
                if (res == NULL) break;
                else continue;
                //throw ItemCell::ITEM_ECONNREFUSED;
            }

            connected = true;
        }

        if (!connected)
        {
            report = _("Connection refused: ") + hostname;
            itemcell->Send_message_to_gui(report, MSG_DOWNLOAD_ERROR);
            throw ItemCell::ITEM_ECONNREFUSED;
        } else
        {
            freeaddrinfo(res0);
        }
    } catch (ItemCell::ItemErrorType err)
    {
        freeaddrinfo(res0);
        throw err;
    }
#else // not INET6
            if (retval > 0)
            {
            } else if (retval == 0)  // timed out
            {
                throw ItemCell::ITEM_ETIMEDOUT;
            } else       // Connection Refused
            {
                throw ItemCell::ITEM_ECONNREFUSED;
            }

            if (socket.check_error())
            {
                throw ItemCell::ITEM_ECONNREFUSED;
            }
#endif
    // set the socket blocking mode
    socket.Setflags(flags&~O_NONBLOCK);

    socket.setHostPort(server, port);
    socket.setRedirectedHostPort(redirectedServer, redirectedPort);

    itemcell->Send_message_to_gui(_("Connection established"), MSG_DOWNLOAD_INFO);
}

void Retrieve::SEND(const string &command, const Socket &socket)
{
    list<int> fd_list;

    int retval = socket.is_writeready(0, fd_list);
    if (retval && socket.is_set(fd_list)) {
        // send GET request to WWW server
        if (socket.Send(command) < 0) {
            // failed to send request
            throw ItemCell::ITEM_ESEND;
        } else {
            // send successful;
        }
    } else {
        // cannot send data because of error
        throw ItemCell::ITEM_ESEND;
    }
}

unsigned int Retrieve::Get_starting_byte()
{
    struct stat filestat;

    string filename;
    //if(documentroot_dir == options.ret_Store_Dir() && options.ret_recurse_count() > 1 && options.ret_with_hostname_dir()) {
    //filename = options.ret_Store_Dir()+urlcon.ret_Hostname()+urlcon.ret_Dir()+'/'+ret_Filename();
    // } else {
    filename = itemcell->ret_Options().ret_Store_Dir() + itemcell->ret_Filename();

    //}
    // rollback featrue added 2001/3/17
    int rollback = 0;
    if (itemcell->ret_previous_dl_size() > 0 &&
            (unsigned int)itemcell->ret_Options().ret_Rollback_bytes() > itemcell->ret_previous_dl_size()) {
        rollback = itemcell->ret_previous_dl_size();
    } else {
        rollback = itemcell->ret_Options().ret_Rollback_bytes();
    }

    if (stat(filename.c_str(), &filestat) < 0 || !S_ISREG(filestat.st_mode) ||
            filestat.st_size <= rollback) {
        return 0;
    } else {
        if (truncate(filename.c_str(), filestat.st_size - rollback) < 0) {
            // fix this
            itemcell->Send_message_to_gui(_("Unable to truncate the file '") + filename + _("'. Rollback disabled"), MSG_DOWNLOAD_ERROR);

            return filestat.st_size;
        } else {
            return filestat.st_size - rollback;
        }
    }
}

void Retrieve::Make_directory_if_needed()
{
    pthread_mutex_lock(&g_appLock);
    struct stat dir_stat;
    if (stat(itemcell->ret_Options().ret_Store_Dir().c_str(), &dir_stat) == -1 ||
            !S_ISDIR(dir_stat.st_mode)) {
        string temp_storedir = itemcell->ret_Options().ret_Store_Dir();

        if (temp_storedir.at(temp_storedir.size() - 1) == '/') {
            temp_storedir.erase(temp_storedir.size() - 1);
        }
        string dir_to_make = Token_splitter(temp_storedir, "/");

        while (temp_storedir.size()) {
            dir_to_make += '/' + Token_splitter(temp_storedir, "/");
            struct stat dir_stat;
            if (stat(dir_to_make.c_str(), &dir_stat) == -1 || !S_ISDIR(dir_stat.st_mode)) {
                if (mkdir(dir_to_make.c_str(), 0755) < 0) {
                    pthread_mutex_unlock(&g_appLock);
                    itemcell->Send_message_to_gui(_("Failed to make directory '") + dir_to_make + "'\nreason:" + strerror(errno), MSG_DOWNLOAD_ERROR);
                    itemcell->set_Errno(ItemCell::ITEM_EIOFILE);
                    throw ItemCell::ITEM_EIOFILE;
                }
            }
        }
    }
    pthread_mutex_unlock(&g_appLock);
}

bool Retrieve::Is_older_than_remote(time_t modtime)
{
    string filename = itemcell->ret_Options().ret_Store_Dir() + itemcell->ret_Filename();
    struct stat filestat;
    //cerr << filename << endl;
    if (stat(filename.c_str(), &filestat) < 0 || !S_ISREG(filestat.st_mode)) {
        return false;
    } else if (filestat.st_mtime > modtime) return false;
    else return true;
}

ItemCell::DownloadStatusType Retrieve::Download_Main()
{
    return ItemCell::DLSUCCESS;
}

