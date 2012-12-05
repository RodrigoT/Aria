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

// $Id: gui_utils.h,v 1.3 2001/10/12 16:47:39 tujikawa Exp $

#ifndef _GUI_UTILS_H_
#define _GUI_UTILS_H_
#include <string>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "aria_mime.h"

using namespace std;

GtkWidget *GTK_create_menu_item_with_icon(GtkWidget *menu,
                                          char *label,
                                          GCallback func,
                                          gpointer func_data,
                                          const char **xpmData,
                                          GtkWidget *toplevel,
                                          GtkAccelGroup *accel_group,
                                          unsigned int accel_key,
                                          GdkModifierType accel_mods);

GtkWidget *GTK_create_menu_item_with_icon(GtkWidget *menu,
                                          char *label,
                                          GCallback func,
                                          gpointer func_data,
                                          const char **xpmData,
                                          GtkWidget *toplevel);

GtkWidget *GTK_create_menu_item_with_icon(GtkWidget *menu,
                                          char *label,
                                          GCallback func,
                                          gpointer data,
                                          GtkAccelGroup *accel_group,
                                          unsigned int accel_key,
                                          unsigned int accel_mods);

GtkWidget *GTK_create_menu_item_with_icon(GtkWidget *menu,
                                          char *label,
                                          GCallback func,
                                          gpointer func_data);

GtkWidget *GTK_create_menu_separator(GtkWidget *menu);

void Setup_dnd(GtkWidget *toplevel, void (*Signal_Func)(GtkWidget  *w,
                                                        GdkDragContext *context,
                                                        int x,
                                                        int y,
                                                        GtkSelectionData *data,
                                                        unsigned int info,
                                                        unsigned int time));

int findNearestSelectedRow(GtkWidget *clist, unsigned int row_current);

bool Create_pixmap_from_file(GdkPixbuf **pixmap_ptr,
                             GdkBitmap **bitmap_ptr,
                             const string &filepath,
                             GtkWidget *styleWidget);

#endif // _GUI_UTILS_H_
