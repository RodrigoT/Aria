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

// $Id: ItemStatusDynamic.cc,v 1.24 2002/01/20 14:41:58 tujikawa Exp $

#include "ItemStatusDynamic.h"

extern bool Is_track_required();
extern bool Download_clear_sub(ItemCell *itemcell, ListEntry *listentry);
extern void Set_sensitive__no_item_selected();
extern void Set_sensitive__list_empty();
extern bool Is_track_required();
extern ListManager *g_listManager;
extern AppOption *g_appOption;

ItemStatusDynamic::ItemStatusDynamic(ItemCell *itemcell_in, ItemCell::ItemStatusType status_in, unsigned int retry_count_in, unsigned int size_current_in, unsigned int size_total_in) : ItemStatus(itemcell_in)
{
    size_current = size_current_in;
    size_total = size_total_in;
    retry_count = retry_count_in;
    status = status_in;
    speed = 0.0;
    avgSpeed = 0.0;
    delete_flag = false;
    update_flag = name_col | progress_col | crc_col | static_col;
}

ItemStatusDynamic::ItemStatusDynamic(ItemCell *itemcell_in, ItemCell::ItemStatusType status_in, unsigned int retry_count_in, unsigned int size_current_in, unsigned int size_total_in, float speed_in, float avgSpeed_in) : ItemStatus(itemcell_in)
{
    size_current = size_current_in;
    size_total = size_total_in;
    retry_count = retry_count_in;
    status = status_in;
    speed = speed_in;
    avgSpeed = avgSpeed_in;
    delete_flag = false;
    update_flag = speed_col | progress_col;
}

ItemStatusDynamic::~ItemStatusDynamic()
{
}

void ItemStatusDynamic::Update()
{
    ItemCell *itemcell = ret_ItemCell();

    if (!g_listManager->Search(listentry)) {
        return;
    }

    if (!listentry->getItemManager()->search_item(itemcell)) {
        //listentry->release_Dl_clist_lock();
        return;
    }

    listentry->get_Dl_clist_lock();

    size_t rowindex = listentry->getRowForCell(itemcell);

    if (update_flag & ItemStatusDynamic::name_col) {
        if (itemcell->Is_Partial()) {
            string line = "|->" + itemcell->ret_Filename();
            listentry->Set_clist_column__filename(rowindex, line);
        } else {
            listentry->Set_clist_column__filename(rowindex, itemcell->ret_Filename());
            // fix this
            listentry->Set_clist_column__md5(rowindex, itemcell->ret_md5string());
        }
    }
    if (update_flag & ItemStatusDynamic::progress_col) {
        int progress = 0;
        if (size_total != 0) {
            progress = (int)(100.0 * ((float)size_current / size_total)); //fixed 2001/3/18
        }
        listentry->Set_clist_column__progress(rowindex, progress);
        /*if(g_appOption->ret_use_size_human_readable()) {
          listentry->Set_clist_column__cursize(rowindex, get_human_readable_size(size_current));
          listentry->Set_clist_column__totsize(rowindex, get_human_readable_size(size_total));
        } else {
          listentry->Set_clist_column__cursize(rowindex, itos(size_current, true));
          listentry->Set_clist_column__totsize(rowindex, itos(size_total, true));
        }*/
        listentry->updateRow(rowindex, COL_TOTSIZE);
        listentry->updateRow(rowindex, COL_CURSIZE);
    }
    if (update_flag & ItemStatusDynamic::speed_col) {
        string speed_string = ftos(speed, 1) + "k";
        listentry->Set_clist_column__speed(rowindex, speed_string.c_str());

        int rtime = (int)((size_total - size_current) / avgSpeed / 1024);
        if (rtime < 0 || size_total == 0) {
            listentry->Set_clist_column__rtime(rowindex, "--:--:--");
        } else {
            int hour = rtime / 3600;
            int temp = rtime % 3600;
            int min = temp / 60;
            int sec = temp % 60;
            listentry->Set_clist_column__rtime(rowindex, itos(hour, 2, '0') + ":" + itos(min, 2, '0') + ":" + itos(sec, 2, '0'));
        }
    }
    if (update_flag & ItemStatusDynamic::static_col) {
        listentry->Set_clist_column__try(rowindex, retry_count, itemcell->ret_Options_opt().ret_Retry());
        listentry->Set_clist_column__save(rowindex, itemcell->ret_Options().ret_Store_Dir().c_str());
        listentry->Set_clist_column__url(rowindex, itemcell->ret_URL().c_str());
        if (itemcell->ret_URL_Container().ret_Protocol() == "http:"
#ifdef HAVE_OPENSSL
                || itemcell->ret_URL_Container().ret_Protocol() == "https:"
#endif // HAVE_OPENSSL
           ) {
            listentry->Set_clist_column__rec(rowindex, itos(itemcell->ret_Options().ret_recurse_count()));
        } else {
            listentry->Set_clist_column__rec(rowindex, itos(itemcell->ret_Options().ret_FTP_recurse_count()));
        }
    }
    //listentry->Set_clist_column__icon(rowindex, status);

    switch (status) {
        case ItemCell::ITEM_COMPLETE: {
            itemcell->get_Options_Lock();
            if (delete_flag && (itemcell->Is_Partial() ||
                                (itemcell->ret_Options_opt().ret_Delete_When_Finish() &&
                                 ((itemcell->ret_CRC_Type() != ItemCell::CRC_NONE ||
                                   (itemcell->ret_md5string().size() && !itemcell->ret_Options_opt().ret_no_crc_checking())) ||
                                  !itemcell->ret_Options_opt().ret_Dont_Delete_Without_CRC())))) {
                itemcell->release_Options_Lock();
                // freeze clist
                listentry->freezeDlCList();
                if (GTK_CLIST(listentry->ret_Dl_clist())->selection != NULL) {
                    gtk_clist_unselect_row(GTK_CLIST(listentry->ret_Dl_clist()), rowindex, 0);
                    if (GTK_CLIST(listentry->ret_Dl_clist())->selection == NULL &&
                            listentry == g_listManager->ret_Current_listentry()) {
                        Set_sensitive__no_item_selected();
                    }
                }

                if (Is_track_required()) {
                    Download_clear_sub(itemcell, listentry);
                } else {
                    bool vadj_flag = false;
                    float vadj = 0.0;
                    GtkAdjustment *adj = NULL;
                    if (gtk_clist_row_is_visible(GTK_CLIST(listentry->ret_Dl_clist()), GTK_CLIST(listentry->ret_Dl_clist())->rows - 1) == GTK_VISIBILITY_NONE) {
                        vadj_flag = true;
                        adj = gtk_clist_get_vadjustment(GTK_CLIST(listentry->ret_Dl_clist()));
                        vadj = adj->value;
                    }
                    Download_clear_sub(itemcell, listentry);
                    if (vadj_flag) {
                        gtk_adjustment_set_value(adj, vadj);
                    }
                }
                // unfreeze clist
                listentry->thawDlCList();

                if (GTK_CLIST(listentry->ret_Dl_clist())->rows == 0 &&
                        listentry == g_listManager->ret_Current_listentry()) {
                    Set_sensitive__list_empty();
                }
            } else {
                itemcell->release_Options_Lock();
            }
            break;
        }
        default:
            break;
    }
    listentry->release_Dl_clist_lock();
}


void ItemStatusDynamic::set_UpdateFlag(unsigned int flag)
{
    update_flag = flag;
}

void ItemStatusDynamic::set_DeleteFlag(bool flag)
{
    delete_flag = flag;
}
