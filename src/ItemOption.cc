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

// $Id: ItemOption.cc,v 1.73 2002/10/01 15:32:00 tujikawa Exp $

// implementation of class ItemOption

#include "ItemOption.h"
#include "AppOption.h"

extern void Send_report(MessageType reporttype, ItemStatus *itemstatus, ListEntry *listentry);
extern GtkWidget *Create_CheckCList(GtkWidget **clist_ptr, char *titles[], int n_titles);
extern GtkWidget *Create_popup_menu(GtkWidget **clist_ptr);
extern void create_default_filter_nodown_target_list(list<string>& filter_nodown_target_list);
extern void create_default_parse_target_list(list<string>& parse_target_list);
extern void Download_change_speed_sub(ItemCell *itemcell, float fspeed);
extern GtkWidget *create_dir_browser(const char* title, const char* current_path, GtkSelectionMode mode, void (*handler) (char*));

extern ItemCell *g_consoleItem;
extern ListManager *g_listManager;
extern ItemOption *g_itemOption;
extern ItemManager *g_itemManagerPaste;
extern AppOption *g_appOption;
extern ProxyList *g_httpProxyList;
extern ProxyList *g_ftpProxyList;

extern UseragentList *g_userAgentList;
extern Dialog *g_cDialog;
extern PasteWindow *g_pasteWindow;

static ProxyList *sg_httpProxyListTemp = NULL;
static ProxyList *sg_ftpProxyListTemp = NULL;

static GtkWidget *store_dir_browser = NULL;
static string current_store_dir;
static string temp_storedir;
static Dialog *edialog = NULL;
//static float speed_value = 0.0;
static const char *http_version_string[] = {
  "1.0",
  "1.1"
};

#define DFTPPROXY_PL "USER proxyuser@host; PASS proxypass"
#define DFTPPROXY_PL_OPEN "USER proxyuser; PASS proxypass; OPEN host"
#define DFTPPROXY_PL_OPEN2 "USER proxyuser; PASS proxypass; open host"
#define DFTPPROXY_PL_SITE "USER proxyuser; PASS proxypass; SITE host"
#define DFTPPROXY_PL_USER "USER proxyuser; PASS proxypass; USER user@host"
#define DFTPPROXY_USER "USER user@host"
#define DFTPPROXY_OPEN "OPEN host"

// gtk check button hack, written by Adam Purkrt
GtkWidget*
gtk_check_button_new_with_llabel (const gchar *label)
{
  GtkWidget *check_button;
  GtkWidget *label_widget;
  
  check_button = gtk_check_button_new ();
  label_widget = gtk_label_new (label);
  gtk_misc_set_alignment (GTK_MISC (label_widget), 0.0, 0.5);
  gtk_label_set_justify (GTK_LABEL (label_widget), GTK_JUSTIFY_LEFT);
  
  gtk_container_add (GTK_CONTAINER (check_button), label_widget);
  gtk_widget_show (label_widget);
  
  return check_button;
} 

GtkWidget *HBOX_NEW(GtkWidget *vbox)
{
  GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
  
  return hbox;
}

void get_extension_target_clist(GtkWidget *clist,
				list<string>& target_list,
				list<string>& activated_target_list)
{
  target_list.clear();
  activated_target_list.clear();
  for(int rowindex = 0; rowindex < GTK_CLIST(clist)->rows; ++rowindex) {
    char *extension_c_str;
    gtk_clist_get_text(GTK_CLIST(clist), rowindex, 0, &extension_c_str);
    target_list.push_back(extension_c_str);
  }
  GList *node = GTK_CLIST(clist)->selection;
  while(node) {
    int rowindex = GPOINTER_TO_INT(node->data);
    char *extension_c_str;
    gtk_clist_get_text(GTK_CLIST(clist), rowindex, 0, &extension_c_str);
    activated_target_list.push_back(extension_c_str);
    node = g_list_next(node);
  }
}

void setup_extension_target_clist(GtkWidget *clist,
				  const list<string>& target_list,
				  const list<string>& activated_target_list)
{
  gtk_clist_clear(GTK_CLIST(clist));
  for(list<string>::const_iterator itr = target_list.begin(); itr != target_list.end(); ++itr) {
    char *title[1];
    title[0] = strdup(itr->c_str());
    int rowindex = gtk_clist_append(GTK_CLIST(clist), title);
    delete [] title[0];
    for(list<string>::const_iterator act_itr = activated_target_list.begin(); act_itr != activated_target_list.end(); ++act_itr) {
      if(*itr == *act_itr) {
	gtk_clist_select_row(GTK_CLIST(clist), rowindex, 0);
	break;
      }
    }
  }
}

static GList *list2glist(const list<Proxyserver*>& proxy_list)
{
  GList *proxy_glist = NULL;

  for(list<Proxyserver*>::const_iterator itr = proxy_list.begin(); itr != proxy_list.end(); ++itr) {
    char *entry_string = g_strconcat((*itr)->ret_Server().c_str(), ":", itos((*itr)->ret_Port()).c_str(), NULL);
    proxy_glist = g_list_append(proxy_glist, entry_string);
  }
  return proxy_glist;
}

static GList *vector2glist(const vector<string>& useragent_list)
{
  GList *useragent_glist = NULL;

  for(vector<string>::const_iterator itr = useragent_list.begin(); itr != useragent_list.end(); ++itr) {
    useragent_glist = g_list_append(useragent_glist, strdup((*itr).c_str()));
  }
  return useragent_glist;
}

static void Add_common_proxy_entry(const string& proxy_entry,
				   int proxyserver_port,
				   ProxyList *proxy_list_temp,
				   GtkWidget *proxy_cbox)
{
  unsigned int colon_pos;
  string proxyserver_name;

  if(string::npos != (colon_pos = proxy_entry.find(':')) && proxy_entry.substr(colon_pos).size() > 1) {
    proxyserver_name = proxy_entry.substr(0, colon_pos);
    proxyserver_port = stoi(proxy_entry.substr(colon_pos+1), 10);
  } else {
    proxyserver_name = proxy_entry;
  }

  Proxyserver proxy(proxyserver_name, proxyserver_port);

  if(proxy_list_temp->add(proxy)) {
    GList *proxy_glist = list2glist(proxy_list_temp->ret_list());
    gtk_combo_set_popdown_strings(GTK_COMBO(proxy_cbox), proxy_glist);
    GList *node = g_list_first(proxy_glist);
    while(node) {
      //delete [] (char *)node->data;
      g_free(node->data);
      node = g_list_next(node);
    }
    g_list_free(proxy_glist);
    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(proxy_cbox)->entry), proxy_entry.c_str());
  }
}

static void Add_ftp_proxy_entry(GtkWidget *w, GtkWidget *ftp_proxy_cbox)
{
  string proxy_entry = Remove_white(gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(ftp_proxy_cbox)->entry)));
  //int proxyserver_port = DEFAULT_PROXY_PORT;// 8021??

  Add_common_proxy_entry(proxy_entry, DEFAULT_FTP_PROXY_PORT, sg_ftpProxyListTemp,
			 ftp_proxy_cbox);
}
  
static void Add_http_proxy_entry(GtkWidget *w, GtkWidget *http_proxy_cbox)
{
  string proxy_entry = Remove_white(gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(http_proxy_cbox)->entry)));
  //unsigned int colon_pos;
  //string proxyserver_name;
  //int proxyserver_port = DEFAULT_PROXY_PORT;

  Add_common_proxy_entry(proxy_entry, DEFAULT_HTTP_PROXY_PORT, sg_httpProxyListTemp,
			 http_proxy_cbox);
  /*
  if(string::npos != (colon_pos = proxy_entry.find(':')) && proxy_entry.substr(colon_pos).size() > 1) {
    proxyserver_name = proxy_entry.substr(0, colon_pos);
    proxyserver_port = stoi(proxy_entry.substr(colon_pos+1), 10);
  } else {
    proxyserver_name = proxy_entry;
  }

  Proxyserver http_proxy(proxyserver_name, proxyserver_port);

  if(sg_httpProxyListTemp->add(http_proxy)) {
    GList *proxy_glist = list2glist(sg_httpProxyListTemp->ret_list());
    gtk_combo_set_popdown_strings(GTK_COMBO(http_proxy_cbox), proxy_glist);
    GList *node = g_list_first(proxy_glist);
    while(node) {
      //delete [] (char *)node->data;
      g_free(node->data);
      node = g_list_next(node);
    }
    g_list_free(proxy_glist);
    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(http_proxy_cbox)->entry), proxy_entry.c_str());
  }
  */
}

static void Delete_common_proxy_entry(const string& proxy_entry,
				   int proxyserver_port,
				   ProxyList *proxy_list_temp,
				   GtkWidget *proxy_cbox)
{
  unsigned int colon_pos;
  string proxyserver_name;

  if(string::npos != (colon_pos = proxy_entry.find(':')) && proxy_entry.substr(colon_pos).size() > 1) {
    proxyserver_name = proxy_entry.substr(0, colon_pos);
    proxyserver_port = stoi(proxy_entry.substr(colon_pos+1), 10);
  } else {
    proxyserver_name = proxy_entry;
  }

  Proxyserver proxy(proxyserver_name, proxyserver_port);

  proxy_list_temp->remove(proxy);
  GList *proxy_glist = list2glist(proxy_list_temp->ret_list());
  
  if(proxy_glist == NULL) {
    GtkWidget *list = GTK_COMBO(proxy_cbox)->list;
    gtk_list_clear_items(GTK_LIST(list), 0, -1);
  } else {
    gtk_combo_set_popdown_strings(GTK_COMBO(proxy_cbox), proxy_glist);
  }
  GList *node = g_list_first(proxy_glist);
  while(node) {
    //delete [] (char *)node->data;
    g_free(node->data);
    node = g_list_next(node);
  }
  g_list_free(proxy_glist);
  gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(proxy_cbox)->entry), "");
}

static void Delete_ftp_proxy_entry(GtkWidget *w, GtkWidget *ftp_proxy_cbox)
{
  string proxy_entry = Remove_white(gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(ftp_proxy_cbox)->entry)));
  //unsigned int colon_pos;
  //string proxyserver_name;
  //int proxyserver_port = DEFAULT_PROXY_PORT;
  Delete_common_proxy_entry(proxy_entry, DEFAULT_FTP_PROXY_PORT, sg_ftpProxyListTemp,
			    ftp_proxy_cbox);

}

static void Delete_http_proxy_entry(GtkWidget *w, GtkWidget *http_proxy_cbox)
{
  string proxy_entry = Remove_white(gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(http_proxy_cbox)->entry)));
  //unsigned int colon_pos;
  //string proxyserver_name;
  //int proxyserver_port = DEFAULT_PROXY_PORT;
  Delete_common_proxy_entry(proxy_entry, DEFAULT_HTTP_PROXY_PORT, sg_httpProxyListTemp,
			    http_proxy_cbox);

  /*
  if(string::npos != (colon_pos = proxy_entry.find(':')) && proxy_entry.substr(colon_pos).size() > 1) {
    proxyserver_name = proxy_entry.substr(0, colon_pos);
    proxyserver_port = stoi(proxy_entry.substr(colon_pos+1), 10);
  } else {
    proxyserver_name = proxy_entry;
  }

  Proxyserver http_proxy(proxyserver_name, proxyserver_port);

  sg_httpProxyListTemp->remove(http_proxy);
  GList *proxy_glist = list2glist(sg_httpProxyListTemp->ret_list());
  
  if(proxy_glist == NULL) {
    GtkWidget *list = GTK_COMBO(http_proxy_cbox)->list;
    gtk_list_clear_items(GTK_LIST(list), 0, -1);
  } else {
    gtk_combo_set_popdown_strings(GTK_COMBO(http_proxy_cbox), proxy_glist);
  }
  GList *node = g_list_first(proxy_glist);
  while(node) {
    //delete [] (char *)node->data;
    g_free(node->data);
    node = g_list_next(node);
  }
  g_list_free(proxy_glist);
  gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(http_proxy_cbox)->entry), "");
  */
}

static gboolean Error_OK(GtkWidget *w, GtkWidget *window)
{
  edialog->hide();
  return TRUE;
}

static void Show_error_dialog(const string& error_message, GtkWidget *option_window)
{
  if(edialog == NULL) {
    edialog = new Dialog(GTK_WINDOW(option_window));
  }

  edialog->setup(_("Error"),
		 error_message,
		 Error_OK);
  edialog->set_no_button_visible(false);
  edialog->set_cancel_button_visible(false);
  edialog->set_transient(GTK_WINDOW(option_window));
  edialog->show();
}

static void store_dir_handler(char *dir)
{
  g_itemOption->set_Store_Dir(dir);
}

static gboolean Mkdir_OK(GtkWidget *w, GtkWidget *window)
{
  g_cDialog->hide();

  if(temp_storedir.size() && temp_storedir.at(temp_storedir.size()-1) == '/') {
    temp_storedir.erase(temp_storedir.size()-1);
  }
  string dir_to_make = Token_splitter(temp_storedir, "/");

  while(temp_storedir.size()) {
    dir_to_make += '/'+Token_splitter(temp_storedir, "/");
    struct stat dir_stat;
    if(stat(dir_to_make.c_str(), &dir_stat) == -1 || !S_ISDIR(dir_stat.st_mode)) {
      if(mkdir(dir_to_make.c_str(), 0755) < 0) {
	Show_error_dialog(_("Failed to make directory '")+dir_to_make+"'\nreason:"+strerror(errno), g_itemOption->getWindow());
	return FALSE;
      }
    }
  }
  return TRUE;
}

static gboolean Mkdir_NO(GtkWidget *w, GtkWidget *window)
{
  g_cDialog->hide();

  return TRUE;
}

static void Ask_mkdir_dialog(GtkWidget *option_window)
{
  g_cDialog->setup(_("Error"),
		 _("Specified directory does not exist.\nDo you wish to make it now?"),
		 Mkdir_OK,
		 Mkdir_NO,
		 Mkdir_NO);
  g_cDialog->set_cancel_button_visible(false);
  g_cDialog->set_transient(GTK_WINDOW(option_window));
  g_cDialog->show();
}

static gboolean Show_dir_browser(GtkWidget *w, GtkWidget *option_window)
{
  if(store_dir_browser == NULL) {
    store_dir_browser = create_dir_browser(_("Select directory"), current_store_dir.c_str(), GTK_SELECTION_SINGLE, store_dir_handler);
    
    g_signal_connect(GTK_OBJECT(store_dir_browser), "destroy",
		       GTK_SIGNAL_FUNC(gtk_widget_destroyed),
		       (void *)&store_dir_browser);
    
    gtk_window_set_modal(GTK_WINDOW(store_dir_browser), TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(store_dir_browser),
				 GTK_WINDOW(option_window));
  }

  return TRUE;
}

static void Option_OK(GtkWidget *w, ItemOption *itemoption)
{
  switch(itemoption->Process_changes()) {
  case OPTION_INVALID_DIR:
    Ask_mkdir_dialog(itemoption->getWindow());
    break;
  case OPTION_INVALID_URL:
    Show_error_dialog(_("Invalid URL"), itemoption->getWindow());
    break;
  case OPTION_INVALID_FILENAME:
    Show_error_dialog(_("Invalid Filename"), itemoption->getWindow());
    break;
  case OPTION_INVALID_CRC:
    Show_error_dialog(_("Invalid CRC"), itemoption->getWindow());
    break;
  case OPTION_INVALID_MD5:
    Show_error_dialog(_("Invalid MD5"), itemoption->getWindow());
    break;
  default:
    //gtk_widget_hide(itemoption->getWindow());
    itemoption->hide();
    break;
  }
}

//static void Option_cancel(GtkWidget *w, ItemOption *iop)
static void Option_cancel(GtkObject *obj)
{
  ItemOption *iop = (ItemOption *)obj;
  //gtk_widget_hide(option_window);

  iop->hide();
}

static void Option_default(GtkWidget *w, ItemOption *itemoption)
{
  ItemCell *itemcell = itemoption->ret_ItemCell();
  ListEntry *listentry = itemoption->getListEntry();
  //pthread_mutex_lock(&itemlistlock); //mod 2001/4/11
  if(itemcell != g_consoleItem &&
     (!g_listManager->Search(listentry) || !listentry->getItemManager()->search_item(itemcell))
     && !g_itemManagerPaste->search_item(itemcell)) {
    //pthread_mutex_unlock(&itemlistlock); //mod 2001/4/11
    //gtk_widget_hide(itemoption->getWindow());
    itemoption->hide();
    return;
  }

  if(itemcell == g_consoleItem) {
    char *current_dir = g_get_current_dir();
    Proxyserver proxy;
    Userdata user(DEFAULT_OPTION_USER, DEFAULT_OPTION_PASSWORD);
    Userdata http_proxy_user(DEFAULT_HTTP_PROXY_USER, DEFAULT_HTTP_PROXY_PASSWORD);
    Userdata ftp_proxy_user(DEFAULT_FTP_PROXY_USER, DEFAULT_FTP_PROXY_PASSWORD);
    list<string> filter_down_target_list;
    list<string> filter_nodown_target_list;
    create_default_filter_nodown_target_list(filter_nodown_target_list);
    list<string> parse_target_list;
    create_default_parse_target_list(parse_target_list);
    list<string> ign_domain_list;
    list<string> ftp_filter_target_list;
    Command command;
    Options options_temp(DEFAULT_USE_AUTHENTICATION,
			 user,
			 current_dir,
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
			 http_proxy_user,
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
			 filter_down_target_list,
			 filter_nodown_target_list,
			 parse_target_list,
			 ign_domain_list,
			 DEFAULT_FTP_MODE,
			 DEFAULT_FTP_RET_MODE,
			 DEFAULT_USE_FTP_PROXY,
			 DEFAULT_USE_FTP_PROXY_AUTHENTICATION,
			 ftp_proxy_user,
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
			 ftp_filter_target_list,
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
    //delete [] current_dir;
    g_free(current_dir);
    options_temp.activate_filter_nodown_target_list(filter_nodown_target_list);
    options_temp.activate_parse_target_list(parse_target_list);
    itemoption->setOptionValues(g_consoleItem, options_temp,
				itemoption->getListEntry());
  } else {
    ListEntry *listentry = itemoption->getListEntry();
    // fix this
    // it may be user friendly to show some kind of dialog
    if(g_listManager->Search(listentry)) {
      itemoption->setOptionValues(itemcell, listentry->ret_Options(), listentry);
    }
  }
  //pthread_mutex_unlock(&itemlistlock); //mod 2001/4/11
}

static void HTTPRecursive_toggled_event_cb(GtkWidget *toggle, ItemOption *iop) {
  iop->setSensitiveHTTPRecursive();
}

//  static gboolean Option_quit(GtkWidget* w, ItemOption *iop)
//  {
//    //gtk_widget_hide(w);
//    iop->hide();

//    return(TRUE);
//  }

ItemOption::ItemOption(GtkWidget* app_window)
{
  option_window = gtk_dialog_new();
  gtk_widget_set_usize(GTK_WIDGET(option_window), 685, 600);
  g_signal_connect_swapped(GTK_OBJECT(option_window),
		     "delete_event",
		     GTK_SIGNAL_FUNC(Option_cancel),
		     (GtkObject *)this);
  gtk_window_set_modal(GTK_WINDOW(option_window), TRUE);
  gtk_window_set_transient_for(GTK_WINDOW(option_window), GTK_WINDOW(app_window));
  gtk_widget_realize(option_window);
  visibleFlag = false;

  GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
  gtk_widget_show(hbox);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(option_window)->vbox),
		     hbox, TRUE, TRUE, 10);

  notebook = gtk_notebook_new();
  gtk_notebook_set_scrollable(GTK_NOTEBOOK(notebook), TRUE);//added 2001/3/18
  gtk_notebook_popup_enable(GTK_NOTEBOOK(notebook)); //added 2001/3/18

  gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_TOP);
  gtk_box_pack_start(GTK_BOX(hbox),
		     notebook, TRUE, TRUE, 10);
  gtk_widget_show(notebook);

  {
    GtkWidget *tab_label = gtk_label_new(_("General"));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
			     Create_General_page(),
			     tab_label);
  }
  {
    GtkWidget *tab_label = gtk_label_new("HTTP/HTTPS");
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
			     Create_HTTP_page(),
			     tab_label);
  }
  {
    GtkWidget *tab_label = gtk_label_new("FTP");
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
			     Create_FTP_page(),
			     tab_label);
  }
  /*
  GtkWidget *tab1_label = gtk_label_new("URL");
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
			   Create_URL_page(),
			   tab1_label);
  GtkWidget *tab2_label = gtk_label_new(_("DOWNLOAD1"));
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
			   Create_Download1_page(),
			   tab2_label);

  GtkWidget *tab3_label = gtk_label_new(_("HTTP1"));
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
			   Create_HTTP1_page(),
			   tab3_label);

  GtkWidget *tab4_label = gtk_label_new("HTTP2");
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
			   Create_HTTP2_page(),
			   tab4_label);
  
  GtkWidget *tab5_label = gtk_label_new("HTTP3");
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
			   Create_HTTP3_page(),
			   tab5_label);

  GtkWidget *tab6_label = gtk_label_new("FTP1");
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
			   Create_FTP1_page(),
			   tab6_label);

  GtkWidget *tab7_label = gtk_label_new("FTP2");
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
			   Create_FTP2_page(),
			   tab7_label);

  GtkWidget *tab8_label = gtk_label_new("Command");
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
			   Create_Command_page(),
			   tab8_label);
  */
  //// action area
  GtkWidget *bbox = gtk_hbutton_box_new();
  gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_END);
  gtk_button_box_set_spacing(GTK_BUTTON_BOX(bbox), 5);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(option_window)->action_area),
		     bbox, FALSE, FALSE, 0);
  gtk_widget_show(bbox);
  // OK button
  GtkWidget *OK_button = gtk_button_new_with_label(_("OK"));
  GTK_WIDGET_SET_FLAGS(OK_button, GTK_CAN_DEFAULT);
  gtk_window_set_default(GTK_WINDOW(option_window), OK_button);
  g_signal_connect(GTK_OBJECT(OK_button),
		     "clicked",
		     GTK_SIGNAL_FUNC(Option_OK),
		     (void *)this);
  gtk_box_pack_start(GTK_BOX(bbox),
		     OK_button, TRUE, TRUE, 0);
  gtk_widget_show(OK_button);
  // Cancel button
  GtkWidget *Cancel_button = gtk_button_new_with_label(_("Cancel"));
  //GTK_WIDGET_SET_FLAGS(Cancel_button, GTK_CAN_DEFAULT);
  g_signal_connect_swapped(GTK_OBJECT(Cancel_button),
		     "clicked",
		     GTK_SIGNAL_FUNC(Option_cancel),
		     (GtkObject *)this);
  gtk_box_pack_start(GTK_BOX(bbox),
		     Cancel_button, TRUE, TRUE, 0);
  gtk_widget_show(Cancel_button);
  // default button
  GtkWidget *Default_button = gtk_button_new_with_label(_("Default"));
  g_signal_connect(GTK_OBJECT(Default_button),
		     "clicked",
		     GTK_SIGNAL_FUNC(Option_default),
		     (void *)this);
  gtk_box_pack_start(GTK_BOX(bbox),
		     Default_button, TRUE, TRUE, 0);
  gtk_widget_show(Default_button);
}

GtkWidget *ItemOption::Create_General_page()
{
  GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
  gtk_widget_show(hbox);
  GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
  gtk_widget_show(vbox);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 10);

  general_notebook = gtk_notebook_new();
  gtk_notebook_set_scrollable(GTK_NOTEBOOK(general_notebook), TRUE);
  gtk_notebook_popup_enable(GTK_NOTEBOOK(general_notebook));
  gtk_notebook_set_tab_pos(GTK_NOTEBOOK(general_notebook), GTK_POS_TOP);
  gtk_box_pack_start(GTK_BOX(vbox),
		     general_notebook, TRUE, TRUE, 10);
  gtk_widget_show(general_notebook);
  {
    GtkWidget *tab_label = gtk_label_new(_("URL"));
    gtk_notebook_append_page(GTK_NOTEBOOK(general_notebook),
			     Create_General_URL_page(),
			     tab_label);
  }
  {
    GtkWidget *tab_label = gtk_label_new(_("Authentication"));
    gtk_notebook_append_page(GTK_NOTEBOOK(general_notebook),
			     Create_General_Authentication_page(),
			     tab_label);
  }
  {
    GtkWidget *tab_label = gtk_label_new(_("Download 1"));
    gtk_notebook_append_page(GTK_NOTEBOOK(general_notebook),
			     Create_General_Download1_page(),
			     tab_label);
  }
  {
    GtkWidget *tab_label = gtk_label_new(_("Download 2"));
    gtk_notebook_append_page(GTK_NOTEBOOK(general_notebook),
			     Create_General_Download2_page(),
			     tab_label);
  }
  {
    GtkWidget *tab_label = gtk_label_new(_("Command"));
    gtk_notebook_append_page(GTK_NOTEBOOK(general_notebook),
			     Create_General_Command_page(),
			     tab_label);
  }
  return hbox;
}

GtkWidget *ItemOption::Create_HTTP_page()
{
  GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
  gtk_widget_show(hbox);
  GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
  gtk_widget_show(vbox);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 10);

  http_notebook = gtk_notebook_new();
  gtk_notebook_set_scrollable(GTK_NOTEBOOK(http_notebook), TRUE);
  gtk_notebook_popup_enable(GTK_NOTEBOOK(http_notebook));
  gtk_notebook_set_tab_pos(GTK_NOTEBOOK(http_notebook), GTK_POS_TOP);
  gtk_box_pack_start(GTK_BOX(vbox),
		     http_notebook, TRUE, TRUE, 10);
  gtk_widget_show(http_notebook);
  {
    GtkWidget *tab_label = gtk_label_new(_("Referer"));
    gtk_notebook_append_page(GTK_NOTEBOOK(http_notebook),
			     Create_HTTP_Referer_page(),
			     tab_label);
  }
  {
    GtkWidget *tab_label = gtk_label_new(_("Cookie"));
    gtk_notebook_append_page(GTK_NOTEBOOK(http_notebook),
			     Create_HTTP_Cookie_page(),
			     tab_label);
  }
  {
    GtkWidget *tab_label = gtk_label_new(_("Agent"));
    gtk_notebook_append_page(GTK_NOTEBOOK(http_notebook),
			     Create_HTTP_Agent_page(),
			     tab_label);
  }
  {
    GtkWidget *tab_label = gtk_label_new(_("Proxy"));
    gtk_notebook_append_page(GTK_NOTEBOOK(http_notebook),
			     Create_HTTP_Proxy_page(),
			     tab_label);
  }
  {
    GtkWidget *tab_label = gtk_label_new(_("Recursive 1"));
    gtk_notebook_append_page(GTK_NOTEBOOK(http_notebook),
			     Create_HTTP_Recursive1_page(),
			     tab_label);
  }
  {
    GtkWidget *tab_label = gtk_label_new(_("Recursive 2"));
    gtk_notebook_append_page(GTK_NOTEBOOK(http_notebook),
			     Create_HTTP_Recursive2_page(),
			     tab_label);
  }
  {
    GtkWidget *tab_label = gtk_label_new(_("Misc"));
    gtk_notebook_append_page(GTK_NOTEBOOK(http_notebook),
			     Create_HTTP_Misc_page(),
			     tab_label);
  }
  return hbox;
}

GtkWidget *ItemOption::Create_FTP_page()
{
  GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
  gtk_widget_show(hbox);
  GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
  gtk_widget_show(vbox);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 10);

  ftp_notebook = gtk_notebook_new();
  gtk_notebook_set_scrollable(GTK_NOTEBOOK(ftp_notebook), TRUE);
  gtk_notebook_popup_enable(GTK_NOTEBOOK(ftp_notebook));
  gtk_notebook_set_tab_pos(GTK_NOTEBOOK(ftp_notebook), GTK_POS_TOP);
  gtk_box_pack_start(GTK_BOX(vbox),
		     ftp_notebook, TRUE, TRUE, 10);
  gtk_widget_show(ftp_notebook);
  {
    GtkWidget *tab_label = gtk_label_new(_("Mode"));
    gtk_notebook_append_page(GTK_NOTEBOOK(ftp_notebook),
			     Create_FTP_Mode_page(),
			     tab_label);
  }
  {
    GtkWidget *tab_label = gtk_label_new(_("Proxy"));
    gtk_notebook_append_page(GTK_NOTEBOOK(ftp_notebook),
			     Create_FTP_Proxy_page(),
			     tab_label);
  }
  {
    GtkWidget *tab_label = gtk_label_new(_("Recursive"));
    gtk_notebook_append_page(GTK_NOTEBOOK(ftp_notebook),
			     Create_FTP_Recursive_page(),
			     tab_label);
  }
  {
    GtkWidget *tab_label = gtk_label_new(_("Misc"));
    gtk_notebook_append_page(GTK_NOTEBOOK(ftp_notebook),
			     Create_FTP_Misc_page(),
			     tab_label);
  }
  return hbox;
}

Options::FTPproxyLoginProcType DFTP2FTPproxy(const string& login_proc)
{
  if(login_proc == DFTPPROXY_PL) {
    return Options::FTPPROXY_PL;
  } else if(login_proc == DFTPPROXY_PL_OPEN) {
    return Options::FTPPROXY_PL_OPEN;
  } else if(login_proc == DFTPPROXY_PL_OPEN2) {
    return Options::FTPPROXY_PL_OPEN2;
  } else if(login_proc == DFTPPROXY_PL_SITE) {
    return Options::FTPPROXY_PL_SITE;
  } else if(login_proc == DFTPPROXY_PL_USER) {
    return Options::FTPPROXY_PL_USER;
  } else if(login_proc == DFTPPROXY_USER) {
    return Options::FTPPROXY_USER;
  } else {
    return Options::FTPPROXY_OPEN;
  }
}

char *FTPproxy2DFTP(Options::FTPproxyLoginProcType login_proc)
{
  if(login_proc == Options::FTPPROXY_PL) {
    return DFTPPROXY_PL;
  } else if(login_proc == Options::FTPPROXY_PL_OPEN) {
    return DFTPPROXY_PL_OPEN;
  } else if(login_proc == Options::FTPPROXY_PL_OPEN2) {
    return DFTPPROXY_PL_OPEN2;
  } else if(login_proc == Options::FTPPROXY_PL_SITE) {
    return DFTPPROXY_PL_SITE;
  } else if(login_proc == Options::FTPPROXY_PL_USER) {
    return DFTPPROXY_PL_USER;
  } else if(login_proc == Options::FTPPROXY_USER) {
    return DFTPPROXY_USER;
  } else {
    return DFTPPROXY_OPEN;
  }
}

void set_common_proxy_list(GtkWidget *proxy_cbox, const ProxyList *proxy_list)
{
  GList *proxy_glist = list2glist(proxy_list->ret_list());
  if(proxy_glist != NULL) {
    gtk_combo_set_popdown_strings(GTK_COMBO(proxy_cbox), proxy_glist);
  } else {
    GtkWidget *list = GTK_COMBO(proxy_cbox)->list;
    gtk_list_clear_items(GTK_LIST(list), 0, -1);
  }
  GList *node = g_list_first(proxy_glist);
  while(node) {
    //delete [] (char *)node->data;
    g_free(node->data);
    node = g_list_next(node);
  }
  g_list_free(proxy_glist);
}

void ItemOption::set_ftp_proxy_list(const ProxyList *proxy_list)
{
  set_common_proxy_list(ftp_proxy_cbox, proxy_list);
}

void ItemOption::set_http_proxy_list(const ProxyList *proxy_list)
{
  set_common_proxy_list(http_proxy_cbox, proxy_list);
}

void ItemOption::set_useragent_list(const UseragentList *useragent_list)
{
  GList *useragent_glist = vector2glist(useragent_list->ret_vector());
  if(useragent_glist != NULL) {
    gtk_combo_set_popdown_strings(GTK_COMBO(useragent_cbox), useragent_glist);
  } else {
    GtkWidget *list = GTK_COMBO(useragent_cbox)->list;
    gtk_list_clear_items(GTK_LIST(list), 0, -1);
  }
  GList *node = g_list_first(useragent_glist);
  while(node) {
    //delete [] (char *)node->data;
    g_free(node->data);
    node = g_list_next(node);
  }
  g_list_free(useragent_glist);
}

void Add_target_item2(GtkWidget *w, GtkWidget *clist)
{
  GtkWidget *entry = (GtkWidget *)gtk_object_get_user_data(GTK_OBJECT(clist));
  string new_targets = gtk_entry_get_text(GTK_ENTRY(entry));
  gtk_entry_set_text(GTK_ENTRY(entry), "");
  /*
  while(new_targets.size()) {
    string new_target = Token_splitter(new_targets, " \t");
    if(new_target.empty()) continue;
    bool flag = false;
    for(int rowindex = 0; rowindex < GTK_CLIST(clist)->rows; ++rowindex) {
      char *target_c_str;
      gtk_clist_get_text(GTK_CLIST(clist), rowindex, 0, &target_c_str);
      string target = target_c_str;
      if(target == new_target) {
	flag = true;
	break;
      }
    }
    if(!flag) {
      char *title[1];
      title[0] = strdup(new_target.c_str());
      int rowindex = gtk_clist_append(GTK_CLIST(clist), title);
      gtk_clist_select_row(GTK_CLIST(clist), rowindex, 0);
      delete [] title[0];
    }
  }
  */
  string new_target = Remove_white(new_targets);
  if(new_target.empty()) return;
  bool flag = false;
  for(int rowindex = 0; rowindex < GTK_CLIST(clist)->rows; ++rowindex) {
    char *target_c_str;
    gtk_clist_get_text(GTK_CLIST(clist), rowindex, 0, &target_c_str);
    string target = target_c_str;
    if(target == new_target) {
      flag = true;
      break;
    }
  }
  if(!flag) {
    char *title[1];
    title[0] = strdup(new_target.c_str());
    int rowindex = gtk_clist_append(GTK_CLIST(clist), title);
    gtk_clist_select_row(GTK_CLIST(clist), rowindex, 0);
    delete [] title[0];
  }
}

void Add_target_item(GtkWidget *w, GtkWidget *clist)
{
  GtkWidget *entry = (GtkWidget *)gtk_object_get_user_data(GTK_OBJECT(clist));
  string new_targets = gtk_entry_get_text(GTK_ENTRY(entry));
  gtk_entry_set_text(GTK_ENTRY(entry), "");

  while(new_targets.size()) {
    string new_target = Token_splitter(new_targets, " \t");
    if(new_target.empty()) continue;
    bool flag = false;
    for(int rowindex = 0; rowindex < GTK_CLIST(clist)->rows; ++rowindex) {
      char *target_c_str;
      gtk_clist_get_text(GTK_CLIST(clist), rowindex, 0, &target_c_str);
      string target = target_c_str;
      if(target == new_target) {
	flag = true;
	break;
      }
    }
    if(!flag) {
      char *title[1];
      title[0] = strdup(new_target.c_str());
      int rowindex = gtk_clist_append(GTK_CLIST(clist), title);
      gtk_clist_select_row(GTK_CLIST(clist), rowindex, 0);
      delete [] title[0];
    }
  }
}

/*
static void speedScale_buttonReleaseEvent_cb(GtkWidget *speedScale,
					     GdkEventButton *event,
					     ItemOption *iopt)
{
  iopt->speedScaleButtonReleaseEvent();
}

void ItemOption::speedScaleButtonReleaseEvent() {
  GtkAdjustment *adj = gtk_range_get_adjustment(GTK_RANGE(speed_scale));
  //speed_value = adj->value;
}
*/

static void speedSpin_changed_cb(GtkEditable *w, ItemOption *iopt) {
  iopt->speedSpinChanged();
}

void ItemOption::speedSpinChanged() {
  GtkAdjustment *adj = gtk_range_get_adjustment(GTK_RANGE(speed_scale));

  adj->value = gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(speedSpin));
  //speed_value = adj->value;
  gtk_range_set_adjustment(GTK_RANGE(speed_scale), GTK_ADJUSTMENT(adj));

  //gtk_range_slider_update(GTK_RANGE(speed_scale));
  //adj = gtk_range_get_adjustment(GTK_RANGE(speed_scale));
  //gtk_signal_emit_by_name(GTK_OBJECT(speed_scale), "button-release-event");
  //speedScaleButtonReleaseEvent();
}

static gboolean speedScale_motionNotifyEvent_cb(GtkWidget *w,
						GdkEventMotion *e,
						ItemOption *iopt) {
  iopt->speedScaleMotionNotifyEvent();

  return TRUE;
}

void ItemOption::speedScaleMotionNotifyEvent() {
  GtkAdjustment *adj = gtk_range_get_adjustment(GTK_RANGE(speed_scale));

  gtk_spin_button_set_value(GTK_SPIN_BUTTON(speedSpin), adj->value);
}

static gboolean url_entry_focusOutEvent_cb(GtkWidget *url_entry,
					   GdkEventFocus *event,
					   gpointer data) {

  GtkWidget *sync_with_url_toggle = (GtkWidget *)gtk_object_get_data(GTK_OBJECT(url_entry), "sync_toggle");
  if(GTK_TOGGLE_BUTTON(sync_with_url_toggle)->active) {
    GtkWidget *filename_entry = (GtkWidget *)gtk_object_get_data(GTK_OBJECT(url_entry), "filename_entry");
    string url = Remove_white(gtk_entry_get_text(GTK_ENTRY(url_entry)));
    URLcontainer urlcon;
    if(urlcon.Parse_URL(url)) {
      gtk_entry_set_text(GTK_ENTRY(filename_entry), urlcon.ret_Filename().c_str());
    }
  }

  return TRUE;
}

static gboolean sync_with_url_toggled_cb(GtkToggleButton *sync_with_url_toggle,
					 gpointer data) {
  GtkWidget *filename_entry = (GtkWidget *)gtk_object_get_data(GTK_OBJECT(sync_with_url_toggle), "filename_entry");
  if(GTK_TOGGLE_BUTTON(sync_with_url_toggle)->active) {
    gtk_widget_set_sensitive(filename_entry, FALSE);
    GtkWidget *url_entry = (GtkWidget *)gtk_object_get_data(GTK_OBJECT(sync_with_url_toggle), "url_entry");
    string url = Remove_white(gtk_entry_get_text(GTK_ENTRY(url_entry)));
    URLcontainer urlcon;
    if(urlcon.Parse_URL(url)) {
      gtk_entry_set_text(GTK_ENTRY(filename_entry), urlcon.ret_Filename().c_str());
    }
  } else {
    gtk_widget_set_sensitive(filename_entry, TRUE);
  }

  return TRUE;
}
					 
GtkWidget *ItemOption::Create_General_URL_page()
{
  GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
  gtk_widget_show(vbox);

  // URL entry
  {
    GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
    gtk_widget_show(hbox);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);

    GtkWidget *URL_label = gtk_label_new("URL");
    gtk_widget_show(URL_label);
    gtk_box_pack_start(GTK_BOX(hbox), URL_label, FALSE, FALSE, 10);

    url_entry = gtk_entry_new();
    gtk_widget_set_usize(GTK_WIDGET(url_entry), 500, -1);
    gtk_widget_show(url_entry);
    gtk_box_pack_start(GTK_BOX(hbox), url_entry, FALSE, FALSE, 10);
  }
  // CRC entry
  {
    GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
    gtk_widget_show(hbox);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    GtkWidget *CRC_label = gtk_label_new("CRC");
    gtk_widget_show(CRC_label);
    gtk_box_pack_start(GTK_BOX(hbox), CRC_label, FALSE, FALSE, 10);

    crc_entry = gtk_entry_new_with_max_length(8);
    gtk_widget_set_usize(GTK_WIDGET(crc_entry), 100, -1);
    gtk_widget_show(crc_entry);
    gtk_box_pack_start(GTK_BOX(hbox), crc_entry, FALSE, FALSE, 10);
  }
  // MD5 entry
  {
    GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
    gtk_widget_show(hbox);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    GtkWidget *label = gtk_label_new("MD5");
    gtk_widget_show(label);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 10);

    md5_entry = gtk_entry_new_with_max_length(32);
    gtk_widget_set_usize(GTK_WIDGET(md5_entry), 400, -1);
    gtk_widget_show(md5_entry);
    gtk_box_pack_start(GTK_BOX(hbox), md5_entry, FALSE, FALSE, 10);
  }
  {
    GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
    gtk_widget_show(hbox);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);

    GtkWidget *label = gtk_label_new(_("Save as"));
    gtk_widget_show(label);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 10);

    filename_entry = gtk_entry_new_with_max_length(256);
    gtk_widget_set_usize(filename_entry, 300, -1);
    gtk_widget_show(filename_entry);
    gtk_box_pack_start(GTK_BOX(hbox), filename_entry, FALSE, FALSE, 10);

    sync_with_url_toggle = gtk_check_button_new_with_label(_("Sync with URL"));
    gtk_widget_show(sync_with_url_toggle);
    gtk_box_pack_start(GTK_BOX(hbox), sync_with_url_toggle, FALSE, FALSE, 0);
  }
  {
    // Add some callbacks here
    // lost focus on URL text field
    gtk_object_set_data(GTK_OBJECT(url_entry),
			"sync_toggle",
			(gpointer)sync_with_url_toggle);
    gtk_object_set_data(GTK_OBJECT(url_entry),
			"filename_entry",
			(gpointer)filename_entry);
    g_signal_connect(GTK_OBJECT(url_entry),
		       "focus-out-event",
		       GTK_SIGNAL_FUNC(url_entry_focusOutEvent_cb),
		       NULL);

    // switch on Sync with URL toggle button
    gtk_object_set_data(GTK_OBJECT(sync_with_url_toggle),
			"url_entry",
			(gpointer)url_entry);
    gtk_object_set_data(GTK_OBJECT(sync_with_url_toggle),
			"filename_entry",
			(gpointer)filename_entry);
    sync_with_url_toggled_cb_id = g_signal_connect(GTK_OBJECT(sync_with_url_toggle),
						     "toggled",
						     GTK_SIGNAL_FUNC(sync_with_url_toggled_cb),
						     NULL);
  }
  // Save Directory entry
  {
    GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
    gtk_widget_show(hbox);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);

    GtkWidget *label = gtk_label_new(_("Save directory"));
    gtk_widget_show(label);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 10);


    storedir_entry = gtk_entry_new();
    gtk_widget_set_usize(GTK_WIDGET(storedir_entry), 300, -1);
    gtk_widget_show(storedir_entry);
    gtk_box_pack_start(GTK_BOX(hbox), storedir_entry, FALSE, FALSE, 10);

    GtkWidget *button = gtk_button_new_with_label(_("select"));
    gtk_widget_show(button);
    gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
    g_signal_connect(GTK_OBJECT(button), "clicked",
		       GTK_SIGNAL_FUNC(Show_dir_browser),
		       GTK_OBJECT(option_window));
  }

  return vbox;
}

GtkWidget *ItemOption::Create_General_Authentication_page()
{
  GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
  gtk_widget_show(hbox);
  GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
  gtk_widget_show(vbox);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 5);

  // Authentication frame
  {
    GtkWidget *authentication_frame = gtk_frame_new(_("Authentication"));
    gtk_widget_show(authentication_frame);
    gtk_box_pack_start(GTK_BOX(vbox), authentication_frame, FALSE, FALSE, 10);
    GtkWidget *vbox = gtk_vbox_new(FALSE, 5);
    gtk_widget_show(vbox);
    gtk_container_add(GTK_CONTAINER(authentication_frame), vbox);
    // Use authentication toggle
    {
      GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
      gtk_widget_show(hbox);
      gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 10);

      toggle_use_authentication = gtk_check_button_new_with_label(_("Use authentication"));
      gtk_widget_show(toggle_use_authentication);
      gtk_box_pack_start(GTK_BOX(hbox), toggle_use_authentication, FALSE, FALSE, 0);
    }
    // User name entry
    {
      GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
      gtk_widget_show(hbox);
      gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 10);
    
      GtkWidget *label = gtk_label_new(_("User name"));
      gtk_widget_show(label);
      gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 10);
      
      user_entry = gtk_entry_new();//_with_max_length(127)
      gtk_widget_set_usize(GTK_WIDGET(user_entry), 400, -1);
      gtk_widget_show(user_entry);
      gtk_box_pack_start(GTK_BOX(hbox), user_entry, FALSE, FALSE, 10);
    }
    // Password entry
    {
      GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
      gtk_widget_show(hbox);
      gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 10);

      GtkWidget *label = gtk_label_new(_("Password"));
      gtk_widget_show(label);
      gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 10);
      
      password_entry = gtk_entry_new();//_with_max_length(127);
      gtk_widget_set_usize(GTK_WIDGET(password_entry), 400, -1);
      gtk_widget_show(password_entry);
      gtk_entry_set_visibility(GTK_ENTRY(password_entry), FALSE);
      gtk_box_pack_start(GTK_BOX(hbox), password_entry, FALSE, FALSE, 10);
    }
  }
  return hbox;
}

GtkWidget *ItemOption::Create_General_Download1_page()
{
  GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
  gtk_widget_show(hbox);
  GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 5);
  gtk_widget_show(vbox);
  
  // "Delete when download completes" toggle
  {
    toggle_delete_when_finish = gtk_check_button_new_with_label(_("Delete item when download completes"));
    gtk_widget_show(toggle_delete_when_finish);
    gtk_box_pack_start(GTK_BOX(vbox), toggle_delete_when_finish, FALSE, FALSE, 0);
  }
  {
    toggle_dont_delete_without_crc = gtk_check_button_new_with_label(_("Don't delete item if CRC/MD5 is not set"));
    gtk_widget_show(toggle_dont_delete_without_crc);
    gtk_box_pack_start(GTK_BOX(vbox), toggle_dont_delete_without_crc, FALSE, FALSE, 0);
  }
  {
    no_crc_check_toggle = gtk_check_button_new_with_label(_("Don't check CRC/MD5"));
    gtk_widget_show(no_crc_check_toggle);
    gtk_box_pack_start(GTK_BOX(vbox), no_crc_check_toggle, FALSE, FALSE, 0);
  }
  {
    ignore_crc_error_toggle = gtk_check_button_new_with_label(_("Ignore CRC/MD5 error"));
    gtk_widget_show(ignore_crc_error_toggle);
    gtk_box_pack_start(GTK_BOX(vbox), ignore_crc_error_toggle, FALSE, FALSE, 0);
  }
  {
    use_content_md5_toggle = gtk_check_button_new_with_label(_("Use Content-MD5"));
    gtk_widget_show(use_content_md5_toggle);
    gtk_box_pack_start(GTK_BOX(vbox), use_content_md5_toggle, FALSE, FALSE, 0);
  }
  {
    use_no_download_samename_toggle = gtk_check_button_new_with_label(_("Do not download already existing files"));
    gtk_widget_show(use_no_download_samename_toggle);
    gtk_box_pack_start(GTK_BOX(vbox), use_no_download_samename_toggle, FALSE, FALSE, 0);
  }
  {
    use_no_redownload_toggle = gtk_check_button_new_with_label(_("Do not redownload if resuming is not available"));
    gtk_widget_show(use_no_redownload_toggle);
    gtk_box_pack_start(GTK_BOX(vbox), use_no_redownload_toggle, FALSE, FALSE, 0);
  }
  // timed out
  {
    GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
    gtk_widget_show(hbox);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
    
    GtkWidget *label_timedout = gtk_label_new(_("Time out (sec) "));
    gtk_widget_show(label_timedout);
    gtk_box_pack_start(GTK_BOX(hbox), label_timedout, FALSE, FALSE, 0);
    
    GtkObject *adjustment_timedout = gtk_adjustment_new(30,
							0,
							MAXTIMEOUT,
							1,
							10,
							0);
    spin_timedout = gtk_spin_button_new(GTK_ADJUSTMENT(adjustment_timedout),
					1.0,
					0);
    gtk_widget_show(spin_timedout);
    gtk_box_pack_start(GTK_BOX(hbox), spin_timedout, FALSE, FALSE, 0);
  }
  // retry
  {
    GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
    gtk_widget_show(hbox);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
    
    GtkWidget *label_retry = gtk_label_new(_("Retry (-1: unlimited) "));
    gtk_widget_show(label_retry);
    gtk_box_pack_start(GTK_BOX(hbox), label_retry, FALSE, FALSE, 0);
    
    GtkObject *adjustment_retry = gtk_adjustment_new(5,
						     -1,
						     MAXRETRY,
						     1,
						     5,
						     0);
    spin_retry = gtk_spin_button_new(GTK_ADJUSTMENT(adjustment_retry),
				     1.0,
				     0);
    gtk_widget_show(spin_retry);
    gtk_box_pack_start(GTK_BOX(hbox), spin_retry, FALSE, FALSE, 0);

    /*
    GtkWidget *label_retry_push_back = gtk_label_new(_("if all failed, push back item to the end of list, repeating"));
    gtk_widget_show(label_retry_push_back);
    gtk_box_pack_start(GTK_BOX(hbox), label_retry_push_back, FALSE, FALSE, 0);
    
    GtkObject *adjustment_retry_push_back = gtk_adjustment_new(0,
							       0,
							       MAXRETRYPB,
							       1,
							       5,
							       0);
    spin_retry_push_back = gtk_spin_button_new(GTK_ADJUSTMENT(adjustment_retry_push_back),
				     1.0,
				     0);
    gtk_widget_show(spin_retry_push_back);
    gtk_box_pack_start(GTK_BOX(hbox), spin_retry_push_back, FALSE, FALSE, 0);
    */
  }
  // retry interval
  {
    GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
    gtk_widget_show(hbox);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
    
    GtkWidget *label = gtk_label_new(_("Retry interval (sec) "));
    gtk_widget_show(label);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
    
    GtkObject *adjustment = gtk_adjustment_new(10,
					       0,
					       MAXRETRYINTERVAL,
					       1,
					       5,
					       0);
    spin_retry_interval = gtk_spin_button_new(GTK_ADJUSTMENT(adjustment),
					      1.0,
					      0);
    gtk_widget_show(spin_retry_interval);
    gtk_box_pack_start(GTK_BOX(hbox), spin_retry_interval, FALSE, FALSE, 0);
  }
  // divide file into N parts
  {
    GtkWidget *hbox = HBOX_NEW(vbox);
    gtk_widget_show(hbox);
    
    GtkWidget *label = gtk_label_new(_("Divide file into "));
    gtk_widget_show(label);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
    
    GtkObject *adjustment = gtk_adjustment_new(5,
					       0,
					       MAXSPLIT,
					       1,
					       5,
					       0);
    spin_divide = gtk_spin_button_new(GTK_ADJUSTMENT(adjustment),
				      1.0,
				      0);
    gtk_widget_show(spin_divide);
    gtk_box_pack_start(GTK_BOX(hbox), spin_divide, FALSE, FALSE, 0);
  }
  // Rollback
  {
    GtkWidget *hbox = HBOX_NEW(vbox);
    gtk_widget_show(hbox);
    
    GtkWidget *label = gtk_label_new(_("Rollback (bytes) "));
    gtk_widget_show(label);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

    rollback_bytes_entry = gtk_entry_new();
    gtk_widget_set_usize(rollback_bytes_entry, 100, -1);
    gtk_widget_show(rollback_bytes_entry);
    gtk_box_pack_start(GTK_BOX(hbox), rollback_bytes_entry, FALSE, FALSE, 0);
  }
  // limit file size
  {
    GtkWidget *hbox = HBOX_NEW(vbox);
    gtk_widget_show(hbox);

    use_size_lower_limit_toggle = gtk_check_button_new_with_label(_("Do not download the files less than (bytes) "));
    gtk_widget_show(use_size_lower_limit_toggle);
    gtk_box_pack_start(GTK_BOX(hbox), use_size_lower_limit_toggle, FALSE, FALSE, 0);
    
    size_lower_limit_entry = gtk_entry_new();
    gtk_widget_set_usize(size_lower_limit_entry, 100, -1);
    gtk_widget_show(size_lower_limit_entry);
    gtk_box_pack_start(GTK_BOX(hbox), size_lower_limit_entry, FALSE, FALSE, 0);
  }

  return hbox;
}

GtkWidget *ItemOption::Create_General_Download2_page()
{
  GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
  gtk_widget_show(hbox);
  GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
  gtk_widget_show(vbox);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 5);

  // Resume
  {
    GtkWidget *frame = gtk_frame_new(_("Resume"));
    gtk_widget_show(frame);
    gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 10);

    GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
    gtk_container_add(GTK_CONTAINER(frame), hbox);
    gtk_widget_show(hbox);

    GtkWidget *vbox = gtk_vbox_new(FALSE, 5);
    gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 10);
    gtk_widget_show(vbox);

    // always resume
    alwaysresume_toggle = gtk_radio_button_new_with_label(NULL, _("Always resume"));
    gtk_widget_show(alwaysresume_toggle);
    GSList *group = gtk_radio_button_group(GTK_RADIO_BUTTON(alwaysresume_toggle));
    gtk_box_pack_start(GTK_BOX(vbox), alwaysresume_toggle, FALSE, FALSE, 2);

    // use if-modified-since
    useifmodsince_toggle = gtk_radio_button_new_with_label(group, _("Resume if remote contents have not been modified"));
    gtk_box_pack_start(GTK_BOX(vbox), useifmodsince_toggle, FALSE, FALSE, 2);
    gtk_widget_show (useifmodsince_toggle);

    // no resume
    noresume_toggle = gtk_radio_button_new_with_label(gtk_radio_button_group(GTK_RADIO_BUTTON(useifmodsince_toggle)), _("Do not resume"));
    gtk_box_pack_start(GTK_BOX(vbox), noresume_toggle, FALSE, FALSE, 2);
    gtk_widget_show(noresume_toggle);
  }
  // speed limit
  {
    GtkWidget *frame = gtk_frame_new(_("Limit download speed (in Kbytes/sec) (0.0: unlimited) "));
    gtk_widget_show(frame);
    gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

    GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(hbox);
    gtk_container_add(GTK_CONTAINER(frame), hbox);

    GtkObject *adj = gtk_adjustment_new(0,
					0,
					MAXSPEEDLIMIT,
					0.5,
					10.0,
					0);
    speed_scale = gtk_hscale_new(GTK_ADJUSTMENT(adj));
    gtk_scale_set_draw_value(GTK_SCALE(speed_scale), FALSE);
    gtk_scale_set_digits(GTK_SCALE(speed_scale), 1);

    gtk_widget_show(speed_scale);
    gtk_widget_set_usize(speed_scale, 300, -1);
    gtk_box_pack_start(GTK_BOX(hbox), speed_scale, FALSE, FALSE, 0);
    /*
    g_signal_connect(GTK_OBJECT(speed_scale),
		       "button-release-event",
		       GTK_SIGNAL_FUNC(speedScale_buttonReleaseEvent_cb),
		       (void *)this);
    */
    g_signal_connect(GTK_OBJECT(speed_scale),
		       "motion-notify-event",
		       GTK_SIGNAL_FUNC(speedScale_motionNotifyEvent_cb),
		       (void *)this);

    // spin box for better manipulation of speed limiter
    GtkObject *adjustment = gtk_adjustment_new(0,
					       0,
					       MAXSPEEDLIMIT,
					       0.1,
					       10,
					       0);
    speedSpin = gtk_spin_button_new(GTK_ADJUSTMENT(adjustment),
				    1.0,
				    0);
    gtk_spin_button_set_digits(GTK_SPIN_BUTTON(speedSpin), 1);
    gtk_spin_button_set_update_policy(GTK_SPIN_BUTTON(speedSpin),
				      GTK_UPDATE_IF_VALID);				     
    gtk_widget_set_usize(speedSpin, 60, -1);
    gtk_widget_show(speedSpin);
    gtk_box_pack_start(GTK_BOX(hbox), speedSpin, FALSE, FALSE, 0);
    
    g_signal_connect(GTK_OBJECT(speedSpin),
		       "changed",
		       GTK_SIGNAL_FUNC(speedSpin_changed_cb),
		       (void *)this);
    
  }
  return hbox;
}

GtkWidget *ItemOption::Create_General_Command_page()
{
  GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
  gtk_widget_show(hbox);
  GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 5);
  gtk_widget_show(vbox);

  {
    use_command_toggle = gtk_check_button_new_with_label(_("Execute the following command after download"));
    gtk_widget_show(use_command_toggle);
    gtk_box_pack_start(GTK_BOX(vbox), use_command_toggle, FALSE, FALSE, 0);

    command_entry = gtk_text_view_new();//gtk_entry_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(command_entry), TRUE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(command_entry), GTK_WRAP_WORD_CHAR);
    gtk_widget_show(command_entry);

    GtkWidget* scrolledWindow = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledWindow),
				   GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
    gtk_container_add(GTK_CONTAINER(vbox), scrolledWindow);
    gtk_widget_show(scrolledWindow);
    
    gtk_container_add(GTK_CONTAINER(scrolledWindow), command_entry);

    use_exit_status_toggle = gtk_check_button_new_with_llabel(_("Download again if the exit status is not in the following list (delimiter is ',')"));
    gtk_widget_show(use_exit_status_toggle);
    gtk_box_pack_start(GTK_BOX(vbox), use_exit_status_toggle, FALSE, FALSE, 0);

    exit_status_entry = gtk_entry_new();
    gtk_widget_show(exit_status_entry);
    gtk_box_pack_start(GTK_BOX(vbox), exit_status_entry, FALSE, FALSE, 0);

  }
  return hbox;
}

GtkWidget *ItemOption::Create_HTTP_Referer_page()
{
  GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
  gtk_widget_show(hbox);
  GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
  gtk_widget_show(vbox);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 5);

  // HTTP Referer
  {
    GtkWidget *referer_frame = gtk_frame_new(_("Referer"));
    gtk_widget_show(referer_frame);
    gtk_box_pack_start(GTK_BOX(vbox), referer_frame, FALSE, FALSE, 10);

    GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
    gtk_container_add(GTK_CONTAINER(referer_frame), hbox);
    gtk_widget_show(hbox);

    GtkWidget *vbox = gtk_vbox_new(FALSE, 5);
    gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 10);
    gtk_widget_show(vbox);

    // referer : index.html
    index_button = gtk_radio_button_new_with_label(NULL, "index.html");
    gtk_widget_show(index_button);
    GSList *group = gtk_radio_button_group(GTK_RADIO_BUTTON(index_button));
    gtk_box_pack_start(GTK_BOX(vbox), index_button, FALSE, FALSE, 2);

    // referer : none
    none_button = gtk_radio_button_new_with_label(group, _("none"));
    gtk_box_pack_start(GTK_BOX(vbox), none_button, FALSE, FALSE, 2);
    gtk_widget_show (none_button);

    // referer : url
    urlref_button = gtk_radio_button_new_with_label(gtk_radio_button_group(GTK_RADIO_BUTTON(none_button)), _("URL"));
    gtk_box_pack_start(GTK_BOX(vbox), urlref_button, FALSE, FALSE, 2);
    gtk_widget_show(urlref_button);

    {
      GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
      gtk_widget_show(hbox);
      gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 2);
      user_button = gtk_radio_button_new_with_label(gtk_radio_button_group(GTK_RADIO_BUTTON(urlref_button)),
						    _("user defined"));  
      gtk_widget_show(user_button);
      gtk_box_pack_start(GTK_BOX(hbox), user_button, FALSE, FALSE, 0);
      
      referer_entry = gtk_entry_new();//_with_max_length(1024);
      gtk_widget_set_usize(referer_entry, 400, -1);
      gtk_widget_show(referer_entry);
      gtk_box_pack_start(GTK_BOX(hbox), referer_entry, FALSE, FALSE, 10);
    }
  }
  return hbox;
}

GtkWidget *ItemOption::Create_HTTP_Cookie_page()
{
  GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
  gtk_widget_show(hbox);
  GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
  gtk_widget_show(vbox);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 5);

  // Cookie
  {
    GtkWidget *cookie_frame = gtk_frame_new(_("Cookie"));
    gtk_widget_show(cookie_frame);
    gtk_box_pack_start(GTK_BOX(vbox), cookie_frame, FALSE, FALSE, 5);
    GtkWidget *vbox = gtk_vbox_new(FALSE, 5);
    gtk_widget_show(vbox);
    gtk_container_add(GTK_CONTAINER(cookie_frame), vbox);

    cookie_delete_on_restart_toggle = gtk_check_button_new_with_label(_("Delete cookie on restart (resume or error)"));
    gtk_widget_show(cookie_delete_on_restart_toggle);
    gtk_box_pack_start(GTK_BOX(vbox), cookie_delete_on_restart_toggle, FALSE, FALSE, 5);

    cookie_nosend_toggle = gtk_check_button_new_with_label(_("Do not send cookie"));
    gtk_widget_show(cookie_nosend_toggle);
    gtk_box_pack_start(GTK_BOX(vbox), cookie_nosend_toggle, FALSE, FALSE, 5);

    GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
    gtk_widget_show(hbox);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);

    cookieUserDefinedToggle = gtk_check_button_new_with_label(_("Send user defined cookie"));
    gtk_widget_show(cookieUserDefinedToggle);
    gtk_box_pack_start(GTK_BOX(hbox), cookieUserDefinedToggle, FALSE, FALSE, 0);

    cookieUserDefinedEntry = gtk_entry_new();
    gtk_widget_show(cookieUserDefinedEntry);
    gtk_widget_set_usize(cookieUserDefinedEntry, 250, -1);
    gtk_box_pack_start(GTK_BOX(hbox), cookieUserDefinedEntry, FALSE, FALSE, 5);
  }
  return hbox;
}

GtkWidget *ItemOption::Create_HTTP_Agent_page()
{
  GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
  gtk_widget_show(hbox);
  GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
  gtk_widget_show(vbox);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 5);

  // Useragent
  {
    GtkWidget *useragent_frame = gtk_frame_new(_("User Agent"));
    gtk_widget_show(useragent_frame);
    gtk_box_pack_start(GTK_BOX(vbox), useragent_frame, FALSE, FALSE, 10);

    GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(hbox);
    gtk_container_add(GTK_CONTAINER(useragent_frame), hbox);

    GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(vbox);
    gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 5);

    toggle_random_useragent = gtk_check_button_new_with_label(_("Select randomly"));

    //gtk_widget_show(toggle_random_useragent);
    gtk_box_pack_start(GTK_BOX(vbox), toggle_random_useragent, FALSE, FALSE, 5);

    useragent_cbox = gtk_combo_new();
    gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(useragent_cbox)->entry), FALSE);
    gtk_widget_set_usize(useragent_cbox, 400, -1);
    gtk_widget_show(useragent_cbox);
    gtk_box_pack_start(GTK_BOX(vbox), useragent_cbox, FALSE, FALSE, 5);
  }

  return hbox;
}

GtkWidget *ItemOption::Create_HTTP_Proxy_page()
{
  GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
  gtk_widget_show(hbox);
  GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
  gtk_widget_show(vbox);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 5);

  
  // HTTP Proxy setting
  {
    GtkWidget *proxy_frame = gtk_frame_new(_("Proxy"));
    gtk_widget_show(proxy_frame);
    gtk_box_pack_start(GTK_BOX(vbox), proxy_frame, FALSE, FALSE, 5);

    GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(hbox);
    gtk_container_add(GTK_CONTAINER(proxy_frame), hbox);

    GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(vbox);
    gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 5);


    // use http proxy or not
    {
      use_http_proxy_toggle = gtk_check_button_new_with_label(_("Use proxy"));
      gtk_widget_show(use_http_proxy_toggle);
      gtk_box_pack_start(GTK_BOX(vbox), use_http_proxy_toggle, FALSE, FALSE, 5);
    }
    // authentication
    {
      GtkWidget *authentication_frame = gtk_frame_new(_("Authentication"));
      gtk_widget_show(authentication_frame);
      gtk_box_pack_start(GTK_BOX(vbox), authentication_frame, FALSE, FALSE, 5);
      GtkWidget *authentication_vbox = gtk_vbox_new(FALSE, 0);
      gtk_widget_show(authentication_vbox);
      gtk_container_add(GTK_CONTAINER(authentication_frame), authentication_vbox);

      {
	GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(authentication_vbox), hbox, FALSE, FALSE, 5);

	toggle_use_http_proxy_authentication = gtk_check_button_new_with_label(_("Use authentication"));
	gtk_widget_show(toggle_use_http_proxy_authentication);
	gtk_box_pack_start(GTK_BOX(hbox), toggle_use_http_proxy_authentication, FALSE, FALSE, 5);
      }
      {
	GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(authentication_vbox), hbox, FALSE, FALSE, 5);
    
	GtkWidget *label = gtk_label_new(_("User name"));
	gtk_widget_show(label);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 5);
      
	http_proxy_user_entry = gtk_entry_new();//_with_max_length(127);
	gtk_widget_set_usize(GTK_WIDGET(http_proxy_user_entry), 400, -1);
	gtk_widget_show(http_proxy_user_entry);
	gtk_box_pack_start(GTK_BOX(hbox), http_proxy_user_entry, FALSE, FALSE, 10);
      }
      {
	GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(authentication_vbox), hbox, FALSE, FALSE, 5);
	
	GtkWidget *label = gtk_label_new(_("Password"));
	gtk_widget_show(label);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 5);
      
	http_proxy_password_entry = gtk_entry_new();//_with_max_length(127);
	gtk_widget_set_usize(GTK_WIDGET(http_proxy_password_entry), 400, -1);
	gtk_entry_set_visibility(GTK_ENTRY(http_proxy_password_entry), FALSE);
	gtk_widget_show(http_proxy_password_entry);
	gtk_box_pack_start(GTK_BOX(hbox), http_proxy_password_entry, FALSE, FALSE, 10);
      }
    }

    // toggle whether use cache or not
    toggle_use_http_cache = gtk_check_button_new_with_label(_("Use cache"));
    gtk_widget_show(toggle_use_http_cache);
    gtk_box_pack_start(GTK_BOX(vbox), toggle_use_http_cache, FALSE, FALSE, 5);

    {
      GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
      gtk_widget_show(hbox);
      gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);

      // label
      GtkWidget *name_label = gtk_label_new(_("Server : Port"));
      gtk_widget_show(name_label);
      gtk_box_pack_start(GTK_BOX(hbox), name_label, FALSE, FALSE, 5);

      // combobox
      http_proxy_cbox = gtk_combo_new();
      gtk_widget_set_usize(http_proxy_cbox, 300, -1);
      gtk_widget_show(http_proxy_cbox);
      gtk_box_pack_start(GTK_BOX(hbox), http_proxy_cbox, FALSE, FALSE, 5);

      // add button
      GtkWidget *http_proxy_add_button = gtk_button_new_with_label(_("Add"));
      gtk_widget_show(http_proxy_add_button);
      gtk_box_pack_start(GTK_BOX(hbox), http_proxy_add_button, FALSE, FALSE, 0);
      g_signal_connect(GTK_OBJECT(http_proxy_add_button), "clicked",
			 GTK_SIGNAL_FUNC(Add_http_proxy_entry),
			 GTK_OBJECT(http_proxy_cbox));

      // delete button
      GtkWidget *http_proxy_del_button = gtk_button_new_with_label(_("Delete"));
      gtk_widget_show(http_proxy_del_button);
      gtk_box_pack_start(GTK_BOX(hbox), http_proxy_del_button, FALSE, FALSE, 0);
      g_signal_connect(GTK_OBJECT(http_proxy_del_button), "clicked",
			 GTK_SIGNAL_FUNC(Delete_http_proxy_entry),
			 GTK_OBJECT(http_proxy_cbox));
    }
  }
  return hbox;
}

GtkWidget *ItemOption::Create_HTTP_Recursive1_page()
{
  GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
  gtk_widget_show(hbox);
  GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 5);
  gtk_widget_show(vbox);

  // HTTP recursive download
  {
    // frame
    GtkWidget *frame = gtk_frame_new(_("Recursive download"));
    gtk_widget_show(frame);
    gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 10);

    GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
    gtk_widget_show(hbox);
    gtk_container_add(GTK_CONTAINER(frame), hbox);
    {
      GtkWidget *vbox = gtk_vbox_new(FALSE, 5);
      gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 10);
      gtk_widget_show(vbox);

      {
	GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

	GtkWidget *vbox1 = gtk_vbox_new(FALSE, 5);
	gtk_box_pack_start(GTK_BOX(hbox), vbox1, FALSE, FALSE, 0);
	gtk_widget_show(vbox1);	

	// depth of recursion
	{
	  GtkWidget *hbox = HBOX_NEW(vbox1);
	  gtk_widget_show(hbox);
	  
	  GtkWidget *label = gtk_label_new(_("Depth of recursion "));
	  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 10);
	  gtk_widget_show(label);
	  GtkObject *adjustment = gtk_adjustment_new(1,//value
						     1,//lower
						     MAXRECURSION,//upper
						     1,//step inc
						     10,//page inc
						     0);//page size
	  recurse_depth_spin = gtk_spin_button_new(GTK_ADJUSTMENT(adjustment),
						   1.0,
						   0);
	  gtk_widget_show(recurse_depth_spin);
	  gtk_box_pack_start(GTK_BOX(hbox), recurse_depth_spin, FALSE, FALSE, 10);
	}
	{
	  recurse_hostname_dir_toggle = gtk_check_button_new_with_label(_("With hostname directory"));
	  gtk_widget_show(recurse_hostname_dir_toggle);
	  gtk_box_pack_start(GTK_BOX(vbox1), recurse_hostname_dir_toggle, FALSE, FALSE, 0);
	  g_signal_connect(GTK_OBJECT(recurse_hostname_dir_toggle),
			     "toggled",
			     GTK_SIGNAL_FUNC(HTTPRecursive_toggled_event_cb),
			     this);
	}
	{
	  recurse_abs2rel_toggle = gtk_check_button_new_with_label(_("Convert absolute URLs to relative ones"));
	  gtk_widget_show(recurse_abs2rel_toggle);
	  gtk_box_pack_start(GTK_BOX(vbox1), recurse_abs2rel_toggle, FALSE, FALSE, 0);
	  g_signal_connect(GTK_OBJECT(recurse_abs2rel_toggle),
			     "toggled",
			     GTK_SIGNAL_FUNC(HTTPRecursive_toggled_event_cb),
			     this);
	}
	{
	  recurse_force_convert_toggle = gtk_check_button_new_with_label(_("Force convert"));
	  gtk_widget_show(recurse_force_convert_toggle);
	  gtk_box_pack_start(GTK_BOX(vbox1), recurse_force_convert_toggle, FALSE, FALSE, 0);
	}
	{
	  recurse_del_comment_toggle = gtk_check_button_new_with_label(_("Delete comments"));
	  gtk_widget_show(recurse_del_comment_toggle);
	  gtk_box_pack_start(GTK_BOX(vbox1), recurse_del_comment_toggle, FALSE, FALSE, 0);
	}
	{
	  recurse_del_javascript_toggle = gtk_check_button_new_with_label(_("Delete JavaScript"));
	  gtk_widget_show(recurse_del_javascript_toggle);
	  gtk_box_pack_start(GTK_BOX(vbox1), recurse_del_javascript_toggle, FALSE, FALSE, 0);
	}
	{
	  recurse_del_iframe_toggle = gtk_check_button_new_with_label(_("Delete iframe"));
	  gtk_widget_show(recurse_del_iframe_toggle);
	  gtk_box_pack_start(GTK_BOX(vbox1), recurse_del_iframe_toggle, FALSE, FALSE, 0);
	}
	{
	  recurse_no_other_host_toggle = gtk_check_button_new_with_label(_("No other host"));
	  gtk_widget_show(recurse_no_other_host_toggle);
	  gtk_box_pack_start(GTK_BOX(vbox1), recurse_no_other_host_toggle, FALSE, FALSE, 0);
	}

	/// right side
	GtkWidget *vbox2 = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), vbox2, FALSE, FALSE, 5);
	gtk_widget_show(vbox2);	
	{
	  recurse_no_ascend_toggle = gtk_check_button_new_with_label(_("Do not ascend to parent directory"));
	  gtk_widget_show(recurse_no_ascend_toggle);
	  gtk_box_pack_start(GTK_BOX(vbox2), recurse_no_ascend_toggle, FALSE, FALSE, 0);
	}
	{
	  recurse_relative_only_toggle = gtk_check_button_new_with_label(_("Only relative links"));
	  gtk_widget_show(recurse_relative_only_toggle);
	  gtk_box_pack_start(GTK_BOX(vbox2), recurse_relative_only_toggle, FALSE, FALSE, 0);
	}

	{
	  recurse_referer_override_toggle = gtk_check_button_new_with_label(_("Referer override"));
	  gtk_widget_show(recurse_referer_override_toggle);
	  gtk_box_pack_start(GTK_BOX(vbox2), recurse_referer_override_toggle, FALSE, FALSE, 0);
	}
	{
	  recurse_follow_ftp_link_toggle = gtk_check_button_new_with_label(_("Follow FTP link"));
	  gtk_widget_show(recurse_follow_ftp_link_toggle);
	  gtk_box_pack_start(GTK_BOX(vbox2), recurse_follow_ftp_link_toggle, FALSE, FALSE, 0);
	}
	{
	  recurse_no_redownload_toggle = gtk_check_button_new_with_label(_("Assume download is completed if resuming is not available"));
    gtk_widget_show(recurse_no_redownload_toggle);
    gtk_box_pack_start(GTK_BOX(vbox), recurse_no_redownload_toggle, FALSE, FALSE, 0);
	}
	{
	  recurseAddPasteToggle = gtk_check_button_new_with_label(_("Add to paste window"));
	  gtk_widget_show(recurseAddPasteToggle);
	  gtk_box_pack_start(GTK_BOX(vbox), recurseAddPasteToggle, FALSE, FALSE, 0);
	}
	/*
	{
	  recurse_convert_tilde_toggle = gtk_check_button_new_with_label(_("Convert \"~\" to \"%7E\""));
	  gtk_widget_show(recurse_convert_tilde_toggle);
	  gtk_box_pack_start(GTK_BOX(vbox2), recurse_convert_tilde_toggle, FALSE, FALSE, 0);
	}
	*/
	{
	  GtkWidget *frame = gtk_frame_new(_("Tags"));
	  gtk_widget_show(frame);
	  gtk_box_pack_start(GTK_BOX(vbox2), frame, FALSE, FALSE, 5);
	  {
	    GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
	    gtk_widget_show(hbox);
	    gtk_container_add(GTK_CONTAINER(frame), hbox);
	    
	    GtkWidget *vbox = gtk_vbox_new(FALSE, 5);
	    gtk_widget_show(vbox);
	    gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 10);
	    {
	      recurse_tag_href_toggle = gtk_check_button_new_with_label(_("href"));
	      gtk_widget_show(recurse_tag_href_toggle);
	      gtk_box_pack_start(GTK_BOX(vbox), recurse_tag_href_toggle, FALSE, FALSE, 0);
	    }
	    {
	      recurse_tag_src_toggle = gtk_check_button_new_with_label(_("src"));
	      gtk_widget_show(recurse_tag_src_toggle);
	      gtk_box_pack_start(GTK_BOX(vbox), recurse_tag_src_toggle, FALSE, FALSE, 0);
	    }
	    {
	      recurse_tag_background_toggle = gtk_check_button_new_with_label(_("background"));
	      gtk_widget_show(recurse_tag_background_toggle);
	      gtk_box_pack_start(GTK_BOX(vbox), recurse_tag_background_toggle, FALSE, FALSE, 0);
	    }
	    {
	      recurse_tag_code_toggle = gtk_check_button_new_with_label(_("codebase"));
	      gtk_widget_show(recurse_tag_code_toggle);
	      gtk_box_pack_start(GTK_BOX(vbox), recurse_tag_code_toggle, FALSE, FALSE, 0);
	    }
	  }
	}
      }
    }
  }

  return hbox;
}

GtkWidget *ItemOption::Create_HTTP_Recursive2_page()
{
  GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
  gtk_widget_show(hbox);
  GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 5);
  gtk_widget_show(vbox);

  // HTTP recursive download
  {
    // frame
    GtkWidget *frame = gtk_frame_new(_("Recursive download"));
    gtk_widget_show(frame);
    gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 10);

    GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(hbox);
    gtk_container_add(GTK_CONTAINER(frame), hbox);

    GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 5);
    gtk_widget_show(vbox);

    {
      GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
      gtk_widget_show(hbox);
      gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);
      
      {// Parse Target extension clist
	// frame
	GtkWidget *frame = gtk_frame_new(_("Extensions to parse"));
	gtk_widget_show(frame);
	gtk_box_pack_start(GTK_BOX(hbox), frame, FALSE, FALSE, 0);
	{
	  GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
	  gtk_container_add(GTK_CONTAINER(frame), hbox);
	  gtk_widget_show(hbox);
	  
	  {
	    GtkWidget *vbox = gtk_vbox_new(FALSE, 5);
	    gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);
	    gtk_widget_show(vbox);
	    
	    GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
	    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	    gtk_widget_show(hbox);
	    
	    recurse_parse_target_entry = gtk_entry_new();
	    gtk_widget_set_usize(recurse_parse_target_entry, 100, -1);
	    gtk_box_pack_start(GTK_BOX(hbox), recurse_parse_target_entry, FALSE, FALSE, 10);
	    gtk_widget_show(recurse_parse_target_entry);
	    
	    GtkWidget *add_button = gtk_button_new_with_label(_("Add"));
	    gtk_widget_show(add_button);
	    gtk_box_pack_start(GTK_BOX(hbox), add_button, FALSE, FALSE, 0);
	    g_signal_connect_swapped(GTK_OBJECT(recurse_parse_target_entry), "activate",
				      GTK_SIGNAL_FUNC(gtk_button_clicked),
				      GTK_OBJECT(add_button));
	    
	    //HTML解析する拡張子のclist
	    int n_titles = 1;
	    char *titles[n_titles];
	    titles[0] = _("Extension");
	    // スクロールウインドウにCListをバッキング
	    GtkWidget *scrolled_window = Create_CheckCList(&recurse_parse_target_clist, titles, n_titles);
	    Create_popup_menu(&recurse_parse_target_clist);
	    gtk_object_set_user_data(GTK_OBJECT(recurse_parse_target_clist), recurse_parse_target_entry);
	    g_signal_connect(GTK_OBJECT(add_button), "clicked",
			       GTK_SIGNAL_FUNC(Add_target_item),
			       GTK_OBJECT(recurse_parse_target_clist));
	    
	    gtk_clist_set_column_auto_resize(GTK_CLIST(recurse_parse_target_clist), 0, FALSE);
	    gtk_widget_set_usize(scrolled_window, 100, 100);
	    
	    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, FALSE, FALSE, 5);
	    gtk_widget_show(scrolled_window);
	    //delete [] n_titles;
	  }
	}
      }
      {// Ignore Domain clist	    
	// frame
	GtkWidget *frame = gtk_frame_new(_("Domains to ignore"));
	gtk_widget_show(frame);
	gtk_box_pack_start(GTK_BOX(hbox), frame, FALSE, FALSE, 0);
	
	{
	  GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
	  gtk_container_add(GTK_CONTAINER(frame), hbox);
	  gtk_widget_show(hbox);
	  
	  {
	    GtkWidget *vbox = gtk_vbox_new(FALSE, 5);
	    gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);
	    gtk_widget_show(vbox);
	    
	    GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
	    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	    gtk_widget_show(hbox);
	    
	    recurse_ign_domain_entry = gtk_entry_new();
	    gtk_widget_set_usize(recurse_ign_domain_entry, 100, -1);
	    gtk_box_pack_start(GTK_BOX(hbox), recurse_ign_domain_entry, FALSE, FALSE, 10);
	    gtk_widget_show(recurse_ign_domain_entry);
	    
	    GtkWidget *add_button = gtk_button_new_with_label(_("Add"));
	    gtk_widget_show(add_button);
	    gtk_box_pack_start(GTK_BOX(hbox), add_button, FALSE, FALSE, 0);
	    g_signal_connect_swapped(GTK_OBJECT(recurse_ign_domain_entry), "activate",
				      GTK_SIGNAL_FUNC(gtk_button_clicked),
				      GTK_OBJECT(add_button));
	  
	    //ダウンロードするファイルの拡張子のclist
	    int n_titles = 1;
	    char *titles[n_titles];
	    titles[0] = _("Domain");
	    // スクロールウインドウにCListをバッキング
	    GtkWidget *scrolled_window = Create_CheckCList(&recurse_ign_domain_clist, titles, n_titles);
	    gtk_clist_set_column_auto_resize(GTK_CLIST(recurse_ign_domain_clist), 0, FALSE);
	    Create_popup_menu(&recurse_ign_domain_clist);
	    gtk_object_set_user_data(GTK_OBJECT(recurse_ign_domain_clist), recurse_ign_domain_entry);
	    g_signal_connect(GTK_OBJECT(add_button), "clicked",
			       GTK_SIGNAL_FUNC(Add_target_item),
			       GTK_OBJECT(recurse_ign_domain_clist));
	  
	    gtk_widget_set_usize(scrolled_window, 100, 100);
	  
	    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, FALSE, FALSE, 5);
	    gtk_widget_show(scrolled_window);
	  }
	}
      }
    }
    {
      GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
      gtk_widget_show(hbox);
      gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);
      {// Download Target extension clist
	// frame
	GtkWidget *frame = gtk_frame_new(_("Patterns to download"));
	gtk_widget_show(frame);
	gtk_box_pack_start(GTK_BOX(hbox), frame, FALSE, FALSE, 0);
      
	{
	  GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
	  gtk_container_add(GTK_CONTAINER(frame), hbox);
	  gtk_widget_show(hbox);
	
	  {
	    GtkWidget *vbox = gtk_vbox_new(FALSE, 5);
	    gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);
	    gtk_widget_show(vbox);
	  
	    recurse_use_down_filter_toggle = gtk_check_button_new_with_label(_("Use this filter"));
	    gtk_widget_show(recurse_use_down_filter_toggle);
	    gtk_box_pack_start(GTK_BOX(vbox), recurse_use_down_filter_toggle, FALSE, FALSE, 0);
	  
	    GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
	    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	    gtk_widget_show(hbox);
	  
	    recurse_down_filter_target_entry = gtk_entry_new();
	    gtk_widget_set_usize(recurse_down_filter_target_entry, 100, -1);
	    gtk_box_pack_start(GTK_BOX(hbox), recurse_down_filter_target_entry, FALSE, FALSE, 10);
	    gtk_widget_show(recurse_down_filter_target_entry);
	  
	    GtkWidget *add_button = gtk_button_new_with_label(_("Add"));
	    gtk_widget_show(add_button);
	    gtk_box_pack_start(GTK_BOX(hbox), add_button, FALSE, FALSE, 0);
	    g_signal_connect_swapped(GTK_OBJECT(recurse_down_filter_target_entry), "activate",
				      GTK_SIGNAL_FUNC(gtk_button_clicked),
				      GTK_OBJECT(add_button));
	  
	    //ダウンロードするファイルのパターンのclist
	    int n_titles = 1;
	    char *titles[n_titles];
	    titles[0] = _("Pattern");
	    // スクロールウインドウにCListをバッキング
	    GtkWidget *scrolled_window = Create_CheckCList(&recurse_down_filter_target_clist, titles, n_titles);
	    gtk_clist_set_column_auto_resize(GTK_CLIST(recurse_down_filter_target_clist), 0, FALSE);
	    Create_popup_menu(&recurse_down_filter_target_clist);
	    gtk_object_set_user_data(GTK_OBJECT(recurse_down_filter_target_clist), recurse_down_filter_target_entry);
	    g_signal_connect(GTK_OBJECT(add_button), "clicked",
			       GTK_SIGNAL_FUNC(Add_target_item2),
			       GTK_OBJECT(recurse_down_filter_target_clist));
	  
	    gtk_widget_set_usize(scrolled_window, 100, 100);
	  
	    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, FALSE, FALSE, 5);
	    gtk_widget_show(scrolled_window);
	  }
	}
      }
      {// Ignore Download Target extension clist
	// frame
	GtkWidget *frame = gtk_frame_new(_("Extensions to ignore"));
	gtk_widget_show(frame);
	gtk_box_pack_start(GTK_BOX(hbox), frame, FALSE, FALSE, 0);
      
	{
	  GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
	  gtk_container_add(GTK_CONTAINER(frame), hbox);
	  gtk_widget_show(hbox);
	
	  {
	    GtkWidget *vbox = gtk_vbox_new(FALSE, 5);
	    gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);
	    gtk_widget_show(vbox);
	  
	    GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
	    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	    gtk_widget_show(hbox);
	  
	    recurse_nodown_filter_target_entry = gtk_entry_new();
	    gtk_widget_set_usize(recurse_nodown_filter_target_entry, 100, -1);
	    gtk_box_pack_start(GTK_BOX(hbox), recurse_nodown_filter_target_entry, FALSE, FALSE, 10);
	    gtk_widget_show(recurse_nodown_filter_target_entry);
	  
	    GtkWidget *add_button = gtk_button_new_with_label(_("Add"));
	    gtk_widget_show(add_button);
	    gtk_box_pack_start(GTK_BOX(hbox), add_button, FALSE, FALSE, 0);
	    g_signal_connect_swapped(GTK_OBJECT(recurse_nodown_filter_target_entry), "activate",
				      GTK_SIGNAL_FUNC(gtk_button_clicked),
				      GTK_OBJECT(add_button));
	  
	    //ダウンロードするファイルの拡張子のclist
	    int n_titles = 1;
	    char *titles[n_titles];
	    titles[0] = _("Extension");
	    // スクロールウインドウにCListをバッキング
	    GtkWidget *scrolled_window = Create_CheckCList(&recurse_nodown_filter_target_clist, titles, n_titles);
	    gtk_clist_set_column_auto_resize(GTK_CLIST(recurse_nodown_filter_target_clist), 0, FALSE);
	    Create_popup_menu(&recurse_nodown_filter_target_clist);
	    gtk_object_set_user_data(GTK_OBJECT(recurse_nodown_filter_target_clist), recurse_nodown_filter_target_entry);
	    g_signal_connect(GTK_OBJECT(add_button), "clicked",
			       GTK_SIGNAL_FUNC(Add_target_item),
			       GTK_OBJECT(recurse_nodown_filter_target_clist));
	  
	    gtk_widget_set_usize(scrolled_window, 100, 100);
	  
	    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, FALSE, FALSE, 5);
	    gtk_widget_show(scrolled_window);
	  }
	}
      }
    }
  }
  return hbox;
}

static int selected_http_version;

static gboolean http_version_callback(GtkWidget *widget, gpointer data)
{
  int index = GPOINTER_TO_UINT(data);
  selected_http_version = index;

  return TRUE;
}

GtkWidget* ItemOption::Create_HTTP_Misc_page()
{
  GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
  gtk_widget_show(hbox);
  GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
  gtk_widget_show(vbox);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 5);
  {
    use_retry_404_toggle = gtk_check_button_new_with_label(_("Retry even if HTTP status is 404"));
    gtk_widget_show(use_retry_404_toggle);
    gtk_box_pack_start(GTK_BOX(vbox), use_retry_404_toggle, FALSE, FALSE, 0);
  }
  {
    use_retry_503_toggle = gtk_check_button_new_with_label(_("Retry even if HTTP status is 503"));
    gtk_widget_show(use_retry_503_toggle);
    gtk_box_pack_start(GTK_BOX(vbox), use_retry_503_toggle, FALSE, FALSE, 0);
  }
  {
    GtkWidget *frame = gtk_frame_new(_("HTTP status 416 handling"));
    gtk_widget_show(frame);
    gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 10);

    GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
    gtk_container_add(GTK_CONTAINER(frame), hbox);
    gtk_widget_show(hbox);

    GtkWidget *vbox = gtk_vbox_new(FALSE, 5);
    gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 10);
    gtk_widget_show(vbox);

    // Assume download was completed
    s416SuccButton = gtk_radio_button_new_with_label(NULL, _("Assume download was completed"));
    gtk_widget_show(s416SuccButton);
    GSList *group = gtk_radio_button_group(GTK_RADIO_BUTTON(s416SuccButton));
    gtk_box_pack_start(GTK_BOX(vbox), s416SuccButton, FALSE, FALSE, 2);

    // Error
    s416ErrButton = gtk_radio_button_new_with_label(group, _("Error"));
    gtk_box_pack_start(GTK_BOX(vbox), s416ErrButton, FALSE, FALSE, 2);
    gtk_widget_show(s416ErrButton);

    // Redownload
    s416RedownButton = gtk_radio_button_new_with_label(gtk_radio_button_group(GTK_RADIO_BUTTON(s416ErrButton)), _("Redownload"));
    gtk_box_pack_start(GTK_BOX(vbox), s416RedownButton, FALSE, FALSE, 2);
    gtk_widget_show(s416RedownButton);
  }
  {
    use_no_redirection_toggle = gtk_check_button_new_with_label(_("Do not follow redirection"));
    gtk_widget_show(use_no_redirection_toggle);
    gtk_box_pack_start(GTK_BOX(vbox), use_no_redirection_toggle, FALSE, FALSE, 0);
  }
  {
    httpAcceptCompressionToggle = gtk_check_button_new_with_label(_("Accept compression encodings"));
    gtk_widget_show(httpAcceptCompressionToggle);
    gtk_box_pack_start(GTK_BOX(vbox), httpAcceptCompressionToggle, FALSE, FALSE, 0);
#ifndef HAVE_ZLIB
    gtk_widget_hide(httpAcceptCompressionToggle);
#endif // HAVE_ZLIB
  }
  {
    // Accept-Language request field
    GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
    gtk_widget_show(hbox);

    httpAcceptLangEnabledToggle = gtk_check_button_new_with_label(_("Use Accept-Language request header"));
    gtk_box_pack_start(GTK_BOX(hbox), httpAcceptLangEnabledToggle, FALSE, FALSE, 0);
    gtk_widget_show(httpAcceptLangEnabledToggle);

    httpAcceptLangStringEntry = gtk_entry_new();
    gtk_widget_set_usize(httpAcceptLangStringEntry, 200, -1);
    gtk_box_pack_start(GTK_BOX(hbox), httpAcceptLangStringEntry, FALSE, FALSE, 0);
    gtk_widget_show(httpAcceptLangStringEntry);
  }
  {
    GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
    gtk_widget_show(hbox);

    GtkWidget *label = gtk_label_new(_("HTTP version "));
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
    gtk_widget_show(label);

    http_version_select = gtk_option_menu_new();
    gtk_widget_show(http_version_select);
    gtk_box_pack_start(GTK_BOX(hbox), http_version_select, FALSE, FALSE, 0);

    GtkWidget *menu = gtk_menu_new();
    for(unsigned int i = 0; i < sizeof(http_version_string)/sizeof(http_version_string[0]); ++i) {
      GtkWidget *item = gtk_menu_item_new_with_label(http_version_string[i]);
      g_signal_connect(GTK_OBJECT(item), "activate",
			 GTK_SIGNAL_FUNC(http_version_callback),
			 GUINT_TO_POINTER(i));
      gtk_widget_show(item);
      gtk_menu_append(GTK_MENU(menu), item);
    }
    gtk_option_menu_set_menu(GTK_OPTION_MENU(http_version_select), menu);
  }
  {
    GtkWidget *frame = gtk_frame_new(_("Name of the file to use as a pre-written HTML"));
    gtk_widget_show(frame);
    gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 10);

    GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
    gtk_container_add(GTK_CONTAINER(frame), hbox);
    gtk_widget_show(hbox);

    GtkWidget *vbox = gtk_vbox_new(FALSE, 5);
    gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 10);
    gtk_widget_show(vbox);

    // index.html
    prewrittenHTMLIndexButton = gtk_radio_button_new_with_label(NULL, _("index.html"));
    gtk_widget_show(prewrittenHTMLIndexButton);
    GSList *group = gtk_radio_button_group(GTK_RADIO_BUTTON(prewrittenHTMLIndexButton));
    gtk_box_pack_start(GTK_BOX(vbox), prewrittenHTMLIndexButton, FALSE, FALSE, 2);

    GtkWidget *userDefinedHbox = gtk_hbox_new(FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), userDefinedHbox, FALSE, FALSE, 0);
    gtk_widget_show(userDefinedHbox);

    // User defined
    prewrittenHTMLUserDefinedButton = gtk_radio_button_new_with_label(group, _("User defined"));
    gtk_box_pack_start(GTK_BOX(userDefinedHbox), prewrittenHTMLUserDefinedButton, FALSE, FALSE, 2);
    gtk_widget_show(prewrittenHTMLUserDefinedButton);

    prewrittenHTMLUserDefinedEntry = gtk_entry_new();
    gtk_widget_set_usize(prewrittenHTMLUserDefinedEntry, 150, -1);
    gtk_widget_show(prewrittenHTMLUserDefinedEntry);
    gtk_box_pack_start(GTK_BOX(userDefinedHbox), prewrittenHTMLUserDefinedEntry, FALSE, FALSE, 5);
  }

  return hbox;
}

GtkWidget *ItemOption::Create_FTP_Proxy_page()
{
  GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
  gtk_widget_show(hbox);
  GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
  gtk_widget_show(vbox);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 5);

  // FTP Proxy setting
  {
    GtkWidget *proxy_frame = gtk_frame_new(_("Proxy"));
    gtk_widget_show(proxy_frame);
    gtk_box_pack_start(GTK_BOX(vbox), proxy_frame, FALSE, FALSE, 0);

    GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
    gtk_widget_show(hbox);
    gtk_container_add(GTK_CONTAINER(proxy_frame), hbox);

    GtkWidget *vbox = gtk_vbox_new(FALSE, 5);
    gtk_widget_show(vbox);
    gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 10);


    // use ftp proxy or not
    {
      use_ftp_proxy_toggle = gtk_check_button_new_with_label(_("Use proxy"));
      gtk_widget_show(use_ftp_proxy_toggle);
      gtk_box_pack_start(GTK_BOX(vbox), use_ftp_proxy_toggle, FALSE, FALSE, 2);
    }

    // authentication
    {
      GtkWidget *authentication_frame = gtk_frame_new(_("Authentication"));
      gtk_widget_show(authentication_frame);
      gtk_box_pack_start(GTK_BOX(vbox), authentication_frame, FALSE, FALSE, 10);
      GtkWidget *authentication_vbox = gtk_vbox_new(FALSE, 5);
      gtk_widget_show(authentication_vbox);
      gtk_container_add(GTK_CONTAINER(authentication_frame), authentication_vbox);

      {
	GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(authentication_vbox), hbox, FALSE, FALSE, 10);

	toggle_use_ftp_proxy_authentication = gtk_check_button_new_with_label(_("Use authentication"));
	gtk_widget_show(toggle_use_ftp_proxy_authentication);
	gtk_box_pack_start(GTK_BOX(hbox), toggle_use_ftp_proxy_authentication, FALSE, FALSE, 0);
      }
      {
	GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(authentication_vbox), hbox, FALSE, FALSE, 10);
    
	GtkWidget *label = gtk_label_new(_("User name"));
	gtk_widget_show(label);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 10);
      
	ftp_proxy_user_entry = gtk_entry_new();//_with_max_length(127);
	gtk_widget_set_usize(GTK_WIDGET(ftp_proxy_user_entry), 400, -1);
	gtk_widget_show(ftp_proxy_user_entry);
	gtk_box_pack_start(GTK_BOX(hbox), ftp_proxy_user_entry, FALSE, FALSE, 10);
      }
      {
	GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(authentication_vbox), hbox, FALSE, FALSE, 10);
	
	GtkWidget *label = gtk_label_new(_("Password"));
	gtk_widget_show(label);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 10);
      
	ftp_proxy_password_entry = gtk_entry_new();//_with_max_length(127);
	gtk_widget_set_usize(GTK_WIDGET(ftp_proxy_password_entry), 400, -1);
	gtk_entry_set_visibility(GTK_ENTRY(ftp_proxy_password_entry), FALSE);
	gtk_widget_show(ftp_proxy_password_entry);
	gtk_box_pack_start(GTK_BOX(hbox), ftp_proxy_password_entry, FALSE, FALSE, 10);
      }
    }

    // toggle whether use cache or not
    /*
    toggle_use_ftp_cache = gtk_check_button_new_with_label(_("Use cache"));
    gtk_widget_show(toggle_use_ftp_cache);
    gtk_box_pack_start(GTK_BOX(vbox), toggle_use_ftp_cache, FALSE, FALSE, 0);
    */
    {
      GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
      gtk_widget_show(hbox);
      gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);

      // label
      GtkWidget *name_label = gtk_label_new(_("Server : Port"));
      gtk_widget_show(name_label);
      gtk_box_pack_start(GTK_BOX(hbox), name_label, FALSE, FALSE, 0);

      // combobox
      ftp_proxy_cbox = gtk_combo_new();
      gtk_widget_set_usize(ftp_proxy_cbox, 300, -1);
      gtk_widget_show(ftp_proxy_cbox);
      gtk_box_pack_start(GTK_BOX(hbox), ftp_proxy_cbox, FALSE, FALSE, 10);

      // add button
      GtkWidget *ftp_proxy_add_button = gtk_button_new_with_label(_("Add"));
      gtk_widget_show(ftp_proxy_add_button);
      gtk_box_pack_start(GTK_BOX(hbox), ftp_proxy_add_button, FALSE, FALSE, 0);
      g_signal_connect(GTK_OBJECT(ftp_proxy_add_button), "clicked",
			 GTK_SIGNAL_FUNC(Add_ftp_proxy_entry),
			 GTK_OBJECT(ftp_proxy_cbox));

      // delete button
      GtkWidget *ftp_proxy_del_button = gtk_button_new_with_label(_("Delete"));
      gtk_widget_show(ftp_proxy_del_button);
      gtk_box_pack_start(GTK_BOX(hbox), ftp_proxy_del_button, FALSE, FALSE, 0);
      g_signal_connect(GTK_OBJECT(ftp_proxy_del_button), "clicked",
			 GTK_SIGNAL_FUNC(Delete_ftp_proxy_entry),
			 GTK_OBJECT(ftp_proxy_cbox));
    }
    {
      use_ftp_cache_toggle = gtk_check_button_new_with_label(_("Use cache"));
      gtk_widget_show(use_ftp_cache_toggle);
      gtk_box_pack_start(GTK_BOX(vbox), use_ftp_cache_toggle, FALSE, FALSE, 0);
    }
    {
      use_ftp_proxy_via_http_toggle = gtk_check_button_new_with_label(_("Via HTTP proxy"));
      gtk_widget_show(use_ftp_proxy_via_http_toggle);
      gtk_box_pack_start(GTK_BOX(vbox), use_ftp_proxy_via_http_toggle, FALSE, FALSE, 0);
    }
    {
      GtkWidget *frame = gtk_frame_new(_("Login procedure"));
      gtk_widget_show(frame);
      gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);
      
      GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
      gtk_widget_show(hbox);
      gtk_container_add(GTK_CONTAINER(frame), hbox);
      GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
      gtk_widget_show(vbox);
      gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 5);

      ftp_proxy_login_proc_cbox = gtk_combo_new();
      gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(ftp_proxy_login_proc_cbox)->entry), FALSE);
      gtk_widget_set_usize(ftp_proxy_login_proc_cbox, 500, -1);
      gtk_widget_show(ftp_proxy_login_proc_cbox);
      gtk_box_pack_start(GTK_BOX(vbox), ftp_proxy_login_proc_cbox, FALSE, FALSE, 5);

      GList *ftp_login_proc_glist = NULL;
      ftp_login_proc_glist = g_list_append(ftp_login_proc_glist, (void *)DFTPPROXY_PL);
      ftp_login_proc_glist = g_list_append(ftp_login_proc_glist, (void *)DFTPPROXY_PL_OPEN);
      ftp_login_proc_glist = g_list_append(ftp_login_proc_glist, (void *)DFTPPROXY_PL_OPEN2);
      ftp_login_proc_glist = g_list_append(ftp_login_proc_glist, (void *)DFTPPROXY_PL_SITE);
      ftp_login_proc_glist = g_list_append(ftp_login_proc_glist, (void *)DFTPPROXY_PL_USER);
      ftp_login_proc_glist = g_list_append(ftp_login_proc_glist, (void *)DFTPPROXY_USER);
      ftp_login_proc_glist = g_list_append(ftp_login_proc_glist, (void *)DFTPPROXY_OPEN);
      
      gtk_combo_set_popdown_strings(GTK_COMBO(ftp_proxy_login_proc_cbox), ftp_login_proc_glist);

      g_list_free(ftp_login_proc_glist);
      //gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(ftp_proxy_login_cbox)->entry), (char *)proc_glist->data);

    }
  }
  return hbox;
}

GtkWidget *ItemOption::Create_FTP_Mode_page()
{
  GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
  gtk_widget_show(hbox);
  GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
  gtk_widget_show(vbox);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 5);

  {
    GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
    gtk_widget_show(hbox);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 10);
    {
      GtkWidget *mode_frame = gtk_frame_new(_("FTP mode"));
      gtk_widget_show(mode_frame);
      gtk_box_pack_start(GTK_BOX(hbox), mode_frame, FALSE, FALSE, 10);
      
      GtkWidget *vbox = gtk_vbox_new(FALSE, 5);
      gtk_widget_show(vbox);
      gtk_container_add(GTK_CONTAINER(mode_frame), vbox);
      
      active_button = gtk_radio_button_new_with_label(NULL, "Active");
      gtk_widget_show(active_button);
      {
	GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(hbox), active_button, FALSE, FALSE, 10);
      }
      GSList *group = gtk_radio_button_group(GTK_RADIO_BUTTON(active_button));
    
      passive_button = gtk_radio_button_new_with_label(group, "Passive");
      gtk_widget_show(passive_button);
      {
	GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(hbox), passive_button, FALSE, FALSE, 10);
      }
    }
    {
       GtkWidget *mode_frame = gtk_frame_new(_("Retrieve mode"));
      gtk_widget_show(mode_frame);
      gtk_box_pack_start(GTK_BOX(hbox), mode_frame, FALSE, FALSE, 10);
      
      GtkWidget *vbox = gtk_vbox_new(FALSE, 5);
      gtk_widget_show(vbox);
      gtk_container_add(GTK_CONTAINER(mode_frame), vbox);
      
      binary_button = gtk_radio_button_new_with_label(NULL, "Binary");
      gtk_widget_show(binary_button);
      {
	GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(hbox), binary_button, FALSE, FALSE, 10);
      }
      GSList *group = gtk_radio_button_group(GTK_RADIO_BUTTON(binary_button));
    
      ascii_button = gtk_radio_button_new_with_label(group, "Ascii");
      gtk_widget_show(ascii_button);
      {
	GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(hbox), ascii_button, FALSE, FALSE, 10);
      }
    }
  }
  return hbox;
}

GtkWidget *ItemOption::Create_FTP_Misc_page()
{
  GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
  gtk_widget_show(hbox);
  GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
  gtk_widget_show(vbox);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 5);

  {
    ftp_nosend_quit_toggle = gtk_check_button_new_with_label(_("Don't send QUIT command"));
    gtk_widget_show(ftp_nosend_quit_toggle);
    gtk_box_pack_start(GTK_BOX(vbox), ftp_nosend_quit_toggle, FALSE, FALSE, 0);
  }
  {
    ftpNoCwdToggle = gtk_check_button_new_with_label(_("Don't send CWD command, instead use absolute path."));
    gtk_widget_show(ftpNoCwdToggle);
    gtk_box_pack_start(GTK_BOX(vbox), ftpNoCwdToggle, FALSE, FALSE, 0);
  }

  return hbox;
}

GtkWidget *ItemOption::Create_FTP_Recursive_page()
{
  GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
  gtk_widget_show(hbox);
  GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
  gtk_widget_show(vbox);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 5);
  
  {
    // frame
    GtkWidget *frame = gtk_frame_new(_("Recursive download"));
    gtk_widget_show(frame);
    gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 10);

    GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
    gtk_widget_show(hbox);
    gtk_container_add(GTK_CONTAINER(frame), hbox);
    {
      GtkWidget *vbox1 = gtk_vbox_new(FALSE, 5);
      gtk_box_pack_start(GTK_BOX(hbox), vbox1, FALSE, FALSE, 10);
      gtk_widget_show(vbox1);

      // depth of recursion
      {
	GtkWidget *hbox = HBOX_NEW(vbox1);
	gtk_widget_show(hbox);
	
	GtkWidget *label = gtk_label_new(_("Depth of recursion "));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 10);
	gtk_widget_show(label);
	GtkObject *adjustment = gtk_adjustment_new(1,//value
						   1,//lower
						   MAXRECURSION,//upper
						   1,//step inc
						   10,//page inc
						   0);//page size
	ftp_recurse_depth_spin = gtk_spin_button_new(GTK_ADJUSTMENT(adjustment),
						     1.0,
						     0);
	gtk_widget_show(ftp_recurse_depth_spin);
	gtk_box_pack_start(GTK_BOX(hbox), ftp_recurse_depth_spin, FALSE, FALSE, 10);
      }
      {
	ftp_no_ascend_toggle = gtk_check_button_new_with_label(_("Do not ascend to parent directory"));
	gtk_widget_show(ftp_no_ascend_toggle);
	gtk_box_pack_start(GTK_BOX(vbox1), ftp_no_ascend_toggle, FALSE, FALSE, 0);
      }
      {
	ftp_get_symlink_as_realfile_toggle = gtk_check_button_new_with_label(_("Retrieve symbolic links as real files"));
	gtk_widget_show(ftp_get_symlink_as_realfile_toggle);
	gtk_box_pack_start(GTK_BOX(vbox1), ftp_get_symlink_as_realfile_toggle, FALSE, FALSE, 0);
      }
      {
	ftpRecurseAddPasteToggle = gtk_check_button_new_with_label(_("Add to paste window"));
	gtk_widget_show(ftpRecurseAddPasteToggle);
	gtk_box_pack_start(GTK_BOX(vbox1), ftpRecurseAddPasteToggle, FALSE, FALSE, 0);
      }
      /*
      GtkWidget *vbox2 = gtk_vbox_new(FALSE, 5);
      gtk_box_pack_start(GTK_BOX(hbox), vbox2, FALSE, FALSE, 10);
      gtk_widget_show(vbox2);
      */
      // Download Target extension clist
      {
	// frame
	GtkWidget *frame = gtk_frame_new(_("Patterns to download"));
	gtk_widget_show(frame);
	gtk_box_pack_start(GTK_BOX(vbox1), frame, FALSE, FALSE, 5);
	
	{
	  GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
	  gtk_container_add(GTK_CONTAINER(frame), hbox);
	  gtk_widget_show(hbox);
	  
	  {
	    GtkWidget *vbox = gtk_vbox_new(FALSE, 5);
	    gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);
	    gtk_widget_show(vbox);

	    ftp_use_filter_toggle = gtk_check_button_new_with_label(_("Use this filter"));
	    gtk_widget_show(ftp_use_filter_toggle);
	    gtk_box_pack_start(GTK_BOX(vbox), ftp_use_filter_toggle, FALSE, FALSE, 0);

	    GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
	    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	    gtk_widget_show(hbox);
	      
	    ftp_filter_entry = gtk_entry_new();
	    gtk_widget_set_usize(ftp_filter_entry, 100, -1);
	    gtk_box_pack_start(GTK_BOX(hbox), ftp_filter_entry, FALSE, FALSE, 10);
	    gtk_widget_show(ftp_filter_entry);
	      
	    GtkWidget *add_button = gtk_button_new_with_label(_("Add"));
	    gtk_widget_show(add_button);
	    gtk_box_pack_start(GTK_BOX(hbox), add_button, FALSE, FALSE, 0);
	    g_signal_connect_swapped(GTK_OBJECT(ftp_filter_entry), "activate",
				      GTK_SIGNAL_FUNC(gtk_button_clicked),
				      GTK_OBJECT(add_button));
	    
	    //ダウンロードするファイルのパターンのclist
	    int n_titles = 1;
	    char *titles[n_titles];
	    titles[0] = _("Pattern");
	    // スクロールウインドウにCListをバッキング
	    GtkWidget *scrolled_window = Create_CheckCList(&ftp_filter_clist, titles, n_titles);
	    gtk_clist_set_column_auto_resize(GTK_CLIST(recurse_nodown_filter_target_clist), 0, FALSE);
	    Create_popup_menu(&ftp_filter_clist);
	    gtk_object_set_user_data(GTK_OBJECT(ftp_filter_clist), ftp_filter_entry);
	    g_signal_connect(GTK_OBJECT(add_button), "clicked",
			       GTK_SIGNAL_FUNC(Add_target_item2),
			       GTK_OBJECT(ftp_filter_clist));
	    
	    gtk_widget_set_usize(scrolled_window, 100, 110);
	    
	    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, FALSE, FALSE, 5);
	    gtk_widget_show(scrolled_window);
	  }
	}
      }
    }
  }

  return hbox;
}

void ItemOption::show()
{
  visibleFlag = true;
  // initialize each tab page
  gtk_notebook_set_page(GTK_NOTEBOOK(notebook), 0);
  gtk_notebook_set_page(GTK_NOTEBOOK(general_notebook), 0);
  gtk_notebook_set_page(GTK_NOTEBOOK(http_notebook), 0);
  gtk_notebook_set_page(GTK_NOTEBOOK(ftp_notebook), 0);

  gtk_window_set_position(GTK_WINDOW(option_window), GTK_WIN_POS_MOUSE);
  gtk_widget_show(option_window);
}

void ItemOption::setOptionValues(ItemCell *itemcell_in,
				 const Options& options,
				 ListEntry *listentry_in) {
  string line;

  itemcell = itemcell_in;
  listentry = listentry_in;

  if(g_itemManagerPaste->search_item(itemcell)) {
    // from paste list
    if(g_pasteWindow->getSelectedItemNum() > 1) {
      multipleSelectionFlag = true;
    } else {
      multipleSelectionFlag = false;
    }
  } else {
    // from download list
    if(listentry != NULL &&
//       g_list_length(GTK_CLIST(listentry->ret_Dl_clist())->selection) > 1) {
       listentry->getSelectedRowCount() > 1) {//TODO: condition to asingment
      multipleSelectionFlag = true;
    } else {
      multipleSelectionFlag = false;
    }
  }

  // speed limiter
  // speed spin
  GtkAdjustment *adj = gtk_range_get_adjustment(GTK_RANGE(speed_scale));
  adj->upper = g_appOption->getSpeedLimitMax();
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(speedSpin), options.ret_speed_limit());

  // speed scale
  adj = gtk_range_get_adjustment(GTK_RANGE(speed_scale));
  adj->value = options.ret_speed_limit();
  adj->upper = g_appOption->getSpeedLimitMax();

  // URL page
  if(itemcell == g_consoleItem ||
     (listentry != NULL && itemcell == listentry->ret_Default_item()) ||
     multipleSelectionFlag) {
    gtk_widget_set_sensitive(url_entry, FALSE);
    gtk_widget_set_sensitive(crc_entry, FALSE);
    gtk_widget_set_sensitive(md5_entry, FALSE);
    gtk_widget_set_sensitive(filename_entry, FALSE);

    if(multipleSelectionFlag) {
      gtk_window_set_title(GTK_WINDOW(option_window), _("Selected items option"));
    } else if(listentry != NULL) {
      string title = _("Default item option for ");
      title += "'"+listentry->getName()+"'";
      gtk_window_set_title(GTK_WINDOW(option_window), title.c_str());
    } else {
      gtk_window_set_title(GTK_WINDOW(option_window), _("Default item option for new list"));
    }
    gtk_entry_set_text(GTK_ENTRY(url_entry), "");
    gtk_entry_set_text(GTK_ENTRY(crc_entry), "");
    gtk_entry_set_text(GTK_ENTRY(md5_entry), "");    
    gtk_entry_set_text(GTK_ENTRY(filename_entry), "");

    // switch on Sync with URL toggle button
    if(sync_with_url_toggled_cb_id != 0) {
      gtk_signal_disconnect(GTK_OBJECT(sync_with_url_toggle),
			    sync_with_url_toggled_cb_id);
      sync_with_url_toggled_cb_id = 0;
    }
  } else {
    if(itemcell->Is_Partial()) {
      gtk_widget_set_sensitive(url_entry, FALSE);
      gtk_widget_set_sensitive(crc_entry, FALSE);
      gtk_widget_set_sensitive(md5_entry, FALSE);
      gtk_widget_set_sensitive(filename_entry, FALSE);
      gtk_widget_set_sensitive(storedir_entry, FALSE);

      if(sync_with_url_toggled_cb_id != 0) {
	gtk_signal_disconnect(GTK_OBJECT(sync_with_url_toggle),
			      sync_with_url_toggled_cb_id);
	sync_with_url_toggled_cb_id = 0;
      }
    } else {
      gtk_widget_set_sensitive(url_entry, TRUE);
      gtk_widget_set_sensitive(crc_entry, TRUE);
      gtk_widget_set_sensitive(md5_entry, TRUE);
      gtk_widget_set_sensitive(filename_entry, TRUE);
      gtk_widget_set_sensitive(storedir_entry, TRUE);

      if(sync_with_url_toggled_cb_id == 0) {
	sync_with_url_toggled_cb_id = g_signal_connect(GTK_OBJECT(sync_with_url_toggle),
							 "toggled",
							 GTK_SIGNAL_FUNC(sync_with_url_toggled_cb),
							 NULL);
      }
    }

    string fix;
    // fixed 2001/7/4
    if(itemcell->ret_Filename_opt().empty()) {
      fix = _("<directory>");
    } else {
      fix = itemcell->ret_Filename_opt();
    }
    line = '\''+fix+_("' option");

    gtk_window_set_title(GTK_WINDOW(option_window), line.c_str());

    gtk_entry_set_editable(GTK_ENTRY(url_entry), TRUE);
    gtk_entry_set_text(GTK_ENTRY(url_entry), itemcell->ret_URL_opt().c_str());

    gtk_entry_set_editable(GTK_ENTRY(crc_entry), TRUE);
    if(itemcell->ret_CRC_Type() == ItemCell::CRC_NONE) {
      gtk_entry_set_text(GTK_ENTRY(crc_entry), "");
    } else {
      //gtk_entry_set_sensitive(GTK_ENTRY(url_entry), TRUE);
      if(itemcell->ret_CRC_Type() == ItemCell::CRC_16) {
	line = itos_hex(itemcell->ret_CRC(), 4, '0');
      } else {
	line = itos_hex(itemcell->ret_CRC(), 8, '0');
      }
      gtk_entry_set_text(GTK_ENTRY(crc_entry), line.c_str());
      //gtk_entry_set_sensitive(GTK_ENTRY(crc_entry), TRUE);
    }

    gtk_entry_set_editable(GTK_ENTRY(md5_entry), TRUE);
    gtk_entry_set_text(GTK_ENTRY(md5_entry), itemcell->ret_md5string().c_str());

    gtk_entry_set_text(GTK_ENTRY(filename_entry), itemcell->ret_Filename_opt().c_str());
    //  gtk_notebook_set_page(GTK_NOTEBOOK(notebook), 0);
  }
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(sync_with_url_toggle),
			       options.ret_sync_with_URL());
  if(GTK_TOGGLE_BUTTON(sync_with_url_toggle)->active) {
    gtk_widget_set_sensitive(filename_entry, FALSE);
  } else if(itemcell != g_consoleItem &&
	    listentry != NULL && itemcell != listentry->ret_Default_item()) {
    gtk_widget_set_sensitive(filename_entry, TRUE);
  }

  gtk_entry_set_text(GTK_ENTRY(user_entry), options.ret_User().c_str());
  gtk_entry_set_text(GTK_ENTRY(password_entry), options.ret_Password().c_str());
  gtk_entry_set_text(GTK_ENTRY(storedir_entry), options.ret_Store_Dir().c_str());
  current_store_dir = options.ret_Store_Dir();

  // modified 2000/3/1
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle_use_authentication),
			       options.Whether_use_authentication());

  // Command page
  string command_string;
  for(list<string>::const_iterator itr = options.ret_Command().ret_command().begin(); itr != options.ret_Command().ret_command().end(); ++itr) {
    command_string += *itr;
  }
  //gtk_entry_set_text(GTK_ENTRY(command_entry), command_string.c_str());
  // delete all text
  //gtk_editable_delete_text(GTK_EDITABLE(command_entry), 0, -1);
  gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(command_entry)), "", 0);
  // then, set text
  //gtk_text_set_point(GTK_TEXT(command_entry), 0);
/*  int insertPos = 0;
  for(unsigned int i = 0; i < command_string.size(); i++) {
    if(command_string.at(i) == '\r') {
      command_string.replace(i, 1, "\n");
    }
  }*/
 //gtk_editable_insert_text(GTK_EDITABLE(command_entry),
 gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(command_entry)),
			   command_string.c_str(),
			   command_string.size()
			   );
			   //&insertPos);

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(use_command_toggle),
			       options.ret_use_Command());

  string exit_status_string;
  for(list<int>::const_iterator itr = options.ret_Command().ret_succ_status_list().begin(); itr != options.ret_Command().ret_succ_status_list().end(); ++itr) {
    exit_status_string += itos(*itr)+',';
  }
  if(exit_status_string.size()) exit_status_string.erase(exit_status_string.size()-1);
  gtk_entry_set_text(GTK_ENTRY(exit_status_entry), exit_status_string.c_str());

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(use_exit_status_toggle),
			       options.ret_use_Exit_status());


  // Download page
  // timed out
  GtkObject *adjustment_timedout = gtk_adjustment_new(options.ret_Timed_Out(),
						      0,
						      MAXTIMEOUT,
						      1,
						      10,
						      0);
  gtk_spin_button_configure(GTK_SPIN_BUTTON(spin_timedout),
			    GTK_ADJUSTMENT(adjustment_timedout),
			    1.0, 0);
  // retry
  GtkObject *adjustment_retry = gtk_adjustment_new(options.ret_Retry(),
						   -1,
						   MAXRETRY,
						   1,
						   5,
						   0);
  gtk_spin_button_configure(GTK_SPIN_BUTTON(spin_retry),
			    GTK_ADJUSTMENT(adjustment_retry),
			    1.0, 0);
  /*
  GtkObject *adjustment_retry_pb = gtk_adjustment_new(options.ret_Retry_repeat(),
						   0,
						   MAXRETRYPB,
						   1,
						   5,
						   0);
  gtk_spin_button_configure(GTK_SPIN_BUTTON(spin_retry_push_back),
			    GTK_ADJUSTMENT(adjustment_retry_pb),
			    1.0, 0);
  */
  // retry interval
  GtkObject *adjustment_retry_interval = gtk_adjustment_new(options.ret_Retry_interval(),
							    0,
							    MAXRETRYINTERVAL,
							    1,
							    5,
							    0);
  gtk_spin_button_configure(GTK_SPIN_BUTTON(spin_retry_interval),
			    GTK_ADJUSTMENT(adjustment_retry_interval),
			    1.0, 0);

  // divide
  GtkObject *adjustment_divide;
  //// modified 2001/3/1
  int n_divide;
  if(itemcell->Is_Partial()) {
    n_divide = 1;
    gtk_widget_set_sensitive(spin_divide, FALSE);
  } else {
    n_divide = options.ret_Divide();
    gtk_widget_set_sensitive(spin_divide, TRUE);
  }
  adjustment_divide = gtk_adjustment_new(n_divide,
					 1,
					 MAXSPLIT,
					 1,
					 5,
					 0);
  gtk_spin_button_configure(GTK_SPIN_BUTTON(spin_divide),
			    GTK_ADJUSTMENT(adjustment_divide),
			    1.0, 0);
  // Rollback in bytes
  gtk_entry_set_text(GTK_ENTRY(rollback_bytes_entry),
		     itos(options.ret_Rollback_bytes()).c_str());
  
  // Limit file size
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(use_size_lower_limit_toggle),
			       options.ret_use_size_lower_limit());
  gtk_entry_set_text(GTK_ENTRY(size_lower_limit_entry),
		     itos(options.ret_size_lower_limit()).c_str());
  // do not redownload files if resuming is not available
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(use_no_redownload_toggle),
			       options.ret_use_no_redownload());
  // do not download a file if the file with the same name exists
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(use_no_download_samename_toggle),
			       options.ret_use_no_download_samename());
  // allow delete after download
  ////modified 2001/3/1
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle_delete_when_finish),
			       options.ret_Delete_When_Finish());

  // don't delete if CRC is not set
  ////modified 2001/3/1
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle_dont_delete_without_crc),
			       options.ret_Dont_Delete_Without_CRC());
  // no crc check
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(no_crc_check_toggle),
			       options.ret_no_crc_checking());
  // ignore crc error
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ignore_crc_error_toggle),
			       options.ret_ignore_crc_error());
  // use Content-MD5 entity header field
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(use_content_md5_toggle),
			       options.ret_use_content_md5());
  // recursive download
  // depth of recursion
  GtkObject *adjustment_recurse;
  ////modified 2001/3/1
  int n_rec;
  if(itemcell->Is_Partial()) {
    n_rec = 1;
    gtk_widget_set_sensitive(recurse_depth_spin, FALSE);
  } else {
    n_rec = options.ret_recurse_count();
    gtk_widget_set_sensitive(recurse_depth_spin, TRUE);
  }
  adjustment_recurse = gtk_adjustment_new(n_rec,
					  1,
					  MAXRECURSION,
					  1,
					  10,
					  0);

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(recurse_hostname_dir_toggle), options.ret_with_hostname_dir());
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(recurse_abs2rel_toggle), options.ret_abs2rel_url());
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(recurse_force_convert_toggle), options.ret_force_convert());
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(recurse_del_comment_toggle), options.ret_delete_comment());
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(recurse_del_javascript_toggle), options.ret_delete_javascript());
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(recurse_del_iframe_toggle), options.ret_delete_iframe());
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(recurse_no_other_host_toggle), options.ret_no_other_host());
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(recurse_no_ascend_toggle), options.ret_no_ascend());
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(recurse_relative_only_toggle), options.ret_only_relative_links());
 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(recurse_referer_override_toggle), options.ret_Referer_override());

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(recurse_tag_href_toggle), options.ret_use_tag_href());
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(recurse_tag_src_toggle), options.ret_use_tag_src());
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(recurse_tag_background_toggle), options.ret_use_tag_background());
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(recurse_tag_code_toggle), options.ret_use_tag_code());
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(recurse_follow_ftp_link_toggle), options.ret_Follow_ftp_link());
  //gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(recurse_convert_tilde_toggle), options.ret_convert_tilde());
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(recurse_no_redownload_toggle),
			       options.ret_no_redownload_HTTP_recurse());
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(recurseAddPasteToggle),
			       options.ret_HTTP_recurse_add_paste());

  gtk_spin_button_configure(GTK_SPIN_BUTTON(recurse_depth_spin),
			    GTK_ADJUSTMENT(adjustment_recurse),
			    1.0, 0);

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(recurse_use_down_filter_toggle), options.ret_use_down_filter());
  setup_extension_target_clist(recurse_parse_target_clist,
			       options.ret_parse_target_list(),
			       options.ret_activated_parse_target_list());
  setup_extension_target_clist(recurse_down_filter_target_clist,
			       options.ret_filter_down_target_list(),
			       options.ret_activated_filter_down_target_list());
  setup_extension_target_clist(recurse_nodown_filter_target_clist,
			       options.ret_filter_nodown_target_list(),
			       options.ret_activated_filter_nodown_target_list());
  setup_extension_target_clist(recurse_ign_domain_clist,
			       options.ret_ign_domain_list(),
			       options.ret_activated_ign_domain_list());
  gtk_entry_set_text(GTK_ENTRY(recurse_down_filter_target_entry), "");
  gtk_entry_set_text(GTK_ENTRY(recurse_nodown_filter_target_entry), "");
  gtk_entry_set_text(GTK_ENTRY(recurse_parse_target_entry), "");
  gtk_entry_set_text(GTK_ENTRY(recurse_ign_domain_entry), "");
  // HTTP1 page
  // HTTP Version
  selected_http_version = 0;   // initilize
  for(unsigned int i = 0; i < sizeof(http_version_string)/sizeof(http_version_string[0]); ++i) {
    if(options.ret_HTTP_version() == http_version_string[i]) {
      selected_http_version = i;
      break;
    }
  }
  gtk_option_menu_set_history(GTK_OPTION_MENU(http_version_select), selected_http_version);

  // prewritten HTML name
  switch(options.getPrewrittenHTMLType()) {
  case Options::PREWRITTEN_HTML_INDEX:
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(prewrittenHTMLIndexButton), TRUE);
    break;
  case Options::PREWRITTEN_HTML_USERDEFINED:
  default:
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(prewrittenHTMLUserDefinedButton), TRUE);
    break;
  }
  gtk_entry_set_text(GTK_ENTRY(prewrittenHTMLUserDefinedEntry), options.getPrewrittenHTMLName().c_str());

  // Referer
  switch(options.ret_Referer_Type()) {
  case Options::REFERER_NONE:
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(none_button), TRUE);
    //gtk_entry_set_text(GTK_ENTRY(referer_entry), options.ret_Referer().c_str());
    break;
  case Options::REFERER_USER_DEFINED:
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(user_button), TRUE);
    break;
  case Options::REFERER_URL:
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(urlref_button), TRUE);
    break;
  case Options::REFERER_INDEX:
  default:
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(index_button), TRUE);
    break;
  }
  gtk_entry_set_text(GTK_ENTRY(referer_entry), options.ret_Referer().c_str());
  // force retry even if HTTP status is 404.
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(use_retry_404_toggle),
			       options.ret_force_retry_404());
  // force retry even if HTTP status is 503.
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(use_retry_503_toggle),
			       options.ret_force_retry_503());
  // redownload if HTTP status is 416
  switch(options.ret_status_416_handling()) {
  case Options::S416SUCC:
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s416SuccButton), TRUE);
    break;
  case Options::S416ERR:
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s416ErrButton), TRUE);
    break;
  case Options::S416REDOWN:
  default:
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(s416RedownButton), TRUE);
    break;
  }
  // do not follow redirection
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(use_no_redirection_toggle),
			       options.ret_use_no_redirection());

  // Accept-encoding request header
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(httpAcceptCompressionToggle),
			       options.ret_HTTP_accept_compression());

  // Accept-Language request header
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(httpAcceptLangEnabledToggle),
			       options.ret_HTTP_accept_lang_enabled());
  gtk_entry_set_text(GTK_ENTRY(httpAcceptLangStringEntry),
		     options.ret_HTTP_accept_lang_string().c_str());

  // http proxy list
  // use authentication
  ////modified 2001/3/1
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle_use_http_proxy_authentication), options.ret_use_http_proxy_authentication());
  gtk_entry_set_text(GTK_ENTRY(http_proxy_user_entry), options.ret_http_proxy_User().c_str());
  //http proxy entry
  gtk_entry_set_text(GTK_ENTRY(http_proxy_password_entry), options.ret_http_proxy_Password().c_str());

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(use_http_proxy_toggle), options.ret_use_http_proxy());

  // use cache or not
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle_use_http_cache), options.ret_use_http_cache());

  if(options.ret_http_proxy().ret_Server().empty()) {
    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(http_proxy_cbox)->entry), "");
  } else {
    string proxy_entry = options.ret_http_proxy().ret_Server()+":"+itos(options.ret_http_proxy().ret_Port());
    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(http_proxy_cbox)->entry), proxy_entry.c_str());
  }

  if(sg_httpProxyListTemp != NULL) delete sg_httpProxyListTemp;
  sg_httpProxyListTemp = new ProxyList(*g_httpProxyList);

  // FTP proxy
  // use authentication
  ////modified 2001/3/1
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle_use_ftp_proxy_authentication), options.ret_use_ftp_proxy_authentication());
  gtk_entry_set_text(GTK_ENTRY(ftp_proxy_user_entry), options.ret_ftp_proxy_User().c_str());
  //ftp proxy entry
  gtk_entry_set_text(GTK_ENTRY(ftp_proxy_password_entry), options.ret_ftp_proxy_Password().c_str());

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(use_ftp_proxy_toggle), options.ret_use_ftp_proxy());

  if(options.ret_ftp_proxy().ret_Server().empty()) {
    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(ftp_proxy_cbox)->entry), "");
  } else {
    string proxy_entry = options.ret_ftp_proxy().ret_Server()+":"+itos(options.ret_ftp_proxy().ret_Port());
    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(ftp_proxy_cbox)->entry), proxy_entry.c_str());
  }

  if(sg_ftpProxyListTemp != NULL) delete sg_ftpProxyListTemp;
  sg_ftpProxyListTemp = new ProxyList(*g_ftpProxyList);

  //sg_httpProxyListTemp->set(*g_httpProxyList);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(use_ftp_cache_toggle), options.ret_use_ftp_cache());
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(use_ftp_proxy_via_http_toggle), options.ret_use_ftp_proxy_via_http());
  
  gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(ftp_proxy_login_proc_cbox)->entry), FTPproxy2DFTP(options.ret_ftp_proxy_login_proc()));

  // HTTP2 page

  ////useragent
  if(options.ret_Useragent().empty()) {
    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(useragent_cbox)->entry), "");
  } else {
    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(useragent_cbox)->entry), options.ret_Useragent().c_str());
  }
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle_random_useragent), options.ret_Random_useragent());

  ////cookie
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cookie_delete_on_restart_toggle), options.ret_Cookie_delete_on_restart());
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cookie_nosend_toggle), options.ret_Cookie_nosend());
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cookieUserDefinedToggle), options.getCookieUserDefined());
  gtk_entry_set_text(GTK_ENTRY(cookieUserDefinedEntry), options.getCookieUserDefinedString().c_str());

  // resume
  switch(options.ret_downm_type()) {
  case Options::DOWNM_ALWAYSRESUME:
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(alwaysresume_toggle), TRUE);
    break;
  case Options::DOWNM_IFMODSINCE:
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(useifmodsince_toggle), TRUE);
    break;
  case Options::DOWNM_NORESUME:
  default:
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(noresume_toggle), TRUE);
    break;
  }
  // FTP page
  switch(options.ret_FTP_Mode()) {
  case Options::FTP_ACTIVE_MODE:
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(active_button), TRUE);
    break;
  case Options::FTP_PASSIVE_MODE:
  default:
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(passive_button), TRUE);
    break;
  }
  switch(options.ret_FTP_ret_mode()) {
  case Options::FTP_BINARY:
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(binary_button), TRUE);
    break;
  case Options::FTP_ASCII:
  default:
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ascii_button), TRUE);
    break;
  }

  //// recursive download
  ////// depth of recursion
  GtkObject *adjustment_ftp_recurse;
  int ftp_n_rec;
  if(itemcell->Is_Partial()) {
    ftp_n_rec = 1;
    gtk_widget_set_sensitive(ftp_recurse_depth_spin, FALSE);
  } else {
    ftp_n_rec = options.ret_FTP_recurse_count();
    gtk_widget_set_sensitive(ftp_recurse_depth_spin, TRUE);
  }
  adjustment_ftp_recurse = gtk_adjustment_new(ftp_n_rec,
					  1,
					  MAXRECURSION,
					  1,
					  10,
					  0);
  gtk_spin_button_configure(GTK_SPIN_BUTTON(ftp_recurse_depth_spin),
			    GTK_ADJUSTMENT(adjustment_ftp_recurse),
			    1.0, 0);

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ftp_no_ascend_toggle), options.ret_FTP_no_ascend());
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ftp_get_symlink_as_realfile_toggle), options.ret_FTP_get_symlink_as_realfile());
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ftp_use_filter_toggle), options.ret_FTP_use_filter());
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ftpRecurseAddPasteToggle),
			       options.ret_FTP_recurse_add_paste());

  setup_extension_target_clist(ftp_filter_clist,
			       options.ret_FTP_filter_target_list(),
			       options.ret_FTP_activated_filter_target_list());
  gtk_entry_set_text(GTK_ENTRY(ftp_filter_entry), "");
  //// send QUIT command or not
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ftp_nosend_quit_toggle), options.ret_FTP_nosend_quit());
  // send CWD command toggle
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ftpNoCwdToggle), options.isFtpNoCwdEnabled());

  setSensitiveHTTPRecursive();
}

GtkWidget *ItemOption::getWindow() const
{
  return option_window;
}

ItemCell *ItemOption::ret_ItemCell() const
{
  return itemcell;
}

ListEntry *ItemOption::getListEntry() const
{
  return listentry;
}

int ItemOption::Process_changes()
{
  Options::RefererType referer_type;
  unsigned int timedout;
  bool delete_when_finish;

  bool dont_delete_without_crc;
  bool use_authentication = false; 
  string referer;
  string storedir;

  // Common authentication for HTTP and FTP
  Userdata user(Remove_white(gtk_entry_get_text(GTK_ENTRY(user_entry))),
		Remove_white(gtk_entry_get_text(GTK_ENTRY(password_entry))));
  if(GTK_TOGGLE_BUTTON(toggle_use_authentication)->active) {
    use_authentication = true;
  } else {
    use_authentication = false;
  }

  // Save directory
  storedir = Remove_white(gtk_entry_get_text(GTK_ENTRY(storedir_entry)));
  // Show dialog when the specified directory doesnot exist.
  struct stat dir_stat;
  if(stat(storedir.c_str(), &dir_stat) == -1 || !S_ISDIR(dir_stat.st_mode)) {
    temp_storedir = storedir;
    return OPTION_INVALID_DIR;
  }

  // Sync with URL text entry
  bool sync_with_url = false;
  if(GTK_TOGGLE_BUTTON(sync_with_url_toggle)->active) {
    sync_with_url = true;
  }

  // Command page
  bool use_command;
  if(GTK_TOGGLE_BUTTON(use_command_toggle)->active) {
    use_command = true;
  } else {
    use_command = false;
  }

  bool use_exit_status;
  if(GTK_TOGGLE_BUTTON(use_exit_status_toggle)->active) {
    use_exit_status = true;
  } else {
    use_exit_status = false;
  }

  //string command_string = Remove_white(gtk_entry_get_text(GTK_ENTRY(command_entry)));
  string command_string;
  GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(command_entry));
  GtkTextIter start_iter;
  GtkTextIter end_iter;
  gtk_text_buffer_get_start_iter(buffer, &start_iter);
  gtk_text_buffer_get_end_iter(buffer, &end_iter);
  char* command_string_char = gtk_text_buffer_get_text(buffer, &start_iter, &end_iter, false);
  command_string = command_string_char;
  g_free(command_string_char);

  string exit_status_string = Remove_white(gtk_entry_get_text(GTK_ENTRY(exit_status_entry)));

  Command command(command_string, exit_status_string);

  // HTTP Version
  string http_version = http_version_string[selected_http_version];

  // prewritten HTML name
  Options::PrewrittenHTMLType prewrittenHTMLType;
  if(GTK_TOGGLE_BUTTON(prewrittenHTMLIndexButton)->active) {
    prewrittenHTMLType = Options::PREWRITTEN_HTML_INDEX;
  } else {
    prewrittenHTMLType = Options::PREWRITTEN_HTML_USERDEFINED;
  }
  string prewrittenHTMLName = Remove_white(gtk_entry_get_text(GTK_ENTRY(prewrittenHTMLUserDefinedEntry)));

  // referer
  if(GTK_TOGGLE_BUTTON(index_button)->active) {
    referer_type = Options::REFERER_INDEX;
  } else if(GTK_TOGGLE_BUTTON(none_button)->active) {
    referer_type = Options::REFERER_NONE;
  } else if(GTK_TOGGLE_BUTTON(urlref_button)->active) {
    referer_type = Options::REFERER_URL;
  } else {
    referer_type = Options::REFERER_USER_DEFINED;
  }
  referer = Remove_white(gtk_entry_get_text(GTK_ENTRY(referer_entry)));

  // HTTP Proxy
  bool use_http_proxy_authentication;
  if(GTK_TOGGLE_BUTTON(toggle_use_http_proxy_authentication)->active) {
    use_http_proxy_authentication = true;
  } else {
    use_http_proxy_authentication = false;
  }
  Userdata http_proxy_user(Remove_white(gtk_entry_get_text(GTK_ENTRY(http_proxy_user_entry))),
			   Remove_white(gtk_entry_get_text(GTK_ENTRY(http_proxy_password_entry))));

  bool use_http_proxy;
  if(GTK_TOGGLE_BUTTON(use_http_proxy_toggle)->active) {
    use_http_proxy = true;    
  } else {
    use_http_proxy = false;
  }
  bool use_http_cache;
  if(GTK_TOGGLE_BUTTON(toggle_use_http_cache)->active) {
    use_http_cache = true;
  } else {
    use_http_cache = false;
  }

  string proxy_entry = Remove_white(gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(http_proxy_cbox)->entry)));
  std::size_t colon_pos;
  string proxyserver_name;
  int proxyserver_port = DEFAULT_HTTP_PROXY_PORT;
  if(string::npos != (colon_pos = proxy_entry.find(':')) && proxy_entry.substr(colon_pos).size() > 1) {
    proxyserver_name = proxy_entry.substr(0, colon_pos);
    proxyserver_port = stoi(proxy_entry.substr(colon_pos+1), 10);
  } else {
    proxyserver_name = proxy_entry;
  }

  Proxyserver http_proxy(proxyserver_name, proxyserver_port);

  // FTP Proxy
  bool use_ftp_proxy_authentication;
  if(GTK_TOGGLE_BUTTON(toggle_use_ftp_proxy_authentication)->active) {
    use_ftp_proxy_authentication = true;
  } else {
    use_ftp_proxy_authentication = false;
  }
  Userdata ftp_proxy_user(Remove_white(gtk_entry_get_text(GTK_ENTRY(ftp_proxy_user_entry))),
			  Remove_white(gtk_entry_get_text(GTK_ENTRY(ftp_proxy_password_entry))));

  bool use_ftp_proxy;
  if(GTK_TOGGLE_BUTTON(use_ftp_proxy_toggle)->active) {
    use_ftp_proxy = true;    
  } else {
    use_ftp_proxy = false;
  }
  proxy_entry = Remove_white(gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(ftp_proxy_cbox)->entry)));
  proxyserver_port = DEFAULT_FTP_PROXY_PORT;
  if(string::npos != (colon_pos = proxy_entry.find(':')) && proxy_entry.substr(colon_pos).size() > 1) {
    proxyserver_name = proxy_entry.substr(0, colon_pos);
    proxyserver_port = stoi(proxy_entry.substr(colon_pos+1), 10);
  } else {
    proxyserver_name = proxy_entry;
  }

  Proxyserver ftp_proxy(proxyserver_name, proxyserver_port);

  bool use_ftp_cache;
  if(GTK_TOGGLE_BUTTON(use_ftp_cache_toggle)->active) {
    use_ftp_cache = true;
  } else {
    use_ftp_cache = false;
  }
  bool use_ftp_proxy_via_http;
  if(GTK_TOGGLE_BUTTON(use_ftp_proxy_via_http_toggle)->active) {
    use_ftp_proxy_via_http = true;
  } else {
    use_ftp_proxy_via_http = false;
  }

  Options::FTPproxyLoginProcType ftp_proxy_login_proc = DFTP2FTPproxy(gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(ftp_proxy_login_proc_cbox)->entry)));

  // useragent
  string useragent = Remove_white(gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(useragent_cbox)->entry)));
  bool random_useragent = false;
  if(GTK_TOGGLE_BUTTON(toggle_random_useragent)->active) {
    random_useragent = true;
  }
  // cookie
  bool cookie_delete_on_restart = false;
  if(GTK_TOGGLE_BUTTON(cookie_delete_on_restart_toggle)->active) {
    cookie_delete_on_restart = true;
  }
  bool cookie_nosend = false;
  if(GTK_TOGGLE_BUTTON(cookie_nosend_toggle)->active) {
    cookie_nosend = true;
  }
  bool cookieUserDefined;
  if(GTK_TOGGLE_BUTTON(cookieUserDefinedToggle)->active) {
    cookieUserDefined = true;
  } else {
    cookieUserDefined = false;
  }
  string cookieUserDefinedString = Remove_white(gtk_entry_get_text(GTK_ENTRY(cookieUserDefinedEntry)));

  // download method type
 Options::DownloadMethodType downm_type = Options::DOWNM_NORESUME;
  if(GTK_TOGGLE_BUTTON(alwaysresume_toggle)->active) {
    downm_type = Options::DOWNM_ALWAYSRESUME;
  } else if(GTK_TOGGLE_BUTTON(useifmodsince_toggle)->active) {
    downm_type = Options::DOWNM_IFMODSINCE;
  } 
 
  float speed_value = gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(speedSpin));
  // depth of recursion
  int recurse_count = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(recurse_depth_spin));;
  // extensions to download in recursive download
  bool use_down_filter;
  if(GTK_TOGGLE_BUTTON(recurse_use_down_filter_toggle)->active) {
    use_down_filter = true;
  } else {
    use_down_filter = false;
  }
  list<string> filter_down_target_list, activated_filter_down_target_list;
  get_extension_target_clist(recurse_down_filter_target_clist, filter_down_target_list, activated_filter_down_target_list);
  // extensions not to download in recursive download
  list<string> filter_nodown_target_list, activated_filter_nodown_target_list;
  get_extension_target_clist(recurse_nodown_filter_target_clist, filter_nodown_target_list, activated_filter_nodown_target_list);
  // extensions to parse
  list<string> parse_target_list, activated_parse_target_list;
  get_extension_target_clist(recurse_parse_target_clist, parse_target_list, activated_parse_target_list);
  // various options for recursive download

  bool recurse_with_hostname_dir = false;
  if(GTK_TOGGLE_BUTTON(recurse_hostname_dir_toggle)->active) {
    recurse_with_hostname_dir = true;
  }
  bool recurse_abs2rel = false;
  if(GTK_TOGGLE_BUTTON(recurse_abs2rel_toggle)->active) {
    recurse_abs2rel = true;
  }
  bool recurse_force_convert = false;
  if(GTK_TOGGLE_BUTTON(recurse_force_convert_toggle)->active) {
    recurse_force_convert = true;
  }
  bool recurse_del_comment = false;
  if(GTK_TOGGLE_BUTTON(recurse_del_comment_toggle)->active) {
    recurse_del_comment = true;
  }
  bool recurse_del_javascript = false;
  if(GTK_TOGGLE_BUTTON(recurse_del_javascript_toggle)->active) {
    recurse_del_javascript = true;
  }
  bool recurse_del_iframe = false;
  if(GTK_TOGGLE_BUTTON(recurse_del_iframe_toggle)->active) {
    recurse_del_iframe = true;
  }
  bool recurse_no_other_host = false;
  if(GTK_TOGGLE_BUTTON(recurse_no_other_host_toggle)->active) {
    recurse_no_other_host = true;
  }
  bool recurse_no_ascend = false;
  if(GTK_TOGGLE_BUTTON(recurse_no_ascend_toggle)->active) {
    recurse_no_ascend = true;
  }
  bool recurse_relative_only = false;
  if(GTK_TOGGLE_BUTTON(recurse_relative_only_toggle)->active) {
    recurse_relative_only = true;
  }
  bool recurse_referer_override = false;
  if(GTK_TOGGLE_BUTTON(recurse_referer_override_toggle)->active) {
    recurse_referer_override = true;
  }
  bool recurse_follow_ftp_link = false;
  if(GTK_TOGGLE_BUTTON(recurse_follow_ftp_link_toggle)->active) {
    recurse_follow_ftp_link = true;
  }
  bool recurse_convert_tilde = false;
  bool recurse_no_redownload = false;
  if(GTK_TOGGLE_BUTTON(recurse_no_redownload_toggle)->active) {
    recurse_no_redownload = true;
  }

  bool recurseAddPaste = false;
  if(GTK_TOGGLE_BUTTON(recurseAddPasteToggle)->active) {
    recurseAddPaste = true;
  }

  /*
  if(GTK_TOGGLE_BUTTON(recurse_convert_tilde_toggle)->active) {
    recurse_convert_tilde = true;
  }
  */
  bool recurse_use_tag_href = false;
  if(GTK_TOGGLE_BUTTON(recurse_tag_href_toggle)->active) {
    recurse_use_tag_href = true;
  }
  bool recurse_use_tag_src = false;
  if(GTK_TOGGLE_BUTTON(recurse_tag_src_toggle)->active) {
    recurse_use_tag_src = true;
  }
  bool recurse_use_tag_background = false;
  if(GTK_TOGGLE_BUTTON(recurse_tag_background_toggle)->active) {
    recurse_use_tag_background = true;
  }
  bool recurse_use_tag_code = false;
  if(GTK_TOGGLE_BUTTON(recurse_tag_code_toggle)->active) {
    recurse_use_tag_code = true;
  }

  list<string> ign_domain_list, activated_ign_domain_list;
  get_extension_target_clist(recurse_ign_domain_clist, ign_domain_list, activated_ign_domain_list);

  // timed out
  timedout = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_timedout));
  // retry
  int retry = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_retry));
  int retry_repeat = 0;
  //int retry_repeat = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_retry_push_back));
  // retry interval
  int retry_interval = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_retry_interval));
  bool use_retry_404 = false;
  if(GTK_TOGGLE_BUTTON(use_retry_404_toggle)->active) {
    use_retry_404 = true;
  }
  bool use_retry_503 = false;
  if(GTK_TOGGLE_BUTTON(use_retry_503_toggle)->active) {
    use_retry_503 = true;
  }
  Options::Status416HandlingType s416Handling;
  if(GTK_TOGGLE_BUTTON(s416SuccButton)->active) {
    s416Handling = Options::S416SUCC;
  } else if(GTK_TOGGLE_BUTTON(s416ErrButton)->active) {
    s416Handling = Options::S416ERR;
  } else {
    s416Handling = Options::S416REDOWN;
  }
  bool use_no_redirection = false;
  if(GTK_TOGGLE_BUTTON(use_no_redirection_toggle)->active) {
    use_no_redirection = true;
  }

  // Accept-Encoding request header
  bool httpAcceptCompression = false;
  if(GTK_TOGGLE_BUTTON(httpAcceptCompressionToggle)->active) {
    httpAcceptCompression = true;
  }

  // Accept-Language request header
  bool httpAcceptLangEnabled = false;
  if(GTK_TOGGLE_BUTTON(httpAcceptLangEnabledToggle)->active) {
    httpAcceptLangEnabled = true;
  }
  string httpAcceptLangString = Remove_white(gtk_entry_get_text(GTK_ENTRY(httpAcceptLangStringEntry)));

  // how many parts for a file to be divided  
  unsigned int divide = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_divide));

  // Rollback in bytes
  int rollback_bytes = stoi(gtk_entry_get_text(GTK_ENTRY(rollback_bytes_entry)));

  // Limit file size
  bool use_size_lower_limit;
  if(GTK_TOGGLE_BUTTON(use_size_lower_limit_toggle)->active) {
    use_size_lower_limit = true;
  } else {
    use_size_lower_limit = false;
  }
  int size_lower_limit = stoi(gtk_entry_get_text(GTK_ENTRY(size_lower_limit_entry)));
  bool use_no_redownload = false;
  if(GTK_TOGGLE_BUTTON(use_no_redownload_toggle)->active) {
    use_no_redownload = true;
  }

  bool use_no_download_samename = false;
  if(GTK_TOGGLE_BUTTON(use_no_download_samename_toggle)->active) {
    use_no_download_samename = true;
  }

  // whether delete item when downloading finished
  if(GTK_TOGGLE_BUTTON(toggle_delete_when_finish)->active) {
    delete_when_finish = true;
  } else {
    delete_when_finish = false;
  }

  // whether delete item without crc
  if(GTK_TOGGLE_BUTTON(toggle_dont_delete_without_crc)->active) {
    dont_delete_without_crc = true;
  } else {
    dont_delete_without_crc = false;
  }

  // no crc check
  bool no_crc_check;
  if(GTK_TOGGLE_BUTTON(no_crc_check_toggle)->active) {
    no_crc_check = true;
  } else {
    no_crc_check = false;
  }

  // ignore crc error
  bool ignore_crc_error;
  if(GTK_TOGGLE_BUTTON(ignore_crc_error_toggle)->active) {
    ignore_crc_error = true;
  } else {
    ignore_crc_error = false;
  }
  
  // use Content-MD5
  bool use_content_md5;
  if(GTK_TOGGLE_BUTTON(use_content_md5_toggle)->active) {
    use_content_md5 = true;
  } else {
    use_content_md5 = false;
  }
  // FTP mode
  Options::FTP_Mode ftp_mode;
  if(GTK_TOGGLE_BUTTON(active_button)->active) {
    ftp_mode = Options::FTP_ACTIVE_MODE;
  } else {
    ftp_mode = Options::FTP_PASSIVE_MODE;
  }
  Options::FTPretModeType ftp_ret_mode;
  if(GTK_TOGGLE_BUTTON(binary_button)->active) {
    ftp_ret_mode = Options::FTP_BINARY;
  } else {
    ftp_ret_mode = Options::FTP_ASCII;
  }

  bool ftp_nosend_quit = false;
  if(GTK_TOGGLE_BUTTON(ftp_nosend_quit_toggle)->active) {
    ftp_nosend_quit = true;
  }
  bool ftpNoCwd = false;
  if(GTK_TOGGLE_BUTTON(ftpNoCwdToggle)->active) {
    ftpNoCwd = true;
  }
  int ftp_recurse_count = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(ftp_recurse_depth_spin));
  // extensions to download in recursive download
  list<string> ftp_filter_target_list, ftp_activated_filter_target_list;
  get_extension_target_clist(ftp_filter_clist, ftp_filter_target_list, ftp_activated_filter_target_list);

  // various options for recursive download

  bool ftp_no_ascend = false;
  if(GTK_TOGGLE_BUTTON(ftp_no_ascend_toggle)->active) {
    ftp_no_ascend = true;
  }
  bool ftp_get_symlink_as_realfile = false;
  if(GTK_TOGGLE_BUTTON(ftp_get_symlink_as_realfile_toggle)->active) {
    ftp_get_symlink_as_realfile = true;
  }
  bool ftp_use_filter = false;
  if(GTK_TOGGLE_BUTTON(ftp_use_filter_toggle)->active) {
    ftp_use_filter = true;
  }
  bool ftp_allow_crawl_subdir = true;

  bool ftpRecurseAddPaste = false;
  if(GTK_TOGGLE_BUTTON(ftpRecurseAddPasteToggle)->active) {
    ftpRecurseAddPaste = true;
  }

  // update new option values
  if((!g_listManager->Search(listentry) || !listentry->getItemManager()->search_item(itemcell)) &&
     !g_itemManagerPaste->search_item(itemcell) &&
     g_consoleItem != itemcell &&
     (listentry == NULL || listentry->ret_Default_item() != itemcell)) {

    return OPTION_SUCCESS;
  }

  // acquire lock
  itemcell->get_Options_Lock();

  GtkWidget *itemlist = 0;
  if(g_itemManagerPaste->search_item(itemcell)) {
    // called from PasteWindow
    itemlist = g_pasteWindow->getPasteList();
  } else if(listentry != NULL) {
    itemlist = listentry->ret_Dl_clist();
  }

  if(itemcell != g_consoleItem &&
     itemcell != listentry->ret_Default_item() &&
     !itemcell->Is_Partial() &&
     !multipleSelectionFlag) {
    string url = Remove_white(gtk_entry_get_text(GTK_ENTRY(url_entry)));
    URLcontainer urlcon;
    if(urlcon.Parse_URL(url)) {
     itemcell->set_URL_Container_opt(urlcon);
     itemcell->set_URL_opt(urlcon.ret_URL());//modified 2001/3/14
     // if url is username:passwd@host notation, then extract username and
     // password. override previous user class variable
     if(urlcon.ret_Username().size()) {
       use_authentication = true;
       user.set_userpasswd(urlcon.ret_Username(), urlcon.ret_Password());
     }
    } else {// invalid URL format
      // release Options lock
      itemcell->release_Options_Lock();
      return OPTION_INVALID_URL;
    }
    string md5String = Remove_white(gtk_entry_get_text(GTK_ENTRY(md5_entry)));
    if(md5String.size() && md5String.size() != 32) {
      // release Options lock
      itemcell->release_Options_Lock();
      return OPTION_INVALID_MD5;
    } else {
      int rowindex = gtk_clist_find_row_from_data(GTK_CLIST(itemlist), itemcell);
      itemcell->set_md5string(md5String);
      listentry->Set_clist_column__md5(rowindex, md5String);

    }

    // acquire CRC lock
    itemcell->get_CRC_Lock();
    string crc_string = Remove_white(gtk_entry_get_text(GTK_ENTRY(crc_entry)));
    unsigned int crc = 0;
    if(crc_string.empty()) {
      itemcell->set_CRC_Type(ItemCell::CRC_NONE);
      // fixed
      if(itemlist == listentry->ret_Dl_clist()) {
	int rowindex = gtk_clist_find_row_from_data(GTK_CLIST(itemlist), itemcell);
	listentry->Set_clist_column__crc(rowindex, crc_string);
      }
    } else if((crc_string.size() != 4 && crc_string.size() != 8) ||
	      (crc = stoui(crc_string, 16)) == ULONG_MAX && errno == ERANGE) {
      // release lock
      itemcell->release_CRC_Lock();
      itemcell->release_Options_Lock();
      //pthread_mutex_unlock(&itemlistlock); //mod 2001/4/11
      return(OPTION_INVALID_CRC);
    } else {
      itemcell->set_CRC(crc);
      if(crc_string.size() == 4) { // 16bit CRC
	itemcell->set_CRC_Type(ItemCell::CRC_16);
      } else { // 32bit CRC
	itemcell->set_CRC_Type(ItemCell::CRC_32);
      }
      // fixed
      if(itemlist == listentry->ret_Dl_clist()) {
	int rowindex = gtk_clist_find_row_from_data(GTK_CLIST(itemlist), itemcell);
	listentry->Set_clist_column__crc(rowindex, crc_string);
      }
    }
    // release CRC lock
    itemcell->release_CRC_Lock();
    string filename = Remove_white(gtk_entry_get_text(GTK_ENTRY(filename_entry)));
    if(urlcon.ret_Filename().size() && filename.empty()) {
      itemcell->release_Options_Lock();
      //filename = itemcell->ret_URL_Container().ret_Filename();
      return OPTION_INVALID_FILENAME;
    }
    itemcell->set_Filename_opt(filename);
  }

  itemcell->release_Options_Lock();

  // set option values
  // ugly long long constructor arguments, fix this. :(
  g_httpProxyList->set(*sg_httpProxyListTemp);
  g_ftpProxyList->set(*sg_ftpProxyListTemp);
  Options options_opt(use_authentication,
		      user,
		      storedir,
		      http_version,
		      prewrittenHTMLType,
		      prewrittenHTMLName,
		      sync_with_url,
		      referer_type,
		      referer,
		      useragent,
		      random_useragent,
		      use_http_proxy,
		      use_http_cache,
		      use_http_proxy_authentication,
		      http_proxy_user,
		      http_proxy,
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
		      recurseAddPaste,
		      recurse_use_tag_href,
		      recurse_use_tag_src,
		      recurse_use_tag_background,
		      recurse_use_tag_code,
		      use_down_filter,
		      filter_down_target_list,
		      filter_nodown_target_list,
		      parse_target_list,
		      ign_domain_list,
		      ftp_mode,
		      ftp_ret_mode,
		      use_ftp_proxy,
		      use_ftp_proxy_authentication,
		      ftp_proxy_user,
		      ftp_proxy,
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
		      ftpRecurseAddPaste,
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
		      cookie_delete_on_restart,
		      cookie_nosend,
		      cookieUserDefined,
		      cookieUserDefinedString,
		      downm_type,
		      speed_value,
		      use_command,
		      use_exit_status,
		      command);

  options_opt.activate_filter_down_target_list(activated_filter_down_target_list);
  options_opt.activate_filter_nodown_target_list(activated_filter_nodown_target_list);
  options_opt.activate_parse_target_list(activated_parse_target_list);
  options_opt.activate_ign_domain_list(activated_ign_domain_list);
  options_opt.set_FTP_activated_filter_target_list(ftp_activated_filter_target_list);

  GList *node = NULL;
  if(g_itemManagerPaste->search_item(itemcell)) {
    node = GTK_CLIST(itemlist)->selection;
  } else if(itemcell == g_consoleItem ||
	    (listentry != NULL && itemcell == listentry->ret_Default_item())) {
    itemcell->get_Options_Lock();
    itemcell->set_Options_opt(options_opt);
    itemcell->release_Options_Lock();
    return OPTION_SUCCESS;
  } else {
    node = GTK_CLIST(itemlist)->selection;
  }

  while(node) {
    ItemCell *itemcell = (ItemCell *)gtk_clist_get_row_data(GTK_CLIST(itemlist), GPOINTER_TO_INT(node->data));
    itemcell->get_Options_Lock();

    itemcell->set_Options_opt(options_opt);
    itemcell->Raise_option_update_flag();

    if(g_itemManagerPaste->search_item(itemcell)) {
      int rowindex = gtk_clist_find_row_from_data(GTK_CLIST(itemlist), itemcell);
      gtk_clist_set_text(GTK_CLIST(itemlist), rowindex, 0, itemcell->ret_URL_opt().c_str());
      gtk_clist_set_text(GTK_CLIST(itemlist), rowindex, 1, itemcell->ret_Options_opt().ret_Store_Dir().c_str());
    } else if(listentry != NULL && itemcell != g_consoleItem &&
	      itemcell != listentry->ret_Default_item()) {
      switch(itemcell->ret_Status()) {
      case ItemCell::ITEM_STOP:
      case ItemCell::ITEM_READY:
      case ItemCell::ITEM_ERROR:
      case ItemCell::ITEM_COMPLETE:
	{
	itemcell->Apply_new_options();
	break;
	}
      case ItemCell::ITEM_DOWNLOAD:
      case ItemCell::ITEM_INUSE:
      case ItemCell::ITEM_INUSE_AGAIN:
      case ItemCell::ITEM_INUSE_CONCAT:
      case ItemCell::ITEM_DOWNLOAD_AGAIN:
      case ItemCell::ITEM_DOWNLOAD_INTERNAL_AGAIN: {
	ItemCommand itemcommand;
	itemcommand.commandtype = ItemCommand::COMMAND_CHANGE_SPEED;
	itemcommand.eventtype = ItemCommand::EV_USERINTER;
	itemcommand.value = speed_value;
	write(itemcell->ret_Desc_w(), &itemcommand, sizeof(ItemCommand));
	break;
      }
      case ItemCell::ITEM_DOWNLOAD_PARTIAL:
	for(list<ItemCell*>::const_iterator item_ptr = itemcell->ret_Worker_list().begin(); item_ptr != itemcell->ret_Worker_list().end(); ++item_ptr) {
	  Download_change_speed_sub((ItemCell*)*item_ptr, speed_value);
	}
      default:
	break;
      }
      // fixed
      
      ItemStatusDynamic *itemstatus = new ItemStatusDynamic(itemcell, itemcell->ret_Status(), itemcell->ret_Count(), itemcell->ret_Size_Current(), itemcell->ret_Size_Total());
      Send_report(MSG_DOWNLOAD_STATUS, itemstatus, listentry);
    }
    // release lock
    itemcell->release_Options_Lock();
    node = g_list_next(node);
  }

  return OPTION_SUCCESS;
}

void ItemOption::set_Store_Dir(const char* dir)
{
  gtk_entry_set_text(GTK_ENTRY(storedir_entry), dir);
}

void ItemOption::hide() {
  visibleFlag = false;

  gtk_widget_hide(option_window);
}

bool ItemOption::isVisible() const {
  return visibleFlag;
}

void ItemOption::setSensitiveHTTPRecursive() {
  if(GTK_TOGGLE_BUTTON(recurse_hostname_dir_toggle)->active) {
    gtk_widget_set_sensitive(recurse_abs2rel_toggle, TRUE);
    if(GTK_TOGGLE_BUTTON(recurse_abs2rel_toggle)->active) {
      gtk_widget_set_sensitive(recurse_force_convert_toggle, TRUE);
    } else {
      gtk_widget_set_sensitive(recurse_force_convert_toggle, FALSE);
    }
  } else {
    gtk_widget_set_sensitive(recurse_abs2rel_toggle, FALSE);
    gtk_widget_set_sensitive(recurse_force_convert_toggle, FALSE);
  }
  /*
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
  */
}
