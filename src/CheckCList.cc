//
//  aria - yet another download tool
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

// $Id: CheckCList.cc,v 1.6 2001/09/11 13:13:31 tujikawa Exp $

#include <iostream>
#include "aria.h"
#include "gui_utils.h"

static void CheckCList_selectRow_cb(GtkWidget *clist,
                                    int row,
                                    int column,
                                    GdkEventButton *event,
                                    gpointer data)
{
    static bool enterflag = false;
    if (enterflag == true || event == NULL) return;
    enterflag = true;

    switch (event->button) {
        case 1:
            if (event->state & GDK_CONTROL_MASK) {
            } else if (event->state & GDK_SHIFT_MASK) {
                int nearestSelectedRow = findNearestSelectedRow(clist, row);
                if (nearestSelectedRow < row) {
                    for (int index = nearestSelectedRow + 1; index < row; ++index) {
                        gtk_clist_select_row(GTK_CLIST(clist), index, 0);
                    }
                } else {
                    for (int index = row + 1; index < nearestSelectedRow; ++index) {
                        gtk_clist_select_row(GTK_CLIST(clist), index, 0);
                    }
                }
            } else {// with no mask
            }
            break;
        case 3:
            gtk_clist_unselect_row(GTK_CLIST(clist), row, 0);
            break;
    }
    enterflag = false;
    return;
}

static void CheckCList_unselectRow_cb(GtkWidget *clist,
                                      int row,
                                      int column,
                                      GdkEventButton *event,
                                      gpointer data)
{
    static bool enterflag = false;

    if (enterflag == true || event == NULL) return;
    enterflag = true;
    /*
    switch(event->button) {
    case 1:
      break;
    case 3:
      break;
    }
    */
    enterflag = false;
    return;
}

static int row;

static gboolean right_click_cb(GtkWidget *widget,
                               GdkEvent *event,
                               GtkWidget *popup_menu)
{
    if (event->button.button == 3) {
        row = -1;
        int column;
        gtk_clist_get_selection_info(GTK_CLIST(widget),
                                     (int)event->button.x, (int)event->button.y,
                                     &row, &column);
        if (row > -1) {
            gtk_menu_popup (GTK_MENU (popup_menu), NULL, NULL,
                            NULL, NULL, 3, event->button.time);
            return TRUE;
        }
    }
    return FALSE;
}

static void Delete_item(GtkWidget *w, GtkWidget *clist)
{
    gtk_clist_remove(GTK_CLIST(clist), row);
}

GtkWidget *Create_popup_menu(GtkWidget **clist_ptr)
{
    GtkWidget *popup_menu = gtk_menu_new();

    GtkWidget *delete_item = gtk_menu_item_new_with_label(_("Delete"));

    gtk_menu_append(GTK_MENU(popup_menu), delete_item);

    // signal handling
    g_signal_connect(GTK_OBJECT(delete_item), "activate",
                     GTK_SIGNAL_FUNC(Delete_item), GTK_OBJECT(*clist_ptr));

    gtk_widget_show(delete_item);

    GtkWidget *popup_item = gtk_menu_item_new_with_label(_("Popup menu"));
    gtk_widget_show(popup_item);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(popup_item), popup_menu);

    g_signal_connect_after(GTK_OBJECT(*clist_ptr), "button_press_event",
                           GTK_SIGNAL_FUNC(right_click_cb),
                           (gpointer)popup_menu);

    return popup_menu;
}

//check付きCList(仮)
GtkWidget *Create_CheckCList(GtkWidget **clist_ptr, char *titles[], int n_titles)
{
    // CList ウィジェットをパックするスクロールドウィンドウを作成する
    GtkWidget *scrolled_window = gtk_scrolled_window_new (NULL, NULL);

    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    gtk_widget_set_usize(scrolled_window, 300, 150);
    // CList を作成する。この例では n_titles 列を使う
    *clist_ptr = gtk_clist_new_with_titles(n_titles, titles);
    gtk_widget_set_name(*clist_ptr, "clist");
    gtk_clist_column_titles_passive(GTK_CLIST(*clist_ptr));
    gtk_clist_set_selection_mode(GTK_CLIST(*clist_ptr), GTK_SELECTION_MULTIPLE);

    // selection callbacks
    g_signal_connect(GTK_OBJECT(*clist_ptr), "select-row",
                     GTK_SIGNAL_FUNC(CheckCList_selectRow_cb),
                     NULL);
    g_signal_connect(GTK_OBJECT(*clist_ptr), "unselect-row",
                     GTK_SIGNAL_FUNC(CheckCList_unselectRow_cb),
                     NULL);

    // 境界に影を付ける必要などないが、そうすれば見栄えが良くなる :)
    gtk_clist_set_shadow_type(GTK_CLIST(*clist_ptr), GTK_SHADOW_ETCHED_OUT);

    // CList ウィジェットを垂直ボックスに加え、それを表示する
    gtk_container_add(GTK_CONTAINER(scrolled_window), *clist_ptr);
    gtk_widget_show(*clist_ptr);

    return scrolled_window;
}
