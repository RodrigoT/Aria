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

// $Id: ItemCell.cc,v 1.58 2002/04/06 22:13:06 tujikawa Exp $

// class ItemCell implementation
#include "ItemCell.h"
#include "ItemStatusDynamic.h"
#include "ItemStatusLog.h"
#include "ItemStatusPartial.h"
#include "ItemStatusRec.h"
#include "ItemOption.h"
#include "RetrieveHTTP.h"
#include "RetrieveFTP.h"
#ifdef HAVE_OPENSSL
#include "RetrieveHTTPS.h"
#endif // HAVE_OPENSSL
#include "md5check.h"
#include "crc.h"

extern void Send_report(MessageType reporttype, ItemStatus *data);
extern pthread_mutex_t itemlistlock;

ItemCell::ItemCell(const string& url_in,
		   const URLcontainer& urlcon_in,
		   const Options& options_in,
		   const string& initial_log)
{
  id = -1;
  urlcon = urlcon_in;

  filename = urlcon.ret_Filename();
  filename_opt = filename;
  options = options_in;
  // set username and password
  if(urlcon.ret_Username().size()) {
    options.set_use_authentication(true);
    Userdata userdata(urlcon.ret_Username(), urlcon.ret_Password());
    options.set_userpasswd(userdata);
  }
  url = urlcon.ret_URL();

  // initialize log message list

  loglist.push_back(ItemLogCell(initial_log, MSG_INIT));

  msgpipe[0] = -1;
  msgpipe[1] = -1;

  crc = 0;
  crc_type = CRC_NONE;
  status = ITEM_STOP;// ITEM_READY mod 2001/5/17
  currentsize = 0;
  totalsize = 0;
  count = 0;
  prevDlSize = 0;

  url_opt = url;
  urlcon_opt = urlcon;
  options_opt = options;

  flag_opt_updated = false;
  split_complete = true;
  // initialize mutex locks
  pthread_mutex_init(&itemloglock, NULL);
  pthread_mutex_init(&optionslock, NULL);
  pthread_mutex_init(&crclock, NULL);
  pthread_mutex_init(&itemlock, NULL);

  // init logging option
  log_flag = true;
  // init error code
  item_errno = ITEM_ENONE;
  // init event type
  dl_event = ItemCommand::EV_NOEVENT;

  // for http download
  session_counter = 1;
  retrieved_urlcon = urlcon_in;
}

ItemCell::ItemCell()
{
  split_complete = true;
  count = 0;

  msgpipe[0] = -1;
  msgpipe[1] = -1;
  
  // initialize mutex locks
  pthread_mutex_init(&itemloglock, NULL);
  pthread_mutex_init(&optionslock, NULL);
  pthread_mutex_init(&crclock, NULL);
  pthread_mutex_init(&itemlock, NULL);

  // init event type
  dl_event = ItemCommand::EV_NOEVENT;
}

ItemCell::~ItemCell()
{
  if(msgpipe[0] > 0) close(msgpipe[0]);
  if(msgpipe[1] > 0) close(msgpipe[1]);
  pthread_mutex_destroy(&itemloglock);
  pthread_mutex_destroy(&optionslock);
  pthread_mutex_destroy(&crclock);
  pthread_mutex_destroy(&itemlock);
}

const ItemLogList& ItemCell::ret_Log_list() const
{
  return loglist;
}

void ItemCell::Clear_log()
{
  pthread_mutex_lock(&itemloglock);
  loglist.clear();
  pthread_mutex_unlock(&itemloglock);
}

void ItemCell::reset_Count()
{
  count = 0;
}

void ItemCell::inc_Count()
{
  ++count;
}

int ItemCell::ret_Count() const
{
  return count;
}


void ItemCell::begin_split()
{
  split_complete = false;
}

void ItemCell::end_split()
{
  split_complete = true;
}

void ItemCell::set_logging(bool flag)
{
  log_flag = flag;
}

void ItemCell::Send_message_to_gui(const string& message,
				   MessageType reporttype)
{
  pthread_mutex_lock(&itemloglock);
  ItemLogCell itemlogcell(message, reporttype);
  if(log_flag) Append_itemlog(itemlogcell);
  pthread_mutex_unlock(&itemloglock);
  ItemStatusLog *itemstatus = new ItemStatusLog(this, itemlogcell);

  Send_report(reporttype, itemstatus);
}

void ItemCell::Send_partial(ItemCell *itemcell_partial)
{
  ItemStatusPartial *itemstatus = new ItemStatusPartial(this, itemcell_partial);
  Send_report(MSG_DOWNLOAD_ADD_PARTIAL, itemstatus);
}

void ItemCell::Send_status()
{
  ItemStatusDynamic *itemstatus = new ItemStatusDynamic(this, status, count, currentsize, totalsize);
  Send_report(MSG_DOWNLOAD_STATUS, itemstatus);
}

void ItemCell::Send_status(float speed, float avgSpeed)
{
  ItemStatusDynamic *itemstatus = new ItemStatusDynamic(this, status, count, currentsize, totalsize, speed, avgSpeed);
  Send_report(MSG_DOWNLOAD_STATUS, itemstatus);
}

void ItemCell::Send_status_complete()
{
  ItemStatusDynamic *itemstatus = new ItemStatusDynamic(this, status, count, currentsize, totalsize);
  itemstatus->set_DeleteFlag(true);
  Send_report(MSG_DOWNLOAD_STATUS, itemstatus);
} 


ItemCell::DownloadStatusType ItemCell::Process_command(ItemCommand itemcommand)
{
  switch(itemcommand.commandtype) {
  case ItemCommand::COMMAND_DELETE_ITEM:
    return DLDELETEITEM;
  case ItemCommand::COMMAND_DELETE_ITEM_FILE:
    return DLDELETEITEMFILE;
  case ItemCommand::COMMAND_HALT:
    return DLHALT;
  case ItemCommand::COMMAND_DOWNLOAD_AGAIN:
    {
      // delete file
      string filename = options.ret_Store_Dir()+ret_Filename();
      unlink(filename.c_str());
      
      Send_message_to_gui(_("Download again"), MSG_DOWNLOAD_INFO);
      set_Status(ITEM_DOWNLOAD_AGAIN);
      return DLAGAIN;
    }
  case ItemCommand::COMMAND_CHANGE_SPEED:
    get_Options_Lock();
    options_opt.set_speed_limit(itemcommand.value);
    options.set_speed_limit(itemcommand.value);
    release_Options_Lock();
    return DLCHANGE;
  case ItemCommand::COMMAND_STOP:
  default:
    return DLSTOP;
  }
}

ItemCell::DownloadStatusType ItemCell::ret_Dl_status() const
{
  return dl_status;
}

ItemCommand::EventCause ItemCell::ret_Dl_event() const
{
  return dl_event;
}

//  void ItemCell::Show_option_window()
//  {
//    g_itemOption->set_Option_Values(this, options_opt);
//    g_itemOption->Show_Option_Window();
//  }

// inherited class must override this method
ItemCell::DownloadStatusType ItemCell::Download_Main()
{
  Retrieve *retr_obj;

  if(urlcon.ret_Protocol() == "http:" ||
     ( urlcon.ret_Protocol() == "ftp:" &&
     options.ret_use_ftp_proxy() &&
     options.ret_use_ftp_proxy_via_http() &&
     !options.ret_ftp_proxy().ret_Server().empty())) {
    retr_obj = new RetrieveHTTP(this);
  } else if(urlcon.ret_Protocol() == "ftp:") {
    retr_obj = new RetrieveFTP(this);
  }
#ifdef HAVE_OPENSSL
  else if(urlcon.ret_Protocol() == "https:") { // fix this; add ftp proxy vis https(?)
    retr_obj = new RetrieveHTTPS(this);
  }
#endif // HAVE_OPENSSL
  else {
    Send_message_to_gui(_("Unknown protocol"), MSG_DOWNLOAD_ERROR);
    set_Command(ItemCell::DLERRORSTOP);
    return DLERRORSTOP;
  }
  DownloadStatusType retval = retr_obj->Download_Main();
  /*
  if(retval == DLSUCCESS || retval == DLSUCCESSALR) {
    if(retr_obj->Post_process() != ItemCell::DLSUCCESS) {//crc check and etc
      retval = ItemCell::DLERROR;
      set_Command(ItemCell::DLERROR);// fixed 2001/3/16
      set_Status(ItemCell::ITEM_CRCERROR);
    }
  }
  */
  delete retr_obj;

  return retval;
}

ItemCell::DownloadStatusType ItemCell::Post_process()
{
  DownloadStatusType retval = DLSUCCESS;
  try {
    if(!ret_Options().ret_no_crc_checking()) {
      if(ret_CRC_Type() != CRC_NONE) {
	if(!CRC_check_main(this)) {
	  if(ret_Options().ret_ignore_crc_error()) {
	    Send_message_to_gui(_("Ignore CRC error"), MSG_DOWNLOAD_INFO);
	  } else {
	    retval = DLERROR;
	  }
	}
      }
      if(md5str.size()) {
	string md5spec = ret_md5string();
	Send_message_to_gui(_("Checking MD5 checksum"), MSG_DOWNLOAD_INFO);
	try {
	  string md5comp = md5_check(ret_Options().ret_Store_Dir()+ret_Filename());
	  Send_message_to_gui(_("Calculated MD5 checksum is ")+md5comp, MSG_DOWNLOAD_INFO);
	  if(!casecomp(md5spec, md5comp)) {
	    Send_message_to_gui(_("MD5 checking failed"), MSG_DOWNLOAD_ERROR);
	    if(ret_Options().ret_ignore_crc_error()) {
	      Send_message_to_gui(_("Ignore MD5 error"), MSG_DOWNLOAD_INFO);     
	    } else {
	      retval = DLERROR;
	  }
	  } else {
	    Send_message_to_gui(_("MD5 checking successful"), MSG_DOWNLOAD_SUCCESS);
	  }
	} catch (md5ExceptionType err) {
	  Send_message_to_gui(_("IO error occurred"), MSG_DOWNLOAD_ERROR);
	  retval = DLERROR;
	}
      }
    }

    // HTTP only
    if(ret_URL_Container().ret_Protocol() == "http:" &&
       retval == DLSUCCESS) {
      if(!Execute_program()) {
	set_Status(ItemCell::ITEM_EXECERROR);
	set_Command(ItemCell::DLERROR);
	retval = DLERROR;
      }
    }
    Reset_Session_counter();
  } catch(ItemErrorType err) {
    PERROR(err);
    retval = DLERROR;
  }

  return retval;
}

bool ItemCell::Execute_program()
{
  bool retval = true;
  if(ret_svt().Is_valid(ret_Session_counter()) &&
    ret_svt().ret_session(ret_Session_counter()).Is_program_avaiable()) {
    Send_message_to_gui(_("Executing program: ")+ret_svt().ret_session(ret_Session_counter()).ret_program_line(ret_Filename(), ret_Options().ret_Store_Dir(), ret_URL_Container()), MSG_DOWNLOAD_INFO);
    int return_status = ret_svt().ret_session(ret_Session_counter()).Execute_program(ret_Filename(), ret_Options().ret_Store_Dir(), ret_URL_Container());
    switch(return_status) {
    case Session::SESSION_EXEC_SUCC:
      Send_message_to_gui(_("Good exit status"), MSG_DOWNLOAD_SUCCESS);
      retval = true;
      break;
    case Session::SESSION_EXEC_FAIL:
      Send_message_to_gui(_("Bad exit status"), MSG_DOWNLOAD_ERROR);
      retval = false;
      break;
    case Session::SESSION_EXEC_IGN_FATAL:
      Send_message_to_gui(_("Program exited abnormally"), MSG_DOWNLOAD_ERROR);
    case Session::SESSION_EXEC_IGN:
    default:
      Send_message_to_gui(_("Exit status ignored"), MSG_DOWNLOAD_INFO);
      retval = true;
      break;
    }
  }
  return retval;
}

ItemCell::CRC_Type ItemCell::ret_CRC_Type() const
{
  return crc_type;
}

const string& ItemCell::ret_URL() const
{
  return url;
}

unsigned int ItemCell::ret_CRC() const
{
  return crc;
}

ItemCell::ItemStatusType ItemCell::ret_Status()
{
  pthread_mutex_lock(&itemlock);
  ItemStatusType retval = status;
  pthread_mutex_unlock(&itemlock);
  return retval;
}

unsigned int ItemCell::ret_Size_Current() const
{
  return currentsize;
}

unsigned int ItemCell::ret_Size_Total() const
{
  return totalsize;
}

int ItemCell::ret_Desc_w() const
{
  return msgpipe[1];
}

int ItemCell::ret_Desc_r() const
{
  return msgpipe[0];
}

string ItemCell::ret_Filename() const
{
  // because ulcon.file returns something like "/filename",
  // so we need to ignore first character '/'
  //string filename = urlcon.ret_File();

  //int slash_index = filename.rfind('/');
  //if(slash_index >= 0) {
  //  filename.erase(0, slash_index+1);
  //}

  //return urlcon.ret_Filename();
  return filename;
}

string ItemCell::ret_Filename_opt() const
{
  // because ulcon.file returns something like "/filename",
  // so we need to ignore first character '/'
  //string filename = urlcon_opt.ret_File();

  //int slash_index = filename.rfind('/');
  //if(slash_index >= 0) {
  //  filename.erase(0, slash_index+1);
  //}

  //return urlcon_opt.ret_Filename();
  return filename_opt;
}

URLcontainer& ItemCell::ret_URL_Container()
{
  return urlcon;
}

void ItemCell::set_CRC_Type(CRC_Type crc_type_new)
{
  crc_type = crc_type_new;
}

void ItemCell::set_CRC(unsigned int crc_new)
{
  crc = crc_new;
}

void ItemCell::set_Status(ItemStatusType status_new)
{
  pthread_mutex_lock(&itemlock);
  status = status_new;
  pthread_mutex_unlock(&itemlock);
}

void ItemCell::set_Size_Current(unsigned int currentsize_new)
{
  currentsize = currentsize_new;
}

void ItemCell::set_Size_Total(unsigned int totalsize_new)
{
  totalsize = totalsize_new;
}

void ItemCell::set_URL_Container(const URLcontainer& urlcon_new)
{
  urlcon = urlcon_new;
}

void ItemCell::set_URL(const string& url_in)
{
  url = url_in;
}

void ItemCell::Append_itemlog(const ItemLogCell& itemlogcell)
{
  loglist.push_back(itemlogcell);
}

void ItemCell::Open_Desc()
{
  pipe(msgpipe);
}

void ItemCell::Close_Desc()
{
  close(msgpipe[0]);
  close(msgpipe[1]);
  msgpipe[0] = -1;
  msgpipe[1] = -1;
}

// some mutex locks
void ItemCell::get_Log_Lock()
{
  pthread_mutex_lock(&itemloglock);
}

void ItemCell::release_Log_Lock()
{
  pthread_mutex_unlock(&itemloglock);
}

void ItemCell::get_Options_Lock()
{
  pthread_mutex_lock(&optionslock);
}

void ItemCell::release_Options_Lock()
{
  pthread_mutex_unlock(&optionslock);
}

void ItemCell::get_CRC_Lock()
{
  pthread_mutex_lock(&crclock);
}

void ItemCell::release_CRC_Lock()
{
  pthread_mutex_unlock(&crclock);
}

void ItemCell::set_Options(const Options& options_in)
{
  options = options_in;
}

Options& ItemCell::ret_Options()
{
  return options;
}

void ItemCell::set_Errno(ItemErrorType item_errno_in)
{
  item_errno = item_errno_in;
}

ItemCell::ItemErrorType ItemCell::ret_Errno() const
{
  return item_errno;
}

void ItemCell::set_Command(DownloadStatusType dl_status_in)
{
  dl_status = dl_status_in;
  dl_event = ItemCommand::EV_INTERNAL; //internal event
}

void ItemCell::set_Command(const ItemCommand& itemcommand_in)
{
  dl_status = Process_command(itemcommand_in); // convert
  dl_event = itemcommand_in.eventtype;
}

ItemCell::DownloadStatusType ItemCell::ret_Command() const
{
  return dl_status;
}

bool ItemCell::Is_Partial() const
{
  return false;
}

void ItemCell::PERROR(ItemErrorType item_errno)
{
  string report;

  switch(item_errno) {
  case ITEM_ECANTRESOLVE:
    Send_message_to_gui(_("Can't resolve host"), MSG_DOWNLOAD_ERROR);
    break;
  case ITEM_ESOCKET:
    Send_message_to_gui(_("Socket error"), MSG_DOWNLOAD_ERROR);
    break;
  case ITEM_ECONNREFUSED:
    report = _("Connection refused - ")+itos(errno)+" : "+strerror(errno);
    Send_message_to_gui(report, MSG_DOWNLOAD_ERROR);
    break;
  case ITEM_ETIMEDOUT:
    Send_message_to_gui(_("Time out"), MSG_DOWNLOAD_ERROR);
    break;
  case ITEM_EIO:
    report = _("I/O error - ")+itos(errno)+" : "+strerror(errno);
    Send_message_to_gui(report, MSG_DOWNLOAD_ERROR);
    break;
  case ITEM_EIOFILE:
    report = _("File I/O error - maybe wrong save location or disk full?");
    Send_message_to_gui(report, MSG_DOWNLOAD_ERROR);
    break;
  case ITEM_ELOCATION:
    Send_message_to_gui(_("Location retrieval failed"), MSG_DOWNLOAD_ERROR);
    break;
  case ITEM_ECANTFINDURL:
    Send_message_to_gui(_("Cannot find embedded URL"), MSG_DOWNLOAD_ERROR);
    break;
  case ITEM_EFTPCOM:
    Send_message_to_gui(_("FTP server returned negative response"), MSG_DOWNLOAD_ERROR);
    break;
  case ITEM_EBIND:
    report = _("Bind error - ")+itos(errno)+" : "+strerror(errno);
    Send_message_to_gui(report, MSG_DOWNLOAD_ERROR);
    break;
  case ITEM_ELISTEN:
    report = _("Listen error - ")+itos(errno)+" : "+strerror(errno);
    Send_message_to_gui(report, MSG_DOWNLOAD_ERROR);
    break;
  case ITEM_EACCEPT:
    report = _("Accept error - ")+itos(errno)+" : "+strerror(errno);
    Send_message_to_gui(report, MSG_DOWNLOAD_ERROR);
    break;
  case ITEM_ESEND:
    report = _("Send error - ")+itos(errno)+" : "+strerror(errno);
    Send_message_to_gui(report, MSG_DOWNLOAD_ERROR);
    break;
  case ITEM_ERECV:
    report = _("Recv error - ")+itos(errno)+" : "+strerror(errno);
    Send_message_to_gui(report, MSG_DOWNLOAD_ERROR);
    break;
  case ITEM_EPROT:
    report = _("General protocol error");//+itos(errno)+" : "+strerror(errno);
    Send_message_to_gui(report, MSG_DOWNLOAD_ERROR);
    break;
  default:
    Send_message_to_gui(_("Unknown error occurred"), MSG_DOWNLOAD_ERROR);
    break;
  }
}

void ItemCell::Append_worker(ItemCell *itemcell_partial)
{
  worker_list.push_back(itemcell_partial);
}

void ItemCell::Remove_worker(ItemCell *itemcell_partial)
{
  worker_list.remove(itemcell_partial);
}

void ItemCell::Clear_worker()
{
  worker_list.clear();
}

bool ItemCell::No_more_worker() const
{
  if(worker_list.size() || !split_complete) return false;
  else return true;
}

const list<ItemCell*>& ItemCell::ret_Worker_list() const
{
  return worker_list;
}

Options& ItemCell::ret_Options_opt()
{
  return options_opt;
}

void ItemCell::set_Options_opt(const Options& options_in)
{
  options_opt = options_in;
}

void ItemCell::set_URL_opt(const string& url_in)
{
  url_opt = url_in;
}

const string& ItemCell::ret_URL_opt() const
{
  return url_opt;
}

URLcontainer& ItemCell::ret_URL_Container_opt()
{
  return urlcon_opt;
}

void ItemCell::set_URL_Container_opt(const URLcontainer& urlcon_in)
{
  urlcon_opt = urlcon_in;
}

void ItemCell::Raise_option_update_flag()
{
  flag_opt_updated = true;
}

bool ItemCell::Is_option_updated()
{
  return flag_opt_updated;
}

void ItemCell::Apply_new_options()
{
  if(flag_opt_updated) {
    flag_opt_updated = false;
    //options_opt.set_documentroot_dir(options.ret_documentroot_dir());
    if(options.ret_Store_Dir() != options_opt.ret_Store_Dir()) {
      root_url = "";
      documentroot_dir = "";
    }
    options = options_opt;
    if(urlcon.ret_URL() != urlcon_opt.ret_URL()) {
      // if url changed, reset previous download size to 0
      prevDlSize = 0;// added 2001/5/20
    }
    urlcon = urlcon_opt;
    url = url_opt;
    filename = filename_opt;
    count = 0; // reset retry count; added 2001/3/14
    //set_Size_Total(0);
    //set_Size_Current(0);
    //set_Status(ITEM_READY);
    Reset_Session_counter();// added 2001/7/4
    Send_status();
  }
}

bool ItemCell::Concatenate_partial_files()
{
  try {
    string filename = options.ret_Store_Dir()+ret_Filename();
    //string partial_filename = filename+".-1";
    //struct stat file_stat;
    unsigned int index_start;
    /*
    if(stat(partial_filename.c_str(), &file_stat) == -1 || !S_ISREG(file_stat.st_mode)) {
      partial_filename = filename+".0";
      if(rename(partial_filename.c_str(), filename.c_str())) {
	throw ITEM_EIO;
      }
      index_start = 1;
    } else {
      if(rename(partial_filename.c_str(), filename.c_str())) {
	throw ITEM_EIO;
      }
      index_start = 0;
    }
    */
    string partial_filename = filename+".0";
    if(rename(partial_filename.c_str(), filename.c_str())) {
      throw ITEM_EIO;
    }
    index_start = 1;
    string indexfilename = partial_filename+".index";
    unlink(indexfilename.c_str());

    ofstream outfile(filename.c_str(), ios::binary|ios::out|ios::app);
    if(outfile.bad()) {
      throw ITEM_EIO;
    }
    for(unsigned int i = index_start; i < options.ret_Divide(); ++i) {
      ifstream infile;
      partial_filename = filename+"."+itos(i);
      infile.open(partial_filename.c_str(), ios::binary|ios::in);
      if(infile.bad()) {
	throw ITEM_EIO;
      }
      while(!infile.eof()) {
	char buf[8192];
	infile.read(buf, sizeof(buf));
	outfile.write(buf, infile.gcount());

	// infile.fail() is true here. Why?
	if(infile.bad() || outfile.bad() || outfile.fail()) {
	  throw ITEM_EIO;
	}
      }
      infile.close();
      unlink(partial_filename.c_str());
      string indexfilename = partial_filename+".index";
      unlink(indexfilename.c_str());
    }
    outfile.close();

    /*
    FILE *outfile = fopen(filename.c_str(), "a");
    if(outfile == NULL) throw ITEM_EIO;
    for(unsigned int i = index_start; i < options.ret_Divide(); ++i) {
      partial_filename = filename+"."+itos(i);
      FILE *infile = fopen(partial_filename.c_str(), "r");
      if(infile == NULL) {
	throw ITEM_EIO;
      }
      while(1) {
	char buf[8192];
	size_t size = fread(buf, sizeof(buf), 1, infile);
	if(size == 0) {
	  if(ferror(infile)) throw ITEM_EIO;
	  break;
	}
	size_t size_w = fwrite(buf, sizeof(buf), 1, outfile);
	if(size_w < size) {
	  throw ITEM_EIO;
	}
      }
      fclose(infile);
      unlink(partial_filename.c_str());
    }
    fclose(outfile);
    */
    return true;
  }
  catch(ItemErrorType err) {
    PERROR(err);
    return false;
  }
}

void ItemCell::set_id(int id_in)
{
  //if(id != -1 && id_in != -1) cerr << "warning: new id provided, while old id is still valid" << endl;
  id = id_in;
}

int ItemCell::ret_id() const
{
  return id;
}

const string& ItemCell::ret_root_url() const
{
  return root_url;
}

void ItemCell::set_root_url(const string& root_url_in)
{
  root_url = root_url_in;
}

const string& ItemCell::ret_documentroot_dir() const
{
  return documentroot_dir;
}

void ItemCell::set_documentroot_dir(const string& documentroot_dir_in)
{
  documentroot_dir = documentroot_dir_in;
}

void ItemCell::Send_status_recursive(const list<ItemCell *>& item_list)
{
  bool addPaste = false;
  if(ret_URL_Container().ret_Protocol() == "http:") {
    addPaste = ret_Options().ret_HTTP_recurse_add_paste();
  } else if(ret_URL_Container().ret_Protocol() == "ftp:") {
    addPaste = ret_Options().ret_FTP_recurse_add_paste();
  }
  ItemStatusRec *itemstatus = new ItemStatusRec(this, item_list, addPaste);
  Send_report(MSG_DOWNLOAD_STATUS, itemstatus);
}

int ItemCell::ret_Session_counter()
{
  return session_counter;
}

void ItemCell::Inc_Session_counter()
{
  ++session_counter;
}

void ItemCell::Reset_Session_counter()
{
  session_counter = 1;
}

URLcontainer& ItemCell::ret_Retrieved_urlcon()
{
  return retrieved_urlcon;
}

void ItemCell::set_Retrieved_urlcon(const URLcontainer& urlcon)
{
  retrieved_urlcon = urlcon;
}

Session& ItemCell::ret_current_session()
{
  return svt.ret_session(session_counter);
}

bool ItemCell::Is_current_session_valid()
{
  return svt.Is_valid(session_counter);
}

ServerTemplate& ItemCell::ret_svt()
{
  return svt;
}

void ItemCell::set_svt(const ServerTemplate& svt_in)
{
  svt = svt_in;
}

CookieList& ItemCell::ret_Cookie_list()
{
  return cookie_list;
}

void ItemCell::set_Cookie_list(const CookieList& cookie_list_in)
{
  cookie_list = cookie_list_in;
}

void ItemCell::set_Filename(const string& filename_in)
{
  filename = filename_in;
}

void ItemCell::set_Filename_opt(const string& filename_in)
{
  filename_opt = filename_in;
}

string ItemCell::ret_md5string()
{
  pthread_mutex_lock(&itemlock);
  string retval = md5str;
  pthread_mutex_unlock(&itemlock);

  return retval;
}

void ItemCell::set_md5string(const string& md5string_new)
{
  pthread_mutex_lock(&itemlock);
  md5str = md5string_new;
  pthread_mutex_unlock(&itemlock);
}

void ItemCell::set_previous_dl_size(unsigned int size)
{
  prevDlSize = size;
}

unsigned int ItemCell::ret_previous_dl_size()
{
  return prevDlSize;
}


