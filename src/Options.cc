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

// $Id: Options.cc,v 1.42 2002/04/03 13:33:52 tujikawa Exp $

// implementation of class Options

#include "Options.h"

Userdata::Userdata()
{
}

Userdata::Userdata(const string& username_in, const string& password_in)
{
  username = username_in;
  password = password_in;
}

Userdata::~Userdata()
{
}

void Userdata::set_userpasswd(const string& username_in, const string& password_in)
{
  username = username_in;
  password = password_in;
}

Options::Options(bool use_authentication_in,
		 const Userdata& user_in,
		 const string& store_dir_in,
		 const string& http_version_in,
		 PrewrittenHTMLType prewrittenHTMLType_in,
		 const string& prewrittenHTMLName_in,
		 bool sync_with_url_in,
		 RefererType referer_type_in,
		 const string& referer_in,
		 const string& useragent_in,
		 bool random_useragent_in,
		 bool use_http_proxy_in,
		 bool use_http_cache_in,
		 bool use_http_proxy_authentication_in,
		 const Userdata& http_proxy_user_in,
		 const Proxyserver& http_proxy_in,
		 unsigned int timedout_in,
		 unsigned int divide_in,
		 int rollback_bytes_in,
		 bool use_size_lower_limit_in,
		 int size_lower_limit_in,
		 bool use_no_redownload_in,
		 bool use_no_download_samename_in,
		 unsigned int recurse_in,
		 bool recurse_with_hostname_dir_in,
		 bool recurse_abs2rel_in,
		 bool recurse_force_convert_in,
		 bool recurse_del_comment_in,
		 bool recurse_del_javascript_in,
		 bool recurse_del_iframe_in,
		 bool recurse_no_other_host_in,
		 bool recurse_no_ascend_in,
		 bool recurse_relative_only_in,
		 bool recurse_referer_override_in,
		 bool recurse_follow_ftp_link_in,
		 bool recurse_convert_tilde_in,
		 bool recurse_no_redownload_in,
		 bool recurse_add_paste_in,
		 bool tag_href_in,
		 bool tag_src_in,
		 bool tag_background_in,
		 bool tag_code_in,
		 bool use_down_filter_in,
		 const list<string>& filter_down_target_list_in,
		 const list<string>& filter_nodown_target_list_in,
		 const list<string>& parse_target_list_in,
		 const list<string>& ign_domain_list_in,
		 FTP_Mode ftp_mode_in,
		 FTPretModeType ftp_ret_mode_in,
		 bool use_ftp_proxy_in,
		 bool use_ftp_proxy_authentication_in,
		 const Userdata& ftp_proxy_user_in,
		 const Proxyserver& ftp_proxy_in,
		 bool use_ftp_cache_in,
		 bool use_ftp_proxy_via_http_in,
		 FTPproxyLoginProcType ftp_proxy_login_proc_in,
		 bool ftp_nosend_quit_in,
		 bool ftpNoCwd_in,
		 int ftp_recurse_count_in,
		 bool ftp_use_filter_in,
		 bool ftp_allow_crawl_subdir_in,
		 bool ftp_no_ascend_in,
		 bool ftp_get_symlink_as_realfile_in,
		 bool ftp_recurse_add_paste_in,
		 const list<string>& ftp_filter_target_list_in,
		 bool delete_when_finish_in,
		 int retry_in,
		 int retry_repeat_in,
		 int retry_interval_in,
		 bool use_retry_404_in,
		 bool use_retry_503_in,
		 Status416HandlingType s416Handling_in,
		 bool use_no_redirection_in,
		 bool httpAcceptCompression_in,
		 bool httpAcceptLangEnabled_in,
		 const string& httpAcceptLangString_in,
		 bool dont_delete_without_crc_in,
		 bool no_crc_check_in,
		 bool ignore_crc_error_in,
		 bool use_content_md5_in,
		 bool cookie_delete_on_restart_in,
		 bool cookie_nosend_in,
		 bool cookieUserDefined_in,
		 const string& cookieUserDefinedString_in,
		 DownloadMethodType downm_type_in,
		 float speed_limit_in,
		 bool use_command_in,
		 bool use_exit_status_in,
		 const Command& command_in
		 )
{
  boolOpt = FULLZERO;
  boolOptRec = FULLZERO;

  boolOpt |= use_authentication_in << b_use_authentication;

  user = user_in;
  
  store_dir = store_dir_in;
  if(store_dir.at(store_dir.size()-1) != '/') {
    store_dir += "/";
  }
  if(http_version_in.size()) {
    http_version = http_version_in;
  } else {
    http_version = DEFAULT_HTTP_VERSION;
  }

  prewrittenHTMLType = prewrittenHTMLType_in;
  prewrittenHTMLName = prewrittenHTMLName_in;

  boolOpt |= httpAcceptCompression_in << b_httpAcceptCompression;
  boolOpt |= httpAcceptLangEnabled_in << b_httpAcceptLangEnabled;
  httpAcceptLangString = httpAcceptLangString_in;
  boolOpt |= sync_with_url_in << b_sync_with_url;
  referer_type = referer_type_in;
  referer = referer_in;
  useragent = useragent_in;
  boolOpt |= random_useragent_in << b_random_useragent;

  boolOpt |= use_http_proxy_in << b_use_http_proxy;
  boolOpt |= use_http_cache_in << b_use_http_cache;
  http_proxy = http_proxy_in;
  boolOpt |= use_http_proxy_authentication_in << b_use_http_proxy_authentication;
  http_proxy_user = http_proxy_user_in;

  boolOpt |= use_ftp_proxy_in << b_use_ftp_proxy;
  boolOpt |= use_ftp_cache_in << b_use_ftp_cache;
  ftp_proxy = ftp_proxy_in;
  boolOpt |= use_ftp_proxy_authentication_in << b_use_ftp_proxy_authentication;
  ftp_proxy_user = ftp_proxy_user_in;
  boolOpt |= use_ftp_proxy_via_http_in << b_use_ftp_proxy_via_http;
  ftp_proxy_login_proc = ftp_proxy_login_proc_in;

  timedout = timedout_in;
  divide = divide_in;

  if(rollback_bytes_in < 0) {
    rollback_bytes = 0;
  } else {
    rollback_bytes = rollback_bytes_in;
  }

  boolOpt |= use_size_lower_limit_in << b_use_size_lower_limit;
  if(size_lower_limit_in < 0) {
    size_lower_limit = 0;
  } else {
    size_lower_limit = size_lower_limit_in;
  }
  boolOpt |= use_no_redownload_in << b_use_no_redownload;
  boolOpt |= use_no_download_samename_in << b_use_no_download_samename;

  ftp_mode = ftp_mode_in;
  ftp_ret_mode = ftp_ret_mode_in;
  boolOpt |= ftp_nosend_quit_in << b_ftp_nosend_quit;
  boolOpt |= ftpNoCwd_in << b_ftpNoCwd;
  if(ftp_recurse_count_in < 1) {
    ftp_recurse_count = 1;
  } else {
    ftp_recurse_count = ftp_recurse_count_in;
  }
  boolOptRec |= ftp_use_filter_in << b_ftp_use_filter;
  boolOptRec |= ftp_allow_crawl_subdir_in << b_ftp_allow_crawl_subdir;
  boolOptRec |= ftp_no_ascend_in << b_ftp_no_ascend;
  boolOptRec |= ftp_get_symlink_as_realfile_in << b_ftp_get_symlink_as_realfile;
  boolOptRec |= ftp_recurse_add_paste_in << b_ftp_recurse_add_paste;
  ftp_filter_target_list =  ftp_filter_target_list_in;

  boolOpt |= delete_when_finish_in << b_delete_when_finish;
  boolOpt |= no_crc_check_in << b_no_crc_check;
  boolOpt |= ignore_crc_error_in << b_ignore_crc_error;
  boolOpt |= use_content_md5_in << b_use_content_md5;

  if(retry_in < -1) {
    retry = -1;
  } else if(retry_in > MAXRETRY) {
    retry = MAXRETRY;
  } else {
    retry = retry_in;
  }

  retry_repeat = retry_repeat_in;

  if(retry_interval_in < 0) {
    retry_interval = 0;
  } else if(retry_interval_in > MAXRETRYINTERVAL) {
    retry_interval = MAXRETRYINTERVAL;
  } else {
    retry_interval = retry_interval_in;
  }

  boolOpt |= use_retry_404_in << b_use_retry_404;
  boolOpt |= use_retry_503_in << b_use_retry_503;
  s416Handling = s416Handling_in;
  boolOpt |= use_no_redirection_in << b_use_no_redirection;

  boolOpt |= dont_delete_without_crc_in << b_dont_delete_without_crc;
  boolOpt |= cookie_delete_on_restart_in << b_cookie_delete_on_restart;
  boolOpt |= cookie_nosend_in << b_cookie_nosend;
  boolOpt |= cookieUserDefined_in << b_cookieUserDefined;
  cookieUserDefinedString = cookieUserDefinedString_in;

  if(recurse_in < 1) recurse_count = 1;
  else recurse_count = recurse_in;
  boolOptRec |= recurse_with_hostname_dir_in << b_recurse_with_hostname_dir;
  boolOptRec |= recurse_abs2rel_in << b_recurse_abs2rel;
  boolOptRec |= recurse_force_convert_in << b_recurse_force_convert;
  boolOptRec |= recurse_del_comment_in << b_recurse_del_comment;
  boolOptRec |= recurse_del_javascript_in << b_recurse_del_javascript;
  boolOptRec |= recurse_del_iframe_in << b_recurse_del_iframe;
  boolOptRec |= recurse_no_other_host_in << b_recurse_no_other_host;
  boolOptRec |= recurse_no_ascend_in << b_recurse_no_ascend;
  boolOptRec |= recurse_relative_only_in << b_recurse_relative_only;
  boolOptRec |= recurse_referer_override_in << b_recurse_referer_override;
  boolOptRec |= recurse_follow_ftp_link_in << b_recurse_follow_ftp_link;
  boolOptRec |= recurse_convert_tilde_in << b_recurse_convert_tilde;
  boolOptRec |= recurse_no_redownload_in << b_recurse_no_redownload;
  boolOptRec |= recurse_add_paste_in << b_recurse_add_paste;
  //documentroot_dir = store_dir_in;
  boolOptRec |= tag_href_in << b_tag_href;
  boolOptRec |= tag_src_in << b_tag_src;
  boolOptRec |= tag_background_in << b_tag_background;
  boolOptRec |= tag_code_in << b_tag_code;

  boolOptRec |= use_down_filter_in << b_use_down_filter;
  filter_down_target_list = filter_down_target_list_in;
  filter_nodown_target_list = filter_nodown_target_list_in;
  parse_target_list = parse_target_list_in;
  ign_domain_list = ign_domain_list_in;

  speed_limit = speed_limit_in;
  //printf("%f\n", speed_limit_in);
  //cerr << "inoptions" << speed_limit_in << endl;
  downm_type = downm_type_in;

  boolOpt |= use_command_in << b_use_command;
  boolOpt |= use_exit_status_in << b_use_exit_status;
  command = command_in;
}

Options::Options()
{
  boolOpt = FULLZERO;
  boolOptRec = FULLZERO;

  boolOpt |= DEFAULT_USE_AUTHENTICATION << b_use_authentication;

  user.username = DEFAULT_OPTION_USER;
  user.password = DEFAULT_OPTION_PASSWORD;

  char *current_dir = g_get_current_dir();
  store_dir = current_dir;
  g_free(current_dir);

  if(store_dir.at(store_dir.size()-1) != '/') {
    store_dir += "/";
  }
  http_version = DEFAULT_HTTP_VERSION;
  
  prewrittenHTMLType = DEFAULT_PREWRITTEN_HTML_TYPE;
  prewrittenHTMLName = DEFAULT_PREWRITTEN_HTML_NAME;

  boolOpt |= DEFAULT_HTTP_ACCEPT_COMPRESSION << b_httpAcceptCompression;
  boolOpt |= DEFAULT_HTTP_ACCEPT_LANG_ENABLED << b_httpAcceptLangEnabled;
  httpAcceptLangString = DEFAULT_HTTP_ACCEPT_LANG_STRING;
  boolOpt |= DEFAULT_OPTION_SYNC_WITH_URL_ENABLED << b_sync_with_url;
  referer_type = DEFAULT_OPTION_HTTP_REFERER_TYPE;
  referer = DEFAULT_OPTION_HTTP_REFERER;
  useragent = "";
  boolOpt |= DEFAULT_OPTION_HTTP_RANDOM_USER_AGENT_ENABLED << b_random_useragent;

  boolOpt |= DEFAULT_USE_HTTP_PROXY << b_use_http_proxy;
  boolOpt |= DEFAULT_USE_HTTP_CACHE << b_use_http_cache;
  boolOpt |= DEFAULT_USE_HTTP_PROXY_AUTHENTICATION << b_use_http_proxy_authentication;
  http_proxy_user.username = DEFAULT_HTTP_PROXY_USER;
  http_proxy_user.password = DEFAULT_HTTP_PROXY_PASSWORD;
  
  boolOpt |= DEFAULT_USE_FTP_PROXY << b_use_ftp_proxy;
  boolOpt |= DEFAULT_USE_FTP_PROXY_AUTHENTICATION << b_use_ftp_proxy_authentication;
  ftp_proxy_user.username = DEFAULT_FTP_PROXY_USER;
  ftp_proxy_user.password = DEFAULT_FTP_PROXY_PASSWORD;
  boolOpt |= DEFAULT_USE_FTP_CACHE << b_use_ftp_cache;
  boolOpt |= DEFAULT_USE_FTP_PROXY_VIA_HTTP << b_use_ftp_proxy_via_http;
  ftp_proxy_login_proc = DEFAULT_FTP_PROXY_LOGIN_PROC;

  timedout = DEFAULT_OPTION_TIME_OUT;
  divide = DEFAULT_OPTION_SPLIT_NUM;
  rollback_bytes = DEFAULT_OPTION_ROLLBACK_BYTES;
  boolOpt |= DEFAULT_OPTION_SIZE_LOWER_LIMIT_ENABLED << b_use_size_lower_limit;
  size_lower_limit = DEFAULT_OPTION_SIZE_LOWER_LIMIT;
  boolOpt |= DEFAULT_USE_NO_REDOWNLOAD << b_use_no_redownload;
  boolOpt |= DEFAULT_USE_NO_DOWNLOAD_SAMENAME << b_use_no_download_samename;

  ftp_mode = DEFAULT_FTP_MODE;
  ftp_ret_mode = DEFAULT_FTP_RET_MODE;
  boolOpt |= DEFAULT_FTP_NOSEND_QUIT << b_ftp_nosend_quit;
  boolOpt |= DEFAULT_FTP_NO_CWD << b_ftpNoCwd;
  ftp_recurse_count = DEFAULT_FTP_RECURSE_COUNT;
  boolOptRec |= DEFAULT_FTP_USE_FILTER << b_ftp_use_filter;
  boolOptRec |= DEFAULT_FTP_ALLOW_CRAWL_SUBDIR << b_ftp_allow_crawl_subdir;
  boolOptRec |= DEFAULT_FTP_NO_ASCEND << b_ftp_no_ascend;
  boolOptRec |= DEFAULT_FTP_GET_SYMLINK_AS_REALFILE << b_ftp_get_symlink_as_realfile;
  boolOptRec |= DEFAULT_FTP_RECURSE_ADD_PASTE << b_ftp_recurse_add_paste;

  boolOpt |= DEFAULT_IFDEL << b_delete_when_finish;
  boolOpt |= DEFAULT_NO_CRC_CHECKING << b_no_crc_check;
  boolOpt |= DEFAULT_IGNORE_CRC_ERROR << b_ignore_crc_error;
  boolOpt |= DEFAULT_USE_CONTENT_MD5 << b_use_content_md5;
  retry = DEFAULT_RETRY;
  retry_repeat = DEFAULT_RETRY_REPEAT;
  retry_interval = DEFAULT_OPTION_RETRY_INTERVAL;
  boolOpt |= DEFAULT_FORCE_RETRY_404 << b_use_retry_404;
  boolOpt |= DEFAULT_FORCE_RETRY_503 << b_use_retry_503;
  s416Handling = DEFAULT_STATUS_416_HANDLING;
  boolOpt |= DEFAULT_USE_NO_REDIRECTION << b_use_no_redirection;

  boolOpt |= DEFAULT_IFCRC << b_dont_delete_without_crc;
  boolOpt |= DEFAULT_COOKIE_DELETE_ON_RESTART << b_cookie_delete_on_restart;
  boolOpt |= DEFAULT_COOKIE_NOSEND << b_cookie_nosend;
  boolOpt |= DEFAULT_COOKIE_USERDEFINED << b_cookieUserDefined;
  cookieUserDefinedString = DEFAULT_COOKIE_USERDEFINED_STRING;

  // remove_this!
  recurse_count = DEFAULT_OPTION_HTTP_RECURSE_COUNT_COUNT;
  boolOptRec |= DEFAULT_WITH_HOSTNAME_DIR << b_recurse_with_hostname_dir;
  boolOptRec |= DEFAULT_ABS2REL << b_recurse_abs2rel;
  boolOptRec |= DEFAULT_FORCE_CONVERT << b_recurse_force_convert;
  boolOptRec |= DEFAULT_DEL_COMMENT << b_recurse_del_comment;
  boolOptRec |= DEFAULT_DEL_JAVASCRIPT << b_recurse_del_javascript;
  boolOptRec |= DEFAULT_DEL_IFRAME << b_recurse_del_iframe;
  boolOptRec |= DEFAULT_NO_OTHER_HOST << b_recurse_no_other_host;
  boolOptRec |= DEFAULT_NO_ASCEND << b_recurse_no_ascend;
  boolOptRec |= DEFAULT_RELATIVE_ONLY << b_recurse_relative_only;
  boolOptRec |= DEFAULT_REFERER_OVERRIDE << b_recurse_referer_override;
  boolOptRec |= DEFAULT_FOLLOW_FTP_LINK << b_recurse_follow_ftp_link;
  boolOptRec |= DEFAULT_CONVERT_TILDE << b_recurse_convert_tilde;
  boolOptRec |= DEFAULT_NO_REDOWNLOAD_HTTP_RECURSE << b_recurse_no_redownload;
  boolOptRec |= DEFAULT_HTTP_RECURSE_ADD_PASTE << b_recurse_add_paste;
  boolOptRec |= DEFAULT_TAG_HREF << b_tag_href;
  boolOptRec |= DEFAULT_TAG_SRC << b_tag_src;
  boolOptRec |= DEFAULT_TAG_BACKGROUND << b_tag_background;
  boolOptRec |= DEFAULT_TAG_CODE << b_tag_code;
  //documentroot_dir = store_dir;

  //filter_target_list = filter_target_list_in;
  //parse_target_list = parse_target_list_in;
  speed_limit = DEFAULT_SPEED_LIMIT;
  boolOptRec |= DEFAULT_USE_DOWN_FILTER << b_use_down_filter;
  downm_type = DOWNM_ALWAYSRESUME;

  boolOpt |= DEFAULT_USE_COMMAND << b_use_command;
  boolOpt |= DEFAULT_USE_EXIT_STATUS << b_use_exit_status;
}

Options::~Options()
{
}

void Options::Change_Values(bool use_authentication_in,
			    const Userdata& user_in,
			    const string& store_dir_in,
			    const string& http_version_in,
			    PrewrittenHTMLType prewrittenHTMLType_in,
			    const string& prewrittenHTMLName_in,
			    bool sync_with_url_in,
			    RefererType referer_type_in,
			    const string& referer_in,
			    const string& useragent_in,
			    bool random_useragent_in,
			    bool use_http_proxy_in,
			    bool use_http_cache_in,
			    bool use_http_proxy_authentication_in,
			    const Userdata& http_proxy_user_in,
			    const Proxyserver& http_proxy_in,
			    unsigned int timedout_in,
			    unsigned int divide_in,
			    int rollback_bytes_in,
			    bool use_size_lower_limit_in,
			    int size_lower_limit_in,
			    bool use_no_redownload_in,
			    bool use_no_download_samename_in,
			    unsigned int recurse_count_in,
			    bool recurse_with_hostname_dir_in,
			    bool recurse_abs2rel_in,
			    bool recurse_force_convert_in,
			    bool recurse_del_comment_in,
			    bool recurse_del_javascript_in,
			    bool recurse_del_iframe_in,
			    bool recurse_no_other_host_in,
			    bool recurse_no_ascend_in,
			    bool recurse_relative_only_in,
			    bool recurse_referer_override_in,
			    bool recurse_follow_ftp_link_in,
			    bool recurse_convert_tilde_in,
			    bool recurse_no_redownload_in,
			    bool recurse_add_paste_in,
			    bool tag_href_in,
			    bool tag_src_in,
			    bool tag_background_in,
			    bool tag_code_in,
			    bool use_down_filter_in,
			    const list<string>& filter_down_target_list_in,
			    const list<string>& filter_nodown_target_list_in,
			    const list<string>& parse_target_list_in,
			    const list<string>& ign_domain_list_in,
			    FTP_Mode ftp_mode_in,
			    FTPretModeType ftp_ret_mode_in,
			    bool use_ftp_proxy_in,
			    bool use_ftp_proxy_authentication_in,
			    const Userdata& ftp_proxy_user_in,
			    const Proxyserver& ftp_proxy_in,
			    bool use_ftp_cache_in,
			    bool use_ftp_proxy_via_http_in,
			    FTPproxyLoginProcType ftp_proxy_login_proc_in,
			    bool ftp_nosend_quit_in,
			    bool ftpNoCwd_in,
			    int ftp_recurse_count_in,
			    bool ftp_use_filter_in,
			    bool ftp_allow_crawl_subdir_in,
			    bool ftp_no_ascend_in,
			    bool ftp_get_symlink_as_realfile_in,
			    bool ftp_recurse_add_paste_in,
			    const list<string>& ftp_filter_target_list_in,
			    bool delete_when_finish_in,
			    int retry_in,
			    int retry_repeat_in,
			    int retry_interval_in,
			    bool use_retry_404_in,
			    bool use_retry_503_in,
			    Status416HandlingType s416Handling_in,
			    bool use_no_redirection_in,
			    bool httpAcceptCompression_in,
			    bool httpAcceptLangEnabled_in,
			    const string& httpAcceptLangString_in,
			    bool dont_delete_without_crc_in,
			    bool no_crc_check_in,
			    bool ignore_crc_error_in,
			    bool use_content_md5_in,
			    bool cookie_delete_on_restart_in,
			    bool cookie_nosend_in,
			    bool cookieUserDefined_in,
			    const string& cookieUserDefinedString_in,
			    DownloadMethodType downm_type_in,
			    float speed_limit_in,
			    bool use_command_in,
			    bool use_exit_status_in,
			    const Command& command_in
			    )
{
  boolOpt = FULLZERO;
  boolOptRec = FULLZERO;

  boolOpt |= use_authentication_in << b_use_authentication;
  user = user_in;

  store_dir = store_dir_in;

  if(store_dir.at(store_dir.size()-1) != '/') {
    store_dir += "/";
  }
  if(http_version_in.size()) {
    http_version = http_version_in;
  } else {
    http_version = DEFAULT_HTTP_VERSION;
  }

  prewrittenHTMLType = prewrittenHTMLType_in;
  prewrittenHTMLName = prewrittenHTMLName_in;

  boolOpt |= httpAcceptCompression_in << b_httpAcceptCompression;
  boolOpt |= httpAcceptLangEnabled_in << b_httpAcceptLangEnabled;
  httpAcceptLangString = httpAcceptLangString_in;
  boolOpt |= sync_with_url_in << b_sync_with_url;
  referer_type = referer_type_in;
  referer = referer_in;
  useragent = useragent_in;
  boolOpt |= random_useragent_in << b_random_useragent;

  boolOpt |= use_http_proxy_in << b_use_http_proxy;
  boolOpt |= use_http_cache_in << b_use_http_cache;
  http_proxy = http_proxy_in;
  boolOpt |= use_http_proxy_authentication_in << b_use_http_proxy_authentication;
  http_proxy_user = http_proxy_user_in;

  boolOpt |= use_ftp_proxy_in << b_use_ftp_proxy;
  ftp_proxy = ftp_proxy_in;
  boolOpt |= use_ftp_proxy_authentication_in << b_use_ftp_proxy_authentication;
  ftp_proxy_user = ftp_proxy_user_in;
  boolOpt |= use_ftp_cache_in << b_use_ftp_cache;
  boolOpt |= use_ftp_proxy_via_http_in << b_use_ftp_proxy_via_http;
  ftp_proxy_login_proc = ftp_proxy_login_proc_in;

  timedout = timedout_in;
  divide = divide_in;
  if(rollback_bytes_in < 0) {
    rollback_bytes = 0;
  } else {
    rollback_bytes = rollback_bytes_in;
  }
  boolOpt |= use_size_lower_limit_in << b_use_size_lower_limit;
  if(size_lower_limit_in < 0) {
    size_lower_limit = 0;
  } else {
    size_lower_limit = size_lower_limit_in;
  }
  boolOpt |= use_no_redownload_in << b_use_no_redownload;
  boolOpt |= use_no_download_samename_in << b_use_no_download_samename;

  ftp_mode = ftp_mode_in;
  ftp_ret_mode = ftp_ret_mode_in;

  boolOpt |= ftp_nosend_quit_in << b_ftp_nosend_quit;
  boolOpt |= ftpNoCwd_in << b_ftpNoCwd;
  if(ftp_recurse_count_in < 1) {
    ftp_recurse_count = 1;
  } else {
    ftp_recurse_count = ftp_recurse_count_in;
  }

  boolOptRec |= ftp_use_filter_in << b_ftp_use_filter;
  boolOptRec |= ftp_allow_crawl_subdir_in << b_ftp_allow_crawl_subdir;
  boolOptRec |= ftp_no_ascend_in << b_ftp_no_ascend;
  boolOptRec |= ftp_get_symlink_as_realfile_in << b_ftp_get_symlink_as_realfile;
  ftp_filter_target_list =  ftp_filter_target_list_in;
  boolOptRec |= ftp_recurse_add_paste_in << b_ftp_recurse_add_paste;

  boolOpt |= delete_when_finish_in << b_delete_when_finish;
  boolOpt |= no_crc_check_in << b_no_crc_check;
  boolOpt |= ignore_crc_error_in << b_ignore_crc_error;
  boolOpt |= use_content_md5_in << b_use_content_md5;
  retry = retry_in;
  retry_repeat = retry_repeat_in;
  retry_interval = retry_interval_in;
  boolOpt |= use_retry_404_in << b_use_retry_404;
  boolOpt |= use_retry_503_in << b_use_retry_503;
  s416Handling = s416Handling_in;
  boolOpt |= use_no_redirection_in << b_use_no_redirection;

  boolOpt |= dont_delete_without_crc_in << b_dont_delete_without_crc;
  boolOpt |= cookie_delete_on_restart_in << b_cookie_delete_on_restart;
  boolOpt |= cookie_nosend_in << b_cookie_nosend;
  boolOpt |= cookieUserDefined_in << b_cookieUserDefined;
  cookieUserDefinedString = cookieUserDefinedString_in;

  if(recurse_count_in < 1) recurse_count = 1;
  else recurse_count = recurse_count_in;

  boolOptRec |= recurse_with_hostname_dir_in << b_recurse_with_hostname_dir;
  boolOptRec |= recurse_abs2rel_in << b_recurse_abs2rel;
  boolOptRec |= recurse_force_convert_in << b_recurse_force_convert;
  boolOptRec |= recurse_del_comment_in << b_recurse_del_comment;
  boolOptRec |= recurse_del_javascript_in << b_recurse_del_javascript;
  boolOptRec |= recurse_del_iframe_in << b_recurse_del_iframe;
  boolOptRec |= recurse_no_other_host_in << b_recurse_no_other_host;
  boolOptRec |= recurse_no_ascend_in << b_recurse_no_ascend;
  boolOptRec |= recurse_relative_only_in << b_recurse_relative_only;
  boolOptRec |= recurse_referer_override_in << b_recurse_referer_override;
  boolOptRec |= recurse_follow_ftp_link_in << b_recurse_follow_ftp_link;
  boolOptRec |= recurse_convert_tilde_in << b_recurse_convert_tilde;
  boolOptRec |= recurse_no_redownload_in << b_recurse_no_redownload;
  boolOptRec |= recurse_add_paste_in << b_recurse_add_paste;

  boolOptRec |= tag_href_in << b_tag_href;
  boolOptRec |= tag_src_in << b_tag_src;
  boolOptRec |= tag_background_in << b_tag_background;
  boolOptRec |= tag_code_in << b_tag_code;


  boolOptRec |= use_down_filter_in << b_use_down_filter;
  filter_down_target_list = filter_down_target_list_in;
  filter_nodown_target_list = filter_nodown_target_list_in;
  parse_target_list = parse_target_list_in;
  ign_domain_list = ign_domain_list_in;

  speed_limit = speed_limit_in;
  downm_type = downm_type_in;

  boolOpt |= use_command_in << b_use_command;
  boolOpt |= use_exit_status_in << b_use_exit_status;
  command = command_in;
}

bool Options::ret_use_no_download_samename() const
{
  return boolOpt & 1 << b_use_no_download_samename;
}

bool Options::ret_sync_with_URL() const
{
  return boolOpt & 1 << b_sync_with_url;
}

Options::DownloadMethodType Options::ret_downm_type() const
{
  return downm_type;
}

bool Is_in_list(const list<string>& target_list, const string& item)
{
  for(list<string>::const_iterator itr = target_list.begin();
      itr != target_list.end(); ++itr) {
    if(itr->size() < item.size()) {
      if(item.substr(item.size()-itr->size()) == *itr) return true;
    }
  }
  return false;

}

bool Is_in_list_by_pattern(const list<string>& target_list, const string& item)
{
  for(list<string>::const_iterator itr = target_list.begin();
      itr != target_list.end(); ++itr) {
    if(itr->size() < item.size()) {
      if(patternMatch(item, *itr)) return true;
    }
  }
  return false;

}

void Options::set_downm_type(DownloadMethodType downm_type_in)
{
  downm_type = downm_type_in;
}

const string& Options::ret_Useragent() const
{
  return useragent;
}

bool Options::Whether_use_authentication() const
{
  return(boolOpt & 1 << b_use_authentication);
}

const string& Options::ret_HTTP_version() const
{
  return http_version;
}

Options::PrewrittenHTMLType Options::getPrewrittenHTMLType() const
{
  return prewrittenHTMLType;
}

const string& Options::getPrewrittenHTMLName() const
{
  return prewrittenHTMLName;
}

Options::RefererType Options::ret_Referer_Type() const
{
  return(referer_type);
}

const string& Options::ret_Referer() const
{
  return(referer);
}

unsigned int Options::ret_Timed_Out() const
{
  return(timedout);
}

Options::FTP_Mode Options::ret_FTP_Mode() const
{
  return ftp_mode;
}

Options::FTPretModeType Options::ret_FTP_ret_mode() const
{
  return ftp_ret_mode;
}

bool Options::ret_FTP_nosend_quit() const
{
  return(boolOpt & 1 << b_ftp_nosend_quit);
}

bool Options::isFtpNoCwdEnabled() const
{
  return(boolOpt & 1 << b_ftpNoCwd);
}

void Options::setFtpNoCwd(bool toggle)
{
  boolOpt |= toggle << b_ftpNoCwd;
}

unsigned int Options::ret_FTP_recurse_count() const
{
  return ftp_recurse_count;
}

void Options::set_FTP_recurse_count(int count)
{
  if(count < 1) count = 1;
  ftp_recurse_count = count;
}

bool Options::ret_FTP_use_filter() const
{
  return(boolOptRec & 1 << b_ftp_use_filter);
}

void Options::set_FTP_use_filter(bool toggle)
{
  boolOptRec |= toggle << b_ftp_use_filter;
}

bool Options::ret_FTP_allow_crawl_subdir() const
{
  return(boolOptRec & 1 << b_ftp_allow_crawl_subdir);
}

void Options::set_FTP_allow_crawl_subdir(bool toggle)
{
  boolOptRec |= toggle << b_ftp_allow_crawl_subdir;
}

bool Options::ret_FTP_no_ascend() const
{
  return(boolOptRec & 1 << b_ftp_no_ascend);
}

void Options::set_FTP_no_ascend(bool toggle)
{
  boolOptRec |= toggle << b_ftp_no_ascend;
}

bool Options::ret_FTP_get_symlink_as_realfile() const
{
  return(boolOptRec & 1 << b_ftp_get_symlink_as_realfile);
}

void Options::set_FTP_get_symlink_as_realfile(bool toggle)
{
  boolOptRec |= toggle << b_ftp_get_symlink_as_realfile;
}

bool Options::ret_FTP_recurse_add_paste() const
{
  return(boolOptRec & 1 << b_ftp_recurse_add_paste);
}

void Options::set_FTP_recurse_add_paste(bool toggle)
{
  boolOptRec |= toggle << b_ftp_recurse_add_paste;
}

const list<string>& Options::ret_FTP_filter_target_list() const
{
  return ftp_filter_target_list;
}

const list<string>& Options::ret_FTP_activated_filter_target_list() const
{
  return ftp_active_filter_target_list;
}

void Options::set_FTP_filter_target_list(const list<string>& target_list)
{
  ftp_filter_target_list = target_list;
}

void Options::set_FTP_activated_filter_target_list(const list<string>& target_list)
{
  ftp_active_filter_target_list = target_list;
}

bool Options::Is_in_FTP_filter_target_list(const string& file) const
{
  return Is_in_list_by_pattern(ftp_active_filter_target_list, file);
}

int Options::ret_Retry() const
{
  return retry;
}

int Options::ret_Retry_repeat() const
{
  return retry_repeat;
}

int Options::ret_Retry_interval() const
{
  return retry_interval;
}

bool Options::ret_force_retry_404() const
{
  return(boolOpt & 1 << b_use_retry_404);
}

bool Options::ret_force_retry_503() const
{
  return(boolOpt & 1 << b_use_retry_503);
}

Options::Status416HandlingType Options::ret_status_416_handling() const
{
  return s416Handling;
}

bool Options::ret_use_no_redirection() const
{
  return(boolOpt & 1 << b_use_no_redirection);
}

bool Options::ret_Delete_When_Finish() const
{
  return(boolOpt & 1 << b_delete_when_finish);
}

bool Options::ret_Dont_Delete_Without_CRC() const
{
  return(boolOpt & 1 << b_dont_delete_without_crc);
}

bool Options::ret_no_crc_checking() const
{
  return(boolOpt & 1 << b_no_crc_check);
}

bool Options::ret_ignore_crc_error() const
{
  return(boolOpt & 1 << b_ignore_crc_error);
}

bool Options::ret_use_content_md5() const
{
  return(boolOpt & 1 << b_use_content_md5);
}

bool Options::ret_use_no_redownload() const
{
  return(boolOpt & 1 << b_use_no_redownload);
}

const string& Options::ret_User() const
{
  return(user.username);
}

const string& Options::ret_Password() const
{
  return(user.password);
}

const string& Options::ret_http_proxy_User() const
{
  return(http_proxy_user.username);
}

const string& Options::ret_http_proxy_Password() const
{
  return(http_proxy_user.password);
}
 
const string& Options::ret_Store_Dir() const
{
  return store_dir;
}

unsigned int Options::ret_Divide() const
{
  return divide;
}

void Options::set_Divide(unsigned int divide_in)
{
  divide = divide_in;
}

int Options::ret_Rollback_bytes() const
{
  return rollback_bytes;
}

bool Options::ret_use_http_proxy() const
{
  return(boolOpt & 1 << b_use_http_proxy);
}

bool Options::ret_use_http_cache() const
{
  return(boolOpt & 1 << b_use_http_cache);
}

const Proxyserver& Options::ret_http_proxy() const
{
  return http_proxy;
}

void Options::set_Store_Dir(const string& store_dir_in)
{
  store_dir = store_dir_in;
  if(store_dir.at(store_dir.size()-1) != '/') {
    store_dir += "/";
  }
}

bool Options::ret_use_http_proxy_authentication() const
{
  return(boolOpt & 1 << b_use_http_proxy_authentication);
}

void Options::set_use_http_proxy(bool toggle)
{
  boolOpt |= toggle << b_use_http_proxy;
}

bool Options::ret_Random_useragent() const
{
  return(boolOpt & 1 << b_random_useragent);
}

void Options::set_Referer_Type(RefererType referer_type_in)
{
  referer_type = referer_type_in;
}

void Options::set_Referer(const string& referer_in)
{
  referer = referer_in;
}

bool Options::ret_Cookie_delete_on_restart() const
{
  return(boolOpt & 1 << b_cookie_delete_on_restart);
}

bool Options::ret_Cookie_nosend() const
{
  return(boolOpt & 1 << b_cookie_nosend);
}

bool Options::getCookieUserDefined() const
{
  return(boolOpt & 1 << b_cookieUserDefined);
}

const string& Options::getCookieUserDefinedString() const
{
  return cookieUserDefinedString;
}

// depth of recursive download
void Options::set_recurse_count(unsigned int n_rec)
{
  recurse_count = n_rec;
}

unsigned int Options::ret_recurse_count() const
{
  return recurse_count;
}

const list<string>& Options::ret_parse_target_list() const
{
  return parse_target_list;
}

const list<string>& Options::ret_filter_down_target_list() const
{
  return filter_down_target_list;
}

const list<string>& Options::ret_filter_nodown_target_list() const
{
  return filter_nodown_target_list;
}

const list<string>& Options::ret_ign_domain_list() const
{
  return ign_domain_list;
}

const list<string>& Options::ret_activated_parse_target_list() const
{
  return active_parse_target_list;
}

const list<string>& Options::ret_activated_filter_down_target_list() const
{
  return active_filter_down_target_list;
}

const list<string>& Options::ret_activated_filter_nodown_target_list() const
{
  return active_filter_nodown_target_list;
}

const list<string>& Options::ret_activated_ign_domain_list() const
{
  return active_ign_domain_list;
}

void Options::activate_parse_target_list(const list<string>& target_list)
{
  active_parse_target_list = target_list;
}

void Options::activate_filter_down_target_list(const list<string>& target_list)
{
  active_filter_down_target_list = target_list;
}

void Options::activate_filter_nodown_target_list(const list<string>& target_list)
{
  active_filter_nodown_target_list = target_list;
}

void Options::activate_ign_domain_list(const list<string>& target_list)
{
  active_ign_domain_list = target_list;
}

bool Options::Is_in_activated_parse_target_list(const string& filename) const
{
  return Is_in_list(active_parse_target_list, filename);
}

bool Options::Is_in_activated_filter_down_target_list(const string& filename) const {
  return Is_in_list_by_pattern(active_filter_down_target_list, filename);
}

bool Options::Is_in_activated_filter_nodown_target_list(const string& filename) const
{
  return Is_in_list(active_filter_nodown_target_list, filename);
  /*
  for(list<string>::const_iterator itr = active_filter_nodown_target_list.begin();
      itr != active_filter_nodown_target_list.end(); ++itr) {
    if(itr->size() < filename.size()) {
      if(filename.substr(filename.size()-itr->size()) == *itr) return true;
    }
  }
  return false;
  */
}

bool Options::Is_in_activated_ign_domain_list(const string& domain) const
{
  return Is_in_list(active_ign_domain_list, domain);
}

bool Options::ret_with_hostname_dir() const
{
  return(boolOptRec & 1 << b_recurse_with_hostname_dir);
}

bool Options::ret_abs2rel_url() const
{
  return(boolOptRec & 1 << b_recurse_abs2rel);
}

bool Options::ret_force_convert() const
{
  return(boolOptRec & 1 << b_recurse_force_convert);
}

bool Options::ret_no_other_host() const
{
  return(boolOptRec & 1 << b_recurse_no_other_host);
}

bool Options::ret_no_ascend() const
{
  return(boolOptRec & 1 << b_recurse_no_ascend);
}

bool Options::ret_only_relative_links() const
{
  return(boolOptRec & 1 << b_recurse_relative_only);
}

bool Options::ret_Referer_override() const
{
  return(boolOptRec & 1 << b_recurse_referer_override);
}

bool Options::ret_Follow_ftp_link() const
{
  return(boolOptRec & 1 << b_recurse_follow_ftp_link);
}

bool Options::ret_HTTP_recurse_add_paste() const
{
  return(boolOptRec & 1 << b_recurse_add_paste);
}

void Options::set_HTTP_recurse_add_paste(bool toggle)
{
  boolOptRec |= toggle << b_recurse_add_paste;
}

bool Options::ret_convert_tilde() const
{
  return(boolOptRec & 1 << b_recurse_convert_tilde);
}

bool Options::ret_use_tag_href() const
{
  return(boolOptRec & 1 << b_tag_href);
}

bool Options::ret_use_tag_src() const
{
  return(boolOptRec & 1 << b_tag_src);
}

bool Options::ret_use_tag_background() const
{
  return(boolOptRec & 1 << b_tag_background);
}

bool Options::ret_use_tag_code() const
{
  return(boolOptRec & 1 << b_tag_code);
}

void Options::set_with_hostname_dir(bool toggle)
{
  boolOptRec |= toggle << b_recurse_with_hostname_dir;
}

void Options::set_abs2rel_url(bool toggle)
{
  boolOptRec |= toggle << b_recurse_abs2rel;
}

void Options::set_force_convert(bool toggle)
{
  boolOptRec |= toggle << b_recurse_force_convert;
}

void Options::set_no_other_host(bool toggle)
{
  boolOptRec |= toggle << b_recurse_no_other_host;
}

// delete commets from file
void Options::set_delete_comment(bool toggle)
{
  boolOptRec |= toggle << b_recurse_del_comment;
}

bool Options::ret_delete_comment() const
{
  return(boolOptRec & 1 << b_recurse_del_comment);
}

// delete java scripts from file
void Options::set_delete_javascript(bool toggle)
{
  boolOptRec |= toggle << b_recurse_del_javascript;
}

bool Options::ret_delete_javascript() const
{
  return(boolOptRec & 1 << b_recurse_del_javascript);
}

void Options::set_delete_iframe(bool toggle)
{
  boolOptRec |= toggle << b_recurse_del_iframe;
}

bool Options::ret_delete_iframe() const
{
  return(boolOptRec & 1 << b_recurse_del_iframe);
}

void Options::set_no_ascend(bool toggle)
{
  boolOptRec |= toggle << b_recurse_no_ascend;
}

void Options::set_only_relative_links(bool toggle)
{
  boolOptRec |= toggle << b_recurse_relative_only;
}

void Options::set_Referer_override(bool toggle)
{
  boolOptRec |= toggle << b_recurse_referer_override;
}

void Options::set_Follow_ftp_link(bool toggle)
{
  boolOptRec |= toggle << b_recurse_follow_ftp_link;
}

void Options::set_convert_tilde(bool toggle)
{
  boolOptRec |= toggle << b_recurse_convert_tilde;
}

void Options::set_no_redownload_HTTP_recurse(bool toggle)
{
  boolOptRec |= toggle << b_recurse_no_redownload;
}

bool Options::ret_no_redownload_HTTP_recurse() const
{
  return(boolOptRec & 1 << b_recurse_no_redownload);
}

void Options::set_parse_target_list(const list<string>& target_list)
{
  parse_target_list = target_list;
}

void Options::set_filter_down_target_list(const list<string>& target_list)
{
  filter_down_target_list = target_list;
}

void Options::set_filter_nodown_target_list(const list<string>& target_list)
{
  filter_nodown_target_list = target_list;
}

void Options::set_ign_domain_list(const list<string>& target_list)
{
  ign_domain_list = target_list;
}

void Options::set_activated_parse_target_list(const list<string>& target_list)
{
  active_parse_target_list = target_list;
}

void Options::set_activated_filter_down_target_list(const list<string>& target_list)
{
  active_filter_down_target_list = target_list;
}

void Options::set_activated_filter_nodown_target_list(const list<string>& target_list)
{
  active_filter_nodown_target_list = target_list;
}

void Options::set_activated_ign_domain_list(const list<string>& target_list)
{
  active_ign_domain_list = target_list;
}

void Options::set_use_down_filter(bool toggle)
{
  boolOptRec |= toggle << b_use_down_filter;
}

bool Options::ret_use_down_filter() const
{
  return(boolOptRec & 1 << b_use_down_filter);
}

void Options::set_use_tag_href(bool toggle)
{
  boolOptRec |= toggle << b_tag_href;
}

void Options::set_use_tag_src(bool toggle)
{
  boolOptRec |= toggle << b_tag_src;
}
  
void Options::set_use_tag_background(bool toggle)
{
  boolOptRec |= toggle << b_tag_background;
}

void Options::set_use_tag_code(bool toggle)
{
  boolOptRec |= toggle << b_tag_code;
}

void Options::set_Delete_When_Finish(bool toggle)
{
  boolOpt |= toggle << b_delete_when_finish;
}

void Options::set_Dont_Delete_Without_CRC(bool toggle)
{
  boolOpt |= toggle << b_dont_delete_without_crc;
}

void Options::set_use_authentication(bool toggle)
{
  boolOpt |= toggle << b_use_authentication;
}

void Options::set_userpasswd(const Userdata& userdata_in)
{
  user = userdata_in;
}

float Options::ret_speed_limit() const
{
  return speed_limit;
}

void Options::set_speed_limit(float speed)
{
  speed_limit = speed;
}

bool Options::ret_use_Command() const
{
  return(boolOpt & 1 << b_use_command);
}

bool Options::ret_use_Exit_status() const
{
  return(boolOpt & 1 << b_use_exit_status);
}

const Command& Options::ret_Command() const
{
  return(command);
}

const string& Options::ret_ftp_proxy_User() const
{
  return ftp_proxy_user.username;
}

const string& Options::ret_ftp_proxy_Password() const
{
  return ftp_proxy_user.password;
}

bool Options::ret_use_ftp_proxy_authentication() const
{
  return(boolOpt & 1 << b_use_ftp_proxy_authentication);
}

void Options::set_use_ftp_proxy(bool toggle)
{
  boolOpt |= toggle << b_use_ftp_proxy;
}

bool Options::ret_use_ftp_proxy() const
{
  return(boolOpt & 1 << b_use_ftp_proxy);
}

const Proxyserver& Options::ret_ftp_proxy() const
{
  return ftp_proxy;
}

bool Options::ret_use_ftp_cache() const
{
  return(boolOpt & 1 << b_use_ftp_cache);
}

bool Options::ret_use_ftp_proxy_via_http() const
{
  return(boolOpt & 1 << b_use_ftp_proxy_via_http);
}

Options::FTPproxyLoginProcType Options::ret_ftp_proxy_login_proc() const
{
  return ftp_proxy_login_proc;
}

bool Options::ret_use_size_lower_limit() const
{
  return(boolOpt & 1 << b_use_size_lower_limit);
}

int Options::ret_size_lower_limit() const
{
  return size_lower_limit;
}

bool Options::ret_HTTP_accept_compression() const
{
  return(boolOpt & 1 << b_httpAcceptCompression);
}

bool Options::ret_HTTP_accept_lang_enabled() const
{
  return(boolOpt & 1 << b_httpAcceptLangEnabled);
}

const string& Options::ret_HTTP_accept_lang_string() const
{
  return(httpAcceptLangString);
}
