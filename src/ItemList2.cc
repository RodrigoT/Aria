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

// $Id: ItemList2.cc,v 1.20 2002/10/01 15:32:00 tujikawa Exp $

// implementaion of class ItemList

#include "ItemList.h"
#include "SumInfo.h"
#include "UseragentList.h"
#include "HistoryWindow.h"
#include "StringHash.h"

#define stoi(ARG) strtol(ARG.c_str(), (char **)NULL, 10)

extern void Add_new_item_to_downloadlist(ItemCell *itemcell, ListEntry *listentry);
extern void create_default_filter_nodown_target_list(list<string>& filter_target_list);
extern void create_default_parse_target_list(list<string>& parse_target_list);
extern void Track_enabled(bool flag);
extern void Adjust_speed_scale(int max);

extern ItemCell *g_consoleItem;
extern AppOption *g_appOption;
extern ListManager *g_listManager;
extern PasteWindow *g_pasteWindow;
extern int g_threadLimit;
extern SumInfo g_summaryInfo;
extern HistoryWindow *g_historyWindow;

void parse_extension_target_list(list<string>& target_list, string source_string)
{
  while(source_string.size()) {
    string target = Token_splitter(source_string, " \t");
    if(target.size()) {
      target_list.push_back(target);
    }
  }
}

void parse_extension_target_list2(list<string>& target_list, string source_string)
{
  while(source_string.size()) {
    string target = Token_splitter(source_string, "\t");
    if(target.size()) {
      target_list.push_back(target);
    }
  }
}

bool ItemList::Restore_default_item_option_sub(ifstream& infile, Options& default_option)
{
  StringHash default_map;
  while(1) {
    string item;
    string line;
    getline(infile, line, '\n');
    item = Token_splitter(line, " \t");
    if(item == "%End_default" || !infile.good()) {
      unsigned int use_authentication = stoi(default_map.get(FI_USE_AUTHENTICATION));
      Userdata user(default_map.get(FI_USER), default_map.get(FI_PASSWORD));
      string storedir = default_map.get(FI_STORE_DIRECTORY);
      Options::RefererType referer_type = (Options::RefererType)stoi(default_map.get(FI_REFERER_TYPE));
      string http_version = default_map.get(FI_HTTP_VERSION);
      Options::PrewrittenHTMLType prewrittenHTMLType = (Options::PrewrittenHTMLType)stoi(default_map.get(FI_PREWRITTEN_HTML_TYPE));
      string prewrittenHTMLName = default_map.get(FI_PREWRITTEN_HTML_NAME);
      bool sync_with_url = stoi(default_map.get(FI_SYNC_WITH_URL));
      string referer_string = default_map.get(FI_REFERER_STRING);
      string useragent = default_map.get(FI_USERAGENT);
      unsigned int random_useragent = stoi(default_map.get(FI_RANDOM_USERAGENT));
      unsigned int cookie_delete_on_restart = stoi(default_map.get(FI_COOKIE_DELETE_ON_RESTART));
      unsigned int cookie_nosend = stoi(default_map.get(FI_COOKIE_NOSEND));
      unsigned int cookieUserDefined = stoi(default_map.get(FI_COOKIE_USERDEFINED));
      string cookieUserDefinedString = default_map.get(FI_COOKIE_USERDEFINED_STRING);

      int use_http_proxy = stoi(default_map.get(FI_USE_HTTP_PROXY));
      int use_http_cache = stoi(default_map.get(FI_USE_HTTP_CACHE));
      unsigned int use_http_proxy_authentication = stoi(default_map.get(FI_USE_HTTP_PROXY_AUTHENTICATION));      
      Userdata http_proxy_user(default_map.get(FI_HTTP_PROXY_USER),
			       default_map.get(FI_HTTP_PROXY_PASSWORD));
      Proxyserver http_proxyserver(default_map.get(FI_HTTP_PROXY_SERVER), stoi(default_map.get(FI_HTTP_PROXY_PORT)));

      // FTP proxy
      int use_ftp_proxy = stoi(default_map.get(FI_USE_FTP_PROXY));
      unsigned int use_ftp_proxy_authentication = stoi(default_map.get(FI_USE_FTP_PROXY_AUTHENTICATION));      
      Userdata ftp_proxy_user(default_map.get(FI_FTP_PROXY_USER),
			       default_map.get(FI_FTP_PROXY_PASSWORD));
      Proxyserver ftp_proxyserver(default_map.get(FI_FTP_PROXY_SERVER), stoi(default_map.get(FI_FTP_PROXY_PORT)));
      bool use_ftp_cache = (bool)stoi(default_map.get(FI_USE_FTP_CACHE));
      bool use_ftp_proxy_via_http = (bool)stoi(default_map.get(FI_USE_FTP_PROXY_VIA_HTTP));
      Options::FTPproxyLoginProcType ftp_proxy_login_proc =
	(Options::FTPproxyLoginProcType)stoi(default_map.get(FI_FTP_PROXY_LOGIN_PROC));


      int timedout = stoi(default_map.get(FI_TIMED_OUT));
      int retry = stoi(default_map.get(FI_RETRY));
      int retry_repeat = stoi(default_map.get(FI_RETRY_REPEAT));
      int retry_interval;
      if(default_map.find(FI_RETRY_INTERVAL)) {
	retry_interval = stoi(default_map.get(FI_RETRY_INTERVAL));
      } else {
	retry_interval = DEFAULT_OPTION_RETRY_INTERVAL;
      }
      bool use_retry_404 = (bool)stoi(default_map.get(FI_FORCE_RETRY_404));
      bool use_retry_503 = (bool)stoi(default_map.get(FI_FORCE_RETRY_503));
      Options::Status416HandlingType s416Handling = (Options::Status416HandlingType)stoi(default_map.get(FI_STATUS_416_HANDLING));
      bool use_no_redirection = (bool)stoi(default_map.get(FI_USE_NO_REDIRECTION));
      bool httpAcceptCompression = DEFAULT_HTTP_ACCEPT_COMPRESSION;
      if(default_map.find(FI_HTTP_ACCEPT_COMPRESSION)) {
	httpAcceptCompression = (bool)stoi(default_map.get(FI_HTTP_ACCEPT_COMPRESSION));
      }
      bool httpAcceptLangEnabled = stoi(default_map.get(FI_HTTP_ACCEPT_LANG_ENABLED));
      string httpAcceptLangString = default_map.get(FI_HTTP_ACCEPT_LANG_STRING);

      int divide = stoi(default_map.get(FI_HOW_MANY_PARTS));
      int rollback_bytes = stoi(default_map.get(FI_ROLLBACK_BYTES));
      bool use_size_lower_limit = (bool)stoi(default_map.get(FI_USE_SIZE_LOWER_LIMIT));
      int size_lower_limit = stoi(default_map.get(FI_SIZE_LOWER_LIMIT));
      bool use_no_redownload = stoi(default_map.get(FI_USE_NO_REDOWNLOAD));
      bool use_no_download_samename = stoi(default_map.get(FI_USE_NO_DOWNLOAD_SAMENAME));

      int recurse_count = stoi(default_map.get(FI_RECURSE_COUNT));

      list<string> filter_down_target_list;
      list<string> activated_filter_down_target_list;
      if(default_map.find(FI_FILTER_DOWN_TARGET_LIST)) {
	parse_extension_target_list2(filter_down_target_list, default_map.get(FI_FILTER_DOWN_TARGET_LIST));
	parse_extension_target_list2(activated_filter_down_target_list, default_map.get(FI_ACTIVATED_FILTER_DOWN_TARGET_LIST));
      }
      bool use_down_filter = (bool)stoi(default_map.get(FI_USE_DOWN_FILTER));

      list<string> filter_nodown_target_list;
      list<string> activated_filter_nodown_target_list;
      if(default_map.find(FI_FILTER_NODOWN_TARGET_LIST)) {
	parse_extension_target_list(filter_nodown_target_list, default_map.get(FI_FILTER_NODOWN_TARGET_LIST));
	parse_extension_target_list(activated_filter_nodown_target_list, default_map.get(FI_ACTIVATED_FILTER_NODOWN_TARGET_LIST));
      } else {
	create_default_filter_nodown_target_list(filter_nodown_target_list);
	activated_filter_nodown_target_list = filter_nodown_target_list;
      }

      list<string> parse_target_list;
      list<string> activated_parse_target_list;
      if(default_map.find(FI_PARSE_TARGET_LIST)) {
	parse_extension_target_list(parse_target_list, default_map.get(FI_PARSE_TARGET_LIST));
	parse_extension_target_list(activated_parse_target_list, default_map.get(FI_ACTIVATED_PARSE_TARGET_LIST));
      } else {
	create_default_parse_target_list(parse_target_list);
	activated_parse_target_list = parse_target_list;
      }
      // various options for recursive download
      bool recurse_with_hostname_dir = true;
      if(default_map.find(FI_WITH_HOSTNAME_DIR)) {
	recurse_with_hostname_dir = (bool)stoi(default_map.get(FI_WITH_HOSTNAME_DIR));
      }
      bool recurse_abs2rel = (bool)stoi(default_map.get(FI_ABS2REL));
      bool recurse_force_convert = (bool)stoi(default_map.get(FI_FORCE_CONVERT));
      bool recurse_del_comment = (bool)stoi(default_map.get(FI_DEL_COMMENT));
      bool recurse_del_javascript = (bool)stoi(default_map.get(FI_DEL_JAVASCRIPT));
      bool recurse_del_iframe = (bool)stoi(default_map.get(FI_DEL_IFRAME));
      bool recurse_no_other_host = (bool)stoi(default_map.get(FI_NO_OTHER_HOST));
      bool recurse_no_ascend = (bool)stoi(default_map.get(FI_NO_ASCEND));
      bool recurse_relative_only = (bool)stoi(default_map.get(FI_ONLY_RELATIVE_LINKS));
      bool recurse_referer_override = (bool)stoi(default_map.get(FI_REFERER_OVERRIDE));
      bool recurse_follow_ftp_link = (bool)stoi(default_map.get(FI_FOLLOW_FTP_LINK));
      bool recurse_convert_tilde = (bool)stoi(default_map.get(FI_CONVERT_TILDE));
      bool recurse_no_redownload = (bool)stoi(default_map.get(FI_NO_REDOWNLOAD_HTTP_RECURSE));
      bool recurse_add_paste = (bool)stoi(default_map.get(FI_HTTP_RECURSE_ADD_PASTE));

      bool use_tag_href = true;
      if(default_map.find(FI_USE_TAG_HREF)) {
	use_tag_href = (bool)stoi(default_map.get(FI_USE_TAG_HREF));
      }
      bool use_tag_src = true;
      if(default_map.find(FI_USE_TAG_SRC)) {
	use_tag_src = (bool)stoi(default_map.get(FI_USE_TAG_SRC));
      }
      bool use_tag_background = true;
      if(default_map.find(FI_USE_TAG_BACKGROUND)) {
	use_tag_background = (bool)stoi(default_map.get(FI_USE_TAG_BACKGROUND));
      }
      bool use_tag_code = true;
      if(default_map.find(FI_USE_TAG_CODE)) {
	use_tag_code = (bool)stoi(default_map.get(FI_USE_TAG_CODE));
      }

      list<string> ign_domain_list, activated_ign_domain_list;
      parse_extension_target_list(ign_domain_list, default_map.get(FI_IGN_DOMAIN_LIST));
      parse_extension_target_list(activated_ign_domain_list, default_map.get(FI_ACTIVATED_IGN_DOMAIN_LIST));

      Options::DownloadMethodType downm_type = (Options::DownloadMethodType)stoi(default_map.get(FI_DOWNM_TYPE));
      //int speed_limit = stoi(default_map.get(FI_SPEED_LIMIT));
      //cerr << default_map.get(FI_SPEED_LIMIT).c_str() << endl;
      float speed_limit = strtod(default_map.get(FI_SPEED_LIMIT).c_str(), NULL);
      //cerr << "in file" << speed_limit << endl;
      Options::FTP_Mode ftp_mode = (Options::FTP_Mode)stoi(default_map.get(FI_FTP_MODE));
      Options::FTPretModeType ftp_ret_mode = (Options::FTPretModeType)stoi(default_map.get(FI_FTP_RET_MODE));
      bool ftp_nosend_quit = (bool)stoi(default_map.get(FI_FTP_NOSEND_QUIT));
      bool ftpNoCwd = (bool)stoi(default_map.get(FI_FTP_NO_CWD));
      int ftp_recurse_count = 1;
      if(default_map.find(FI_FTP_RECURSE_COUNT)) {
	ftp_recurse_count = stoi(default_map.get(FI_FTP_RECURSE_COUNT));
      }
      bool ftp_use_filter = (bool)stoi(default_map.get(FI_FTP_USE_FILTER));
      bool ftp_allow_crawl_subdir = (bool)stoi(default_map.get(FI_FTP_ALLOW_CRAWL_SUBDIR));
      bool ftp_no_ascend = true;
      if(default_map.find(FI_FTP_NO_ASCEND)) {
	ftp_no_ascend = (bool)stoi(default_map.get(FI_FTP_NO_ASCEND));
      }
      bool ftp_get_symlink_as_realfile = DEFAULT_FTP_GET_SYMLINK_AS_REALFILE;
      if(default_map.find(FI_FTP_GET_SYMLINK_AS_REALFILE)) {
	ftp_get_symlink_as_realfile = (bool)stoi(default_map.get(FI_FTP_GET_SYMLINK_AS_REALFILE));
      }
      bool ftp_recurse_add_paste = (bool)stoi(default_map.get(FI_FTP_RECURSE_ADD_PASTE));

      list<string> ftp_filter_target_list, ftp_activated_filter_target_list;
      parse_extension_target_list2(ftp_filter_target_list, default_map.get(FI_FTP_FILTER_TARGET_LIST));
      parse_extension_target_list2(ftp_activated_filter_target_list, default_map.get(FI_FTP_ACTIVATED_FILTER_TARGET_LIST));

      bool delete_when_finish = (bool)stoi(default_map.get(FI_DELETE_WHEN_COMPLETED));
      bool dont_delete_without_crc = (bool)stoi(default_map.get(FI_DONT_DELETE_WITHOUT_CRC));
      bool no_crc_check = (bool)stoi(default_map.get(FI_NO_CRC_CHECKING));
      bool ignore_crc_error = (bool)stoi(default_map.get(FI_IGNORE_CRC_ERROR));
      bool use_content_md5 = (bool)stoi(default_map.get(FI_USE_CONTENT_MD5));
      bool use_command = (bool)stoi(default_map.get(FI_USE_COMMAND));
      bool use_exit_status = (bool)stoi(default_map.get(FI_USE_EXIT_STATUS));

      string command_string = default_map.get(FI_COMMAND_STRING);
      for(unsigned int i = 0; i < command_string.size(); i++) {
	if(command_string.at(i) == '\r') {
	      command_string.replace(i, 1, "\n");
	}
      }
      Command command(command_string,
		      default_map.get(FI_EXIT_STATUS_STRING));

      // set default item settings

      default_option.Change_Values((bool)use_authentication,
				   user,
				   storedir,
				   http_version,
				   prewrittenHTMLType,
				   prewrittenHTMLName,
				   sync_with_url,
				   referer_type,
				   referer_string,
				   useragent,
				   (bool)random_useragent,
				   (bool)use_http_proxy,
				   (bool)use_http_cache,
				   use_http_proxy_authentication,
				   http_proxy_user,
				   http_proxyserver,
				   timedout,
				   divide,
				   rollback_bytes,
				   use_size_lower_limit,
				   size_lower_limit,
				   use_no_redownload,
				   use_no_download_samename,
				   recurse_count,
				   recurse_with_hostname_dir,
				   recurse_abs2rel,
				   recurse_force_convert,
				   recurse_del_comment,
				   recurse_del_javascript,
				   recurse_del_iframe,
				   recurse_no_other_host,
				   recurse_no_ascend,
				   recurse_relative_only,
				   recurse_referer_override,
				   recurse_follow_ftp_link,
				   recurse_convert_tilde,
				   recurse_no_redownload,
				   recurse_add_paste,
				   use_tag_href,
				   use_tag_src,
				   use_tag_background,
				   use_tag_code,
				   use_down_filter,
				   filter_down_target_list,
				   filter_nodown_target_list,
				   parse_target_list,
				   ign_domain_list,
				   ftp_mode,
				   ftp_ret_mode,
				   (bool)use_ftp_proxy,
				   use_ftp_proxy_authentication,
				   ftp_proxy_user,
				   ftp_proxyserver,
				   use_ftp_cache,
				   use_ftp_proxy_via_http,
				   ftp_proxy_login_proc,
				   ftp_nosend_quit,
				   ftpNoCwd,
				   ftp_recurse_count,
				   ftp_use_filter,
				   ftp_allow_crawl_subdir,
				   ftp_no_ascend,
				   ftp_get_symlink_as_realfile,
				   ftp_recurse_add_paste,
				   ftp_filter_target_list,
				   delete_when_finish,
				   retry,
				   retry_repeat,
				   retry_interval,
				   use_retry_404,
				   use_retry_503,
				   s416Handling,
				   use_no_redirection,
				   httpAcceptCompression,
				   httpAcceptLangEnabled,
				   httpAcceptLangString,
				   dont_delete_without_crc,
				   no_crc_check,
				   ignore_crc_error,
				   use_content_md5,
				   (bool)cookie_delete_on_restart,
				   (bool)cookie_nosend,
				   (bool)cookieUserDefined,
				   cookieUserDefinedString,
				   downm_type,
				   speed_limit,
				   use_command,
				   use_exit_status,
				   command);
      
      default_option.activate_filter_down_target_list(activated_filter_down_target_list);
      default_option.activate_filter_nodown_target_list(activated_filter_nodown_target_list);
      default_option.activate_parse_target_list(activated_parse_target_list);
      default_option.activate_ign_domain_list(activated_ign_domain_list);
      default_option.set_FTP_activated_filter_target_list(ftp_activated_filter_target_list);
      break;
    } else {
      default_map.add(item, Remove_white(line));
    }
  }
  /*
  struct stat dir_stat;
  if(stat(g_consoleItem->ret_Options_opt().ret_Store_Dir().c_str(), &dir_stat) == -1 || !S_ISDIR(dir_stat.st_mode)) {
    string line = _("directory '")+g_consoleItem->ret_Options_opt().ret_Store_Dir()+_("' does not exist. Use your home directory instead");
    g_consoleItem->Send_message_to_gui(line, MSG_SYS_ERROR);
    char *home_dir = g_get_home_dir();
    g_consoleItem->ret_Options_opt().set_Store_Dir(home_dir);
  }
  */
  return true;
}

bool ItemList::Restore_saved_list()
{
  return Restore_saved_list(file_default_item_list);
}

bool ItemList::Restore_saved_list(const string& filename)
{
  char *clist_item[TOTALCOL];
  ifstream infile(filename.c_str(), ios::in);//ios::skipws|ios::in);

  if(infile.bad() || infile.fail()) return false;
  // read magic number
  string header;
  getline(infile, header, '\n');
  if(header != ARIA_VERSION) {
    g_consoleItem->Send_message_to_gui(_("Warning: different version number found in file header"), MSG_SYS_INFO);
    //return false;
  }
  // restore default saved list
  //Restore_default_item_option_sub(infile);
  // restore item list
  while(1) {
    string line;

    getline(infile, line, '\n');

    //item = Token_splitter(line, " \t");
    if(!infile.good()) return true;
    
    bool compat_flag = false;
    if(line != "%Begin_Tab") {
      if(line != "%Begin_Items_Section") return false;
      else compat_flag = true;
    }
    StringHash tab_map;
    ListEntry *listentry;
    
    // Create listentry and register it to g_listManager
    bool active_flag = false;
    if(!compat_flag) {
      string item;
      getline(infile, line, '\n');
      item = Token_splitter(line, " \t");
      if(item == FI_TAB_ACTIVE) {
	active_flag = true;
	getline(infile, line, '\n');
	item = Token_splitter(line, " \t");
      }
      tab_map.add(item, Remove_white(line));
      getline(infile, line, '\n');
      item = Token_splitter(line, " \t");
      tab_map.add(item, Remove_white(line));
      if(header != "Aria - version 0.10.0test1") { // compatibility for v0.10.0test1
    char *current_dir = g_get_current_dir();
	Options options(current_dir);
    g_free(current_dir);
	
	Restore_default_item_option_sub(infile, options);
	
	listentry = new ListEntry(tab_map.get(FI_TAB_NAME),
				  stoi(tab_map.get(FI_TAB_THREAD_LIMIT)),
				  options);
	// read the sizes of each column
	getline(infile, line, '\n');
	item = Token_splitter(line, " \t");
	if(item != "%Begin_Item" && item == "%Columns-Size:") {
	  for(int index = 0; index < TOTALCOL; ++index) {
	    int size = stoi(Token_splitter(line, " \t"));
	    //gtk_clist_set_column_width(GTK_CLIST(listentry->ret_Dl_clist()),
		//		       index, size);
	  }
	} 
      } else {
	listentry = new ListEntry(tab_map.get(FI_TAB_NAME),
				  stoi(tab_map.get(FI_TAB_THREAD_LIMIT)),
				  g_consoleItem->ret_Options_opt());
      }
    } else {
    // backward compatibility (version 0.9.x or earlier)
      listentry = new ListEntry(g_appOption->getThreadMax(),
				g_consoleItem->ret_Options_opt());
    }
    g_listManager->Register(listentry);
    if(active_flag) g_listManager->Set_active_page();

    //gtk_clist_freeze(GTK_CLIST(listentry->ret_Dl_clist()));
    listentry->freezeDlCList();
    while(1) {
      string item;

      getline(infile, line, '\n');

      if(line == "%End_Tab" || line == "%End_Items_Section" || !infile.good()) break;
      StringHash item_map;
      while(1) {
	getline(infile, line, '\n');

	if(!infile.good()) break;// added 2001/9/4

	item = Token_splitter(line, " \t");

	if(item == "%End_Item") {

	  string url = item_map.get(FI_URL);
	  string filename = item_map.get(FI_FILENAME);
	  ItemCell::ItemStatusType status = (ItemCell::ItemStatusType)stoi(item_map.get(FI_STATUS));
	  int size_current = stoi(item_map.get(FI_SIZE_CURRENT));
	  int size_total = stoi(item_map.get(FI_SIZE_TOTAL));
	  ItemCell::CRC_Type crc_type = (ItemCell::CRC_Type)stoi(item_map.get(FI_CRC_TYPE));
	  unsigned int crc = stoui(item_map.get(FI_CRC), 16);
	  string md5string = item_map.get(FI_MD5);
	  unsigned int prevDlSize = stoui(item_map.get(FI_PREVDLSIZE));

	  //int use_authentication = stoi(item_map[FI_USE_AUTHENTICATION]);
	  Userdata user(item_map.get(FI_USER), item_map.get(FI_PASSWORD));
	  //string storedir = item_map[FI_STORE_DIRECTORY];
	  //Options::RefererType referer_type = (Options::RefererType)stoi(item_map[FI_REFERER_TYPE]);
	  //string referer_string = item_map[FI_REFERER_STRING];
	  //string http_version = item_map[FI_HTTP_VERSION];
	  //string useragent = item_map[FI_USERAGENT];
	  //unsigned int random_useragent = stoi(item_map[FI_RANDOM_USERAGENT]);
	  //unsigned int cookie_delete_on_restart = stoi(item_map[FI_COOKIE_DELETE_ON_RESTART]);
	  //unsigned int cookie_nosend = stoi(item_map[FI_COOKIE_NOSEND]);
	  //int use_http_proxy = stoi(item_map[FI_USE_HTTP_PROXY]);
	  //int use_http_cache = stoi(item_map[FI_USE_HTTP_CACHE]);
	  //int use_http_proxy_authentication = stoi(item_map[FI_USE_HTTP_PROXY_AUTHENTICATION]);
	  Userdata http_proxy_user(item_map.get(FI_HTTP_PROXY_USER),
				   item_map.get(FI_HTTP_PROXY_PASSWORD));
	  Proxyserver http_proxyserver(item_map.get(FI_HTTP_PROXY_SERVER), stoi(item_map.get(FI_HTTP_PROXY_PORT)));

	  // FTP proxy
	  //int use_ftp_proxy = stoi(item_map[FI_USE_FTP_PROXY]);
	  //unsigned int use_ftp_proxy_authentication = stoi(item_map[FI_USE_FTP_PROXY_AUTHENTICATION]);      
	  Userdata ftp_proxy_user(item_map.get(FI_FTP_PROXY_USER),
				  item_map.get(FI_FTP_PROXY_PASSWORD));
	  Proxyserver ftp_proxyserver(item_map.get(FI_FTP_PROXY_SERVER), stoi(item_map.get(FI_FTP_PROXY_PORT)));

	  //bool use_ftp_cache = (bool)stoi(item_map[FI_USE_FTP_CACHE]);
	  //bool use_ftp_proxy_via_http = (bool)stoi(item_map[FI_USE_FTP_PROXY_VIA_HTTP]);
	  //Options::FTPproxyLoginProcType ftp_proxy_login_proc =
	  // (Options::FTPproxyLoginProcType)stoi(item_map[FI_FTP_PROXY_LOGIN_PROC]);

	  //int timedout = stoi(item_map[FI_TIMED_OUT]);
	  //int retry = stoi(item_map[FI_RETRY]);
	  //int retry_repeat = stoi(item_map[FI_RETRY_REPEAT]);
	  //bool use_retry_404 = (bool)stoi(item_map[FI_FORCE_RETRY_404]);
	  //bool use_retry_503 = (bool)stoi(item_map[FI_FORCE_RETRY_503]);
	  //int divide = stoi(item_map[FI_HOW_MANY_PARTS]);
	  //int rollback_bytes = stoi(item_map[FI_ROLLBACK_BYTES]);
	  //int recurse_count = stoi(item_map[FI_RECURSE_COUNT]);
	  string documentroot_dir = item_map.get(FI_DOCUMENTROOT_DIR);
	  string root_url = item_map.get(FI_ROOT_URL);

	  //bool use_down_filter = (bool)stoi(item_map[FI_USE_DOWN_FILTER]);
	  list<string> filter_down_target_list;
	  parse_extension_target_list2(filter_down_target_list, item_map.get(FI_FILTER_DOWN_TARGET_LIST));
	  list<string> activated_filter_down_target_list;
	  parse_extension_target_list2(activated_filter_down_target_list, item_map.get(FI_ACTIVATED_FILTER_DOWN_TARGET_LIST));

	  list<string> filter_nodown_target_list;
	  parse_extension_target_list(filter_nodown_target_list, item_map.get(FI_FILTER_NODOWN_TARGET_LIST));
	  list<string> activated_filter_nodown_target_list;
	  parse_extension_target_list(activated_filter_nodown_target_list, item_map.get(FI_ACTIVATED_FILTER_NODOWN_TARGET_LIST));
	  list<string> parse_target_list;
	  parse_extension_target_list(parse_target_list, item_map.get(FI_PARSE_TARGET_LIST));
	  list<string> activated_parse_target_list;
	  parse_extension_target_list(activated_parse_target_list, item_map.get(FI_ACTIVATED_PARSE_TARGET_LIST));
	
	  //Options::DownloadMethodType downm_type = (Options::DownloadMethodType)stoi(item_map.get(FI_DOWNM_TYPE));
	  //int speed_limit = stoi(item_map.get(FI_SPEED_LIMIT));
	  //float speed_limit = strtod(item_map.get(FI_SPEED_LIMIT).c_str(), NULL);
	  //cerr << "speed " << speed_limit << endl;
	  // various options for recursive download
	  //bool recurse_with_hostname_dir = (bool)stoi(item_map.get(FI_WITH_HOSTNAME_DIR));
	  //bool recurse_abs2rel = (bool)stoi(item_map.get(FI_ABS2REL));
	  //bool recurse_force_convert = (bool)stoi(item_map.get(FI_FORCE_CONVERT));
	  //bool recurse_del_comment = (bool)stoi(item_map.get(FI_DEL_COMMENT));
	  //bool recurse_del_javascript = (bool)stoi(item_map.get(FI_DEL_JAVASCRIPT));
	  //bool recurse_no_other_host = (bool)stoi(item_map.get(FI_NO_OTHER_HOST));
	  //bool recurse_no_ascend = (bool)stoi(item_map.get(FI_NO_ASCEND));
	  //bool recurse_relative_only = (bool)stoi(item_map.get(FI_ONLY_RELATIVE_LINKS));
	  //bool recurse_referer_override = (bool)stoi(item_map.get(FI_REFERER_OVERRIDE));
	  //bool recurse_follow_ftp_link = (bool)stoi(item_map.get(FI_FOLLOW_FTP_LINK));
	  //bool recurse_convert_tilde = (bool)stoi(item_map.get(FI_CONVERT_TILDE));
	  //bool use_tag_href = (bool)stoi(item_map.get(FI_USE_TAG_HREF));
	  //bool use_tag_src = (bool)stoi(item_map.get(FI_USE_TAG_SRC));
	  //bool use_tag_background = (bool)stoi(item_map.get(FI_USE_TAG_BACKGROUND));
	  //bool use_tag_code = (bool)stoi(item_map.get(FI_USE_TAG_CODE));

	  list<string> ign_domain_list, activated_ign_domain_list;
	  parse_extension_target_list(ign_domain_list, item_map.get(FI_IGN_DOMAIN_LIST));
	  parse_extension_target_list(activated_ign_domain_list, item_map.get(FI_ACTIVATED_IGN_DOMAIN_LIST));

	  //Options::FTP_Mode ftp_mode = (Options::FTP_Mode)stoi(item_map.get(FI_FTP_MODE));
	  //Options::FTPretModeType ftp_ret_mode = (Options::FTPretModeType)stoi(item_map.get(FI_FTP_RET_MODE));
	  //bool ftp_nosend_quit = (bool)stoi(item_map.get(FI_FTP_NOSEND_QUIT));

	  //int ftp_recurse_count = stoi(item_map.get(FI_FTP_RECURSE_COUNT));

	  //bool ftp_use_filter = (bool)stoi(item_map.get(FI_FTP_USE_FILTER));
	  //bool ftp_allow_crawl_subdir = (bool)stoi(item_map.get(FI_FTP_ALLOW_CRAWL_SUBDIR));
	  //bool ftp_no_ascend = (bool)stoi(item_map.get(FI_FTP_NO_ASCEND));
	
	  //bool ftp_get_symlink_as_realfile = (bool)stoi(item_map.get(FI_FTP_GET_SYMLINK_AS_REALFILE));
	  list<string> ftp_filter_target_list, ftp_activated_filter_target_list;
	  parse_extension_target_list2(ftp_filter_target_list, item_map.get(FI_FTP_FILTER_TARGET_LIST));
	  parse_extension_target_list2(ftp_activated_filter_target_list, item_map.get(FI_FTP_ACTIVATED_FILTER_TARGET_LIST));

	  //bool delete_when_finish = (bool)stoi(item_map.get(FI_DELETE_WHEN_COMPLETED));
	  //bool dont_delete_without_crc = (bool)stoi(item_map.get(FI_DONT_DELETE_WITHOUT_CRC));
	  //bool no_crc_check = (bool)stoi(item_map.get(FI_NO_CRC_CHECKING));
	  //bool ignore_crc_error = (bool)stoi(item_map.get(FI_IGNORE_CRC_ERROR));

	  //bool use_command = (bool)stoi(item_map.get(FI_USE_COMMAND));
	  //bool use_exit_status = (bool)stoi(item_map.get(FI_USE_EXIT_STATUS));

	  string command_string = item_map.get(FI_COMMAND_STRING);
	  for(unsigned int i = 0; i < command_string.size(); i++) {
	    if(command_string.at(i) == '\r') {
	      command_string.replace(i, 1, "\n");
	    }
	  }

	  Command command(command_string,
			  item_map.get(FI_EXIT_STATUS_STRING));

	  // create URLcontainer
	  URLcontainer urlcon;
	  urlcon.Parse_URL(url);
	  // create Options

	  Options options((bool)stoi(item_map.get(FI_USE_AUTHENTICATION)),
			  user,
			  item_map.get(FI_STORE_DIRECTORY),
			  item_map.get(FI_HTTP_VERSION),
			  (Options::PrewrittenHTMLType)stoi(item_map.get(FI_PREWRITTEN_HTML_TYPE)),
			  item_map.get(FI_PREWRITTEN_HTML_NAME),
			  (bool)stoi(item_map.get(FI_SYNC_WITH_URL)),
			  (Options::RefererType)stoi(item_map.get(FI_REFERER_TYPE)),
			  item_map.get(FI_REFERER_STRING),
			  item_map.get(FI_USERAGENT),
			  (bool)stoi(item_map.get(FI_RANDOM_USERAGENT)),
			  (bool)stoi(item_map.get(FI_USE_HTTP_PROXY)),
			  (bool)stoi(item_map.get(FI_USE_HTTP_CACHE)),
			  stoi(item_map.get(FI_USE_HTTP_PROXY_AUTHENTICATION)),
			  http_proxy_user,
			  http_proxyserver,
			  stoi(item_map.get(FI_TIMED_OUT)),
			  stoi(item_map.get(FI_HOW_MANY_PARTS)),
			  stoi(item_map.get(FI_ROLLBACK_BYTES)),
			  (bool)stoi(item_map.get(FI_USE_SIZE_LOWER_LIMIT)),
			  stoi(item_map.get(FI_SIZE_LOWER_LIMIT)),
			  (bool)stoi(item_map.get(FI_USE_NO_REDOWNLOAD)),
			  (bool)stoi(item_map.get(FI_USE_NO_DOWNLOAD_SAMENAME)),
			  stoi(item_map.get(FI_RECURSE_COUNT)),
			  (bool)stoi(item_map.get(FI_WITH_HOSTNAME_DIR)),
			  (bool)stoi(item_map.get(FI_ABS2REL)),
			  (bool)stoi(item_map.get(FI_FORCE_CONVERT)),
			  (bool)stoi(item_map.get(FI_DEL_COMMENT)),
			  (bool)stoi(item_map.get(FI_DEL_JAVASCRIPT)),
			  (bool)stoi(item_map.get(FI_DEL_IFRAME)),
			  (bool)stoi(item_map.get(FI_NO_OTHER_HOST)),
			  (bool)stoi(item_map.get(FI_NO_ASCEND)),
			  (bool)stoi(item_map.get(FI_ONLY_RELATIVE_LINKS)),
			  (bool)stoi(item_map.get(FI_REFERER_OVERRIDE)),
			  (bool)stoi(item_map.get(FI_FOLLOW_FTP_LINK)),
			  false,//(bool)stoi(item_map.get(FI_CONVERT_TILDE)),
			  (bool)stoi(item_map.get(FI_NO_REDOWNLOAD_HTTP_RECURSE)),
			  (bool)stoi(item_map.get(FI_HTTP_RECURSE_ADD_PASTE)),
			  (bool)stoi(item_map.get(FI_USE_TAG_HREF)),
			  (bool)stoi(item_map.get(FI_USE_TAG_SRC)),
			  (bool)stoi(item_map.get(FI_USE_TAG_BACKGROUND)),
			  (bool)stoi(item_map.get(FI_USE_TAG_CODE)),
			  (bool)stoi(item_map.get(FI_USE_DOWN_FILTER)),
			  filter_down_target_list,
			  filter_nodown_target_list,
			  parse_target_list,
			  ign_domain_list,
			  (Options::FTP_Mode)stoi(item_map.get(FI_FTP_MODE)),
			  (Options::FTPretModeType)stoi(item_map.get(FI_FTP_RET_MODE)),
			  stoi(item_map.get(FI_USE_FTP_PROXY)),
			  stoi(item_map.get(FI_USE_FTP_PROXY_AUTHENTICATION)),
			  ftp_proxy_user,
			  ftp_proxyserver,
			  (bool)stoi(item_map.get(FI_USE_FTP_CACHE)),
			  (bool)stoi(item_map.get(FI_USE_FTP_PROXY_VIA_HTTP)),
			  (Options::FTPproxyLoginProcType)stoi(item_map.get(FI_FTP_PROXY_LOGIN_PROC)),
			  (bool)stoi(item_map.get(FI_FTP_NOSEND_QUIT)),
			  (bool)stoi(item_map.get(FI_FTP_NO_CWD)),
			  stoi(item_map.get(FI_FTP_RECURSE_COUNT)),
			  (bool)stoi(item_map.get(FI_FTP_USE_FILTER)),
			  (bool)stoi(item_map.get(FI_FTP_ALLOW_CRAWL_SUBDIR)),
			  (bool)stoi(item_map.get(FI_FTP_NO_ASCEND)),
			  (bool)stoi(item_map.get(FI_FTP_GET_SYMLINK_AS_REALFILE)),
			  (bool)stoi(item_map.get(FI_FTP_RECURSE_ADD_PASTE)),
			  ftp_filter_target_list,
			  (bool)stoi(item_map.get(FI_DELETE_WHEN_COMPLETED)),
			  stoi(item_map.get(FI_RETRY)),
			  stoi(item_map.get(FI_RETRY_REPEAT)),
			  stoi(item_map.get(FI_RETRY_INTERVAL)),
			  (bool)stoi(item_map.get(FI_FORCE_RETRY_404)),
			  (bool)stoi(item_map.get(FI_FORCE_RETRY_503)),
			  (Options::Status416HandlingType)stoi(item_map.get(FI_STATUS_416_HANDLING)),
			  (bool)stoi(item_map.get(FI_USE_NO_REDIRECTION)),
			  (bool)stoi(item_map.get(FI_HTTP_ACCEPT_COMPRESSION)),
			  (bool)stoi(item_map.get(FI_HTTP_ACCEPT_LANG_ENABLED)),
			  item_map.get(FI_HTTP_ACCEPT_LANG_STRING),
			  (bool)stoi(item_map.get(FI_DONT_DELETE_WITHOUT_CRC)),
			  (bool)stoi(item_map.get(FI_NO_CRC_CHECKING)),
			  (bool)stoi(item_map.get(FI_IGNORE_CRC_ERROR)),
			  (bool)stoi(item_map.get(FI_USE_CONTENT_MD5)),
			  (bool)stoi(item_map.get(FI_COOKIE_DELETE_ON_RESTART)),
			  (bool)stoi(item_map.get(FI_COOKIE_NOSEND)),
			  (bool)stoi(item_map.get(FI_COOKIE_USERDEFINED)),
			  item_map.get(FI_COOKIE_USERDEFINED_STRING),
			  (Options::DownloadMethodType)stoi(item_map.get(FI_DOWNM_TYPE)),
			  strtod(item_map.get(FI_SPEED_LIMIT).c_str(), NULL),
			  (bool)stoi(item_map.get(FI_USE_COMMAND)),
			  (bool)stoi(item_map.get(FI_USE_EXIT_STATUS)),
			  command
			  );
	  /*
	    Options options((bool)use_authentication,
	    user,
	    storedir,
	    http_version,
	    referer_type,
	    referer_string,
	    useragent,
	    (bool)random_useragent,
	    (bool)use_http_proxy,
	    (bool)use_http_cache,
	    use_http_proxy_authentication,
	    http_proxy_user,
	    http_proxyserver,
	    timedout,
	    divide,
	    rollback_bytes,
	    recurse_count,
	    recurse_with_hostname_dir,
	    recurse_abs2rel,
	    recurse_force_convert,
	    recurse_del_comment,
	    recurse_del_javascript,
	    recurse_no_other_host,
	    recurse_no_ascend,
	    recurse_relative_only,
	    recurse_referer_override,
	    recurse_follow_ftp_link,
	    recurse_convert_tilde,
	    use_tag_href,
	    use_tag_src,
	    use_tag_background,
	    use_tag_code,
	    use_down_filter,
	    filter_down_target_list,
	    filter_nodown_target_list,
	    parse_target_list,
	    ign_domain_list,
	    ftp_mode,
	    ftp_ret_mode,
	    (bool)use_ftp_proxy,
	    use_ftp_proxy_authentication,
	    ftp_proxy_user,
	    ftp_proxyserver,
	    use_ftp_cache,
	    use_ftp_proxy_via_http,
	    ftp_proxy_login_proc,
	    ftp_nosend_quit,
	    ftp_recurse_count,
	    ftp_use_filter,
	    ftp_allow_crawl_subdir,
	    ftp_no_ascend,
	    ftp_get_symlink_as_realfile,
	    ftp_filter_target_list,
	    delete_when_finish,
	    retry,
	    retry_repeat,
	    use_retry_404,
	    use_retry_503,
	    dont_delete_without_crc,
	    no_crc_check,
	    ignore_crc_error,
	    (bool)cookie_delete_on_restart,
	    (bool)cookie_nosend,
	    downm_type,
	    speed_limit,
	    use_command,
	    use_exit_status,
	    command
	    );
	  */

	  options.activate_filter_down_target_list(activated_filter_down_target_list);
	  options.activate_filter_nodown_target_list(activated_filter_nodown_target_list);
	  options.activate_parse_target_list(activated_parse_target_list);
	  options.activate_ign_domain_list(activated_ign_domain_list);
	  options.set_FTP_activated_filter_target_list(ftp_activated_filter_target_list);
	  // create ItemCell
	  ItemCell *itemcell = new ItemCell(url, urlcon, options, _("Created"));
	  if(!filename.empty()) {
	    itemcell->set_Filename(filename);
	    itemcell->set_Filename_opt(filename);
	  }
	  itemcell->set_documentroot_dir(documentroot_dir);
	  itemcell->set_root_url(root_url);
	  itemcell->set_Status(status);
	  itemcell->set_Size_Current(size_current);
	  itemcell->set_Size_Total(size_total);
	  itemcell->set_CRC_Type(crc_type);
	  itemcell->set_CRC(crc);
	  itemcell->set_md5string(md5string);
	  itemcell->set_previous_dl_size(prevDlSize);
	  if(status == ItemCell::ITEM_ERROR) {
	    g_summaryInfo.inc_error();
	  }

	  clist_item[COL_ICON] = NULL;
	  if(itemcell->ret_Filename().empty()) {
	    clist_item[COL_FILENAME] = g_strdup(_("<directory>"));
	  } else {
	    clist_item[COL_FILENAME] = g_strdup(itemcell->ret_Filename().c_str());
	  }
	  clist_item[COL_SPEED] = "";
	  clist_item[COL_RTIME] = "";
	  clist_item[COL_CRC] = g_strdup(item_map.get(FI_CRC).c_str());
	  clist_item[COL_MD5] = g_strdup(item_map.get(FI_MD5).c_str());
	  clist_item[COL_STATUS] = "";
	  int progress;
	  string cursize;
	  string totsize;
	  if(itemcell->ret_Size_Total() > 0) {
	    if(g_appOption->ret_use_size_human_readable()) {
	      cursize = get_human_readable_size(size_current);
	      totsize = get_human_readable_size(size_total);
	    } else {
	      cursize = itos(itemcell->ret_Size_Current(), true);
	      totsize = itos(itemcell->ret_Size_Total(), true);
	    }
	    progress = (int)((float)itemcell->ret_Size_Current()/itemcell->ret_Size_Total()*100);
	  } else {
	    if(g_appOption->ret_use_size_human_readable()) {
	      cursize = get_human_readable_size(size_current);
	    } else {
	      cursize = itos(size_current);
	    }
	    totsize = _("unknown");
	    progress = 0;
	  }
	  clist_item[COL_CURSIZE] = g_strdup(cursize.c_str());
	  clist_item[COL_TOTSIZE] = g_strdup(totsize.c_str());
	  clist_item[COL_PROGRESS] = NULL;//g_strdup(progress.c_str());

	  if(itemcell->ret_Options().ret_Retry() == -1) {
	    // modified 2001/5/21
	    clist_item[COL_RETRY] = g_strdup("0/-");
	  } else {
	    clist_item[COL_RETRY] = g_strdup_printf("0/%d", itemcell->ret_Options().ret_Retry());
	  }
	  int n_rec;
	  if(urlcon.ret_Protocol() == "http:"
#ifdef HAVE_OPENSSL
	     || urlcon.ret_Protocol() == "https:"
#endif // HAVE_OPENSSL
	     ) {
	    n_rec = itemcell->ret_Options().ret_recurse_count();
	  } else {
	    n_rec = itemcell->ret_Options().ret_FTP_recurse_count();
	  }

	  clist_item[COL_REC] = g_strdup_printf("%d", n_rec);

	  clist_item[COL_SAVE] = g_strdup(itemcell->ret_Options().ret_Store_Dir().c_str());
	  clist_item[COL_URL] = g_strdup(itemcell->ret_URL().c_str());
	  int rowindex = listentry->Append_dl_item(clist_item, itemcell);
	  //listentry->Set_clist_column__icon(rowindex, itemcell->ret_Status());
	  //listentry->Set_clist_column__progress(rowindex, progress);
	
	  g_free(clist_item[COL_FILENAME]);
	  g_free(clist_item[COL_CRC]);
	  g_free(clist_item[COL_MD5]);
	  g_free(clist_item[COL_CURSIZE]);
	  g_free(clist_item[COL_TOTSIZE]);
	  g_free(clist_item[COL_RETRY]);
	  g_free(clist_item[COL_REC]);
	  g_free(clist_item[COL_SAVE]);
	  g_free(clist_item[COL_URL]);
	  break;
	} else {
	  item_map.add(item, Remove_white(line));
	}
      }
    }
    //gtk_clist_thaw(GTK_CLIST(listentry->ret_Dl_clist()));
    listentry->thawDlCList();
  }

  
  return true;
}

bool ItemList::Restore_saved_default_item_settings()
{
  ifstream infile(file_default_item_settings.c_str(), ios::in);//ios::skipws|ios::in);
  if(infile.bad() || infile.fail()) return false;// || infile.eof()) return false;

  string header;
  getline(infile, header, '\n');
  if(header != ARIA_VERSION) {
    g_consoleItem->Send_message_to_gui(_("Warning: different version number found in file header"), MSG_SYS_INFO);
  }
  // restore default item option
  Restore_default_item_option_sub(infile, g_consoleItem->ret_Options_opt());

  return true;
}

bool ItemList::Restore_saved_app_settings()
{
  ifstream infile(file_app_settings.c_str(), ios::in);//ios::skipws|ios::in);
  if(infile.bad() || infile.fail()) return false;

  string line;
  getline(infile, line, '\n');
  if(line != ARIA_VERSION) {
    g_consoleItem->Send_message_to_gui(_("Warning: different version number found in file header"), MSG_SYS_INFO);
  }
  //hash_map<string, string, hash<string> > app_map;
  StringHash app_map;
  app_map.add("%History-Limit:", "10001");
  while(1) {
    string item;
    infile >> item;
    if(!infile.good()) break;
    string value;
    getline(infile, value, '\n');
    app_map.add(item, Remove_white(value));
  }
  infile.close();

  int hour_start = stoi(app_map.get("%Timer-Start-Hour:"));
  int min_start = stoi(app_map.get("%Timer-Start-Min:"));
  int hour_stop = stoi(app_map.get("%Timer-Stop-Hour:"));
  int min_stop = stoi(app_map.get("%Timer-Stop-Min:"));
  TimerData timerdata(hour_start, min_start, hour_stop, min_stop);

  int history_limit = stoi(app_map.get("%History-Limit:"));
  if(history_limit > MAXHISTORY) history_limit = DEFAULTHISTORY;

  int speedLimit = stoi(app_map.get("%Max-Speed-Limit:"));
  if(speedLimit > MAXSPEEDLIMIT) {
    speedLimit = DEFAULTSPEEDLIMIT;
  } else if(speedLimit <= 0) { // fix this
    speedLimit = DEFAULTSPEEDLIMIT;
  }

  int maxthread = stoui(app_map.get("%Threads:"));
  if(maxthread < 0) { 
    maxthread = 1;
  } else if(maxthread > g_threadLimit) {
    maxthread = g_threadLimit;
  }

  bool track_download = stoi(app_map.get("%Track-Download:"));
  Track_enabled(track_download);

  string tempBuff;
  list<string> ignore_extension_list;
  tempBuff = app_map.get("%IGNORE-EXTENSION-LIST:");
  while(tempBuff.size()) {
    ignore_extension_list.push_back(Token_splitter(tempBuff, " ,"));
  }

  list<string> svt_name_list;
  tempBuff = app_map.get("%SERVER-TEMPLATE-IN-USE:");
  while(tempBuff.size()) {
    svt_name_list.push_back(Token_splitter(tempBuff, " ,"));
  }
  list<string> com_name_list;
  tempBuff = app_map.get("%COMMAND-LIST-IN-USE:");
  while(tempBuff.size()) {
    com_name_list.push_back(Token_splitter(tempBuff, " ,"));
  }

  g_appOption->Set_Option_Values(maxthread,
				 stoi(app_map.get("%Automatic_Start:")),
				 stoi(app_map.get("%Autostart.CurrentListOnly:")),
				 stoi(app_map.get("%ForceDownloadNow:")),
				 stoi(app_map.get("%Use-Ignore-Error-Item:")),
				 stoi(app_map.get("%USE-IGNORE-EXTENSION-LIST:")),
				 ignore_extension_list,
				 stoi(app_map.get("%Use-Autosave:")),
				 stoi(app_map.get("%Autosave-Interval:")),
				 history_limit,
				 speedLimit,
				 stoi(app_map.get("%Use-Arbitrary-Command:")),
				 stoi(app_map.get("%Use-Arbitrary-Command-Timer:")),
				 app_map.get("%Arbitrary-Command:"),
				 stoi(app_map.get("%Use-Quit-Program:")),
				 stoi(app_map.get("%StartTimerEnabled:")),
				 stoi(app_map.get("%StopTimerEnabled:")),
				 stoi(app_map.get("%Timer-Start-All-Tab:")),
				 stoi(app_map.get("%Timer-No-Stop-Download-On-Timer:")),
				 timerdata,
				 stoi(app_map.get("%CONFIRM-CLEAR:")),
				 stoi(app_map.get("%CONFIRM-DELETE-TAB:")),
				 stoi(app_map.get("%CONFIRM-EXIT:")),
				 stoi(app_map.get("%CONFIRM-CLEARLOG:")),
				 stoi(app_map.get("%USE-SERVERTEMPLATE:")),
				 stoi(app_map.get("%USE-COMMANDLIST:")),
			         stoi(app_map.get("%USE-SIZE-HUMAN-READABLE:")),
				 app_map.get("%ICON-DIR:"),
				 app_map.get("%BASKET-PIXMAP-FILE:"),
				 stoi(app_map.get("%BASKET-DIRECT-PASTING:")),
				 svt_name_list,
				 com_name_list);
  
  g_historyWindow->setHistoryMax(g_appOption->ret_history_limit());
  Adjust_speed_scale(speedLimit);

  return true;
}

