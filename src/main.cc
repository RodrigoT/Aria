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

// $Id: main.cc,v 1.78 2002/12/18 15:41:05 tujikawa Exp $

#include <iostream>
#include <string>
#include <iomanip>
#include <fstream>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/utsname.h>
#include <signal.h>
#include "getopt.h"
#include "aria.h"

#ifdef HAVE_OPENSSL
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif // HAVE_OPENSSL

#include "ProxyList.h"
#include "ListManager.h"
#include "ItemCell.h"
#include "ItemList.h"
#include "AppOption.h"
#include "ItemOption.h"
#include "FileBrowser.h"
#include "ServerTemplateList.h"
#include "CommandList.h"
#include "CtrlSocket.h"
#include "LockList.h"
#include "HistoryWindow.h"
#include "Basket.h"
using namespace std;

// external functions
extern GtkWidget *GUI_main(int main_width, int main_height, int main_x, int main_y);
extern void *Download_thread_main();
extern void *Set_sensitive__list_not_empty();
extern void Toolbar_set_sensitive__thread();
extern void Toolbar_set_thread_spin(int n_thread);
extern void Set_file_selection_default_path(const string &selected_dir);
extern bool Create_new_item(string url,
                            bool onPaste = false,
                            const string &dir = "",
                            const string &referer = "");
extern void Set_suminfo_label();
extern bool Initialize_resource_files(const string &rcdir,
                                      bool force_flag = false);
extern gboolean Download_start_all_by_listentry(ListEntry *listentry);

extern Basket *basket;

// global variables
FileBrowser *g_cFileBrowser;
Dialog *g_cDialog;
ListManager *g_listManager;
pthread_key_t g_tsdKey;
LockList *g_lockList;
AppOption *g_appOption;
ItemList *g_itemList;
ItemCell *g_consoleItem;
ItemOption *g_itemOption;
ItemManager *g_itemManagerPaste;
HistoryWindow *g_historyWindow;
ProxyList *g_httpProxyList = NULL;
ProxyList *g_ftpProxyList = NULL;
UseragentList *g_userAgentList = NULL;
pthread_mutex_t g_appLock;
pthread_mutex_t g_appLock2;
int g_pipetogui[2];
ServerTemplateList g_servTempList;
CommandList g_commandList;
CtrlSocket g_ctrlSock;
string g_machineInfo;
int g_threadLimit;

pthread_t guiThreadId;

#ifdef HAVE_OPENSSL
pthread_mutex_t *sslMutexArray = NULL;
#endif // HAVE_OPENSSL

void Show_usage()
{
    cerr << _("Usage: aria [OPTION]...") << '\n';
    cerr << _("Yet another download tool") << '\n';
    cerr << '\n';
    // command line options
    cerr << _("Options:") << '\n';
    cerr << _("  -g, --get URL                    URL to get") << '\n';
    cerr << _("  -d, --dir SAVEDIR                save directory") << '\n';
    cerr << _("  -r, --ref REFERER                referer") << '\n';
    cerr << _("  -u, --url-list FILE              open URL list") << '\n';
    cerr << _("  -c, --crc-list FILE              open CRC list") << '\n';
    cerr << _("  --basket                         Show basket and iconify Aria") << '\n';
    cerr << _("  -v, --version                    print version information") << '\n';
    cerr << _("  -h, --help                       print this help") << '\n';
    cerr << _("  --initrc                         initialize resource files") << '\n';
    cerr << _("  --geometry WIDTHxHEIGHT+XOFF+YOFF\n                                   set the size and position of main window") << '\n';
    cerr << _("  --thread-limit-override NUM      if you want more simultaneous downloads...") << '\n';
    cerr << '\n';
    cerr << _("Control Options:\n the following options are effective only when another aria is already running") << '\n';
    cerr << '\n';
    cerr << _("  -s, --start                      start all downloads") << '\n';
    cerr << _("  -p, --stop                       stop all downloads") << '\n';
    cerr << _("  -a, --again                      re-download all items") << '\n';
    cerr << _("  -q, --quit                       quit Aria(with no confirmation)") << '\n';
    cerr << _("  --list-name                      show the name of the current list") << '\n';
    cerr << _("  --list-prev                      switch to the previous list") << '\n';
    cerr << _("  --list-next                      switch to the next list") << '\n';
    cerr << _("  --list-delete                    delete the current list") << '\n';
    cerr << _("  --clear-complete                 clear downloaded items") << '\n';
    cerr << _("  --ls-complete                    show URLs downloaded") << '\n';
    cerr << _("  --ls-ready                       show URLs waiting in queue") << '\n';
    cerr << _("  --ls-error                       show URLs in error") << '\n';
    cerr << _("  --ls-run                         show URLs now downloading") << '\n';
    cerr << _("  --ls-stop                        show URLs stopped") << '\n';
    cerr << _("  --ls-lock                        show URLs locked") << '\n';
    cerr << _("  --ls-all                         show all URLs in queue") << '\n';
}

void Show_version()
{
    cerr << "Aria (version " << VERSION << "; " << D_MACHINE_COMPILED << ")" << '\n';
    cerr << _("Compiled date: ") << D_DATE_COMPILED << '\n';
    cerr << '\n';
    cerr << _("Copyright (C) 2000-2002 Tatsuhiro Tsujikawa") << '\n';
    cerr << '\n';
    cerr << _("The part of directory browser is a modified code, the original code was written by Peter Alm, Mikael Alm, Olle Hallnas, Thomas Nilsson and 4Front Technologies.\n"
              "The original code is copyrighted by Peter Alm, Mikael Alm, Olle Hallnas, Thomas Nilsson and 4Front Technologies.") << "\n\n";
    cerr << _("The part of MD5 checking is a modified code, the original code was written for GnuPG, "
              "copyrighted 1995, 1996, 1998, 1999 Free Software Foundation, Inc.") << "\n\n";
#ifdef HAVE_ZLIB
    cerr << _("This program uses zlib, Copyright (C) 1995-1998 Jean-loup Gailly and Mark Adler") << "\n\n";
#endif // HAVE_ZLIB
#ifdef HAVE_OPENSSL
    cerr << _("This program uses OpenSSL, Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)") << '\n';
#endif // HAVE_OPENSSL
    //cerr << "\nAny suggestions or bug reports: tujikawa@pop16.odn.ne.jp" << endl;
}

void create_default_filter_nodown_target_list(list<string> &filter_target_list)
{
    filter_target_list.push_back(".arc");
    filter_target_list.push_back(".arj");
    filter_target_list.push_back(".avi");
    filter_target_list.push_back(".bin");
    filter_target_list.push_back(".cgi");
    filter_target_list.push_back(".exe");
    filter_target_list.push_back(".deb");
    filter_target_list.push_back(".gz");
    filter_target_list.push_back(".gzip");
    filter_target_list.push_back(".lha");
    filter_target_list.push_back(".lzh");
    filter_target_list.push_back(".lzw");
    filter_target_list.push_back(".mod");
    filter_target_list.push_back(".mpa");
    filter_target_list.push_back(".mpeg");
    filter_target_list.push_back(".mpg");
    filter_target_list.push_back(".qt");
    filter_target_list.push_back(".rar");
    filter_target_list.push_back(".rm");
    filter_target_list.push_back(".rpm");
    filter_target_list.push_back(".snd");
    filter_target_list.push_back(".tar.bz2");
    filter_target_list.push_back(".tar.gz");
    filter_target_list.push_back(".tgz");
    filter_target_list.push_back(".wav");
    filter_target_list.push_back(".zip");
    filter_target_list.push_back(".Z");
    filter_target_list.push_back(".z");
    filter_target_list.push_back(".zoo");

    /*
    filter_target_list.push_back(".html");
    filter_target_list.push_back(".htm");
    filter_target_list.push_back(".jpg");
    filter_target_list.push_back(".jpeg");
    filter_target_list.push_back(".png");
    filter_target_list.push_back(".gif");
    filter_target_list.push_back(".php");
    filter_target_list.push_back(".php3");
    filter_target_list.push_back(".asp");
    */
}

void create_default_parse_target_list(list<string> &parse_target_list)
{
    parse_target_list.push_back(".html");
    parse_target_list.push_back(".htm");
    parse_target_list.push_back(".shtml");
    parse_target_list.push_back(".xhtml");
    parse_target_list.push_back(".css");
    parse_target_list.push_back(".asp");
    parse_target_list.push_back(".php");
    parse_target_list.push_back(".php3");
}

// メッセージを受信
void ctrlsock_getmsg(void *w, int ctrl_fd, GdkInputCondition dummy_cond)
{
    g_ctrlSock.Read_message();
}

void receive_listname(int fd)
{
    char *data;
    int size = g_ctrlSock.Recv_command(fd, (void **)&data);
    if (size < 0) {
        return;
    } else {
        cerr << data << endl;
        delete [] data;
    }
    g_ctrlSock.Send_command(fd, CmdPacketHeader::CMD_PING);
}

void receive_list(int fd)
{
    receive_listname(fd);
    cerr << _("Status (Ready/Error/Downloading/Stop/Complete/Lock)") << '\n';
    cerr << '|' << _("Split?(Yes/No/split Parts)") << '\n';
    cerr << "|/\n";
    cerr << "++-=============================================================================\n";
    int count = 0;
    while (1) {
        char *data;
        int size = g_ctrlSock.Recv_command(fd, (void **)&data);
        if (size < 0) {
            return;
        } else if (size == 0) {
            break;
        } else {
            cout << data << endl;
            delete [] data;
        }
        g_ctrlSock.Send_command(fd, CmdPacketHeader::CMD_PING);
        ++count;
    }
    cerr << '\n' << _("Total ") << count << _(" URLs") << '\n';
}

void show_no_remote_warning()
{
    cerr << _("Another Aria is not running. This option makes no effect.") << endl;
}

#ifdef HAVE_OPENSSL
void ssl_locking_callback(int mode, int type, char *file, int line)
{
    if (mode & CRYPTO_LOCK) {
        pthread_mutex_lock(&(sslMutexArray[type]));
    } else {
        pthread_mutex_unlock(&(sslMutexArray[type]));
    }
}

unsigned long ssl_id_callback()
{
    return (unsigned long)pthread_self();
}
#endif // HAVE_OPENSSL

int main(int argc, char **argv)
{
    int option_index;
    int c;
    string filename;
    string crcfilename;
    string svtfilename;
    int main_width = 0, main_height = 0;
    int main_x = -1, main_y = -1;

    // get gui thread id; this is used in message passing
    guiThreadId = pthread_self();

    static struct option long_options[] = {
        { "get", required_argument, NULL, 'g' },
        { "dir", required_argument, NULL, 'd' },
        { "ref", required_argument, NULL, 'r' },
        { "url-list", required_argument, NULL, 'u' },
        { "crc-list", required_argument, NULL, 'c' },
        { "basket", no_argument, NULL, 'B' },
        { "help", no_argument, NULL, 'h' },
        { "version", no_argument, NULL, 'v' },
        { "initrc", no_argument, NULL, 'i' },
        { "start", no_argument, NULL, 's' },
        { "stop", no_argument, NULL, 'p' },
        { "again", no_argument, NULL, 'a' },
        { "list-name", no_argument, NULL, 'l' },
        { "list-prev", no_argument, NULL, 'j' },
        { "list-next", no_argument, NULL, 'o' },
        { "list-delete", no_argument, NULL, 'f' },
        { "quit", no_argument, NULL, 'q' },
        { "clear-complete", no_argument, NULL, 'm' },
        { "ls-complete", no_argument, NULL, 't' },
        { "ls-error", no_argument, NULL, 'e' },
        { "ls-ready", no_argument, NULL, 'w' },
        { "ls-stop", no_argument, NULL, 'b' },
        { "ls-lock", no_argument, NULL, 'k' },
        { "ls-run", no_argument, NULL, 'x' },
        { "ls-all", no_argument, NULL, 'y' },
        { "thread-limit-override", required_argument, NULL, 'n' },
        { "geometry", required_argument, NULL, 'G'},
        { "pass-pw", no_argument, NULL, 'P' },
        { 0, 0, 0, 0}
    };

    // open pipe
    if ( pipe(g_pipetogui) != 0)
        return 99;
    //cerr << socketpair(PF_UNIX, SOCK_STREAM, 0, g_pipetogui) << endl;

    // process gtk-specific arguments
    g_thread_init(NULL);
# ifdef ENABLE_NLS
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);
# endif
    //gtk_set_locale();
    bool guiReadyFlag = gtk_init_check(&argc, &argv);
#ifdef HAVE_OPENSSL
    SSL_load_error_strings();
    SSL_library_init();
    sslMutexArray = (pthread_mutex_t *)malloc(CRYPTO_num_locks() * sizeof(pthread_mutex_t));
    CRYPTO_set_locking_callback((void ( *)(int, int, const char *, int))ssl_locking_callback);
    CRYPTO_set_id_callback((unsigned long ( *)())ssl_id_callback);
#endif // HAVE_OPENSSL
    // read configuration file: rc.aria
    string home_dir = g_get_home_dir();
    string rc_dir =  home_dir + "/.aria";
    //Make_configuration_directory(rc_dir);
    Initialize_resource_files(rc_dir);
    string rc_filename = rc_dir + "/rc.aria";
    gtk_rc_parse(rc_filename.c_str());
    string url;
    string dir;
    string ref;
    bool showBasketFlag = false;
    bool passPasteWindowFlag = false;

    g_threadLimit = THREADLIMIT;//default thread limit
    while ((c = getopt_long(argc, argv, "g:d:r:u:c:hvspaq", long_options, &option_index)) != EOF) {
        switch (c) {
            case 'u':
                if (!filename.empty()) {
                    cerr << "ERROR: don't specify multitple URL list file" << endl;
                    Show_usage();
                    exit(1);
                }
                filename = optarg;
                if (g_ctrlSock.Is_remote_running()) {
                    if (filename.at(0) != '/') {
                        char *gtmp = g_get_current_dir();
                        string tmp = gtmp;
                        filename = tmp + '/' + filename;
                        g_free(gtmp);

                    }
                    g_ctrlSock.Send_command(CmdPacketHeader::CMD_URLLIST, (void *)filename.c_str(), filename.size() + 1);
                }
                break;
            case 'c':
                if (!crcfilename.empty()) {
                    cerr << "ERROR: don't specify multitple CRC list file" << endl;
                    Show_usage();
                    exit(1);
                }
                crcfilename = optarg;
                if (g_ctrlSock.Is_remote_running()) {
                    if (filename.at(0) != '/') {
                        char *gtmp = g_get_current_dir();
                        string tmp = gtmp;
                        filename = tmp + '/' + filename;
                        g_free(gtmp);
                    }
                    g_ctrlSock.Send_command(CmdPacketHeader::CMD_CRCLIST, (void *)filename.c_str(), filename.size() + 1);
                }
                break;
            case 'i':
                Initialize_resource_files(rc_dir, true);
                break;
            case 's':
                if (g_ctrlSock.Is_remote_running()) {
                    g_ctrlSock.Send_command(CmdPacketHeader::CMD_START);
                } else {
                    show_no_remote_warning();
                }
                break;
            case 'p':
                if (g_ctrlSock.Is_remote_running()) {
                    g_ctrlSock.Send_command(CmdPacketHeader::CMD_STOP);
                } else {
                    show_no_remote_warning();
                }
                break;
            case 'a':
                if (g_ctrlSock.Is_remote_running()) {
                    g_ctrlSock.Send_command(CmdPacketHeader::CMD_AGAIN);
                } else {
                    show_no_remote_warning();
                }
                break;
            case 'q':
                if (g_ctrlSock.Is_remote_running()) {
                    g_ctrlSock.Send_command(CmdPacketHeader::CMD_QUIT);
                } else {
                    show_no_remote_warning();
                }
                break;
            case 'l':
                if (g_ctrlSock.Is_remote_running()) {
                    int fd;
                    if ((fd = g_ctrlSock.connect_to()) > 0) {
                        g_ctrlSock.Send_command(fd, CmdPacketHeader::CMD_LISTNAME);
                        receive_listname(fd);
                        close(fd);
                    }
                } else {
                    show_no_remote_warning();
                }
                break;
            case 'j':
                if (g_ctrlSock.Is_remote_running()) {
                    g_ctrlSock.Send_command(CmdPacketHeader::CMD_PREVLIST);
                } else {
                    show_no_remote_warning();
                }
                break;
            case 'o':
                if (g_ctrlSock.Is_remote_running()) {
                    g_ctrlSock.Send_command(CmdPacketHeader::CMD_NEXTLIST);
                } else {
                    show_no_remote_warning();
                }
                break;
            case 'f':
                if (g_ctrlSock.Is_remote_running()) {
                    g_ctrlSock.Send_command(CmdPacketHeader::CMD_DELLIST);
                } else {
                    show_no_remote_warning();
                }
                break;
            case 'm':
                if (g_ctrlSock.Is_remote_running()) {
                    g_ctrlSock.Send_command(CmdPacketHeader::CMD_DELCOMPLETE);
                } else {
                    show_no_remote_warning();
                }
                break;
            case 't':
                if (g_ctrlSock.Is_remote_running()) {
                    int fd;
                    if ((fd = g_ctrlSock.connect_to()) > 0) {
                        g_ctrlSock.Send_command(fd, CmdPacketHeader::CMD_LSCOMP);
                        receive_list(fd);
                        close(fd);
                    }
                } else {
                    show_no_remote_warning();
                }
                break;
            case 'e':
                if (g_ctrlSock.Is_remote_running()) {
                    int fd;
                    if ((fd = g_ctrlSock.connect_to()) > 0) {
                        g_ctrlSock.Send_command(fd, CmdPacketHeader::CMD_LSFAILED);
                        receive_list(fd);
                        close(fd);
                    }
                } else {
                    show_no_remote_warning();
                }
                break;
            case 'w':
                if (g_ctrlSock.Is_remote_running()) {
                    int fd;
                    if ((fd = g_ctrlSock.connect_to()) > 0) {
                        g_ctrlSock.Send_command(fd, CmdPacketHeader::CMD_LSREADY);
                        receive_list(fd);
                        close(fd);
                    }
                } else {
                    show_no_remote_warning();
                }
                break;
            case 'b':
                if (g_ctrlSock.Is_remote_running()) {
                    int fd;
                    if ((fd = g_ctrlSock.connect_to()) > 0) {
                        g_ctrlSock.Send_command(fd, CmdPacketHeader::CMD_LSSTOP);
                        receive_list(fd);
                        close(fd);
                    }
                } else {
                    show_no_remote_warning();
                }
                break;
            case 'x':
                if (g_ctrlSock.Is_remote_running()) {
                    int fd;
                    if ((fd = g_ctrlSock.connect_to()) > 0) {
                        g_ctrlSock.Send_command(fd, CmdPacketHeader::CMD_LSGO);
                        receive_list(fd);
                        close(fd);
                    }
                } else {
                    show_no_remote_warning();
                }
                break;
            case 'k':
                if (g_ctrlSock.Is_remote_running()) {
                    int fd;
                    if ((fd = g_ctrlSock.connect_to()) > 0) {
                        g_ctrlSock.Send_command(fd, CmdPacketHeader::CMD_LSLOCK);
                        receive_list(fd);
                        close(fd);
                    }
                } else {
                    show_no_remote_warning();
                }
                break;
            case 'y':
                if (g_ctrlSock.Is_remote_running()) {
                    int fd;
                    if ((fd = g_ctrlSock.connect_to()) > 0) {
                        g_ctrlSock.Send_command(fd, CmdPacketHeader::CMD_LSALL);
                        receive_list(fd);
                        close(fd);
                    }
                } else {
                    show_no_remote_warning();
                }
                break;
            case 'n': {
                int new_limit = stoi(optarg);
                if (new_limit > 0 && new_limit <= MAXTHREAD) {
                    g_threadLimit = new_limit;
                } else {
                    cerr << "thread limit exceeded or invalid. valid range is 1 to " << MAXTHREAD << ".\n";
                    cerr << "Abort." << endl;
                    exit(1);
                }
                break;
            }
            case 'h':
                Show_usage();
                exit(0);
            case 'v':
                Show_version();
                exit(0);
            case 'g':
                url = optarg;
                break;
            case 'd':
                dir = optarg;
                break;
            case 'r':
                ref = optarg;
                break;
            case 'G': {
                string arg = optarg;
                //	    try {
                unsigned int loc_pos;
                unsigned int locsep_pos;
                if ((loc_pos = arg.find('+')) == string::npos ||
                        loc_pos + 1 == arg.size()) {
                    loc_pos = arg.size();
                } else {
                    if ((locsep_pos = arg.find('+', loc_pos + 1)) == string::npos ||
                            locsep_pos + 1 == arg.size()) {
                        // do nothing
                    } else {
                        int xTemp = stoi(arg.substr(loc_pos + 1, locsep_pos));
                        int yTemp = stoi(arg.substr(locsep_pos + 1));

                        if (xTemp >= 0 && yTemp >= 0) {
                            main_x = xTemp;
                            main_y = yTemp;
                        }
                    }
                }
                unsigned int x_pos;
                if ((x_pos = arg.find('x')) == string::npos ||
                        x_pos + 1 == arg.size() ||
                        loc_pos == x_pos + 1) {
                    // do nothing
                } else {
                    int wTemp = stoi(arg.substr(0, x_pos));
                    int hTemp = stoi(arg.substr(x_pos + 1, loc_pos));
                    if (wTemp > 0 && hTemp > 0) {
                        main_width = wTemp;
                        main_height = hTemp;
                    }
                }
                break;
                //  	    } catch (int err) {
                //  	      cerr << "invalid argument for option \"--geometry\"" << endl;
                //  	      cerr << "Usage: --geometry WIDTHxHEIGHT" << endl;
                //  	      cerr << "       e.g. --geometry 800x600" << endl;
                //  	      exit(1);
                //  	    }
            }
            case 'B':
                showBasketFlag = true;
                break;
            case 'P':
                passPasteWindowFlag = true;
                break;
            case '?':
                exit(0);
                break;
        }
    }
    if (g_ctrlSock.Is_remote_running()) {
        if (url.size()) {
            // connect to remote Aria
            int fd = g_ctrlSock.connect_to();
            // send URL
            if (passPasteWindowFlag) {
                g_ctrlSock.Send_command(fd, CmdPacketHeader::CMD_URL2PW, (void *)url.c_str(), url.size() + 1);
            } else {
                g_ctrlSock.Send_command(fd, CmdPacketHeader::CMD_URL, (void *)url.c_str(), url.size() + 1);
            }
            if (dir.size()) {
                // send SAVEDIR
                try {
                    if (dir.at(0) != '/') {
                        gchar *temp = g_get_current_dir();
                        dir = get_storedir(temp, dir);
                        g_free(temp);
                    }
                    g_ctrlSock.Send_command(fd, CmdPacketHeader::CMD_URL, (void *)dir.c_str(), dir.size() + 1);
                } catch (int err) {
                    cerr << "ERROR: invalid save dir" << endl;
                    g_ctrlSock.Send_command(fd, CmdPacketHeader::CMD_URL);
                }
            } else {
                // if no savedir specified, then send ""
                g_ctrlSock.Send_command(fd, CmdPacketHeader::CMD_URL);
            }
            if (ref.size()) {
                // send REFERER
                g_ctrlSock.Send_command(fd, CmdPacketHeader::CMD_URL, (void *)ref.c_str(), ref.size() + 1);
            } else {
                // if no referer specified, then send ""
                g_ctrlSock.Send_command(fd, CmdPacketHeader::CMD_URL);
            }
            // close connection
            close(fd);
        }
        exit(0);
    } else {
        // add input watch handler
        gdk_input_add(g_ctrlSock.ret_ctrl_fd(), GDK_INPUT_READ, ctrlsock_getmsg, NULL);
    }
    // added 2001/6/2
    if (!guiReadyFlag) {
        cerr << "ERROR: GTK initialization failed" << endl;
        exit(1);
    }
    // get uname
    struct utsname *uname_st_ptr = new struct utsname;
    if (uname(uname_st_ptr) < 0) {
        g_machineInfo = D_MACHINE_COMPILED;
    } else {
        char *temp = g_strconcat(uname_st_ptr->sysname, " ", uname_st_ptr->release, " ", uname_st_ptr->machine, NULL);
        g_machineInfo = temp;
        g_free(temp);
    }
    delete uname_st_ptr;

    // initialize pthread related variables
    pthread_mutex_init(&g_appLock, NULL);
    pthread_mutex_init(&g_appLock2, NULL);
    pthread_key_create(&g_tsdKey, NULL);

    // set random seeds
    srandom(time(NULL));
    // ignore the signal SIGPIPE
    //signal(SIGPIPE, SIG_IGN);
    struct sigaction sigact;
    sigact.sa_handler = SIG_IGN;
    sigact.sa_flags = 0;
    sigemptyset(&sigact.sa_mask);
    sigaction(SIGPIPE, &sigact, NULL);

    // create global objects
    g_itemList = new ItemList();

    GtkWidget *appWindow = GUI_main(main_width, main_height, main_x, main_y);

    g_lockList = new LockList();
    g_itemManagerPaste = new ItemManager();
    g_itemOption = new ItemOption(appWindow);
    g_userAgentList = new UseragentList();

    // patch from an Aria user
    g_userAgentList->add("Mozilla/4.0 (compatible)");

    char *originalUserAgent = g_strconcat("Aria/", VERSION, " (", g_machineInfo.c_str(), ")", NULL);
    g_userAgentList->add(originalUserAgent);
    g_free(originalUserAgent);
    g_userAgentList->Read_useragent_list(g_itemList->ret_file_useragent_list());
    g_itemOption->set_useragent_list(g_userAgentList);

    g_consoleItem = new ItemCell();
    g_consoleItem->set_logging(false);
    g_consoleItem->set_Status(ItemCell::ITEM_CONSOLE);

    g_httpProxyList = new ProxyList();
    g_httpProxyList->Read_proxy_list(g_itemList->ret_file_http_proxy_list());
    g_itemOption->set_http_proxy_list(g_httpProxyList);

    g_ftpProxyList = new ProxyList();
    g_ftpProxyList->Read_proxy_list(g_itemList->ret_file_ftp_proxy_list());
    g_itemOption->set_ftp_proxy_list(g_ftpProxyList);

    g_appOption = new AppOption(appWindow);

    // read server config file
    g_consoleItem->Send_message_to_gui(_("Reading Server Template file..."),
                                       MSG_SYS_INFO);
    if (svtfilename.empty()) svtfilename = g_itemList->ret_file_server_settings();
    if (g_servTempList.Read_config_file(svtfilename)) {
        g_consoleItem->Send_message_to_gui(_("Server Template status : good"),
                                           MSG_SYS_INFO);
    } else {
        g_consoleItem->Send_message_to_gui(_("Server Template status : error"),
                                           MSG_SYS_ERROR);
    }
    // read command list file
    g_consoleItem->Send_message_to_gui(_("Reading Command list file..."),
                                       MSG_SYS_INFO);
    if (g_commandList.Read_from_file(g_itemList->ret_file_command_list())) {
        g_consoleItem->Send_message_to_gui(_("Command list status : good"),
                                           MSG_SYS_INFO);
    } else {
        g_consoleItem->Send_message_to_gui(_("Command list status : error"),
                                           MSG_SYS_ERROR);
    }

    // read default item option
    g_consoleItem->Send_message_to_gui(_("Reading Default item config file..."),
                                       MSG_SYS_INFO);
    if (!g_itemList->Restore_saved_default_item_settings()) {
        g_consoleItem->Send_message_to_gui(_("Default item config status: error >> using default config"), MSG_SYS_INFO);
        char *currentDir = g_get_current_dir();
        Proxyserver proxy;

        Userdata user(DEFAULT_OPTION_USER, DEFAULT_OPTION_PASSWORD);
        Userdata httpProxyUser(DEFAULT_HTTP_PROXY_USER, DEFAULT_HTTP_PROXY_PASSWORD);
        Userdata ftpProxyUser(DEFAULT_FTP_PROXY_USER, DEFAULT_FTP_PROXY_PASSWORD);
        list<string> filterDownTargetList;
        list<string> filterNodownTargetList;
        create_default_filter_nodown_target_list(filterNodownTargetList);
        list<string> parseTargetList;
        create_default_parse_target_list(parseTargetList);
        list<string> ignDomainList;
        list<string> ftpFilterTargetList;
        Command command;
        g_consoleItem->ret_Options_opt().Change_Values(DEFAULT_USE_AUTHENTICATION,
                                                       user,
                                                       currentDir,
                                                       DEFAULT_HTTP_VERSION,
                                                       DEFAULT_PREWRITTEN_HTML_TYPE,
                                                       DEFAULT_PREWRITTEN_HTML_NAME,
                                                       DEFAULT_OPTION_SYNC_WITH_URL_ENABLED,
                                                       DEFAULT_OPTION_HTTP_REFERER_TYPE,
                                                       DEFAULT_OPTION_HTTP_REFERER,
                                                       g_userAgentList->ret_vector().front(),
                                                       DEFAULT_OPTION_HTTP_RANDOM_USER_AGENT_ENABLED,
                                                       DEFAULT_USE_HTTP_PROXY,
                                                       DEFAULT_USE_HTTP_CACHE,
                                                       DEFAULT_USE_HTTP_PROXY_AUTHENTICATION,
                                                       httpProxyUser,
                                                       proxy,
                                                       DEFAULT_OPTION_TIME_OUT,
                                                       DEFAULT_OPTION_SPLIT_NUM,
                                                       DEFAULT_OPTION_ROLLBACK_BYTES,
                                                       DEFAULT_OPTION_SIZE_LOWER_LIMIT_ENABLED,
                                                       DEFAULT_OPTION_SIZE_LOWER_LIMIT,
                                                       DEFAULT_USE_NO_REDOWNLOAD,
                                                       DEFAULT_USE_NO_DOWNLOAD_SAMENAME,
                                                       DEFAULT_OPTION_HTTP_RECURSE_COUNT_COUNT,
                                                       DEFAULT_WITH_HOSTNAME_DIR,
                                                       DEFAULT_ABS2REL,
                                                       DEFAULT_FORCE_CONVERT,
                                                       DEFAULT_DEL_COMMENT,
                                                       DEFAULT_DEL_JAVASCRIPT,
                                                       DEFAULT_DEL_IFRAME,
                                                       DEFAULT_NO_OTHER_HOST,
                                                       DEFAULT_NO_ASCEND,
                                                       DEFAULT_RELATIVE_ONLY,
                                                       DEFAULT_REFERER_OVERRIDE,
                                                       DEFAULT_FOLLOW_FTP_LINK,
                                                       DEFAULT_CONVERT_TILDE,
                                                       DEFAULT_NO_REDOWNLOAD_HTTP_RECURSE,
                                                       DEFAULT_HTTP_RECURSE_ADD_PASTE,
                                                       DEFAULT_TAG_HREF,
                                                       DEFAULT_TAG_SRC,
                                                       DEFAULT_TAG_BACKGROUND,
                                                       DEFAULT_TAG_CODE,
                                                       DEFAULT_USE_DOWN_FILTER,
                                                       filterDownTargetList,
                                                       filterNodownTargetList,
                                                       parseTargetList,
                                                       ignDomainList,
                                                       DEFAULT_FTP_MODE,
                                                       DEFAULT_FTP_RET_MODE,
                                                       DEFAULT_USE_FTP_PROXY,
                                                       DEFAULT_USE_FTP_PROXY_AUTHENTICATION,
                                                       ftpProxyUser,
                                                       proxy,
                                                       DEFAULT_USE_FTP_CACHE,
                                                       DEFAULT_USE_FTP_PROXY_VIA_HTTP,
                                                       DEFAULT_FTP_PROXY_LOGIN_PROC,
                                                       DEFAULT_FTP_NOSEND_QUIT,
                                                       DEFAULT_FTP_NO_CWD,
                                                       DEFAULT_FTP_RECURSE_COUNT,
                                                       DEFAULT_FTP_USE_FILTER,
                                                       DEFAULT_FTP_ALLOW_CRAWL_SUBDIR,
                                                       DEFAULT_FTP_NO_ASCEND,
                                                       DEFAULT_FTP_GET_SYMLINK_AS_REALFILE,
                                                       DEFAULT_FTP_RECURSE_ADD_PASTE,
                                                       ftpFilterTargetList,
                                                       DEFAULT_IFDEL,
                                                       DEFAULT_RETRY,
                                                       DEFAULT_RETRY_REPEAT,
                                                       DEFAULT_OPTION_RETRY_INTERVAL,
                                                       DEFAULT_FORCE_RETRY_404,
                                                       DEFAULT_FORCE_RETRY_503,
                                                       DEFAULT_STATUS_416_HANDLING,
                                                       DEFAULT_USE_NO_REDIRECTION,
                                                       DEFAULT_HTTP_ACCEPT_COMPRESSION,
                                                       DEFAULT_HTTP_ACCEPT_LANG_ENABLED,
                                                       DEFAULT_HTTP_ACCEPT_LANG_STRING,
                                                       DEFAULT_IFCRC,
                                                       DEFAULT_NO_CRC_CHECKING,
                                                       DEFAULT_IGNORE_CRC_ERROR,
                                                       DEFAULT_USE_CONTENT_MD5,
                                                       DEFAULT_COOKIE_DELETE_ON_RESTART,
                                                       DEFAULT_COOKIE_NOSEND,
                                                       DEFAULT_COOKIE_USERDEFINED,
                                                       DEFAULT_COOKIE_USERDEFINED_STRING,
                                                       DEFAULT_DOWNM_TYPE,
                                                       DEFAULT_SPEED_LIMIT,
                                                       DEFAULT_USE_COMMAND,
                                                       DEFAULT_USE_EXIT_STATUS,
                                                       command);
        g_free(currentDir);
        //g_consoleItem->ret_Options_opt().activate_filter_down_target_list(filter_down_target_list);
        g_consoleItem->ret_Options_opt().activate_filter_nodown_target_list(filterNodownTargetList);
        g_consoleItem->ret_Options_opt().activate_parse_target_list(parseTargetList);
    } else {
        g_consoleItem->Send_message_to_gui(_("Default item config status: good"),
                                           MSG_SYS_INFO);
    }

    // read application setting
    g_consoleItem->Send_message_to_gui(_("Reading Application config file..."), MSG_SYS_INFO);
    // download history window
    g_historyWindow = new HistoryWindow(GTK_WINDOW(appWindow), g_appOption->ret_history_limit());
    if (!g_itemList->Restore_saved_app_settings()) {
        g_consoleItem->Send_message_to_gui(_("Application config status: error >> using default config"), MSG_SYS_INFO);
        //g_consoleItem->Send_message_to_gui(_("This error can be safely ignored if this is the first time you run Aria"), MSG_SYS_INFO);
        list<string> ignoreList;
        ignoreList.push_back("html");
        ignoreList.push_back("cgi");
        ignoreList.push_back("pl");
        list<string> dummy;
        TimerData timerData;
        g_appOption->Set_Option_Values(1, true, true, false, false, false, ignoreList, false, 10, DEFAULTHISTORY, DEFAULTSPEEDLIMIT, false, false, "", false, false, false, false, false, timerData, true, true, true, true, true, false, true, ""/*DEFAULT_ICONSET*/, ""/*DEFAULT_BASKET_PIXMAP*/, true, dummy, dummy);
        g_historyWindow->setHistoryMax(DEFAULTHISTORY);
    } else {
        g_consoleItem->Send_message_to_gui(_("Application config status: good"), MSG_SYS_INFO);
    }
    // read history list
    g_historyWindow->readFile(g_itemList->ret_file_history());

    // read URL file specified in command line
    if (!filename.empty()) {
        if (!g_itemList->Read_URL_from_file(NULL, filename)) {
            g_consoleItem->Send_message_to_gui(_("URL file read error"), MSG_SYS_ERROR);
        } else {
            g_consoleItem->Send_message_to_gui(_("URL file opened"), MSG_SYS_INFO);
        }
    } else {
        if (g_itemList->Restore_saved_list()) {
            g_consoleItem->Send_message_to_gui(_("Saved file opened"), MSG_SYS_INFO);
        }
    }

    // create tab if there is no tab at this time
    if (!g_listManager->ret_Length()) {
        ListEntry *listEntry = new ListEntry(g_appOption->getThreadMax(),
                                             g_consoleItem->ret_Options_opt());
        g_listManager->Register(listEntry);
    }

    ListEntry *listEntry = g_listManager->ret_Current_listentry();//nth_listentry(0);
    if (listEntry->getRowCount() > 0) {
        Set_sensitive__list_not_empty();// fix this
    }
    Toolbar_set_thread_spin(listEntry->getThreadLimit());

    // this directory is used as the default directory of file selection widgets
    g_cFileBrowser->set_default_path(g_consoleItem->ret_Options_opt().ret_Store_Dir());

    /// crc file

    if (!crcfilename.empty()) {
        if (!g_itemList->Read_CRC_from_file(listEntry, crcfilename)) {
            g_consoleItem->Send_message_to_gui(_("CRC file read error"), MSG_SYS_ERROR);
        } else {
            g_consoleItem->Send_message_to_gui(_("CRC file opened"), MSG_SYS_INFO);
        }
    }

    // create version information
    string versionInfo;
    versionInfo = versionInfo + "Aria (version " + VERSION + "; " + g_machineInfo + ") " + _("Ready");
    g_consoleItem->Send_message_to_gui(versionInfo.c_str(), MSG_SYS_INFO);

    // add URL specified in command line
    if (url.size()) {
        if (dir.size() && dir.at(0) != '/') {
            gchar *temp = g_get_current_dir();
            dir = get_storedir(temp, dir);
            g_free(temp);
        }
        Create_new_item(url, false, dir, ref);
    }

    if (showBasketFlag) {
        basket->show();
        Display *display = GDK_WINDOW_XDISPLAY(appWindow->window);
        XIconifyWindow(display, GDK_WINDOW_XWINDOW(appWindow->window), DefaultScreen (display));
        basket->setMainWindowVisibleFlag(false);
    }
    if (g_appOption->Whether_use_automatic_start()) {
        if (g_appOption->isAutostartCurrentListOnly()) {
            ListEntry *listEntry = g_listManager->ret_Current_listentry();
            listEntry->get_Dl_clist_lock();
            listEntry->freezeDlCList();
            Download_start_all_by_listentry(listEntry);
            //listEntry->Send_start_signal();
            listEntry->thawDlCList();
            listEntry->release_Dl_clist_lock();
        } else {
            for (list<ListEntry *>::const_iterator itr = g_listManager->ret_Listentry_list().begin(); itr != g_listManager->ret_Listentry_list().end(); ++itr) {
                ListEntry *listEntry = *itr;
                listEntry->get_Dl_clist_lock();
                listEntry->freezeDlCList();
                Download_start_all_by_listentry(listEntry);
                //listEntry->Send_start_signal();
                listEntry->thawDlCList();
                listEntry->release_Dl_clist_lock();
            }
        }
    }

    Set_suminfo_label();

    gdk_threads_enter();
    gtk_main();
    gdk_threads_leave();

    exit(0);
}
