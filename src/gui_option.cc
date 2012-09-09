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

// $Id: gui_option.cc,v 1.38 2001/11/19 16:26:21 tujikawa Exp $

#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include "aria.h"
#include "ItemCell.h"
#include "ItemOption.h"
#include "AppOption.h"
#include "ListManager.h"
#include "HistoryWindow.h"
#include "Basket.h"
#include "ItemList.h"
#include "Dialog.h"
#include "ShortCutKey.h"
#include "gui_utils.h"
#include "pixmaps/config.xpm"
#include "pixmaps/config_all.xpm"

extern AppOption *g_appOption;
extern GtkWidget *g_consoleText;
extern ItemOption *g_itemOption;
extern ListManager *g_listManager;
extern ItemCell *g_consoleItem;
extern ItemList *g_itemList;
extern Dialog *g_cDialog;
extern HistoryWindow *g_historyWindow;

extern Basket *basket;
static GtkWidget *item_individual_item;
static GtkWidget *item_default_list_item;
static GtkWidget *item_default_item;
static GtkWidget *program_item;
static GtkWidget *history_item;
static GtkWidget *basket_item;
static GtkWidget *clear_syslog;
static GtkWidget *reset_to_default_item;
static GtkWidget *reset_to_default_nosd_item;
static GtkWidget *reset_to_default_norec_item;
static GtkWidget *reset_to_default_all_item;
static GtkWidget *reset_to_default_all_nosd_item;
static GtkWidget *reset_to_default_all_norec_item;


void Option_set_sensitive__no_item_selected()
{
  gtk_widget_set_sensitive(item_individual_item, FALSE);
  gtk_widget_set_sensitive(reset_to_default_item, FALSE);
  gtk_widget_set_sensitive(reset_to_default_nosd_item, FALSE);
  gtk_widget_set_sensitive(reset_to_default_norec_item, FALSE);
}

void Option_set_sensitive__items_selected()
{
  gtk_widget_set_sensitive(item_individual_item, TRUE);
  gtk_widget_set_sensitive(reset_to_default_item, TRUE);
  gtk_widget_set_sensitive(reset_to_default_nosd_item, TRUE);
  gtk_widget_set_sensitive(reset_to_default_norec_item, TRUE);
}

void Option_set_sensitive__list_empty()
{
  gtk_widget_set_sensitive(reset_to_default_all_item, FALSE);
  gtk_widget_set_sensitive(reset_to_default_all_nosd_item, FALSE);
  gtk_widget_set_sensitive(reset_to_default_all_norec_item, FALSE);
}

void Option_set_sensitive__list_not_empty()
{
  gtk_widget_set_sensitive(reset_to_default_all_item, TRUE);
  gtk_widget_set_sensitive(reset_to_default_all_nosd_item, TRUE);  
  gtk_widget_set_sensitive(reset_to_default_all_norec_item, TRUE);  
}

gboolean Option_history(GtkWidget *w, gpointer data)
{
  g_historyWindow->show();
  return TRUE;
}

// clear system log
static gboolean Option_clear_syslog(GtkWidget *w, gpointer data)
{
  g_cDialog->hide();
  //pthread_mutex_lock(&itemlistlock); //mod 2001/4/11
  g_consoleItem->Clear_log();
  /*gtk_text_freeze(GTK_TEXT(g_consoleText));
  unsigned int length = gtk_text_get_length(GTK_TEXT(g_consoleText));
  gtk_text_set_point(GTK_TEXT(g_consoleText), length);
  gtk_text_backward_delete(GTK_TEXT(g_consoleText), length);
  gtk_text_thaw(GTK_TEXT(g_consoleText));*/
  gtk_text_buffer_set_text( gtk_text_view_get_buffer(GTK_TEXT_VIEW(w)), "", 0);
  //pthread_mutex_unlock(&itemlistlock); //mod 2001/4/11
  return TRUE;
}


static gboolean Option_clear_syslog_c(GtkWidget *w, gpointer data)
{
  if(g_appOption->ret_confirm_clearlog()) {
    g_cDialog->setup(_("Clear system log"),
		   _("Are you sure to clear system log?"),
		   (gboolean (*)(GtkWidget *, GtkWidget *))Option_clear_syslog);
    g_cDialog->set_cancel_button_visible(false);
    g_cDialog->show();
  } else {
    Option_clear_syslog(NULL, NULL);
  }
  return TRUE;
}

gboolean Option_item_individual(GtkWidget *w, gpointer data)
{
  ListEntry *listentry = g_listManager->ret_Current_listentry();
  //GList *node = g_list_last(GTK_CLIST(listentry->ret_Dl_clist())->selection);
  gint selectedrow = listentry->getLastSelectedRow();
  if(selectedrow == -1) return TRUE;
//  ItemCell* itemcell = (ItemCell*)gtk_clist_get_row_data(GTK_CLIST(listentry->ret_Dl_clist()), GPOINTER_TO_UINT(node->data));
  ItemCell* itemcell = listentry->getItemCellByRow(selectedrow);
  //pthread_mutex_lock(&itemlistlock); //mod 2001/4/11
  g_itemOption->setOptionValues(itemcell,
				itemcell->ret_Options_opt(),
				listentry);
  g_itemOption->show();
  //pthread_mutex_unlock(&itemlistlock); //mod 2001/4/11

  return TRUE;
}

gboolean Option_item_default_list(GtkWidget *w, gpointer data)
{
  ListEntry *listentry = g_listManager->ret_Current_listentry();
  ItemCell *itemcell = listentry->ret_Default_item();
  g_itemOption->setOptionValues(itemcell,
				itemcell->ret_Options_opt(),
				listentry);
  g_itemOption->show();

  return TRUE;
}

static gboolean Option_item_default(GtkWidget *w, gpointer data)
{
  // fix this
  g_itemOption->setOptionValues(g_consoleItem,
				g_consoleItem->ret_Options_opt(),
				NULL);
  g_itemOption->show();
  return TRUE;
}

static gboolean Option_reset_to_default_all(GtkWidget *w, gpointer data)
{
  ListEntry *listentry = g_listManager->ret_Current_listentry();
  const Options& console_option = listentry->ret_Options();
  for(unsigned int rowindex = 0; rowindex < (unsigned int)GTK_CLIST(listentry->ret_Dl_clist())->rows; ++rowindex) {
    ItemCell *itemcell = (ItemCell*)gtk_clist_get_row_data(GTK_CLIST(listentry->ret_Dl_clist()), rowindex);
    itemcell->get_Options_Lock();
    itemcell->ret_Options_opt() = console_option;

    listentry->Set_clist_column__try(rowindex, itemcell->ret_Count(), itemcell->ret_Options_opt().ret_Retry());
    listentry->Set_clist_column__save(rowindex, itemcell->ret_Options_opt().ret_Store_Dir());
    if(itemcell->ret_URL_Container_opt().ret_Protocol() == "http:"
#ifdef HAVE_OPENSSL
       || itemcell->ret_URL_Container_opt().ret_Protocol() == "https:"
#endif // HAVE_OPENSSL
       ) {
      listentry->Set_clist_column__rec(rowindex, itos(itemcell->ret_Options_opt().ret_recurse_count()));
    } else {
      listentry->Set_clist_column__rec(rowindex, itos(itemcell->ret_Options_opt().ret_FTP_recurse_count()));
    }
    itemcell->Raise_option_update_flag();
    itemcell->release_Options_Lock();
  }
  return TRUE;
}

// same as Option_reset_to_default_all except that store directory does not change
static gboolean Option_reset_to_default_all_nosd(GtkWidget *w, gpointer data)
{
  ListEntry *listentry = g_listManager->ret_Current_listentry();
  const Options& console_option = listentry->ret_Options();
  for(unsigned int rowindex = 0; rowindex < (unsigned int)GTK_CLIST(listentry->ret_Dl_clist())->rows; ++rowindex) {
    ItemCell *itemcell = (ItemCell*)gtk_clist_get_row_data(GTK_CLIST(listentry->ret_Dl_clist()), rowindex);
    itemcell->get_Options_Lock();
    string storedir_temp = itemcell->ret_Options_opt().ret_Store_Dir();
    itemcell->ret_Options_opt() = console_option;
    itemcell->ret_Options_opt().set_Store_Dir(storedir_temp);

    listentry->Set_clist_column__try(rowindex, itemcell->ret_Count(), itemcell->ret_Options_opt().ret_Retry());

    if(itemcell->ret_URL_Container_opt().ret_Protocol() == "http:"
#ifdef HAVE_OPENSSL
       || itemcell->ret_URL_Container_opt().ret_Protocol() == "https:"
#endif // HAVE_OPENSSL
       ) {
      listentry->Set_clist_column__rec(rowindex, itos(itemcell->ret_Options_opt().ret_recurse_count()));
    } else {
      listentry->Set_clist_column__rec(rowindex, itos(itemcell->ret_Options_opt().ret_FTP_recurse_count()));
    }

    itemcell->Raise_option_update_flag();
    itemcell->release_Options_Lock();
  }
  return TRUE;
}

void recursive_option_backup(Options& console_option, const Options& options)
{
  console_option.set_recurse_count(options.ret_recurse_count());
  console_option.set_with_hostname_dir(options.ret_with_hostname_dir());
  console_option.set_abs2rel_url(options.ret_abs2rel_url());
  console_option.set_force_convert(options.ret_force_convert());
  console_option.set_delete_comment(options.ret_delete_comment());
  console_option.set_delete_javascript(options.ret_delete_javascript());
  console_option.set_delete_iframe(options.ret_delete_iframe());
  console_option.set_no_other_host(options.ret_no_other_host());
  console_option.set_no_ascend(options.ret_no_ascend());
  console_option.set_only_relative_links(options.ret_only_relative_links());
  console_option.set_Referer_override(options.ret_Referer_override());
  console_option.set_Follow_ftp_link(options.ret_Follow_ftp_link());
  console_option.set_convert_tilde(options.ret_convert_tilde());
  console_option.set_no_redownload_HTTP_recurse(options.ret_no_redownload_HTTP_recurse());
  console_option.set_HTTP_recurse_add_paste(options.ret_HTTP_recurse_add_paste());
  console_option.set_parse_target_list(options.ret_parse_target_list());
  console_option.set_use_down_filter(options.ret_use_down_filter());
  console_option.set_filter_down_target_list(options.ret_filter_down_target_list());
  console_option.set_filter_nodown_target_list(options.ret_filter_nodown_target_list());
  console_option.set_ign_domain_list(options.ret_ign_domain_list());
  console_option.set_activated_parse_target_list(options.ret_activated_parse_target_list());
  console_option.set_activated_filter_down_target_list(options.ret_activated_filter_down_target_list());
  console_option.set_activated_filter_nodown_target_list(options.ret_activated_filter_nodown_target_list());
  console_option.set_activated_ign_domain_list(options.ret_activated_ign_domain_list());
  console_option.set_use_tag_href(options.ret_use_tag_href());
  console_option.set_use_tag_src(options.ret_use_tag_src());
  console_option.set_use_tag_background(options.ret_use_tag_background());
  console_option.set_use_tag_code(options.ret_use_tag_code());
  // FTP recursive
  console_option.set_FTP_no_ascend(options.ret_FTP_no_ascend());
  console_option.set_FTP_use_filter(options.ret_FTP_use_filter());
  console_option.set_FTP_allow_crawl_subdir(options.ret_FTP_allow_crawl_subdir());
  console_option.set_FTP_get_symlink_as_realfile(options.ret_FTP_get_symlink_as_realfile());
  console_option.set_FTP_recurse_add_paste(options.ret_FTP_recurse_add_paste());
  console_option.set_FTP_recurse_count(options.ret_FTP_recurse_count());
  console_option.set_FTP_filter_target_list(options.ret_FTP_filter_target_list());
  console_option.set_FTP_activated_filter_target_list(options.ret_FTP_activated_filter_target_list());
}

static gboolean Option_reset_to_default_all_norec(GtkWidget *w, gpointer data)
{
  //pthread_mutex_lock(&itemlistlock); //mod 2001/4/11
  ListEntry *listentry = g_listManager->ret_Current_listentry();
  Options console_option = listentry->ret_Options();
  for(unsigned int rowindex = 0; rowindex < (unsigned int)GTK_CLIST(listentry->ret_Dl_clist())->rows; ++rowindex) {
    ItemCell *itemcell = (ItemCell*)gtk_clist_get_row_data(GTK_CLIST(listentry->ret_Dl_clist()), rowindex);
    itemcell->get_Options_Lock();
    const Options& options = itemcell->ret_Options_opt();
    recursive_option_backup(console_option, options);
    // preserve current save directory
    console_option.set_Store_Dir(options.ret_Store_Dir());

    itemcell->ret_Options_opt() = console_option;

    listentry->Set_clist_column__try(rowindex, itemcell->ret_Count(), itemcell->ret_Options_opt().ret_Retry());

    itemcell->Raise_option_update_flag();
    itemcell->release_Options_Lock();
  }
  //pthread_mutex_unlock(&itemlistlock); //mod 2001/4/11
  return TRUE;
}

static gboolean Option_reset_to_default(GtkWidget *w, gpointer data)
{
  ListEntry *listentry = g_listManager->ret_Current_listentry();

  GList *node = GTK_CLIST(listentry->ret_Dl_clist())->selection;
  const Options& console_option = listentry->ret_Options();
  while(node) {
    unsigned int rowindex = GPOINTER_TO_UINT(node->data);
    ItemCell* itemcell = (ItemCell*)gtk_clist_get_row_data(GTK_CLIST(listentry->ret_Dl_clist()), rowindex);
    itemcell->get_Options_Lock();
    itemcell->ret_Options_opt() = console_option;

    listentry->Set_clist_column__try(rowindex, itemcell->ret_Count(), itemcell->ret_Options_opt().ret_Retry());
    listentry->Set_clist_column__save(rowindex, itemcell->ret_Options_opt().ret_Store_Dir());
    itemcell->Raise_option_update_flag();
    itemcell->release_Options_Lock();
    node = g_list_next(node);
  }
  return TRUE;
}

static gboolean Option_reset_to_default_nosd(GtkWidget *w, gpointer data)
{
  ListEntry *listentry = g_listManager->ret_Current_listentry();

  GList *node = GTK_CLIST(listentry->ret_Dl_clist())->selection;
  const Options& console_option = listentry->ret_Options();
  while(node) {
    unsigned int rowindex = GPOINTER_TO_UINT(node->data);
    ItemCell* itemcell = (ItemCell*)gtk_clist_get_row_data(GTK_CLIST(listentry->ret_Dl_clist()), rowindex);
    itemcell->get_Options_Lock();

    string storedir_temp = itemcell->ret_Options_opt().ret_Store_Dir();
    itemcell->ret_Options_opt() = console_option;
    itemcell->ret_Options_opt().set_Store_Dir(storedir_temp);

    listentry->Set_clist_column__try(rowindex, itemcell->ret_Count(), itemcell->ret_Options_opt().ret_Retry());

    itemcell->Raise_option_update_flag();
    itemcell->release_Options_Lock();
    node = g_list_next(node);
  }
  return TRUE;
}

static gboolean Option_reset_to_default_norec(GtkWidget *w, gpointer data)
{
  ListEntry *listentry = g_listManager->ret_Current_listentry();
  GList *node = GTK_CLIST(listentry->ret_Dl_clist())->selection;
  Options console_option = listentry->ret_Options();
  while(node) {
    unsigned int rowindex = GPOINTER_TO_UINT(node->data);
    ItemCell* itemcell = (ItemCell*)gtk_clist_get_row_data(GTK_CLIST(listentry->ret_Dl_clist()), rowindex);
    itemcell->get_Options_Lock();

    const Options& options = itemcell->ret_Options_opt();
    recursive_option_backup(console_option, options);

    // save directory
    console_option.set_Store_Dir(options.ret_Store_Dir());

    itemcell->ret_Options_opt() = console_option;

    listentry->Set_clist_column__try(rowindex, itemcell->ret_Count(), itemcell->ret_Options_opt().ret_Retry());

    itemcell->Raise_option_update_flag();
    itemcell->release_Options_Lock();
    node = g_list_next(node);
  }
  return TRUE;
}

static gboolean Option_showBasket(GtkWidget *w, gpointer data) {
  basket->setMainWindowVisibleFlag(true);// fix this
  if(basket->isVisible()) {
    basket->hide();
  } else {
    basket->show();
  }

  return TRUE;
}

static gboolean Option_program(GtkWidget* w, gpointer data)
{
  g_appOption->Show_option_window();
  return TRUE;
}

void Create_option_menu(GtkWidget *toplevel, GtkWidget *menu_bar, GtkAccelGroup *accel_group)
{
  GtkWidget *menu;
  GtkWidget *root_item;

  //create option menu
  menu = gtk_menu_new();

  item_individual_item = GTK_create_menu_item_with_icon(menu,
							_("Selected item option"),
							GTK_SIGNAL_FUNC(Option_item_individual),
							NULL,
							config_xpm,
							toplevel,
							accel_group,
							SC_ITEMOPTION,
							SCM_ITEMOPTION);

  item_default_list_item = GTK_create_menu_item_with_icon(menu,
							  _("Default item option for current list"),
							  GTK_SIGNAL_FUNC(Option_item_default_list),
							  NULL,
							  config_all_xpm,
							  toplevel,
							  accel_group,
							  SC_ITEMOPTION_DEFAULT_LIST,
							  SCM_ITEMOPTION_DEFAULT_LIST);
					   
  GtkWidget *apply_item = GTK_create_menu_item_with_icon(menu,
					       _("Apply default item option"),
					       NULL,
					       NULL);
  GtkWidget *apply_menu = gtk_menu_new();
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(apply_item), apply_menu);
  //gtk_widget_show(apply_item);
  GtkWidget *selected_item = GTK_create_menu_item_with_icon(apply_menu,
						  _("To selected items"),
						  NULL,
						  NULL);

  GtkWidget *all_item = GTK_create_menu_item_with_icon(apply_menu,
					     _("To all items in current list"),
					     NULL,
					     NULL);

  GtkWidget *selected_menu = gtk_menu_new();
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(selected_item), selected_menu);

  reset_to_default_item = GTK_create_menu_item_with_icon(selected_menu,
					       _("All settings"),
					       GTK_SIGNAL_FUNC(Option_reset_to_default),
					       NULL,
					       accel_group,
					       SC_SETDEFAULT,
					       SCM_SETDEFAULT);

  reset_to_default_nosd_item = GTK_create_menu_item_with_icon(selected_menu,
						    _("Ignore save directory"),
						    GTK_SIGNAL_FUNC(Option_reset_to_default_nosd),
						    NULL,
						    accel_group,
						    SC_SETDEFAULT_NOSAVEDIR,
						    SCM_SETDEFAULT_NOSAVEDIR);

  reset_to_default_norec_item = GTK_create_menu_item_with_icon(selected_menu,
						     _("Ignore save directory and recursive settings"),
						     GTK_SIGNAL_FUNC(Option_reset_to_default_norec),
						     NULL);

  GtkWidget *all_menu = gtk_menu_new();
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(all_item), all_menu);

  reset_to_default_all_item = GTK_create_menu_item_with_icon(all_menu,
						   _("All settings"),
						   GTK_SIGNAL_FUNC(Option_reset_to_default_all),
						   NULL,
						   accel_group,
						   SC_SETDEFAULT_ALL,
						   SCM_SETDEFAULT_ALL);

  reset_to_default_all_nosd_item = GTK_create_menu_item_with_icon(all_menu,
							_("Ignore save directory"),
							GTK_SIGNAL_FUNC(Option_reset_to_default_all_nosd),
							NULL,
							accel_group,
							SC_SETDEFAULT_ALL_NOSAVEDIR,
							SCM_SETDEFAULT_ALL_NOSAVEDIR);

  reset_to_default_all_norec_item = GTK_create_menu_item_with_icon(all_menu,
							 _("Ignore save directory and recursive settings"),
							 GTK_SIGNAL_FUNC(Option_reset_to_default_all_norec),
							 NULL);

  GTK_create_menu_separator(menu);

  item_default_item = GTK_create_menu_item_with_icon(menu,
					   _("Default item option for new list"),
					   GTK_SIGNAL_FUNC(Option_item_default),
					   NULL);


  GTK_create_menu_separator(menu);

  program_item = GTK_create_menu_item_with_icon(menu,
				      _("Application settings"),
				      GTK_SIGNAL_FUNC(Option_program),
				      NULL,
				      accel_group,
				      SC_APPOPTION,
				      SCM_APPOPTION);

  GTK_create_menu_separator(menu);

  history_item = GTK_create_menu_item_with_icon(menu,
				      _("Download history"),
				      GTK_SIGNAL_FUNC(Option_history),
				      NULL,
				      accel_group,
				      SC_HISTORY,
				      SCM_HISTORY);

  basket_item = GTK_create_menu_item_with_icon(menu,
					       _("Toggle basket on/off"),
					       GTK_SIGNAL_FUNC(Option_showBasket),
					       NULL,
					       accel_group,
					       SC_SHOWBASKET,
					       SCM_SHOWBASKET);
  GTK_create_menu_separator(menu);

  clear_syslog = GTK_create_menu_item_with_icon(menu,
				      _("Clear system log"),
				      GTK_SIGNAL_FUNC(Option_clear_syslog_c),
				      NULL);


  Option_set_sensitive__no_item_selected();
  Option_set_sensitive__list_empty();

  root_item = gtk_menu_item_new_with_label(_("Option"));
  gtk_widget_show(root_item);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(root_item), menu);
  gtk_menu_bar_append(GTK_MENU_BAR(menu_bar), root_item);
}
