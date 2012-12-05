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

// $Id: gui_list_rename_list.cc,v 1.6 2001/09/14 16:41:06 tujikawa Exp $

#include "aria.h"
#include "AppOption.h"
#include "ListManager.h"
#include "Dialog.h"
#include "PasteWindow.h"

using namespace std;

// external functions
extern gboolean Hide_window(GtkWidget *window, gpointer unused);

// external variables
extern ListManager *g_listManager;
extern Dialog *g_cDialog;
extern PasteWindow *g_pasteWindow;

// global variables
static GtkWidget *rename_list_window;
static GtkWidget *list_name_entry;

static gboolean List_rename_list_Ok_callback(GtkWidget *w, GtkWidget *rename_list_window)
{

  string list_name = Remove_white(gtk_entry_get_text(GTK_ENTRY(list_name_entry)));
  ListEntry *listentry = g_listManager->ret_Current_listentry();
  if(listentry->getName() == list_name) return TRUE;
  if(g_listManager->checkDuplicatedName(list_name)) {
    g_cDialog->setup(_("Error"),
		     _("The specified name already exists."));
    g_cDialog->set_cancel_button_visible(false);
    g_cDialog->set_transient(GTK_WINDOW(rename_list_window));
    g_cDialog->show();
    return TRUE;
  }

  gtk_widget_hide(rename_list_window);

  listentry->setName(list_name);
  g_pasteWindow->updateTargetList();

  int page_num = gtk_notebook_get_current_page(GTK_NOTEBOOK(g_listManager->ret_baseNotebook()));

  GtkWidget *child = gtk_notebook_get_nth_page(GTK_NOTEBOOK(g_listManager->ret_baseNotebook()), page_num);

  gtk_notebook_set_tab_label_text(GTK_NOTEBOOK(g_listManager->ret_baseNotebook()),
				  child,
				  list_name.c_str());
  gtk_notebook_set_menu_label_text(GTK_NOTEBOOK(g_listManager->ret_baseNotebook()),
				   child,
				   list_name.c_str());
  return TRUE;
}

gboolean List_rename_list(GtkWidget *w, gpointer data)
{
  ListEntry *listentry = g_listManager->ret_Current_listentry();

  gtk_entry_set_text(GTK_ENTRY(list_name_entry), listentry->getName().c_str());
  gtk_editable_select_region(GTK_EDITABLE(list_name_entry),
			     0,
			     -1);

  gtk_widget_grab_focus(list_name_entry);
  gtk_widget_show(GTK_WIDGET(rename_list_window));

  return TRUE;
}

void Create_rename_list_window(GtkWidget *toplevel)
{
  rename_list_window = gtk_dialog_new();
  gtk_window_set_title(GTK_WINDOW(rename_list_window), _("Rename list"));
  gtk_widget_set_usize(rename_list_window, 500, 100);

  // delete event
  g_signal_connect_swapped(GTK_OBJECT(rename_list_window),
			    "delete_event",
			    GTK_SIGNAL_FUNC(Hide_window),
			    GTK_OBJECT(rename_list_window));
  
  //// action area
  GtkWidget *bbox = gtk_hbutton_box_new();
  gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_END);
  gtk_button_box_set_spacing(GTK_BUTTON_BOX(bbox), 5);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(rename_list_window)->action_area),
		     bbox, FALSE, FALSE, 0);
  gtk_widget_show(bbox);
  // Ok button
  GtkWidget *Ok_button = gtk_button_new_with_label(_("Ok"));
  GTK_WIDGET_SET_FLAGS(Ok_button, GTK_CAN_DEFAULT);
  gtk_window_set_default(GTK_WINDOW(rename_list_window), Ok_button);
  g_signal_connect(GTK_OBJECT(Ok_button),
		     "clicked",
		     GTK_SIGNAL_FUNC(List_rename_list_Ok_callback),
		     GTK_OBJECT(rename_list_window));
  gtk_box_pack_start(GTK_BOX(bbox), Ok_button, TRUE, TRUE, 0);
  gtk_widget_show(Ok_button);

  // Cancel button
  GtkWidget *Cancel_button = gtk_button_new_with_label(_("Cancel"));
  //GTK_WIDGET_SET_FLAGS(Cancel_button, GTK_CAN_DEFAULT);
  g_signal_connect_swapped(GTK_OBJECT(Cancel_button),
			    "clicked",
			    GTK_SIGNAL_FUNC(Hide_window),
			    GTK_OBJECT(rename_list_window));
  gtk_box_pack_start(GTK_BOX(bbox), Cancel_button, TRUE, TRUE, 0);
  gtk_widget_show(Cancel_button);

  //// function area
  GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(rename_list_window)->vbox),
		     hbox, TRUE, TRUE, 10);
  gtk_widget_show(hbox);
  // label
  GtkWidget *label = gtk_label_new(_("Name:"));
  gtk_widget_show(label);
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 10);
  // text entry for new URL
  list_name_entry = gtk_entry_new_with_max_length(128);
  g_signal_connect_swapped(GTK_OBJECT(list_name_entry), "activate",
			    GTK_SIGNAL_FUNC(gtk_button_clicked),
			    GTK_OBJECT(Ok_button));
  gtk_box_pack_start(GTK_BOX(hbox), list_name_entry, TRUE, TRUE, 10);
  gtk_widget_show(list_name_entry);

  //// window property
  gtk_window_set_modal(GTK_WINDOW(rename_list_window), TRUE);
  gtk_window_set_transient_for(GTK_WINDOW(rename_list_window),
			       GTK_WINDOW(toplevel));
}
