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

// $Id: ListEntry.h,v 1.9 2001/11/04 10:18:07 tujikawa Exp $

#ifndef _LISTENTRY_H_
#define _LISTENTRY_H_
#include "aria.h"
#include "gui_utils.h"
#include "CRCList.h"
#include "ItemCell.h"
#include "ThreadManager.h"
#include "ItemManager.h"


class ListEntry
{
private:
  ThreadManager *thread_manager;
  ItemManager *item_manager;
  
  GtkListStore *dl_model;
  GtkWidget *dl_table;
  std::vector<ItemCell* > items;
  string name;
  int n_thread;

  pthread_mutex_t dl_clist_lock;
  pthread_cond_t dl_clist_signal;

  ItemCell *default_item;

  void Create_dl_clist();

  GdkPixbuf *statusIcon[ICON_TOTAL];
  GdkBitmap *statusIconMask[ICON_TOTAL];
public:
  ThreadManager *getThreadManager();
  void setThreadManager(ThreadManager *threadManager_in);

  ItemManager *getItemManager();
  void setItemManager(ItemManager *item_manager_in);

  const string& getName() const;
  void setName(const string& name_in);

  void setThreadLimit(int n_thread_in);
  int getThreadLimit() const;

  GtkWidget *ret_Dl_clist();

  // append clist_item and set rowdata and item_manager->regist_item_back(itemcell)
  int Append_dl_item(char *clist_item[], ItemCell *itemcell);
  // do same things above
  int Insert_dl_item(char *clist_item[], ItemCell *itemcell, ItemCell *itemcell_base);

  void setMD5List(list<CRCList*>& md5List);
  void setCRCList(list<CRCList*>& crcList);

  void setStatusIcon(GdkPixbuf *pixmaps[], GdkBitmap *bitmaps[]);
  void setSizeDisplayStyle(bool flagHumanReadble);

  void Set_clist_column__crc(int rowindex, const string& crc_string);
  void Set_clist_column__md5(int rowindex, const string& md5String);
  void Set_clist_column__cursize(int rowindex, const string& size_string);
  void Set_clist_column__totsize(int rowindex, const string& size_string);
  void Set_clist_column__progress(int rowindex, int progress);
  void Set_clist_column__filename(int rowindex, const string& filename);
  void Set_clist_column__speed(int rowindex, const string& speed_string);
  void Set_clist_column__rtime(int rowindex, const string& rtime_string);
  void Set_clist_column__status(int rowindex, const string& status_string);
  void Set_clist_column__rec(int rowindex, const string& rec_string);
  void Set_clist_column__url(int rowindex, const string& url_string);
  void Set_clist_column__save(int rowindex, const string& save_string);
  void Set_clist_column__try(int rowindex, int currentCount, int maxCount);
  void Set_clist_column__icon(int rowindex , ItemCell::ItemStatusType status);
  void updateRow(int rowIndex, int columnIndex = -1); // col -1 means full row

  void Sort_by_filename(GtkSortType stype);
  void Sort_by_extension(GtkSortType stype);
  void Sort_by_save(GtkSortType stype);
  void Sort_by_url(GtkSortType stype);
  void Sort_by_csize(GtkSortType stype);
  void Sort_by_tsize(GtkSortType stype);
  void Sort_by_progress(GtkSortType stype);
  void Sort_by_retry(GtkSortType stype);
  void Sort_by_rec(GtkSortType stype);
  void Sort_by_status(GtkSortType stype);

  void get_Dl_clist_lock();
  void release_Dl_clist_lock();

  void freezeDlCList();
  void thawDlCList();

  void dlCList_selectRow(int row, int column, GdkEventButton *event);
  void dlCList_unselectRow(int row, int column, GdkEventButton *event);
  void dlCList_clickColumn(int column);

  void Send_start_signal();
  void Signal_wait();
  
  ItemCell *ret_Default_item();
  const Options& ret_Options() const;
  void set_Options(const Options& options_in);

  ItemCell *Get_next_item();
  ItemCell *getItemCellByRow(int rowindex) const;
  std::size_t getRowCount() const { return items.size();};
  std::size_t getRowForCell(ItemCell *item) {
	  return distance(items.begin(), std::find(items.begin(), items.end(), item) );
	};
		

  list<int> getActiveRowList();

  void init(const string& name_in,
	    int n_thread_in,
	    const Options& options_in);

  ListEntry(const string& name_in, int n_thread_in, const Options& options_in);
  ListEntry(int n_thread_in, const Options& options_in);
  ~ListEntry();

  //GtkCallback func
  //Row_moved();
};
#endif // _LISTENTRY_H_
