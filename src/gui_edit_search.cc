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

// $Id: gui_edit_search.cc,v 1.7 2001/09/11 13:13:32 tujikawa Exp $

#include "aria.h"
#include "Dialog.h"
#include "ListManager.h"

// external functions
extern void Show_download_log(ItemCell *itemcell);

extern gboolean Hide_window(GtkWidget *window, gpointer unused);
extern ListManager *g_listManager;
extern Dialog *g_cDialog;

static GtkWidget *search_entry;
static GtkWidget *search_window;


static gboolean Edit_search_Search_callback(GtkWidget *w, gpointer data)
{
    string target = Remove_white(gtk_entry_get_text(GTK_ENTRY(search_entry)));
    if (target.empty()) return TRUE;

    ListEntry *listentry = g_listManager->ret_Current_listentry();

    listentry->get_Dl_clist_lock();

    GList *node = GTK_CLIST(listentry->ret_Dl_clist())->selection;
    int start_index = 0;
    if (node != NULL) {
        start_index = GPOINTER_TO_INT(node->data) + 1;
    }
    gtk_clist_unselect_all(GTK_CLIST(listentry->ret_Dl_clist()));
    for (int rowindex = start_index; rowindex < GTK_CLIST(listentry->ret_Dl_clist())->rows; ++rowindex) {
        char *filename;
        gtk_clist_get_text(GTK_CLIST(listentry->ret_Dl_clist()), rowindex, COL_FILENAME, &filename);

        if (casefind(filename, target.c_str()) != string::npos) {
            gtk_clist_select_row(GTK_CLIST(listentry->ret_Dl_clist()), rowindex, COL_FILENAME);
            Show_download_log(listentry->getItemCellByRow(rowindex));
            gtk_clist_moveto(GTK_CLIST(listentry->ret_Dl_clist()), rowindex, COL_ICON, 0, 0);
            listentry->release_Dl_clist_lock();
            return TRUE;
        }
    }
    listentry->release_Dl_clist_lock();

    g_cDialog->setup(_("Search"),
                     _("Search complete"));
    g_cDialog->set_cancel_button_visible(false);
    g_cDialog->set_no_button_visible(false);
    g_cDialog->set_transient(GTK_WINDOW(search_window));
    g_cDialog->show();

    return TRUE;
}

gboolean Edit_search(GtkWidget *w, gpointer data)
{
    gtk_editable_select_region(GTK_EDITABLE(search_entry),
                               0,
                               -1);

    gtk_widget_grab_focus(search_entry);
    gtk_widget_show(GTK_WIDGET(search_window));

    gdk_window_raise(search_window->window);

    return TRUE;
}

void Create_search_window()
{
    search_window = gtk_dialog_new();
    gtk_window_set_title(GTK_WINDOW(search_window), _("Search"));
    gtk_widget_set_usize(search_window, 500, 150);

    // delete event
    g_signal_connect_swapped(GTK_OBJECT(search_window),
                             "delete_event",
                             GTK_SIGNAL_FUNC(Hide_window),
                             GTK_OBJECT(search_window));

    //// action area
    GtkWidget *bbox = gtk_hbutton_box_new();
    gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_END);
    gtk_button_box_set_spacing(GTK_BUTTON_BOX(bbox), 5);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(search_window)->action_area),
                       bbox, FALSE, FALSE, 0);
    gtk_widget_show(bbox);
    // Search button
    GtkWidget *Search_button = gtk_button_new_with_label(_("Search"));
    GTK_WIDGET_SET_FLAGS(Search_button, GTK_CAN_DEFAULT);
    gtk_window_set_default(GTK_WINDOW(search_window), Search_button);
    g_signal_connect(GTK_OBJECT(Search_button),
                     "clicked",
                     GTK_SIGNAL_FUNC(Edit_search_Search_callback),
                     GTK_OBJECT(search_window));
    gtk_box_pack_start(GTK_BOX(bbox), Search_button, TRUE, TRUE, 0);
    gtk_widget_show(Search_button);

    // Cancel button
    GtkWidget *Cancel_button = gtk_button_new_with_label(_("Cancel"));
    //GTK_WIDGET_SET_FLAGS(Cancel_button, GTK_CAN_DEFAULT);
    g_signal_connect_swapped(GTK_OBJECT(Cancel_button),
                             "clicked",
                             GTK_SIGNAL_FUNC(Hide_window),
                             GTK_OBJECT(search_window));
    gtk_box_pack_start(GTK_BOX(bbox), Cancel_button, TRUE, TRUE, 0);
    gtk_widget_show(Cancel_button);

    //// function area
    GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(search_window)->vbox),
                       hbox, TRUE, TRUE, 10);
    gtk_widget_show(hbox);
    // label
    GtkWidget *label = gtk_label_new(_("String:"));
    gtk_widget_show(label);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 10);
    // text entry for new URL
    search_entry = gtk_entry_new_with_max_length(128);
    g_signal_connect_swapped(GTK_OBJECT(search_entry), "activate",
                             GTK_SIGNAL_FUNC(gtk_button_clicked),
                             GTK_OBJECT(Search_button));
    gtk_box_pack_start(GTK_BOX(hbox), search_entry, TRUE, TRUE, 10);
    gtk_widget_show(search_entry);

    //// window property
    gtk_window_set_modal(GTK_WINDOW(search_window), FALSE);
    //gtk_window_set_transient_for(GTK_WINDOW(search_window),
    //		       GTK_WINDOW(toplevel));

}
