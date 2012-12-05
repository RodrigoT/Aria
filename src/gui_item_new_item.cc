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

// $Id: gui_item_new_item.cc,v 1.10 2002/02/13 12:09:24 tujikawa Exp $

#include "aria.h"
#include "ItemCell.h"
#include "ListManager.h"
#include "AppOption.h"
#include "PasteWindow.h"
using namespace std;

// external functions
extern void Set_sensitive__list_not_empty();
extern gboolean Hide_window(GtkWidget *window, gpointer unused);
extern void Add_new_item_to_downloadlist(ItemCell *itemcell, ListEntry *listentry);

// external variables
extern ListManager *g_listManager;
extern ItemCell *g_consoleItem;
extern AppOption *g_appOption;
extern PasteWindow *g_pasteWindow;
//extern GtkWidget *g_toplevel;
static Dialog *g_eDialog = NULL;

// global variables
static GtkWidget *new_item_window;
static GtkWidget *new_url_entry;

static gboolean Item_new_item_Unfold_callback(GtkWidget *w, GtkWidget *new_item_window)
{
    gtk_widget_hide(new_item_window);
    string url = Remove_white(gtk_entry_get_text(GTK_ENTRY(new_url_entry)));
    if (g_pasteWindow->addURLByNumericalExpansion(url)) {
        g_pasteWindow->show();
    }

    return TRUE;
}

bool Create_new_item(string url, bool onPaste = false, const string &dir = "", const string &referer = "")
{
    //gtk_widget_hide(new_item_window);
    URLcontainer urlcon;
    ListEntry *listentry = g_listManager->ret_Current_listentry();

    url = URLcontainer::Find_URL(url, false);
//    unsigned int rtCharPos;
//    if((rtCharPos = url.find_first_of("\r\n")) != string::npos) {
//      url = url.substr(0, rtCharPos);
//    }
    if (urlcon.Parse_URL(url) &&
            //!listentry->getItemManager()->search_by_url(urlcon.ret_URL())) {
            !listentry->getItemManager()->search_by_url_with_local_path(urlcon.ret_URL(), listentry->ret_Default_item()->ret_Options_opt().ret_Store_Dir() + urlcon.ret_Filename())) {
        Options options = listentry->ret_Default_item()->ret_Options_opt();

        if (dir.size()) {
            options.set_Store_Dir(dir);
        }
        if (referer.size()) {
            options.set_Referer_Type(Options::REFERER_USER_DEFINED);
            options.set_Referer(referer);
        }
        ItemCell *itemcell = new ItemCell(url, urlcon, options, _("Created"));
        /*
        if(urlcon.ret_Protocol() == "http:") {
          itemcell = new ItemCell_HTTP(url, urlcon, options, _("Created"));
        } else if(urlcon.ret_Protocol() == "ftp:") {
          itemcell = new ItemCell_FTP(url, urlcon, options, _("Created"));
        } else {
          return true;
        }
        */
        //pthread_mutex_lock(&itemlistlock);
        if (onPaste) {
            g_pasteWindow->addItem(itemcell);
            g_pasteWindow->show();
        } else {
            listentry->get_Dl_clist_lock();
            Add_new_item_to_downloadlist(itemcell, listentry);
            // modified 2001/5/20
            if (g_appOption->Whether_use_automatic_start()) {
                listentry->Send_start_signal();
            }

            /*gtk_clist_moveto(GTK_CLIST(listentry->ret_Dl_clist()),
                   GTK_CLIST(listentry->ret_Dl_clist())->rows-1,
                   0,
                   0.0,
                   0.0);*/
            if (listentry->getRowCount() > 0)
                listentry->scrollToRow(listentry->getRowCount() - 1);
            //pthread_mutex_unlock(&itemlistlock);
            listentry->release_Dl_clist_lock();
            g_consoleItem->Send_message_to_gui(_("item added"), MSG_SYS_INFO);
            if (listentry->getRowCount() > 0) {
                Set_sensitive__list_not_empty();//fix this
            }
        }
    }

    return true;
}

static gboolean Item_new_item_Add_callback(GtkWidget *w, GtkWidget *new_item_window)
{
    string url = Remove_white(gtk_entry_get_text(GTK_ENTRY(new_url_entry)));
    if (g_pasteWindow->addURL(url)) {
        gtk_widget_hide(new_item_window);
        g_pasteWindow->show();
    } else {
        if (g_eDialog == NULL) {
            g_eDialog = new Dialog(GTK_WINDOW(gtk_widget_get_toplevel(w)));
        }

        g_eDialog->setup(_("Error"),
                         _("Invalid URL")
                        );
        g_eDialog->set_no_button_visible(false);
        g_eDialog->set_cancel_button_visible(false);
        g_eDialog->set_transient(GTK_WINDOW(gtk_widget_get_toplevel(w)));
        g_eDialog->show();
    }
    ////modified 2001/3/1
    //return Create_new_item(Remove_white(gtk_entry_get_text(GTK_ENTRY(new_url_entry))));
    return TRUE;
}

gboolean Item_new_item(GtkWidget *w, gpointer data)
{
    gtk_entry_set_text(GTK_ENTRY(new_url_entry), "");
    gtk_widget_grab_focus(new_url_entry);
    gtk_widget_show(GTK_WIDGET(new_item_window));

    return TRUE;
}

void Create_new_item_window(GtkWidget *toplevel)
{
    new_item_window = gtk_dialog_new();
    gtk_window_set_title(GTK_WINDOW(new_item_window), _("New item"));
    gtk_widget_set_usize(new_item_window, 500, 100);

    // delete event
    g_signal_connect_swapped(GTK_OBJECT(new_item_window),
                             "delete_event",
                             GTK_SIGNAL_FUNC(Hide_window),
                             GTK_OBJECT(new_item_window));

    //// action area
    GtkWidget *bbox = gtk_hbutton_box_new();
    gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_END);
    gtk_button_box_set_spacing(GTK_BUTTON_BOX(bbox), 5);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(new_item_window)->action_area),
                       bbox, FALSE, FALSE, 0);
    gtk_widget_show(bbox);
    // Add button
    GtkWidget *Add_button = gtk_button_new_with_label(_("Add"));
    GTK_WIDGET_SET_FLAGS(Add_button, GTK_CAN_DEFAULT);
    gtk_window_set_default(GTK_WINDOW(new_item_window), Add_button);
    g_signal_connect(GTK_OBJECT(Add_button),
                     "clicked",
                     GTK_SIGNAL_FUNC(Item_new_item_Add_callback),
                     GTK_OBJECT(new_item_window));
    gtk_box_pack_start(GTK_BOX(bbox), Add_button, TRUE, TRUE, 0);
    gtk_widget_show(Add_button);
    // unfold button
    GtkWidget *Unfold_button = gtk_button_new_with_label(_("Numerical expansion"));
    //GTK_WIDGET_SET_FLAGS(Unfold_button, GTK_CAN_DEFAULT);
    g_signal_connect(GTK_OBJECT(Unfold_button),
                     "clicked",
                     GTK_SIGNAL_FUNC(Item_new_item_Unfold_callback),
                     GTK_OBJECT(new_item_window));
    gtk_box_pack_start(GTK_BOX(bbox), Unfold_button, TRUE, TRUE, 0);
    gtk_widget_show(Unfold_button);
    // Cancel button
    GtkWidget *Cancel_button = gtk_button_new_with_label(_("Cancel"));
    //GTK_WIDGET_SET_FLAGS(Cancel_button, GTK_CAN_DEFAULT);
    g_signal_connect_swapped(GTK_OBJECT(Cancel_button),
                             "clicked",
                             GTK_SIGNAL_FUNC(Hide_window),
                             GTK_OBJECT(new_item_window));
    gtk_box_pack_start(GTK_BOX(bbox), Cancel_button, TRUE, TRUE, 0);
    gtk_widget_show(Cancel_button);

    //// function area
    GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(new_item_window)->vbox),
                       hbox, TRUE, TRUE, 10);
    gtk_widget_show(hbox);
    // label
    GtkWidget *label = gtk_label_new("URL:");
    gtk_widget_show(label);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 10);
    // text entry for new URL
    new_url_entry = gtk_entry_new_with_max_length(512);
    g_signal_connect_swapped(GTK_OBJECT(new_url_entry), "activate",
                             GTK_SIGNAL_FUNC(gtk_button_clicked),
                             GTK_OBJECT(Add_button));
    gtk_box_pack_start(GTK_BOX(hbox), new_url_entry, TRUE, TRUE, 10);
    gtk_widget_show(new_url_entry);

    //// window property

    gtk_window_set_modal(GTK_WINDOW(new_item_window), TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(new_item_window),
                                 GTK_WINDOW(toplevel));
}
