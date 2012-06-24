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

// $Id: ItemList.h,v 1.43 2002/03/16 14:13:00 tujikawa Exp $

//definition of class ItemList

#ifndef _ITEMLIST_H_
#define _ITEMLIST_H_
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
//#include <map>
#include <utility>
#include "aria.h"
#include "utils.h"
#include "ProxyList.h"
#include "ItemCell.h"
#include "ItemManager.h"
#include "ItemStatusDynamic.h"
#include "ListEntry.h"
#include "HTMLparse.h"
#include "PasteWindow.h"
#include "AppOption.h"
#include "CRCList.h"
using namespace std;

// File Format
#define FI_TAB_NAME "%Tab-Name:"
#define FI_TAB_THREAD_LIMIT "%Tab-Thread-Limit:"
#define FI_TAB_ACTIVE "%Tab-Active:"

#define FI_URL "%URL:"
#define FI_FILENAME "%Filename:"
#define FI_STATUS "%Status:"
#define FI_SIZE_CURRENT "%Current-Size:"
#define FI_SIZE_TOTAL "%Total-Size:"
#define FI_CRC_TYPE "%CRC-Type:"
#define FI_CRC "%CRC:"
#define FI_MD5 "%MD5:"
#define FI_PREVDLSIZE "%Previous-Downloaded-Size:"
#define FI_USE_AUTHENTICATION "%Use_Authentication:"
#define FI_USER "%User:"
#define FI_PASSWORD "%Password:"
#define FI_STORE_DIRECTORY "%Store-Directory:"
#define FI_HTTP_VERSION "%HTTP-Version:"
#define FI_PREWRITTEN_HTML_TYPE "%Prewritten-HTML-Type:"
#define FI_PREWRITTEN_HTML_NAME "%Prewritten-HTML-Name:"
#define FI_SYNC_WITH_URL "%Sync-With-URL:"
#define FI_REFERER_TYPE "%Referer-Type:"
#define FI_REFERER_STRING "%Referer-String:"
#define FI_USERAGENT "%USERAGENT:"
#define FI_RANDOM_USERAGENT "%RANDOM-USERAGENT:"
#define FI_USE_HTTP_PROXY_AUTHENTICATION "%Use-HTTP-Proxy-Authentication:"
#define FI_HTTP_PROXY_USER "%HTTP-Proxy-User:"
#define FI_HTTP_PROXY_PASSWORD "%HTTP-Proxy-Password:"
#define FI_USE_HTTP_PROXY "%Use-HTTP-Proxy:"
#define FI_USE_HTTP_CACHE "%Use-HTTP-Cache:"
#define FI_HTTP_PROXY_SERVER "%HTTP-Proxy-Server:"
#define FI_HTTP_PROXY_PORT "%HTTP-Proxy-Port:"

#define FI_USE_FTP_PROXY_AUTHENTICATION "%Use-FTP-Proxy-Authentication:"
#define FI_FTP_PROXY_USER "%FTP-Proxy-User:"
#define FI_FTP_PROXY_PASSWORD "%FTP-Proxy-Password:"
#define FI_USE_FTP_PROXY "%Use-FTP-Proxy:"
#define FI_USE_FTP_CACHE "%Use-FTP-Cache:"
#define FI_FTP_PROXY_SERVER "%FTP-Proxy-Server:"
#define FI_FTP_PROXY_PORT "%FTP-Proxy-Port:"
#define FI_USE_FTP_PROXY_VIA_HTTP "%Use-FTP-Proxy-Via-HTTP:"
#define FI_FTP_PROXY_LOGIN_PROC "%FTP-Proxy-Login-Proc:"

#define FI_TIMED_OUT "%Timed-Out:"
#define FI_HOW_MANY_PARTS "%How_many_parts:"
#define FI_ROLLBACK_BYTES "%Rollback_bytes:"
#define FI_USE_SIZE_LOWER_LIMIT "%Use-Size-Lower-Limit:"
#define FI_SIZE_LOWER_LIMIT "%Size-Lower-Limit:"
#define FI_USE_NO_REDOWNLOAD "%Use-No-Redownload:"
#define FI_USE_NO_DOWNLOAD_SAMENAME "%Use-No-Download-SameName:"
#define FI_FTP_MODE "%FTP-Mode:"
#define FI_FTP_RET_MODE "%FTP-Retrieve-Mode:"
#define FI_FTP_NOSEND_QUIT "%FTP-Dont-Send-QUIT:"
#define FI_FTP_NO_CWD "%FTP-NO-CWD:"
#define FI_FTP_RECURSE_COUNT "%FTP-Recurse-Count:"
#define FI_FTP_USE_FILTER "%FTP-Use-Filter:"
#define FI_FTP_ALLOW_CRAWL_SUBDIR "%FTP-Allow-Crawl-Subdir:"
#define FI_FTP_NO_ASCEND "%FTP-No-Ascend:"
#define FI_FTP_GET_SYMLINK_AS_REALFILE "%FTP-Get-Symlink-As-RealFile:"
#define FI_FTP_RECURSE_ADD_PASTE "%FTP-Recurse-Add-Paste:"
#define FI_FTP_FILTER_TARGET_LIST "%FTP-Filter-Target-List:"
#define FI_FTP_ACTIVATED_FILTER_TARGET_LIST "%FTP-Activated-Filter-Target-List:"
#define FI_RETRY "%Retry:"
#define FI_RETRY_REPEAT "%Retry-Repeat:"
#define FI_RETRY_INTERVAL "%Retry-Interval:"
#define FI_FORCE_RETRY_404 "%Force-Retry-404:"
#define FI_FORCE_RETRY_503 "%Force-Retry-503:"
#define FI_STATUS_416_HANDLING "%Status-416-Handling:"
#define FI_USE_NO_REDIRECTION "%Use-No-Redirection:"
#define FI_DELETE_WHEN_COMPLETED "%Delete_when_finish:"
#define FI_DONT_DELETE_WITHOUT_CRC "%Dont_delete_without_CRC:"
#define FI_NO_CRC_CHECKING "%No-CRC-Checking:"
#define FI_IGNORE_CRC_ERROR "%Ignore-CRC-Error:"
#define FI_USE_CONTENT_MD5 "%Use-Content-MD5:"
#define FI_COOKIE_DELETE_ON_RESTART "%Cookie_delete_on_restart:"
#define FI_COOKIE_NOSEND "%Cookie_nosend:"
#define FI_COOKIE_USERDEFINED "%Cookie-User-Defined:"
#define FI_COOKIE_USERDEFINED_STRING "%Cookie-User-Defined-String:"
#define FI_DOWNM_TYPE "%Download-Method-Type:"
#define FI_SPEED_LIMIT "%Speed-Limit:"
#define FI_RECURSE_COUNT "%Recurse-Count:"
#define FI_DOCUMENTROOT_DIR "%DocumentRoot-Dir:"
#define FI_ROOT_URL "%Root-URL:"
#define FI_USE_DOWN_FILTER "%Use-Down-Filter:"
#define FI_FILTER_DOWN_TARGET_LIST "%Allow-Down-Target-List:"
#define FI_ACTIVATED_FILTER_DOWN_TARGET_LIST "%Activated-Allow-Down-Target-List:"
#define FI_FILTER_NODOWN_TARGET_LIST "%Down-Target-List:"
#define FI_ACTIVATED_FILTER_NODOWN_TARGET_LIST "%Activated-Down-Target-List:"
#define FI_PARSE_TARGET_LIST "%Parse-Target-List:"
#define FI_ACTIVATED_PARSE_TARGET_LIST "%Activated-Parse-Target-List:"
#define FI_IGN_DOMAIN_LIST "%Ignore-Domain-List:"
#define FI_ACTIVATED_IGN_DOMAIN_LIST "%Activated-Domain-List:"
#define FI_WITH_HOSTNAME_DIR "%With-Hostname-Dir:"
#define FI_ABS2REL "%Convert-ABS2REL-URL:"
#define FI_FORCE_CONVERT "%Force-Convert:"
#define FI_DEL_COMMENT "%Delete-Comment:"
#define FI_DEL_JAVASCRIPT "%Delete-JavaScript:"
#define FI_DEL_IFRAME "%Delete-Iframe:"
#define FI_NO_OTHER_HOST "%No-Other-Host:"
#define FI_ONLY_RELATIVE_LINKS "%Only-Relative-Links:"
#define FI_REFERER_OVERRIDE "%Referer-Override:"
#define FI_FOLLOW_FTP_LINK "%Follow-FTP-Link:"
#define FI_CONVERT_TILDE "%Convert-Tilde:"
#define FI_NO_REDOWNLOAD_HTTP_RECURSE "%No-Redownload-HTTP-Recurse:"
#define FI_HTTP_RECURSE_ADD_PASTE "%HTTP-Recurse-Add-Paste:"
#define FI_NO_ASCEND "%No-Ascend:"
#define FI_USE_TAG_HREF "%Use-Tag-Href:"
#define FI_USE_TAG_SRC "%Use-Tag-Src:"
#define FI_USE_TAG_BACKGROUND "%Use-Tag-Background:"
#define FI_USE_TAG_CODE "%Use-Tag-Code:"
#define FI_USE_COMMAND "%Use-Command:"
#define FI_USE_EXIT_STATUS "%Use-Exit-Status:"
#define FI_COMMAND_STRING "%Command-String:"
#define FI_EXIT_STATUS_STRING "%Exit-Status-String:"
#define FI_HTTP_ACCEPT_COMPRESSION "%HTTP-Accept-Compression:"
#define FI_HTTP_ACCEPT_LANG_ENABLED "%HTTP-Accept-Lang-Enabled:"
#define FI_HTTP_ACCEPT_LANG_STRING "%HTTP-Accept-Lang-String:"

class ItemList {
private:
  string file_app_settings;         // default file name for app settings
  string file_server_settings;
  string file_default_item_settings;// default file name for default item
  string file_default_item_list;    // default file name for item list
  string file_http_proxy_list;
  string file_ftp_proxy_list;
  string file_useragent_list;
  string file_command_list;
  string file_history;
  string file_gui_info;
public:
  ItemList();

  const string& ret_file_http_proxy_list() const;
  const string& ret_file_ftp_proxy_list() const;
  const string& ret_file_useragent_list() const;
  const string& ret_file_server_settings() const;
  const string& ret_file_command_list() const;
  const string& ret_file_history() const;
  const string& ret_file_gui_info() const;
  bool Read_CRC_from_file(ListEntry *listentry, const string& filename);
  bool Read_md5_from_file(ListEntry *listentry, const string& filename);
  bool Read_URL_from_file(ListEntry *listentry, const string& filename);
  bool Find_Hyperlink_from_file(const string& filename, const string& base_url, int mode);
  list<ItemCell*> Recursive_add_http_item(ItemCell *itemcell, ListEntry *listentry);
  bool Restore_saved_list(const string& filename);
  bool Restore_saved_list();
  bool Restore_saved_default_item_settings();
  bool Restore_saved_app_settings();
  bool Restore_default_item_option_sub(ifstream& infile, Options& default_options);
  bool Save_default_item_option_sub(ofstream& outfile, const Options& default_options);
  bool Save_current_list(const string& filename);
  bool Save_current_list();
  bool Save_default_item_settings();
  bool Save_app_settings();

  static const int FINDHREF_PASTE = 0;
  static const int FINDHREF_ADD = 1;
  static const int STRICT_HREF = 2;
}; 

#endif // _ITEMLIST_H_
