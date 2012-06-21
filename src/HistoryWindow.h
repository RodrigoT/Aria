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

// $Id: HistoryWindow.h,v 1.7 2001/09/11 13:13:31 tujikawa Exp $

#ifndef _HISTORYWINDOW_H_
#define _HISTORYWINDOW_H_
#include <gtk/gtk.h>
#include <string>
#include <fstream>
#include "aria.h"
#include "URLcontainer.h"
#include "ItemManager.h"
#include "PasteWindow.h"
#include "ShortCutKey.h"
#include "gui_utils.h"

using namespace std;

class HistoryWindow
{
private:
  GtkWidget *window;
  GtkWidget *historyList;
  GtkWidget *queryEntry;
  GtkWidget *searchButton;

  int maxHistory;
  pthread_mutex_t historyLock;

  GtkWidget *Create_menu_bar(GtkWidget *window);
  GtkWidget *createQueryUI(GtkWidget *window);
  void Create_edit_menu(GtkWidget *window, GtkWidget *menu_bar, GtkAccelGroup *accel_group);
public:
  HistoryWindow(GtkWindow *toplevel = NULL, int maxHistory = DEFAULTHISTORY);

  bool addItem(ItemCell *itemcell);
  bool addItem(const string& file,
		const string& size,
		time_t time_log,
		const string& url,
		const string& save);
  void selectRow(int row, int column, GdkEventButton *event);
  void unselectRow(int row, int column, GdkEventButton *event);
  
  void selectAll();
  void invertSelection();

  bool readFile(const string& filename);
  bool writeFile(const string& filename);
  void setHistoryMax(int max);
  void fitHistory();
  void show();
  void hide();
  void deleteEntry();
  void pasteEntry();
  void search();
  GtkWidget *getWindow();
  GtkWidget *getHistoryList();

  enum {
    COL_FILE_H = 0,
    COL_SIZE_H = 1,
    COL_DATE_H = 2,
    COL_URL_H = 3,
    COL_SAVE_H = 4,
    COL_TOTAL_H = 5
  };
};
#endif //_HISTORYWINDOW_H_
