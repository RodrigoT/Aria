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

// $Id: ItemCell.h,v 1.35 2002/04/03 13:33:51 tujikawa Exp $

//definition of class ItemCell

#ifndef _ITEMCELL_H_
#define _ITEMCELL_H_
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include <string>
#include <fstream>
#include <list>
#include "aria.h"
#include "URLcontainer.h"
#include "HTTPcontainer.h"
#include "Options.h"
#include "Socket.h"
#include "ItemLogCell.h"
#include "ServerTemplate.h"
#include "CookieList.h"

using namespace std;

// 各スレッドがGUIスレッドに送るメッセージのフォーマット
class StatusReport {
public:
  MessageType reporttype;
  void *data;
  pthread_mutex_t* mutex_p;
  pthread_cond_t* cond_p;
};

// 各スレッドに送るコマンド
class ItemCommand {
public:
// signal to threads
  enum ThreadCommandType {
    COMMAND_STOP,
    COMMAND_START,
    COMMAND_REMOVE,
    COMMAND_RESTART,
    COMMAND_DELETE_ITEM,
    COMMAND_DELETE_ITEM_FILE,
    COMMAND_HALT,
    COMMAND_DOWNLOAD_AGAIN,
    COMMAND_CHANGE_SPEED
  };

  // event caused by
  enum EventCause {
    EV_USERINTER, // user interaction
    EV_APPINTER, // event caused by application: quit program
    EV_TIMERINTER, // event caused by timer
    EV_INTERNAL, // no external event
    EV_NOEVENT // no event
  };

  ThreadCommandType commandtype;
  EventCause eventtype;
  float value;
};

typedef list<ItemLogCell> ItemLogList;
//typedef list<ItemCell*> ItemCellPtrList;

class ItemCell {
public:
  // download status
  enum DownloadStatusType {
    DLERROR,
    DLSUCCESS,
    DLSUCCESSALR,
    DLPARTIALSUCCESS,
    DLAGAIN,
    DLINTERNALAGAIN,
    DLSTOP,
    DLHALT,
    DLERRORSTOP,
    DLDELETEITEM,
    DLDELETEITEMFILE,
    DLCHANGE
  };

  // item status
  enum ItemStatusType {
    ITEM_READY,
    ITEM_READY_AGAIN,
    ITEM_READY_CONCAT,
    ITEM_CRCERROR,
    ITEM_STOP,
    ITEM_LOCK,
    ITEM_DOWNLOAD, 
    ITEM_ERROR,
    ITEM_COMPLETE,
    ITEM_CONSOLE,
    ITEM_DOWNLOAD_PARTIAL,
    ITEM_INUSE,
    ITEM_INUSE_AGAIN,
    ITEM_INUSE_CONCAT,
    ITEM_DOWNLOAD_AGAIN,
    ITEM_DOWNLOAD_INTERNAL_AGAIN,
    ITEM_EXECERROR
  };

  enum ItemErrorType {
    ITEM_ENONE,
    ITEM_ECANTRESOLVE,
    ITEM_ESOCKET,
    ITEM_ECONNREFUSED,
    ITEM_ETIMEDOUT,
    ITEM_EIO,
    ITEM_EIOFILE,
    ITEM_ELOCATION,
    ITEM_ESERVERCONFIG,
    ITEM_ECANTFINDURL,
    ITEM_EFTPCOM,
    ITEM_EBIND,
    ITEM_ELISTEN,
    ITEM_EACCEPT,
    ITEM_ESEND,
    ITEM_ERECV,
    ITEM_EINTER,
    ITEM_EPROT,
    ITEM_ENEXTSTAGE,
    ITEM_ESUCCESS,
    ITEM_ESUCCESSALR,
  };

  // CRC type
  enum CRC_Type {
    CRC_16,
    CRC_32,
    CRC_NONE
  };
private:
  int id;
  // URL文字列
  // URL string
  string url;
  // CRC
  unsigned int crc;
  CRC_Type crc_type;

  // MD5
  string md5str;

  string filename;
  string filename_opt;
  // 現在の状態
  // current status
  ItemStatusType status;
  // ダウンロード済ファイルサイズ
  // size of downloaded file
  unsigned int currentsize;
  // ファイルサイズ
  // total size of file
  unsigned int totalsize;

  unsigned int prevDlSize;
  // コマンド受け付け用パイプ
  // pipe for receiving user command
  int msgpipe[2];
  int count;
  // URLcontainerクラスへのポインタ
  // pointer to URLcontainer class
  URLcontainer urlcon;
  // Optionクラスへのポインタ
  // pointer to Option class
  Options options;
  // ログメッセージのリスト
  // list of log messages
  ItemLogList loglist;
  // 受け取ったコマンド
  // received user command
  DownloadStatusType dl_status;
  ItemCommand::EventCause dl_event;
  
  // エラータイプ
  // type of error
  ItemErrorType item_errno;
  // ログメッセージのロック
  // pthread lock for log messages
  pthread_mutex_t itemloglock;
  // 
  pthread_mutex_t optionslock;
  // this lock is necessary because same variable is used in option screen
  // and crc calculation
  pthread_mutex_t crclock;

  // general mutex(internal use only)
  pthread_mutex_t itemlock;

  // 分割されたアイテムのエントリリスト
  // entry list of divided item
  list<ItemCell*> worker_list;
  bool split_complete;

  // 必要な時点でそれぞれurlcon_ptr, options_ptr, urlに反映される
  URLcontainer urlcon_opt;
  Options options_opt;
  string root_url;
  string documentroot_dir;
  string url_opt;
  bool flag_opt_updated;
  bool log_flag;

  // for HTTP download
  int session_counter;
  URLcontainer retrieved_urlcon;
  ServerTemplate svt;
  CookieList cookie_list;
public:
  // constructor
  ItemCell(const string& url,
	   const URLcontainer& urlcon,
	   const Options& options,
	   const string& initial_log
	   );
  ItemCell();
  // destructor
  virtual ~ItemCell();

  // ユーザーの操作を処理する
  // deal with user interaction
  DownloadStatusType Process_command(ItemCommand itemcommand);

  // ダウンロード処理.
  virtual DownloadStatusType Download_Main();

  virtual DownloadStatusType Post_process();

  bool Execute_program();

  // ダウンロードするファイルの名前を返す
  virtual string ret_Filename() const;
  virtual string ret_Filename_opt() const;
 
  void set_Filename(const string& filename);
  void set_Filename_opt(const string& filename);

  virtual bool Is_Partial() const;

  // ダウンロード後の処理
  //virtual DownloadStatusType Post_process();

  int ret_Session_counter();
  void Inc_Session_counter();
  void Reset_Session_counter();
  URLcontainer& ret_Retrieved_urlcon();
  void set_Retrieved_urlcon(const URLcontainer& urlcon);
  ServerTemplate& ret_svt();

  Session& ret_current_session();
  bool Is_current_session_valid();

  void set_svt(const ServerTemplate& svt);
  CookieList& ret_Cookie_list();
  void set_Cookie_list(const CookieList& cookie_list_in);

  //void Show_option_window(); // オプション設定ウインドウを表示する

  // GUIにログメッセージを送る
  // send log messages to GUI manager
  void Send_message_to_gui(const string& message, MessageType reporttype);

  // GUIにitemcell_partialをダウンロードリストに追加要請
  void Send_partial(ItemCell* itemcell_partial);

  // GUIの情報を更新
  void Send_status();
  // 速度付き
  void Send_status(float speed, float avgSpeed);
  // ダウンロード完了時
  void Send_status_complete();

  void Send_status_recursive(const list<ItemCell *>& item_list);

  // エラー出力
  void PERROR(ItemErrorType err);

  // ログItemLogCellをログ管理リストに追加
  void Append_itemlog(const ItemLogCell& itemlogcell);
  void set_logging(bool flag);
  // GUIからの操作受け付け用パイプのオープンとクローズ
  void Open_Desc();
  void Close_Desc();

  // 分割ダウンロード時の部分アイテムをworker_listに追加
  void Append_worker(ItemCell* itemcell_partial);

  // 分割ダウンロード時の部分アイテムをworker_listから削除
  void Remove_worker(ItemCell* itemcell_partial);
  void Clear_worker();
  // worker_listが空がどうか
  bool No_more_worker() const;
  bool Concatenate_partial_files();

  // 更新されたオプションを適用
  virtual void Apply_new_options();
  void Raise_option_update_flag();
  bool Is_option_updated();
  void begin_split();
  void end_split();

  CRC_Type                  ret_CRC_Type() const;
  unsigned int              ret_CRC() const;
  const string&             ret_URL() const;
  const string&             ret_URL_opt() const;
  ItemStatusType            ret_Status();
  unsigned int              ret_Size_Current() const;
  unsigned int              ret_Size_Total() const;

  void get_Log_Lock();
  void release_Log_Lock();

  void get_Options_Lock();
  void release_Options_Lock();

  void get_CRC_Lock();
  void release_CRC_Lock();

  int                       ret_Desc_w() const;
  int                       ret_Desc_r() const;
  URLcontainer&             ret_URL_Container();
  URLcontainer&             ret_URL_Container_opt();
  Options&                  ret_Options();
  Options&                  ret_Options_opt();
  const ItemLogList&        ret_Log_list() const;
  void                      Clear_log();
  ItemErrorType             ret_Errno() const;
  const std::list<ItemCell*>&    ret_Worker_list() const;
  DownloadStatusType        ret_Command() const;
  DownloadStatusType        ret_Dl_status() const;
  ItemCommand::EventCause   ret_Dl_event() const;
  const string&             ret_root_url() const;
  void                      set_root_url(const string& root_url);
  const string& ret_documentroot_dir() const;
  void set_documentroot_dir(const string& root_dir_in);
  void reset_Count();
  void inc_Count();
  int ret_Count() const;
  void set_CRC_Type(CRC_Type crc_type);
  void set_CRC(unsigned int crc);
  void set_Status(ItemStatusType status);
  void set_Size_Current(unsigned int size);
  void set_Size_Total(unsigned int size);
  void set_URL_Container(const URLcontainer& urlcon);
  void set_URL_Container_opt(const URLcontainer& urlcon);
  void set_Options(const Options& options_in);
  void set_Options_opt(const Options& options_in);
  void set_URL(const string& url);
  void set_URL_opt(const string& url);
  void set_Errno(ItemErrorType item_errno);
  void set_Command(DownloadStatusType command_num);
  void set_Command(const ItemCommand& command);
  void set_id(int id);
  int ret_id() const;
  void set_previous_dl_size(unsigned int size);
  unsigned int ret_previous_dl_size();
  string ret_md5string();
  void set_md5string(const string& md5string_new);
};

#endif // _ITEMCELL_H_
