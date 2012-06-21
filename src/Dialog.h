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

// $Id: Dialog.h,v 1.4 2001/03/09 06:49:09 tujikawa Exp $

#ifndef _DIALOG_H_
#define _DIALOG_H_
#include <gtk/gtk.h>
#include <string>
#include "aria.h"
using namespace std;

class Dialog
{
private:
  GtkWidget *window;
  GtkWidget *Yes_button;
  GtkWidget *No_button;
  GtkWidget *Cancel_button;
  GtkWidget *label;
  GtkWindow *toplevel;

  unsigned int Yes_connection_id;
  unsigned int No_connection_id;
  unsigned int Cancel_connection_id;

public:
  Dialog(GtkWindow *toplevel_in);
  ~Dialog();

  void setup(const string& title_in,
	     const string& label_in,
	     gboolean (*yes_SignalFunc)(GtkWidget *w, GtkWidget *window) = NULL,
	     gboolean (*no_SignalFunc)(GtkWidget *w, GtkWidget *window) = NULL,
	     gboolean (*cancel_SignalFunc)(GtkWidget *w, GtkWidget *window) = NULL);
  void set_yes_button_visible(bool isvisible);
  void set_no_button_visible(bool isvisible);
  void set_cancel_button_visible(bool isvisible);
  void set_data(void *data);
  void *get_data();
  void set_window_usize(int x, int y);
  void set_transient(GtkWindow *parent);
  void show();
  void hide();
};

#endif //_DIALOG_H_
