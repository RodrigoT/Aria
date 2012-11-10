//
//  Aria - yet another download tool
//  Copyright (C) 2000, 2001, 2002 Tatsuhiro Tsujikawa
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

// $Id: gui_help.cc,v 1.34 2002/09/30 13:29:46 tujikawa Exp $

#include <string>
#include <gtk/gtk.h>
#include "utils.h"
#include "aria.h"
#include "pixmaps/ARIA_LOGO.xpm"
#include "ShortCutKey.h"

extern string g_machineInfo;
GdkBitmap *mask;
GdkPixmap *aria_logo;

static gboolean Help_destroy_about_dialog(GtkWidget *about_dialog)
{
  gtk_widget_destroy(about_dialog);
  gdk_pixmap_unref(aria_logo);
  gdk_bitmap_unref(mask);
  return(TRUE);
}

static gboolean Help_about(GtkWidget *w, GtkWidget *top_level)
{
  GtkWidget *about_dialog = gtk_dialog_new();
  gtk_widget_set_usize(GTK_WIDGET(about_dialog), 400, 300);
  gtk_window_set_title(GTK_WINDOW(about_dialog), _("About Aria"));
  gtk_signal_connect_object(GTK_OBJECT(about_dialog),
				   "delete_event",
				   GTK_SIGNAL_FUNC(Help_destroy_about_dialog),
				   GTK_OBJECT(about_dialog));
  gtk_window_set_transient_for(GTK_WINDOW(about_dialog), GTK_WINDOW(top_level));
  gtk_window_set_modal(GTK_WINDOW(about_dialog), TRUE);
  gtk_widget_realize(about_dialog);

  // create Aria logo and show it
  GtkStyle *style = gtk_widget_get_style(about_dialog);
  aria_logo = gdk_pixmap_create_from_xpm_d(about_dialog->window, &mask, &style->bg[GTK_STATE_NORMAL], (char **)ARIA_LOGO_xpm);
  GtkWidget *aria_logo_widget = gtk_pixmap_new(aria_logo, mask);
  gtk_widget_show(aria_logo_widget);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(about_dialog)->vbox),
		     aria_logo_widget, FALSE, FALSE, 0);

  // text box
  GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
  gtk_widget_show(hbox);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(about_dialog)->vbox), hbox, FALSE, FALSE, 5);

  GtkWidget *scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
				 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_widget_show(scrolled_window);
  gtk_box_pack_start(GTK_BOX(hbox),
		     scrolled_window, TRUE, TRUE, 5);

  // create text widget and write some information into it
  GtkWidget *text = gtk_text_view_new ();
  gtk_widget_set_usize(text, 350, 130);
  gtk_widget_show(text);
  // テキスト入力を不可にする
  gtk_text_view_set_editable (GTK_TEXT_VIEW(text), FALSE);
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text), GTK_WRAP_WORD_CHAR);

  // テキストウィジェットの表示
  gtk_container_add(GTK_CONTAINER(scrolled_window), text);


  // show aria version and copyright information
  string line;
  //line = line+"Aria (version "+VERSION+"; "+machine_info+")\n"+"Copyright(C) 2000, 2001 Tatsuhiro Tsujikawa\n"+"Released: "+DATE_ROLLOUT+"\n\n";
  line = "Aria (version "VERSION"; "+g_machineInfo+")\n"
    "Copyright(C) 2000, 2001, 2002 Tatsuhiro Tsujikawa\n";
  line += _("Rollout: ");
  line += DATE_ROLLOUT;
  line +=  "\n\n";
  //gtk_text_insert(GTK_TEXT(text), NULL, NULL, NULL, line.c_str(), -1);

  // show transraters
  //line.erase();
  line += _("Translations:"); line += '\n';
  line +=  "    Adam Purkrt         "; line += _("(Czech)");
  line +=  "\n    Alejandro N. Vargas "; line += _("(Spanish)");
  line +=  "\n    Tristan Chabredier  "; line += _("(French)");
  line +=  "\n    Nicolas Boos        "; line += _("(French)");
  line +=  "\n    Hermann J. Beckers  "; line += _("(German)");
  line +=  "\n    Boldizsar Nagy      "; line += _("(Hungarian)");
  line +=  "\n    Giovanni Pardini    "; line += _("(Italian)");
  line +=  "\n    Tatsuhiro Tsujikawa "; line += _("(Japanese)");
  line +=  "\n    Przemyslaw Sulek    "; line += _("(Polish)");
  line +=  "\n    Igor V. Youdytsky   "; line += _("(Russian)");
  line +=  "\n    hardded             "; line += _("(Traditional Chinese)");
  line +=  "\n\n";

  //gtk_text_insert(GTK_TEXT(text), NULL, NULL, NULL, line.c_str(), -1);

  // show special thanks
  //line.erase();

  line += _("Aria Logo:"); line += '\n';
  line +=  "    aLEczapKA";
  line +=  "\n\n";

  line += _("Special Thanks:"); line += '\n';
  line += "    Brian Chapman\n"
    "    Chris Debenham\n"
    "    IWATA Ray\n"
    "    Matthias Babisch\n"
    "    Tsuyoshi Iguchi\n";
  line += _("    and all who support Aria");
  line += "\n\n";

  //gtk_text_insert(GTK_TEXT(text), NULL, NULL, NULL, line.c_str(), -1);

  // GPL issue
  //line.erase();
  line += _("License:"); line += '\n';
  line +=
    _("This program is free software; you can redistribute it and/or modify"
      " it under the terms of the GNU General Public License as published by"
      " the Free Software Foundation; either version 2 of the License, or"
      " (at your option) any later version.\n\n"
      "This program is distributed in the hope that it will be useful,"
      " but WITHOUT ANY WARRANTY; without even the implied warranty of"
      " MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the"
      " GNU General Public License for more details.\n\n"
      "You should have received a copy of the GNU General Public License"
      " along with this program; if not, write to the Free Software"
      " Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.");
  line += "\n\n";
  line +=
    _("The part of directory browser is a modified code, the original code was written by Peter Alm, Mikael Alm, Olle Hallnas, Thomas Nilsson and 4Front Technologies.\n"
      "The original code is copyrighted by Peter Alm, Mikael Alm, Olle Hallnas, Thomas Nilsson and 4Front Technologies.");
  line += "\n\n";
  line +=
    _("The part of MD5 checking is a modified code, the original code was written for GnuPG, "
      "copyrighted 1995, 1996, 1998, 1999 Free Software Foundation, Inc.");
  line += "\n\n";

#ifdef HAVE_ZLIB
  line +=
    _("This program uses zlib, Copyright (C) 1995-1998 Jean-loup Gailly and Mark Adler");
  line += "\n\n";
#endif // HAVE_ZLIB

#ifdef HAVE_OPENSSL
  line +=  _("This program uses OpenSSL, Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)");
  line += "\n\n";
#endif // HAVE_OPENSSL

  //gtk_text_insert(GTK_TEXT(text), NULL, NULL, NULL, line.c_str(), -1);  

  // show compile information
  //line.erase();
  line += _("Built on:");
  line += "\n    "D_MACHINE_COMPILED"\n    "D_DATE_COMPILED"\n";
  //gtk_text_insert(GTK_TEXT(text), NULL, NULL, NULL, line.c_str(), -1);

  // show gtk+ version
  //line.erase();
  line += "Gtk+ version: "+itos(gtk_major_version)+"."+itos(gtk_minor_version)+"."+itos(gtk_micro_version)+"\n";

  // show text
  GtkTextIter lastpos;
  gtk_text_buffer_get_iter_at_offset(gtk_text_view_get_buffer(GTK_TEXT_VIEW(text)), &lastpos, -1);
  gtk_text_buffer_insert(gtk_text_view_get_buffer(GTK_TEXT_VIEW(text)), &lastpos, line.c_str(), -1);

  //// action area
  GtkWidget *bbox = gtk_hbutton_box_new();
  gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_END);
  gtk_button_box_set_spacing(GTK_BUTTON_BOX(bbox), 5);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(about_dialog)->action_area),
		     bbox, FALSE, FALSE, 0);
  gtk_widget_show(bbox);
  // button
  GtkWidget *button = gtk_button_new_with_label(_("OK"));
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_window_set_default(GTK_WINDOW(about_dialog), button);
  gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, TRUE, 0);
  gtk_signal_connect_object(GTK_OBJECT(button), "clicked",
			    GTK_SIGNAL_FUNC(Help_destroy_about_dialog),
			    GTK_OBJECT(about_dialog));
  gtk_widget_show(button);

  // show about_dialog
  gtk_window_set_position(GTK_WINDOW(about_dialog), GTK_WIN_POS_MOUSE);
  gtk_widget_show(about_dialog);
  return(TRUE);
}

void Create_help_menu(GtkWidget *window, GtkWidget *menu_bar, GtkAccelGroup *accel_group)
{
  GtkWidget *help_menu;
  GtkWidget *about_item;

  //create Help menu
  help_menu = gtk_menu_new();

  about_item = gtk_menu_item_new_with_label(_("About Aria"));
  
  gtk_menu_append(GTK_MENU(help_menu), about_item);

  gtk_signal_connect(GTK_OBJECT(about_item), "activate",
		     GTK_SIGNAL_FUNC(Help_about),
		     GTK_OBJECT(window));

  gtk_widget_add_accelerator(about_item,
			     "activate",
			     accel_group,
			     SC_ABOUT,
			     SCM_ABOUT,
			     GTK_ACCEL_VISIBLE);

  gtk_widget_show(about_item);

  GtkWidget *help_item = gtk_menu_item_new_with_label(_("Help"));
  gtk_widget_show(help_item);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(help_item), help_menu);
  gtk_menu_item_right_justify(GTK_MENU_ITEM(help_item));
  gtk_menu_bar_append(GTK_MENU_BAR(menu_bar), help_item);
}
