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

// $Id: ListEntry.cc,v 1.20 2002/02/13 12:09:24 tujikawa Exp $

#include "ListEntry.h"
#include "ItemOption.h"

#include <iostream>

extern void dragged_to_download_list(GtkWidget *, GdkDragContext *, int, int, GtkSelectionData *, unsigned int, unsigned int);
extern void Set_sensitive__no_item_selected();
extern void Set_sensitive__items_selected();
extern void Set_speed_scale(ItemCell* itemcell);
extern void Show_download_log(ItemCell* itemcell);
extern ItemOption *g_itemOption;

extern GtkWidget *g_popupMenu;
extern int g_threadLimit;
extern pthread_mutex_t g_appLock;
extern GdkBitmap *sg_progressBarMask[51];
extern GdkPixmap *sg_progressBar[51];

bool ListEntry::useHumanReadable;

static void
dlCList_rowMove_cb(GtkCList *clist,
                   int srcRow,
                   int dstRow,
                   ListEntry *listentry)
{
	// at this time, the row doesn't move yet.
	//ItemCell *itemcell = (ItemCell *)gtk_clist_get_row_data(clist, src_row);

	//item_manager->move_to(dst_row, itemcell);
	/*
	if(gtk_clist_row_is_visible(GTK_CLIST(clist), dst_row) != GTK_VISIBILITY_FULL) {
	  gtk_clist_moveto(GTK_CLIST(clist), dst_row, 0, 0.0, 0.0);
	}
	*/
}

// ユーザがリストの行を選択するとここにくる
static void
dlCList_unselectRow_cb(GtkWidget *clist,
                       int row,
                       int column,
                       GdkEventButton *event,
                       ListEntry *listentry)
{
	static bool enterflag = false;
	if(enterflag == true || event == NULL) return;
	enterflag = true;

	listentry->dlCList_unselectRow(row, column, event);

	enterflag = false;
}

// ユーザがリストの行を選択するとここにくる
static void
dlCList_selectRow_cb(GtkWidget *clist,
                     int row,
                     int column,
                     GdkEventButton *event,
                     ListEntry *listentry)
{
	static bool enterflag = false;
	if(enterflag == true || event == NULL) return;
	enterflag = true;

	listentry->dlCList_selectRow(row, column, event);

	enterflag = false;
}

void
ListEntry::dlCList_unselectRow(int row,
                               int column,
                               GdkEventButton *event)
{
	std::cout << "unselect " << row << std::endl;
		ItemCell* itemcell = items[row] ;
		GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(dl_table));
	    switch(event->button) {
	    case 1:
	        if(event->state & (GDK_CONTROL_MASK | GDK_SHIFT_MASK)) {
	            //      GList *node = GTK_CLIST(dl_clist)->selection;
	            if(gtk_tree_selection_count_selected_rows(selection) == 0) {
	                Set_sensitive__no_item_selected();
	            } else {
	                //gtk_clist_unselect_row(GTK_CLIST(dl_clist), row, 0);
	                //	GList *node = GTK_CLIST(dl_clist)->selection;
					GtkTreeIter iter;
					gtk_tree_model_iter_nth_child (GTK_TREE_MODEL(dl_model), &iter, NULL, row);
					gtk_tree_selection_unselect_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(dl_table)), &iter);
	                if(gtk_tree_selection_count_selected_rows(selection) == 0) {
	                    Set_sensitive__no_item_selected();
	                }
	            }
	        } else {
	            //GList *node = GTK_CLIST(dl_clist)->selection;
	            if(gtk_tree_selection_count_selected_rows(selection) != 0) {
	                //gtk_clist_unselect_all(GTK_CLIST(dl_clist));
	                gtk_tree_selection_unselect_all(selection);
	                //gtk_clist_select_row(GTK_CLIST(dl_clist), row, 0);
					GtkTreeIter iter;
					gtk_tree_model_iter_nth_child (GTK_TREE_MODEL(dl_model), &iter, NULL, row);
					gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(dl_table)), &iter);
	                Set_sensitive__items_selected();
	                Set_speed_scale(itemcell);
	                Show_download_log(itemcell);
	            } else {
	                Set_sensitive__no_item_selected();
	            }
	        }
	        break;
	    case 2:
	    case 3:
	    {
	        //gtk_clist_select_row(GTK_CLIST(dl_clist), row, 0);
			GtkTreeIter iter;
			gtk_tree_model_iter_nth_child (GTK_TREE_MODEL(dl_model), &iter, NULL, row);
			gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(dl_table)), &iter);
	        Set_speed_scale(itemcell);
	        Show_download_log(itemcell);
	        Set_sensitive__items_selected();

	        gtk_menu_popup(GTK_MENU(g_popupMenu),
	                       (GtkWidget*)NULL,
	                       (GtkWidget*)NULL,
	                       (GtkMenuPositionFunc)NULL,
	                       NULL,
	                       0,// fix this
	                       0);
	        break;
	    }
	    }
	return;
}

void
ListEntry::dlCList_selectRow(int row,
                             int column,
                             GdkEventButton *event)
{
	std::cout << "select " << row << std::endl;
		ItemCell* itemcell = items[row] ;

	    if(event->type == GDK_2BUTTON_PRESS) {
	        g_itemOption->setOptionValues(itemcell,
	                                      itemcell->ret_Options_opt(),
	                                      this);
	        g_itemOption->show();
	        Set_sensitive__items_selected();
	    } else {
	        switch(event->button) {
	        case 1:
	            if(event->state & GDK_CONTROL_MASK) {
	                Set_speed_scale(itemcell);
	                Show_download_log(itemcell);
	            } else if(event->state & GDK_SHIFT_MASK) {
/*	                int nearestSelectedRow = findNearestSelectedRow(dl_clist, row);
	                if(nearestSelectedRow < row) {
	                    for(int index = nearestSelectedRow+1; index < row; ++index) {
	                        gtk_clist_select_row(GTK_CLIST(dl_clist), index, 0);
	                    }
	                } else {
	                    for(int index = row+1; index < nearestSelectedRow; ++index) {
	                        gtk_clist_select_row(GTK_CLIST(dl_clist), index, 0);
	                    }
	                }*/
	                Set_speed_scale(itemcell);
	                Show_download_log(itemcell);
	            } else {// with no mask
					gtk_tree_selection_unselect_all(gtk_tree_view_get_selection(GTK_TREE_VIEW(dl_table)));
	                //gtk_clist_select_row(GTK_CLIST(dl_clist), row, 0);
					GtkTreeIter iter;
					gtk_tree_model_iter_nth_child (GTK_TREE_MODEL(dl_model), &iter, NULL, row);
					gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(dl_table)), &iter);
	                Set_speed_scale(itemcell);
	                Show_download_log(itemcell);
	            }
	            Set_sensitive__items_selected();
	            break;
	        case 2:
	        case 3:
	        {
	            //gtk_clist_unselect_all(GTK_CLIST(dl_clist));
	            //gtk_clist_select_row(GTK_CLIST(dl_clist), row, 0);
	            Set_speed_scale(itemcell);
	            Show_download_log(itemcell);
	            Set_sensitive__items_selected();

	            gtk_menu_popup(GTK_MENU(g_popupMenu),
	                           (GtkWidget*)NULL,
	                           (GtkWidget*)NULL,
	                           (GtkMenuPositionFunc)NULL,
	                           NULL,
	                           0,//fix this
	                           0);
	            break;
	        }
	        default:
	            break;
	        }
	    }
}

/*gboolean view_selection_func (GtkTreeSelection *selection,
		GtkTreeModel     *model,
		GtkTreePath      *path,
		gboolean          path_currently_selected,
		gpointer          userdata)
{
	GtkTreeIter iter;

    if (gtk_tree_model_get_iter(model, &iter, path))
    {
		std::cout << "SelectFunction " << gtk_tree_model_get_string_from_iter(model, &iter) << std::endl;
	}
	return TRUE; // allow selection state to change
}*/

/*static void evt_selection_changed(GtkTreeSelection *ts, gpointer user)
{
	std::cout << "Selection signal" << std::endl;
}
*/
static gboolean evt_dirpane_button_press(GtkWidget *wid, GdkEventButton *event, gpointer user)
{
	std::cout << "Click" << std::endl;
	GtkTreeView* gtv = GTK_TREE_VIEW(wid);
	GtkTreePath *path;
	if (gtk_tree_view_get_path_at_pos(gtv, (gint) event->x, (gint) event->y, &path, NULL, NULL, NULL))
	{
		g_print ("\t\there\n");
		GtkTreeIter iter;
		GtkTreeModel *model = gtk_tree_view_get_model (gtv);
		gtk_tree_model_get_iter (model, &iter, path);
		std::cout << "SelectFunction " << gtk_tree_model_get_string_from_iter(model, &iter) << std::endl;
		GtkTreeSelection* sel = gtk_tree_view_get_selection(gtv);

		gint pathdepth;
		gint *indices = gtk_tree_path_get_indices_with_depth (path, &pathdepth);
		gint row = indices[pathdepth-1];
		ListEntry* listEntry = (ListEntry*) user;
		if (gtk_tree_selection_path_is_selected(sel, path))
		{//already selected: unselect
			dlCList_unselectRow_cb(wid, row, 0, event, listEntry);
		}
		else
		{//do selection
			dlCList_selectRow_cb(wid, row, 0, event, listEntry);
		}
	}
	//return FALSE; //do not consume the event
	return TRUE; // consume the event
}

ListEntry::ListEntry(const string& name_in,
                     int n_thread_in,
                     const Options& options_in)
{
	init(name_in, n_thread_in, options_in);
}

ListEntry::ListEntry(int n_thread_in,
                     const Options& options_in)
{
	init(_("New list"), n_thread_in, options_in);
}

void ListEntry::init(const string& name_in,
                     int n_thread_in,
                     const Options& options_in)
{
	pthread_mutex_init(&dl_clist_lock, NULL);
	//pthread_cond_init(&dl_clist_signal, NULL);

	pthread_mutex_lock(&dl_clist_lock);
	name = name_in;

	default_item = new ItemCell();
	default_item->set_Options_opt(options_in);

	if(n_thread_in > g_threadLimit) {
		n_thread = g_threadLimit;
	} else if(n_thread_in == 0) {
		n_thread = 1;
	} else {
		n_thread = n_thread_in;
	}

	item_manager = new ItemManager();
	Create_dl_clist();
	thread_manager = new ThreadManager(n_thread, this);
	pthread_mutex_unlock(&dl_clist_lock);
}

ListEntry::~ListEntry()
{
	// need pthread lock in order to avoid seg fault
	thread_manager->setHaltFlag();
	pthread_mutex_lock(&dl_clist_lock);// fix this
	for(size_t rowindex = 0; rowindex < items.size(); ++rowindex) {
		ItemCell* itemcell = items[rowindex];
		switch(itemcell->ret_Status()) {

		case ItemCell::ITEM_DOWNLOAD:
		case ItemCell::ITEM_INUSE:
		case ItemCell::ITEM_DOWNLOAD_AGAIN:
		case ItemCell::ITEM_DOWNLOAD_INTERNAL_AGAIN:
		case ItemCell::ITEM_INUSE_AGAIN:
		{
			ItemCommand itemcommand;
			itemcommand.commandtype = ItemCommand::COMMAND_HALT;
			//itemcell->set_Status(ItemCell::ITEM_READY);
			write(itemcell->ret_Desc_w(), &itemcommand, sizeof(ItemCommand));
		}
		break;
		case ItemCell::ITEM_DOWNLOAD_PARTIAL:
		case ItemCell::ITEM_READY_CONCAT:
		case ItemCell::ITEM_READY_AGAIN:
			//itemcell->set_Status(ItemCell::ITEM_READY);
			break;
		default:
			break;
		}
	}
	//pthread_cond_broadcast(&dl_clist_signal); // fix this
	pthread_mutex_unlock(&dl_clist_lock);// fix this

	thread_manager->waitThreadTermination();
	pthread_mutex_lock(&dl_clist_lock);
	delete thread_manager;
	delete item_manager;

	for(size_t rowindex = 0; rowindex < items.size(); ++rowindex) {
		ItemCell *itemcell = items[rowindex];
		delete itemcell;
	}
	pthread_mutex_unlock(&dl_clist_lock);
	pthread_mutex_destroy(&dl_clist_lock);
	//pthread_cond_destroy(&dl_clist_signal);
	// dl_clist is deleted when notebook page is destroyed
}

// compare functions for sorting download list
/*static int SortByAlphaFunc(GtkCList *clist,
                           gconstpointer ptr1,
                           gconstpointer ptr2)
{
	const GtkCListRow *row1 = (const GtkCListRow *)ptr1;
	const GtkCListRow *row2 = (const GtkCListRow *)ptr2;

	char *text1 = GTK_CELL_TEXT(row1->cell[clist->sort_column])->text;
	char *text2 = GTK_CELL_TEXT(row2->cell[clist->sort_column])->text;

	return strcmp(text1, text2);
}

static int SortByExtensionFunc(GtkCList *clist,
                               gconstpointer ptr1,
                               gconstpointer ptr2)
{
	const GtkCListRow *row1 = (const GtkCListRow *)ptr1;
	const GtkCListRow *row2 = (const GtkCListRow *)ptr2;

	string ext1 = ((ItemCell *)(row1->data))->ret_URL_Container().ret_Extension();
	string ext2 = ((ItemCell *)(row2->data))->ret_URL_Container().ret_Extension();

	int retval = strcmp(ext1.c_str(), ext2.c_str());

	if(retval == 0) {
		char *text1 = GTK_CELL_TEXT(row1->cell[clist->sort_column])->text;
		char *text2 = GTK_CELL_TEXT(row2->cell[clist->sort_column])->text;
		retval = strcmp(text1, text2);
	}

	return retval;
}

static int SortByCurrentSizeFunc(GtkCList *clist,
                                 gconstpointer ptr1,
                                 gconstpointer ptr2)
{
	const GtkCListRow *row1 = (const GtkCListRow *)ptr1;
	const GtkCListRow *row2 = (const GtkCListRow *)ptr2;

	int size1 = ((ItemCell *)(row1->data))->ret_Size_Current();
	int size2 = ((ItemCell *)(row2->data))->ret_Size_Current();

	if(size1 < size2) return -1;
	else if(size1 > size2) return 1;
	else return 0;
}

static int SortByTotalSizeFunc(GtkCList *clist,
                               gconstpointer ptr1,
                               gconstpointer ptr2)
{
	const GtkCListRow *row1 = (const GtkCListRow *)ptr1;
	const GtkCListRow *row2 = (const GtkCListRow *)ptr2;

	int size1 = ((ItemCell *)(row1->data))->ret_Size_Total();
	int size2 = ((ItemCell *)(row2->data))->ret_Size_Total();

	if(size1 < size2) return -1;
	else if(size1 > size2) return 1;
	else return 0;
}

static int SortByProgressFunc(GtkCList *clist,
                              gconstpointer ptr1,
                              gconstpointer ptr2)
{
	const GtkCListRow *row1 = (const GtkCListRow *)ptr1;
	const GtkCListRow *row2 = (const GtkCListRow *)ptr2;

	int progress1 = 0;
	int size1 = ((ItemCell *)(row1->data))->ret_Size_Current();
	if(size1 > 0) {
		progress1 = (int)((float)size1/((ItemCell *)(row1->data))->ret_Size_Total()*100);
	}
	int progress2 = 0;
	int size2 = ((ItemCell *)(row2->data))->ret_Size_Current();
	if(size2 > 0) {
		progress2 = (int)((float)size2/((ItemCell *)(row2->data))->ret_Size_Total()*100);
	}

	if(progress1 < progress2) return -1;
	else if(progress1 > progress2) return 1;
	else return 0;
}

static int SortByRetryFunc(GtkCList *clist,
                           gconstpointer ptr1,
                           gconstpointer ptr2)
{
	const GtkCListRow *row1 = (const GtkCListRow *)ptr1;
	const GtkCListRow *row2 = (const GtkCListRow *)ptr2;

	int size1 = ((ItemCell *)(row1->data))->ret_Count();
	int size2 = ((ItemCell *)(row2->data))->ret_Count();

	if(size1 < size2) return -1;
	else if(size1 > size2) return 1;
	else return 0;
}
*/
static int SortByRecFunc(GtkCList *clist,
                         gconstpointer ptr1,
                         gconstpointer ptr2)
{
	const GtkCListRow *row1 = (const GtkCListRow *)ptr1;
	const GtkCListRow *row2 = (const GtkCListRow *)ptr2;

	char *text1 = GTK_CELL_TEXT(row1->cell[clist->sort_column])->text;
	char *text2 = GTK_CELL_TEXT(row2->cell[clist->sort_column])->text;

	int size1 = stoi(text1);
	int size2 = stoi(text2);

	if(size1 > size2) return 1;
	else if(size1 < size2) return -1;
	else return 0;
}

static int SortByStatusFunc(GtkCList *clist,
                            gconstpointer ptr1,
                            gconstpointer ptr2)
{
	const GtkCListRow *row1 = (const GtkCListRow *)ptr1;
	const GtkCListRow *row2 = (const GtkCListRow *)ptr2;

	ItemCell::ItemStatusType stat1 = ((ItemCell *)(row1->data))->ret_Status();
	ItemCell::ItemStatusType stat2 = ((ItemCell *)(row2->data))->ret_Status();

	if(stat1 > stat2) return 1;
	else if(stat1 < stat2) return -1;
	else return 0;
}

// ダウンロードリストをソートする
// クリック毎に昇順,降順を切替える
static GtkSortType sg_sortSwitch = GTK_SORT_ASCENDING;

static void
dlCList_clickColumn_cb(GtkCList *clist,
                       int column,
                       ListEntry *listentry)
{
	listentry->dlCList_clickColumn(column);
}

void
ListEntry::dlCList_clickColumn(int column)
{
/*	gtk_clist_set_sort_column(GTK_CLIST(dl_clist), column);

	if(sg_sortSwitch == GTK_SORT_ASCENDING) {
		gtk_clist_set_sort_type(GTK_CLIST(dl_clist), GTK_SORT_DESCENDING);
		sg_sortSwitch = GTK_SORT_DESCENDING;
	} else {
		gtk_clist_set_sort_type(GTK_CLIST(dl_clist), GTK_SORT_ASCENDING);
		sg_sortSwitch = GTK_SORT_ASCENDING;
	}

	switch(column) {
	case COL_FILENAME:
	case COL_SAVE:
	case COL_URL:
		gtk_clist_set_compare_func(GTK_CLIST(dl_clist), (GtkCListCompareFunc)SortByAlphaFunc);
		break;
	case COL_CURSIZE:
		gtk_clist_set_compare_func(GTK_CLIST(dl_clist), (GtkCListCompareFunc)SortByCurrentSizeFunc);
		break;
	case COL_TOTSIZE:
		gtk_clist_set_compare_func(GTK_CLIST(dl_clist), (GtkCListCompareFunc)SortByTotalSizeFunc);
		break;
	case COL_PROGRESS:
		gtk_clist_set_compare_func(GTK_CLIST(dl_clist), (GtkCListCompareFunc)SortByProgressFunc);
		break;
	case COL_RETRY:
		gtk_clist_set_compare_func(GTK_CLIST(dl_clist), (GtkCListCompareFunc)SortByRetryFunc);
		break;
	case COL_REC:
		gtk_clist_set_compare_func(GTK_CLIST(dl_clist), (GtkCListCompareFunc)SortByRecFunc);
		break;
	case COL_STATUS:
	case COL_ICON:
		gtk_clist_set_compare_func(GTK_CLIST(dl_clist), (GtkCListCompareFunc)SortByStatusFunc);
		break;
		//case COL_CRC:
		//case COL_MD5:
		//case COL_SPEED:
		//case COL_RTIME:
	default:
		return;
		break;
	}

	get_Dl_clist_lock();
	gtk_clist_sort(GTK_CLIST(dl_clist));
	release_Dl_clist_lock();*/
}

/*
static void set_sort_type(GtkWidget *clist)
{
  if(sg_sortSwitch == GTK_SORT_ASCENDING) {
    gtk_clist_set_sort_type(GTK_CLIST(clist), GTK_SORT_DESCENDING);
    sg_sortSwitch = GTK_SORT_DESCENDING;
  } else {
    gtk_clist_set_sort_type(GTK_CLIST(clist), GTK_SORT_ASCENDING);
    sg_sortSwitch = GTK_SORT_ASCENDING;
  }
}
*/

void ListEntry::Sort_by_filename(GtkSortType stype)
{
/*	gtk_clist_set_sort_column(GTK_CLIST(dl_clist), COL_FILENAME);
	gtk_clist_set_sort_type(GTK_CLIST(dl_clist), stype);
	gtk_clist_set_compare_func(GTK_CLIST(dl_clist), (GtkCListCompareFunc)SortByAlphaFunc);
	pthread_mutex_lock(&dl_clist_lock);
	gtk_clist_sort(GTK_CLIST(dl_clist));
	pthread_mutex_unlock(&dl_clist_lock);*/
}

void ListEntry::Sort_by_extension(GtkSortType stype)
{
/*	gtk_clist_set_sort_type(GTK_CLIST(dl_clist), stype);
	gtk_clist_set_compare_func(GTK_CLIST(dl_clist), (GtkCListCompareFunc)SortByExtensionFunc);
	pthread_mutex_lock(&dl_clist_lock);
	gtk_clist_sort(GTK_CLIST(dl_clist));
	pthread_mutex_unlock(&dl_clist_lock);*/
}

void ListEntry::Sort_by_save(GtkSortType stype)
{
/*	gtk_clist_set_sort_column(GTK_CLIST(dl_clist), COL_SAVE);
	gtk_clist_set_sort_type(GTK_CLIST(dl_clist), stype);
	gtk_clist_set_compare_func(GTK_CLIST(dl_clist), (GtkCListCompareFunc)SortByAlphaFunc);
	pthread_mutex_lock(&dl_clist_lock);
	gtk_clist_sort(GTK_CLIST(dl_clist));
	pthread_mutex_unlock(&dl_clist_lock);*/
}

void ListEntry::Sort_by_url(GtkSortType stype)
{
/*	gtk_clist_set_sort_column(GTK_CLIST(dl_clist), COL_URL);
	gtk_clist_set_sort_type(GTK_CLIST(dl_clist), stype);
	gtk_clist_set_compare_func(GTK_CLIST(dl_clist), (GtkCListCompareFunc)SortByAlphaFunc);
	pthread_mutex_lock(&dl_clist_lock);
	gtk_clist_sort(GTK_CLIST(dl_clist));
	pthread_mutex_unlock(&dl_clist_lock);*/
}

void ListEntry::Sort_by_csize(GtkSortType stype)
{
	//gtk_clist_set_sort_column(GTK_CLIST(dl_clist), COL_CURSIZE);
/*	gtk_clist_set_sort_type(GTK_CLIST(dl_clist), stype);
	gtk_clist_set_compare_func(GTK_CLIST(dl_clist), (GtkCListCompareFunc)SortByCurrentSizeFunc);
	pthread_mutex_lock(&dl_clist_lock);
	gtk_clist_sort(GTK_CLIST(dl_clist));
	pthread_mutex_unlock(&dl_clist_lock);*/
}

void ListEntry::Sort_by_tsize(GtkSortType stype)
{
	//gtk_clist_set_sort_column(GTK_CLIST(dl_clist), COL_TOTSIZE);
/*	gtk_clist_set_sort_type(GTK_CLIST(dl_clist), stype);
	gtk_clist_set_compare_func(GTK_CLIST(dl_clist), (GtkCListCompareFunc)SortByTotalSizeFunc);
	pthread_mutex_lock(&dl_clist_lock);
	gtk_clist_sort(GTK_CLIST(dl_clist));
	pthread_mutex_unlock(&dl_clist_lock);*/
}

void ListEntry::Sort_by_progress(GtkSortType stype)
{
	//gtk_clist_set_sort_column(GTK_CLIST(dl_clist), COL_PROGRESS);
/*	gtk_clist_set_sort_type(GTK_CLIST(dl_clist), stype);
	gtk_clist_set_compare_func(GTK_CLIST(dl_clist), (GtkCListCompareFunc)SortByProgressFunc);
	pthread_mutex_lock(&dl_clist_lock);
	gtk_clist_sort(GTK_CLIST(dl_clist));
	pthread_mutex_unlock(&dl_clist_lock);*/
}

void ListEntry::Sort_by_retry(GtkSortType stype)
{
	//gtk_clist_set_sort_column(GTK_CLIST(dl_clist), COL_RETRY);
/*	gtk_clist_set_sort_type(GTK_CLIST(dl_clist), stype);
	gtk_clist_set_compare_func(GTK_CLIST(dl_clist), (GtkCListCompareFunc)SortByRetryFunc);
	pthread_mutex_lock(&dl_clist_lock);
	gtk_clist_sort(GTK_CLIST(dl_clist));
	pthread_mutex_unlock(&dl_clist_lock);*/
}

void ListEntry::Sort_by_rec(GtkSortType stype)
{
/*	gtk_clist_set_sort_column(GTK_CLIST(dl_clist), COL_REC);
	gtk_clist_set_sort_type(GTK_CLIST(dl_clist), stype);
	gtk_clist_set_compare_func(GTK_CLIST(dl_clist), (GtkCListCompareFunc)SortByRecFunc);
	pthread_mutex_lock(&dl_clist_lock);
	gtk_clist_sort(GTK_CLIST(dl_clist));
	pthread_mutex_unlock(&dl_clist_lock);*/
}

void ListEntry::Sort_by_status(GtkSortType stype)
{
	//gtk_clist_set_sort_column(GTK_CLIST(dl_clist), COL_STATUS);
/*	gtk_clist_set_sort_type(GTK_CLIST(dl_clist), stype);
	gtk_clist_set_compare_func(GTK_CLIST(dl_clist), (GtkCListCompareFunc)SortByStatusFunc);
	pthread_mutex_lock(&dl_clist_lock);
	gtk_clist_sort(GTK_CLIST(dl_clist));
	pthread_mutex_unlock(&dl_clist_lock);*/
}

void ListEntry::Create_dl_clist()
{
	const char *titles[TOTALCOL];

	titles[COL_ICON] = _("*");
	titles[COL_FILENAME] = _("File");
	titles[COL_CURSIZE] = _("Downloaded");
	titles[COL_TOTSIZE] = _("Total");
	titles[COL_PROGRESS] = _("%");
	titles[COL_SPEED] = _("Speed");
	titles[COL_RTIME] = _("Remaining");
	titles[COL_RETRY] = _("Retry");
	titles[COL_REC] = _("Recurse");
	titles[COL_STATUS] = _("Status");
	titles[COL_CRC] = "CRC";
	titles[COL_MD5] = "MD5";
	titles[COL_SAVE] = _("Save");
	titles[COL_URL] = "URL";

	//create dl_clist(CLIST) with TOTALCOL columns
	//dl_clist = gtk_clist_new_with_titles(TOTALCOL, titles);
	dl_model = gtk_list_store_new(TOTALCOL, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, GDK_TYPE_PIXBUF,//*,file,downloaded,total,%
		G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, //speed,remainig,retry,recurse,status,crc,md5
		G_TYPE_STRING, G_TYPE_STRING);//save,url
	dl_table = gtk_tree_view_new_with_model(GTK_TREE_MODEL(dl_model));

	for (size_t i = 0; i < TOTALCOL; ++i)
	{
		GtkTreeViewColumn *col = gtk_tree_view_column_new();
		gtk_tree_view_append_column(GTK_TREE_VIEW(dl_table), col);
		gtk_tree_view_column_set_title( col, titles[i]);
		gtk_tree_view_column_set_resizable( col, (i != COL_ICON && i != COL_PROGRESS) );
		if (i != 0 && i != 4)
		{
			GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
			gtk_tree_view_column_pack_start(col, renderer, TRUE);
			gtk_tree_view_column_add_attribute(col, renderer, "text", i);//set renderer att text as value to update from model
		} else {
			GtkCellRenderer *renderer = gtk_cell_renderer_pixbuf_new();
			gtk_tree_view_column_pack_start(col, renderer, TRUE);
			gtk_tree_view_column_add_attribute(col, renderer, "pixbuf", i);//set renderer att text as value to update from model
		}
	}

	//gtk_object_set_user_data(GTK_OBJECT(dl_clist), (void *)this);

	// セレクションモードをMULTIPLEに設定
//	gtk_clist_set_selection_mode(GTK_LIST_STORE(dl_clist), GTK_SELECTION_MULTIPLE);
	gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(dl_table)), GTK_SELECTION_MULTIPLE);
	gtk_tree_view_set_rubber_banding(GTK_TREE_VIEW(dl_table), true);
//	gtk_tree_selection_set_select_function(gtk_tree_view_get_selection(GTK_TREE_VIEW(dl_table)), view_selection_func, NULL, NULL);
//	g_signal_connect(G_OBJECT(gtk_tree_view_get_selection(GTK_TREE_VIEW(dl_table))), "changed", G_CALLBACK(evt_selection_changed), NULL);
	g_signal_connect(G_OBJECT(dl_table), "button_press_event", G_CALLBACK(evt_dirpane_button_press), this);

/*	gtk_clist_set_button_actions(GTK_CLIST(dl_clist),
	                             2,
	                             GTK_BUTTON_SELECTS);

	// dragでの順序入れ換えを許可する
	//gtk_clist_set_reorderable(GTK_CLIST(dl_clist), TRUE);
	// 選択,非選択時のコールバックを設定

	gtk_signal_connect(GTK_OBJECT(dl_clist), "select-row",
	                   GTK_SIGNAL_FUNC(dlCList_selectRow_cb),
	                   (GtkObject *)this);
	gtk_signal_connect(GTK_OBJECT(dl_clist), "unselect-row",
	                   GTK_SIGNAL_FUNC(dlCList_unselectRow_cb),
	                   (GtkObject *)this);
	gtk_signal_connect(GTK_OBJECT(dl_clist), "row-move",
	                   GTK_SIGNAL_FUNC(dlCList_rowMove_cb),
	                   (GtkObject *)this);
	gtk_signal_connect(GTK_OBJECT(dl_clist), "click-column",
	                   GTK_SIGNAL_FUNC(dlCList_clickColumn_cb),
	                   (GtkObject *)this);
*/
	//gtk_signal_connect_after(GTK_OBJECT(dl_clist), "button_press_event",
	//			   GTK_SIGNAL_FUNC(right_click_cb),
	//			   NULL);

	// 境界に影を付ける
//	gtk_clist_set_shadow_type(GTK_CLIST(dl_clist), GTK_SHADOW_ETCHED_OUT);

	//gtk_clist_column_titles_passive(GTK_CLIST(dl_clist));
	//各列の幅を設定
/*	gtk_clist_set_column_width(GTK_CLIST(dl_clist), COL_ICON, 30);
	gtk_clist_set_column_width(GTK_CLIST(dl_clist), COL_FILENAME, 310);
	gtk_clist_set_column_width(GTK_CLIST(dl_clist), COL_CURSIZE, 80);
	gtk_clist_set_column_width(GTK_CLIST(dl_clist), COL_TOTSIZE, 80);
	gtk_clist_set_column_width(GTK_CLIST(dl_clist), COL_PROGRESS, 50);
	gtk_clist_set_column_width(GTK_CLIST(dl_clist), COL_SPEED, 50);
	gtk_clist_set_column_width(GTK_CLIST(dl_clist), COL_RTIME, 70);
	gtk_clist_set_column_width(GTK_CLIST(dl_clist), COL_CRC, 80);
	gtk_clist_set_column_width(GTK_CLIST(dl_clist), COL_MD5, 80);
	gtk_clist_set_column_width(GTK_CLIST(dl_clist), COL_STATUS, 150);
	gtk_clist_set_column_width(GTK_CLIST(dl_clist), COL_RETRY, 60);
	gtk_clist_set_column_width(GTK_CLIST(dl_clist), COL_REC, 60);
*/
	// modified 2001/3/19
	//gtk_clist_set_column_width(GTK_CLIST(dl_clist), COL_SAVE, 400);

	// modified 2001/5/26
	//gtk_clist_set_column_auto_resize(GTK_CLIST(dl_clist),
	//			   COL_FILENAME, TRUE);
/*
	gtk_clist_set_column_auto_resize(GTK_CLIST(dl_clist), COL_CRC, TRUE);
	gtk_clist_set_column_auto_resize(GTK_CLIST(dl_clist), COL_MD5, TRUE);

	gtk_clist_set_column_auto_resize(GTK_CLIST(dl_clist), COL_SAVE, TRUE);
	// modified 2001/3/19
	//gtk_clist_set_column_width(GTK_CLIST(dl_clist), COL_URL, 700);
	gtk_clist_set_column_auto_resize(GTK_CLIST(dl_clist),
	                                 COL_URL, TRUE);

	gtk_clist_set_column_justification(GTK_CLIST(dl_clist),
	                                   COL_CURSIZE,
	                                   GTK_JUSTIFY_RIGHT);
	gtk_clist_set_column_justification(GTK_CLIST(dl_clist),
	                                   COL_TOTSIZE,
	                                   GTK_JUSTIFY_RIGHT);
	gtk_clist_set_column_justification(GTK_CLIST(dl_clist),
	                                   COL_PROGRESS,
	                                   GTK_JUSTIFY_RIGHT);
	gtk_clist_set_column_justification(GTK_CLIST(dl_clist),
	                                   COL_SPEED,
	                                   GTK_JUSTIFY_RIGHT);
	gtk_clist_set_column_justification(GTK_CLIST(dl_clist),
	                                   COL_RETRY,
	                                   GTK_JUSTIFY_RIGHT);
	gtk_clist_set_column_justification(GTK_CLIST(dl_clist),
	                                   COL_REC,
	                                   GTK_JUSTIFY_RIGHT);
	gtk_clist_set_column_justification(GTK_CLIST(dl_clist),
	                                   COL_CRC,
	                                   GTK_JUSTIFY_RIGHT);
	gtk_clist_set_column_justification(GTK_CLIST(dl_clist),
	                                   COL_MD5,
	                                   GTK_JUSTIFY_RIGHT);
	gtk_clist_set_column_justification(GTK_CLIST(dl_clist),
	                                   COL_RTIME,
	                                   GTK_JUSTIFY_RIGHT);
	gtk_clist_set_row_height(GTK_CLIST(dl_clist), 0);
*/
	Setup_dnd(dl_table, dragged_to_download_list);
	gtk_widget_show(dl_table);
}

void ListEntry::get_Dl_clist_lock()
{
	pthread_mutex_lock(&dl_clist_lock);
}

void ListEntry::release_Dl_clist_lock()
{
	pthread_mutex_unlock(&dl_clist_lock);
}

void ListEntry::freezeDlCList() {
//	gtk_clist_freeze(GTK_CLIST(dl_clist));
}

void ListEntry::thawDlCList() {
//	gtk_clist_thaw(GTK_CLIST(dl_clist));
}

void ListEntry::Send_start_signal()
{
	//pthread_cond_broadcast(&dl_clist_signal);
	thread_manager->ManageThread();
}

void ListEntry::Signal_wait()
{
	//pthread_cond_wait(&dl_clist_signal, &dl_clist_lock);
}

ThreadManager *ListEntry::getThreadManager()
{
	return thread_manager;
}

void ListEntry::setThreadManager(ThreadManager *thread_manager_in)
{
	// set halt flag and wait for join then ...
	delete thread_manager;
	thread_manager = thread_manager_in;
}

ItemManager *ListEntry::getItemManager()
{
	return item_manager;
}

void ListEntry::setItemManager(ItemManager *item_manager_in)
{
	item_manager = item_manager_in;
}

const string& ListEntry::getName() const
{
	return name;
}

void ListEntry::setName(const string& name_in)
{
	name = name_in;
}

void ListEntry::setThreadLimit(int n_thread_in)
{
	if(n_thread_in > g_threadLimit) {
		n_thread = g_threadLimit;
	} else {
		n_thread = n_thread_in;
	}

	thread_manager->ManageThread(n_thread);
}

int ListEntry::getThreadLimit() const
{
	return n_thread;
}

GtkWidget *ListEntry::ret_Dl_clist()
{
	return dl_table;
}

int ListEntry::Append_dl_item(char *clist_item[], ItemCell *itemcell)
{
	std::cout << "Append Item" << std::endl;
	items.push_back(itemcell);
	GtkTreeIter   iter;	
	gtk_list_store_append(dl_model, &iter);
	//int rowindex = gtk_clist_append(GTK_CLIST(dl_clist), clist_item);
	//gtk_clist_set_row_data(GTK_CLIST(dl_clist), rowindex, itemcell);
	item_manager->regist_item_back(itemcell);
	//TODO: update row data
	updateRow(items.size()-1);

	return items.size()-1;
}

int ListEntry::Insert_dl_item(char *clist_item[], ItemCell *itemcell, ItemCell *itemcell_base)
{
	std::cout << "Insert Item" << std::endl;
/*	int destindex = gtk_clist_find_row_from_data(GTK_CLIST(dl_clist), itemcell_base)+1;
	int rowindex = gtk_clist_insert(GTK_CLIST(dl_clist), destindex, clist_item);
	gtk_clist_set_row_data(GTK_CLIST(dl_clist), rowindex, itemcell);*/
	std::vector<ItemCell*>::iterator it = std::find(items.begin(), items.end(), itemcell_base);
	++it;
	items.insert(it, itemcell);
	item_manager->regist_item_back(itemcell);
	//TODO: update row data
	std::size_t pos = it - items.begin();
	updateRow(pos);
	return pos;
}

ItemCell *ListEntry::ret_Default_item()
{
	return default_item;
}

const Options& ListEntry::ret_Options() const
{
	return default_item->ret_Options_opt();
}

void ListEntry::set_Options(const Options& options_in)
{
	default_item->set_Options_opt(options_in);
}


ItemCell *ListEntry::Get_next_item()
{
	ItemCell *itemCellNext = NULL;

	pthread_mutex_lock(&g_appLock);

	for(size_t rowindex = 0; rowindex < items.size(); ++rowindex) {
		ItemCell *itemCell = items[rowindex];
		if(itemCell->ret_Status() == ItemCell::ITEM_READY) {
			itemCell->Open_Desc();
			itemCell->set_Status(ItemCell::ITEM_INUSE);
			itemCellNext = itemCell;
			break;
		} else if(itemCell->ret_Status() == ItemCell::ITEM_READY_AGAIN) {
			itemCell->Open_Desc();
			itemCell->set_Status(ItemCell::ITEM_INUSE_AGAIN);
			itemCellNext = itemCell;
			break;
		} else if(itemCell->ret_Status() == ItemCell::ITEM_READY_CONCAT) {
			itemCell->Open_Desc();
			itemCell->set_Status(ItemCell::ITEM_INUSE_CONCAT);
			itemCellNext = itemCell;
			break;
		}
	}
	pthread_mutex_unlock(&g_appLock);

	return itemCellNext;
}

// 状態アイコンをセットする
void ListEntry::setStatusIcon(GdkPixbuf *pixmaps[], GdkBitmap *bitmaps[]) {
	pthread_mutex_lock(&dl_clist_lock);
	for(int index = 0; index < ICON_TOTAL; ++index) {
		statusIcon[index] = pixmaps[index];
		statusIconMask[index] = bitmaps[index];
	}
//	gtk_clist_freeze(GTK_CLIST(dl_clist));
	for(size_t rowindex = 0; rowindex < items.size(); ++rowindex) {
		//ItemCell *itemcell = items[rowindex];
		//Set_clist_column__icon(rowindex, itemcell->ret_Status());
		updateRow(rowindex, COL_STATUS);
	}
	// here

//	GtkWidget *pixmap = gtk_pixmap_new(*pixmaps, *bitmaps);
//	int height = pixmap->requisition.height;
//	GtkStyle *style = gtk_widget_get_style(dl_clist);
	//int fontHeight = gdk_char_height(style->font, 'A');
//  if(height < fontHeight) {
//    height = fontHeight;
//  }

//	gtk_clist_set_row_height(GTK_CLIST(dl_clist),height);
	//delete pixmap;
//	gtk_clist_thaw(GTK_CLIST(dl_clist));
	pthread_mutex_unlock(&dl_clist_lock);
}

// サイズの表示方法
// flagHumanReadble : true (kbyte or mbyte)
//                    false (byte)
void ListEntry::setSizeDisplayStyle(bool flagHumanReadable) {
	pthread_mutex_lock(&dl_clist_lock);
//	gtk_clist_freeze(GTK_CLIST(dl_clist));
    useHumanReadable = flagHumanReadable;
	for(size_t rowindex = 0; rowindex < items.size(); ++rowindex) {
		updateRow(rowindex,COL_TOTSIZE);
		updateRow(rowindex,COL_CURSIZE);
	}
//	gtk_clist_thaw(GTK_CLIST(dl_clist));
	pthread_mutex_unlock(&dl_clist_lock);
}


// update each column
void ListEntry::Set_clist_column__crc(int rowindex, const string& crc_string)
{
	GtkTreeIter iter;
	gtk_tree_model_iter_nth_child (GTK_TREE_MODEL(dl_model), &iter, NULL, rowindex);
	gtk_list_store_set (dl_model, &iter, COL_CRC, crc_string.c_str(), -1);
//	gtk_clist_set_text(GTK_CLIST(dl_clist), rowindex, COL_CRC, crc_string.c_str());
}

void ListEntry::Set_clist_column__md5(int rowindex, const string& md5String)
{
	GtkTreeIter iter;
	gtk_tree_model_iter_nth_child (GTK_TREE_MODEL(dl_model), &iter, NULL, rowindex);
	gtk_list_store_set (dl_model, &iter, COL_MD5, md5String.c_str(), -1);
//	gtk_clist_set_text(GTK_CLIST(dl_clist), rowindex, COL_MD5, md5String.c_str());
}

void ListEntry::Set_clist_column__cursize(int rowindex, const string& size_string)
{
	GtkTreeIter iter;
	gtk_tree_model_iter_nth_child (GTK_TREE_MODEL(dl_model), &iter, NULL, rowindex);
	gtk_list_store_set (dl_model, &iter, COL_CURSIZE, size_string.c_str(), -1);
//	gtk_clist_set_text(GTK_CLIST(dl_clist), rowindex, COL_CURSIZE, size_string.c_str());
}

/*void ListEntry::Set_clist_column__totsize(int rowindex, const string& size_string)
{
	GtkTreeIter iter;
	gtk_tree_model_iter_nth_child (GTK_TREE_MODEL(dl_model), &iter, NULL, rowindex);
	gtk_list_store_set (dl_model, &iter, COL_TOTSIZE, size_string.c_str(), -1);
//	gtk_clist_set_text(GTK_CLIST(dl_clist), rowindex, COL_TOTSIZE, size_string.c_str());
}
*/
void ListEntry::Set_clist_column__progress(int rowindex, int progress)
{
	int index = progress/2;
	if(index > 50) {
		index = 0;
	}
	GtkTreeIter iter;
	gtk_tree_model_iter_nth_child (GTK_TREE_MODEL(dl_model), &iter, NULL, rowindex);
//	gtk_list_store_set (dl_model, &iter, 0, sg_progressBar[0], -1);
	gtk_list_store_set (dl_model, &iter, COL_PROGRESS, sg_progressBar[index], -1);
//	gtk_clist_set_pixmap(GTK_CLIST(dl_clist), rowindex, COL_PROGRESS, sg_progressBar[index], sg_progressBarMask[index]);
}

void ListEntry::Set_clist_column__filename(int rowindex, const string& filename)
{
	string fix;
	if(filename.empty()) {
		fix = _("<directory>");
	} else {
		fix = filename;
	}
	GtkTreeIter iter;
	gtk_tree_model_iter_nth_child (GTK_TREE_MODEL(dl_model), &iter, NULL, rowindex);
	gtk_list_store_set (dl_model, &iter, COL_FILENAME, fix.c_str(), -1);
//	gtk_clist_set_text(GTK_CLIST(dl_clist), rowindex, COL_FILENAME, fix.c_str());
}

void ListEntry::Set_clist_column__speed(int rowindex, const string& speed_string)
{
	GtkTreeIter iter;
	gtk_tree_model_iter_nth_child (GTK_TREE_MODEL(dl_model), &iter, NULL, rowindex);
	gtk_list_store_set (dl_model, &iter, COL_SPEED, speed_string.c_str(), -1);
//	gtk_clist_set_text(GTK_CLIST(dl_clist), rowindex, COL_SPEED, speed_string.c_str());
}

void ListEntry::Set_clist_column__rtime(int rowindex, const string& rtime_string)
{
	GtkTreeIter iter;
	gtk_tree_model_iter_nth_child (GTK_TREE_MODEL(dl_model), &iter, NULL, rowindex);
	gtk_list_store_set (dl_model, &iter, COL_RTIME, rtime_string.c_str(), -1);
//	gtk_clist_set_text(GTK_CLIST(dl_clist), rowindex, COL_RTIME, rtime_string.c_str());
}

void ListEntry::Set_clist_column__rec(int rowindex, const string& rec_string)
{
	GtkTreeIter iter;
	gtk_tree_model_iter_nth_child (GTK_TREE_MODEL(dl_model), &iter, NULL, rowindex);
	gtk_list_store_set (dl_model, &iter, COL_REC, rec_string.c_str(), -1);
	//gtk_clist_set_text(GTK_CLIST(dl_clist), rowindex, COL_REC, rec_string.c_str());
}

/*void ListEntry::Set_clist_column__status(int rowindex, const string& status_string)
{
	GtkTreeIter iter;
	gtk_tree_model_iter_nth_child (GTK_TREE_MODEL(dl_model), &iter, NULL, rowindex);
	gtk_list_store_set (dl_model, &iter, COL_STATUS, status_string.c_str(), -1);
	//gtk_clist_set_text(GTK_CLIST(dl_clist), rowindex, COL_STATUS, status_string.c_str());
}

void ListEntry::Set_clist_column__icon(int rowindex, ItemCell::ItemStatusType status)
{
	GtkTreeIter iter;
	gtk_tree_model_iter_nth_child (GTK_TREE_MODEL(dl_model), &iter, NULL, rowindex);
	switch(status) {
	case ItemCell::ITEM_READY:
	case ItemCell::ITEM_READY_AGAIN:
	case ItemCell::ITEM_INUSE:
	case ItemCell::ITEM_INUSE_AGAIN:
		//case ITEM_INUSE_CONCAT:
		gtk_list_store_set (dl_model, &iter, COL_ICON, statusIcon[ICON_READY], -1);//, statusIconMask[ICON_READY]);
		gtk_list_store_set (dl_model, &iter, COL_STATUS, _("READY"), -1);
		break;
	case ItemCell::ITEM_DOWNLOAD_PARTIAL:
		gtk_list_store_set (dl_model, &iter, COL_ICON, statusIcon[ICON_DIVIDE], -1);
		gtk_list_store_set (dl_model, &iter, COL_STATUS, _("DIVIDED"), -1);
		break;
	case ItemCell::ITEM_READY_CONCAT:
	case ItemCell::ITEM_INUSE_CONCAT:
		//icon_divide = gdk_pixmap_ref(icon_divide);// added 2001/3/8
		//icon_divide_mask = gdk_bitmap_ref(icon_divide_mask);

		gtk_list_store_set (dl_model, &iter, COL_ICON, statusIcon[ICON_DIVIDE], -1);
		gtk_list_store_set (dl_model, &iter, COL_STATUS, _("READY TO CONCAT"), -1);
		break;

	case ItemCell::ITEM_CRCERROR:
		//icon_error = gdk_pixmap_ref(icon_error);// added 2001/3/8
		//icon_error_mask = gdk_bitmap_ref(icon_error_mask);

		gtk_list_store_set (dl_model, &iter, COL_ICON, statusIcon[ICON_ERROR], -1);
		gtk_list_store_set (dl_model, &iter, COL_STATUS, _("CRC ERROR"), -1);
		break;
	case ItemCell::ITEM_EXECERROR:
		//icon_error = gdk_pixmap_ref(icon_error);// added 2001/3/8
		//icon_error_mask = gdk_bitmap_ref(icon_error_mask);

		gtk_list_store_set (dl_model, &iter, COL_ICON, statusIcon[ICON_ERROR], -1);
		gtk_list_store_set (dl_model, &iter, COL_STATUS, _("EXEC ERROR"), -1);
		break;
	case ItemCell::ITEM_ERROR:
		//icon_error = gdk_pixmap_ref(icon_error);// added 2001/3/8
		//icon_error_mask = gdk_bitmap_ref(icon_error_mask);

		gtk_list_store_set (dl_model, &iter, COL_ICON, statusIcon[ICON_ERROR], -1);
		gtk_list_store_set (dl_model, &iter, COL_STATUS, _("DOWNLOAD ERROR"), -1);
		break;

	case ItemCell::ITEM_STOP:
		//icon_stop = gdk_pixmap_ref(icon_stop);// added 2001/3/8
		//icon_stop_mask = gdk_bitmap_ref(icon_stop_mask);

		gtk_list_store_set (dl_model, &iter, COL_ICON, statusIcon[ICON_STOP], -1);
		gtk_list_store_set (dl_model, &iter, COL_STATUS, _("STOPPED"), -1);
		break;
	case ItemCell::ITEM_LOCK:
		//icon_lock = gdk_pixmap_ref(icon_lock);// added 2001/3/8
		//icon_lock_mask = gdk_bitmap_ref(icon_lock_mask);

		gtk_list_store_set (dl_model, &iter, COL_ICON, statusIcon[ICON_LOCK], -1);
		gtk_list_store_set (dl_model, &iter, COL_STATUS, _("LOCKED"), -1);
		break;
	case ItemCell::ITEM_DOWNLOAD:
	case ItemCell::ITEM_DOWNLOAD_AGAIN:
	case ItemCell::ITEM_DOWNLOAD_INTERNAL_AGAIN:
		//icon_download = gdk_pixmap_ref(icon_download);// added 2001/3/8
		//icon_download_mask = gdk_bitmap_ref(icon_download_mask);

		gtk_list_store_set (dl_model, &iter, COL_ICON, statusIcon[ICON_DOWNLOAD], -1);
		gtk_list_store_set (dl_model, &iter, COL_STATUS, _("DOWNLOADING"), -1);
		break;

	case ItemCell::ITEM_COMPLETE:
		//icon_complete = gdk_pixmap_ref(icon_complete);// added 2001/3/8
		//icon_complete_mask = gdk_bitmap_ref(icon_complete_mask);

		gtk_list_store_set (dl_model, &iter, COL_ICON, statusIcon[ICON_COMPLETE], -1);
		gtk_list_store_set (dl_model, &iter, COL_STATUS, _("DOWNLOAD COMPLETE"), -1);
		break;
	default:
		break;
	}
}*/

void ListEntry::Set_clist_column__try(int rowindex, int currentCount, int maxCount)
{
	string try_string = itos(currentCount)+'/';
	if(maxCount == -1) {
		try_string += '-';
	} else {
		try_string += itos(maxCount);
	}
	GtkTreeIter iter;
	gtk_tree_model_iter_nth_child (GTK_TREE_MODEL(dl_model), &iter, NULL, rowindex);
	gtk_list_store_set (dl_model, &iter, COL_RETRY, try_string.c_str(), -1);
}

void ListEntry::Set_clist_column__url(int rowindex, const string& url_string)
{
	std::cout << "Set URL[" << rowindex << "]: " << url_string << std::endl;
	GtkTreeIter iter;
	gtk_tree_model_iter_nth_child (GTK_TREE_MODEL(dl_model), &iter, NULL, rowindex);
	gtk_list_store_set (dl_model, &iter, COL_URL, url_string.c_str(), -1);
}

void ListEntry::Set_clist_column__save(int rowindex, const string& save_string)
{
	GtkTreeIter iter;
	gtk_tree_model_iter_nth_child (GTK_TREE_MODEL(dl_model), &iter, NULL, rowindex);
	gtk_list_store_set (dl_model, &iter, COL_SAVE, save_string.c_str(), -1);
}

void ListEntry::updateRow(int rowIndex, int columnIndex)
{
	GtkTreeIter iter;
	gtk_tree_model_iter_nth_child (GTK_TREE_MODEL(dl_model), &iter, NULL, rowIndex);
	ItemCell* item = items [rowIndex];

	if (columnIndex == COL_ICON || columnIndex == COL_STATUS || columnIndex == -1)
	{
		switch(item->ret_Status()) {
		case ItemCell::ITEM_READY:
		case ItemCell::ITEM_READY_AGAIN:
		case ItemCell::ITEM_INUSE:
		case ItemCell::ITEM_INUSE_AGAIN:
			//case ITEM_INUSE_CONCAT:
			gtk_list_store_set (dl_model, &iter, COL_ICON, statusIcon[ICON_READY], -1);//, statusIconMask[ICON_READY]);
			gtk_list_store_set (dl_model, &iter, COL_STATUS, _("READY"), -1);
			break;

		case ItemCell::ITEM_DOWNLOAD_PARTIAL:
			gtk_list_store_set (dl_model, &iter, COL_ICON, statusIcon[ICON_DIVIDE], -1);
			gtk_list_store_set (dl_model, &iter, COL_STATUS, _("DIVIDED"), -1);
			break;

		case ItemCell::ITEM_READY_CONCAT:
		case ItemCell::ITEM_INUSE_CONCAT:
			gtk_list_store_set (dl_model, &iter, COL_ICON, statusIcon[ICON_DIVIDE], -1);
			gtk_list_store_set (dl_model, &iter, COL_STATUS, _("READY TO CONCAT"), -1);
			break;

		case ItemCell::ITEM_CRCERROR:
			gtk_list_store_set (dl_model, &iter, COL_ICON, statusIcon[ICON_ERROR], -1);
			gtk_list_store_set (dl_model, &iter, COL_STATUS, _("CRC ERROR"), -1);
			break;
		case ItemCell::ITEM_EXECERROR:
			gtk_list_store_set (dl_model, &iter, COL_ICON, statusIcon[ICON_ERROR], -1);
			gtk_list_store_set (dl_model, &iter, COL_STATUS, _("EXEC ERROR"), -1);
			break;
		case ItemCell::ITEM_ERROR:
			gtk_list_store_set (dl_model, &iter, COL_ICON, statusIcon[ICON_ERROR], -1);
			gtk_list_store_set (dl_model, &iter, COL_STATUS, _("DOWNLOAD ERROR"), -1);
			break;

		case ItemCell::ITEM_STOP:
			gtk_list_store_set (dl_model, &iter, COL_ICON, statusIcon[ICON_STOP], -1);
			gtk_list_store_set (dl_model, &iter, COL_STATUS, _("STOPPED"), -1);
			break;

		case ItemCell::ITEM_LOCK:
			gtk_list_store_set (dl_model, &iter, COL_ICON, statusIcon[ICON_LOCK], -1);
			gtk_list_store_set (dl_model, &iter, COL_STATUS, _("LOCKED"), -1);
			break;

		case ItemCell::ITEM_DOWNLOAD:
		case ItemCell::ITEM_DOWNLOAD_AGAIN:
		case ItemCell::ITEM_DOWNLOAD_INTERNAL_AGAIN:
			gtk_list_store_set (dl_model, &iter, COL_ICON, statusIcon[ICON_DOWNLOAD], -1);
			gtk_list_store_set (dl_model, &iter, COL_STATUS, _("DOWNLOADING"), -1);
			break;

		case ItemCell::ITEM_COMPLETE:
			gtk_list_store_set (dl_model, &iter, COL_ICON, statusIcon[ICON_COMPLETE], -1);
			gtk_list_store_set (dl_model, &iter, COL_STATUS, _("DOWNLOAD COMPLETE"), -1);
			break;

		default:
			break;
		}
	}
	if (columnIndex == COL_FILENAME || columnIndex == -1)
	{
		string fix;
		if(item->ret_Filename().empty()) {
			fix = _("<directory>");
		} else {
			fix = item->ret_Filename();
		}
		gtk_list_store_set (dl_model, &iter, COL_FILENAME, fix.c_str(), -1);
	}
	if (columnIndex == COL_CURSIZE || columnIndex == -1)
	{
		if (useHumanReadable)
		{
			gtk_list_store_set (dl_model, &iter, COL_CURSIZE, get_human_readable_size(item->ret_Size_Current()).c_str(), -1);
		} else {
			gtk_list_store_set (dl_model, &iter, COL_CURSIZE, itos(item->ret_Size_Current()).c_str(), -1);
		}
	}
	if (columnIndex == COL_TOTSIZE || columnIndex == -1)
	{
		if (useHumanReadable)
		{
			gtk_list_store_set (dl_model, &iter, COL_TOTSIZE, get_human_readable_size(item->ret_Size_Total()).c_str(), -1);
		} else {
			gtk_list_store_set (dl_model, &iter, COL_TOTSIZE, itos(item->ret_Size_Total()).c_str(), -1);
		}
	}
	if (columnIndex == COL_PROGRESS || columnIndex == -1)
	{
		int progress = 0;
		if(item->ret_Size_Total() != 0) {
		  progress = (int)(100.0*((float)item->ret_Size_Current()/item->ret_Size_Total()));//fixed 2001/3/18
		}
		int index = progress/2;
		if(index > 50) {
			index = 0;
		}
		gtk_list_store_set (dl_model, &iter, COL_PROGRESS, sg_progressBar[index], -1);
	}
	if (columnIndex == COL_SPEED || columnIndex == -1)
	{//TODO:
		//gtk_list_store_set (dl_model, &iter, COL_SPEED, speed_string.c_str(), -1);
		gtk_list_store_set (dl_model, &iter, COL_SPEED, "", -1);
	}
	if (columnIndex == COL_RTIME || columnIndex == -1)
	{//TODO
		gtk_list_store_set (dl_model, &iter, COL_RTIME, "", -1);
	}
	if (columnIndex == COL_RETRY || columnIndex == -1)
	{
		string try_string = itos(item->ret_Count())+'/';
		if(item->ret_Options_opt().ret_Retry() == -1) {
			try_string += '-';
		} else {
			try_string += itos(item->ret_Options_opt().ret_Retry());
		}
		gtk_list_store_set (dl_model, &iter, COL_RETRY, try_string.c_str(), -1);
	}
	if (columnIndex == COL_REC || columnIndex == -1)
	{//TODO: the item should take care to return the correct value per protocol
		if(item->ret_URL_Container().ret_Protocol() == "http:"
#ifdef HAVE_OPENSSL
		   || item->ret_URL_Container().ret_Protocol() == "https:"
#endif // HAVE_OPENSSL
		   ) {
			gtk_list_store_set (dl_model, &iter, COL_REC, itos(item->ret_Options().ret_recurse_count()).c_str(), -1);
		} else {
			gtk_list_store_set (dl_model, &iter, COL_REC, itos(item->ret_Options().ret_FTP_recurse_count()).c_str(), -1);
		}
	}
	if (columnIndex == COL_CRC || columnIndex == -1)
	{//TODO
		gtk_list_store_set (dl_model, &iter, COL_CRC, "", -1);
	}
	if (columnIndex == COL_MD5 || columnIndex == -1)
	{//TODO
		gtk_list_store_set (dl_model, &iter, COL_MD5, "", -1);
	}
	if (columnIndex == COL_SAVE || columnIndex == -1)
	{
		gtk_list_store_set (dl_model, &iter, COL_SAVE, item->ret_Options_opt().ret_Store_Dir().c_str(), -1);
	}
	if (columnIndex == COL_URL || columnIndex == -1)
	{
		gtk_list_store_set (dl_model, &iter, COL_URL, item->ret_URL().c_str(), -1);
	}
}

// deletes matched items. unmatched items are returned
void ListEntry::setMD5List(list<CRCList*>& md5List) {
	pthread_mutex_lock(&dl_clist_lock);
//	gtk_clist_freeze(GTK_CLIST(dl_clist));
	for(size_t rowindex = 0; rowindex < items.size(); ++rowindex) {
		ItemCell *itemcell = items[rowindex];
		for(list<CRCList*>::iterator md5List_itr = md5List.begin(); md5List_itr != md5List.end(); ++md5List_itr) {
			if(itemcell->ret_URL_Container().ret_Filename() == (*md5List_itr)->ref_Filename()) {
				itemcell->set_md5string((*md5List_itr)->ref_CRC_string());
				Set_clist_column__md5(rowindex, (*md5List_itr)->ref_CRC_string());
				// set md5 to its dedicated column
				md5List.remove(*md5List_itr);
				delete *md5List_itr;
				break;
			}
		}
	}
//	gtk_clist_thaw(GTK_CLIST(dl_clist));
	pthread_mutex_unlock(&dl_clist_lock);
}

list<int> ListEntry::getActiveRowList() {
	list<ItemCell *> itemCellList = thread_manager->getActiveItemCell();
	list<int> rowList;

	for(list<ItemCell *>::iterator itr = itemCellList.begin();
	        itr != itemCellList.end(); ++itr) {
		ItemCell *itemcell = *itr;
//		int rowindex = gtk_clist_find_row_from_data(GTK_CLIST(dl_clist),
//		               itemcell);
		size_t rowindex = distance(items.begin(), std::find(items.begin(), items.end(), itemcell) );
		GtkTreeIter iter;
		gtk_tree_model_iter_nth_child (GTK_TREE_MODEL(dl_model), &iter, NULL, rowindex);
		//if(!g_list_find(GTK_CLIST(dl_clist)->selection, (void *)rowindex) && rowindex > 0) {
		if (gtk_tree_selection_iter_is_selected(gtk_tree_view_get_selection (GTK_TREE_VIEW(dl_table)), &iter)) {
			rowList.push_back(rowindex);
		}
	}
	rowList.sort();

	return rowList;
}

ItemCell *ListEntry::getItemCellByRow(int rowindex) const {
	return items[rowindex];
}

// deletes matched items. unmatched items are returned
void ListEntry::setCRCList(list<CRCList*>& crcList) {
	pthread_mutex_lock(&dl_clist_lock);
	//gtk_clist_freeze(GTK_CLIST(dl_clist));
	for(size_t rowindex = 0; rowindex < items.size(); ++rowindex) {
		ItemCell *itemcell = items[rowindex];

		for(list<CRCList*>::iterator crc_list_itr = crcList.begin(); crc_list_itr != crcList.end(); ++crc_list_itr) {
			if(itemcell->ret_URL_Container().ret_Filename() == (*crc_list_itr)->ref_Filename()) {
				switch((*crc_list_itr)->ref_CRC_string().size()) {
				case 8:
					itemcell->set_CRC(stoui((*crc_list_itr)->ref_CRC_string(), 16));
					itemcell->set_CRC_Type(ItemCell::CRC_32);
					Set_clist_column__crc(rowindex, (*crc_list_itr)->ref_CRC_string());
					crcList.remove(*crc_list_itr);
					delete *crc_list_itr;
					break;
				case 4:
					itemcell->set_CRC(stoui((*crc_list_itr)->ref_CRC_string(), 16));
					itemcell->set_CRC_Type(ItemCell::CRC_16);

					Set_clist_column__crc(rowindex, (*crc_list_itr)->ref_CRC_string());
					crcList.remove(*crc_list_itr);
					delete *crc_list_itr;
					break;
				default:
					cerr << "Warning: unsupported CRC format" << endl;
				}
				break;
			}
		}
	}
	//gtk_clist_thaw(GTK_CLIST(dl_clist));
	pthread_mutex_unlock(&dl_clist_lock);
}
