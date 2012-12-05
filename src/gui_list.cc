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

// $Id: gui_list.cc,v 1.10 2002/02/13 12:09:24 tujikawa Exp $

#include <gdk/gdkkeysyms.h>
#include "ListManager.h"
#include "AppOption.h"
#include "Dialog.h"
#include "ShortCutKey.h"
#include "gui_utils.h"
#include "PasteWindow.h"

//// new item window
extern gboolean List_new_list(GtkWidget *w, gpointer data);
extern gboolean List_rename_list(GtkWidget *w, gpointer data);

extern void Create_new_list_window(GtkWidget *toplevel);
extern void Create_rename_list_window(GtkWidget *toplevel);

extern ItemCell *g_consoleItem;
extern AppOption *g_appOption;
extern ListManager *g_listManager;
extern Dialog *g_cDialog;
extern PasteWindow *g_pasteWindow;
// global variables

void List_delete_list()
{
    g_cDialog->hide();
    g_listManager->DeleteCurrentList();
    g_pasteWindow->updateTargetList();
    g_consoleItem->Send_message_to_gui(_("1 list deleted"), MSG_SYS_INFO);
}

gboolean List_delete_list_c(GtkWidget *w, gpointer data)
{
    if (g_listManager->ret_Length() == 1) return TRUE;

    if (g_appOption->ret_confirm_delete_list()) {
        g_cDialog->setup(_("Delete current list"),
                         _("Are you sure to delete the current list?"),
                         (gboolean ( *)(GtkWidget *, GtkWidget *))List_delete_list);
        g_cDialog->set_cancel_button_visible(false);
        g_cDialog->show();
    } else {
        List_delete_list();
    }
    return TRUE;
}

static gboolean List_shuffle_list(GtkWidget *w, gpointer data)
{
    ListEntry *listEntry = g_listManager->ret_Current_listentry();
    listEntry->get_Dl_clist_lock();
    gtk_clist_freeze(GTK_CLIST(listEntry->ret_Dl_clist()));
    for (int rowindex1 = 0; rowindex1 < GTK_CLIST(listEntry->ret_Dl_clist())->rows; ++rowindex1) {
        int rowindex2 = (int)((float)GTK_CLIST(listEntry->ret_Dl_clist())->rows * rand() / (RAND_MAX + 1.0));
        if (rowindex1 != rowindex2) {
            gtk_clist_swap_rows(GTK_CLIST(listEntry->ret_Dl_clist()),
                                rowindex1,
                                rowindex2);
        }
    }
    gtk_clist_moveto(GTK_CLIST(listEntry->ret_Dl_clist()), 0, 0, 0.0, 0.0);
    gtk_clist_thaw(GTK_CLIST(listEntry->ret_Dl_clist()));
    listEntry->release_Dl_clist_lock();

    return TRUE;
}

static void List_move_left(GtkWidget *w, gpointer data)
{
    g_listManager->Move_left();
}

static void List_move_right(GtkWidget *w, gpointer data)
{
    g_listManager->Move_right();
}

void List_previous_list(GtkWidget *w, gpointer data)
{
    gtk_notebook_prev_page(GTK_NOTEBOOK(g_listManager->ret_baseNotebook()));
}

void List_next_list(GtkWidget *w, gpointer data)
{
    gtk_notebook_next_page(GTK_NOTEBOOK(g_listManager->ret_baseNotebook()));
}

static gboolean List_sort_filename_asc(GtkWidget *w, gpointer data)
{
    ListEntry *listentry = g_listManager->ret_Current_listentry();

    listentry->Sort_by_filename(GTK_SORT_ASCENDING);
    return TRUE;
}

static gboolean List_sort_filename_des(GtkWidget *w, gpointer data)
{
    ListEntry *listentry = g_listManager->ret_Current_listentry();

    listentry->Sort_by_filename(GTK_SORT_DESCENDING);
    return TRUE;
}

static gboolean List_sort_extension_asc(GtkWidget *w, gpointer data)
{
    ListEntry *listentry = g_listManager->ret_Current_listentry();

    listentry->Sort_by_extension(GTK_SORT_ASCENDING);
    return TRUE;
}

static gboolean List_sort_extension_des(GtkWidget *w, gpointer data)
{
    ListEntry *listentry = g_listManager->ret_Current_listentry();

    listentry->Sort_by_extension(GTK_SORT_DESCENDING);
    return TRUE;
}

static gboolean List_sort_csize_asc(GtkWidget *w, gpointer data)
{
    ListEntry *listentry = g_listManager->ret_Current_listentry();

    listentry->Sort_by_csize(GTK_SORT_ASCENDING);
    return TRUE;
}

static gboolean List_sort_csize_des(GtkWidget *w, gpointer data)
{
    ListEntry *listentry = g_listManager->ret_Current_listentry();

    listentry->Sort_by_csize(GTK_SORT_DESCENDING);
    return TRUE;
}

static gboolean List_sort_tsize_asc(GtkWidget *w, gpointer data)
{
    ListEntry *listentry = g_listManager->ret_Current_listentry();

    listentry->Sort_by_tsize(GTK_SORT_ASCENDING);
    return TRUE;
}

static gboolean List_sort_tsize_des(GtkWidget *w, gpointer data)
{
    ListEntry *listentry = g_listManager->ret_Current_listentry();

    listentry->Sort_by_tsize(GTK_SORT_DESCENDING);
    return TRUE;
}

static gboolean List_sort_progress_asc(GtkWidget *w, gpointer data)
{
    ListEntry *listentry = g_listManager->ret_Current_listentry();

    listentry->Sort_by_progress(GTK_SORT_ASCENDING);
    return TRUE;
}

static gboolean List_sort_progress_des(GtkWidget *w, gpointer data)
{
    ListEntry *listentry = g_listManager->ret_Current_listentry();

    listentry->Sort_by_progress(GTK_SORT_DESCENDING);
    return TRUE;
}

static gboolean List_sort_retry_asc(GtkWidget *w, gpointer data)
{
    ListEntry *listentry = g_listManager->ret_Current_listentry();

    listentry->Sort_by_retry(GTK_SORT_ASCENDING);
    return TRUE;
}

static gboolean List_sort_retry_des(GtkWidget *w, gpointer data)
{
    ListEntry *listentry = g_listManager->ret_Current_listentry();

    listentry->Sort_by_retry(GTK_SORT_DESCENDING);
    return TRUE;
}

static gboolean List_sort_save_asc(GtkWidget *w, gpointer data)
{
    ListEntry *listentry = g_listManager->ret_Current_listentry();

    listentry->Sort_by_save(GTK_SORT_ASCENDING);
    return TRUE;
}

static gboolean List_sort_save_des(GtkWidget *w, gpointer data)
{
    ListEntry *listentry = g_listManager->ret_Current_listentry();

    listentry->Sort_by_save(GTK_SORT_DESCENDING);
    return TRUE;
}

static gboolean List_sort_url_asc(GtkWidget *w, gpointer data)
{
    ListEntry *listentry = g_listManager->ret_Current_listentry();

    listentry->Sort_by_url(GTK_SORT_ASCENDING);
    return TRUE;
}

static gboolean List_sort_url_des(GtkWidget *w, gpointer data)
{
    ListEntry *listentry = g_listManager->ret_Current_listentry();

    listentry->Sort_by_url(GTK_SORT_DESCENDING);
    return TRUE;
}

static gboolean List_sort_rec_asc(GtkWidget *w, gpointer data)
{
    ListEntry *listentry = g_listManager->ret_Current_listentry();

    listentry->Sort_by_rec(GTK_SORT_ASCENDING);
    return TRUE;
}

static gboolean List_sort_rec_des(GtkWidget *w, gpointer data)
{
    ListEntry *listentry = g_listManager->ret_Current_listentry();

    listentry->Sort_by_rec(GTK_SORT_DESCENDING);
    return TRUE;
}

static gboolean List_sort_status_asc(GtkWidget *w, gpointer data)
{
    ListEntry *listentry = g_listManager->ret_Current_listentry();

    listentry->Sort_by_status(GTK_SORT_ASCENDING);
    return TRUE;
}

static gboolean List_sort_status_des(GtkWidget *w, gpointer data)
{
    ListEntry *listentry = g_listManager->ret_Current_listentry();

    listentry->Sort_by_status(GTK_SORT_DESCENDING);
    return TRUE;
}

static void Create_sort_menu(GtkWidget *menu,
                             char *label,
                             GCallback AscFunc,
                             GCallback DecFunc)
{
    // Sort by Filename
    GtkWidget *sortByItem = GTK_create_menu_item_with_icon(menu,
                                                           label,
                                                           NULL,
                                                           NULL);
    GtkWidget *sortByMenu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(sortByItem),
                              sortByMenu);

    //GtkWidget *sortByAscItem =
    GTK_create_menu_item_with_icon(sortByMenu,
                                   _("Ascending"),
                                   AscFunc,
                                   NULL);
    //GtkWidget *sortByDecItem =
    GTK_create_menu_item_with_icon(sortByMenu,
                                   _("Descending"),
                                   DecFunc,
                                   NULL);
}


// ファイルメニューを作成
void Create_list_menu(GtkWidget *toplevel, GtkWidget *menu_bar, GtkAccelGroup *accel_group)
{
    GtkWidget *menu;
    GtkWidget *root_item;

    GtkWidget *newList_item;
    GtkWidget *deleteList_item;
    GtkWidget *renameList_item;
    GtkWidget *moveLeftList_item;
    GtkWidget *moveRightList_item;
    GtkWidget *previousList_item;
    GtkWidget *nextList_item;
    GtkWidget *shuffleList_item;

    //create file menu
    menu = gtk_menu_new();

    newList_item = GTK_create_menu_item_with_icon(menu,
                                                  _("New list"),
                                                  GTK_SIGNAL_FUNC(List_new_list),
                                                  NULL,
                                                  accel_group,
                                                  SC_NEWLIST,
                                                  SCM_NEWLIST);

    deleteList_item = GTK_create_menu_item_with_icon(menu,
                                                     _("Delete current list"),
                                                     GTK_SIGNAL_FUNC(List_delete_list_c),
                                                     NULL,
                                                     accel_group,
                                                     SC_DELLIST,
                                                     SCM_DELLIST);

    GTK_create_menu_separator(menu);

    renameList_item = GTK_create_menu_item_with_icon(menu,
                                                     _("Rename current list"),
                                                     GTK_SIGNAL_FUNC(List_rename_list),
                                                     NULL,
                                                     accel_group,
                                                     SC_RENAMELIST,
                                                     SCM_RENAMELIST);

    GTK_create_menu_separator(menu);

    moveLeftList_item = GTK_create_menu_item_with_icon(menu,
                                                       _("Move left"),
                                                       GTK_SIGNAL_FUNC(List_move_left),
                                                       NULL,
                                                       accel_group,
                                                       SC_MOVELEFT,
                                                       SCM_MOVELEFT);

    moveRightList_item = GTK_create_menu_item_with_icon(menu,
                                                        _("Move right"),
                                                        GTK_SIGNAL_FUNC(List_move_right),
                                                        NULL,
                                                        accel_group,
                                                        SC_MOVERIGHT,
                                                        SCM_MOVERIGHT);

    GTK_create_menu_separator(menu);

    previousList_item = GTK_create_menu_item_with_icon(menu,
                                                       _("Previous list"),
                                                       GTK_SIGNAL_FUNC(List_previous_list),
                                                       NULL,
                                                       accel_group,
                                                       SC_PREVIOUSLIST,
                                                       SCM_PREVIOUSLIST);

    nextList_item = GTK_create_menu_item_with_icon(menu,
                                                   _("Next list"),
                                                   GTK_SIGNAL_FUNC(List_next_list),
                                                   NULL,
                                                   accel_group,
                                                   SC_NEXTLIST,
                                                   SCM_NEXTLIST);

    GTK_create_menu_separator(menu);

    // Sort
    GtkWidget *sortItem = GTK_create_menu_item_with_icon(menu,
                                                         _("Sort"),
                                                         NULL,
                                                         NULL);
    GtkWidget *sortMenu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(sortItem), sortMenu);

    Create_sort_menu(sortMenu, _("Filename"),
                     GTK_SIGNAL_FUNC(List_sort_filename_asc),
                     GTK_SIGNAL_FUNC(List_sort_filename_des));

    Create_sort_menu(sortMenu, _("Extension"),
                     GTK_SIGNAL_FUNC(List_sort_extension_asc),
                     GTK_SIGNAL_FUNC(List_sort_extension_des));

    Create_sort_menu(sortMenu, _("Downloaded size"),
                     GTK_SIGNAL_FUNC(List_sort_csize_asc),
                     GTK_SIGNAL_FUNC(List_sort_csize_des));

    Create_sort_menu(sortMenu, _("Total size"),
                     GTK_SIGNAL_FUNC(List_sort_tsize_asc),
                     GTK_SIGNAL_FUNC(List_sort_tsize_des));

    Create_sort_menu(sortMenu, _("Progress"),
                     GTK_SIGNAL_FUNC(List_sort_progress_asc),
                     GTK_SIGNAL_FUNC(List_sort_progress_des));

    Create_sort_menu(sortMenu, _("Retry"),
                     GTK_SIGNAL_FUNC(List_sort_retry_asc),
                     GTK_SIGNAL_FUNC(List_sort_retry_des));

    Create_sort_menu(sortMenu, _("Recursive"),
                     GTK_SIGNAL_FUNC(List_sort_rec_asc),
                     GTK_SIGNAL_FUNC(List_sort_rec_des));

    Create_sort_menu(sortMenu, _("Status"),
                     GTK_SIGNAL_FUNC(List_sort_status_asc),
                     GTK_SIGNAL_FUNC(List_sort_status_des));

    Create_sort_menu(sortMenu, _("Save dir"),
                     GTK_SIGNAL_FUNC(List_sort_save_asc),
                     GTK_SIGNAL_FUNC(List_sort_save_des));

    Create_sort_menu(sortMenu, _("URL"),
                     GTK_SIGNAL_FUNC(List_sort_url_asc),
                     GTK_SIGNAL_FUNC(List_sort_url_des));

    shuffleList_item = GTK_create_menu_item_with_icon(menu,
                                                      _("Shuffle list"),
                                                      GTK_SIGNAL_FUNC(List_shuffle_list),
                                                      NULL,
                                                      accel_group,
                                                      SC_SHUFFLELIST,
                                                      SCM_SHUFFLELIST);


    // create name entry for new list
    Create_new_list_window(toplevel);
    // create rename entry for the current list
    Create_rename_list_window(toplevel);

    root_item = gtk_menu_item_new_with_label(_("List"));
    gtk_widget_show(root_item);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(root_item), menu);
    gtk_menu_bar_append(GTK_MENU_BAR(menu_bar), root_item);
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
