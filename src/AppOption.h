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

// $Id: AppOption.h,v 1.31 2002/01/20 14:41:58 tujikawa Exp $

#ifndef _APPOPTION_H_
#define _APPOPTION_H_
#include <iostream>
#include <stdio.h>
#include <gtk/gtk.h>
#include <sys/types.h>
#include <dirent.h>
#include "aria.h"
#include "ThreadManager.h"
#include "ItemManager.h"
#include "Dialog.h"
#include "CommandList.h"
#include "ItemList.h"
#include "TimerData.h"
#include "Basket.h"
#include "gui_utils.h"

using namespace std;

class AppOption {
private:
  GtkWidget *notebook;
  GtkWidget *option_window;

  pthread_mutex_t option_lock;

  // maximum number of thread
  int maxthread;
  GtkWidget *spin_thread;

  bool use_automatic_start;
  GtkWidget *autoStartToggle;
  bool autostartModCurrentListOnly;
  GtkWidget *autostartModCurrentListOnly_toggle;

  bool use_ignore_error_item;
  GtkWidget *ignore_error_item_toggle;
  GtkWidget *ignore_extension_toggle;
  GtkWidget *ignore_extension_entry;
  list<string> ignore_extension_list;
  bool use_ignore_extension_list;
  GtkWidget *confirm_clear_toggle;
  GtkWidget *confirm_exit_toggle;
  GtkWidget *confirm_clearlog_toggle;
  GtkWidget *confirm_delete_list_toggle;
  bool confirm_clear;
  bool confirm_exit;
  bool confirm_clearlog;
  bool confirm_delete_list;
  GtkWidget *use_servertemplate_toggle;
  bool use_servertemplate;
  GtkWidget *svt_clist;

  GtkWidget *use_commandlist_toggle;
  bool use_commandlist;
  GtkWidget *com_clist;

  bool use_size_human_readable;
  GtkWidget *use_size_human_readable_toggle;

  // force download now (stopping other downloads if necessary)
  bool useForceDownloadNow;
  GtkWidget *useForceDownloadNowToggle;

  // icon set
  string statusIconDir;
  GtkWidget *statusIconDirList;
  GtkWidget *statusIconPreviewList;

  GtkWidget *statusIconApplyButton;
  GdkPixbuf *statusIcon[ICON_TOTAL];
  GdkBitmap *statusIconMask[ICON_TOTAL];

  // pixmap for DND basket
  string basketPixmapFile;
  GtkWidget *basketPixmapFileList;  // clist for file browse
  GtkWidget *basketPixmapPreview; // GtkPixmap for preview
  GtkWidget *basketPixmapPreviewVBox; // GtkVBox that contains basketPixmapPreview
  GtkWidget *basketPixmapApplyButton; // "Apply" button
  GdkPixbuf *basketPixmapTemp;
  GdkBitmap *basketBitmapTemp;

  bool directPastingFromBasket;
  GtkWidget *directPastingFromBasketToggle;

  // autosave
  bool use_autosave;
  int autosave_interval;
  int autosave_tag;
  GtkWidget *autosave_spin;
  GtkWidget *autosave_toggle;

  // history list
  int history_limit;
  GtkWidget *history_limit_spin;

  // max value of speed limiter
  int speedLimit;
  GtkWidget *speedLimitSpin;

  // Track download
  bool use_track_download;

  // run command when all downloads are over
  bool use_arb_command;
  bool use_arb_command_timer;
  string arb_command;
  GtkWidget *arb_command_entry;
  GtkWidget *use_arb_command_toggle;
  GtkWidget *use_arb_command_timer_toggle;

  // quit Aria when all downloads are over
  bool use_quit_program;
  GtkWidget *use_quit_program_toggle;

  // simple timer
  bool startTimerEnabled;
  bool stopTimerEnabled;
  int timer_start_tag;
  int timer_stop_tag;
  bool timer_start_all_list;
  GtkWidget *timer_start_all_list_toggle;
  TimerData timerdata;
  GtkWidget *timer_hour_start_spin;
  GtkWidget *timer_min_start_spin;
  GtkWidget *timer_hour_stop_spin;
  GtkWidget *timer_min_stop_spin;
  GtkWidget *startTimerEnabledToggle;
  GtkWidget *stopTimerEnabledToggle;
  GtkWidget *iterate_timer_toggle;
  bool noStopDownloadOnTimer;
  GtkWidget *noStopDownloadOnTimerToggle;

  void setDirEntryToCList(GtkCList *clist, const string& dirPath);
public:
  AppOption(GtkWidget *app_window);

  void Set_Option_Values(int maxthread_in,
			 bool use_automatic_start,
			 bool autostartModCurrentListOnly,
			 bool useForceDownloadNow,
			 bool use_ignore_error_item_in,
			 bool use_ignore_extension_list,
			 const list<string>& ignore_extension_list_in,
			 bool use_autosave,
			 int autosave_interval,
			 int history_limit,
			 int speedLimit,
			 bool use_arb_command,
			 bool use_arb_command_timer,
			 const string& arb_command,
			 bool use_quit_program,
			 bool startTimerEnabled,// modified on 2002/01/20
			 bool endTimerEnabled,
			 bool timer_start_all_list,
			 bool noStopDownloadOnTimer,
			 const TimerData& timerdata,
			 bool confirm_clear,
			 bool confirm_delete_list,
			 bool confirm_exit,
			 bool confirm_clearlog,
			 bool use_servertemplate,
			 bool use_commandlist,
			 bool use_size_human_readable,
			 const string& statusIconDir,
			 const string& basketPixmapFile,
			 bool directPastingFromBasket,
			 const list<string>& svt_names,
			 const list<string>& command_names);
  void       Show_option_window();
  void updateStatusIconDirList();
  void updateBasketPixmapFileList();

  GtkWidget *ret_Option_Window() const;
  void       Process_changes();
  GtkWidget *Create_COMMANDLIST_page();
  GtkWidget *Create_SERVERTEMPLATE_page();
  GtkWidget *Create_DOWNLOAD_page();
  GtkWidget *Create_CONFIRMATION_page();
  GtkWidget *Create_DISPLAY_page();
  GtkWidget *Create_BASKET_page();
  GtkWidget *Create_TIMER_page();

  int        getThreadMax() const;
  void       setThreadMax(int maxthread_in);
  bool       Increase_thread();
  bool       Decrease_thread();
  bool       Whether_use_automatic_start();
  bool       isAutostartCurrentListOnly();
  bool       ret_use_ignore_error_item();
  bool       Is_in_ignore_extension_list(const string& file);
  const list<string>& ret_ignore_extension_list();
  bool       ret_use_ignore_extension_list();
  bool       ret_confirm_clear();
  bool       ret_confirm_delete_list();
  bool       ret_confirm_exit();
  bool       ret_confirm_clearlog();
  bool       ret_use_servertemplate();
  bool       ret_use_commandlist();
  bool       ret_use_size_human_readable();
  bool       ret_use_autosave();
  int        getAutosaveInterval();
  int        ret_autosave_tag();
  int        ret_history_limit();
  int getSpeedLimitMax();
  list<string> ret_name_list(GtkWidget *clist);
  list<string> ret_svt_name_list();
  list<string> ret_com_name_list();
  void set_svt_clist(const list<string>& names);
  void set_com_clist(const list<string>& names);
  
  void setStatusIconDir(const string& icon_dir);

  string getStatusIconDir();
  bool ret_use_arb_command();
  bool ret_use_arb_command_timer();
  string ret_arb_command();
  bool ret_use_quit_program();

  // timer related methods
  bool isStartTimerEnabled();
  bool isStopTimerEnabled();
  bool ret_timer_start_all_list();
  bool ret_iterate_timer();
  time_t ret_timer_start_time();
  time_t ret_timer_stop_time();
  void Start_start_timer(int check_interval);
  void Start_stop_timer(int check_interval);
  void Update_timer_start();
  void Update_timer_stop();
  int ret_timer_hour_start();
  int ret_timer_min_start();
  int ret_timer_hour_stop();
  int ret_timer_min_stop();

  // get interval between start time and stop time of the timer.
  int getTimerInterval();

  bool isNoStopDownloadOnTimerEnabled();
  bool isForceDownloadNowEnabled();

  // Button callbacks
  void optionWindowOkButton_clicked();
  void optionWindowCancelButton_clicked();
  void optionWindowDefaultButton_clicked();

  // auto start
  void autoStartToggle_toggled();

  // Command
  void updateCommandListButton_clicked();
  
  // Server
  void updateServerTemplateButton_clicked();

  // DND basket related methods
  string getBasketPixmapFile();
  bool createBasketPixmapFromFile(const string& filename);
  void setBasketPixmapPreview();
  void basketPixmapFileList_selectRow(int row, int column, GdkEventButton *event);
  void basketPixmapFileList_unselectRow(int row, int column, GdkEventButton *event);
  void basketPixmapApplyButton_clicked();
  void showBasketPixmapError();
  void setBasketPixmapApplyButtonEnabled(bool toggle);
  bool isDirectPastingFromBasketEnabled();

  // status icon related methods
  void createStatusIconDefault();
  bool createStatusIcon(const string& dirname);
  void setStatusIconPreview();
  void statusIconDirList_selectRow(int row, int column, GdkEventButton *event);
  void statusIconDirList_unselectRow(int row, int column, GdkEventButton *event);
  void statusIconApplyButton_clicked();
  void showStatusIconError();
  void setStatusIconApplyButtonEnabled(bool toggle);
};

#endif // _APPOPTION_H_
