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

// $Id: gui_utils.cc,v 1.5 2001/10/12 16:47:39 tujikawa Exp $

#include "gui_utils.h"
#include <stdlib.h>

bool Create_pixmap_from_file(GdkPixbuf **pixmap_ptr,
			     GdkBitmap **bitmap_ptr,
			     const string& filepath,
			     GtkWidget *widget)
{
  if(*pixmap_ptr != NULL) {
    g_object_unref(*pixmap_ptr);
    *pixmap_ptr = NULL;
  }

  if(*bitmap_ptr != NULL) {
    g_object_unref(*bitmap_ptr);
    *bitmap_ptr = NULL;
  }
  
//  GtkStyle *style = gtk_widget_get_style(widget);

  *pixmap_ptr = gdk_pixbuf_new_from_file(filepath.c_str(), NULL);
  if(*pixmap_ptr) {
    return true;
  } else {
    return false;
  }
}

string get_accel_string(unsigned int accelKey,
			unsigned int accelMods)
{
  string label;

  if(accelMods & GDK_SHIFT_MASK) {
    label += "Shift+";
  }
  if(accelMods & GDK_CONTROL_MASK) {
    label += "Ctrl+";
  }
  if(accelMods & GDK_MOD1_MASK) {
    label += "Alt+";
  }
  switch(accelKey) {
  case GDK_F1:
    label += "F1";
    break;
  case GDK_F2:
    label += "F2";
    break;
  case GDK_F3:
    label += "F3";
    break;
  case GDK_F4:
    label += "F4";
    break;
  case GDK_F5:
    label += "F5";
    break;
  case GDK_F6:
    label += "F6";
    break;
  default:
    label += (char)accelKey;
    break;
  }

  return label;
}

GtkWidget *GTK_create_menu_item_with_icon(GtkWidget *menu,
					  char *label,
					  GtkSignalFunc func,
					  gpointer func_data,
					  const char **xpmData,
					  GtkWidget *toplevel,
					  GtkAccelGroup *accel_group,
					  unsigned int accel_key,
					  GdkModifierType accel_mods)
{
  int spacerWidth = 5;

  GtkWidget *menu_item = gtk_menu_item_new();

  GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
  gtk_widget_show(hbox);
  gtk_container_add(GTK_CONTAINER(menu_item), hbox);

  if(xpmData != NULL) {
    GdkBitmap *bitmap;
    GdkPixmap *pixmap = gdk_pixmap_create_from_xpm_d(toplevel->window,
						     &bitmap,
						     &toplevel->style->bg[GTK_STATE_NORMAL],
						     const_cast<char**>(xpmData));
    
    GtkWidget *icon = gtk_pixmap_new(pixmap, bitmap);
    gtk_widget_show(icon);
    gtk_box_pack_start(GTK_BOX(hbox), icon, FALSE, FALSE, 0);
  } else {
    spacerWidth += 24;
  }

  GtkWidget *spacerBox1 = gtk_hbox_new(FALSE, 0);
  gtk_widget_show(spacerBox1);
  gtk_widget_set_usize(spacerBox1, spacerWidth, -1);
  gtk_box_pack_start(GTK_BOX(hbox), spacerBox1, FALSE, FALSE, 0);
  
  GtkWidget *text = gtk_label_new(label);
  gtk_widget_show(text);
  gtk_box_pack_start(GTK_BOX(hbox), text, FALSE, FALSE, 0);

  gtk_menu_append(GTK_MENU(menu), menu_item);
  gtk_widget_show(menu_item);

  if(func != NULL) {
    // signal handling function
    gtk_signal_connect(GTK_OBJECT(menu_item),
		       "activate",
		       func,
		       func_data);
  }

  GtkWidget *spacerBox2 = gtk_hbox_new(FALSE, 0);
  gtk_widget_show(spacerBox2);
  gtk_widget_set_usize(spacerBox2, 30, -1);
  gtk_box_pack_start(GTK_BOX(hbox), spacerBox2, FALSE, FALSE, 0);

  if(accel_group != NULL) {
    gtk_widget_add_accelerator(menu_item,
			       "activate",
			       accel_group,
			       accel_key,
			       accel_mods,
			       GTK_ACCEL_VISIBLE);
    GtkWidget *accelLabel = gtk_label_new(get_accel_string(accel_key,
							   accel_mods).c_str());
    gtk_widget_show(accelLabel);
    gtk_box_pack_end(GTK_BOX(hbox), accelLabel, FALSE, FALSE, 0);
  } else {
    GtkWidget *spacerBox3 = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(spacerBox3);
    gtk_widget_set_usize(spacerBox3, 30, -1);
    gtk_box_pack_start(GTK_BOX(hbox), spacerBox3, FALSE, FALSE, 0);
  }
  
  return menu_item;
}

GtkWidget *GTK_create_menu_item_with_icon(GtkWidget *menu,
					  char *label,
					  GtkSignalFunc func,
					  gpointer data,
					  const char **xpmData,
					  GtkWidget *toplevel)
{
  return GTK_create_menu_item_with_icon(menu, label, func, data, xpmData, toplevel, NULL, 0, (GdkModifierType)0);
}

GtkWidget *GTK_create_menu_item_with_icon(GtkWidget *menu,
					  char *label,
					  GtkSignalFunc func,
					  gpointer data,
					  GtkAccelGroup *accel_group,
					  unsigned int accel_key,
					  unsigned int accel_mods)
{
  return GTK_create_menu_item_with_icon(menu, label, func, data, NULL, NULL, accel_group, accel_key, (GdkModifierType)accel_mods);
}

GtkWidget *GTK_create_menu_item_with_icon(GtkWidget *menu,
					  char *label,
					  GtkSignalFunc func,
					  gpointer data)
{
  return GTK_create_menu_item_with_icon(menu, label, func, data, NULL, NULL, NULL, 0, (GdkModifierType)0);
}

GtkWidget *GTK_create_menu_separator(GtkWidget *menu)
{
  GtkWidget *menu_item = gtk_menu_item_new();
  gtk_menu_append(GTK_MENU(menu), menu_item);
  gtk_widget_show(menu_item);
  
  return menu_item;
}

void Setup_dnd(GtkWidget *toplevel, void (*Signal_Func)(GtkWidget  *w,
							GdkDragContext *context,
							int x,
							int y,
							GtkSelectionData *data,
							unsigned int info,
							unsigned int time))
{
  static GtkTargetEntry mime_types[] = {
    { "x-url/http", 0, MIME_URL },
    { "x-url/ftp", 0, MIME_URL },
    { "x-url/*", 0, MIME_URL }, /* */
    { "text/uri-list", 0, MIME_URL },
    { "_NETSCAPE_URL", 0, MIME_URL },
    { "text/plain", 0, MIME_TEXT_PLAIN },
    { "text/html", 0, MIME_TEXT_HTML }
  };

  static int n_mime_types = sizeof(mime_types)/sizeof(mime_types[0]);

  gtk_signal_connect(GTK_OBJECT(toplevel), "drag_data_received",
		     GTK_SIGNAL_FUNC(Signal_Func),
		     NULL);

  gtk_drag_dest_set(toplevel,
		    (GtkDestDefaults)(GTK_DEST_DEFAULT_MOTION |
				      GTK_DEST_DEFAULT_HIGHLIGHT |
				      GTK_DEST_DEFAULT_DROP),
		    mime_types, n_mime_types,
		    (GdkDragAction)(GDK_ACTION_COPY | GDK_ACTION_MOVE));
}

int findNearestSelectedRow(GtkWidget *clist, unsigned int row_current)
{
  GList *node = GTK_CLIST(clist)->selection;
  int diff;
  unsigned int rowindex;
  unsigned int row_nearest;
#ifdef DEBUG
  cout << "newest selected row: " << row_current << "\n";
#endif
  rowindex = GPOINTER_TO_UINT(node->data);
#ifdef DEBUG
  cout << "rowindex: " << rowindex << "\n";
#endif
  row_nearest = rowindex;
  node = g_list_next(node);

  while (node)
  {
    rowindex = GPOINTER_TO_UINT(node->data);
    diff = abs((int)(row_current-row_nearest))-abs((int)(row_current-rowindex));
    if(row_current != rowindex &&
       ((diff == 0 && row_nearest > rowindex) || diff > 0)) {
      row_nearest = rowindex;
    }
    node = g_list_next(node);
  }
#ifdef DEBUG
  cout << "find: " << row_nearest << "\n";
#endif
  return row_nearest;
}

/*
GtkWidget *GTK_create_menu_item(GtkWidget *menu,
				char *label,
				GtkSignalFunc func,
				gpointer func_data,
				GtkAccelGroup *accel_group,
				unsigned int accel_key,
				unsigned int accel_mods)
{
  GtkWidget *menu_item = gtk_menu_item_new_with_label(label);
  gtk_menu_append(GTK_MENU(menu), menu_item);
  gtk_widget_show(menu_item);

  if(func != NULL) {
    // signal handling function
    gtk_signal_connect(GTK_OBJECT(menu_item),
		       "activate",
		       func,
		       func_data);
  }

  if(accel_group != NULL) {
    gtk_widget_add_accelerator(menu_item,
			       "activate",
			       accel_group,
			       accel_key,
			       accel_mods,
			       GTK_ACCEL_VISIBLE);
  }
  
  return menu_item;
}

GtkWidget *GTK_create_menu_item(GtkWidget *menu,
				char *label,
				GtkSignalFunc func,
				gpointer data)
{
  return GTK_create_menu_item(menu, label, func, data, NULL, 0, 0);
}
*/
