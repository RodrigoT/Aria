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

// $Id: gui_file.cc,v 1.45 2002/09/30 13:29:46 tujikawa Exp $

#include <fstream.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "aria.h"
#include "ItemCell.h"
#include "ListManager.h"
#include "AppOption.h"
#include "ItemList.h"
#include "Dialog.h"
#include "HistoryWindow.h"
#include "ShortCutKey.h"
#include "gui_utils.h"

#include "pixmaps/url_list.xpm"
#include "pixmaps/open.xpm"
#include "pixmaps/save.xpm"

// external functions
extern gboolean File_find_hyperlink(GtkWidget *w, gpointer data);
extern void Create_find_hyperlink_window(GtkWidget *toplevel);
//// Open URL list
extern gboolean File_open_URL_list(GtkWidget *w, gpointer unused);
//// Open CRC list
extern gboolean File_open_CRC_list(GtkWidget *w, gpointer unused);
//// Open MD5 list
extern gboolean File_open_md5_list(GtkWidget *w, gpointer unused);
//// Open saved list
extern gboolean File_open_Saved_list(GtkWidget *w, gpointer unused);
//// Save list
extern gboolean File_save_list(GtkWidget *w, gpointer unused);
extern bool save_gui_info(const string& filename);
extern gboolean Download_start_all_by_listentry(ListEntry *listentry);
extern gboolean Download_stop_all_list_on_timer(GtkWidget* w, gpointer unused);
extern void Do_something_on_event(ItemCommand::EventCause last_event);
extern bool Is_all_thread_sleeping();

// global variables
extern int g_pipetogui[2];
extern ItemCell *g_consoleItem;
extern ProxyList *g_httpProxyList;
extern ProxyList *g_ftpProxyList;
extern ItemList *g_itemList;
extern AppOption *g_appOption;
extern ListManager *g_listManager;
extern Dialog *g_cDialog;
extern HistoryWindow *g_historyWindow;
// global variables

// windowを隠す
gboolean Hide_window(GtkWidget *window, gpointer unused)
{
  gtk_widget_hide(window);

  return TRUE;
}
  
// Autosave
gboolean Autosave_list(gpointer data)
{
  //pthread_mutex_lock(&itemlistlock); //mod 2001/4/11
  g_itemList->Save_current_list();
  g_itemList->Save_default_item_settings();
  g_itemList->Save_app_settings();
  g_httpProxyList->Save_proxy_list(g_itemList->ret_file_http_proxy_list());
  g_ftpProxyList->Save_proxy_list(g_itemList->ret_file_ftp_proxy_list());
   
  g_historyWindow->writeFile(g_itemList->ret_file_history());
  //pthread_mutex_unlock(&itemlistlock); //mod 2001/4/11
  save_gui_info(g_itemList->ret_file_gui_info());
  g_consoleItem->Send_message_to_gui(_("Autosave done"), MSG_SYS_INFO);
  return TRUE;
}

// Start timer
gboolean Timer_start(gpointer data)
{
  int curTime = time(NULL);
  int diff = g_appOption->ret_timer_start_time()-curTime;
  //int diff = g_appOption->getTimerInterval();
  if(diff <= 60 && diff > 0) {
    g_appOption->Start_start_timer(1000);
  } else if(diff <= 0) {
    if(g_appOption->ret_timer_start_all_list()) {
      for(list<ListEntry*>::const_iterator itr = g_listManager->ret_Listentry_list().begin(); itr != g_listManager->ret_Listentry_list().end(); ++itr) {
	ListEntry *listentry = *itr;
	listentry->get_Dl_clist_lock();
	listentry->freezeDlCList();
	Download_start_all_by_listentry(listentry);
	listentry->thawDlCList();
	listentry->release_Dl_clist_lock();
      }
      //Download_start_all_list(NULL, NULL);
    } else {
      ListEntry *listentry = g_listManager->ret_Current_listentry();
      listentry->get_Dl_clist_lock();
      listentry->freezeDlCList();
      Download_start_all_by_listentry(listentry);
      listentry->thawDlCList();
      listentry->release_Dl_clist_lock();
      //Download_start_all(NULL, NULL);
    }
    g_appOption->Update_timer_start();
    g_appOption->Start_start_timer(60000);
  }
  return TRUE;
}

// Stop timer
gboolean Timer_stop(gpointer data)
{
  int curTime = time(NULL);
  int diff = g_appOption->ret_timer_stop_time()-curTime;
  //int diff = g_appOption->getTimerInterval();

  if(diff <= 60 && diff > 0) {
    g_appOption->Start_stop_timer(1000);
  } else if(diff <= 0) {
    if(g_appOption->isNoStopDownloadOnTimerEnabled()
       || Is_all_thread_sleeping()) {
      Do_something_on_event(ItemCommand::EV_TIMERINTER);
    } else {
      Download_stop_all_list_on_timer(NULL, NULL);
    }
    g_appOption->Update_timer_stop();
    g_appOption->Start_stop_timer(60000);
  }
  return TRUE;
}

// プログラム終了メッセージを各スレッドに通達
// Broadcast halt message to all threads
void Send_halt_message()
{
  for(list<ListEntry*>::const_iterator itr = g_listManager->ret_Listentry_list().begin();
      itr != g_listManager->ret_Listentry_list().end(); ++itr) {
    ListEntry *listEntry = *itr;

    listEntry->get_Dl_clist_lock();
    listEntry->getThreadManager()->setHaltFlag();
    for(int rowindex = 0; rowindex < GTK_CLIST(listEntry->ret_Dl_clist())->rows; ++rowindex) {
      ItemCell *itemCell = (ItemCell *)gtk_clist_get_row_data(GTK_CLIST(listEntry->ret_Dl_clist()), rowindex);
      switch(itemCell->ret_Status()) {
      case ItemCell::ITEM_DOWNLOAD:
      case ItemCell::ITEM_INUSE:
      case ItemCell::ITEM_DOWNLOAD_AGAIN:
      case ItemCell::ITEM_DOWNLOAD_INTERNAL_AGAIN:
      case ItemCell::ITEM_INUSE_AGAIN:
	{
	  ItemCommand itemCommand;
	  itemCommand.commandtype = ItemCommand::COMMAND_HALT;
	  //itemcell->set_Status(ItemCell::ITEM_READY);
	  itemCell->set_Status(ItemCell::ITEM_STOP);
	  write(itemCell->ret_Desc_w(), &itemCommand, sizeof(ItemCommand));
	}
	break;
	/*
      case ItemCell::ITEM_DOWNLOAD_PARTIAL:
      case ItemCell::ITEM_READY_CONCAT:
      case ItemCell::ITEM_READY_AGAIN:
	//itemcell->set_Status(ItemCell::ITEM_READY);
	itemcell->set_Status(ItemCell::ITEM_STOP);
	break;
	*/
      case ItemCell::ITEM_COMPLETE:
      case ItemCell::ITEM_LOCK:
      case ItemCell::ITEM_ERROR:
	break;
      default:
	itemCell->set_Status(ItemCell::ITEM_STOP);// added 2001/5/17
	break;
      }
    }
    listEntry->Send_start_signal();
    listEntry->release_Dl_clist_lock();
    //pthread_mutex_unlock(&itemlistlock);
    //listentry->release_Dl_clist_lock();
  }
  for(list<ListEntry*>::const_iterator itr = g_listManager->ret_Listentry_list().begin();
      itr != g_listManager->ret_Listentry_list().end(); ++itr) {
    ListEntry *listEntry = *itr;
    listEntry->getThreadManager()->waitThreadTermination();
  }
}

static void Save_default_settings()
{
  g_itemList->Save_default_item_settings();
}

static void Save_app_settings()
{
  g_itemList->Save_app_settings();

}

static void Save_proxy_list()
{
  g_httpProxyList->Save_proxy_list(g_itemList->ret_file_http_proxy_list());
  g_ftpProxyList->Save_proxy_list(g_itemList->ret_file_ftp_proxy_list());
}

static void Save_history()
{
  g_historyWindow->writeFile(g_itemList->ret_file_history());
}

static void Save_gui_info()
{
  save_gui_info(g_itemList->ret_file_gui_info());
}

void Save_files()
{
  Save_default_settings();
  Save_app_settings();
  Save_proxy_list();
  Save_history();
  Save_gui_info();
  // save current list and its crc
  //pthread_mutex_lock(&itemlistlock);
  g_itemList->Save_current_list();
  //pthread_mutex_unlock(&itemlistlock);
}

static void* doExitSequence1()
{
  g_appOption->Update_timer_start();
  g_appOption->Start_start_timer(600000);

  Send_halt_message();  
  Save_default_settings();
  Save_app_settings();
  Save_proxy_list();
  Save_history();
  Save_gui_info();

  close(g_pipetogui[0]);
  close(g_pipetogui[1]);

  exit(0);
}

static void* doExitSequence2()
{
  g_appOption->Update_timer_stop();
  g_appOption->Start_stop_timer(600000);

  Send_halt_message();  
  Save_files();

  close(g_pipetogui[0]);
  close(g_pipetogui[1]);

  exit(0);
}

// 現在のリストを保存せず終了
gboolean File_quit_wos(GtkWidget *w, GtkWidget *window)
{
  g_cDialog->hide();

  pthread_t thread;
  pthread_create(&thread, (pthread_attr_t*)NULL,
		 (void*(*)(void*))doExitSequence1(),
		 (void*)NULL);
  //exit(0);

  return TRUE;
}

// 現在のリストを保存して終了
gboolean File_quit(GtkWidget *w, GtkWidget *unused)
{
  g_cDialog->hide();
  /*
  Send_halt_message();

  Save_files();
  */
  pthread_t thread;
  pthread_create(&thread, (pthread_attr_t*)NULL,
		 (void*(*)(void*))doExitSequence2(),
		 (void*)NULL);

  /*
  close(g_pipetogui[0]);
  close(g_pipetogui[1]);
  */
  //exit(0);

  return TRUE;
}

gboolean File_quit_c(GtkWidget *w, gboolean (*SignalFunc)(GtkWidget *x, GtkWidget *window))
{
  if(g_appOption->ret_confirm_exit()) {
    g_cDialog->setup(_("Quit program"),
		   _("Are you sure to quit?"),
		   SignalFunc);
    g_cDialog->set_cancel_button_visible(false);
    g_cDialog->show();
  } else {
    (*SignalFunc)(NULL, NULL);
  }
  return TRUE;
}

/*
gboolean File_signal_func(GtkWidget* w, gboolean (*Signal_Func)())
{
  (*Signal_Func)();
  return TRUE;
}
*/

// ファイルメニューを作成
void Create_file_menu(GtkWidget* topLevel, GtkWidget* menuBar, GtkAccelGroup* accelGroup)
{
  GtkWidget *menu, *rootItem;
  GtkWidget *urlOpenItem, *crcOpenItem;
  GtkWidget *md5OpenItem;
  GtkWidget *findHyperlinkItem;
  GtkWidget *openSavedListItem, *saveListItem;
  GtkWidget *quitItem, *quitWosItem;

      //create file menu
  menu = gtk_menu_new();

  urlOpenItem = GTK_create_menu_item_with_icon(menu,
					       _("Open URL list"),
					       GTK_SIGNAL_FUNC(File_open_URL_list),
					       NULL,
					       url_list_xpm,
					       topLevel,
					       accelGroup,
					       SC_OPENURL,
					       SCM_OPENURL);

  crcOpenItem = GTK_create_menu_item_with_icon(menu,
				     _("Open CRC list"),
				     GTK_SIGNAL_FUNC(File_open_CRC_list),
				     NULL,
				     accelGroup,
				     SC_OPENCRC,
				     SCM_OPENCRC);

  md5OpenItem = GTK_create_menu_item_with_icon(menu,
				     _("Open MD5 list"),
				     GTK_SIGNAL_FUNC(File_open_md5_list),
				     NULL);
  /*
				     accelGroup,
				     SC_OPENCRC,
				     SCM_OPENCRC);
  */

  GTK_create_menu_separator(menu);

  findHyperlinkItem = GTK_create_menu_item_with_icon(menu,
					   _("Find hyperlink"),
					   GTK_SIGNAL_FUNC(File_find_hyperlink),
					   NULL,
					   accelGroup,
					   SC_FINDHREF,
					   SCM_FINDHREF);
  
  Create_find_hyperlink_window(topLevel);
  GTK_create_menu_separator(menu);

  openSavedListItem = GTK_create_menu_item_with_icon(menu,
						     _("Open saved list"),
						     GTK_SIGNAL_FUNC(File_open_Saved_list),
						     NULL,
						     open_xpm,
						     topLevel,
						     accelGroup,
						     SC_OPEN,
						     SCM_OPEN);

  saveListItem = GTK_create_menu_item_with_icon(menu,
						_("Save current list"),
						GTK_SIGNAL_FUNC(File_save_list),
						NULL,
						save_xpm,
						topLevel,
						accelGroup,
						SC_SAVE,
						SCM_SAVE);

  GTK_create_menu_separator(menu);

  quitItem = GTK_create_menu_item_with_icon(menu,
				  _("Quit with saving lists"),
				  GTK_SIGNAL_FUNC(File_quit_c),
				  (void*)File_quit,
				  accelGroup,
				  SC_QUIT,
				  SCM_QUIT);

  quitWosItem = GTK_create_menu_item_with_icon(menu,
				     _("Quit without saving lists"),
				     GTK_SIGNAL_FUNC(File_quit_c),
				     (void*)File_quit_wos);


  rootItem = gtk_menu_item_new_with_label(_("File"));
  gtk_widget_show(rootItem);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(rootItem), menu);
  gtk_menu_bar_append(GTK_MENU_BAR(menuBar), rootItem);
}

  //gtk_toolips_set_tip(tooltips, menuitem, szTip, NULL);
  //GTK_TOGGLE_BUTTON(widget)->active;
  //GTK_CHECK_MENU_ITEM(widget)->acitve;
  //toolbar = gtk_toolbar_new(GTK_ORIENTAION_HORIZONTAOL,
  //  GTK_TOOLBAR_ICONS);
  //gtk_widget_show(toolbar);
  //gtk_toolbar_append_item(GTK_TOOLBAR(toolbar),
  //NULL, "New windw", NULL,
  //CreateWidgetFromXpm(vbox_main, (gchar **)xpm_new),
  //(GtkSIgnalFunc)ButtonClicked,
  //NULL);
