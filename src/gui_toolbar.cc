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

// $Id: gui_toolbar.cc,v 1.23 2001/10/17 13:06:35 tujikawa Exp $

#include "AppOption.h"
#include "ThreadManager.h"
#include "aria.h"
#include "pixmaps/url_list.xpm"
#include "pixmaps/open.xpm"
#include "pixmaps/paste.xpm"
#include "pixmaps/config.xpm"
#include "pixmaps/config_all.xpm"
#include "pixmaps/start.xpm"
#include "pixmaps/start_all.xpm"
#include "pixmaps/save.xpm"
#include "pixmaps/pause.xpm"
#include "pixmaps/pause_all.xpm"
#include "pixmaps/trash.xpm"
#include "pixmaps/restart.xpm"
#include "pixmaps/moveup.xpm"
#include "pixmaps/movedown.xpm"
#include "pixmaps/lockon.xpm"
#include "pixmaps/unlock.xpm"
#include "pixmaps/track.xpm"

extern void File_quit();
extern gboolean File_open_URL_list(GtkWidget *w, gpointer unused);
extern gboolean File_open_CRC_list(GtkWidget *w, gpointer unused);
extern gboolean File_open_Saved_list(GtkWidget *w, gpointer unused);
extern gboolean File_save_list(GtkWidget *w, gpointer unused);

extern gboolean Download_clear_c(GtkWidget *w, gpointer data);
extern gboolean Download_stop(GtkWidget *w, gpointer data);
extern gboolean Download_start(GtkWidget *w, gpointer data);
extern gboolean Download_download_again(GtkWidget *w, gpointer data);
extern gboolean Option_item_individual(GtkWidget *w, gpointer data);
extern gboolean Item_lock_item(GtkWidget *w, gpointer data);
extern gboolean Item_unlock_item(GtkWidget *w, gpointer data);
extern gboolean Item_move_up(GtkWidget *w, gpointer data);
extern gboolean Item_move_down(GtkWidget *w, gpointer data);

extern gboolean Download_clear_all_c(GtkWidget *w, gpointer data);
extern gboolean Download_start_all(GtkWidget *w, gpointer data);
extern gboolean Download_stop_all(GtkWidget *w, gpointer data);
extern gboolean Option_item_default_list(GtkWidget *w, gpointer data);

extern gboolean Paste_get_clipboard(GtkWidget *widget, gpointer data);
extern gboolean Paste_selection_received (GtkWidget *widget, GtkSelectionData *selection_data, gpointer data);

extern AppOption *g_appOption;
extern int g_threadLimit;
extern ListManager *g_listManager;

static GtkWidget *open_saved_button, *save_button;
static GtkWidget *open_url_button;

//static GtkWidget *open_crc_button;
static GtkWidget *paste_url_button;
static GtkWidget *start_button, *start_all_button;
static GtkWidget *start_again_button;
static GtkWidget *lock_button, *unlock_button;
static GtkWidget *stop_button, *stop_all_button;
static GtkWidget *delete_button;
static GtkWidget *pref_button, *pref_default_button;
static GtkWidget *tb_thread_spin, *thread_spin;
static GtkWidget *inc_thread_button, *dec_thread_button;
static GtkWidget *moveup_button, *movedown_button;
static GtkWidget *track_button;
static GtkWidget *track_toggle_button;
static bool track_download = false;
static pthread_mutex_t track_lock;

void Toolbar_set_sensitive__no_item_selected()
{
    gtk_widget_set_sensitive(delete_button, FALSE);
    gtk_widget_set_sensitive(start_button, FALSE);
    gtk_widget_set_sensitive(stop_button, FALSE);
    gtk_widget_set_sensitive(start_again_button, FALSE);
    gtk_widget_set_sensitive(pref_button, FALSE);
    gtk_widget_set_sensitive(moveup_button, FALSE);
    gtk_widget_set_sensitive(movedown_button, FALSE);
    gtk_widget_set_sensitive(lock_button, FALSE);
    gtk_widget_set_sensitive(unlock_button, FALSE);
}

void Toolbar_set_sensitive__items_selected()
{
    gtk_widget_set_sensitive(delete_button, TRUE);
    gtk_widget_set_sensitive(start_button, TRUE);
    gtk_widget_set_sensitive(stop_button, TRUE);
    gtk_widget_set_sensitive(start_again_button, TRUE);
    gtk_widget_set_sensitive(pref_button, TRUE);
    gtk_widget_set_sensitive(moveup_button, TRUE);
    gtk_widget_set_sensitive(movedown_button, TRUE);
    gtk_widget_set_sensitive(lock_button, TRUE);
    gtk_widget_set_sensitive(unlock_button, TRUE);
}

void Toolbar_set_sensitive__list_empty()
{
    gtk_widget_set_sensitive(start_all_button, FALSE);
    gtk_widget_set_sensitive(stop_all_button, FALSE);
}

void Toolbar_set_sensitive__list_not_empty()
{
    gtk_widget_set_sensitive(start_all_button, TRUE);
    gtk_widget_set_sensitive(stop_all_button, TRUE);
}

void Toolbar_set_sensitive__thread()
{
    if (g_appOption->getThreadMax() >= g_threadLimit) {
        gtk_widget_set_sensitive(inc_thread_button, FALSE);
        gtk_widget_set_sensitive(dec_thread_button, TRUE);
    } else if (g_appOption->getThreadMax() <= 1) {
        gtk_widget_set_sensitive(inc_thread_button, TRUE);
        gtk_widget_set_sensitive(dec_thread_button, FALSE);
    } else {
        gtk_widget_set_sensitive(inc_thread_button, TRUE);
        gtk_widget_set_sensitive(dec_thread_button, TRUE);
    }
}

static bool thread_spin_quark = false;
void Toolbar_set_thread_spin(int n_thread)
{
    thread_spin_quark = true;
    GtkObject *adjustment_thread = gtk_adjustment_new(n_thread,
                                                      1,
                                                      g_threadLimit,
                                                      1,
                                                      1,
                                                      0);
    gtk_spin_button_configure(GTK_SPIN_BUTTON(thread_spin),
                              GTK_ADJUSTMENT(adjustment_thread),
                              1.0, 0);
    thread_spin_quark = false;
}

void Thread_spin_changed(GtkEditable *w,
                         gpointer data)
{
    if (thread_spin_quark) return;
    ListEntry *listentry = g_listManager->ret_Current_listentry();
    listentry->setThreadLimit(gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(thread_spin)));
}

gboolean Toolbar_increase_thread(GtkWidget *w, gpointer data)
{
    g_appOption->Increase_thread();
    return(TRUE);
}

gboolean Toolbar_decrease_thread(GtkWidget *w, gpointer data)
{
    g_appOption->Decrease_thread();
    return(TRUE);
}

static void Track_download(GtkWidget *w, gpointer data)
{
    pthread_mutex_lock(&track_lock);
    if (GTK_TOGGLE_BUTTON(w)->active) {
        track_download = true;
    } else {
        track_download = false;
    }
    pthread_mutex_unlock(&track_lock);
}

bool Is_track_required()
{
    bool retval;
    pthread_mutex_lock(&track_lock);
    retval = track_download;
    pthread_mutex_unlock(&track_lock);
    return retval;
}

void Track_enabled(bool flag)
{
    if (flag) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(track_button), TRUE);
        track_download = true;
    } else {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(track_button), FALSE);
        track_download = false;
    }
}

GtkWidget *Create_toolbar(GtkWidget *toplevel)
{
    GdkPixmap *icon;
    GdkBitmap *mask;
    GtkWidget *iconw;

    //GtkWidget *toolbar = gtk_toolbar_new(GTK_ORIENTATION_HORIZONTAL,
//				       GTK_TOOLBAR_ICONS);
    GtkWidget *toolbar = gtk_toolbar_new();
    gtk_widget_show(toolbar);
    gtk_container_set_border_width(GTK_CONTAINER(toolbar), 5);
//  gtk_toolbar_set_space_size(GTK_TOOLBAR(toolbar), 2);

    // open saved list
    {
        icon = gdk_pixmap_create_from_xpm_d(toplevel->window,
                                            &mask,
                                            &toplevel->style->white,
                                            const_cast<char **>(open_xpm));
        iconw = gtk_pixmap_new(icon, mask);
        open_saved_button = gtk_toolbar_append_item(GTK_TOOLBAR(toolbar),
                                                    "List",
                                                    _("Open saved list"),
                                                    "Private",
                                                    iconw,
                                                    GTK_SIGNAL_FUNC(File_open_Saved_list),
                                                    NULL);
        //gtk_toolbar_append_space(GTK_TOOLBAR(toolbar));
    }
    // open URL list
    {
        icon = gdk_pixmap_create_from_xpm_d(toplevel->window,
                                            &mask,
                                            &toplevel->style->white,
                                            const_cast<char **>(url_list_xpm));
        iconw = gtk_pixmap_new(icon, mask);
        open_url_button = gtk_toolbar_append_item(GTK_TOOLBAR(toolbar),
                                                  "URL",
                                                  _("Open URL list"),
                                                  "Private",
                                                  iconw,
                                                  GTK_SIGNAL_FUNC(File_open_URL_list),
                                                  NULL);
        //gtk_toolbar_append_space(GTK_TOOLBAR(toolbar));
    }
    // open CRC list
    /*
    {
      icon = gdk_pixmap_create_from_xpm_d(toplevel->window,
    				&mask,
    				&toplevel->style->white,
    				tb_new_crc_xpm);
      iconw = gtk_pixmap_new(icon, mask);
      open_crc_button = gtk_toolbar_append_item(GTK_TOOLBAR(toolbar),
    				      "CRC",
    				      _("Open CRC list"),
    				      "Private",
    				      iconw,
    				      GTK_SIGNAL_FUNC(File_open_CRC_list),
    				      NULL);
      //gtk_toolbar_append_space(GTK_TOOLBAR(toolbar));
    }
    */
    // save current list
    {
        icon = gdk_pixmap_create_from_xpm_d(toplevel->window,
                                            &mask,
                                            &toplevel->style->white,
                                            const_cast<char **>(save_xpm));
        iconw = gtk_pixmap_new(icon, mask);
        save_button = gtk_toolbar_append_item(GTK_TOOLBAR(toolbar),
                                              "List",
                                              _("Save list"),
                                              "Private",
                                              iconw,
                                              GTK_SIGNAL_FUNC(File_save_list),
                                              NULL);
        //gtk_toolbar_append_space(GTK_TOOLBAR(toolbar));
    }
    gtk_toolbar_append_space(GTK_TOOLBAR(toolbar));
    gtk_toolbar_append_space(GTK_TOOLBAR(toolbar));
    // paste URL
    {
        icon = gdk_pixmap_create_from_xpm_d(toplevel->window,
                                            &mask,
                                            &toplevel->style->white,
                                            const_cast<char **>(paste_xpm));
        iconw = gtk_pixmap_new(icon, mask);
        paste_url_button = gtk_toolbar_append_item(GTK_TOOLBAR(toolbar),
                                                   "Paste URL list",
                                                   _("Paste URL list from clipboard"),
                                                   "Private",
                                                   iconw,
                                                   GTK_SIGNAL_FUNC(Paste_get_clipboard),
                                                   NULL);
        g_signal_connect(GTK_OBJECT(paste_url_button), "selection_received",
                         GTK_SIGNAL_FUNC(Paste_selection_received),
                         NULL);
        //gtk_toolbar_append_space(GTK_TOOLBAR(toolbar));
    }
    //gtk_toolbar_append_space(GTK_TOOLBAR(toolbar));
    gtk_toolbar_append_space(GTK_TOOLBAR(toolbar));

    // delete selected items
    {
        icon = gdk_pixmap_create_from_xpm_d(toplevel->window,
                                            &mask,
                                            &toplevel->style->white,
                                            const_cast<char **>(trash_xpm));
        iconw = gtk_pixmap_new(icon, mask);
        delete_button = gtk_toolbar_append_item(GTK_TOOLBAR(toolbar),
                                                "Del",
                                                _("Clear item(s)"),
                                                "Private",
                                                iconw,
                                                GTK_SIGNAL_FUNC(Download_clear_c),
                                                NULL);
        //gtk_toolbar_append_space(GTK_TOOLBAR(toolbar));
    }
    // start downloading selected items
    {
        icon = gdk_pixmap_create_from_xpm_d(toplevel->window,
                                            &mask,
                                            &toplevel->style->white,
                                            const_cast<char **>(start_xpm));
        iconw = gtk_pixmap_new(icon, mask);
        start_button = gtk_toolbar_append_item(GTK_TOOLBAR(toolbar),
                                               "Start",
                                               _("Start downloading selected item(s)"),
                                               "Private",
                                               iconw,
                                               GTK_SIGNAL_FUNC(Download_start),
                                               NULL);
        //gtk_toolbar_append_space(GTK_TOOLBAR(toolbar));
    }
    // stop downloading selected items
    {
        icon = gdk_pixmap_create_from_xpm_d(toplevel->window,
                                            &mask,
                                            &toplevel->style->white,
                                            const_cast<char **>(pause_xpm));
        iconw = gtk_pixmap_new(icon, mask);
        stop_button = gtk_toolbar_append_item(GTK_TOOLBAR(toolbar),
                                              "Stop",
                                              _("Stop downloading selected item(s)"),
                                              "Private",
                                              iconw,
                                              GTK_SIGNAL_FUNC(Download_stop),
                                              NULL);
        //gtk_toolbar_append_space(GTK_TOOLBAR(toolbar));
    }
    // download again selected item
    {
        icon = gdk_pixmap_create_from_xpm_d(toplevel->window,
                                            &mask,
                                            &toplevel->style->white,
                                            const_cast<char **>(restart_xpm));
        iconw = gtk_pixmap_new(icon, mask);
        start_again_button = gtk_toolbar_append_item(GTK_TOOLBAR(toolbar),
                                                     "Again",
                                                     _("Download again selected item(s)"),
                                                     "Private",
                                                     iconw,
                                                     GTK_SIGNAL_FUNC(Download_download_again),
                                                     NULL);
        //gtk_toolbar_append_space(GTK_TOOLBAR(toolbar));
    }
    // item option
    {
        icon = gdk_pixmap_create_from_xpm_d(toplevel->window,
                                            &mask,
                                            &toplevel->style->white,
                                            const_cast<char **>(config_xpm));
        iconw = gtk_pixmap_new(icon, mask);
        pref_button = gtk_toolbar_append_item(GTK_TOOLBAR(toolbar),
                                              "Option",
                                              _("Item option"),
                                              "Private",
                                              iconw,
                                              GTK_SIGNAL_FUNC(Option_item_individual),
                                              NULL);
        //gtk_toolbar_append_space(GTK_TOOLBAR(toolbar));
    }
    // lock selected items
    {
        icon = gdk_pixmap_create_from_xpm_d(toplevel->window,
                                            &mask,
                                            &toplevel->style->white,
                                            const_cast<char **>(lockon_xpm));
        iconw = gtk_pixmap_new(icon, mask);
        lock_button = gtk_toolbar_append_item(GTK_TOOLBAR(toolbar),
                                              "Lock",
                                              _("Lock selected item(s)"),
                                              "Private",
                                              iconw,
                                              GTK_SIGNAL_FUNC(Item_lock_item),
                                              NULL);
        //gtk_toolbar_append_space(GTK_TOOLBAR(toolbar));
    }

    // unlock selected items
    {
        icon = gdk_pixmap_create_from_xpm_d(toplevel->window,
                                            &mask,
                                            &toplevel->style->white,
                                            const_cast<char **>(unlock_xpm));
        iconw = gtk_pixmap_new(icon, mask);
        unlock_button = gtk_toolbar_append_item(GTK_TOOLBAR(toolbar),
                                                "Unlock",
                                                _("Unlock selected item(s)"),
                                                "Private",
                                                iconw,
                                                GTK_SIGNAL_FUNC(Item_unlock_item),
                                                NULL);
        //gtk_toolbar_append_space(GTK_TOOLBAR(toolbar));
    }
    // item move up
    {
        icon = gdk_pixmap_create_from_xpm_d(toplevel->window,
                                            &mask,
                                            &toplevel->style->white,
                                            const_cast<char **>(moveup_xpm));
        iconw = gtk_pixmap_new(icon, mask);
        moveup_button = gtk_toolbar_append_item(GTK_TOOLBAR(toolbar),
                                                "MoveUp",
                                                _("Move up"),
                                                "Private",
                                                iconw,
                                                GTK_SIGNAL_FUNC(Item_move_up),
                                                NULL);
        //gtk_toolbar_append_space(GTK_TOOLBAR(toolbar));
    }
    // item move down
    {
        icon = gdk_pixmap_create_from_xpm_d(toplevel->window,
                                            &mask,
                                            &toplevel->style->white,
                                            const_cast<char **>(movedown_xpm));
        iconw = gtk_pixmap_new(icon, mask);
        movedown_button = gtk_toolbar_append_item(GTK_TOOLBAR(toolbar),
                                                  "MoveDown",
                                                  _("Move down"),
                                                  "Private",
                                                  iconw,
                                                  GTK_SIGNAL_FUNC(Item_move_down),
                                                  NULL);
        //gtk_toolbar_append_space(GTK_TOOLBAR(toolbar));
    }

    // track download
    {
        icon = gdk_pixmap_create_from_xpm_d(toplevel->window,
                                            &mask,
                                            &toplevel->style->white,
                                            const_cast<char **>(track_xpm));
        iconw = gtk_pixmap_new(icon, mask);
        track_toggle_button = NULL;

        track_button = gtk_toolbar_append_element(GTK_TOOLBAR(toolbar),
                                                  GTK_TOOLBAR_CHILD_TOGGLEBUTTON,
                                                  track_toggle_button,
                                                  "TrackDownload",
                                                  _("Track download"),
                                                  "Private",
                                                  iconw,
                                                  GTK_SIGNAL_FUNC(Track_download),
                                                  NULL);
        //gtk_toolbar_append_space(GTK_TOOLBAR(toolbar));
    }

    gtk_toolbar_append_space(GTK_TOOLBAR(toolbar));
    gtk_toolbar_append_space(GTK_TOOLBAR(toolbar));
    // start downloading all items
    {
        icon = gdk_pixmap_create_from_xpm_d(toplevel->window,
                                            &mask,
                                            &toplevel->style->white,
                                            const_cast<char **>(start_all_xpm));
        iconw = gtk_pixmap_new(icon, mask);
        start_all_button = gtk_toolbar_append_item(GTK_TOOLBAR(toolbar),
                                                   "Start all",
                                                   _("Start downloading all items"),
                                                   "Private",
                                                   iconw,
                                                   GTK_SIGNAL_FUNC(Download_start_all),
                                                   NULL);
        //gtk_toolbar_append_space(GTK_TOOLBAR(toolbar));
    }
    // stop downloading all items
    {
        icon = gdk_pixmap_create_from_xpm_d(toplevel->window,
                                            &mask,
                                            &toplevel->style->white,
                                            const_cast<char **>(pause_all_xpm));
        iconw = gtk_pixmap_new(icon, mask);
        stop_all_button = gtk_toolbar_append_item(GTK_TOOLBAR(toolbar),
                                                  "Stop",
                                                  _("Stop downloading all items"),
                                                  "Private",
                                                  iconw,
                                                  GTK_SIGNAL_FUNC(Download_stop_all),
                                                  NULL);
        //gtk_toolbar_append_space(GTK_TOOLBAR(toolbar));
    }
    // default item option
    {
        icon = gdk_pixmap_create_from_xpm_d(toplevel->window,
                                            &mask,
                                            &toplevel->style->white,
                                            const_cast<char **>(config_all_xpm));
        iconw = gtk_pixmap_new(icon, mask);
        pref_default_button = gtk_toolbar_append_item(GTK_TOOLBAR(toolbar),
                                                      "Option default",
                                                      _("Default item option for current list"),
                                                      "Private",
                                                      iconw,
                                                      GTK_SIGNAL_FUNC(Option_item_default_list),
                                                      NULL);
        //gtk_toolbar_append_space(GTK_TOOLBAR(toolbar));
    }
    gtk_toolbar_append_space(GTK_TOOLBAR(toolbar));
    gtk_toolbar_append_space(GTK_TOOLBAR(toolbar));
    // adjust the number of working thread
    {
        GtkObject *adjustment_thread = gtk_adjustment_new(1,// value
                                                          1,// min value
                                                          g_threadLimit,// max value
                                                          1,// inc/dec with 1st btn
                                                          1,// inc/dec with 2nd btn
                                                          0// don't use
                                                         );
        thread_spin = gtk_spin_button_new(GTK_ADJUSTMENT(adjustment_thread),
                                          1.0,
                                          0);
        gtk_widget_show(thread_spin);
        gtk_entry_set_editable(GTK_ENTRY(thread_spin), FALSE);

        g_signal_connect(GTK_OBJECT(thread_spin), "changed",
                         GTK_SIGNAL_FUNC(Thread_spin_changed),
                         (void *)thread_spin);

        tb_thread_spin = gtk_toolbar_append_element(GTK_TOOLBAR(toolbar),
                                                    GTK_TOOLBAR_CHILD_WIDGET,
                                                    thread_spin,
                                                    "Adjust the number of working thread",
                                                    _("Adjust the number of working thread"),
                                                    "Private",
                                                    NULL,
                                                    NULL,
                                                    NULL);

    }
    /*
    // increase the number of working thread
    {
      icon = gdk_pixmap_create_from_xpm_d(toplevel->window,
    				&mask,
    				&toplevel->style->white,
    				stock_up_arrow_xpm);
      iconw = gtk_pixmap_new(icon, mask);
      inc_thread_button = gtk_toolbar_append_item(GTK_TOOLBAR(toolbar),
    					"Increase the number of working thread",
    					_("Increase the number of working thread"),
    					"Private",
    					iconw,
    					GTK_SIGNAL_FUNC(Toolbar_increase_thread),
    					NULL);
      //gtk_toolbar_append_space(GTK_TOOLBAR(toolbar));
    }
    // decrease the number of working thread
    {
      icon = gdk_pixmap_create_from_xpm_d(toplevel->window,
    				&mask,
    				&toplevel->style->white,
    				stock_down_arrow_xpm);
      iconw = gtk_pixmap_new(icon, mask);
      dec_thread_button = gtk_toolbar_append_item(GTK_TOOLBAR(toolbar),
    					"Decrease the number of working thread",
    					_("Decrease the number of working thread"),
    					"Private",
    					iconw,
    					GTK_SIGNAL_FUNC(Toolbar_decrease_thread),
    					NULL);
      //gtk_toolbar_append_space(GTK_TOOLBAR(toolbar));
    }
    */
    pthread_mutex_init(&track_lock, NULL);
    Toolbar_set_sensitive__no_item_selected();
    Toolbar_set_sensitive__list_empty();
    return(toolbar);
}
