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

// $Id: PasteWindow.h,v 1.12 2001/10/17 13:06:35 tujikawa Exp $

#ifndef _PASTEWINDOW_H_
#define _PASTEWINDOW_H_
#include <gtk/gtk.h>
#include <string>
#include "aria.h"
#include "URLcontainer.h"
#include "Options.h"
#include "ListManager.h"
#include "AppOption.h"
#include "aria_mime.h"
#include "ShortCutKey.h"
#include "gui_utils.h"

using namespace std;

class PasteWindow
{
private:
  GtkWidget *window;
  GtkWidget *paste_list;
  GtkWidget *targetListCombo;
  GtkWidget *autostartToggle;
  pthread_mutex_t listLock;
  bool visibleFlag;
public:
  PasteWindow(GtkWindow *toplevel = NULL);

  GtkWidget *Create_menu_bar(GtkWidget *window);
  void Create_edit_menu(GtkWidget *window, GtkWidget *menu_bar, GtkAccelGroup *accel_group);
  void Create_option_menu(GtkWidget *window, GtkWidget *menu_bar, GtkAccelGroup *accel_group);

  bool addURL(string url_string, int mime_info = MIME_URL);
  bool addURLByNumericalExpansion(string url_string);
  bool addItem(const string& url);
  bool addItem(ItemCell *itemcell);
  void deleteItem();
  void deleteAllItem();
  void paste();
  GtkWidget *getPasteList();
  void selectAll();
  void invertSelection();
  void selectRow(int row, int column, GdkEventButton *event);
  void unselectRow(int row, int column, GdkEventButton *event);

  void show();
  void show(const string& listName);
  void hide();
  void updateTargetList();
  void updateTargetListWithCurrentList();
  void updateAutostart();
  string getTargetListName() const;
  bool isAutostartEnabled() const;
  GtkWidget *getWindow();
  int getSelectedItemNum() const;
  bool isVisible() const;
};
#endif //_PASTEWINDOW_H_
