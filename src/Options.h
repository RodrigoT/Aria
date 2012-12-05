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

// $Id: Options.h,v 1.47 2002/04/03 13:33:52 tujikawa Exp $

#ifndef _OPTIONS_H_
#define _OPTIONS_H_

#include <string>
#include <list>
#include "aria.h"
#include "Proxyserver.h"
#include "Command.h"
using namespace std;

#define DEFAULT_OPTION_USER ""
#define DEFAULT_OPTION_PASSWORD ""
#define DEFAULT_OPTION_HTTP_REFERER_TYPE Options::REFERER_URL
#define DEFAULT_OPTION_HTTP_REFERER ""
#define DEFAULT_OPTION_SYNC_WITH_URL_ENABLED true
#define DEFAULT_OPTION_HTTP_RANDOM_USER_AGENT_ENABLED false
#define DEFAULT_OPTION_TIME_OUT 50
#define DEFAULT_OPTION_RETRY_INTERVAL 10
#define DEFAULT_OPTION_SPLIT_NUM 1
#define DEFAULT_OPTION_ROLLBACK_BYTES 0
#define DEFAULT_OPTION_SIZE_LOWER_LIMIT_ENABLED false
#define DEFAULT_OPTION_SIZE_LOWER_LIMIT 0
#define DEFAULT_OPTION_HTTP_RECURSE_COUNT 0
#define DEFAULT_SIZE_LIMIT 0
#define DEFAULT_USE_SIZE_FILTER false
#define DEFAULT_FTP_MODE Options::FTP_ACTIVE_MODE
#define DEFAULT_FTP_RET_MODE Options::FTP_BINARY
#define DEFAULT_FTP_NOSEND_QUIT true
#define DEFAULT_FTP_NO_CWD false
#define DEFAULT_FTP_RECURSE_COUNT 1
#define DEFAULT_FTP_USE_FILTER false
#define DEFAULT_FTP_NO_ASCEND true
#define DEFAULT_FTP_ALLOW_CRAWL_SUBDIR true
#define DEFAULT_FTP_GET_SYMLINK_AS_REALFILE true
#define DEFAULT_IFDEL false
#define DEFAULT_RETRY 10
#define DEFAULT_RETRY_REPEAT 0
#define DEFAULT_FORCE_RETRY_404 false
#define DEFAULT_FORCE_RETRY_503 false
#define DEFAULT_STATUS_416_HANDLING Options::S416SUCC
#define DEFAULT_USE_NO_REDIRECTION false
#define DEFAULT_IFCRC false
#define DEFAULT_NO_CRC_CHECKING false
#define DEFAULT_IGNORE_CRC_ERROR false
#define DEFAULT_USE_CONTENT_MD5 false
#define DEFAULT_HTTP_VERSION "1.1"
#define DEFAULT_USE_AUTHENTICATION false
#define DEFAULT_USE_NO_REDOWNLOAD true
#define DEFAULT_USE_NO_DOWNLOAD_SAMENAME false

#define DEFAULT_USE_HTTP_PROXY_AUTHENTICATION false
#define DEFAULT_HTTP_PROXY_USER ""
#define DEFAULT_HTTP_PROXY_PASSWORD ""
#define DEFAULT_USE_HTTP_PROXY false
#define DEFAULT_USE_HTTP_CACHE false

#define DEFAULT_USE_FTP_PROXY_AUTHENTICATION false
#define DEFAULT_FTP_PROXY_USER ""
#define DEFAULT_FTP_PROXY_PASSWORD ""
#define DEFAULT_USE_FTP_PROXY false
#define DEFAULT_USE_FTP_CACHE false
#define DEFAULT_USE_FTP_PROXY_VIA_HTTP false
#define DEFAULT_FTP_PROXY_LOGIN_PROC Options::FTPPROXY_PL_SITE

#define DEFAULT_COOKIE_DELETE_ON_RESTART false
#define DEFAULT_COOKIE_NOSEND false
#define DEFAULT_COOKIE_USERDEFINED false
#define DEFAULT_COOKIE_USERDEFINED_STRING ""
#define DEFAULT_OPTION_HTTP_RECURSE_COUNT_COUNT 1
#define DEFAULT_WITH_HOSTNAME_DIR true
#define DEFAULT_ABS2REL false
#define DEFAULT_FORCE_CONVERT false
#define DEFAULT_DEL_COMMENT false
#define DEFAULT_DEL_JAVASCRIPT false
#define DEFAULT_DEL_IFRAME false
#define DEFAULT_NO_OTHER_HOST false
#define DEFAULT_NO_ASCEND true
#define DEFAULT_RELATIVE_ONLY true
#define DEFAULT_REFERER_OVERRIDE false
#define DEFAULT_FOLLOW_FTP_LINK false
#define DEFAULT_CONVERT_TILDE false
#define DEFAULT_NO_REDOWNLOAD_HTTP_RECURSE false
#define DEFAULT_TAG_HREF true
#define DEFAULT_TAG_SRC true
#define DEFAULT_TAG_BACKGROUND true
#define DEFAULT_TAG_CODE true
#define DEFAULT_USE_DOWN_FILTER false
#define DEFAULT_DOWNM_TYPE Options::DOWNM_ALWAYSRESUME
#define DEFAULT_SPEED_LIMIT 0
#define DEFAULT_USE_COMMAND false
#define DEFAULT_USE_EXIT_STATUS false
#define DEFAULT_PREWRITTEN_HTML_TYPE Options::PREWRITTEN_HTML_INDEX
#define DEFAULT_PREWRITTEN_HTML_NAME ""
#define DEFAULT_HTTP_ACCEPT_COMPRESSION true
#define DEFAULT_HTTP_RECURSE_ADD_PASTE false
#define DEFAULT_FTP_RECURSE_ADD_PASTE false
#define DEFAULT_HTTP_ACCEPT_LANG_ENABLED false
#define DEFAULT_HTTP_ACCEPT_LANG_STRING ""

#define FULLZERO 0x0000

class Userdata
{
private:
    string username;
    string password;
    friend class Options;
public:
    Userdata();
    Userdata(const string &username, const string &password);
    void set_userpasswd(const string &username, const string &password);
    ~Userdata();
};

class Options
{
public:
    enum FTP_Mode {
        FTP_PASSIVE_MODE,
        FTP_ACTIVE_MODE
    };

    enum RefererType {
        REFERER_INDEX,
        REFERER_URL,
        REFERER_NONE,
        REFERER_NOSEND,
        REFERER_USER_DEFINED
    };

    enum DownloadMethodType {
        DOWNM_ALWAYSRESUME,
        DOWNM_IFMODSINCE,
        DOWNM_NORESUME
    };

    enum FTPretModeType {
        FTP_BINARY,
        FTP_ASCII
    };

    enum FTPproxyLoginProcType {
        FTPPROXY_PL,
        FTPPROXY_PL_OPEN,
        FTPPROXY_PL_OPEN2,
        FTPPROXY_PL_SITE,
        FTPPROXY_PL_USER,
        FTPPROXY_USER,
        FTPPROXY_OPEN
    };

    enum Status416HandlingType {
        S416SUCC,
        S416ERR,
        S416REDOWN
    };

    enum PrewrittenHTMLType {
        PREWRITTEN_HTML_INDEX,
        PREWRITTEN_HTML_USERDEFINED
    };

    //enum FilterType {
    //  ALLOW_DOWNLOAD,
    //  DISALLOW_DOWNLOAD
    //};
private:
    // general options
    //bool delete_when_finish;
#define b_delete_when_finish 0
    float speed_limit;
    int retry;
    int retry_repeat;
    int retry_interval;

    unsigned int boolOpt;

    //bool sync_with_url;
#define b_sync_with_url 1
    //bool use_no_download_samename;
#define b_use_no_download_samename 2
    //bool dont_delete_without_crc;
#define b_dont_delete_without_crc 3
    //bool no_crc_check;
#define b_no_crc_check 4
    //bool ignore_crc_error;
#define b_ignore_crc_error 5
    //bool use_content_md5;
#define b_use_content_md5 6
    //bool cookie_delete_on_restart;
#define b_cookie_delete_on_restart 7
    //bool use_no_redownload;
#define b_use_no_redownload 8

    DownloadMethodType downm_type;

    string store_dir;
    unsigned int timedout;
    unsigned int divide;
    int rollback_bytes;

    ////authentication
    //bool use_authentication;
#define b_use_authentication 9
    Userdata user;

    // command
    //bool use_command;
#define b_use_command 10
    //bool use_exit_status;
#define b_use_exit_status 11
    Command command;

    // HTTP related options
    string http_version;
    //bool use_retry_404;
#define b_use_retry_404 12
    //bool use_retry_503;
#define b_use_retry_503 13
    Status416HandlingType s416Handling;
    //bool use_no_redirection;
#define b_use_no_redirection  14

    PrewrittenHTMLType prewrittenHTMLType;
    string prewrittenHTMLName;

    //bool httpAcceptCompression;
#define b_httpAcceptCompression  15

    //bool httpAcceptLangEnabled;
#define b_httpAcceptLangEnabled  16
    string httpAcceptLangString;

    RefererType referer_type; // index.html or NONE or user-defined
    string referer; // user-defined referer string
    string useragent;
    //bool random_useragent;
#define b_random_useragent  17
    //bool use_http_proxy_authentication;
#define b_use_http_proxy_authentication 18
    Userdata http_proxy_user;
    Proxyserver http_proxy;
    //bool use_http_proxy;
#define b_use_http_proxy  19
    //bool use_http_cache;
#define b_use_http_cache  20
    //bool cookie_nosend;
#define b_cookie_nosend  21
    //bool cookieUserDefined;
#define b_cookieUserDefined  22
    string cookieUserDefinedString;
    //Filter by file size
    int size_lower_limit;
    //bool use_size_lower_limit;
#define b_use_size_lower_limit  23
    //bool use_ftp_proxy_authentication;
#define b_use_ftp_proxy_authentication  24
    Userdata ftp_proxy_user;
    Proxyserver ftp_proxy;
    //bool use_ftp_proxy;
#define b_use_ftp_proxy  25
    //bool use_ftp_cache;
#define b_use_ftp_cache  26
    //bool use_ftp_proxy_via_http;
#define b_use_ftp_proxy_via_http  27
    FTPproxyLoginProcType ftp_proxy_login_proc;
    //bool ftpNoCwd;
#define b_ftpNoCwd  28

    unsigned int boolOptRec;

    int recurse_count;
    list<string> filter_nodown_target_list;
    list<string> active_filter_nodown_target_list;
    //bool use_down_filter;
#define b_use_down_filter 0

    list<string> filter_down_target_list;
    list<string> active_filter_down_target_list;
    list<string> parse_target_list;
    list<string> active_parse_target_list;
    list<string> ign_domain_list;
    list<string> active_ign_domain_list;
    //bool recurse_with_hostname_dir;
#define b_recurse_with_hostname_dir  1
    //bool recurse_abs2rel;
#define b_recurse_abs2rel 2
    //bool recurse_force_convert;
#define b_recurse_force_convert 3
    //bool recurse_del_comment;
#define b_recurse_del_comment  4
    //bool recurse_del_javascript;
#define b_recurse_del_javascript 5
    //bool recurse_del_iframe;
#define b_recurse_del_iframe 6
    //bool recurse_no_other_host;
#define b_recurse_no_other_host 7
    //bool recurse_no_ascend;
#define b_recurse_no_ascend 8
    //bool recurse_relative_only;
#define b_recurse_relative_only 9
    //bool recurse_referer_override;
#define b_recurse_referer_override 10
    //bool recurse_follow_ftp_link;
#define b_recurse_follow_ftp_link  11
    //bool recurse_convert_tilde;
#define b_recurse_convert_tilde  12
    //bool recurse_no_redownload;
#define b_recurse_no_redownload  13
    //bool recurse_add_paste;
#define b_recurse_add_paste  14
    //FilterType recurse_filter_type;
    //bool tag_href;
#define b_tag_href  15
    //bool tag_src;
#define b_tag_src  16
    //bool tag_background;
#define b_tag_background  17
    //bool tag_code;
#define b_tag_code  18

    // FTP related options
    FTP_Mode ftp_mode; // PASV or ACTIVE
    FTPretModeType ftp_ret_mode; // BINARY or ASCII
    //bool ftp_nosend_quit; // send QUIT command or not
#define b_ftp_nosend_quit  19
    list<string> ftp_filter_target_list;// extension to download
    list<string> ftp_active_filter_target_list;
    int ftp_recurse_count;
    //bool ftp_use_filter; // use file extension filter or not
#define b_ftp_use_filter  20
    //bool ftp_allow_crawl_subdir; // crawl subdir or not
#define b_ftp_allow_crawl_subdir  21
    //bool ftp_no_ascend; // not to ascend parent directory
#define b_ftp_no_ascend  22
    //bool ftp_get_symlink_as_realfile; //  use real path instead of link
#define b_ftp_get_symlink_as_realfile  23
    //bool ftp_recurse_add_paste;
#define b_ftp_recurse_add_paste  24


public:
    Options(bool use_authentication_in,
            const Userdata &user_in,
            const string &store_dir_in,
            const string &http_version_in,
            PrewrittenHTMLType prewrittenHTMLType_in,
            const string &prewrittenHTMLName_in,
            bool sync_with_url,
            RefererType referer_type_in,
            const string &referer_in,
            const string &useragent_in,
            bool random_useragent_in,
            bool use_http_proxy_in,
            bool use_http_cache_in,
            bool use_http_proxy_authentication_in,
            const Userdata &http_proxy_user_in,
            const Proxyserver &http_proxy_in,
            unsigned int timedout_in,
            unsigned int divide_in,
            int rollback_bytes_in,
            bool use_size_lower_limit,
            int size_lower_limit,
            bool use_no_redownload,
            bool use_no_download_samename,
            unsigned int recurse_in,
            bool recurse_with_hostname_dir,
            bool recurse_abs2rel,
            bool recurse_force_convert,
            bool recurse_del_comment,
            bool recurse_del_javascript,
            bool recurse_del_iframe,
            bool recurse_no_other_host,
            bool recurse_no_ascend,
            bool recurse_relative_only,
            bool recurse_referer_override,
            bool recurse_follow_ftp_link,
            bool recurse_convert_tilde,
            bool recurse_no_download,
            bool recurse_add_paste,
            bool tag_href,
            bool tag_src,
            bool tag_background,
            bool tag_code,
            bool use_down_filter_in,
            const list<string> &filter_down_target_list_in,
            const list<string> &filter_nodown_target_list_in,
            const list<string> &parse_target_list_in,
            const list<string> &ign_domain_list_in,
            FTP_Mode ftp_mode_in,
            FTPretModeType ftp_ret_mode_in,
            bool use_ftp_proxy_in,
            bool use_ftp_proxy_authentication_in,
            const Userdata &ftp_proxy_user_in,
            const Proxyserver &ftp_proxy_in,
            bool use_ftp_cache,
            bool use_ftp_proxy_via_http,
            FTPproxyLoginProcType ftp_proxy_login_proc,
            bool ftp_nosend_quit,
            bool ftpNoCwd,
            int ftp_recurse_count,
            bool ftp_use_filter,
            bool ftp_allow_crawl_subdir,
            bool ftp_no_ascend,
            bool ftp_get_symlink_as_realfile,
            bool ftp_recurse_add_paste,
            const list<string> &ftp_filter_target_list,
            bool delete_when_finish_in,
            int retry_in,
            int retry_repeat_in,
            int retry_interval_in,
            bool use_retry_404,
            bool use_retry_503,
            Status416HandlingType s416Handling,
            bool use_no_redirection,
            bool httpAcceptCompression,
            bool httpAcceptLangEnabled,
            const string &httpAcceptLangString,
            bool dont_delete_without_crc_in,
            bool no_crc_check_in,
            bool ignore_crc_error_in,
            bool use_content_md5,
            bool cookie_delete_on_restart_in,
            bool cookie_delete_nosend,
            bool cookieUserDefined,
            const string &cookieUserDefinedString,
            DownloadMethodType downm_type_in,
            float speed_limit,
            bool use_command,
            bool use_exit_status,
            const Command &command
           );
    Options(const string &cwd);
    ~Options();
    void Change_Values(bool use_authentication_in,
                       const Userdata &user,
                       const string &store_dir_in,
                       const string &http_version_in,
                       PrewrittenHTMLType prewrittenHTMLType_in,
                       const string &prewrittenHTMLName_in,
                       bool sync_with_url,
                       RefererType referer_type_in,
                       const string &referer_in,
                       const string &useragent_in,
                       bool random_useragent_in,
                       bool use_http_proxy_in,
                       bool use_http_cache,
                       bool use_http_proxy_authentication,
                       const Userdata &http_proxy_user,
                       const Proxyserver &http_proxy_in,
                       unsigned int timedout_in,
                       unsigned int divide_in,
                       int rollback_bytes_in,
                       bool use_size_lower_limit,
                       int size_lower_limit,
                       bool use_no_redownload,
                       bool use_no_download_samename,
                       unsigned int recurse_in,
                       bool recurse_with_hostname_dir,
                       bool recurse_abs2rel,
                       bool recurse_force_convert,
                       bool recurse_del_comment,
                       bool recurse_del_javascript,
                       bool recurse_del_iframe,
                       bool recurse_no_other_host,
                       bool recurse_no_ascend,
                       bool recurse_relative_only,
                       bool recurse_referer_override,
                       bool recurse_follow_ftp_link,
                       bool recurse_convert_tilde,
                       bool recurse_no_download,
                       bool recurse_add_paste,
                       bool tag_href,
                       bool tag_src,
                       bool tag_background,
                       bool tag_code,
                       bool use_down_filter,
                       const list<string> &filter_down_target_list_in,
                       const list<string> &filter_nodown_target_list_in,
                       const list<string> &parse_target_list_in,
                       const list<string> &ign_domain_list_in,
                       FTP_Mode ftp_mode_in,
                       FTPretModeType ftp_ret_mode_in,
                       bool use_ftp_proxy_in,
                       bool use_ftp_proxy_authentication_in,
                       const Userdata &ftp_proxy_user_in,
                       const Proxyserver &ftp_proxy_in,
                       bool use_ftp_cache,
                       bool use_ftp_proxy_via_http,
                       FTPproxyLoginProcType ftp_proxy_login_proc,
                       bool ftp_nosend_quit,
                       bool ftpNoCwd,
                       int ftp_recurse_count,
                       bool ftp_use_filter,
                       bool ftp_allow_crawl_subdir,
                       bool ftp_no_ascend,
                       bool ftp_get_symlink_as_realfile,
                       bool ftp_recurse_add_paste,
                       const list<string> &ftp_filter_target_list,
                       bool delete_when_finish_in,
                       int retry_in,
                       int retry_repeat_in,
                       int retry_interval_in,
                       bool use_retry_404,
                       bool use_retry_503,
                       Status416HandlingType s416Handling,
                       bool use_no_redirection,
                       bool httpAcceptCompression,
                       bool httpAcceptLangEnabled,
                       const string &httpAcceptLangString,
                       bool dont_delete_without_crc_in,
                       bool no_crc_check_in,
                       bool ignore_crc_error_in,
                       bool use_content_md5,
                       bool cookie_delete_on_restart_in,
                       bool cookie_delete_nosend,
                       bool cookieUserDefined,
                       const string &cookieUserDefinedString,
                       DownloadMethodType downm_type_in,
                       float speed_limit,
                       bool use_command,
                       bool use_exit_status,
                       const Command &command
                      );
    bool Whether_use_authentication() const;
    void set_use_authentication(bool toggle);
    const string &ret_User() const;
    const string &ret_Password() const;
    void set_userpasswd(const Userdata &userdata);

    const string &ret_http_proxy_User() const;
    const string &ret_http_proxy_Password() const;

    bool ret_sync_with_URL() const;
    bool ret_use_no_download_samename() const;

    const string &ret_ftp_proxy_User() const;
    const string &ret_ftp_proxy_Password() const;
    void set_use_ftp_proxy(bool flag);
    bool ret_use_ftp_proxy() const;
    bool ret_use_ftp_proxy_authentication() const;
    const Proxyserver &ret_ftp_proxy() const;
    bool ret_use_ftp_cache() const;
    bool ret_use_ftp_proxy_via_http() const;
    FTPproxyLoginProcType ret_ftp_proxy_login_proc() const;

    // HTTP version
    const string &ret_HTTP_version() const;

    // Name of the file to use as a prewritten HTML file
    PrewrittenHTMLType getPrewrittenHTMLType() const;
    const string &getPrewrittenHTMLName() const;

    bool ret_HTTP_accept_compression() const;
    void set_HTTP_accept_compression(bool toggle);

    bool ret_HTTP_accept_lang_enabled() const;
    void set_HTTP_accept_lang_enalbed(bool toggle);
    const string &ret_HTTP_accept_lang_string() const;

    // Referer
    RefererType ret_Referer_Type() const;
    const string &ret_Referer() const;
    void set_Referer_Type(RefererType referer_type);
    void set_Referer(const string &referer);
    // user agent
    const string &ret_Useragent() const;
    bool ret_Random_useragent() const;
    // save directory
    const string &ret_Store_Dir() const;
    void set_Store_Dir(const string &store_dir_in);
    // time out
    unsigned int ret_Timed_Out() const;
    // split download
    unsigned int ret_Divide() const;
    void set_Divide(unsigned int divide_in);
    // rollback
    int ret_Rollback_bytes() const;
    void set_Roolback_bytes(int roolback_in);
    // no download if resume is not available
    bool ret_use_no_redownload() const;

    // recursive download
    unsigned int ret_recurse_count() const;
    void set_recurse_count(unsigned int n_rec);
    // FTP mode (passive or active)
    FTP_Mode ret_FTP_Mode() const;
    FTPretModeType ret_FTP_ret_mode() const;
    bool ret_FTP_nosend_quit() const;
    bool isFtpNoCwdEnabled() const;
    void setFtpNoCwd(bool toggle);
    unsigned int ret_FTP_recurse_count() const;
    void set_FTP_recurse_count(int count);
    bool ret_FTP_use_filter() const;
    void set_FTP_use_filter(bool toggle);
    bool ret_FTP_allow_crawl_subdir() const;
    void set_FTP_allow_crawl_subdir(bool toggle);
    bool ret_FTP_no_ascend() const;
    void set_FTP_no_ascend(bool toggle);
    bool ret_FTP_get_symlink_as_realfile() const;
    void set_FTP_get_symlink_as_realfile(bool toggle);
    bool ret_FTP_recurse_add_paste() const;
    void set_FTP_recurse_add_paste(bool toggle);
    const list<string> &ret_FTP_filter_target_list() const;
    const list<string> &ret_FTP_activated_filter_target_list() const;
    void set_FTP_activated_filter_target_list(const list<string> &target_list);
    void set_FTP_filter_target_list(const list<string> &target_list);
    bool Is_in_FTP_filter_target_list(const string &file) const;

    // retry count
    int ret_Retry() const;
    int ret_Retry_repeat() const;
    int ret_Retry_interval() const;
    bool ret_force_retry_404() const;
    bool ret_force_retry_503() const;
    Status416HandlingType ret_status_416_handling() const;
    bool ret_use_no_redirection() const;
    void set_use_http_proxy(bool flag);
    bool ret_use_http_proxy() const;
    bool ret_use_http_cache() const;
    bool ret_use_http_proxy_authentication() const;
    const Proxyserver &ret_http_proxy() const;
    bool ret_Delete_When_Finish() const;
    void set_Delete_When_Finish(bool toggle);
    void set_Dont_Delete_Without_CRC(bool toggle);
    bool ret_Dont_Delete_Without_CRC() const;
    bool ret_no_crc_checking() const;
    bool ret_ignore_crc_error() const;
    bool ret_use_content_md5() const;
    bool ret_Cookie_delete_on_restart() const;
    bool ret_Cookie_nosend() const;
    bool getCookieUserDefined() const;
    const string &getCookieUserDefinedString() const;
    bool ret_with_hostname_dir() const;
    void set_with_hostname_dir(bool toggle);
    bool ret_abs2rel_url() const;
    void set_abs2rel_url(bool toggle);
    bool ret_force_convert() const;
    void set_force_convert(bool toggle);
    bool ret_no_other_host() const;
    void set_no_other_host(bool toggle);
    bool ret_no_ascend() const;
    void set_no_ascend(bool toggle);
    bool ret_only_relative_links() const;
    void set_only_relative_links(bool toggle);
    bool ret_delete_comment() const;
    void set_delete_comment(bool toggle);
    bool ret_delete_javascript() const;
    void set_delete_javascript(bool toggle);
    bool ret_delete_iframe() const;
    void set_delete_iframe(bool toggle);
    bool ret_Referer_override() const;
    void set_Referer_override(bool toggle);
    bool ret_Follow_ftp_link() const;
    void set_Follow_ftp_link(bool toggle);
    bool ret_convert_tilde() const;
    void set_convert_tilde(bool toggle);
    bool ret_no_redownload_HTTP_recurse() const;
    void set_no_redownload_HTTP_recurse(bool toggle);
    bool ret_HTTP_recurse_add_paste() const;
    void set_HTTP_recurse_add_paste(bool toggle);
    bool Is_in_parse_target_list(string extension) const;
    bool Is_in_filter_nodown_target_list(string extension) const;
    bool Is_in_activated_parse_target_list(const string &file) const;
    bool Is_in_activated_filter_down_target_list(const string &file) const;
    bool Is_in_activated_filter_nodown_target_list(const string &file) const;
    bool Is_in_activated_ign_domain_list(const string &file) const;
    const list<string> &ret_parse_target_list() const;
    const list<string> &ret_filter_down_target_list() const;
    const list<string> &ret_filter_nodown_target_list() const;
    const list<string> &ret_ign_domain_list() const;
    const list<string> &ret_activated_parse_target_list() const;
    const list<string> &ret_activated_filter_down_target_list() const;
    const list<string> &ret_activated_filter_nodown_target_list() const;
    const list<string> &ret_activated_ign_domain_list() const;
    void set_parse_target_list(const list<string> &target_list);
    void set_filter_down_target_list(const list<string> &target_list);
    void set_filter_nodown_target_list(const list<string> &target_list);
    void set_ign_domain_list(const list<string> &target_list);
    void set_activated_parse_target_list(const list<string> &target_list);
    void set_activated_filter_down_target_list(const list<string> &target_list);
    void set_activated_filter_nodown_target_list(const list<string> &target_list);
    void set_activated_ign_domain_list(const list<string> &target_list);

    void activate_parse_target_list(const list<string> &target_list);
    void activate_filter_down_target_list(const list<string> &target_list);
    void activate_filter_nodown_target_list(const list<string> &target_list);
    void activate_ign_domain_list(const list<string> &target_list);
    void set_use_down_filter(bool toggle);
    bool ret_use_down_filter() const;
    DownloadMethodType ret_downm_type() const;
    void set_downm_type(DownloadMethodType downm_type);
    bool ret_use_tag_href() const;
    bool ret_use_tag_src() const;
    bool ret_use_tag_background() const;
    bool ret_use_tag_code() const;
    void set_use_tag_href(bool toggle);
    void set_use_tag_src(bool toggle);
    void set_use_tag_background(bool toggle);
    void set_use_tag_code(bool toggle);

    bool ret_use_Command() const;
    bool ret_use_Exit_status() const;
    const Command &ret_Command() const;

    float ret_speed_limit() const;
    void set_speed_limit(float speed);

    bool ret_use_size_lower_limit() const;
    int ret_size_lower_limit() const;
};

#endif // _OPTIOINS_H_
