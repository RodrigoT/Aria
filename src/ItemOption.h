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

// $Id: ItemOption.h,v 1.43 2002/04/03 13:33:52 tujikawa Exp $

#ifndef _ITEMOPTION_H_
#define _ITEMOPTION_H_

#include <gtk/gtk.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <algorithm>
#include "aria.h"
#include "ProxyList.h"
#include "UseragentList.h"
#include "ListManager.h"
#include "utils.h"
#include "ItemCell.h"
#include "Dialog.h"
#include "PasteWindow.h"
using namespace std;

#define DEFAULT_HTTP_PROXY_PORT 8080
#define DEFAULT_FTP_PROXY_PORT 8021

enum OptionErrorType {
    OPTION_INVALID_DIR,
    OPTION_INVALID_URL,
    OPTION_INVALID_FILENAME,
    OPTION_INVALID_CRC,
    OPTION_INVALID_MD5,
    OPTION_SUCCESS
};

class ItemOption
{
private:
    ItemCell *itemcell;
    ListEntry *listentry;
    // option window
    GtkWidget *option_window;
    GtkWidget *notebook;
    GtkWidget *general_notebook;
    GtkWidget *http_notebook;
    GtkWidget *ftp_notebook;

    // URL page
    GtkWidget *url_entry, *crc_entry;
    GtkWidget *md5_entry;
    GtkWidget *filename_entry;
    GtkWidget *toggle_use_authentication;
    GtkWidget *user_entry, *password_entry;
    GtkWidget *storedir_entry;
    GtkWidget *sync_with_url_toggle;
    int sync_with_url_toggled_cb_id;

    // Command page
    GtkWidget *use_command_toggle;
    GtkWidget *use_exit_status_toggle;
    GtkWidget *command_entry;
    GtkWidget *exit_status_entry;

    // Cookie page
    GtkWidget *cookie_delete_on_restart_toggle;
    GtkWidget *cookie_nosend_toggle;
    GtkWidget *cookieUserDefinedToggle;
    GtkWidget *cookieUserDefinedEntry;

    // HTTP1 page
    GtkWidget *referer_entry, *timedout_entry;
    GtkWidget *none_button, *index_button, *user_button, *urlref_button;

    GtkWidget *useifmodsince_toggle;
    GtkWidget *noresume_toggle;
    GtkWidget *alwaysresume_toggle;
    GtkWidget *use_retry_404_toggle;
    GtkWidget *use_retry_503_toggle;
    GtkWidget *s416SuccButton, *s416ErrButton, *s416RedownButton;
    GtkWidget *use_no_redirection_toggle;
    GtkWidget *httpAcceptCompressionToggle;
    GtkWidget *httpAcceptLangEnabledToggle;
    GtkWidget *httpAcceptLangStringEntry;
    GtkWidget *http_version_select;
    GtkWidget *prewrittenHTMLIndexButton;
    GtkWidget *prewrittenHTMLUserDefinedButton;
    GtkWidget *prewrittenHTMLUserDefinedEntry;

    // HTTP2 page
    GtkWidget *useragent_cbox;
    GtkWidget *toggle_random_useragent;
    GtkWidget *toggle_use_http_proxy_authentication;
    GtkWidget *http_proxy_user_entry, *http_proxy_password_entry;
    GtkWidget *http_proxy_cbox;
    GtkWidget *toggle_use_http_cache;
    GtkWidget *http_proxy_add, *http_proxy_delete;
    GtkWidget *use_http_proxy_toggle;

    // FTP1 page
    GtkWidget *active_button, *passive_button;
    GtkWidget *ftp_nosend_quit_toggle;
    GtkWidget *ftpNoCwdToggle;
    GtkWidget *ftp_recurse_depth_spin;
    GtkWidget *ftp_no_ascend_toggle;
    GtkWidget *ftp_get_symlink_as_realfile_toggle;
    GtkWidget *ftpRecurseAddPasteToggle;
    GtkWidget *ftp_use_filter_toggle;
    GtkWidget *ftp_allow_crawl_toggle;
    GtkWidget *ftp_filter_entry;
    GtkWidget *ftp_filter_clist;
    GtkWidget *binary_button, *ascii_button;

    // FTP2 page
    GtkWidget *toggle_use_ftp_proxy_authentication;
    GtkWidget *ftp_proxy_user_entry, *ftp_proxy_password_entry;
    GtkWidget *ftp_proxy_cbox;
    GtkWidget *toggle_use_ftp_cache;
    GtkWidget *ftp_proxy_add, *ftp_proxy_delete;
    GtkWidget *use_ftp_proxy_toggle;
    GtkWidget *use_ftp_cache_toggle;
    GtkWidget *use_ftp_proxy_via_http_toggle;
    GtkWidget *ftp_proxy_login_proc_cbox;

    // DOWNLOAD1 page
    GtkWidget *toggle_delete_when_finish;
    GtkWidget *toggle_dont_delete_without_crc;
    GtkWidget *no_crc_check_toggle;
    GtkWidget *ignore_crc_error_toggle;
    GtkWidget *use_content_md5_toggle;
    GtkWidget *spin_timedout, *spin_divide;
    GtkWidget *spin_retry, *spin_retry_push_back;
    GtkWidget *spin_retry_interval;
    GtkWidget *rollback_bytes_entry;
    GtkWidget *use_size_lower_limit_toggle;
    GtkWidget *size_lower_limit_entry;
    GtkWidget *speed_scale;
    GtkWidget *speedSpin;
    GtkWidget *use_no_redownload_toggle;
    GtkWidget *use_no_download_samename_toggle;

    // HTTP3 page
    GtkWidget *documentroot_dir_entry;
    GtkWidget *recurse_depth_spin;

    GtkWidget *recurse_hostname_dir_toggle;
    GtkWidget *recurse_abs2rel_toggle;
    GtkWidget *recurse_force_convert_toggle;
    GtkWidget *recurse_del_comment_toggle;
    GtkWidget *recurse_del_javascript_toggle;
    GtkWidget *recurse_del_iframe_toggle;
    GtkWidget *recurse_no_other_host_toggle;
    GtkWidget *recurse_no_ascend_toggle;
    GtkWidget *recurse_relative_only_toggle;
    GtkWidget *recurse_referer_override_toggle;
    GtkWidget *recurse_follow_ftp_link_toggle;
    GtkWidget *recurse_convert_tilde_toggle;
    GtkWidget *recurse_no_redownload_toggle;
    GtkWidget *recurseAddPasteToggle;

    GtkWidget *recurse_tag_href_toggle;
    GtkWidget *recurse_tag_src_toggle;
    GtkWidget *recurse_tag_background_toggle;
    GtkWidget *recurse_tag_code_toggle;

    GtkWidget *recurse_parse_target_entry;
    GtkWidget *recurse_nodown_filter_target_entry;
    GtkWidget *recurse_parse_target_clist;
    GtkWidget *recurse_nodown_filter_target_clist;
    GtkWidget *recurse_ign_domain_entry;
    GtkWidget *recurse_ign_domain_clist;

    GtkWidget *recurse_down_filter_target_entry;
    GtkWidget *recurse_down_filter_target_clist;
    GtkWidget *recurse_use_down_filter_toggle;

    bool multipleSelectionFlag;
    bool visibleFlag;
public:
    ItemOption(GtkWidget *app_window);//constructor

    void setOptionValues(ItemCell *itemcell,
                         const Options &options,
                         ListEntry *listentry = NULL);
    void show();
    void hide();

    int Process_changes();
    GtkWidget *getWindow() const;
    ItemCell *ret_ItemCell() const;
    ListEntry *getListEntry() const;

    // general options
    GtkWidget *Create_General_page();
    GtkWidget *Create_General_URL_page();
    GtkWidget *Create_General_Authentication_page();
    GtkWidget *Create_General_Download1_page();
    GtkWidget *Create_General_Download2_page();
    GtkWidget *Create_General_Command_page();

    // HTTP download options
    GtkWidget *Create_HTTP_page();
    GtkWidget *Create_HTTP_Referer_page();
    GtkWidget *Create_HTTP_Cookie_page();
    GtkWidget *Create_HTTP_Agent_page();
    GtkWidget *Create_HTTP_Proxy_page();
    GtkWidget *Create_HTTP_Recursive1_page();
    GtkWidget *Create_HTTP_Recursive2_page();
    GtkWidget *Create_HTTP_Misc_page();

    // FTP download options
    GtkWidget *Create_FTP_page();
    GtkWidget *Create_FTP_Mode_page();
    GtkWidget *Create_FTP_Proxy_page();
    GtkWidget *Create_FTP_Recursive_page();
    GtkWidget *Create_FTP_Misc_page();

    void set_http_proxy_list(const ProxyList *proxy_list);
    void set_ftp_proxy_list(const ProxyList *proxy_list);
    void set_useragent_list(const UseragentList *useragent_list);
    void set_Store_Dir(const char *dir);
    void setSensitiveHTTPRecursive();

    // return true if option window is visible
    bool isVisible() const;

    // callback functions
    void speedScaleMotionNotifyEvent();
    void speedScaleButtonReleaseEvent();
    void speedSpinChanged();
};

#endif // _ITEMOPTION_H_
