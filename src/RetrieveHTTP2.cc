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

// $Id: RetrieveHTTP2.cc,v 1.9 2002/04/06 22:13:06 tujikawa Exp $

#include "RetrieveHTTP2.h"
#include "LockList.h"
extern LockList *g_lockList;

RetrieveHTTP2::RetrieveHTTP2(ItemCellPartial *itemcell_in)
    : RetrieveHTTP(itemcell_in)
{
    itemcell = itemcell_in;
}

RetrieveHTTP2::~RetrieveHTTP2()
{
    // nothing to do here
}

ItemCell::DownloadStatusType RetrieveHTTP2::Download_Main()
{
    unsigned int startingbyte = 0;
    string report;

    Socket socket(-1, Socket::DEFAULT);
    try {
        if (itemcell->ret_Status() != ItemCell::ITEM_CRCERROR &&
                itemcell->ret_Status() != ItemCell::ITEM_EXECERROR &&
                itemcell->ret_Status() != ItemCell::ITEM_DOWNLOAD_AGAIN &&
                itemcell->ret_Options().ret_downm_type() != Options::DOWNM_NORESUME) {
            startingbyte = Get_starting_byte();
        }
        itemcell->set_Status(ItemCell::ITEM_DOWNLOAD);
        itemcell->Send_status();

        // connect to web server
//      if(socket.bad()) {
//        throw ItemCell::ITEM_EIO;
//      }
        establishConnection(socket);
        /*
        if(itemcell->ret_URL_Container().ret_Protocol() == "ftp:") {
          Make_TCP_connection(socket,
        		  itemcell->ret_Options().ret_ftp_proxy().ret_Server(),
        		  itemcell->ret_Options().ret_ftp_proxy().ret_Port());
        } else if(itemcell->ret_Options().ret_use_http_proxy() &&
           !itemcell->ret_Options().ret_http_proxy().ret_Server().empty()) {
          Make_TCP_connection(socket,
        		  itemcell->ret_Options().ret_http_proxy().ret_Server(),
        		  itemcell->ret_Options().ret_http_proxy().ret_Port());
        } else {
          Make_TCP_connection(socket,
        		  itemcell->ret_URL_Container().ret_Hostname(),
        		  itemcell->ret_URL_Container().ret_Port());
        }
        */
        //Make_TCP_connection(socket, ret_URL_Container().ret_Port());

        // send GET request to www server]
        unsigned int startingbyte_real = startingbyte + itemcell->ret_Start_range();
        Send_Request(socket, startingbyte_real);

        // get HTTP header
        HTTPHeaderList httpheaderlist;
        Get_HTTP_header(socket, httpheaderlist);

        // analyze HTTP header and store it to class HTTPcontainer
        HTTPcontainer httpcon;

        httpcon.Parse_HTTP_header(httpheaderlist);
        switch (httpcon.ret_HTTP_Status()) {
            case MultipleChoices:
            case MovedPermanently:
            case Found:
            case SeeOther: {
                // these codes should not be executed
                URLcontainer urlcon_new;
                string location = URLcontainer::URL_Decode(httpcon.ret_Location());
                if (!urlcon_new.Parse_URL(location)) {
                    location += '/';
                    if (!urlcon_new.Parse_URL(location)) {
                        throw ItemCell::ITEM_ELOCATION;
                    }
                }
                itemcell->set_URL_Container(urlcon_new);

                report = _("Redirecting to") + httpcon.ret_Location();
                itemcell->Send_message_to_gui(report, MSG_DOWNLOAD_INFO);

                return ItemCell::DLAGAIN;
            }
        }

        // at this point, httpheaderlist is no longer exists

        if (startingbyte + itemcell->ret_Start_range() == itemcell->ret_End_range() &&
                startingbyte != 0) {
            //すでにダウンロード済
            // downloading has been competed
            report = "'" + itemcell->ret_Filename() + _("' is already downloaded");
            itemcell->Send_message_to_gui(report, MSG_DOWNLOAD_INFO);

            itemcell->set_Size_Total(startingbyte);
            itemcell->set_Size_Current(startingbyte);
            itemcell->Send_status();
            //Send_message_to_gui(_("connection closed"), MSG_DOWNLOAD_INFO);
            throw ItemCell::ITEM_ESUCCESSALR;
        } else if (startingbyte + itemcell->ret_Start_range() > itemcell->ret_End_range() &&
                   httpcon.ret_HTTP_Status() != PartialContent) {
            // ファイルサイズが異なる
            // size mismatch
            itemcell->Send_message_to_gui(_("Size of local file is larger than remote file's one. Download again"), MSG_DOWNLOAD_ERROR);
            startingbyte = 0;
            itemcell->set_Size_Total(itemcell->ret_End_range() - itemcell->ret_Start_range());
        } else if (itemcell->ret_Start_range() + startingbyte > 0 && httpcon.ret_HTTP_Status() == OK) {
            // レジューム不可
            // www server does not support resume
            itemcell->Send_message_to_gui(_("Sorry, cannot resume"), MSG_DOWNLOAD_INFO);
            startingbyte = 0;
            itemcell->set_Size_Total(itemcell->ret_End_range() - itemcell->ret_Start_range());
            itemcell->set_Command(ItemCell::DLERRORSTOP);
            throw ItemCell::ITEM_EINTER;
        } else {
            itemcell->set_Size_Total(itemcell->ret_End_range() - itemcell->ret_Start_range());
        }
        // start downloading
        Start_Download(socket, startingbyte);

        //Send_message_to_gui(_("connection closed"), MSG_DOWNLOAD_INFO);
        throw ItemCell::ITEM_ESUCCESS;
    } catch (ItemCell::ItemErrorType err) {
        switch (err) {
            case ItemCell::ITEM_ESUCCESS:
                itemcell->set_Command(ItemCell::DLSUCCESS);
                return ItemCell::DLSUCCESS;
            case ItemCell::ITEM_ESUCCESSALR:
                socket.Shutdown(2);
                itemcell->Send_message_to_gui(_("Connection closed"), MSG_DOWNLOAD_INFO);
                itemcell->set_Command(ItemCell::DLSUCCESSALR);
                return ItemCell::DLSUCCESSALR;
            case ItemCell::ITEM_EINTER:
                switch (itemcell->ret_Dl_status()) {
                    case ItemCell::DLAGAIN:
                    case ItemCell::DLSTOP:
                    case ItemCell::DLERRORSTOP:
                    case ItemCell::DLDELETEITEM:
                    case ItemCell::DLDELETEITEMFILE:
                        socket.Shutdown(2);
                        itemcell->Send_message_to_gui(_("Connection closed"), MSG_DOWNLOAD_INFO);
                        break;
                    case ItemCell::DLHALT:
                    default:
                        socket.Shutdown(2);
                        break;
                }
                return itemcell->ret_Command();
            case ItemCell::ITEM_EIOFILE:
                socket.Shutdown(2);
                itemcell->Send_message_to_gui(_("Connection closed"), MSG_DOWNLOAD_INFO);
                itemcell->set_Command(ItemCell::DLERRORSTOP);
                itemcell->PERROR(err);
                return ItemCell::DLERRORSTOP;
            default:
                itemcell->PERROR(err);
                itemcell->set_Command(ItemCell::DLERROR);
                socket.Shutdown(2);
                itemcell->Send_message_to_gui(_("Connection closed"), MSG_DOWNLOAD_INFO);
                return ItemCell::DLERROR;
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
void RetrieveHTTP2::Start_Download(const Socket &socket, unsigned int startingbyte)
{
    ofstream outfile;
    string line;
    bool flag_trylock = false;

    Make_directory_if_needed();

    //URLcontainer urlcon_temp = ret_URL_Container();
    //ret_URL_Container().Parse_URL(ret_URL());
    string filename = itemcell->ret_Options().ret_Store_Dir() + itemcell->ret_Filename();
    itemcell->Send_status();

    switch (itemcell->SplitNumberChanged(filename + ".index")) {
        case ItemCellPartial::PARTIAL_NORMAL:
            break;
        case ItemCellPartial::PARTIAL_CHANGED:
            //Send_message_to_gui(_("split count has changed"), MSG_DOWNLOAD_INFO);
        default:
            startingbyte = 0;
    }

    try {
        if (!g_lockList->Try_lock(filename)) {
            itemcell->Send_message_to_gui(_("This file is locked. Aborting download"), MSG_DOWNLOAD_ERROR);
            itemcell->set_Command(ItemCell::DLERRORSTOP);
            throw ItemCell::ITEM_EINTER;
        } else {
            flag_trylock = true;
        }

        line = _("A part of '") + itemcell->ret_URL() + "'" + _(", its index is ") + itos(itemcell->ret_Order()) + _(", its range is from ") + itos(itemcell->ret_Start_range(), true) + " to " + itos(itemcell->ret_End_range(), true);
        itemcell->Send_message_to_gui(line, MSG_DOWNLOAD_INFO);

        line = _("Starting download at ") + itos(itemcell->ret_Start_range() + startingbyte, true) + _(" bytes");
        itemcell->Send_message_to_gui(line, MSG_DOWNLOAD_INFO);

        if (startingbyte == 0) {
            // 新規にダウンロード開始
            outfile.open(filename.c_str(), ios::out | ios::trunc | ios::binary);
        } else {
            // レジュームする
            outfile.open(filename.c_str(), ios::out | ios::app | ios::binary);
        }
        if (outfile.bad()) {
            throw ItemCell::ITEM_EIOFILE;
        }

        itemcell->set_Size_Current(startingbyte);
        // modified 2001/5/20
        itemcell->set_previous_dl_size(Download_data(outfile, socket));
        outfile.close();

        if (itemcell->ret_Size_Current() != itemcell->ret_Size_Total()
                && itemcell->ret_Size_Total() != 0) {
            itemcell->Send_message_to_gui(_("No match size"), MSG_DOWNLOAD_ERROR);
            itemcell->set_Command(ItemCell::DLERROR);
            throw ItemCell::ITEM_EPROT;
        }

        //set_URL_Container(urlcon_temp);
    } catch (ItemCell::ItemErrorType err) {
        //set_URL_Container(urlcon_temp);
        if (flag_trylock) g_lockList->Unlock(filename);
        throw err;
    }
    if (flag_trylock) g_lockList->Unlock(filename);
}

/*
ItemCell::DownloadStatusType RetrieveHTTP2::Post_process()
{
  ItemCell::DownloadStatusType retval = ItemCell::DLSUCCESS;

  //pthread_mutex_lock(&itemlistlock); //mod 2001/4/11
  itemcell->ret_Boss()->get_Options_Lock(); //mod 2001/4/11
  itemcell->ret_Boss()->Remove_worker(itemcell);
  if(itemcell->ret_Boss()->No_more_worker()) {
    itemcell->ret_Boss()->set_Status(ItemCell::ITEM_READY_CONCAT);
    itemcell->ret_Boss()->Send_status();
  }
  //pthread_mutex_unlock(&itemlistlock); //mod 2001/4/11
  itemcell->ret_Boss()->release_Options_Lock(); //mod 2001/4/11
  return retval;
}
*/
