//
//  Aria - yet another download tool
//  Copyright (C) 2000, 2002 Tatsuhiro Tsujikawa
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

// $Id: gui_popup.cc,v 1.14 2002/04/03 13:33:52 tujikawa Exp $

#include <iostream>
#include "aria.h"
#include "gui_utils.h"

#include "pixmaps/config.xpm"
#include "pixmaps/start.xpm"
#include "pixmaps/pause.xpm"
#include "pixmaps/trash.xpm"
#include "pixmaps/restart.xpm"
#include "pixmaps/moveup.xpm"
#include "pixmaps/movedown.xpm"
#include "pixmaps/lockon.xpm"
#include "pixmaps/unlock.xpm"

extern void Download_start(GtkWidget *, gpointer data);
extern void Download_stop(GtkWidget *, gpointer data);
extern void Download_clear_c(GtkWidget *, gpointer data);
extern void Download_clear_with_file_c(GtkWidget *, gpointer data);
extern void Download_clear_crc(GtkWidget *, gpointer data);
extern gboolean Download_clear_md5(GtkWidget* w, gpointer data);
extern gboolean Download_check_crc(GtkWidget *, gpointer);
extern gboolean Download_check_md5(GtkWidget *, gpointer);
extern void Download_download_again(GtkWidget *, gpointer data);
extern gboolean Download_executeCommand(GtkWidget* w, gpointer data);
extern gboolean Option_item_individual(GtkWidget *w, gpointer data);
extern gboolean Item_lock_item(GtkWidget *, gpointer);
extern gboolean Item_unlock_item(GtkWidget *, gpointer);
extern gboolean Edit_copy_item(GtkWidget *w, gpointer data);
extern gboolean Edit_cut_item(GtkWidget *w, gpointer data);
extern gboolean Item_move_up(GtkWidget *, gpointer);
extern gboolean Item_move_down(GtkWidget *, gpointer);
extern gboolean Item_move_top(GtkWidget *, gpointer);
extern gboolean Item_move_bottom(GtkWidget *, gpointer);

GtkWidget *g_popupMenu;

gboolean Popup_download_again(GtkWidget *w, GdkEvent *event, gpointer data)
{
  if(event->type == GDK_BUTTON_PRESS) {
    Download_download_again(w, data);
    gtk_menu_popdown(GTK_MENU(g_popupMenu));
    return TRUE;
  }
  return FALSE;
}

void popup_executeCommand_buttonClicked_cb(GtkWidget* w, gpointer data)
{
  
}

void Create_popup_menu(GtkWidget *toplevel)
{
  GtkWidget *start_item;
  GtkWidget *stop_item;
  GtkWidget *clear_item;
  GtkWidget *downloadAgain_item;

  GtkWidget *config_item;

  GtkWidget *lock_item;
  GtkWidget *unlock_item;
  
  GtkWidget *copyItem_item;
  GtkWidget *cutItem_item;

  GtkWidget *moveUp_item;
  GtkWidget *moveDown_item;
  GtkWidget *moveTop_item;
  GtkWidget *moveBottom_item;

  GtkWidget *clearCRC_item;
  GtkWidget *clearMD5_item;
  GtkWidget *checkCRC_item;
  GtkWidget *checkMD5_item;

  GtkWidget* execCommand_item;

  g_popupMenu = gtk_menu_new();

  start_item = GTK_create_menu_item_with_icon(g_popupMenu,
					      _("Start"),
					      GTK_SIGNAL_FUNC(Download_start),
					      NULL,
					      start_xpm,
					      toplevel);

  stop_item = GTK_create_menu_item_with_icon(g_popupMenu,
					     _("Stop"),
					     GTK_SIGNAL_FUNC(Download_stop),
					     NULL,
					     pause_xpm,
					     toplevel);

  clear_item = GTK_create_menu_item_with_icon(g_popupMenu,
					      _("Clear"),
					      GTK_SIGNAL_FUNC(Download_clear_c),
					      NULL,
					      trash_xpm,
					      toplevel);

  clear_item = GTK_create_menu_item_with_icon(g_popupMenu,
					      _("Clear and delete files"),
					      GTK_SIGNAL_FUNC(Download_clear_with_file_c),
					      NULL);

  downloadAgain_item = GTK_create_menu_item_with_icon(g_popupMenu,
						      _("Download again"),
						      GTK_SIGNAL_FUNC(Download_download_again),
						      NULL,
						      restart_xpm,
						      toplevel);

  GTK_create_menu_separator(g_popupMenu);

  config_item = GTK_create_menu_item_with_icon(g_popupMenu,
					       _("Selected item option"),
					       GTK_SIGNAL_FUNC(Option_item_individual),
					       NULL,
					       config_xpm,
					       toplevel);
  GTK_create_menu_separator(g_popupMenu);

  lock_item = GTK_create_menu_item_with_icon(g_popupMenu,
					     _("Lock"),
					     GTK_SIGNAL_FUNC(Item_lock_item),
					     NULL,
					     lockon_xpm,
					     toplevel);

  unlock_item = GTK_create_menu_item_with_icon(g_popupMenu,
					       _("Unlock"),
					       GTK_SIGNAL_FUNC(Item_unlock_item),
					       NULL,
					       unlock_xpm,
					       toplevel);
  GTK_create_menu_separator(g_popupMenu);

  copyItem_item = GTK_create_menu_item_with_icon(g_popupMenu,
						 _("Copy selected items"),
						 GTK_SIGNAL_FUNC(Edit_copy_item),
						 NULL);
  cutItem_item = GTK_create_menu_item_with_icon(g_popupMenu,
						_("Cut selected items"),
						GTK_SIGNAL_FUNC(Edit_cut_item),
						NULL);

  GTK_create_menu_separator(g_popupMenu);

  moveUp_item = GTK_create_menu_item_with_icon(g_popupMenu,
					       _("Move item up"),
					       GTK_SIGNAL_FUNC(Item_move_up),
					       NULL,
					       moveup_xpm,
					       toplevel);

  moveDown_item = GTK_create_menu_item_with_icon(g_popupMenu,
						 _("Move item down"),
						 GTK_SIGNAL_FUNC(Item_move_down),
						 NULL,
						 movedown_xpm,
						 toplevel);
  
  moveTop_item = GTK_create_menu_item_with_icon(g_popupMenu,
				      _("Move item to top"),
				      GTK_SIGNAL_FUNC(Item_move_top),
				      NULL);

  moveBottom_item = GTK_create_menu_item_with_icon(g_popupMenu,
					 _("Move item to bottom"),
					 GTK_SIGNAL_FUNC(Item_move_bottom),
					 NULL); 
  GTK_create_menu_separator(g_popupMenu);


  checkCRC_item = GTK_create_menu_item_with_icon(g_popupMenu,
				       _("Check CRC"),
				       GTK_SIGNAL_FUNC(Download_check_crc),
				       NULL);


  checkMD5_item = GTK_create_menu_item_with_icon(g_popupMenu,
				       _("Check MD5"),
				       GTK_SIGNAL_FUNC(Download_check_md5),
				       NULL);

  GTK_create_menu_separator(g_popupMenu);


  clearCRC_item = GTK_create_menu_item_with_icon(g_popupMenu,
				       _("Clear CRC"),
				       GTK_SIGNAL_FUNC(Download_clear_crc),
				       NULL);

  clearMD5_item = GTK_create_menu_item_with_icon(g_popupMenu,
				       _("Clear MD5"),
				       GTK_SIGNAL_FUNC(Download_clear_md5),
				       NULL);

  GTK_create_menu_separator(g_popupMenu);

  execCommand_item = GTK_create_menu_item_with_icon(g_popupMenu,
						    _("Execute command"),
						    GTK_SIGNAL_FUNC(Download_executeCommand),
						    NULL);

  //GtkWidget *popup_item = gtk_menu_item_new_with_label(_("Popup menu"));
  //gtk_widget_show(popup_item);
  //gtk_menu_item_set_submenu(GTK_MENU_ITEM(popup_item), g_popupMenu);
  gtk_widget_show_all(g_popupMenu);
}
