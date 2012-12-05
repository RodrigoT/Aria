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

// $Id: FileBrowser.cc,v 1.5 2002/10/01 15:32:00 tujikawa Exp $

#include "FileBrowser.h"

static gboolean dummy_func(GtkWidget *w, GtkWidget *fs)
{
  gtk_widget_hide(fs);
  return TRUE;
}

static gboolean Hide_window(GtkWidget *fs)
{
  gtk_widget_hide(fs);
  return TRUE;
}

void FileBrowser::set_ok_button_visible(bool isvisible)
{
  if(isvisible)
    gtk_widget_show(GTK_FILE_SELECTION(fs)->ok_button);
  else
    gtk_widget_hide(GTK_FILE_SELECTION(fs)->ok_button);
};

void FileBrowser::set_cancel_button_visible(bool isvisible)
{
  if(isvisible)
    gtk_widget_show(GTK_FILE_SELECTION(fs)->cancel_button);
  else
    gtk_widget_hide(GTK_FILE_SELECTION(fs)->cancel_button);
};

void FileBrowser::show()
{
  gtk_window_set_position(GTK_WINDOW(fs), GTK_WIN_POS_MOUSE);
  gtk_widget_show(fs);
}

void FileBrowser::hide()
{
  gtk_widget_hide(fs);
}

void FileBrowser::setup(const string& title_string,
		   gboolean (*ok_Signal_Func)(GtkWidget *w, GtkWidget *fs),
		   gboolean (*cancel_Signal_Func)(GtkWidget *w, GtkWidget *fs))
{
  gtk_window_set_title(GTK_WINDOW(fs), title_string.c_str());

  // OK button
  gtk_signal_disconnect(GTK_OBJECT(GTK_FILE_SELECTION(fs)->ok_button),
			Ok_connection_id);
  Ok_connection_id = g_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(fs)->ok_button),
					 "clicked",
					 GTK_SIGNAL_FUNC(ok_Signal_Func),
					 GTK_OBJECT(fs));
  gtk_widget_show(GTK_FILE_SELECTION(fs)->ok_button);

  // Cancel button

  gtk_signal_disconnect(GTK_OBJECT(GTK_FILE_SELECTION(fs)->cancel_button),
			Cancel_connection_id);
  if(cancel_Signal_Func != NULL) {
    Cancel_connection_id = g_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(fs)->cancel_button),
					      "clicked",
					      GTK_SIGNAL_FUNC(cancel_Signal_Func),
					      GTK_OBJECT(fs));
  } else {
    Cancel_connection_id = g_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(fs)->cancel_button),
					      "clicked",
					      GTK_SIGNAL_FUNC(dummy_func),
					      GTK_OBJECT(fs));
  }
  gtk_widget_show(GTK_FILE_SELECTION(fs)->cancel_button);

  gtk_window_set_transient_for(GTK_WINDOW(fs), GTK_WINDOW(toplevel));
}

void FileBrowser::set_transient(GtkWindow *parent)
{
  gtk_window_set_transient_for(GTK_WINDOW(fs), parent);
}

void FileBrowser::set_default_path(const string& path)
{
  gtk_file_selection_set_filename(GTK_FILE_SELECTION(fs), path.c_str());
}

FileBrowser::FileBrowser(GtkWindow *toplevel_in)
{
  fs = gtk_file_selection_new("");

  toplevel = toplevel_in;
  
  g_signal_connect_swapped(GTK_OBJECT(fs),
			    "delete_event",
			    GTK_SIGNAL_FUNC(Hide_window),
			    GTK_OBJECT(fs));

  //// action area
  // OK button
  Ok_connection_id = g_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(fs)->ok_button),
					"clicked",
					GTK_SIGNAL_FUNC(dummy_func),
					NULL);

  
  // Cancel button
  Cancel_connection_id = g_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(fs)->cancel_button),
					    "clicked",
					    GTK_SIGNAL_FUNC(dummy_func),
					    NULL);

  //// window property
  gtk_window_set_modal(GTK_WINDOW(fs), TRUE);
  gtk_window_set_transient_for(GTK_WINDOW(fs), toplevel);
}

FileBrowser::~FileBrowser()
{
  gtk_widget_destroy(fs);
}

