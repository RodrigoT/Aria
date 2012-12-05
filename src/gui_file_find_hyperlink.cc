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

// $Id: gui_file_find_hyperlink.cc,v 1.11 2001/09/11 13:13:32 tujikawa Exp $
#include "aria.h"
#include "ItemCell.h"
#include "ItemList.h"
#include "ListManager.h"
#include "AppOption.h"
#include "FileBrowser.h"
using namespace std;

// external functions
extern void Set_sensitive__list_not_empty();
extern gboolean Hide_window(GtkWidget *window, gpointer unused);

// global variables
extern ListManager *g_listManager;
extern ItemList *g_itemList;
extern ItemCell *g_consoleItem;
extern AppOption *g_appOption;
extern FileBrowser *g_cFileBrowser;
// static global variables
static GtkWidget *sg_findHlWindow;
static GtkWidget *sg_urlEntry;

static gboolean File_ok_find_hyperlink(GtkWidget *w, GtkWidget *fs)
{
  const char *filename;

  ListEntry *listEntry = g_listManager->ret_Current_listentry();

  g_cFileBrowser->hide();
  if((filename = gtk_file_selection_get_filename(GTK_FILE_SELECTION(fs))) != NULL) {
    string base_url = Remove_white(gtk_entry_get_text(GTK_ENTRY(sg_urlEntry)));
    if(!g_itemList->Find_Hyperlink_from_file(filename, base_url, ItemList::FINDHREF_PASTE)) {
      listEntry->get_Dl_clist_lock();

      g_consoleItem->Send_message_to_gui(_("Error occurred while reading HTML file"), MSG_SYS_ERROR);
    } else {
      /*
      if(g_appOption->Whether_use_automatic_start() ||
	 !listEntry->getThreadManager()->Is_all_threads_sleeping()) {
	listEntry->Send_start_signal();
      }
      */
      //modified 2001/5/20
      if(g_appOption->Whether_use_automatic_start()) {
	listEntry->Send_start_signal();
      }

      listEntry->release_Dl_clist_lock();
      g_consoleItem->Send_message_to_gui(_("Hyperlink retrieval complete"), MSG_SYS_INFO);
    }
    if(GTK_CLIST(listEntry->ret_Dl_clist())->rows > 0) {
      Set_sensitive__list_not_empty();//fix this
    }
  }

  return TRUE;
}

static gboolean File_find_hyperlink_OK_callback(GtkWidget *w, GtkWidget *window)
{
  gtk_widget_hide(window);
  g_cFileBrowser->setup(_("Find Hyperlink"),
		       File_ok_find_hyperlink);
  g_cFileBrowser->show();

  return TRUE;
}

gboolean File_find_hyperlink(GtkWidget *w, gpointer data)
{
  gtk_widget_show(sg_findHlWindow);
  gtk_widget_grab_focus(sg_urlEntry);

  return TRUE;
}

void Create_find_hyperlink_window(GtkWidget *topLevel)
{
  sg_findHlWindow = gtk_dialog_new();
  gtk_window_set_title(GTK_WINDOW(sg_findHlWindow), _("Find Hyperlink"));
  gtk_widget_set_usize(sg_findHlWindow, 500, 100);
  // delete event
  g_signal_connect_swapped(GTK_OBJECT(sg_findHlWindow),
			    "delete_event",
			    GTK_SIGNAL_FUNC(Hide_window),
			    GTK_OBJECT(sg_findHlWindow));
  
  //// action area
  GtkWidget *bbox = gtk_hbutton_box_new();
  gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_END);
  gtk_button_box_set_spacing(GTK_BUTTON_BOX(bbox), 5);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(sg_findHlWindow)->action_area),
		     bbox, FALSE, FALSE, 0);
  gtk_widget_show(bbox);
  // OK button
  GtkWidget *OK_button = gtk_button_new_with_label(_("OK"));
  GTK_WIDGET_SET_FLAGS(OK_button, GTK_CAN_DEFAULT);
  gtk_window_set_default(GTK_WINDOW(sg_findHlWindow), OK_button);
  g_signal_connect(GTK_OBJECT(OK_button),
		     "clicked",
		     GTK_SIGNAL_FUNC(File_find_hyperlink_OK_callback),
		     GTK_OBJECT(sg_findHlWindow));
  gtk_box_pack_start(GTK_BOX(bbox), OK_button, TRUE, TRUE, 0);
  gtk_widget_show(OK_button);
  // Cancel button
  GtkWidget *Cancel_button = gtk_button_new_with_label(_("Cancel"));
  //GTK_WIDGET_SET_FLAGS(Cancel_button, GTK_CAN_DEFAULT);
  g_signal_connect_swapped(GTK_OBJECT(Cancel_button),
			    "clicked",
			    GTK_SIGNAL_FUNC(Hide_window),
			    GTK_OBJECT(sg_findHlWindow));
  gtk_box_pack_start(GTK_BOX(bbox), Cancel_button, TRUE, TRUE, 0);
  gtk_widget_show(Cancel_button);

  //// function area
  GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
  gtk_widget_show(hbox);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(sg_findHlWindow)->vbox),
		     hbox, TRUE, TRUE, 10);
  // label
  GtkWidget *label = gtk_label_new("BASE URL:");
  gtk_widget_show(label);
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 10);
  // text entry for URL
  sg_urlEntry = gtk_entry_new_with_max_length(512);
  g_signal_connect_swapped(GTK_OBJECT(sg_urlEntry), "activate",
			    GTK_SIGNAL_FUNC(gtk_button_clicked),
			    GTK_OBJECT(OK_button));
  gtk_box_pack_start(GTK_BOX(hbox), sg_urlEntry, TRUE, TRUE, 10);
  gtk_widget_show(sg_urlEntry);

  // window property
  gtk_window_set_modal(GTK_WINDOW(sg_findHlWindow), TRUE);
  gtk_window_set_transient_for(GTK_WINDOW(sg_findHlWindow),
			       GTK_WINDOW(topLevel));
}
