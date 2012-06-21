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

// $Id: download.cc,v 1.57 2002/12/18 15:41:05 tujikawa Exp $

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <signal.h>
#include "aria.h"
#include "ItemCell.h"
#include "ListManager.h"
#include "ItemStatusReq.h"
#include "ItemStatusSum.h"
#include "AppOption.h"
#include "ItemList.h"
#include "CommandList.h"
#include "HistoryWindow.h"
#include "SumInfo.h"

extern char **environ; // declared in unistd.h
extern int g_pipetogui[2];
extern pthread_mutex_t g_appLock;
extern pthread_mutex_t g_appLock2;
extern ItemList *g_itemList;
extern ItemCell *g_consoleItem;
extern CommandList g_commandList;
extern HistoryWindow *g_historyWindow;
extern AppOption *g_appOption;
extern ListManager *g_listManager;
extern pthread_key_t g_tsdKey;
extern SumInfo g_summaryInfo;
extern bool Is_track_required();
extern pthread_t guiThreadId;

void Send_report(MessageType reporttype, ItemStatus *itemstatus)
{
  //itemstatus->set_Listentry((ListEntry *)pthread_getspecific(g_tsdKey));
  StatusReport statusreport;
  
  statusreport.reporttype = reporttype;
  statusreport.data = (void *)itemstatus;

  if(pthread_self() == guiThreadId) {
    itemstatus->set_Listentry(g_listManager->ret_Current_listentry());
    statusreport.mutex_p = 0;
    statusreport.cond_p = 0;
    write(g_pipetogui[1], &statusreport, sizeof(statusreport));
  } else {
    itemstatus->set_Listentry((ListEntry *)pthread_getspecific(g_tsdKey));
    // prepare mutex and cond
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    pthread_mutex_init(&mutex, 0);
    pthread_cond_init(&cond, 0);
    pthread_mutex_lock(&mutex);
    statusreport.mutex_p = &mutex;
    statusreport.cond_p = &cond;
    write(g_pipetogui[1], &statusreport, sizeof(statusreport));
    // wait until the gui thread performs their work
    pthread_cond_wait(&cond, &mutex);
    // unlock
    pthread_mutex_unlock(&mutex);
    // clean up
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
  }
}

void Send_report(MessageType reporttype, ItemStatus *itemstatus, ListEntry *listentry)
{
  itemstatus->set_Listentry(listentry);
  StatusReport statusreport;
  
  statusreport.reporttype = reporttype;
  statusreport.data = (void *)itemstatus;
  if(pthread_self() == guiThreadId) {
    statusreport.mutex_p = 0;
    statusreport.cond_p = 0;
    write(g_pipetogui[1], &statusreport, sizeof(statusreport));
  } else {
    // prepare mutex and cond
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    pthread_mutex_init(&mutex, 0);
    pthread_cond_init(&cond, 0);
    pthread_mutex_lock(&mutex);
    statusreport.mutex_p = &mutex;
    statusreport.cond_p = &cond;
    write(g_pipetogui[1], &statusreport, sizeof(statusreport));
    // wait until the gui thread performs their work
    pthread_cond_wait(&cond, &mutex);
    // unlock
    pthread_mutex_unlock(&mutex);
    // clean up
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);    
  }
}

static void UNLOCK_AND_EXIT(ListEntry *listentry)
{
  //ListEntry *listentry = (ListEntry *)pthread_getspecific(g_tsdKey);
  listentry->release_Dl_clist_lock();
  pthread_exit(NULL);
}

// return true if all thread in all tabs are sleeping
bool Is_all_thread_sleeping()
{
  for(list<ListEntry*>::const_iterator itr = g_listManager->ret_Listentry_list().begin();
      itr != g_listManager->ret_Listentry_list().end(); ++itr) {
    ThreadManager *thread_manager = (*itr)->getThreadManager();
    if(!thread_manager->isNoActiveThread()) return false;
  }
  return true;
}

static void Download_success(ItemCell *itemcell)
{
  string line;

  if(itemcell->Is_Partial()) {
    line = _("A part of '")+itemcell->ret_Filename()+_("' download complete");
  } else {
    string filename;
    if(itemcell->ret_Filename().empty()) {
      filename = _("<directory>");
    } else {
      filename = itemcell->ret_Filename();
    }
    line = "'"+filename+_("' download complete");
    g_historyWindow->addItem(itemcell);
  }
  itemcell->Send_message_to_gui(line, MSG_DOWNLOAD_SUCCESS);
  g_consoleItem->Send_message_to_gui(line, MSG_SYS_SUCCESS);

  // update sumup information
  ItemStatusSum *itemstatus = new ItemStatusSum(g_consoleItem);
  if(itemcell->ret_Status() == ItemCell::ITEM_DOWNLOAD_PARTIAL) {
  } else {
    g_summaryInfo.dec_download();
  }
  Send_report(MSG_SYS_INFO, itemstatus);

  itemcell->set_Status(ItemCell::ITEM_COMPLETE);

  itemcell->Close_Desc();

  itemcell->Send_status_complete();
  itemcell = NULL;
}

// this function is called if a item uses split download option and
// is split successfully
static void Download_partial_success(ItemCell *itemcell)
{
  string line = _("Starting split download");

  // update sumup information
  ItemStatusSum *itemstatus = new ItemStatusSum(g_consoleItem);
  g_summaryInfo.dec_download();
  //itemstatus->inc_split();
  Send_report(MSG_SYS_INFO, itemstatus);

  itemcell->Send_message_to_gui(line, MSG_DOWNLOAD_INFO);
  itemcell->Close_Desc();
  itemcell = NULL;
}

static void Download_stop(ItemCell *itemcell)
{
  string line = _("Download stopped");
  itemcell->Send_message_to_gui(line, MSG_DOWNLOAD_INFO);

  // update sumup information
  ItemStatusSum *itemstatus = new ItemStatusSum(g_consoleItem);
  g_summaryInfo.dec_download();
  //g_summaryInfo.inc_stop();
  Send_report(MSG_SYS_INFO, itemstatus);

  itemcell->set_Status(ItemCell::ITEM_STOP);
  itemcell->Send_status();
  itemcell->Close_Desc();
}

static void Download_delete_item(ItemCell *itemcell)
{
  // update sumup information
  ItemStatusSum *itemstatus = new ItemStatusSum(g_consoleItem);
  g_summaryInfo.dec_download();
  Send_report(MSG_SYS_INFO, itemstatus);

  itemcell->Close_Desc();
  delete itemcell;
  itemcell = NULL;
}

static void Download_delete_item_file(ItemCell *itemcell) {
  ItemStatusSum *itemstatus = new ItemStatusSum(g_consoleItem);
  g_summaryInfo.dec_download();
  Send_report(MSG_SYS_INFO, itemstatus);

  itemcell->Close_Desc();
  try {
    if(itemcell->ret_Options().ret_Divide() > 1) {
      for(int i = 0; i < (int)itemcell->ret_Options().ret_Divide(); ++i) {
	string filename = itemcell->ret_Options().ret_Store_Dir()+itemcell->ret_Filename()+"."+itos(i);
	if(unlink(filename.c_str()) < 0) {
	  throw 0;
	}
	filename = filename+".index";
	if(unlink(filename.c_str()) < 0) {
	  throw 0;
	}
      }
    }
    if(unlink((itemcell->ret_Options().ret_Store_Dir()+itemcell->ret_Filename()).c_str()) < 0) {
      throw 0;
    }
  } catch (int err) {
    g_consoleItem->Send_message_to_gui(_("Error occurred while deleting files")+(":"+itemcell->ret_Options().ret_Store_Dir())+itemcell->ret_Filename(),
				       MSG_SYS_ERROR);
  }
  delete itemcell;
  itemcell = NULL;
}

static bool Download_error(ItemCell *itemcell)
{
  fd_set rfds;

  itemcell->Send_message_to_gui(_("Wait for ")+itos(itemcell->ret_Options_opt().ret_Retry_interval())+_(" seconds"), MSG_DOWNLOAD_ERROR);
  itemcell->Send_status();

  itemcell->get_Options_Lock();
  long wait_sec = itemcell->ret_Options_opt().ret_Retry_interval();
  long wait_usec = 0;
  itemcell->release_Options_Lock();

  while(1) {
    FD_ZERO(&rfds);
    FD_SET(itemcell->ret_Desc_r(), &rfds);
    struct timeval tvStart;
    struct timezone tz;
    gettimeofday(&tvStart, &tz);
    
    struct timeval tv;
    tv.tv_sec = wait_sec;
    tv.tv_usec = wait_usec;

    int retval = select(itemcell->ret_Desc_r()+1, &rfds, NULL, NULL, &tv);
    if(retval && FD_ISSET(itemcell->ret_Desc_r(), &rfds)) {
      ItemCommand itemcommand;
      read(itemcell->ret_Desc_r(), &itemcommand, sizeof(struct ItemCommand));
      itemcell->set_Command(itemcommand);
      switch(itemcommand.commandtype) {
      case ItemCommand::COMMAND_STOP:{
	// update sumup information
	ItemStatusSum *itemstatus = new ItemStatusSum(g_consoleItem);
	g_summaryInfo.dec_download();
	//g_summaryInfo.inc_stop();
	Send_report(MSG_SYS_INFO, itemstatus);
	
	itemcell->Close_Desc();
	itemcell->set_Status(ItemCell::ITEM_STOP);
	itemcell->Send_status();
	itemcell->Send_message_to_gui(_("Download stopped"), MSG_DOWNLOAD_INFO);
	return true;
      }

      case ItemCommand::COMMAND_DELETE_ITEM:{
	// update sumup information
	ItemStatusSum *itemstatus = new ItemStatusSum(g_consoleItem);
	g_summaryInfo.dec_download();
	Send_report(MSG_SYS_INFO, itemstatus);
	
	itemcell->Close_Desc();
	delete itemcell;
	itemcell = NULL;
	return true;
      }
      case ItemCommand::COMMAND_DELETE_ITEM_FILE:
	Download_delete_item_file(itemcell);
	return true;
      case ItemCommand::COMMAND_HALT: {
	// update sumup information
	ItemStatusSum *itemstatus = new ItemStatusSum(g_consoleItem);
	g_summaryInfo.dec_download();
	Send_report(MSG_SYS_INFO, itemstatus);
	
	itemcell->Close_Desc();
	pthread_exit(NULL);
      }
      case ItemCommand::COMMAND_DOWNLOAD_AGAIN:{
	itemcell->set_Status(ItemCell::ITEM_DOWNLOAD_AGAIN);
	itemcell->Send_message_to_gui(_("Download again"), MSG_DOWNLOAD_INFO);
	return false;
      }
      case ItemCommand::COMMAND_CHANGE_SPEED:{
	itemcell->Process_command(itemcommand);
	/*
	itemcell->get_Options_Lock();
	itemcell->ret_Options_opt().set_speed_limit(itemcommand.value);
	itemcell->ret_Options().set_speed_limit(itemcommand.value);
	itemcell->release_Options_Lock();
	*/
	struct timeval tvEnd;
	gettimeofday(&tvEnd, &tz);

	long elapsed_usec;
	long elapsed_sec = tvEnd.tv_sec-tvStart.tv_sec;
	if(tvEnd.tv_usec < tvStart.tv_usec) {
	  --elapsed_sec;
	  elapsed_usec = (tvEnd.tv_usec+1000000)-tvStart.tv_usec;
	} else {
	  elapsed_usec = tvEnd.tv_usec-tvStart.tv_usec;
	}

	if(wait_sec <= elapsed_sec) {
	  return false;
	}
	wait_sec -= elapsed_sec;

	if(wait_usec < elapsed_usec) {
	  --wait_sec;
	  wait_usec += 1000000-elapsed_usec;
	} else {
	  wait_usec -= elapsed_usec;
	}
	break;
      }
      default:
	return true;
      }
    } else {
      return false;
    }
  }
}

static void Maximum_retry_count_exceeded(ItemCell *itemcell)//, unsigned int& retry_repeat)
{
  //if(1 || retry_repeat  > retry_repeat_item) {
  // update sumup information
  ItemStatusSum *itemstatus = new ItemStatusSum(g_consoleItem);
  g_summaryInfo.dec_download();
  g_summaryInfo.inc_error();
  Send_report(MSG_SYS_INFO, itemstatus);

  string filenameFix;
  if(itemcell->ret_Filename().size()) {
    filenameFix = itemcell->ret_Filename();
  } else {
    filenameFix = _("<directory>");
  }
  string line = "'"+filenameFix+_("' download aborted");
  itemcell->Send_message_to_gui(line, MSG_DOWNLOAD_ERROR);
  itemcell->set_Status(ItemCell::ITEM_ERROR);
  itemcell->Send_status();
  itemcell->Close_Desc();
    /*
  } else {
    string line = "push back '"+itemcell->ret_Filename()+_("' to the end of the list");
    itemcell->Send_message_to_gui(line, MSG_DOWNLOAD_ERROR);
    itemcell->set_Status(ItemCell::ITEM_ERROR);
    itemcell->Send_status();
    itemcell->Close_Desc();
  }
    */
  g_consoleItem->Send_message_to_gui(line, MSG_SYS_ERROR);
}
/*
static ItemCell *ftp2http(ItemCell *itemcell)
{
  ItemCell *itemcell_new = new ItemCell_HTTP(itemcell->ret_URL_opt(),
					     itemcell->ret_URL_Container_opt(),
					     itemcell->ret_Options_opt(),
					     "converted");
  return itemcell_new;
}

static ItemCell *http2ftp(ItemCell *itemcell)
{
  ItemCell *itemcell_new = new ItemCell_FTP(itemcell->ret_URL_opt(),
					    itemcell->ret_URL_Container_opt(),
					    itemcell->ret_Options_opt(),
					    "converted");
  return itemcell_new;
}

ItemCell *Protocol_conversion(ItemCell *itemcell)
{
  ItemCell *itemcell_new;
  if(itemcell->ret_URL_Container_opt().ret_Protocol() == "http:") {
    itemcell_new = ftp2http(itemcell);
  } else {
    itemcell_new = http2ftp(itemcell);
  }
  return itemcell_new;
}
*/
static string create_commandline2(const string& command) {
  string home_dir = g_get_home_dir();
  string commandline = "cd "+home_dir+"; "+command;
  return commandline;
}

void *Exec_and_wait_thread()
{
  int status;
  int pid = fork();

  string commandline = create_commandline2(g_appOption->ret_arb_command());

  try {
    if(pid == -1)
      throw 0;
    if(pid == 0) {
      const char *argv[4];
      argv[0] = "sh";
      argv[1] = "-c";
      argv[2] = commandline.c_str();
      argv[3] = 0;
      execve("/bin/sh", (char **)argv, environ);
      exit(0);
    } else if(pid < 0) {
      throw 0;
      //g_consoleItem->Send_message_to_gui(_("cannot execute: ")+commandline, MSG_SYS_ERROR);
    }
    while(1) {
      if(waitpid(pid, &status, 0) == -1) {
	if(errno != EINTR)
	  throw 0;
      } else throw 0;
    }
    throw 0;
  } catch (int err) {
    pthread_exit(NULL);
  }
}

static void Do_something_on_event_sub()
{
  if(g_appOption->ret_use_arb_command() && g_appOption->ret_arb_command().size()) {
    string commandline = create_commandline2(g_appOption->ret_arb_command());
    g_consoleItem->Send_message_to_gui(_("Executing program: ")+commandline, MSG_SYS_INFO);
    pthread_t do_anything_thread;
    pthread_create(&do_anything_thread, NULL, (void *(*)(void*))Exec_and_wait_thread, NULL);
    pthread_detach(do_anything_thread);
  }
  if(g_appOption->ret_use_quit_program()) {
    ItemStatusReq *itemstatus = new ItemStatusReq(g_consoleItem, ItemStatusReq::REQ_QUIT);
    Send_report(MSG_SYS_REQ, itemstatus);
  }
}

static void Do_something_on_event_when_timer_expires_sub()
{
  if(g_appOption->ret_use_arb_command_timer() && g_appOption->ret_arb_command().size()) {
    string commandline = create_commandline2(g_appOption->ret_arb_command());
    g_consoleItem->Send_message_to_gui(_("Executing program: ")+commandline, MSG_SYS_INFO);
    pthread_t do_anything_thread;
    pthread_create(&do_anything_thread, NULL, (void *(*)(void*))Exec_and_wait_thread, NULL);
    pthread_detach(do_anything_thread);
  }
}

void Do_something_on_event(ItemCommand::EventCause last_event)
{
  switch(last_event) {
  case ItemCommand::EV_INTERNAL:
    Do_something_on_event_sub();
    break;
  case ItemCommand::EV_TIMERINTER:
    Do_something_on_event_when_timer_expires_sub();
    break;
  default:
    //cerr << "event must be: user interaction or halt message from gui" << endl;
    break;
  }
}

static ItemCell::DownloadStatusType Execute_command_sub(ItemCell *itemcell, const Command& command)
{
  ItemCell::DownloadStatusType retval = ItemCell::DLSUCCESS;

  string line;

  string commandline = command.Create_commandline(itemcell->ret_Filename(), itemcell->ret_Options().ret_Store_Dir(), itemcell->ret_URL_Container());
  line = _("Command line is '")+commandline+"'";
  itemcell->Send_message_to_gui(line, MSG_DOWNLOAD_INFO);
  
  int status = command.Execute_commandline(itemcell->ret_Filename(), itemcell->ret_Options().ret_Store_Dir(), itemcell->ret_URL_Container());
  int ex_stat =  WIFEXITED(status);
  int return_stat = 0;
  if(ex_stat) {
    return_stat = WEXITSTATUS(status);
    line = _("Exit status is ")+itos(return_stat);
    itemcell->Send_message_to_gui(line, MSG_DOWNLOAD_INFO);
  } else {
    line = _("Program exited abnormally");
    itemcell->Send_message_to_gui(line, MSG_DOWNLOAD_ERROR);
  }
  
  if(!command.Is_ignore_return_status()) {
    if(ex_stat && command.Is_in_succ_status_list(return_stat)) {
      itemcell->Send_message_to_gui(_("Good exit status"), MSG_DOWNLOAD_SUCCESS);
    } else {
      itemcell->Send_message_to_gui(_("Bad exit status"), MSG_DOWNLOAD_ERROR);
      itemcell->set_Command(ItemCell::DLERROR);
      retval = ItemCell::DLERROR;
      itemcell->set_Status(ItemCell::ITEM_EXECERROR);
    }
  } else {
    itemcell->Send_message_to_gui(_("Exit status ignored"), MSG_DOWNLOAD_INFO);
  }
  return retval;
}

ItemCell::DownloadStatusType Execute_command_in_option(ItemCell *itemcell)
{
  ItemCell::DownloadStatusType retval = ItemCell::DLSUCCESS;

  const Command& command = itemcell->ret_Options_opt().ret_Command();
  //if(command.Is_valid()) {
  if(command.ret_command().size() > 0) {
    retval = Execute_command_sub(itemcell, command);
  }
  return retval;
}

ItemCell::DownloadStatusType Execute_command_list_entry(ItemCell *itemcell)
{
  ItemCell::DownloadStatusType retval = ItemCell::DLSUCCESS;

  //Command command = g_commandList.search(itemcell->ret_Filename());
  Command command = g_commandList.search(itemcell->ret_URL_Container().ret_Filename());
  if(command.Is_valid()) {
    string line = _("Executing command entry '")+command.ret_command_name()+"'";
    itemcell->Send_message_to_gui(line, MSG_DOWNLOAD_INFO);
    retval = Execute_command_sub(itemcell, command);
  }
  return retval;
}

void *Download_thread_main(ListEntry *listentry)
{
  //signal(SIGPIPE, SIG_IGN);

  int retval;
  ItemCell *itemcell = NULL;

  listentry->get_Dl_clist_lock();
  // set listentry to thread specific data area
  pthread_setspecific(g_tsdKey, (void *)listentry);

  ThreadManager *thread_manager = listentry->getThreadManager();
  //ItemManager *item_manager = listentry->getItemManager();

  bool autostart_flag = thread_manager->getAutostartFlag();
  listentry->release_Dl_clist_lock();
  ItemCommand::EventCause last_event = ItemCommand::EV_NOEVENT;

  nice(20);
  
  while(1) {
    listentry->get_Dl_clist_lock();
    if(thread_manager->getHaltFlag()) {
      UNLOCK_AND_EXIT(listentry);
    } else if(thread_manager->retireThreadByRequest(pthread_self())) {
      UNLOCK_AND_EXIT(listentry);
    }
    if(autostart_flag || true) {
      itemcell = listentry->Get_next_item();
    } else {
      itemcell = NULL;
      autostart_flag = true;
    }
    if(itemcell == NULL) {
      thread_manager->setThreadState(pthread_self(), NULL, THREAD_WAIT);
      if(Is_all_thread_sleeping()) {
	Do_something_on_event(last_event);
      }
      thread_manager->retireThread(pthread_self());
      UNLOCK_AND_EXIT(listentry);
      last_event = ItemCommand::EV_NOEVENT;
    }

    thread_manager->setThreadState(pthread_self(), itemcell, THREAD_ACTIVE);
    listentry->release_Dl_clist_lock();
    ItemStatusSum *itemstatus = new ItemStatusSum(g_consoleItem);
    g_summaryInfo.inc_download();
    Send_report(MSG_SYS_INFO, itemstatus);

    itemcell->reset_Count();
    itemcell->Send_status();

    if(Is_track_required()) {// fixed 2001/5/21
      ItemStatusReq *itemstatus = new ItemStatusReq(itemcell, ItemStatusReq::REQ_MOVEVIEW);
      Send_report(MSG_SYS_REQ, itemstatus);
    }

    //int retry_repeat = 0;
    while(1) {
      // update option values
      if(itemcell->ret_Status() == ItemCell::ITEM_INUSE_CONCAT) {
	itemcell->Send_message_to_gui(_("Concatenating split files..."), MSG_DOWNLOAD_INFO);
	if(itemcell->Concatenate_partial_files()) {
	  retval = ItemCell::DLSUCCESS;
	  itemcell->Send_message_to_gui(_("Concatenation successful"), MSG_DOWNLOAD_SUCCESS);
	  itemcell->set_Size_Current(itemcell->ret_Size_Total());
	} else {
	  retval = ItemCell::DLERRORSTOP;
	  itemcell->Send_message_to_gui(_("Concatenation failed"), MSG_DOWNLOAD_ERROR);
	}
      } else {
	if(itemcell->Is_option_updated()) {
	  // acquire lock
	  itemcell->get_Options_Lock();
	  itemcell->Apply_new_options();
	  // release lock
	  itemcell->release_Options_Lock();
	}
	retval = itemcell->Download_Main();
	//cerr << "in download " << itemcell->ret_Filename() << endl;
      }

      if(retval == ItemCell::DLSUCCESS || retval == ItemCell::DLSUCCESSALR) {
	if(itemcell->Post_process() != ItemCell::DLSUCCESS) {//crc check and etc
	  retval = ItemCell::DLERROR;
	  itemcell->set_Command(ItemCell::DLERROR);// fixed 2001/3/16
	  itemcell->set_Status(ItemCell::ITEM_CRCERROR);
	}
	// recursive download
	if(retval == ItemCell::DLSUCCESS && !itemcell->Is_Partial()) {
	  //cerr << "doc root" << itemcell->ret_documentroot_dir() << endl;
	  //cerr << itemcell->ret_Options().Is_in_activated_parse_target_list(itemcell->ret_URL_Container().ret_Filename()) << endl;
	  //cerr << itemcell->ret_URL_Container().ret_Filename() << endl;
	  if(itemcell->ret_Options().Is_in_activated_parse_target_list(itemcell->ret_URL_Container().ret_Filename()) && !itemcell->ret_documentroot_dir().empty()) {
	    const URLcontainer& urlcon = itemcell->ret_URL_Container();
	    string base_url = urlcon.ret_Protocol()+"//"+urlcon.ret_Hostname()+urlcon.ret_Dir();
	    if(itemcell->ret_documentroot_dir().empty()) {
	      itemcell->set_documentroot_dir(itemcell->ret_Options().ret_Store_Dir());
	    }
	    if(itemcell->ret_root_url().empty()) {
	      itemcell->set_root_url(base_url+'/');
	    }

	    list<ItemCell*> item_list = g_itemList->Recursive_add_http_item(itemcell, listentry); // need some protection for listentry
	    if(item_list.size()) {
	      itemcell->Send_status_recursive(item_list);
	      sleep(2);
	    }
	  }
	}
      }
      // execute program when download finished successfully
      if(!itemcell->Is_Partial() && (retval == ItemCell::DLSUCCESS || retval == ItemCell::DLSUCCESSALR)) {
	if(itemcell->ret_Options_opt().ret_use_Command() &&
	   !itemcell->ret_Options_opt().ret_Command().bad()) {
	  
	  itemcell->Send_status();
	  retval = Execute_command_in_option(itemcell);
	}
      }
      // command list feature
      if(retval == ItemCell::DLSUCCESS || retval == ItemCell::DLSUCCESSALR) {
	if(g_appOption->ret_use_commandlist() && !itemcell->Is_Partial()) {
	  itemcell->Send_status();
	  retval = Execute_command_list_entry(itemcell);
	}
      }

      if(retval == ItemCell::DLSUCCESS || retval == ItemCell::DLSUCCESSALR) {
	last_event = itemcell->ret_Dl_event();
	Download_success(itemcell);
	break;
      } else if(retval == ItemCell::DLPARTIALSUCCESS) {
	//last_event = itemcell->ret_Dl_event();
	last_event = ItemCommand::EV_NOEVENT;
	Download_partial_success(itemcell);
	break;
      } else if(retval == ItemCell::DLERROR) {
	itemcell->get_Options_Lock();
	int retry = itemcell->ret_Options_opt().ret_Retry();
	itemcell->release_Options_Lock();
	if(retry != -1 &&
	   itemcell->ret_Count() >= retry) {
	  last_event = itemcell->ret_Dl_event();
	  Maximum_retry_count_exceeded(itemcell);//, retry_repeat);
	  break;
	} else {
	  if(Download_error(itemcell)) break;
	  last_event = itemcell->ret_Dl_event();
	  itemcell->inc_Count();// modified 2001/3/14
	}
      } else if(retval == ItemCell::DLERRORSTOP) {
	last_event = itemcell->ret_Dl_event();
	Maximum_retry_count_exceeded(itemcell);
	break;
      } else if(retval == ItemCell::DLSTOP) {
	last_event = itemcell->ret_Dl_event();
	Download_stop(itemcell);
	break;
      }	else if(retval == ItemCell::DLDELETEITEM) {
	last_event = itemcell->ret_Dl_event();
	Download_delete_item(itemcell);
	break;
      } else if(retval == ItemCell::DLDELETEITEMFILE) {
	last_event = itemcell->ret_Dl_event();
	Download_delete_item_file(itemcell);
	break;
      } else if(retval == ItemCell::DLINTERNALAGAIN) {
	itemcell->set_Status(ItemCell::ITEM_DOWNLOAD_INTERNAL_AGAIN);
      } else if(retval == ItemCell::DLAGAIN) {
	itemcell->set_Status(ItemCell::ITEM_DOWNLOAD_AGAIN);
      } else if(retval == ItemCell::DLHALT) {
	last_event = itemcell->ret_Dl_event();
	itemcell->Close_Desc();
	break;
      }
    }
  }
  return NULL;
}
