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

// $Id: HistoryWindow.cc,v 1.27 2002/10/01 15:32:00 tujikawa Exp $

#include "HistoryWindow.h"
#include "ItemCell.h"

extern PasteWindow *g_pasteWindow;

static GtkWidget *selectAllItem;
static GtkWidget *invertSelectionItem;

// history_windowのOKボタンをおすと呼び出される
static void History_pasteEntry(GtkWidget *w, HistoryWindow *hw)
{
  hw->pasteEntry();
}

void HistoryWindow::pasteEntry() {
  string line;

  if(GTK_CLIST(historyList)->rows == 0) return;

  pthread_mutex_lock(&historyLock);
  // add to paste list
  bool flag = false;
  GList *node = GTK_CLIST(historyList)->selection;
  while(node) {
    int rowindex = GPOINTER_TO_UINT(node->data);
    char *url_cstr;
    gtk_clist_get_text(GTK_CLIST(historyList), rowindex, HistoryWindow::COL_URL_H, &url_cstr);
    if(g_pasteWindow->addItem(url_cstr)) flag = true;
    node = g_list_next(node);
  }
  pthread_mutex_unlock(&historyLock);
  if(flag) g_pasteWindow->show();
}


// history window上のdeleteボタンが押されたときの処理。
// CLIST additem_list上で選択されているアイテムがあればそれを削除する。
static gboolean History_deleteEntry(GtkWidget *w, HistoryWindow *hw)
{
  hw->deleteEntry();

  return TRUE;
}

void HistoryWindow::deleteEntry() {
  pthread_mutex_lock(&historyLock);
  GList *node = GTK_CLIST(historyList)->selection;
  while(node) {
    unsigned int rowindex = GPOINTER_TO_UINT(node->data);
    time_t *time_ptr = (time_t *)gtk_clist_get_row_data(GTK_CLIST(historyList), rowindex);
    delete time_ptr;
    gtk_clist_remove(GTK_CLIST(historyList), rowindex);
    node = GTK_CLIST(historyList)->selection;
  }
  pthread_mutex_unlock(&historyLock);
}

// history_windowを隠す
static gboolean History_close(HistoryWindow *hw)
{
  hw->hide();
  
  return TRUE;
}

static void HistoryWindow_selectRow_cb(GtkWidget *clist,
				       int row,
				       int column,
				       GdkEventButton *event,
				       HistoryWindow *hw) {
  static bool enterflag = false;
  if(enterflag == true || event == NULL) return;
  enterflag = true;

  hw->selectRow(row, column, event);

  enterflag = false;
}

void HistoryWindow::selectRow(int row, int column, GdkEventButton *event) {
  pthread_mutex_lock(&historyLock);
  switch(event->button) {
  case 1:
  case 3:
    if(event->state & GDK_CONTROL_MASK) {
      // do nothing, just select the row
    } else if(event->state & GDK_SHIFT_MASK) {
      int nearestSelectedRow = findNearestSelectedRow(historyList, row);
      if(nearestSelectedRow < row) {
	for(int index = nearestSelectedRow+1; index < row; ++index) {
	  gtk_clist_select_row(GTK_CLIST(historyList), index, 0);
	}
      } else {
	for(int index = row+1; index < nearestSelectedRow; ++index) {
	  gtk_clist_select_row(GTK_CLIST(historyList), index, 0);
	}
      }
    } else {// with no mask
      gtk_clist_unselect_all(GTK_CLIST(historyList));
      gtk_clist_select_row(GTK_CLIST(historyList), row, 0);
    }
    break;
  }
  pthread_mutex_unlock(&historyLock);
}

static void HistoryWindow_unselectRow_cb(GtkWidget *cist,
					 int row,
					 int column,
					 GdkEventButton *event,
					 HistoryWindow *hw) {
  static bool enterflag = false;
  if(enterflag == true || event == NULL) return;
  enterflag = true;

  hw->unselectRow(row, column, event);

  enterflag = false;
}

void HistoryWindow::unselectRow(int row, int column, GdkEventButton *event) {
  pthread_mutex_lock(&historyLock);

  switch(event->button) {
  case 1:
  case 3:
    if(event->state & (GDK_CONTROL_MASK | GDK_SHIFT_MASK)) {
      GList *node = GTK_CLIST(historyList)->selection;
      if(node == NULL) {
	// do nothing, just unselect the row
      } else {
	gtk_clist_unselect_row(GTK_CLIST(historyList), row, 0);
      }
    } else {
      GList *node = GTK_CLIST(historyList)->selection;
      if(node != NULL) {
	gtk_clist_unselect_all(GTK_CLIST(historyList));
	gtk_clist_select_row(GTK_CLIST(historyList), row, 0);
      } else {
	// do nothing, just unselect the row
      }
    }
    break;
  }

  pthread_mutex_unlock(&historyLock);
}

// select all item in download list
static void Edit_select_all(GtkWidget *w, HistoryWindow *hw)
{
  hw->selectAll();
}

void HistoryWindow::selectAll() {
  gtk_clist_select_all(GTK_CLIST(historyList));
}

// invert selection
static void Edit_invert_selection(GtkWidget *w, HistoryWindow *hw)
{
  hw->invertSelection();
}

void HistoryWindow::invertSelection() {
  gtk_clist_freeze(GTK_CLIST(historyList));
  GList *selection = g_list_copy(GTK_CLIST(historyList)->selection);

  // select all, then unselect row that is in node list
  gtk_clist_select_all(GTK_CLIST(historyList));
  GList *node = selection;
  while(node) {
    int rowindex = GPOINTER_TO_INT(node->data);
    gtk_clist_unselect_row(GTK_CLIST(historyList), rowindex, 0);
    node = g_list_next(node);
  }
  g_list_free(selection);
  gtk_clist_thaw(GTK_CLIST(historyList));

  return;
}

void HistoryWindow::Create_edit_menu(GtkWidget *window, GtkWidget *menu_bar, GtkAccelGroup *accel_group) {
  GtkWidget *menu;
  GtkWidget *root_item;

  menu = gtk_menu_new();

  // Select all items
  selectAllItem = GTK_create_menu_item_with_icon(menu,
						 _("Select all"),
						 GTK_SIGNAL_FUNC(Edit_select_all),
						 (GtkObject *)this,
						 accel_group,
						 SC_SELECT_ALL,
						 SCM_SELECT_ALL);
  
  // Invert selection
  invertSelectionItem = GTK_create_menu_item_with_icon(menu,
						       _("Invert selection"),
						       GTK_SIGNAL_FUNC(Edit_invert_selection),
						       (GtkObject *)this
						       );
  
  // Create root menu item
  root_item = gtk_menu_item_new_with_label(_("Edit"));
  gtk_widget_show(root_item);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(root_item), menu);
  gtk_menu_bar_append(GTK_MENU_BAR(menu_bar), root_item);
}

GtkWidget *HistoryWindow::Create_menu_bar(GtkWidget *window)
{
  GtkAccelGroup *accel_group;

  GtkWidget *menu_bar = gtk_menu_bar_new();
//  gtk_menu_bar_set_shadow_type(GTK_MENU_BAR(menu_bar), GTK_SHADOW_ETCHED_OUT);
  gtk_widget_show(menu_bar);

  accel_group = gtk_accel_group_new();
  gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);

  //Create_file_menu(window, menu_bar, accel_group);
  Create_edit_menu(window, menu_bar, accel_group);
  //Create_download_menu(menu_bar, accel_group);
  //Create_option_menu(window, menu_bar, accel_group);
  //Create_help_menu(window, menu_bar, accel_group);
  return menu_bar;
}

HistoryWindow::HistoryWindow(GtkWindow *toplevel, int maxHistory_in)
{
  char *title[HistoryWindow::COL_TOTAL_H];

  pthread_mutex_init(&historyLock, NULL);
  //URL貼付け用のダイアログの作成
  window = gtk_dialog_new();
  gtk_window_set_title(GTK_WINDOW(window), _("History URL list"));
  gtk_widget_set_usize(GTK_WIDGET(window), 680, 300);
  gtk_signal_connect_object(GTK_OBJECT(window),
			    "delete_event",
			    GTK_SIGNAL_FUNC(History_close),
			    (GtkObject *)this);

  // CList ウィジェットをパックするスクロールドウィンドウを作成する
  GtkWidget *scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
				 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  
  gtk_widget_show(scrolled_window);  
  
  // CList を作成する。この例では 2 列を使う
  title[HistoryWindow::COL_FILE_H] = _("File");
  title[HistoryWindow::COL_SIZE_H] = _("Total");
  title[HistoryWindow::COL_DATE_H] = _("Date");
  title[HistoryWindow::COL_URL_H] = _("URL");
  title[HistoryWindow::COL_SAVE_H] = _("Save");
  historyList = gtk_clist_new_with_titles(COL_TOTAL_H, title);
  gtk_widget_set_name(historyList, "clist");

  gtk_object_set_user_data(GTK_OBJECT(window), historyList);
  gtk_object_set_user_data(GTK_OBJECT(historyList), &historyLock);
  gtk_clist_set_column_auto_resize(GTK_CLIST(historyList),
				   COL_URL_H,
				   TRUE);
  gtk_clist_set_selection_mode(GTK_CLIST(historyList), GTK_SELECTION_MULTIPLE);
  
  gtk_signal_connect(GTK_OBJECT(historyList), "select-row",
		     GTK_SIGNAL_FUNC(HistoryWindow_selectRow_cb),
		     (void *)this);
  gtk_signal_connect(GTK_OBJECT(historyList), "unselect-row",
		     GTK_SIGNAL_FUNC(HistoryWindow_unselectRow_cb),
		     (void *)this);
  // 境界に影を付ける必要などないが、そうすれば見栄えが良くなる :)
  gtk_clist_set_shadow_type(GTK_CLIST(historyList), GTK_SHADOW_ETCHED_OUT);
  gtk_clist_set_column_width(GTK_CLIST(historyList), COL_FILE_H, 300);
  gtk_clist_set_column_width(GTK_CLIST(historyList), COL_SIZE_H, 100);
  gtk_clist_set_column_width(GTK_CLIST(historyList), COL_DATE_H, 180);

  //modified 2001/3/19
  //gtk_clist_set_column_width(GTK_CLIST(historyList), HistoryWindow::COL_URL_H, 400);
  gtk_clist_set_column_auto_resize(GTK_CLIST(historyList),
				   COL_URL_H, TRUE);
  //modified 2001/3/19
  //gtk_clist_set_column_width(GTK_CLIST(historyList), HistoryWindow::COL_SAVE_H, 400);
  gtk_clist_set_column_auto_resize(GTK_CLIST(historyList),
				   COL_SAVE_H, TRUE);

  gtk_clist_set_column_justification(GTK_CLIST(historyList), COL_SIZE_H, GTK_JUSTIFY_RIGHT);
  //gtk_clist_set_column_justification(GTK_CLIST(historyList), HistoryWindow::COL_DATE_H, GTK_JUSTIFY_RIGHT);


  // CList ウィジェットを垂直ボックスに加え、それを表示する
  gtk_container_add(GTK_CONTAINER(scrolled_window), historyList);
  gtk_widget_show(historyList);
  gtk_clist_column_titles_passive(GTK_CLIST(historyList));

  // create menu bar
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(window)->vbox), Create_menu_bar(window), FALSE, TRUE, 0);

  // create user interface of query
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(window)->vbox), createQueryUI(window), FALSE, FALSE, 0);

  // attach the scrolled window to the history window
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(window)->vbox),
		     scrolled_window, TRUE, TRUE, 0);

  //// action area
  GtkWidget *vbox = gtk_vbox_new(FALSE, 5);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(window)->action_area),
		     vbox, FALSE, FALSE, 0);
  gtk_widget_show(vbox);

  {
    GtkWidget *bbox = gtk_hbutton_box_new();
    gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_SPREAD);
    gtk_button_box_set_spacing(GTK_BUTTON_BOX(bbox), 5);
    gtk_box_pack_start(GTK_BOX(vbox),
		       bbox, FALSE, FALSE, 0);
    gtk_widget_show(bbox);
    
    // Add button
    GtkWidget *Add_button = gtk_button_new_with_label(_("Paste"));
    gtk_object_set_user_data(GTK_OBJECT(Add_button), window);
    gtk_signal_connect(GTK_OBJECT(Add_button),
		       "clicked",
		       GTK_SIGNAL_FUNC(History_pasteEntry),
		       (void *)this);
    gtk_box_pack_start(GTK_BOX(bbox),
		       Add_button, TRUE, TRUE, 0);
    gtk_widget_show(Add_button);
    
    // Delete button
    GtkWidget *Delete_button = gtk_button_new_with_label(_("Clear"));
    gtk_signal_connect(GTK_OBJECT(Delete_button),
		       "clicked",
		       GTK_SIGNAL_FUNC(History_deleteEntry),
		       (void *)this);
    gtk_box_pack_start(GTK_BOX(bbox),
		       Delete_button, TRUE, TRUE, 0);
    gtk_widget_show(Delete_button);

    // Close button
    GtkWidget *Close_button = gtk_button_new_with_label(_("Close"));
    gtk_signal_connect_object(GTK_OBJECT(Close_button),
			      "clicked",
			      GTK_SIGNAL_FUNC(History_close),
			      (GtkObject *)this);
    gtk_box_pack_start(GTK_BOX(bbox),
		       Close_button, TRUE, TRUE, 0);
    gtk_widget_show(Close_button);
  }

  //// window property
  gtk_window_set_modal(GTK_WINDOW(window), FALSE);

  // set max of history entry
  setHistoryMax(maxHistory_in);
  // fixed
  //if(toplevel != NULL)
  //  gtk_window_set_transient_for(GTK_WINDOW(window),
  //				 GTK_WINDOW(toplevel));
}

bool HistoryWindow::addItem(ItemCell *itemcell)
{
  time_t time_log = time(NULL);
  return addItem(itemcell->ret_Filename(),
		  itos(itemcell->ret_Size_Current()),
		  time_log,
		  itemcell->ret_URL(),
		  itemcell->ret_Options().ret_Store_Dir());
}

bool HistoryWindow::addItem(const string& file,
			     const string& size,
			     time_t time_log,
			     const string& url,
			     const string& save)
{
  char *clist_item[HistoryWindow::COL_TOTAL_H];
  if(file.empty()) {
    clist_item[HistoryWindow::COL_FILE_H] = strdup(_("<directory>"));
  } else {
    clist_item[HistoryWindow::COL_FILE_H] = strdup(file.c_str());
  }
  clist_item[HistoryWindow::COL_URL_H] = strdup(url.c_str());
  clist_item[HistoryWindow::COL_SAVE_H] = strdup(save.c_str());
  if(size.size() > 3 && size.find(',') == string::npos) {
    clist_item[HistoryWindow::COL_SIZE_H] = strdup(insert_comma(size).c_str());
  } else {
    clist_item[HistoryWindow::COL_SIZE_H] = strdup(size.c_str());
  }
  //struct tm *local_tm = localtime(&time_log);

  string time_str = ctime(&time_log);
  time_str.at(time_str.size()-1) = '\0';
  clist_item[HistoryWindow::COL_DATE_H] = strdup(time_str.c_str());
  //printf("%p\n", ctime(&time_log));
  time_t* time_ptr = new time_t;
  *time_ptr = time_log;

  pthread_mutex_lock(&historyLock);
  bool vadj_flag = false;
  float vadj = 0.0;
  GtkAdjustment *adj = NULL;
  if(gtk_clist_row_is_visible(GTK_CLIST(historyList), GTK_CLIST(historyList)->rows-1) == GTK_VISIBILITY_NONE) {
    vadj_flag = true;
    adj = gtk_clist_get_vadjustment(GTK_CLIST(historyList));
    vadj = adj->value;
  }
  int rowindex = gtk_clist_append(GTK_CLIST(historyList), clist_item);
  //GdkColor color = {0, 0, 0};
  //gtk_clist_set_foreground(GTK_CLIST(historyList), rowindex, &color);

  gtk_clist_set_row_data(GTK_CLIST(historyList), rowindex, time_ptr);
  fitHistory();
  if(vadj_flag) {
    gtk_adjustment_set_value(adj, vadj);
  }

  delete [] clist_item[HistoryWindow::COL_FILE_H];
  delete [] clist_item[HistoryWindow::COL_URL_H];
  delete [] clist_item[HistoryWindow::COL_SAVE_H];
  delete [] clist_item[HistoryWindow::COL_SIZE_H];
  delete [] clist_item[HistoryWindow::COL_DATE_H];
  pthread_mutex_unlock(&historyLock);
  return true;
}

void HistoryWindow::fitHistory()
{
  if(GTK_CLIST(historyList)->rows >= maxHistory) {
    int diff = GTK_CLIST(historyList)->rows-maxHistory;
    for(int i = 0; i <= diff; ++i) {
      time_t *time_ptr = (time_t *)gtk_clist_get_row_data(GTK_CLIST(historyList), 0);
      delete time_ptr;
      gtk_clist_remove(GTK_CLIST(historyList), 0);
    }
  }
}

void HistoryWindow::show()
{
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_MOUSE);
  gtk_widget_show(window);
  gdk_window_raise(window->window);
}

void HistoryWindow::hide()
{
  gtk_widget_hide(window);
  gtk_clist_unselect_all(GTK_CLIST(historyList));
}

bool HistoryWindow::readFile(const string& filename)
{
  ifstream infile(filename.c_str(), ios::in);

  if(infile.bad()) return false;
  int count = 0;
  while(!infile.eof() && count < maxHistory) {
    string line;
    getline(infile, line, '\n');
    if(line.size()) {
      string file = Token_splitter(line, "\t");
      string size = Token_splitter(line, "\t");
      time_t time_log = stoi(Token_splitter(line, "\t"));
      string url = Token_splitter(line, "\t");
      string save = Token_splitter(line, "\n");
      addItem(file, size, time_log, url, save);
      ++count;
    }
  }
  return true;
}

bool HistoryWindow::writeFile(const string& filename)
{
  pthread_mutex_lock(&historyLock);
  string filenameTemp = filename+".temporary.working";
  ofstream outfile(filenameTemp.c_str(), ios::out);//, 0600);
  if(outfile.bad()) {
    pthread_mutex_unlock(&historyLock);
    return false;
  }
  for(int rowindex = 0; rowindex < GTK_CLIST(historyList)->rows; ++rowindex) {
    char *url;
    gtk_clist_get_text(GTK_CLIST(historyList), rowindex, HistoryWindow::COL_URL_H, &url);
    char *size;
    gtk_clist_get_text(GTK_CLIST(historyList), rowindex, HistoryWindow::COL_SIZE_H, &size);
    time_t *time_ptr = (time_t *)gtk_clist_get_row_data(GTK_CLIST(historyList), rowindex);
    
    char *file;
    gtk_clist_get_text(GTK_CLIST(historyList), rowindex, HistoryWindow::COL_FILE_H, &file);
    char *save;
    gtk_clist_get_text(GTK_CLIST(historyList), rowindex, HistoryWindow::COL_SAVE_H, &save);

    outfile << file << '\t' << size << '\t' << *time_ptr << '\t' << url << '\t' << save << endl;

  }

  if(outfile.bad() || outfile.fail() || rename(filenameTemp.c_str(), filename.c_str()) < 0) {
    pthread_mutex_unlock(&historyLock);
    return false;
  }
  chmod(filename.c_str(), S_IRUSR|S_IWUSR);
  pthread_mutex_unlock(&historyLock);
  
  return true;
}

void HistoryWindow::setHistoryMax(int max)
{
  if(max >= 0 && MAXHISTORY >= max) {
    maxHistory = max;
  } else {
    maxHistory = DEFAULTHISTORY;
  }
  fitHistory();
}

GtkWidget *HistoryWindow::getWindow() {
  return window;
}

GtkWidget *HistoryWindow::getHistoryList() {
  return historyList;
}

void HistoryWindow_searchButton_clicked_cb(GtkWidget *button,
					   HistoryWindow *hw) {
  hw->search();
}

void HistoryWindow::search() {
  string query = Remove_white(gtk_entry_get_text(GTK_ENTRY(queryEntry)));
  gtk_widget_set_sensitive(queryEntry, FALSE);

  if(query.empty()) {
    gtk_widget_set_sensitive(queryEntry, TRUE);
    return;
  }

  pthread_mutex_lock(&historyLock);

  GList *node = GTK_CLIST(historyList)->selection;
  int start_index = 0;
  if(node != NULL) {
    start_index = GPOINTER_TO_INT(node->data)+1;
  }
  gtk_clist_unselect_all(GTK_CLIST(historyList));
  for(int rowindex = start_index; rowindex < GTK_CLIST(historyList)->rows; ++rowindex) {
    char *filename;
    gtk_clist_get_text(GTK_CLIST(historyList), rowindex, COL_FILE_H, &filename);

    if(casefind(filename, query) != string::npos) {
      gtk_clist_select_row(GTK_CLIST(historyList), rowindex, COL_FILE_H);

      gtk_clist_moveto(GTK_CLIST(historyList), rowindex, COL_FILE_H, 0, 0);
      pthread_mutex_unlock(&historyLock);

      gtk_widget_set_sensitive(queryEntry, TRUE);
      return;
    }
  }

  pthread_mutex_unlock(&historyLock);
  gtk_widget_set_sensitive(queryEntry, TRUE);

  return;
}

GtkWidget *HistoryWindow::createQueryUI(GtkWidget *window) {
  GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
  gtk_widget_show(hbox);
  
  GtkWidget *queryLabel = gtk_label_new(_("Query"));
  gtk_widget_show(queryLabel);
  gtk_box_pack_start(GTK_BOX(hbox), queryLabel, FALSE, FALSE, 5);

  queryEntry = gtk_entry_new();
  gtk_widget_set_usize(GTK_WIDGET(queryEntry), 300, -1);
  gtk_widget_show(queryEntry);
  gtk_box_pack_start(GTK_BOX(hbox), queryEntry, FALSE, FALSE, 5);

  searchButton = gtk_button_new_with_label(_("Search"));
  gtk_widget_show(searchButton);
  gtk_box_pack_start(GTK_BOX(hbox), searchButton, FALSE, FALSE, 5);
  // set callback functions
  gtk_signal_connect(GTK_OBJECT(searchButton),
		     "clicked",
		     GTK_SIGNAL_FUNC(HistoryWindow_searchButton_clicked_cb),
		     (void *)this);

  // set callback functions
  gtk_signal_connect_object(GTK_OBJECT(queryEntry),
			    "activate",
			    GTK_SIGNAL_FUNC(gtk_button_clicked),
			    GTK_OBJECT(searchButton));

  return hbox;
}
