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

// $Id: Dialog.cc,v 1.5 2002/10/01 15:32:00 tujikawa Exp $

#include "Dialog.h"

static gboolean dummy_func(GtkWidget *w, GtkWidget *window)
{
  //GtkWidget *window = (GtkWidget*)gtk_object_get_data(GTK_OBJECT(w), "window");
  gtk_widget_hide(window);
  return TRUE;
}

static gboolean Hide_window(GtkWidget *window)
{
  gtk_widget_hide(window);
  return TRUE;
}

void Dialog::set_yes_button_visible(bool isvisible)
{
  if(isvisible)
    gtk_widget_show(Yes_button);
  else
    gtk_widget_hide(Yes_button);
};

void Dialog::set_no_button_visible(bool isvisible)
{
  if(isvisible)
    gtk_widget_show(No_button);
  else
    gtk_widget_hide(No_button);
};

void Dialog::set_cancel_button_visible(bool isvisible)
{
  if(isvisible)
    gtk_widget_show(Cancel_button);
  else
    gtk_widget_hide(Cancel_button);
};

void Dialog::set_window_usize(int x, int y)
{
  gtk_widget_set_usize(window, x, y);
}

void Dialog::show()
{
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_MOUSE);
  gtk_widget_show_now(window);
}

void Dialog::hide()
{
  gtk_widget_hide(window);
}

void Dialog::setup(const string& title_string,
		   const string& label_string,
		   gboolean (*yes_Signal_Func)(GtkWidget *w, GtkWidget *window),
		   gboolean (*no_Signal_Func)(GtkWidget *w, GtkWidget *window),
		   gboolean (*cancel_Signal_Func)(GtkWidget *w, GtkWidget *window))
{
  gtk_label_set_text(GTK_LABEL(label), label_string.c_str());
  gtk_window_set_title(GTK_WINDOW(window), title_string.c_str());
  gtk_object_set_user_data(GTK_OBJECT(window), NULL);
  // Yes button
  gtk_signal_disconnect(GTK_OBJECT(Yes_button),
			Yes_connection_id);
  if(yes_Signal_Func != NULL) {
  Yes_connection_id = g_signal_connect(GTK_OBJECT(Yes_button),
					 "clicked",
					 GTK_SIGNAL_FUNC(yes_Signal_Func),
					 GTK_OBJECT(window));
  } else {
    Yes_connection_id = g_signal_connect(GTK_OBJECT(Yes_button),
					   "clicked",
					   GTK_SIGNAL_FUNC(dummy_func),
					   GTK_OBJECT(window));
  }
  gtk_widget_show(Yes_button);
  // No button
  gtk_signal_disconnect(GTK_OBJECT(No_button),
			No_connection_id);
  if(no_Signal_Func != NULL) {
    No_connection_id = g_signal_connect(GTK_OBJECT(No_button),
					  "clicked",
					  GTK_SIGNAL_FUNC(no_Signal_Func),
					  GTK_OBJECT(window));
  } else {
    No_connection_id = g_signal_connect(GTK_OBJECT(No_button),
					  "clicked",
					  GTK_SIGNAL_FUNC(dummy_func),
					  GTK_OBJECT(window));
  }

  gtk_widget_show(No_button);

  // Cancel button

  gtk_signal_disconnect(GTK_OBJECT(Cancel_button),
			Cancel_connection_id);
  if(cancel_Signal_Func != NULL) {
    Cancel_connection_id = g_signal_connect(GTK_OBJECT(Cancel_button),
					      "clicked",
					      GTK_SIGNAL_FUNC(cancel_Signal_Func),
					      GTK_OBJECT(window));
  } else {
    Cancel_connection_id = g_signal_connect(GTK_OBJECT(Cancel_button),
					      "clicked",
					      GTK_SIGNAL_FUNC(dummy_func),
					      GTK_OBJECT(window));
  }
  gtk_widget_show(Cancel_button);

  gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(toplevel));
}

void Dialog::set_transient(GtkWindow *parent)
{
  gtk_window_set_transient_for(GTK_WINDOW(window), parent);
}

Dialog::Dialog(GtkWindow *toplevel_in)
{
  window = gtk_dialog_new();

  toplevel = toplevel_in;
  gtk_widget_set_usize(window, 400, 100);
  g_signal_connect_swapped(GTK_OBJECT(window),
			    "delete_event",
			    GTK_SIGNAL_FUNC(Hide_window),
			    GTK_OBJECT(window));
  // label
  label = gtk_label_new("");
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(window)->vbox),
		     label, TRUE, TRUE, 0);
  gtk_widget_show(label);

  //// action area
  GtkWidget *bbox = gtk_hbutton_box_new();
  gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_SPREAD);
  gtk_button_box_set_spacing(GTK_BUTTON_BOX(bbox), 5);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(window)->action_area),
		     bbox, FALSE, FALSE, 0);
  gtk_widget_show(bbox);

  // OK button
  Yes_button = gtk_button_new_with_label(_("Yes"));
  GTK_WIDGET_SET_FLAGS(Yes_button, GTK_CAN_DEFAULT);
  gtk_window_set_default(GTK_WINDOW(window), Yes_button);
  Yes_connection_id = g_signal_connect(GTK_OBJECT(Yes_button),
					"clicked",
					GTK_SIGNAL_FUNC(dummy_func),
					GTK_OBJECT(window));
  //gtk_object_set_data(GTK_OBJECT(Yes_button), "window", window);
  gtk_box_pack_start(GTK_BOX(bbox), Yes_button, TRUE, TRUE, 0);
  gtk_widget_show(Yes_button);

  // No button
  No_button = gtk_button_new_with_label(_("No"));
  GTK_WIDGET_SET_FLAGS(No_button, GTK_CAN_DEFAULT);
  No_connection_id = g_signal_connect(GTK_OBJECT(No_button),
				       "clicked",
				       GTK_SIGNAL_FUNC(dummy_func),
				       GTK_OBJECT(window));
  //gtk_object_set_data(GTK_OBJECT(No_button), "window", window);
  gtk_box_pack_start(GTK_BOX(bbox), No_button, TRUE, TRUE, 0);
  gtk_widget_show(No_button);

  // Cancel button
  Cancel_button = gtk_button_new_with_label(_("Cancel"));
  GTK_WIDGET_SET_FLAGS(Cancel_button, GTK_CAN_DEFAULT);
  Cancel_connection_id = g_signal_connect(GTK_OBJECT(Cancel_button),
					    "clicked",
					    GTK_SIGNAL_FUNC(dummy_func),
					    GTK_OBJECT(window));
  //gtk_object_set_data(GTK_OBJECT(Cancel_button), "window", window);
  gtk_box_pack_start(GTK_BOX(bbox), Cancel_button, TRUE, TRUE, 0);
  gtk_widget_show(Cancel_button);

  //// window property
  gtk_window_set_modal(GTK_WINDOW(window), TRUE);
  gtk_window_set_transient_for(GTK_WINDOW(window), toplevel);
}

Dialog::~Dialog()
{
  gtk_widget_destroy(window);
}

void Dialog::set_data(void *data)
{
  gtk_object_set_user_data(GTK_OBJECT(window), data);
}

void *Dialog::get_data()
{
  return gtk_object_get_user_data(GTK_OBJECT(window));
}

