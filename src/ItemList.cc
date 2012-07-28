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

// $Id: ItemList.cc,v 1.72 2002/10/01 15:32:00 tujikawa Exp $

// implementaion of class ItemList

#include "ItemList.h"
#include "SumInfo.h"
#include "UseragentList.h"
//#define stoi(ARG, ARG2) strtol(ARG.c_str(), (char**)NULL, ARG2)
#define stoi(ARG) strtol(ARG.c_str(), (char **)NULL, 10)

extern void Add_new_item_to_downloadlist(ItemCell *itemcell, ListEntry *listentry);
extern void create_default_filter_nodown_target_list(list<string>& filter_target_list);
extern void create_default_parse_target_list(list<string>& parse_target_list);
extern bool Is_track_required();
extern ItemCell *g_consoleItem;
extern AppOption *g_appOption;
extern ListManager *g_listManager;
extern PasteWindow *g_pasteWindow;
extern int g_threadLimit;
extern SumInfo g_summaryInfo;

ItemList::ItemList()
{
  // default save files
  string home_dir = g_get_home_dir();
  string save_dir = home_dir+"/.aria/save/";
  
  file_default_item_settings = save_dir+"default.aria";
  file_default_item_list = save_dir+"list.aria";
  file_app_settings = save_dir+"app.aria";
  file_server_settings = save_dir+"server.aria";
  file_http_proxy_list = save_dir+"http_proxy.aria";
  file_ftp_proxy_list = save_dir+"ftp_proxy.aria";
  file_useragent_list = save_dir+"useragent.aria";
  file_command_list = save_dir+"command.aria";
  file_history = save_dir+"history.aria";
  file_gui_info = save_dir+"gui.aria";
}

list<ItemCell*> ItemList::Recursive_add_http_item(ItemCell *itemcell, ListEntry *listentry)
{
  const URLcontainer& p_urlcon = itemcell->ret_URL_Container();
  const Options& p_options = itemcell->ret_Options();
  const string& documentroot_dir = itemcell->ret_documentroot_dir();
  const string& root_url = itemcell->ret_root_url();

  int count = 0;

  list<ItemCell*> item_list;
  //cerr << documentroot_dir << endl;
  int n_rec = p_options.ret_recurse_count()-1;
  //const string filename = p_options.ret_Store_Dir()+p_urlcon.ret_Filename();
  const string filename = p_options.ret_Store_Dir()+itemcell->ret_Filename();
  const string base_url = p_urlcon.ret_Protocol()+"//"+p_urlcon.ret_Hostname()+p_urlcon.ret_Dir();
  try {
    HTMLparse htmlparse(base_url,
			root_url,
			documentroot_dir,
			p_options,
			filename);
    if(htmlparse.in_bad()) {
      return item_list;
    }
    
    while(1) {
      Options options = p_options;
      URLcontainer urlcon = htmlparse.get_href(options);
      if(urlcon.bad() || p_options.ret_recurse_count() <= 1) continue;//fixed 2001/3/14

      string url = urlcon.ret_URL();
      if(!listentry->getItemManager()->search_by_url(url)) {
	options.set_recurse_count(n_rec);
	if(p_options.ret_Referer_override()) {// referer override
	  options.set_Referer_Type(Options::REFERER_USER_DEFINED);
	  options.set_Referer(p_urlcon.ret_URL());
	}
	ItemCell *itemcell;
	itemcell = new ItemCell(url, urlcon, options, _("Created"));
	if(urlcon.ret_Protocol() == "http:"
#ifdef HAVE_OPENSSL
	   || urlcon.ret_Protocol() == "https:"
#endif // HAVE_OPENSSL
	   ) {
	  //itemcell = new ItemCell_HTTP(url, urlcon, options, _("Created"));
	} else if(urlcon.ret_Protocol() == "ftp:") {
	  options.set_use_authentication(false);// added 2001/3/14
	  //itemcell = new ItemCell_FTP(url, urlcon, options, _("Created"));
	} else {
	  continue;
	}
	if(urlcon.ret_Protocol() == "http:"
#ifdef HAVE_OPENSSL
	   || urlcon.ret_Protocol() == "https:"
#endif // HAVE_OPENSSL
	   ) {
	  itemcell->set_documentroot_dir(documentroot_dir);
	  itemcell->set_root_url(root_url);
	}
	// modified 2001/5/26
	// added 2001/5/20
	//if(g_appOption->Whether_use_automatic_start()) {
	  itemcell->set_Status(ItemCell::ITEM_READY);
	  //}
	++count;
	//Add_new_item_to_downloadlist(itemcell);
	item_list.push_back(itemcell);
      }
    }
    throw HTMLPARSE_EOF;
  } catch (HTMLparseExceptionType err) {
    switch(err) {
    case HTMLPARSE_EOF:
      {
	/*
	if(count) {
	  g_consoleItem->Send_message_to_gui(itos(count)+_("item(s) added"), MSG_SYS_INFO);
	}
	*/
	break;
      }
    default:
      {
	/*
	g_consoleItem->Send_message_to_gui(_("parse error in \"")+filename+_("\". Abort further parsing operation"), MSG_SYS_ERROR);
	retval = false;
	*/
      }
    }
    if(p_options.ret_use_down_filter() &&
       !p_options.Is_in_activated_filter_down_target_list(p_urlcon.ret_Filename())) {
      unlink(filename.c_str());
      //itemcell->get_Options_Lock();
      //itemcell->ret_Options_opt().set_Delete_When_Finish(true);
      //itemcell->ret_Options_opt().set_Dont_Delete_Without_CRC(false);
      //itemcell->release_Options_Lock();
    }
  }
  return item_list;
}

bool ItemList::Read_md5_from_file(ListEntry *listentry, const string& filename)
{
  ifstream infile(filename.c_str(), ios::in);//ios::skipws|ios::in);
  if(infile.bad() || infile.fail()) {
    return false;
  }
  list<CRCList*> md5List;
  while(1) {
    string filename_entry;
    string md5String;
    string line;

    getline(infile, line, '\n');
    md5String = Token_splitter(line, " ");
    filename_entry = Token_splitter(line, " \n\r");

    if(filename_entry.size() && md5String.size() == 32) {
      CRCList *md5Entry = new CRCList(filename_entry, md5String);
      md5List.push_back(md5Entry);
    }
    if(infile.eof()) break;
  }

  listentry->setMD5List(md5List);
  /*gtk_clist_freeze(GTK_CLIST(listentry->ret_Dl_clist()));
  for(int rowindex = 0; rowindex < GTK_CLIST(listentry->ret_Dl_clist())->rows; ++rowindex) {
    ItemCell *itemcell = (ItemCell *)gtk_clist_get_row_data(GTK_CLIST(listentry->ret_Dl_clist()), rowindex);
    for(list<CRCList*>::iterator md5List_itr = md5List.begin(); md5List_itr != md5List.end(); ++md5List_itr) {
      if(itemcell->ret_URL_Container().ret_Filename() == (*md5List_itr)->ref_Filename()) {
	itemcell->set_md5string((*md5List_itr)->ref_CRC_string());
	listentry->Set_clist_column__md5(rowindex, (*md5List_itr)->ref_CRC_string());
	// set md5 to its dedicated column
	md5List.remove(*md5List_itr);
	delete *md5List_itr;
	break;
      }
    }
  }
  */
  for(list<CRCList*>::iterator md5List_itr = md5List.begin(); md5List_itr != md5List.end(); ++md5List_itr) {
    delete *md5List_itr;
  }
  //gtk_clist_thaw(GTK_CLIST(listentry->ret_Dl_clist()));

  return true;
}

bool ItemList::Read_CRC_from_file(ListEntry *listentry, const string& filename)
{
  ifstream infile(filename.c_str(), ios::in);//ios::skipws|ios::in);
  if(infile.bad() || infile.fail()) {
    return false;
  }
  list<CRCList*> crc_list;
  while(1) {
    string filename_entry;
    string crc_string;
    string size_string;
    string line;
    getline(infile, line, '\n');
    filename_entry = Token_splitter(line, " ");
    crc_string = Token_splitter(line, " ");
    size_string = Token_splitter(line, " ");

    if(filename_entry.size() && crc_string.size()) {
      CRCList *crc_entry = new CRCList(filename_entry, crc_string);
      crc_list.push_back(crc_entry);
    }
    if(infile.eof()) break;
  }

  listentry->setCRCList(crc_list);

  /*gtk_clist_freeze(GTK_CLIST(listentry->ret_Dl_clist()));
  for(int rowindex = 0; rowindex < GTK_CLIST(listentry->ret_Dl_clist())->rows; ++rowindex) {
    ItemCell *itemcell = (ItemCell *)gtk_clist_get_row_data(GTK_CLIST(listentry->ret_Dl_clist()), rowindex);
    for(list<CRCList*>::iterator crc_list_itr = crc_list.begin(); crc_list_itr != crc_list.end(); ++crc_list_itr) {
      if(itemcell->ret_URL_Container().ret_Filename() == (*crc_list_itr)->ref_Filename()) {
	switch((*crc_list_itr)->ref_CRC_string().size()) {
	case 8:
	  itemcell->set_CRC(stoui((*crc_list_itr)->ref_CRC_string(), 16));
	  itemcell->set_CRC_Type(ItemCell::CRC_32);
	  listentry->Set_clist_column__crc(rowindex, (*crc_list_itr)->ref_CRC_string());
	  crc_list.remove(*crc_list_itr);
	  delete *crc_list_itr;
	  break;
	case 4:
	  itemcell->set_CRC(stoui((*crc_list_itr)->ref_CRC_string(), 16));
	  itemcell->set_CRC_Type(ItemCell::CRC_16);

	  listentry->Set_clist_column__crc(rowindex, (*crc_list_itr)->ref_CRC_string());
	  crc_list.remove(*crc_list_itr);
	  delete *crc_list_itr;
	  break;
	default:
	  cerr << "Warning: unsupported CRC format" << endl;
	}
	break;
      }
    }
    }*/

  for(list<CRCList*>::iterator crc_list_itr = crc_list.begin(); crc_list_itr != crc_list.end(); ++crc_list_itr) {
    delete *crc_list_itr;
  }

  //gtk_clist_thaw(GTK_CLIST(listentry->ret_Dl_clist()));

  return true;
}

bool ItemList::Find_Hyperlink_from_file(const string& filename, const string& base_url, int mode)
{
  ifstream infile(filename.c_str(), ios::in);//ios::skipws|ios::in);
  if(infile.bad() || infile.fail()) {
    cerr << "ERROR:Unable to read from file " << filename << "\n";
    return false;
  }
  int count = 0;

  ListEntry *listentry = g_listManager->ret_Current_listentry();//fix this

  //gtk_clist_freeze(GTK_CLIST(listentry->ret_Dl_clist()));
  listentry->freezeDlCList();
  while(1) {
    string urlbuf;
    getline(infile, urlbuf);
    while(!urlbuf.empty()) {
      string url;
      //if(mode == ItemList::STRICT_HREF) {
      //url = URLcontainer::Find_HREF_strict(urlbuf, base_url);
      //} else {
      url = URLcontainer::Find_HREF(urlbuf, base_url);
      //}
      // parse URL
      URLcontainer urlcon;
      if(urlcon.Parse_URL(url) && !listentry->getItemManager()->search_by_url(urlcon.ret_URL()) &&
	 (!g_appOption->ret_use_ignore_extension_list() ||
	  !g_appOption->Is_in_ignore_extension_list(urlcon.ret_File()))) {
	Options options;
	
	options = listentry->ret_Options();
	ItemCell *itemcell = new ItemCell(url, urlcon, options, _("Created"));
	// added 2001/5/20
	if(g_appOption->Whether_use_automatic_start()) {
	  itemcell->set_Status(ItemCell::ITEM_READY);
	}
	/*
	if(urlcon.ret_Protocol() == "http:") {
	  itemcell = new ItemCell_HTTP(url, urlcon, options, _("Created"));
	} else if(urlcon.ret_Protocol() == "ftp:") {
	  itemcell = new ItemCell_FTP(url, urlcon, options, _("Created"));
	} else {
	  continue;
	}
	*/
	++count;
	if(mode == ItemList::FINDHREF_PASTE) {
	  g_pasteWindow->addItem(itemcell);
	} else if(mode == ItemList::FINDHREF_ADD || mode == ItemList::STRICT_HREF) {
	  //gdk_threads_enter();
	  //listentry->get_Dl_clist_lock();
	  //pthread_mutex_lock(&itemlistlock);
	  Add_new_item_to_downloadlist(itemcell, listentry);
	  //pthread_mutex_unlock(&itemlistlock);
	  //listentry->release_Dl_clist_lock();
	  //gdk_threads_leave();
	} else {
	  delete itemcell;
	}
      }
    }
    if(infile.eof()) break;
  }
  //gtk_clist_thaw(GTK_CLIST(listentry->ret_Dl_clist()));
  listentry->thawDlCList();

  if(count && mode == ItemList::FINDHREF_PASTE) {
    g_pasteWindow->show();
  }
  return true;
}

bool ItemList::Read_URL_from_file(ListEntry *listentry, const string& filename)
{
  ifstream infile;

  // if filename is "-", get URLs from stdin
  if(filename == "-") {
    cerr << "currently '-f -' is not functional. sorry\n";
    return false;
  } else {
    infile.open(filename.c_str(), ios::in);//ios::skipws|ios::in);
    if(infile.bad() || infile.fail()) {
      cerr << "ERROR:Unable to read from file " << filename << "\n";
      return false;
    }
  }

  // get current listentry
  //ListEntry *listentry = g_listManager->ret_Current_listentry();
  if(listentry == NULL) {
    listentry = new ListEntry(g_appOption->getThreadMax(),
			      g_consoleItem->ret_Options_opt());
  }

  //gtk_clist_freeze(GTK_CLIST(listentry->ret_Dl_clist()));
  listentry->freezeDlCList();
  while(1) {
    string urlbuf;
    getline(infile, urlbuf, '\n'); //mod
    //infile >> urlbuf;//mod
    //while(!urlbuf.empty()) {
    string url = urlbuf;//URLcontainer::Find_URL(urlbuf); //mod
    //string url = urlbuf;//mod
      // parse URL
      URLcontainer urlcon;
      if(urlcon.Parse_URL(url) && !listentry->getItemManager()->search_by_url(urlcon.ret_URL())) {
	const Options& options = listentry->ret_Options();

	ItemCell *itemcell = new ItemCell(url, urlcon, options, _("Created"));
	// added 2001/5/20
	if(g_appOption->Whether_use_automatic_start()) {
	  itemcell->set_Status(ItemCell::ITEM_READY);
	}
	/*
	if(urlcon.ret_Protocol() == "http:") {
	  itemcell = new ItemCell_HTTP(url, urlcon, options, _("Created"));
	} else if(urlcon.ret_Protocol() == "ftp:") {
	  itemcell = new ItemCell_FTP(url, urlcon, options, _("Created"));
	} else {
	  continue;
	}
	*/
	char *clist_item[TOTALCOL];
	/*
	//modified start
	for(int i = 0; i < TOTALCOL; ++i) {
	  clist_item[i] = NULL;
	}

	int rowindex = gtk_clist_append(GTK_CLIST(itemlistwidget), clist_item);
	gtk_clist_set_row_data(GTK_CLIST(itemlistwidget), rowindex, itemcell);
	item_manager->regist_item_back(itemcell);
	//modified end
	ItemStatusDynamic itemstatus(itemcell,
				     itemcell->ret_Status(),
				     0,
				     0);
	itemstatus.Update();
*/
	clist_item[COL_ICON] = NULL;
	if(itemcell->ret_Filename().empty()) {
	  clist_item[COL_FILENAME] = g_strdup(_("<directory>"));
	} else {
	  clist_item[COL_FILENAME] = g_strdup(itemcell->ret_Filename().c_str());
	}
	clist_item[COL_CURSIZE] = "0";
	clist_item[COL_TOTSIZE] = _("unknown");
	clist_item[COL_PROGRESS] = NULL;
	clist_item[COL_SPEED] = "";
	clist_item[COL_RTIME] = "";
	if(itemcell->ret_Options().ret_Retry() == -1) {
	  // modified 2001/5/20
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
	//clist_item[COL_STATUS] = _("READY");
	if(g_appOption->Whether_use_automatic_start()) {
	  clist_item[COL_STATUS] = _("READY");
	} else {
	  clist_item[COL_STATUS] = _("STOPPED");
	}
	clist_item[COL_CRC] = "";
	clist_item[COL_MD5] = "";
	clist_item[COL_SAVE] = g_strdup(itemcell->ret_Options().ret_Store_Dir().c_str());
	clist_item[COL_URL] = g_strdup(itemcell->ret_URL().c_str());
	int rowindex = listentry->Append_dl_item(clist_item, itemcell);
	/*listentry->Set_clist_column__progress(rowindex, 0);
	if(g_appOption->Whether_use_automatic_start()) {
	  listentry->Set_clist_column__icon(rowindex, ItemCell::ITEM_READY);
	} else {
	  listentry->Set_clist_column__icon(rowindex, ItemCell::ITEM_STOP);
	}*/

	g_free(clist_item[COL_FILENAME]);
	g_free(clist_item[COL_RETRY]);
	g_free(clist_item[COL_REC]);
	g_free(clist_item[COL_SAVE]);
	g_free(clist_item[COL_URL]);
      }
      //}
    if(infile.eof()) break;
  }
  //gtk_clist_thaw(GTK_CLIST(listentry->ret_Dl_clist()));
  listentry->thawDlCList();
  if(g_appOption->Whether_use_automatic_start()) {
    listentry->Send_start_signal();
  }
  return true;
}

bool ItemList::Save_current_list()
{
  return Save_current_list(file_default_item_list);
}

bool ItemList::Save_default_item_option_sub(ofstream& outfile, const Options& options)
{

  // save default item options
  outfile << "%Begin_default" << '\n';
  outfile << FI_USE_AUTHENTICATION << " " << options.Whether_use_authentication() << '\n';
  if(!options.ret_User().empty()) {
    outfile << FI_USER << " " << options.ret_User() << '\n';
  }
  if(!options.ret_Password().empty()) {
    outfile << FI_PASSWORD << " " << options.ret_Password() << '\n';
  }
  outfile << FI_STORE_DIRECTORY << " " << options.ret_Store_Dir() << '\n';

  // Command
  outfile << FI_USE_COMMAND << " " << dec << (unsigned int)options.ret_use_Command() << '\n';
  outfile << FI_USE_EXIT_STATUS << " " << dec << (unsigned int)options.ret_use_Exit_status() << '\n';
  outfile << FI_COMMAND_STRING << " ";

  string command_string = "";
  for(list<string>::const_iterator itr = options.ret_Command().ret_command().begin(); itr != options.ret_Command().ret_command().end(); ++itr) {
    command_string += *itr;
  }

  for(unsigned int i = 0; i < command_string.size(); i++) {
    if(command_string.at(i) == '\n') {
      command_string.replace(i, 1, "\r");
    }
  }
  outfile << command_string;
  outfile << '\n';

  outfile << FI_EXIT_STATUS_STRING << " ";
  for(list<int>::const_iterator itr = options.ret_Command().ret_succ_status_list().begin(); itr != options.ret_Command().ret_succ_status_list().end(); ++itr) {
    outfile << itos(*itr) << ',';
  }
  outfile << '\n';

  outfile << FI_HTTP_VERSION << " " << options.ret_HTTP_version() << '\n';
  outfile << FI_PREWRITTEN_HTML_TYPE << " " << options.getPrewrittenHTMLType() << '\n';
  outfile << FI_PREWRITTEN_HTML_NAME << " " << options.getPrewrittenHTMLName() << '\n';
  outfile << FI_SYNC_WITH_URL << ' ' << options.ret_sync_with_URL() << '\n';
  outfile << FI_REFERER_TYPE << " " << dec << (unsigned int)options.ret_Referer_Type() << '\n';
  if(!options.ret_Referer().empty()) {
    outfile << FI_REFERER_STRING << " " << options.ret_Referer() << '\n';
  }
  // Useragent
  if(!options.ret_Useragent().empty()) {
    outfile << FI_USERAGENT << " " << options.ret_Useragent() << '\n';
  }
  outfile << FI_RANDOM_USERAGENT << " " << options.ret_Random_useragent() << '\n';
  // Cookie
  outfile << FI_COOKIE_DELETE_ON_RESTART << " " << options.ret_Cookie_delete_on_restart() << '\n';
  outfile << FI_COOKIE_NOSEND << " " << options.ret_Cookie_nosend() << '\n';
  outfile << FI_COOKIE_USERDEFINED << " " << options.getCookieUserDefined() << '\n';
  outfile << FI_COOKIE_USERDEFINED_STRING << " " << options.getCookieUserDefinedString() << '\n';
  // HTTP Proxy
  outfile << FI_USE_HTTP_PROXY_AUTHENTICATION << " " << options.ret_use_http_proxy_authentication() << '\n';
  if(!options.ret_http_proxy_User().empty()) {
    outfile << FI_HTTP_PROXY_USER << " " << options.ret_http_proxy_User() << '\n';
  }
  if(!options.ret_http_proxy_Password().empty()) {
    outfile << FI_HTTP_PROXY_PASSWORD << " " << options.ret_http_proxy_Password() << '\n';
  }
  outfile << FI_USE_HTTP_PROXY << " " << dec << options.ret_use_http_proxy() << '\n';
  outfile << FI_USE_HTTP_CACHE << " " << dec << options.ret_use_http_cache() << '\n';
  if(!options.ret_http_proxy().ret_Server().empty()) {
    outfile << FI_HTTP_PROXY_SERVER << " " << options.ret_http_proxy().ret_Server() << '\n';
    outfile << FI_HTTP_PROXY_PORT << " " << dec << options.ret_http_proxy().ret_Port() << '\n';
  }
  // FTP Proxy
  outfile << FI_USE_FTP_PROXY_AUTHENTICATION << " " << options.ret_use_ftp_proxy_authentication() << '\n';
  if(!options.ret_ftp_proxy_User().empty()) {
    outfile << FI_FTP_PROXY_USER << " " << options.ret_ftp_proxy_User() << '\n';
  }
  if(!options.ret_ftp_proxy_Password().empty()) {
    outfile << FI_FTP_PROXY_PASSWORD << " " << options.ret_ftp_proxy_Password() << '\n';
  }
  outfile << FI_USE_FTP_PROXY << " " << dec << options.ret_use_ftp_proxy() << '\n';
  if(!options.ret_ftp_proxy().ret_Server().empty()) {
    outfile << FI_FTP_PROXY_SERVER << " " << options.ret_ftp_proxy().ret_Server() << '\n';
    outfile << FI_FTP_PROXY_PORT << " " << dec << options.ret_ftp_proxy().ret_Port() << '\n';
  }
  outfile << FI_USE_FTP_CACHE << " " << dec << options.ret_use_ftp_cache() << '\n';
  outfile << FI_USE_FTP_PROXY_VIA_HTTP << " " << dec << options.ret_use_ftp_proxy_via_http() << '\n';
  outfile << FI_FTP_PROXY_LOGIN_PROC << " " << dec << options.ret_ftp_proxy_login_proc() << '\n';
  // download method
  outfile << FI_DOWNM_TYPE << " " << dec << options.ret_downm_type() << '\n';
  // speed limit
  outfile << FI_SPEED_LIMIT << " " << dec << options.ret_speed_limit() << '\n';
  // Time out
  outfile << FI_TIMED_OUT << " " << dec << options.ret_Timed_Out() << '\n';

  // Divide
  outfile << FI_HOW_MANY_PARTS << " " << dec << options.ret_Divide() << '\n';
  // Rollback in bytes
  outfile << FI_ROLLBACK_BYTES << " " << dec << options.ret_Rollback_bytes() << '\n';
  // Limit file size
  outfile << FI_USE_SIZE_LOWER_LIMIT << ' ' << dec << (unsigned int)options.ret_use_size_lower_limit() << '\n';
  outfile << FI_SIZE_LOWER_LIMIT << ' ' << dec << options.ret_size_lower_limit() << '\n';
  // do not redownload files if resuming is not available
  outfile << FI_USE_NO_REDOWNLOAD << ' ' << dec << (unsigned int)options.ret_use_no_redownload() << '\n';
  outfile << FI_USE_NO_DOWNLOAD_SAMENAME << ' ' << dec << (unsigned int)options.ret_use_no_download_samename() << '\n';
  // FTP
  outfile << FI_FTP_MODE << " " << dec << (unsigned int)options.ret_FTP_Mode() << '\n';
  outfile << FI_FTP_RET_MODE << " " << dec << (unsigned int)options.ret_FTP_ret_mode() << '\n';
  outfile << FI_FTP_NOSEND_QUIT << " " << dec << (unsigned int)options.ret_FTP_nosend_quit() << '\n';
  outfile << FI_FTP_NO_CWD << " " << dec << (unsigned int)options.isFtpNoCwdEnabled() << '\n';
  outfile << FI_FTP_RECURSE_COUNT << " " << dec << (unsigned int)options.ret_FTP_recurse_count() << '\n';
  outfile << FI_FTP_NO_ASCEND << " " << dec << (unsigned int)options.ret_FTP_no_ascend() << '\n';
  outfile << FI_FTP_ALLOW_CRAWL_SUBDIR << " " << dec << (unsigned int)options.ret_FTP_allow_crawl_subdir() << '\n';
  outfile << FI_FTP_GET_SYMLINK_AS_REALFILE << " " << dec << (unsigned int)options.ret_FTP_get_symlink_as_realfile() << '\n';
  outfile << FI_FTP_RECURSE_ADD_PASTE << " " << dec << (unsigned int)options.ret_FTP_recurse_add_paste() << '\n';

  outfile << FI_FTP_USE_FILTER << " " << dec << (unsigned int)options.ret_FTP_use_filter() << '\n';
  outfile << FI_FTP_FILTER_TARGET_LIST;
  for(list<string>::const_iterator itr = options.ret_FTP_filter_target_list().begin(); itr != options.ret_FTP_filter_target_list().end(); ++itr) {
    outfile << "\t" << *itr;
  }
  outfile << '\n';
  outfile << FI_FTP_ACTIVATED_FILTER_TARGET_LIST;
  for(list<string>::const_iterator itr = options.ret_FTP_activated_filter_target_list().begin(); itr != options.ret_FTP_activated_filter_target_list().end(); ++itr) {
    outfile << "\t" << *itr;
  }
  outfile << '\n';

  outfile << FI_RETRY << " " << dec << options.ret_Retry() << '\n';
  outfile << FI_RETRY_REPEAT << " " << dec << options.ret_Retry_repeat() << '\n';
  outfile << FI_RETRY_INTERVAL << " " << dec << options.ret_Retry_interval() << '\n';
  outfile << FI_FORCE_RETRY_404 << " " << dec << options.ret_force_retry_404() << '\n';
  outfile << FI_FORCE_RETRY_503 << " " << dec << options.ret_force_retry_503() << '\n';
  outfile << FI_STATUS_416_HANDLING << " " << dec << options.ret_status_416_handling() << '\n';
  outfile << FI_USE_NO_REDIRECTION << ' ' << dec << options.ret_use_no_redirection() << '\n';
  outfile << FI_HTTP_ACCEPT_COMPRESSION << " " << dec << (unsigned int)options.ret_HTTP_accept_compression() << '\n';
  outfile << FI_HTTP_ACCEPT_LANG_ENABLED << " " << dec << options.ret_HTTP_accept_lang_enabled() << '\n';
  outfile << FI_HTTP_ACCEPT_LANG_STRING << " " << dec << options.ret_HTTP_accept_lang_string() << '\n';
  outfile << FI_DELETE_WHEN_COMPLETED << " " << dec << (unsigned int)options.ret_Delete_When_Finish() << '\n';
  outfile << FI_DONT_DELETE_WITHOUT_CRC << " " << dec << (unsigned int)options.ret_Dont_Delete_Without_CRC() << '\n';
  outfile << FI_NO_CRC_CHECKING << " " << dec << (unsigned int)options.ret_no_crc_checking() << '\n';
  outfile << FI_IGNORE_CRC_ERROR << " " << dec << (unsigned int)options.ret_ignore_crc_error() << '\n';
  outfile << FI_USE_CONTENT_MD5 << " " << dec << (unsigned int)options.ret_use_content_md5() << '\n';

  // recursive download
  outfile << FI_RECURSE_COUNT << " " << dec << options.ret_recurse_count() << '\n';
  //outfile << FI_DOCUMENTROOT_DIR << " " << itemcell->ret_documentroot_dir() << '\n';

  outfile << FI_WITH_HOSTNAME_DIR << " " << dec << (unsigned int)options.ret_with_hostname_dir() << '\n';
  outfile << FI_ABS2REL << " " << dec << (unsigned int)options.ret_abs2rel_url() << '\n';
  outfile << FI_FORCE_CONVERT << " " << dec << (unsigned int)options.ret_force_convert() << '\n';
  outfile << FI_DEL_COMMENT << " " << dec << (unsigned int)options.ret_delete_comment() << '\n';
  outfile << FI_DEL_JAVASCRIPT << " " << dec << (unsigned int)options.ret_delete_javascript() << '\n';
  outfile << FI_DEL_IFRAME << " " << dec << (unsigned int)options.ret_delete_iframe() << '\n';
  outfile << FI_NO_OTHER_HOST << " " << dec << (unsigned int)options.ret_no_other_host() << '\n';
  outfile << FI_NO_ASCEND << " " << dec << (unsigned int)options.ret_no_ascend() << '\n';
  outfile << FI_ONLY_RELATIVE_LINKS << " " << dec << (unsigned int)options.ret_only_relative_links() << '\n';
  outfile << FI_REFERER_OVERRIDE << " " << dec << (unsigned int)options.ret_Referer_override() << '\n';
  outfile << FI_FOLLOW_FTP_LINK << " " << dec << (unsigned int)options.ret_Follow_ftp_link() << '\n';
  outfile << FI_CONVERT_TILDE << " " << dec << (unsigned int)options.ret_convert_tilde() << '\n';
  outfile << FI_NO_REDOWNLOAD_HTTP_RECURSE << ' ' << dec << (unsigned int)options.ret_no_redownload_HTTP_recurse() << '\n';
  outfile << FI_HTTP_RECURSE_ADD_PASTE << " " << dec << (unsigned int)options.ret_HTTP_recurse_add_paste() << '\n';

  outfile << FI_USE_TAG_HREF << " " << dec << (unsigned int)options.ret_use_tag_href() << '\n';
  outfile << FI_USE_TAG_SRC << " " << dec << (unsigned int)options.ret_use_tag_src() << '\n';
  outfile << FI_USE_TAG_BACKGROUND << " " << dec << (unsigned int)options.ret_use_tag_background() << '\n';
  outfile << FI_USE_TAG_CODE << " " << dec << (unsigned int)options.ret_use_tag_code() << '\n';

  outfile << FI_IGN_DOMAIN_LIST;
  for(list<string>::const_iterator itr = options.ret_ign_domain_list().begin(); itr != options.ret_ign_domain_list().end(); ++itr) {
    outfile << " " << *itr;
  }
  outfile << '\n';
  outfile << FI_ACTIVATED_IGN_DOMAIN_LIST;
  for(list<string>::const_iterator itr = options.ret_activated_ign_domain_list().begin(); itr != options.ret_activated_ign_domain_list().end(); ++itr) {
    outfile << " " << *itr;
  }
  outfile << '\n';

  outfile << FI_USE_DOWN_FILTER << " " << (unsigned int)options.ret_use_down_filter() << '\n';

  outfile << FI_FILTER_DOWN_TARGET_LIST;
  for(list<string>::const_iterator itr = options.ret_filter_down_target_list().begin(); itr != options.ret_filter_down_target_list().end(); ++itr) {
    outfile << "\t" << *itr;
  }
  outfile << '\n';
  outfile << FI_ACTIVATED_FILTER_DOWN_TARGET_LIST;
  for(list<string>::const_iterator itr = options.ret_activated_filter_down_target_list().begin(); itr != options.ret_activated_filter_down_target_list().end(); ++itr) {
    outfile << "\t" << *itr;
  }
  outfile << '\n';

  outfile << FI_FILTER_NODOWN_TARGET_LIST;
  for(list<string>::const_iterator itr = options.ret_filter_nodown_target_list().begin(); itr != options.ret_filter_nodown_target_list().end(); ++itr) {
    outfile << " " << *itr;
  }
  outfile << '\n';
  outfile << FI_ACTIVATED_FILTER_NODOWN_TARGET_LIST;
  for(list<string>::const_iterator itr = options.ret_activated_filter_nodown_target_list().begin(); itr != options.ret_activated_filter_nodown_target_list().end(); ++itr) {
    outfile << " " << *itr;
  }
  outfile << '\n';
  outfile << FI_PARSE_TARGET_LIST;
  for(list<string>::const_iterator itr = options.ret_parse_target_list().begin(); itr != options.ret_parse_target_list().end(); ++itr) {
    outfile << " " << *itr;
  }
  outfile << '\n';
  outfile << FI_ACTIVATED_PARSE_TARGET_LIST;
  for(list<string>::const_iterator itr = options.ret_activated_parse_target_list().begin(); itr != options.ret_activated_parse_target_list().end(); ++itr) {
    outfile << " " << *itr;
  }
  outfile << '\n';

  outfile << "%End_default" << endl;

  if(outfile.bad() || outfile.fail()) return false;

  return true;
}

bool ItemList::Save_current_list(const string& filename)
{
  ofstream list_out;

  string filenameTemp = filename+".temporary.working";

  list_out.open(filenameTemp.c_str(), ios::out);//, 0600);

  if(list_out.bad() || list_out.fail()) {
    cerr << "ERROR:Unable to save download list" << endl;
    return false;
  }
  // file header
  list_out << ARIA_VERSION << '\n';

  //Save_default_item_option_sub(list_out);

  // save items
  //list_out << "%Begin_Items_Section" << '\n';

  for(list<ListEntry*>::const_iterator itr = g_listManager->ret_Listentry_list().begin();
      itr != g_listManager->ret_Listentry_list().end(); ++itr) {
    
    ListEntry *listentry = *itr;
    listentry->get_Dl_clist_lock();
    
    // fix this
    list_out << "%Begin_Tab" << '\n';
    if(g_listManager->ret_Current_listentry() == *itr) {
      list_out << FI_TAB_ACTIVE << ' ' << "Yes\n";
    }
    list_out << FI_TAB_NAME << ' ' << listentry->getName() << '\n';
    list_out << FI_TAB_THREAD_LIMIT << ' ' << listentry->getThreadLimit() << '\n';
    // save default item option for this tab
    if(!Save_default_item_option_sub(list_out, listentry->ret_Options())) {
      return false;
    }

    list_out << "%Columns-Size:";
    GList *column = gtk_tree_view_get_columns(GTK_TREE_VIEW(listentry->ret_Dl_clist()));
    for(int index = 0; index < TOTALCOL; ++index) {
      list_out << ' ' << gtk_tree_view_column_get_width(GTK_TREE_VIEW_COLUMN(column->data));
      column = g_list_next(column);
    }
    list_out << '\n';
	g_list_free(column);

    //const list<int>& download_list = listentry->getItemManager()->ret_id_list();
    //for(list<int>::const_iterator id_itr = download_list.begin(); id_itr != download_list.end(); ++id_itr) {
    for(size_t rowindex = 0; rowindex < listentry->getRowCount(); ++rowindex) {
      //ItemCell *itemcell = listentry->getItemManager()->ret_itemaddr(*id_itr);
      ItemCell *itemcell = listentry->getItemCellByRow(rowindex);
      if(itemcell->Is_Partial()) continue;
      const Options& options = itemcell->ret_Options_opt();
      list_out << "%Begin_Item" << '\n';
      //URL
      list_out << FI_URL << " " << itemcell->ret_URL_opt() << '\n';
      //Filename
      list_out << FI_FILENAME << " " << itemcell->ret_Filename_opt() << '\n';
      //status
      list_out << FI_STATUS << " ";
      switch(itemcell->ret_Status()) {
      case ItemCell::ITEM_DOWNLOAD:
      case ItemCell::ITEM_INUSE:
      case ItemCell::ITEM_DOWNLOAD_AGAIN:
      case ItemCell::ITEM_DOWNLOAD_INTERNAL_AGAIN:
      case ItemCell::ITEM_INUSE_AGAIN:
      case ItemCell::ITEM_DOWNLOAD_PARTIAL:
      case ItemCell::ITEM_READY_CONCAT:
      case ItemCell::ITEM_READY_AGAIN:
	//list_out << dec << (unsigned int)ItemCell::ITEM_READY;
	list_out << dec << (unsigned int)ItemCell::ITEM_STOP;
	break;
      default:
	list_out << dec << (unsigned int)itemcell->ret_Status();
	break;
      }
      list_out << '\n';

      //size
      list_out << FI_SIZE_CURRENT << " " << dec << itemcell->ret_Size_Current() << '\n';
      list_out << FI_SIZE_TOTAL << " " << dec << itemcell->ret_Size_Total() << '\n';
      //CRC
      switch(itemcell->ret_CRC_Type()) {
      case ItemCell::CRC_16:
	list_out << FI_CRC_TYPE << " " << (unsigned int)itemcell->ret_CRC_Type() << '\n';
	list_out << FI_CRC << " " << hex << setw(4) << setfill('0') << itemcell->ret_CRC() << '\n';
	break;
      case ItemCell::CRC_32:
	list_out << FI_CRC_TYPE << " " << (unsigned int)itemcell->ret_CRC_Type() << '\n';
	list_out << FI_CRC << " " << hex << setw(8) << setfill('0') << itemcell->ret_CRC() << '\n';
	break;
      default:
	list_out << FI_CRC_TYPE << " " << dec << (unsigned int)itemcell->ret_CRC_Type() << '\n';
	break;
      }

      // MD5
      list_out << FI_MD5 << " " << itemcell->ret_md5string() << '\n';
      // previously download size
      list_out << FI_PREVDLSIZE << " " << itemcell->ret_previous_dl_size() << '\n';
      // Command
      list_out << FI_USE_COMMAND << " " << dec << (unsigned int)options.ret_use_Command() << '\n';
      list_out << FI_USE_EXIT_STATUS << " " << dec << (unsigned int)options.ret_use_Exit_status() << '\n';
      list_out << FI_COMMAND_STRING << " ";

      string command_string = "";
      for(list<string>::const_iterator itr = options.ret_Command().ret_command().begin(); itr != options.ret_Command().ret_command().end(); ++itr) {
	command_string += *itr;
      }
      
      for(unsigned int i = 0; i < command_string.size(); i++) {
	if(command_string.at(i) == '\n') {
	  command_string.replace(i, 1, "\r");
	}
      }
      list_out << command_string;

      list_out << '\n';

      list_out << FI_EXIT_STATUS_STRING << " ";
      for(list<int>::const_iterator itr = options.ret_Command().ret_succ_status_list().begin(); itr != options.ret_Command().ret_succ_status_list().end(); ++itr) {
	list_out << itos(*itr) << ',';
      }
      list_out << '\n';

      //authentication
      list_out << FI_USE_AUTHENTICATION << " " << options.Whether_use_authentication() << '\n';
      if(!options.ret_User().empty()) {
	list_out << FI_USER << " " << options.ret_User() << '\n';
      }
      if(!itemcell->ret_Options_opt().ret_Password().empty()) {
	list_out << FI_PASSWORD << " " << options.ret_Password() << '\n';
      }
      list_out << FI_STORE_DIRECTORY << " " << options.ret_Store_Dir() << '\n';

      // HTTP version
      list_out << FI_HTTP_VERSION << " " << options.ret_HTTP_version() << '\n';
      list_out << FI_PREWRITTEN_HTML_TYPE << " " << options.getPrewrittenHTMLType() << '\n';
      list_out << FI_PREWRITTEN_HTML_NAME << " " << options.getPrewrittenHTMLName() << '\n';
      list_out << FI_SYNC_WITH_URL << ' ' << options.ret_sync_with_URL() << '\n';
      // Referer
      list_out << FI_REFERER_TYPE << " " << dec << (unsigned int)options.ret_Referer_Type() << '\n';
      if(!options.ret_Referer().empty()) {
	list_out << FI_REFERER_STRING << " " << options.ret_Referer() << '\n';
      }

      // Useragent
      if(!options.ret_Useragent().empty()) {
	list_out << FI_USERAGENT << " " << options.ret_Useragent() << '\n';
      }
      list_out << FI_RANDOM_USERAGENT << " " << options.ret_Random_useragent() << '\n';

      // Cookie
      list_out << FI_COOKIE_DELETE_ON_RESTART << " " << options.ret_Cookie_delete_on_restart() << '\n';
      list_out << FI_COOKIE_NOSEND << " " << options.ret_Cookie_nosend() << '\n';
      list_out << FI_COOKIE_USERDEFINED << " " << options.getCookieUserDefined() << '\n';
      list_out << FI_COOKIE_USERDEFINED_STRING << " " << options.getCookieUserDefinedString() << '\n';

      // HTTP Proxy
      list_out << FI_USE_HTTP_PROXY_AUTHENTICATION << " " << options.ret_use_http_proxy_authentication() << '\n';
      if(!options.ret_http_proxy_User().empty()) {
	list_out << FI_HTTP_PROXY_USER << " " << options.ret_http_proxy_User() << '\n';
      }
      if(!options.ret_http_proxy_Password().empty()) {
	list_out << FI_HTTP_PROXY_PASSWORD << " " << options.ret_http_proxy_Password() << '\n';
      }

      list_out << FI_USE_HTTP_PROXY << " " << dec << options.ret_use_http_proxy() << '\n';
      list_out << FI_USE_HTTP_CACHE << " " << dec << options.ret_use_http_cache() << '\n';
      if(!options.ret_http_proxy().ret_Server().empty()) {
	list_out << FI_HTTP_PROXY_SERVER << " " << options.ret_http_proxy().ret_Server() << '\n';
	list_out << FI_HTTP_PROXY_PORT << " " << dec << options.ret_http_proxy().ret_Port() << '\n';
      }

      // FTP Proxy
      list_out << FI_USE_FTP_PROXY_AUTHENTICATION << " " << options.ret_use_ftp_proxy_authentication() << '\n';
      if(!options.ret_ftp_proxy_User().empty()) {
	list_out << FI_FTP_PROXY_USER << " " << options.ret_ftp_proxy_User() << '\n';
      }
      if(!options.ret_ftp_proxy_Password().empty()) {
	list_out << FI_FTP_PROXY_PASSWORD << " " << options.ret_ftp_proxy_Password() << '\n';
      }
      list_out << FI_USE_FTP_PROXY << " " << dec << options.ret_use_ftp_proxy() << '\n';
      if(!options.ret_ftp_proxy().ret_Server().empty()) {
	list_out << FI_FTP_PROXY_SERVER << " " << options.ret_ftp_proxy().ret_Server() << '\n';
	list_out << FI_FTP_PROXY_PORT << " " << dec << options.ret_ftp_proxy().ret_Port() << '\n';
      }
      list_out << FI_USE_FTP_CACHE << " " << dec << options.ret_use_ftp_cache() << '\n';
      list_out << FI_USE_FTP_PROXY_VIA_HTTP << " " << dec << options.ret_use_ftp_proxy_via_http() << '\n';
      list_out << FI_FTP_PROXY_LOGIN_PROC << " " << dec << options.ret_ftp_proxy_login_proc() << '\n';

      // download method
      list_out << FI_DOWNM_TYPE << " " << dec << options.ret_downm_type() << '\n';
      // speed limit
      list_out << FI_SPEED_LIMIT << " " << dec << options.ret_speed_limit() << '\n';

      //time out
      list_out << FI_TIMED_OUT << " " << dec << options.ret_Timed_Out() << '\n';
      //split
      list_out << FI_HOW_MANY_PARTS << " " << dec << options.ret_Divide() << '\n';
      // Rollback in bytes
      list_out << FI_ROLLBACK_BYTES << " " << dec << options.ret_Rollback_bytes() << '\n';
      // Limit file size
      list_out << FI_USE_SIZE_LOWER_LIMIT << ' ' << dec << (unsigned int)options.ret_use_size_lower_limit() << '\n';
      list_out << FI_SIZE_LOWER_LIMIT << ' ' << dec << options.ret_size_lower_limit() << '\n';
      // do not redownload if resuming is not available
      list_out << FI_USE_NO_REDOWNLOAD << ' ' << dec << (unsigned int)options.ret_use_no_redownload() << '\n';
      list_out << FI_USE_NO_DOWNLOAD_SAMENAME << ' ' << dec << (unsigned int)options.ret_use_no_download_samename() << '\n';
  
      // accept-encodings request header option
      list_out << FI_HTTP_ACCEPT_COMPRESSION << " " << dec << (unsigned int)options.ret_HTTP_accept_compression() << '\n';

      // accept-languages request header option
      list_out << FI_HTTP_ACCEPT_LANG_ENABLED << " " << dec << options.ret_HTTP_accept_lang_enabled() << '\n';
      list_out << FI_HTTP_ACCEPT_LANG_STRING << " " << dec << options.ret_HTTP_accept_lang_string() << '\n';

      // recursive download
      list_out << FI_RECURSE_COUNT << " " << dec << options.ret_recurse_count() << '\n';
      list_out << FI_DOCUMENTROOT_DIR << " " << itemcell->ret_documentroot_dir() << '\n';
      list_out << FI_ROOT_URL << " " << itemcell->ret_root_url() << '\n';
      list_out << FI_WITH_HOSTNAME_DIR << " " << dec << (unsigned int)options.ret_with_hostname_dir() << '\n';
      list_out << FI_ABS2REL << " " << dec << (unsigned int)options.ret_abs2rel_url() << '\n';
      list_out << FI_FORCE_CONVERT << " " << dec << (unsigned int)options.ret_force_convert() << '\n';
      list_out << FI_DEL_COMMENT << " " << dec << (unsigned int)options.ret_delete_comment() << '\n';
      list_out << FI_DEL_JAVASCRIPT << " " << dec << (unsigned int)options.ret_delete_javascript() << '\n';
      list_out << FI_DEL_IFRAME << " " << dec << (unsigned int)options.ret_delete_iframe() << '\n';
      list_out << FI_NO_OTHER_HOST << " " << dec << (unsigned int)options.ret_no_other_host() << '\n';
      list_out << FI_NO_ASCEND << " " << dec << (unsigned int)options.ret_no_ascend() << '\n';
      list_out << FI_ONLY_RELATIVE_LINKS << " " << dec << (unsigned int)options.ret_only_relative_links() << '\n';
      list_out << FI_REFERER_OVERRIDE << " " << dec << (unsigned int)options.ret_Referer_override() << '\n';
      list_out << FI_FOLLOW_FTP_LINK << " " << dec << (unsigned int)options.ret_Follow_ftp_link() << '\n';
      list_out << FI_CONVERT_TILDE << " " << dec << (unsigned int)options.ret_convert_tilde() << '\n';
      list_out << FI_NO_REDOWNLOAD_HTTP_RECURSE << ' ' << dec << (unsigned int)options.ret_no_redownload_HTTP_recurse() << '\n';
      list_out << FI_HTTP_RECURSE_ADD_PASTE << " " << dec << (unsigned int)options.ret_HTTP_recurse_add_paste() << '\n';

      list_out << FI_USE_TAG_HREF << " " << dec << (unsigned int)options.ret_use_tag_href() << '\n';
      list_out << FI_USE_TAG_SRC << " " << dec << (unsigned int)options.ret_use_tag_src() << '\n';
      list_out << FI_USE_TAG_BACKGROUND << " " << dec << (unsigned int)options.ret_use_tag_background() << '\n';
      list_out << FI_USE_TAG_CODE << " " << dec << (unsigned int)options.ret_use_tag_code() << '\n';

      list_out << FI_IGN_DOMAIN_LIST;
      for(list<string>::const_iterator itr = options.ret_ign_domain_list().begin(); itr != options.ret_ign_domain_list().end(); ++itr) {
	list_out << " " << *itr;
      }
      list_out << '\n';
      list_out << FI_ACTIVATED_IGN_DOMAIN_LIST;
      for(list<string>::const_iterator itr = options.ret_activated_ign_domain_list().begin(); itr != options.ret_activated_ign_domain_list().end(); ++itr) {
	list_out << " " << *itr;
      }
      list_out << '\n';

      list_out << FI_USE_DOWN_FILTER << " " << dec << (unsigned int)options.ret_use_down_filter() << '\n';
      list_out << FI_FILTER_DOWN_TARGET_LIST;
      for(list<string>::const_iterator itr = options.ret_filter_down_target_list().begin(); itr != options.ret_filter_down_target_list().end(); ++itr) {
	list_out << "\t" << *itr;
      }
      list_out << '\n';
      list_out << FI_ACTIVATED_FILTER_DOWN_TARGET_LIST;
      for(list<string>::const_iterator itr = options.ret_activated_filter_down_target_list().begin(); itr != options.ret_activated_filter_down_target_list().end(); ++itr) {
	list_out << "\t" << *itr;
      }
      list_out << '\n';

      list_out << FI_FILTER_NODOWN_TARGET_LIST;
      for(list<string>::const_iterator itr = options.ret_filter_nodown_target_list().begin(); itr != options.ret_filter_nodown_target_list().end(); ++itr) {
	list_out << " " << *itr;
      }
      list_out << '\n';
      list_out << FI_ACTIVATED_FILTER_NODOWN_TARGET_LIST;
      for(list<string>::const_iterator itr = options.ret_activated_filter_nodown_target_list().begin(); itr != options.ret_activated_filter_nodown_target_list().end(); ++itr) {
	list_out << " " << *itr;
      }
      list_out << '\n';
      list_out << FI_PARSE_TARGET_LIST;
      for(list<string>::const_iterator itr = options.ret_parse_target_list().begin(); itr != options.ret_parse_target_list().end(); ++itr) {
	list_out << " " << *itr;
      }
      list_out << '\n';
      list_out << FI_ACTIVATED_PARSE_TARGET_LIST;
      for(list<string>::const_iterator itr = options.ret_activated_parse_target_list().begin(); itr != options.ret_activated_parse_target_list().end(); ++itr) {
	list_out << " " << *itr;
      }
      list_out << '\n';
      //FTP mode
      list_out << FI_FTP_MODE << " " << dec << (unsigned int)options.ret_FTP_Mode() << '\n';
      list_out << FI_FTP_RET_MODE << " " << dec << (unsigned int)options.ret_FTP_ret_mode() << '\n';
      list_out << FI_FTP_NOSEND_QUIT << " " << dec << (unsigned int)options.ret_FTP_nosend_quit() << '\n';
      list_out << FI_FTP_NO_CWD << " " << dec << (unsigned int)options.isFtpNoCwdEnabled() << '\n';
      list_out << FI_FTP_RECURSE_COUNT << " " << dec << (unsigned int)options.ret_FTP_recurse_count() << '\n';
      list_out << FI_FTP_NO_ASCEND << " " << dec << (unsigned int)options.ret_FTP_no_ascend() << '\n';
      list_out << FI_FTP_ALLOW_CRAWL_SUBDIR << " " << dec << (unsigned int)options.ret_FTP_allow_crawl_subdir() << '\n';
      list_out << FI_FTP_GET_SYMLINK_AS_REALFILE << " " << dec << (unsigned int)options.ret_FTP_get_symlink_as_realfile() << '\n';
      list_out << FI_FTP_RECURSE_ADD_PASTE << " " << dec << (unsigned int)options.ret_FTP_recurse_add_paste() << '\n';
      list_out << FI_FTP_USE_FILTER << " " << dec << (unsigned int)options.ret_FTP_use_filter() << '\n';
      list_out << FI_FTP_FILTER_TARGET_LIST;
      for(list<string>::const_iterator itr = options.ret_FTP_filter_target_list().begin(); itr != options.ret_FTP_filter_target_list().end(); ++itr) {
	list_out << "\t" << *itr;
      }
      list_out << '\n';
      list_out << FI_FTP_ACTIVATED_FILTER_TARGET_LIST;
      for(list<string>::const_iterator itr = options.ret_FTP_activated_filter_target_list().begin(); itr != options.ret_FTP_activated_filter_target_list().end(); ++itr) {
	list_out << "\t" << *itr;
      }
      list_out << '\n';

      //retry
      list_out << FI_RETRY << " " << dec << options.ret_Retry() << '\n';
      list_out << FI_RETRY_REPEAT << " " << dec << options.ret_Retry_repeat() << '\n';
      list_out << FI_RETRY_INTERVAL << ' ' << dec << options.ret_Retry_interval() << '\n';
      list_out << FI_FORCE_RETRY_404 << " " << dec << options.ret_force_retry_404() << '\n';
      list_out << FI_FORCE_RETRY_503 << " " << dec << options.ret_force_retry_503() << '\n';
      list_out << FI_STATUS_416_HANDLING << " " << dec << options.ret_status_416_handling() << '\n';
      list_out << FI_USE_NO_REDIRECTION << ' ' << dec << options.ret_use_no_redirection() << '\n';
  
      //some flags
      list_out << FI_DELETE_WHEN_COMPLETED << " " << dec << (unsigned int)options.ret_Delete_When_Finish() << '\n';
      list_out << FI_DONT_DELETE_WITHOUT_CRC << " " << dec << (unsigned int)options.ret_Dont_Delete_Without_CRC() << '\n';
      list_out << FI_NO_CRC_CHECKING << " " << dec << (unsigned int)options.ret_no_crc_checking() << '\n';
      list_out << FI_IGNORE_CRC_ERROR << " " << dec << (unsigned int)options.ret_ignore_crc_error() << '\n';
      list_out << FI_USE_CONTENT_MD5 << " " << dec << (unsigned int)options.ret_use_content_md5() << '\n';

      list_out << "%End_Item" << '\n';
    }
    //list_out << "%End_Items_Section" << '\n';
    list_out << "%End_Tab" << endl;
    listentry->release_Dl_clist_lock();

    if(list_out.bad() || list_out.fail()) return false;
  }

  list_out.close();
  // change the attribute
  if(rename(filenameTemp.c_str(), filename.c_str()) < 0) {
    return false;
  }
  chmod(filename.c_str(), S_IRUSR|S_IWUSR);

  return true;
}

bool ItemList::Save_app_settings()
{
  ofstream app_out;
  
  string filename = file_app_settings.c_str();
  string filenameTemp = filename+".temporary.working";

  app_out.open(filenameTemp.c_str(), ios::out);//, 0600);
  if(app_out.bad() || app_out.fail()) {
    cerr << "ERROR:Unable to save app settings" << endl;
    return false;
  }

  app_out << ARIA_VERSION << '\n';
  app_out << "%Threads: " << g_appOption->getThreadMax() << '\n';
  app_out << "%Automatic_Start: " << (int)g_appOption->Whether_use_automatic_start() << '\n';
  app_out << "%Autostart.CurrentListOnly: " << (int)g_appOption->isAutostartCurrentListOnly() << '\n';
  app_out << "%ForceDownloadNow: " << (int)g_appOption->isForceDownloadNowEnabled() << '\n';
  app_out << "%Use-Ignore-Error-Item: " << (unsigned int)g_appOption->ret_use_ignore_error_item() << '\n';
  app_out << "%USE-IGNORE-EXTENSION-LIST: " << (unsigned int)g_appOption->ret_use_ignore_extension_list() << '\n';
  if(!g_appOption->ret_ignore_extension_list().empty()) {
    app_out << "%IGNORE-EXTENSION-LIST:";
    for(list<string>::const_iterator itr = g_appOption->ret_ignore_extension_list().begin(); itr != g_appOption->ret_ignore_extension_list().end(); ++itr) {
      app_out << " " << *itr;
    }
    app_out << '\n';
  }
  app_out << "%Use-Autosave: " << (unsigned int)g_appOption->ret_use_autosave() << '\n';
  app_out << "%Autosave-Interval: " << (unsigned int)g_appOption->getAutosaveInterval() << '\n';

  app_out << "%Use-Arbitrary-Command: " << (unsigned int)g_appOption->ret_use_arb_command() << '\n';
  app_out << "%Use-Arbitrary-Command-Timer: " << (unsigned int)g_appOption->ret_use_arb_command_timer() << '\n';
  app_out << "%Arbitrary-Command: " << g_appOption->ret_arb_command() << '\n';

  app_out << "%Use-Quit-Program: " << (unsigned int)g_appOption->ret_use_quit_program() << '\n';

  // timer
  app_out << "%StartTimerEnabled: " << (unsigned int)g_appOption->isStartTimerEnabled() << '\n';
  app_out << "%StopTimerEnabled: " << (unsigned int)g_appOption->isStopTimerEnabled() << '\n';

  app_out << "%Timer-Start-All-Tab: " << (unsigned int)g_appOption->ret_timer_start_all_list() << '\n';
  app_out << "%Timer-No-Stop-Download-On-Timer: " << (unsigned int)g_appOption->isNoStopDownloadOnTimerEnabled() << '\n';
  app_out << "%Timer-Start-Hour: " << g_appOption->ret_timer_hour_start() << '\n';
  app_out << "%Timer-Start-Min: " << g_appOption->ret_timer_min_start() << '\n';
  app_out << "%Timer-Stop-Hour: " << g_appOption->ret_timer_hour_stop() << '\n';;
  app_out << "%Timer-Stop-Min: " << g_appOption->ret_timer_min_stop() << '\n';;
  app_out << "%History-Limit: " << g_appOption->ret_history_limit() << '\n';
  app_out << "%Max-Speed-Limit: " << g_appOption->getSpeedLimitMax() << '\n';
  app_out << "%Track-Download: " << Is_track_required() << '\n';

  app_out << "%CONFIRM-CLEAR: " << (unsigned int)g_appOption->ret_confirm_clear() << '\n';
  app_out << "%CONFIRM-DELETE-TAB: " << (unsigned int)g_appOption->ret_confirm_delete_list() << '\n';
  app_out << "%CONFIRM-EXIT: " << (unsigned int)g_appOption->ret_confirm_exit() << '\n';
  app_out << "%CONFIRM-CLEARLOG: " << (unsigned int)g_appOption->ret_confirm_clearlog() << '\n';
  app_out << "%USE-SERVERTEMPLATE: " << (unsigned int)g_appOption->ret_use_servertemplate() << '\n';
  list<string> names;
  app_out << "%SERVER-TEMPLATE-IN-USE: ";
  names = g_appOption->ret_svt_name_list();
  for(list<string>::iterator itr = names.begin(); itr != names.end(); ++itr) {
    app_out << " " << *itr;
  }
  app_out << '\n';
  app_out << "%COMMAND-LIST-IN-USE: ";
  names = g_appOption->ret_com_name_list();
  for(list<string>::iterator itr = names.begin(); itr != names.end(); ++itr) {
    app_out << " " << *itr;
  }
  app_out << '\n';

  app_out << "%USE-COMMANDLIST: " << (unsigned int)g_appOption->ret_use_commandlist() << '\n';
  app_out << "%USE-SIZE-HUMAN-READABLE: " << (unsigned int)g_appOption->ret_use_size_human_readable() << '\n';
  app_out << "%ICON-DIR: " << g_appOption->getStatusIconDir() << '\n';
  app_out << "%BASKET-PIXMAP-FILE: " << g_appOption->getBasketPixmapFile() << '\n';
  app_out << "%BASKET-DIRECT-PASTING: " << g_appOption->isDirectPastingFromBasketEnabled() << endl;
  // flush

  if(app_out.bad() || app_out.fail()) return false;

  app_out.close();
  if(rename(filenameTemp.c_str(), filename.c_str()) < 0) {
    return false;
  }
  // change the attributes
  chmod(filename.c_str(), S_IRUSR|S_IWUSR);

  return true;
}

bool ItemList::Save_default_item_settings()
{
  string filename = file_default_item_settings;
  string filenameTemp = filename+".temporary.working";

  ofstream default_out(filenameTemp.c_str(), ios::out);//, 0600);
  if(default_out.bad() || default_out.fail()) {
    cerr << "ERROR:Unable to save default settings" << endl;
    return false;
  }
  // file header
  default_out << ARIA_VERSION << '\n';

  if(!Save_default_item_option_sub(default_out, g_consoleItem->ret_Options_opt())) {
    return false;
  }

  default_out.close();

  if(rename(filenameTemp.c_str(), filename.c_str()) < 0) {
    return false;
  }
  // change the attributes
  chmod(filename.c_str(), S_IRUSR|S_IWUSR);

  return true;
}

const string& ItemList::ret_file_ftp_proxy_list() const
{
  return file_ftp_proxy_list;
}

const string& ItemList::ret_file_http_proxy_list() const
{
  return file_http_proxy_list;
}

const string& ItemList::ret_file_useragent_list() const
{
  return file_useragent_list;
}

const string& ItemList::ret_file_server_settings() const
{
  return file_server_settings;
}

const string& ItemList::ret_file_command_list() const
{
  return file_command_list;
}

const string& ItemList::ret_file_history() const
{
  return file_history;
}

const string& ItemList::ret_file_gui_info() const
{
  return file_gui_info;
}
