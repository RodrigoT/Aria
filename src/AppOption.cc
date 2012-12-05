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

// $Id: AppOption.cc,v 1.41 2002/01/20 14:41:58 tujikawa Exp $

#include "AppOption.h"
#include "ServerTemplateList.h"
#include "HistoryWindow.h"
#include "pixmaps/download.xpm"
#include "pixmaps/ready.xpm"
#include "pixmaps/divide.xpm"
#include "pixmaps/complete.xpm"
#include "pixmaps/error.xpm"
#include "pixmaps/stop.xpm"
#include "pixmaps/lock.xpm"

#define DEFAULT_ICONSET _("<DEFAULT>")
#define DEFAULT_BASKET_PIXMAP _("<DEFAULT>")

extern void *Download_thread_main();
extern void Adjust_speed_scale(int max);
extern GtkWidget *Create_CheckCList(GtkWidget **clist_ptr, char *titles[], int n_titles);
extern gboolean Autosave_list(gpointer data);
extern gboolean Watch_clip_board(gpointer data);
extern gboolean Timer_start(gpointer data);
extern gboolean Timer_stop(gpointer data);
extern void Toolbar_set_sensitive__thread();

extern ListManager *g_listManager;
extern ServerTemplateList g_servTempList;
extern Dialog *g_cDialog;
extern ItemList *g_itemList;
extern HistoryWindow *g_historyWindow;
extern CommandList g_commandList;
extern int g_threadLimit;
extern Basket *basket;

#define ICONSET_PATH "/statusIcon/iconset/"
#define BASKET_PIXMAP_PATH "/basket/pixmap/"

static void
optionWindowOkButton_clicked_cb(GtkWidget *w, AppOption *appOpt)
{
    appOpt->optionWindowOkButton_clicked();
}

void
AppOption::optionWindowOkButton_clicked()
{
    gtk_widget_hide(option_window);
    Process_changes();
    // fix this
    g_historyWindow->setHistoryMax(ret_history_limit());
}

static void
optionWindowCancelButton_clicked_cb(GtkWidget *w, AppOption *appOpt)
{
    appOpt->optionWindowCancelButton_clicked();
}

static void
optionWindow_deleteEvent_cb(AppOption *appOpt)
{
    appOpt->optionWindowCancelButton_clicked();
}

void
AppOption::optionWindowCancelButton_clicked()
{
    gtk_widget_hide(option_window);
}

void AppOption::set_svt_clist(const list<string> &names)
{
    gtk_clist_clear(GTK_CLIST(svt_clist));
    for (vector<ServerTemplate>::iterator svt_itr = g_servTempList.ret_server_template_list().begin(); svt_itr != g_servTempList.ret_server_template_list().end() - 1; ++svt_itr) {
        char *clist_item[2];
        clist_item[0] = strdup(svt_itr->ret_template_name().c_str());
        clist_item[1] = strdup(svt_itr->ret_template_comment().c_str());

        int rowindex = gtk_clist_append(GTK_CLIST(svt_clist), clist_item);

        delete [] clist_item[0];
        for (list<string>::const_iterator name_itr = names.begin(); name_itr != names.end(); ++name_itr) {
            if (svt_itr->ret_template_name() == *name_itr) {
                gtk_clist_select_row(GTK_CLIST(svt_clist), rowindex, 0);
                g_servTempList.set_valid_safely(svt_itr, true);
            }
        }
    }
}

void AppOption::set_com_clist(const list<string> &names)
{
    gtk_clist_clear(GTK_CLIST(com_clist));
    for (vector<Command>::iterator com_itr = g_commandList.ret_command_list().begin(); com_itr != g_commandList.ret_command_list().end() - 1; ++com_itr) {
        char *clist_item[2];
        clist_item[0] = strdup(com_itr->ret_command_name().c_str());
        clist_item[1] = strdup(com_itr->ret_command_comment().c_str());

        int rowindex = gtk_clist_append(GTK_CLIST(com_clist), clist_item);

        delete [] clist_item[0];
        for (list<string>::const_iterator name_itr = names.begin(); name_itr != names.end(); ++name_itr) {
            if (com_itr->ret_command_name() == *name_itr) {
                gtk_clist_select_row(GTK_CLIST(com_clist), rowindex, 0);
                g_commandList.set_valid_safely(com_itr, true);
            }
        }
    }
}

int get_interval(time_t timer_time)
{
    time_t curtime = time(NULL);
    int interval;

    int diff = timer_time - curtime;
    if (diff <= 60 && diff >= 0) {
        interval = 1000;
    } else {
        interval = 60000;
    }
    return interval;
}

AppOption::AppOption(GtkWidget *app_window)
    : history_limit()
{
    pthread_mutex_init(&option_lock, NULL);
    option_window = gtk_dialog_new();
    gtk_widget_set_usize(GTK_WIDGET(option_window), 600, 450);
    gtk_window_set_title(GTK_WINDOW(option_window), _("Application Option"));
    gtk_window_set_modal(GTK_WINDOW(option_window), TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(option_window), GTK_WINDOW(app_window));

    g_signal_connect_swapped(GTK_OBJECT(option_window),
                             "delete_event",
                             GTK_SIGNAL_FUNC(optionWindow_deleteEvent_cb),
                             (GtkObject *)this);

    GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(hbox);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(option_window)->vbox),
                       hbox, TRUE, TRUE, 10);
    {
        notebook = gtk_notebook_new();
        gtk_notebook_set_scrollable(GTK_NOTEBOOK(notebook), TRUE);//added 2001/3/18
        gtk_notebook_popup_enable(GTK_NOTEBOOK(notebook)); //added 2001/3/18

        gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_TOP);
        gtk_box_pack_start(GTK_BOX(hbox),
                           notebook, TRUE, TRUE, 10);
        gtk_widget_show(notebook);

        gtk_notebook_set_show_border(GTK_NOTEBOOK(notebook), TRUE);
        GtkWidget *tab1_label = gtk_label_new(_("Download"));
        gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
                                 Create_DOWNLOAD_page(),
                                 tab1_label);
        GtkWidget *tab2_label = gtk_label_new(_("Confirmation"));
        gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
                                 Create_CONFIRMATION_page(),
                                 tab2_label);
        GtkWidget *tab3_label = gtk_label_new(_("Display"));
        gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
                                 Create_DISPLAY_page(),
                                 tab3_label);
        GtkWidget *tab4_label = gtk_label_new(_("Basket"));
        gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
                                 Create_BASKET_page(),
                                 tab4_label);
        GtkWidget *tab5_label = gtk_label_new(_("Timer"));
        gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
                                 Create_TIMER_page(),
                                 tab5_label);

        GtkWidget *tab6_label = gtk_label_new(_("Command"));
        gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
                                 Create_COMMANDLIST_page(),
                                 tab6_label);

        GtkWidget *tab7_label = gtk_label_new(_("Server"));
        gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
                                 Create_SERVERTEMPLATE_page(),
                                 tab7_label);

        //// action area
        GtkWidget *bbox = gtk_hbutton_box_new();
        gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_END);
        gtk_button_box_set_spacing(GTK_BUTTON_BOX(bbox), 5);
        gtk_box_pack_start(GTK_BOX(GTK_DIALOG(option_window)->action_area),
                           bbox, FALSE, FALSE, 0);
        gtk_widget_show(bbox);

        // OK button
        GtkWidget *optionWindowOkButton = gtk_button_new_with_label(_("OK"));
        GTK_WIDGET_SET_FLAGS(optionWindowOkButton, GTK_CAN_DEFAULT);
        gtk_window_set_default(GTK_WINDOW(option_window), optionWindowOkButton);
        gtk_box_pack_start(GTK_BOX(bbox),
                           optionWindowOkButton, TRUE, TRUE, 0);
        g_signal_connect(GTK_OBJECT(optionWindowOkButton),
                         "clicked",
                         GTK_SIGNAL_FUNC(optionWindowOkButton_clicked_cb),
                         (GtkObject *)this);
        gtk_widget_show(optionWindowOkButton);

        // Cancel
        GtkWidget *optionWindowCancelButton = gtk_button_new_with_label(_("Cancel"));
        //GTK_WIDGET_SET_FLAGS(Cancel_button, GTK_CAN_DEFAULT);
        gtk_box_pack_start(GTK_BOX(bbox),
                           optionWindowCancelButton, TRUE, TRUE, 0);
        g_signal_connect(GTK_OBJECT(optionWindowCancelButton),
                         "clicked",
                         GTK_SIGNAL_FUNC(optionWindowCancelButton_clicked_cb),
                         (GtkObject *)this);
        gtk_widget_show(optionWindowCancelButton);
    }
    gtk_widget_realize(option_window);
    // initialize input tag
    autosave_tag = -1;
    timer_start_tag = -1;
    timer_stop_tag = -1;
}

static void
updateCommandListButton_clicked_cb(GtkWidget *button, AppOption *appOpt)
{
    appOpt->updateCommandListButton_clicked();
}

void
AppOption::updateCommandListButton_clicked()
{
    if (g_commandList.Read_from_file(g_itemList->ret_file_command_list())) {
        g_cDialog->setup(_("Update Command list"),
                         _("Update successful"));
    } else {
        g_cDialog->setup(_("Update Command list"),
                         _("Update failed"));
    }
    g_cDialog->set_no_button_visible(false);
    g_cDialog->set_cancel_button_visible(false);
    g_cDialog->set_transient(GTK_WINDOW(option_window));
    g_cDialog->show();

    list<string> names = ret_com_name_list();
    set_com_clist(names);
}

GtkWidget *AppOption::Create_COMMANDLIST_page()
{
    GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
    gtk_widget_show(hbox);
    GtkWidget *vbox = gtk_vbox_new(FALSE, 5);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
    gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 5);
    gtk_widget_show(vbox);


    // toggle command list
    use_commandlist_toggle = gtk_check_button_new_with_label(_("Use Command list"));
    gtk_widget_set_name(use_commandlist_toggle, "check_button");
    gtk_box_pack_start(GTK_BOX(vbox), use_commandlist_toggle, FALSE, FALSE, 0);
    gtk_widget_show(use_commandlist_toggle);

    // コマンドリスト更新ボタン
    GtkWidget *updateCommandListButton = gtk_button_new_with_label(_("Update Command list"));
    gtk_widget_set_name(updateCommandListButton, "button");
    gtk_widget_show(updateCommandListButton);
    gtk_box_pack_start(GTK_BOX(vbox), updateCommandListButton, FALSE, FALSE, 10);
    g_signal_connect(GTK_OBJECT(updateCommandListButton), "clicked",
                     GTK_SIGNAL_FUNC(updateCommandListButton_clicked_cb),
                     (GtkObject *)this);

    // コマンドリストのCList
    GtkWidget *clist_label = gtk_label_new(_("Choose commands to use"));
    gtk_widget_set_name(clist_label, "label");
    gtk_box_pack_start(GTK_BOX(vbox), clist_label, FALSE, FALSE, 10);
    gtk_widget_show(clist_label);
    int n_titles = 2;
    char *titles[n_titles];
    titles[0] = _("Command");
    titles[1] = _("Comment");

    // スクロールウインドウにCListをバッキング
    GtkWidget *scrolled_window = Create_CheckCList(&com_clist, titles, n_titles);
    gtk_clist_set_column_auto_resize(GTK_CLIST(com_clist), 0, TRUE);
    gtk_clist_set_column_auto_resize(GTK_CLIST(com_clist), 1, TRUE);

    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, FALSE, FALSE, 0);

    gtk_widget_show(scrolled_window);
    return hbox;
}

void
AppOption::createStatusIconDefault()
{
//  GtkStyle *style = gtk_widget_get_style(option_window);

    for (int i = 0; i < ICON_TOTAL; ++i) {
        if (statusIcon[i] != NULL) gdk_pixmap_unref(statusIcon[i]);
        if (statusIconMask[i] != NULL) gdk_bitmap_unref(statusIconMask[i]);
    }
    statusIcon[ICON_DOWNLOAD] = gdk_pixbuf_new_from_xpm_data((const char **)download_xpm);

    statusIcon[ICON_READY] = gdk_pixbuf_new_from_xpm_data((const char **)ready_xpm);

    statusIcon[ICON_COMPLETE] = gdk_pixbuf_new_from_xpm_data((const char **)complete_xpm);

    statusIcon[ICON_STOP] = gdk_pixbuf_new_from_xpm_data((const char **)stop_xpm);

    statusIcon[ICON_DIVIDE] = gdk_pixbuf_new_from_xpm_data((const char **)divide_xpm);

    statusIcon[ICON_ERROR] = gdk_pixbuf_new_from_xpm_data((const char **)error_xpm);

    statusIcon[ICON_LOCK] = gdk_pixbuf_new_from_xpm_data((const char **)lock_xpm);
}

static void
updateServerTemplateButton_clicked_cb(GtkWidget *button, AppOption *appOpt)
{
    appOpt->updateServerTemplateButton_clicked();
}

void
AppOption::updateServerTemplateButton_clicked()
{
    if (g_servTempList.Read_config_file(g_itemList->ret_file_server_settings())) {
        g_cDialog->setup(_("Update Server Template"),
                         _("Update successful"));
    } else {
        g_cDialog->setup(_("Update Server Template"),
                         _("Update failed"));
    }
    g_cDialog->set_no_button_visible(false);
    g_cDialog->set_cancel_button_visible(false);
    g_cDialog->set_transient(GTK_WINDOW(option_window));
    g_cDialog->show();

    list<string> names = ret_svt_name_list();
    set_svt_clist(names);
}

GtkWidget *AppOption::Create_SERVERTEMPLATE_page()
{
    GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
    gtk_widget_show(hbox);
    GtkWidget *vbox = gtk_vbox_new(FALSE, 5);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
    gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 5);
    gtk_widget_show(vbox);

    // toggle enable/disable Server Template feature
    use_servertemplate_toggle = gtk_check_button_new_with_label(_("Use Server Template"));
    gtk_widget_set_name(use_servertemplate_toggle, "check_button");
    gtk_box_pack_start(GTK_BOX(vbox), use_servertemplate_toggle, FALSE, FALSE, 0);
    gtk_widget_show(use_servertemplate_toggle);

    // server template update button
    GtkWidget *updateServerTemplateButton = gtk_button_new_with_label(_("Update Server Template"));
    gtk_widget_set_name(updateServerTemplateButton, "button");
    gtk_widget_show(updateServerTemplateButton);
    gtk_box_pack_start(GTK_BOX(vbox), updateServerTemplateButton, FALSE, FALSE, 10);
    g_signal_connect(GTK_OBJECT(updateServerTemplateButton), "clicked",
                     GTK_SIGNAL_FUNC(updateServerTemplateButton_clicked_cb),
                     (GtkObject *)this);

    // CList for Server Template
    GtkWidget *clist_label = gtk_label_new(_("Choose templates to use"));
    gtk_box_pack_start(GTK_BOX(vbox), clist_label, FALSE, FALSE, 10);
    gtk_widget_show(clist_label);
    int n_titles = 2;
    char *titles[n_titles];
    titles[0] = _("Template");
    titles[1] = _("Comment");
    GtkWidget *scrolled_window = Create_CheckCList(&svt_clist, titles, n_titles);
    gtk_clist_set_column_auto_resize(GTK_CLIST(svt_clist), 0, TRUE);
    gtk_clist_set_column_auto_resize(GTK_CLIST(svt_clist), 1, TRUE);

    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, FALSE, FALSE, 0);

    gtk_widget_show(scrolled_window);

    return hbox;
}

void
AppOption::setStatusIconPreview()
{
    for (int index = 0; index < ICON_TOTAL; ++index) {
        /*gtk_clist_set_pixmap(GTK_CLIST(statusIconPreviewList),
        		 0,
        		 index,
        		 statusIcon[index],
        		 statusIconMask[index]);*///TODO:
    }
}

/*
static void CLIST_SET_PIXMAP(GtkWidget *clist, int rowindex, int column, int pixmap_n) {
  gtk_clist_set_pixmap(GTK_CLIST(clist), rowindex, column, stat_icon_opt[pixmap_n], stat_iconm_opt[pixmap_n]);
}
*/

extern GtkWidget *g_toplevel; // fix this

bool
AppOption::createStatusIcon(const string &dirname)
{
    //if(dirname.empty()) return false;
    bool retval = false;
    //if(dirname == DEFAULT_ICONSET) {
    if (dirname == "") {
        createStatusIconDefault();
        retval = true;
    } else {
        const char *home_dir = g_get_home_dir();
        string baseDir = home_dir;
        baseDir += "/.aria"ICONSET_PATH + dirname + '/';
        if (Create_pixmap_from_file(&statusIcon[ICON_COMPLETE], &statusIconMask[ICON_COMPLETE], baseDir + "complete.xpm", option_window) &&
                Create_pixmap_from_file(&statusIcon[ICON_DOWNLOAD], &statusIconMask[ICON_DOWNLOAD], baseDir + "download.xpm", option_window) &&
                Create_pixmap_from_file(&statusIcon[ICON_LOCK], &statusIconMask[ICON_LOCK], baseDir + "lock.xpm", option_window) &&
                Create_pixmap_from_file(&statusIcon[ICON_STOP], &statusIconMask[ICON_STOP], baseDir + "stop.xpm", option_window) &&
                Create_pixmap_from_file(&statusIcon[ICON_DIVIDE], &statusIconMask[ICON_DIVIDE], baseDir + "divide.xpm", option_window) &&
                Create_pixmap_from_file(&statusIcon[ICON_ERROR], &statusIconMask[ICON_ERROR], baseDir + "error.xpm", option_window) &&
                Create_pixmap_from_file(&statusIcon[ICON_READY], &statusIconMask[ICON_READY], baseDir + "ready.xpm", option_window)) {
            retval = true;
        } else {
            for (int index = 0; index < ICON_TOTAL; ++index) {
                if (statusIcon[index] != NULL) {
                    gdk_pixmap_unref(statusIcon[index]);
                    statusIcon[index] = NULL;
                }
                if (statusIconMask[index] != NULL) {
                    gdk_bitmap_unref(statusIconMask[index]);
                    statusIconMask[index] = NULL;
                }
            }
        }
    }
    return retval;
}

static void
statusIconApplyButton_clicked_cb(GtkWidget *button,
                                 AppOption *appOpt)
{
    appOpt->statusIconApplyButton_clicked();
}

void
AppOption::statusIconApplyButton_clicked()
{
    //if(GTK_CLIST(clist)->selection == NULL) return TRUE;
    const char *dirname;
    int row = GPOINTER_TO_INT(GTK_CLIST(statusIconDirList)->selection->data);
    if (row == 0) {
        dirname = "";
    } else {
        gtk_clist_get_text(GTK_CLIST(statusIconDirList), row, 0, const_cast<char **>(&dirname));
    }
    statusIconDir = dirname;
    g_listManager->setStatusIcon(statusIcon, statusIconMask);
}

void
AppOption::showStatusIconError()
{
    showBasketPixmapError(); // fix this
}

static void
statusIconDirList_selectRow_cb(GtkWidget *clist,
                               int row,
                               int column,
                               GdkEventButton *event,
                               AppOption *appOpt)
{
    appOpt->statusIconDirList_selectRow(row, column, event);
}

void
AppOption::statusIconDirList_selectRow(int row,
                                       int column,
                                       GdkEventButton *event)
{
    const char *dirname;
    if (row == 0) {
        dirname = "";
    } else {
        gtk_clist_get_text(GTK_CLIST(statusIconDirList), row, 0, const_cast<char **>(&dirname));
    }

    switch (event->button) {
        case 1:
        case 3: {
            if (createStatusIcon(dirname)) {
                setStatusIconPreview();
                setStatusIconApplyButtonEnabled(true);
            } else {
                showStatusIconError();
                setStatusIconApplyButtonEnabled(false);
            }
            break;
        }
    }
}

static void
statusIconDirList_unselectRow_cb(GtkWidget *clist,
                                 int row,
                                 int column,
                                 GdkEventButton *event,
                                 AppOption *appOpt)
{
    appOpt->statusIconDirList_unselectRow(row, column, event);
}

void
AppOption::statusIconDirList_unselectRow(int row,
                                         int column,
                                         GdkEventButton *event)
{
    setStatusIconApplyButtonEnabled(false);
}

void
AppOption::setStatusIconApplyButtonEnabled(bool toggle)
{
    gtk_widget_set_sensitive(statusIconApplyButton, toggle);
}

GtkWidget *AppOption::Create_DISPLAY_page()
{
    GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
    gtk_widget_show(hbox);
    GtkWidget *vbox = gtk_vbox_new(FALSE, 5);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
    gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 5);
    gtk_widget_show(vbox);

    // enable/disable human readable
    {
        use_size_human_readable_toggle = gtk_check_button_new_with_label(_("Display sizes in human readable format (e.g. 1.2K 234.1M 2.2G)"));
        gtk_widget_set_name(use_size_human_readable_toggle, "check_button");
        gtk_box_pack_start(GTK_BOX(vbox), use_size_human_readable_toggle, FALSE, FALSE, 0);
        gtk_widget_show(use_size_human_readable_toggle);
    }

    // icon set
    {
        GtkWidget *frame = gtk_frame_new(_("Status Icon"));
        gtk_widget_show(frame);
        gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 10);

        GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
        gtk_container_add(GTK_CONTAINER(frame), hbox);
        gtk_widget_show(hbox);

        GtkWidget *vbox = gtk_vbox_new(FALSE, 5);
        gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 10);
        gtk_widget_show(vbox);

        // clist for preview
        {
            GtkWidget *scrolled_window = gtk_scrolled_window_new (NULL, NULL);
            gtk_widget_show(scrolled_window);
            gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, FALSE, FALSE, 0);
            gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                           GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
            // set minimum size of scrolled window
            gtk_widget_set_usize(scrolled_window, 210, 35);
            // CList を作成する。この例では n_titles 列を使う
            statusIconPreviewList = gtk_clist_new_with_titles(7, NULL);
            gtk_clist_set_row_height(GTK_CLIST(statusIconPreviewList), 25);
            gtk_clist_column_titles_passive(GTK_CLIST(statusIconPreviewList));

            for (int i = 0; i < ICON_TOTAL; ++i) {
                gtk_clist_set_column_auto_resize(GTK_CLIST(statusIconPreviewList), i, true);
            }
            // add shadow effect
            gtk_clist_set_shadow_type(GTK_CLIST(statusIconPreviewList), GTK_SHADOW_ETCHED_OUT);
            // CList ウィジェットを垂直ボックスに加え、それを表示する
            gtk_container_add(GTK_CONTAINER(scrolled_window), statusIconPreviewList);
            gtk_widget_show(statusIconPreviewList);
            // add 1 row
            char *titles[ICON_TOTAL];
            for (int i = 0; i < ICON_TOTAL; ++i) {
                titles[i] = NULL;
            }
            gtk_clist_append(GTK_CLIST(statusIconPreviewList), titles);
            gtk_clist_set_selectable(GTK_CLIST(statusIconPreviewList), 0, false);
        }
        // clist for browse
        {
            GtkWidget *scrolled_window = gtk_scrolled_window_new (NULL, NULL);
            gtk_widget_show(scrolled_window);
            gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, FALSE, FALSE, 0);
            gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                           GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
            // set minimum size of scrolled window
            gtk_widget_set_usize(scrolled_window, 300, 140);

            // dir list
            char *titles[1];
            titles[0] = _("Icon set");
            statusIconDirList = gtk_clist_new_with_titles(1, titles);
            gtk_clist_column_titles_passive(GTK_CLIST(statusIconDirList));
            // set selection mode SINGLE
            gtk_clist_set_selection_mode(GTK_CLIST(statusIconDirList), GTK_SELECTION_SINGLE);

            // selection callbacks
            g_signal_connect(GTK_OBJECT(statusIconDirList), "select-row",
                             GTK_SIGNAL_FUNC(statusIconDirList_selectRow_cb),
                             (GtkObject *)this);

            g_signal_connect(GTK_OBJECT(statusIconDirList), "unselect-row",
                             GTK_SIGNAL_FUNC(statusIconDirList_unselectRow_cb),
                             (GtkObject *)this);

            // add shadow
            gtk_clist_set_shadow_type(GTK_CLIST(statusIconDirList), GTK_SHADOW_ETCHED_OUT);

            // CList ウィジェットを垂直ボックスに加え、それを表示する
            gtk_container_add(GTK_CONTAINER(scrolled_window), statusIconDirList);
            gtk_widget_show(statusIconDirList);
        }
        {
            GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
            gtk_widget_show(hbox);
            gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
            statusIconApplyButton = gtk_button_new_with_label(_("Apply"));
            gtk_widget_show(statusIconApplyButton);
            gtk_box_pack_start(GTK_BOX(hbox), statusIconApplyButton, FALSE, FALSE, 10);

            g_signal_connect(GTK_OBJECT(statusIconApplyButton), "clicked",
                             GTK_SIGNAL_FUNC(statusIconApplyButton_clicked_cb),
                             (GtkObject *)this);
        }
    }
    return hbox;
}

static void
basketPixmapApplyButton_clicked_cb(GtkWidget *button,
                                   AppOption *appopt)
{
    appopt->basketPixmapApplyButton_clicked();
}

void
AppOption::basketPixmapApplyButton_clicked()
{
    char *filename;
    int row = GPOINTER_TO_INT((GList *)(GTK_CLIST(basketPixmapFileList)->selection)->data);

    if (row == 0) {
        filename = "";
    } else {
        gtk_clist_get_text(GTK_CLIST(basketPixmapFileList), row, 0, &filename);
    }
    basketPixmapFile = filename;
    basket->setPixmap(basketPixmapTemp, basketBitmapTemp);
}

static void
basketPixmapFileList_selectRow_cb(GtkWidget *clist,
                                  int row,
                                  int column,
                                  GdkEventButton *event,
                                  AppOption *appopt)
{
    appopt->basketPixmapFileList_selectRow(row, column, event);
}

void
AppOption::basketPixmapFileList_selectRow(int row,
                                          int column,
                                          GdkEventButton *event)
{
    char *filename;
    if (row == 0) {
        filename = "";
    } else {
        gtk_clist_get_text(GTK_CLIST(basketPixmapFileList), row, 0, &filename);
    }

    switch (event->button) {
        case 1:
        case 3:
            if (createBasketPixmapFromFile(filename)) {
                setBasketPixmapPreview();
                setBasketPixmapApplyButtonEnabled(true);
            } else {
                showBasketPixmapError();
                setBasketPixmapApplyButtonEnabled(false);
            }
            break;
        default:
            break;
    }
}

static void
basketPixmapFileList_unselectRow_cb(GtkWidget *clist,
                                    int row,
                                    int column,
                                    GdkEventButton *event,
                                    AppOption *appopt)
{
    appopt->basketPixmapFileList_unselectRow(row, column, event);
}

void
AppOption::basketPixmapFileList_unselectRow(int row,
                                            int column,
                                            GdkEventButton *event)
{
    setBasketPixmapApplyButtonEnabled(false);
}

void
AppOption::setBasketPixmapApplyButtonEnabled(bool toggle)
{
    gtk_widget_set_sensitive(basketPixmapApplyButton, toggle);
}

string
AppOption::getBasketPixmapFile()
{
    pthread_mutex_lock(&option_lock);
    string retval = basketPixmapFile;
    pthread_mutex_unlock(&option_lock);
    return retval;
}

void
AppOption::showBasketPixmapError()
{
    g_cDialog->setup(_("error"),
                     _("cannot read pixmap file"));
    g_cDialog->set_no_button_visible(false);
    g_cDialog->set_cancel_button_visible(false);
    g_cDialog->set_transient(GTK_WINDOW(option_window));
    g_cDialog->show();
}

bool
AppOption::createBasketPixmapFromFile(const string &filename)
{
    //if(filename.empty()) return false;
    //if(filename != DEFAULT_BASKET_PIXMAP) {
    if (filename == "") {
        // default pixmap is NULL
        if (basketPixmapTemp != NULL) {
            gdk_pixmap_unref(basketPixmapTemp);
            basketPixmapTemp = NULL;
        }
        if (basketBitmapTemp != NULL) {
            gdk_bitmap_unref(basketBitmapTemp);
            basketBitmapTemp = NULL;
        }
        return true;
    } else {
        const char *homeDir = g_get_home_dir();
        string filepath = homeDir;
        filepath += "/.aria"BASKET_PIXMAP_PATH + filename;
        if (Create_pixmap_from_file(&basketPixmapTemp, &basketBitmapTemp,
                                    filepath,
                                    option_window)) {
            return true;
        } else {
            if (basketPixmapTemp != NULL) {
                gdk_pixmap_unref(basketPixmapTemp);
                basketPixmapTemp = NULL;
            }
            if (basketBitmapTemp != NULL) {
                gdk_bitmap_unref(basketBitmapTemp);
                basketBitmapTemp = NULL;
            }
            return false;
        }
    }
}

void
AppOption::setBasketPixmapPreview()
{
    if (basketPixmapPreview == NULL) {
        if (basketPixmapTemp != NULL &&
                basketBitmapTemp != NULL) {
            basketPixmapPreview = gtk_image_new_from_pixbuf(basketPixmapTemp);
            gtk_box_pack_start(GTK_BOX(basketPixmapPreviewVBox), basketPixmapPreview,
                               FALSE, FALSE, 0);
            gtk_widget_show(basketPixmapPreview);
        }
    } else {
        if (basketPixmapTemp != NULL &&
                basketBitmapTemp != NULL) {
            gtk_widget_hide(basketPixmapPreview);
            gtk_image_set_from_pixbuf(GTK_IMAGE(basketPixmapPreview),
                                      basketPixmapTemp);
            gtk_widget_show(basketPixmapPreview);
        } else {
            gtk_widget_hide(basketPixmapPreview);
            gtk_widget_destroy(basketPixmapPreview);
            basketPixmapPreview = NULL;
        }
    }
}

GtkWidget *AppOption::Create_BASKET_page()
{
    GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
    gtk_widget_show(hbox);

    GtkWidget *vbox = gtk_vbox_new(FALSE, 5);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
    gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 5);
    gtk_widget_show(vbox);

    // basket options
    {
        directPastingFromBasketToggle = gtk_check_button_new_with_label(_("Bypass paste window"));
        gtk_widget_show(directPastingFromBasketToggle);
        gtk_box_pack_start(GTK_BOX(vbox), directPastingFromBasketToggle, FALSE, FALSE, 0);
    }
    // pixmap on basket
    {
        GtkWidget *frame = gtk_frame_new(_("Pixmap on DND Basket"));
        gtk_widget_show(frame);
        gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 10);

        GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
        gtk_container_add(GTK_CONTAINER(frame), hbox);
        gtk_widget_show(hbox);

        GtkWidget *vbox = gtk_vbox_new(FALSE, 5);
        gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 10);
        gtk_widget_show(vbox);

        // pixmap for preview
        {
            GtkWidget *frame = gtk_frame_new(_("Preview"));
            gtk_widget_set_usize(frame, 200, 100);
            gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);
            gtk_widget_show(frame);

            basketPixmapPreviewVBox = gtk_vbox_new(FALSE, 5);
            gtk_widget_show(basketPixmapPreviewVBox);
            gtk_container_add(GTK_CONTAINER(frame), basketPixmapPreviewVBox);

            // initialize pixmaps
            basketPixmapPreview = NULL;
            basketPixmapTemp = NULL;
            basketBitmapTemp = NULL;
        }
        // clist for browse
        {
            GtkWidget *scrolled_window = gtk_scrolled_window_new (NULL, NULL);
            gtk_widget_show(scrolled_window);
            gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, FALSE, FALSE, 0);
            gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                           GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
            // set minimum size of scrolled window
            gtk_widget_set_usize(scrolled_window, 300, 140);

            // CList を作成する。この例では n_titles 列を使う
            char *titles[1];
            titles[0] = _("Pixmap");
            basketPixmapFileList = gtk_clist_new_with_titles(1, titles);
            gtk_clist_column_titles_passive(GTK_CLIST(basketPixmapFileList));
            // set selection mode SINGLE
            gtk_clist_set_selection_mode(GTK_CLIST(basketPixmapFileList), GTK_SELECTION_SINGLE);

            // selection callbacks
            g_signal_connect(GTK_OBJECT(basketPixmapFileList), "select-row",
                             GTK_SIGNAL_FUNC(basketPixmapFileList_selectRow_cb),
                             (void *)this);
            g_signal_connect(GTK_OBJECT(basketPixmapFileList), "unselect-row",
                             GTK_SIGNAL_FUNC(basketPixmapFileList_unselectRow_cb),
                             (void *)this);

            // attach the shadow to the clist
            gtk_clist_set_shadow_type(GTK_CLIST(basketPixmapFileList),
                                      GTK_SHADOW_ETCHED_OUT);

            // attach the clist to the scrolled window
            gtk_container_add(GTK_CONTAINER(scrolled_window), basketPixmapFileList);
            gtk_widget_show(basketPixmapFileList);
        }
        {
            // apply button
            GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
            gtk_widget_show(hbox);
            gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
            basketPixmapApplyButton = gtk_button_new_with_label(_("Apply"));
            gtk_widget_set_sensitive(basketPixmapApplyButton, FALSE);
            gtk_widget_show(basketPixmapApplyButton);
            gtk_box_pack_start(GTK_BOX(hbox), basketPixmapApplyButton, FALSE, FALSE, 10);

            // add callbacks
            g_signal_connect(GTK_OBJECT(basketPixmapApplyButton), "clicked",
                             GTK_SIGNAL_FUNC(basketPixmapApplyButton_clicked_cb),
                             (void *)this);
        }
    }
    return hbox;
}

GtkWidget *AppOption::Create_CONFIRMATION_page()
{
    GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
    gtk_widget_show(hbox);
    GtkWidget *vbox = gtk_vbox_new(FALSE, 5);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
    gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 5);
    gtk_widget_show(vbox);

    // アイテムを削除するときに確認するかどうか
    confirm_clear_toggle = gtk_check_button_new_with_label(_("Confirm clear"));
    gtk_widget_set_name(confirm_clear_toggle, "check_button");
    gtk_box_pack_start(GTK_BOX(vbox), confirm_clear_toggle, FALSE, FALSE, 0);
    gtk_widget_show(confirm_clear_toggle);

    // confirm delete list
    confirm_delete_list_toggle = gtk_check_button_new_with_label(_("Confirm delete list"));
    gtk_box_pack_start(GTK_BOX(vbox), confirm_delete_list_toggle, FALSE, FALSE, 0);
    gtk_widget_show(confirm_delete_list_toggle);

    // プログラム終了時に確認するかどうか
    confirm_exit_toggle = gtk_check_button_new_with_label(_("Confirm exit"));
    gtk_widget_set_name(confirm_exit_toggle, "check_button");
    gtk_box_pack_start(GTK_BOX(vbox), confirm_exit_toggle, FALSE, FALSE, 0);
    gtk_widget_show(confirm_exit_toggle);

    // ログクリア時に確認するかどうか
    confirm_clearlog_toggle = gtk_check_button_new_with_label(_("Confirm clear system log"));
    gtk_widget_set_name(confirm_clearlog_toggle, "check_button");
    gtk_box_pack_start(GTK_BOX(vbox), confirm_clearlog_toggle, FALSE, FALSE, 0);
    gtk_widget_show(confirm_clearlog_toggle);

    return hbox;
}

GtkWidget *AppOption::Create_TIMER_page()
{
    GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
    gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);
    gtk_widget_show(hbox);

    GtkWidget *vbox = gtk_vbox_new(FALSE, 5);
    gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);
    gtk_widget_show(vbox);
    {
        {
            GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
            gtk_widget_show(hbox);
            gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

            startTimerEnabledToggle = gtk_check_button_new_with_label(_("Use start timer"));
            gtk_widget_show(startTimerEnabledToggle);
            gtk_box_pack_start(GTK_BOX(hbox), startTimerEnabledToggle, FALSE, FALSE, 10);
            {
                GtkWidget *label = gtk_label_new(_("Start (hour : min) "));
                gtk_widget_show(label);
                gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

                GtkObject *adjustment = gtk_adjustment_new(0,// initial value
                                                           0,// minimal value
                                                           23,// maximal value
                                                           1,
                                                           23,
                                                           0);
                timer_hour_start_spin = gtk_spin_button_new(GTK_ADJUSTMENT(adjustment),
                                                            1.0,
                                                            0);
                gtk_widget_show(timer_hour_start_spin);
                gtk_box_pack_start(GTK_BOX(hbox), timer_hour_start_spin, FALSE, FALSE, 0);
            }
            GtkWidget *label = gtk_label_new(":");
            gtk_widget_show(label);
            gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
            {
                GtkObject *adjustment = gtk_adjustment_new(0,// initial value
                                                           0,// minimal value
                                                           59,// maximal value
                                                           1,
                                                           10,
                                                           0);
                timer_min_start_spin = gtk_spin_button_new(GTK_ADJUSTMENT(adjustment),
                                                           1.0,
                                                           0);
                gtk_widget_show(timer_min_start_spin);
                gtk_box_pack_start(GTK_BOX(hbox), timer_min_start_spin, FALSE, FALSE, 0);
            }
        }

        {
            GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
            gtk_widget_show(hbox);
            gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

            stopTimerEnabledToggle = gtk_check_button_new_with_label(_("Use stop timer"));
            gtk_widget_show(stopTimerEnabledToggle);
            gtk_box_pack_start(GTK_BOX(hbox), stopTimerEnabledToggle, FALSE, FALSE, 10);
            {
                GtkWidget *label = gtk_label_new(_("Stop (hour : min) "));
                gtk_widget_show(label);
                gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

                GtkObject *adjustment = gtk_adjustment_new(0,// initial value
                                                           0,// minimal value
                                                           23,// maximal value
                                                           1,
                                                           23,
                                                           0);
                timer_hour_stop_spin = gtk_spin_button_new(GTK_ADJUSTMENT(adjustment),
                                                           1.0,
                                                           0);
                gtk_widget_show(timer_hour_stop_spin);
                gtk_box_pack_start(GTK_BOX(hbox), timer_hour_stop_spin, FALSE, FALSE, 0);
            }
            GtkWidget *label = gtk_label_new(":");
            gtk_widget_show(label);
            gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
            {
                GtkObject *adjustment = gtk_adjustment_new(0,// initial value
                                                           0,// minimal value
                                                           59,// maximal value
                                                           1,
                                                           10,
                                                           0);
                timer_min_stop_spin = gtk_spin_button_new(GTK_ADJUSTMENT(adjustment),
                                                          1.0,
                                                          0);
                gtk_widget_show(timer_min_stop_spin);
                gtk_box_pack_start(GTK_BOX(hbox), timer_min_stop_spin, FALSE, FALSE, 0);
            }
        }
        {
            timer_start_all_list_toggle = gtk_check_button_new_with_label(_("Start all lists"));
            gtk_widget_show(timer_start_all_list_toggle);
            gtk_box_pack_start(GTK_BOX(vbox), timer_start_all_list_toggle, FALSE, FALSE, 0);
        }
        {
            noStopDownloadOnTimerToggle = gtk_check_button_new_with_label(_("Do not stop downloads when timer expires"));
            gtk_widget_show(noStopDownloadOnTimerToggle);
            gtk_box_pack_start(GTK_BOX(vbox), noStopDownloadOnTimerToggle, FALSE, FALSE, 0);
        }
    }

    return hbox;
}

static void
autoStartToggle_toggled_cb(GtkWidget *toggle,
                           AppOption *appOpt)
{
    appOpt->autoStartToggle_toggled();
}

void
AppOption::autoStartToggle_toggled()
{
    if (GTK_TOGGLE_BUTTON(autoStartToggle)->active) {
        gtk_widget_set_sensitive(autostartModCurrentListOnly_toggle, TRUE);
    } else {
        gtk_widget_set_sensitive(autostartModCurrentListOnly_toggle, FALSE);
    }
}

GtkWidget *AppOption::Create_DOWNLOAD_page()
{
    GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
    gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);
    gtk_widget_show(hbox);

    GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);
    gtk_widget_show(vbox);
    // timed out
    {
        GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
        gtk_widget_show(hbox);
        gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

        GtkWidget *label_thread = gtk_label_new(_("The number of simultaneous downloads for new list "));
        gtk_widget_set_name(label_thread, "label");
        gtk_widget_show(label_thread);
        gtk_box_pack_start(GTK_BOX(hbox), label_thread, FALSE, FALSE, 0);

        GtkObject *adjustment_thread = gtk_adjustment_new(1,// value
                                                          1,// min value
                                                          g_threadLimit,// max value
                                                          1,// inc/dec with 1st btn
                                                          1,// inc/dec with 2nd btn
                                                          0// don't use
                                                         );
        spin_thread = gtk_spin_button_new(GTK_ADJUSTMENT(adjustment_thread),
                                          1.0,
                                          0);
        gtk_widget_show(spin_thread);
        gtk_widget_set_name(spin_thread, "spin");
        gtk_box_pack_start(GTK_BOX(hbox), spin_thread, FALSE, FALSE, 0);
    }
    {
        // toggle automatic start
        autoStartToggle = gtk_check_button_new_with_label(_("Start downloads automatically"));
        gtk_widget_set_name(autoStartToggle, "check_button");
        gtk_widget_show(autoStartToggle);
        gtk_box_pack_start(GTK_BOX(vbox), autoStartToggle, FALSE, FALSE, 0);
    }
    {
        GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
        gtk_widget_show(hbox);

        // toggle automatic start modifier
        autostartModCurrentListOnly_toggle = gtk_check_button_new_with_label(_("Only current list"));
        gtk_widget_show(autostartModCurrentListOnly_toggle);
        gtk_box_pack_start(GTK_BOX(hbox), autostartModCurrentListOnly_toggle, FALSE, FALSE, 10);
        g_signal_connect(GTK_OBJECT(autoStartToggle),
                         "toggled",
                         GTK_SIGNAL_FUNC(autoStartToggle_toggled_cb),
                         (GtkObject *)this);
    }
    {
        // force download now (stopping other downloads if necessary)
        useForceDownloadNowToggle = gtk_check_button_new_with_label(_("Force download now (stopping other downloads if necessary)"));
        gtk_widget_show(useForceDownloadNowToggle);
        gtk_box_pack_start(GTK_BOX(vbox), useForceDownloadNowToggle, FALSE, FALSE, 0);
    }
    {
        // toggle ignore error item in "Start All" menu
        ignore_error_item_toggle = gtk_check_button_new_with_label(_("Ignore error'ed items in \"Start All\" menu"));
        gtk_widget_show(ignore_error_item_toggle);
        gtk_box_pack_start(GTK_BOX(vbox), ignore_error_item_toggle, FALSE, FALSE, 0);
    }
    {
        // ignore list in find hyperlink
        ignore_extension_toggle = gtk_check_button_new_with_label(_("Ignore these extensions when finding hyperlink"));
        gtk_widget_set_name(ignore_extension_toggle, "check_button");
        gtk_box_pack_start(GTK_BOX(vbox), ignore_extension_toggle, FALSE, FALSE, 0);
        gtk_widget_show(ignore_extension_toggle);
        // entry
        ignore_extension_entry = gtk_entry_new();
        gtk_widget_set_usize(ignore_extension_entry, 300, -1);
        gtk_widget_set_name(ignore_extension_entry, "entry");
        gtk_box_pack_start(GTK_BOX(vbox), ignore_extension_entry, FALSE, FALSE, 0);
        gtk_widget_show(ignore_extension_entry);
    }
    {
        GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
        gtk_widget_show(hbox);
        gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

        // toggle autosave
        autosave_toggle = gtk_check_button_new_with_label(_("Autosave (list, history and options) every "));
        gtk_widget_show(autosave_toggle);
        gtk_box_pack_start(GTK_BOX(hbox), autosave_toggle, FALSE, FALSE, 0);

        GtkObject *adjustment = gtk_adjustment_new(10,
                                                   1,
                                                   MAXAUTOSAVEINTERVAL,
                                                   1,
                                                   30,
                                                   0);
        autosave_spin = gtk_spin_button_new(GTK_ADJUSTMENT(adjustment),
                                            1.0,
                                            0);
        gtk_widget_show(autosave_spin);
        gtk_box_pack_start(GTK_BOX(hbox), autosave_spin, FALSE, FALSE, 0);

        GtkWidget *label = gtk_label_new(_("minute(s)"));
        gtk_widget_show(label);
        gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
    }
    {
        // limit history list
        GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
        gtk_widget_show(hbox);
        gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

        GtkWidget *label = gtk_label_new(_("Max entry of download history"));
        gtk_widget_show(label);
        gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

        GtkObject *adjustment = gtk_adjustment_new(DEFAULTHISTORY,
                                                   0,
                                                   MAXHISTORY,
                                                   1,
                                                   100,
                                                   0);
        history_limit_spin = gtk_spin_button_new(GTK_ADJUSTMENT(adjustment),
                                                 1.0,
                                                 0);
        gtk_widget_set_usize(history_limit_spin, 60, -1);
        gtk_widget_show(history_limit_spin);
        gtk_box_pack_start(GTK_BOX(hbox), history_limit_spin, FALSE, FALSE, 0);
    }
    {
        // max value of speed limiter
        GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
        gtk_widget_show(hbox);
        gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

        GtkWidget *label = gtk_label_new(_("Max value of speed limiter"));
        gtk_widget_show(label);
        gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

        GtkObject *adjustment = gtk_adjustment_new(DEFAULTSPEEDLIMIT,
                                                   1,
                                                   MAXSPEEDLIMIT,
                                                   1,
                                                   100,
                                                   0);
        speedLimitSpin = gtk_spin_button_new(GTK_ADJUSTMENT(adjustment),
                                             1.0,
                                             0);
        gtk_widget_set_usize(speedLimitSpin, 50, -1);
        gtk_widget_show(speedLimitSpin);
        gtk_box_pack_start(GTK_BOX(hbox), speedLimitSpin, FALSE, FALSE, 0);
    }
    {
        // execute arbitrary command when all downloads are over(ignore stopped or error items)
        {
            {
                GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
                gtk_widget_show(hbox);
                gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);


                GtkWidget *label = gtk_label_new(_("Execute command when:"));
                gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
                gtk_widget_show(label);
            }
            {
                GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
                gtk_widget_show(hbox);
                gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

                use_arb_command_toggle = gtk_check_button_new_with_label(_("all downloads are stopped or completed"));
                gtk_widget_show(use_arb_command_toggle);
                gtk_box_pack_start(GTK_BOX(hbox), use_arb_command_toggle, FALSE, FALSE, 10);
            }
            {
                GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
                gtk_widget_show(hbox);
                gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

                use_arb_command_timer_toggle = gtk_check_button_new_with_label(_("timer expires"));
                gtk_widget_show(use_arb_command_timer_toggle);
                gtk_box_pack_start(GTK_BOX(hbox), use_arb_command_timer_toggle, FALSE, FALSE, 10);
            }
        }
        {
            GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
            gtk_widget_show(hbox);
            gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
            GtkWidget *label = gtk_label_new(_("Command "));
            gtk_widget_show(label);
            gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 10);
            //// command entry
            arb_command_entry = gtk_entry_new();
            gtk_widget_set_usize(arb_command_entry, 300, -1);
            gtk_box_pack_start(GTK_BOX(hbox), arb_command_entry, FALSE, FALSE, 0);
            gtk_widget_show(arb_command_entry);
        }
    }
    {
        use_quit_program_toggle = gtk_check_button_new_with_label(_("Quit Aria when all downloads are over or stopped by timer"));
        gtk_widget_show(use_quit_program_toggle);
        gtk_box_pack_start(GTK_BOX(vbox), use_quit_program_toggle, FALSE, FALSE, 0);
    }
    return hbox;
}

bool AppOption::ret_use_ignore_extension_list()
{
    pthread_mutex_lock(&option_lock);
    bool retval = use_ignore_extension_list;
    pthread_mutex_unlock(&option_lock);
    return retval;

}

const list<string> &AppOption::ret_ignore_extension_list()
{
    pthread_mutex_lock(&option_lock);
    const list<string> &retval = ignore_extension_list;
    pthread_mutex_unlock(&option_lock);

    return retval;
}

void AppOption::Set_Option_Values(int maxthread_in,
                                  bool use_automatic_start_in,
                                  bool autostartModCurrentListOnly_in,
                                  bool useForceDownloadNow_in,
                                  bool use_ignore_error_item_in,
                                  bool use_ignore_extension_list_in,
                                  const list<string> &ignore_extension_list_in,
                                  bool use_autosave_in,
                                  int autosave_interval_in,
                                  int history_limit_in,
                                  int speedLimit_in,
                                  bool use_arb_command_in,
                                  bool use_arb_command_timer_in,
                                  const string &arb_command_in,
                                  bool use_quit_program_in,
                                  bool startTimerEnabled_in,
                                  bool stopTimerEnabled_in,
                                  bool timer_start_all_list_in,
                                  bool noStopDownloadOnTimer_in,
                                  const TimerData &timerdata_in,
                                  bool confirm_clear_in,
                                  bool confirm_delete_list_in,
                                  bool confirm_exit_in,
                                  bool confirm_clearlog_in,
                                  bool use_servertemplate_in,
                                  bool use_commandlist_in,
                                  bool use_size_human_readable_in,
                                  const string &statusIconDir_in,
                                  const string &basketPixmapFile_in,
                                  bool directPastingFromBasket_in,
                                  const list<string> &svt_name_list,
                                  const list<string> &com_name_list)
{
    if (maxthread_in > g_threadLimit) {
        maxthread = g_threadLimit;
    } else {
        maxthread = maxthread_in;
    }
    use_automatic_start = use_automatic_start_in;
    autostartModCurrentListOnly = autostartModCurrentListOnly_in;
    useForceDownloadNow = useForceDownloadNow_in;
    use_ignore_error_item = use_ignore_error_item_in;
    use_ignore_extension_list = use_ignore_extension_list_in;
    ignore_extension_list = ignore_extension_list_in;
    confirm_clear = confirm_clear_in;
    confirm_delete_list = confirm_delete_list_in;
    confirm_exit = confirm_exit_in;
    confirm_clearlog = confirm_clearlog_in;
    use_servertemplate = use_servertemplate_in;
    use_commandlist = use_commandlist_in;
    use_size_human_readable = use_size_human_readable_in;
    set_svt_clist(svt_name_list);
    set_com_clist(com_name_list);

    use_autosave = use_autosave_in;
    if (autosave_interval_in < 1) {
        autosave_interval = 10;
    } else {
        autosave_interval = autosave_interval_in;
    }

    if (autosave_tag == -1 && use_autosave) {
        //gtk_timeout_remove(autosave_tag);
        autosave_tag = gtk_timeout_add(autosave_interval * 60000,
                                       GtkFunction(Autosave_list),
                                       NULL);
    }

    if (history_limit_in < 0) {
        history_limit = 0;
    } else if (history_limit_in > MAXHISTORY) {
        history_limit = MAXHISTORY;
    } else {
        history_limit = history_limit_in;
    }

    if (speedLimit_in <= 0) { // fix this
        speedLimit = 1; // fix this
    } else if (speedLimit_in > MAXSPEEDLIMIT) {
        speedLimit = MAXSPEEDLIMIT;
    } else {
        speedLimit = speedLimit_in;
    }
    // fix this
    /*
    int watchClipBoard_tag = -1;
    if(watchClipBoard_tag == -1) {
      watchClipBoard_tag = gtk_timeout_add(750,
    				 GtkFunction(Watch_clip_board),
    				 NULL);
    }
    */

    // execute command when ...
    use_arb_command = use_arb_command_in;
    use_arb_command_timer = use_arb_command_timer_in;
    arb_command = Remove_white(arb_command_in);

    use_quit_program = use_quit_program_in;

    // timer
    startTimerEnabled = startTimerEnabled_in;
    stopTimerEnabled = stopTimerEnabled_in;

    timer_start_all_list = timer_start_all_list_in;
    noStopDownloadOnTimer = noStopDownloadOnTimer_in;
    timerdata = timerdata_in;

//    cerr << timerdata.ret_start_time() << endl;
//    cerr << timerdata.ret_stop_time() << endl;

//    cerr << (int)(timerdata.ret_stop_time()-timerdata.ret_start_time()) << endl;

    if (startTimerEnabled && stopTimerEnabled) {
        timerdata.adjustStopTimeBasedOnStartTime();
    }

//    cerr << timerdata.ret_stop_time() << endl;

    if (timer_start_tag == -1 && startTimerEnabled) {
        timer_start_tag = gtk_timeout_add(get_interval(timerdata.ret_start_time()),
                                          GtkFunction(Timer_start),
                                          NULL);
    }
    if (timer_stop_tag == -1 && stopTimerEnabled) {
        timer_stop_tag = gtk_timeout_add(get_interval(timerdata.ret_stop_time()),
                                         GtkFunction(Timer_stop),
                                         NULL);
    }

    // basket option
    directPastingFromBasket = directPastingFromBasket_in;

    // set pixmap to basket
    if (basketPixmapFile_in.empty()) basketPixmapFile = ""; //DEFAULT_BASKET_PIXMAP;
    else basketPixmapFile = basketPixmapFile_in;

    createBasketPixmapFromFile(basketPixmapFile);
    basket->setPixmap(basketPixmapTemp, basketBitmapTemp);

    // set iconset
    // initialize pixmap and mask array
    for (int i = 0; i < ICON_TOTAL; ++i) {
        statusIcon[i] = NULL;
        statusIconMask[i] = NULL;
    }

    if (statusIconDir_in.empty()) statusIconDir = ""; //DEFAULT_ICONSET;
    else statusIconDir = statusIconDir_in;

    if (!createStatusIcon(statusIconDir)) {
        for (int i = 0; i < ICON_TOTAL; ++i) {
            if (statusIcon[i] != NULL) gdk_pixmap_unref(statusIcon[i]);
            if (statusIconMask[i] != NULL) gdk_bitmap_unref(statusIconMask[i]);
        }
        statusIconDir = "";//DEFAULT_ICONSET;
        createStatusIcon(statusIconDir);
    }

    g_listManager->setStatusIcon(statusIcon, statusIconMask);
}

void AppOption::setDirEntryToCList(GtkCList *clist, const string &dirPath)
{
    DIR *dir = opendir(dirPath.c_str());
    gtk_clist_clear(clist);
    char *titles[1];
    titles[0] = DEFAULT_ICONSET;
    gtk_clist_append(clist, titles);
    if (dir) {
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) {
                titles[0] = entry->d_name;
                gtk_clist_append(clist, titles);
            }
        }
    }
}

void AppOption::updateStatusIconDirList()
{
    const char *homeDir = g_get_home_dir();
    string baseDir = homeDir;
    baseDir += "/.aria"ICONSET_PATH;

    setDirEntryToCList(GTK_CLIST(statusIconDirList), baseDir);
}

void AppOption::updateBasketPixmapFileList()
{
    const char *homeDir = g_get_home_dir();
    string baseDir = homeDir;
    baseDir += "/.aria"BASKET_PIXMAP_PATH;

    setDirEntryToCList(GTK_CLIST(basketPixmapFileList), baseDir);
}

list<string> AppOption::ret_name_list(GtkWidget *clist)
{
    list<string> names;
    GList *node = GTK_CLIST(clist)->selection;
    while (node) {
        int rowindex = GPOINTER_TO_UINT(node->data);
        char *name;
        gtk_clist_get_text(GTK_CLIST(clist), rowindex, 0, &name);
        names.push_back(name);
        node = g_list_next(node);
    }
    return names;
}

list<string> AppOption::ret_svt_name_list()
{
    return ret_name_list(svt_clist);
}

list<string> AppOption::ret_com_name_list()
{
    return ret_name_list(com_clist);
}

bool AppOption::Is_in_ignore_extension_list(const string &filename)
{
    pthread_mutex_lock(&option_lock);
    bool retval = false;
    for (list<string>::const_iterator itr = ignore_extension_list.begin();
            itr != ignore_extension_list.end(); ++itr) {
        if (itr->size() < filename.size()) {
            if (filename.substr(filename.size() - itr->size()) == *itr) retval =  true;
            break;
        }
    }
    pthread_mutex_unlock(&option_lock);
    return retval;
}

void set_timer_spin(GtkWidget *spin, int value, int min, int max, int step_min, int step_max)
{
    GtkObject *adjustment = gtk_adjustment_new(value,
                                               min,
                                               max,
                                               step_min,
                                               step_max,
                                               0);
    gtk_spin_button_configure(GTK_SPIN_BUTTON(spin),
                              GTK_ADJUSTMENT(adjustment),
                              1.0, 0);
}

void AppOption::Show_option_window()
{
    //ListEntry *listentry = g_listManager->ret_Current_listentry();

    GtkObject *adjustment_thread = gtk_adjustment_new(maxthread,
                                                      1,
                                                      g_threadLimit,
                                                      1,
                                                      1,
                                                      0);
    gtk_spin_button_configure(GTK_SPIN_BUTTON(spin_thread),
                              GTK_ADJUSTMENT(adjustment_thread),
                              1.0, 0);


    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(autoStartToggle), use_automatic_start);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(autostartModCurrentListOnly_toggle), autostartModCurrentListOnly);
    if (GTK_TOGGLE_BUTTON(autoStartToggle)->active) {
        gtk_widget_set_sensitive(autostartModCurrentListOnly_toggle, TRUE);
    } else {
        gtk_widget_set_sensitive(autostartModCurrentListOnly_toggle, FALSE);
    }
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(useForceDownloadNowToggle), useForceDownloadNow);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ignore_error_item_toggle), use_ignore_error_item);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ignore_extension_toggle), use_ignore_extension_list);

    string ignore_extension_string;
    for (list<string>::const_iterator itr = ignore_extension_list.begin();
            itr != ignore_extension_list.end(); ++itr) {
        ignore_extension_string += *itr + ' ';
    }
    gtk_entry_set_text(GTK_ENTRY(ignore_extension_entry), ignore_extension_string.c_str());

    // autosave
    GtkObject *adjustment_autosave = gtk_adjustment_new(autosave_interval,
                                                        1,
                                                        MAXAUTOSAVEINTERVAL,
                                                        1,
                                                        30,
                                                        0);
    gtk_spin_button_configure(GTK_SPIN_BUTTON(autosave_spin),
                              GTK_ADJUSTMENT(adjustment_autosave),
                              1.0, 0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(autosave_toggle), use_autosave);

    // limit history list
    GtkObject *adjustment_history = gtk_adjustment_new(history_limit,
                                                       0,
                                                       MAXHISTORY,
                                                       1,
                                                       100,
                                                       0);
    gtk_spin_button_configure(GTK_SPIN_BUTTON(history_limit_spin),
                              GTK_ADJUSTMENT(adjustment_history),
                              1.0, 0);

    // max value of speed limiter
    GtkObject *adjustment_speed = gtk_adjustment_new(speedLimit,
                                                     1,
                                                     MAXSPEEDLIMIT,
                                                     1,
                                                     100,
                                                     0);
    gtk_spin_button_configure(GTK_SPIN_BUTTON(speedLimitSpin),
                              GTK_ADJUSTMENT(adjustment_speed),
                              1.0, 0);


    // execute command when ....
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(use_arb_command_toggle), use_arb_command);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(use_arb_command_timer_toggle), use_arb_command_timer);
    gtk_entry_set_text(GTK_ENTRY(arb_command_entry), arb_command.c_str());

    // quit Aria when ...
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(use_quit_program_toggle), use_quit_program);

    // timer
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(startTimerEnabledToggle), startTimerEnabled);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(stopTimerEnabledToggle), stopTimerEnabled);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(noStopDownloadOnTimerToggle),
                                 noStopDownloadOnTimer);
    set_timer_spin(timer_hour_start_spin, timerdata.ret_hour_start(), 0, 23, 1, 23);
    set_timer_spin(timer_min_start_spin, timerdata.ret_min_start(), 0, 59, 1, 10);
    set_timer_spin(timer_hour_stop_spin, timerdata.ret_hour_stop(), 0, 23, 1, 23);
    set_timer_spin(timer_min_stop_spin, timerdata.ret_min_stop(), 0, 59, 1, 10);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(timer_start_all_list_toggle), timer_start_all_list);
    // confirmation
    /*
    if(confirm_clear) {
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(confirm_clear_toggle), TRUE);
    } else {
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(confirm_clear_toggle), FALSE);
    }
    */
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(confirm_clear_toggle), confirm_clear);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(confirm_delete_list_toggle), confirm_delete_list);
    /*
    if(confirm_exit) {
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(confirm_exit_toggle), TRUE);
    } else {
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(confirm_exit_toggle), FALSE);
    }
    */
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(confirm_exit_toggle), confirm_exit);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(confirm_clearlog_toggle), confirm_clearlog);

    if (use_size_human_readable) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(use_size_human_readable_toggle), TRUE);
    } else {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(use_size_human_readable_toggle), FALSE);
    }

    if (use_servertemplate) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(use_servertemplate_toggle), TRUE);
    } else {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(use_servertemplate_toggle), FALSE);
    }

    if (use_commandlist) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(use_commandlist_toggle), TRUE);
    } else {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(use_commandlist_toggle), FALSE);
    }

    // basket options
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(directPastingFromBasketToggle), directPastingFromBasket);

    updateStatusIconDirList();
    updateBasketPixmapFileList();

    gtk_notebook_set_page(GTK_NOTEBOOK(notebook), 0);
    gtk_widget_show(option_window);
}

GtkWidget *AppOption::ret_Option_Window() const
{
    return(option_window);
}

bool AppOption::Increase_thread()
{
    if (maxthread == g_threadLimit) return false;
    else {
        ++maxthread;
        Toolbar_set_sensitive__thread();
        //pthread_mutex_lock(&itemlistlock);  //mod 2001/4/11
        ListEntry *listentry = g_listManager->ret_Current_listentry();
        listentry->getThreadManager()->ManageThread(maxthread);
        //pthread_mutex_unlock(&itemlistlock);  //mod 2001/4/11
    }
    return true;
}

bool AppOption::Decrease_thread()
{
    if (maxthread == 1) return false;
    else {
        --maxthread;
        Toolbar_set_sensitive__thread();
        //pthread_mutex_lock(&itemlistlock); //mod 2001/4/11
        ListEntry *listentry = g_listManager->ret_Current_listentry();
        listentry->getThreadManager()->ManageThread(maxthread);
        //pthread_mutex_unlock(&itemlistlock); //mod 2001/4/11
    }
    return true;
}

int AppOption::getThreadMax() const
{
    return maxthread;
}

void AppOption::setThreadMax(int maxthread_in)
{
    maxthread = maxthread_in;
}

int AppOption::getTimerInterval()
{
    return 0;
}

void AppOption::Start_start_timer(int check_interval)
{
    if (timer_start_tag > -1) {
        gtk_timeout_remove(timer_start_tag);
    }
    timer_start_tag = gtk_timeout_add(check_interval,
                                      GtkFunction(Timer_start),
                                      NULL);
}

void AppOption::Start_stop_timer(int check_interval)
{
    if (timer_stop_tag > -1) {
        gtk_timeout_remove(timer_stop_tag);
    }
    timer_stop_tag = gtk_timeout_add(check_interval,
                                     GtkFunction(Timer_stop),
                                     NULL);
}

bool AppOption::isNoStopDownloadOnTimerEnabled()
{
    pthread_mutex_lock(&option_lock);
    bool retval = noStopDownloadOnTimer;
    pthread_mutex_unlock(&option_lock);

    return retval;
}

void AppOption::Process_changes()
{
    pthread_mutex_lock(&option_lock);

    use_automatic_start = false;
    if (GTK_TOGGLE_BUTTON(autoStartToggle)->active) {
        use_automatic_start = true;
    }

    if (GTK_TOGGLE_BUTTON(autostartModCurrentListOnly_toggle)->active) {
        autostartModCurrentListOnly = true;
    } else {
        autostartModCurrentListOnly = false;
    }

    if (GTK_TOGGLE_BUTTON(useForceDownloadNowToggle)->active) {
        useForceDownloadNow = true;
    } else {
        useForceDownloadNow = false;
    }

    use_ignore_error_item = false;
    if (GTK_TOGGLE_BUTTON(ignore_error_item_toggle)->active) {
        use_ignore_error_item = true;
    }

    int autosave_interval_in = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(autosave_spin));
    if (GTK_TOGGLE_BUTTON(autosave_toggle)->active) {
        if (use_autosave && autosave_interval == autosave_interval_in) {
            autosave_interval = autosave_interval_in;
        } else {
            autosave_interval = autosave_interval_in;
            use_autosave = true;
            if (autosave_tag > -1) {
                gtk_timeout_remove(autosave_tag);
            }
            autosave_tag = gtk_timeout_add(autosave_interval * 60000,
                                           GtkFunction(Autosave_list),
                                           NULL);
        }
    } else {
        if (autosave_tag > -1) {
            gtk_timeout_remove(autosave_tag);
            autosave_tag = -1;
        }
        use_autosave = false;
        autosave_interval = autosave_interval_in;
    }
    // timer
    int hour_start = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(timer_hour_start_spin));
    int min_start = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(timer_min_start_spin));
    int hour_stop = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(timer_hour_stop_spin));
    int min_stop = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(timer_min_stop_spin));
    timerdata.set_start_time(hour_start, min_start);
    timerdata.set_stop_time(hour_stop, min_stop);

    timer_start_all_list = false;
    if (GTK_TOGGLE_BUTTON(timer_start_all_list_toggle)->active) {
        timer_start_all_list = true;
    }
    noStopDownloadOnTimer = false;
    if (GTK_TOGGLE_BUTTON(noStopDownloadOnTimerToggle)->active) {
        noStopDownloadOnTimer = true;
    }
    if (GTK_TOGGLE_BUTTON(startTimerEnabledToggle)->active) {
        startTimerEnabled = true;
        Start_start_timer(get_interval(timerdata.ret_start_time()));
    } else {
        startTimerEnabled = false;
        if (timer_start_tag > -1) {
            gtk_timeout_remove(timer_start_tag);
            timer_start_tag = -1;
        }
    }
    if (GTK_TOGGLE_BUTTON(stopTimerEnabledToggle)->active) {
        stopTimerEnabled = true;
        if (startTimerEnabled) {
            timerdata.adjustStopTimeBasedOnStartTime();
        }

//      cerr << "Current time is " << time(NULL) << endl;
//      cerr << timerdata.ret_start_time() << endl;
//      cerr << timerdata.ret_stop_time() << endl;

        Start_stop_timer(get_interval(timerdata.ret_stop_time()));
    } else {
        stopTimerEnabled = false;
        if (timer_stop_tag > -1) {
            gtk_timeout_remove(timer_stop_tag);
            timer_stop_tag = -1;
        }
    }

    // limit history list
    history_limit = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(history_limit_spin));
    //if(history_limit > MAXHISTORY) history_limit = MAXHISTORY;

    // max value of speed limiter
    speedLimit = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(speedLimitSpin));
    Adjust_speed_scale(speedLimit);

    // execute command when ...
    if (GTK_TOGGLE_BUTTON(use_arb_command_toggle)->active) {
        use_arb_command = true;
    } else {
        use_arb_command = false;
    }
    if (GTK_TOGGLE_BUTTON(use_arb_command_timer_toggle)->active) {
        use_arb_command_timer = true;
    } else {
        use_arb_command_timer = false;
    }
    arb_command = Remove_white(gtk_entry_get_text(GTK_ENTRY(arb_command_entry)));

    // quit Aria when ...
    if (GTK_TOGGLE_BUTTON(use_quit_program_toggle)->active) {
        use_quit_program = true;
    } else {
        use_quit_program = false;
    }

    // max thread
    int maxthread_in = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_thread));

    if (maxthread_in > 0 && maxthread_in <= g_threadLimit) {
        maxthread = maxthread_in;
        /*
        Toolbar_set_sensitive__thread();
        //pthread_mutex_lock(&itemlistlock); //mod 2001/4/11
        ListEntry *listentry = g_listManager->ret_Current_listentry();// fix this
        listentry->getThreadmanager()->ManageThread(maxthread_in);
        //pthread_mutex_unlock(&itemlistlock); //mod 2001/4/11
        */
    }

    use_ignore_extension_list = false;
    if (GTK_TOGGLE_BUTTON(ignore_extension_toggle)->active) {
        use_ignore_extension_list = true;
    }

    string ignore_extension_string = Remove_white(gtk_entry_get_text(GTK_ENTRY(ignore_extension_entry)));
    list<string> ignore_extension_list_in;
    while (!ignore_extension_string.empty()) {
        ignore_extension_list_in.push_back(Token_splitter(ignore_extension_string, " ,"));
    }
    ignore_extension_list = ignore_extension_list_in;

    confirm_clear = false;
    if (GTK_TOGGLE_BUTTON(confirm_clear_toggle)->active) {
        confirm_clear = true;
    }

    confirm_delete_list = false;
    if (GTK_TOGGLE_BUTTON(confirm_delete_list_toggle)->active) {
        confirm_delete_list = true;
    }

    confirm_exit = false;
    if (GTK_TOGGLE_BUTTON(confirm_exit_toggle)->active) {
        confirm_exit = true;
    }

    confirm_clearlog = false;
    if (GTK_TOGGLE_BUTTON(confirm_clearlog_toggle)->active) {
        confirm_clearlog = true;
    }

    use_servertemplate = false;
    if (GTK_TOGGLE_BUTTON(use_servertemplate_toggle)->active) {
        use_servertemplate = true;
    }
    g_servTempList.update_validity(GTK_CLIST(svt_clist)->selection);

    use_commandlist = false;
    if (GTK_TOGGLE_BUTTON(use_commandlist_toggle)->active) {
        use_commandlist = true;
    }
    g_commandList.update_validity(GTK_CLIST(com_clist)->selection);

    bool use_size_human_readable_temp = use_size_human_readable;
    use_size_human_readable = false;
    if (GTK_TOGGLE_BUTTON(use_size_human_readable_toggle)->active) {
        use_size_human_readable = true;
    }

    // basket options
    directPastingFromBasket = false;
    if (GTK_TOGGLE_BUTTON(directPastingFromBasketToggle)->active) {
        directPastingFromBasket = true;
    }

    // clear up basket pixmap preview
    if (basketPixmapPreview != NULL) {
        gtk_widget_hide(basketPixmapPreview);
        gtk_widget_destroy(basketPixmapPreview);
        basketPixmapPreview = NULL;
    }

    pthread_mutex_unlock(&option_lock);
    // clear icon preview clist
    gtk_clist_clear(GTK_CLIST(statusIconPreviewList));
    // add 1 row
    char *titles[ICON_TOTAL];
    for (int i = 0; i < ICON_TOTAL; ++i) {
        titles[i] = NULL;
    }
    gtk_clist_append(GTK_CLIST(statusIconPreviewList), titles);

    if ((use_size_human_readable && !use_size_human_readable_temp) ||
            (!use_size_human_readable && use_size_human_readable_temp)) {
        for (list<ListEntry *>::const_iterator itr = g_listManager->ret_Listentry_list().begin(); itr != g_listManager->ret_Listentry_list().end(); ++itr) {
            (*itr)->setSizeDisplayStyle(use_size_human_readable);
        }
    }
}

bool AppOption::Whether_use_automatic_start()
{
    pthread_mutex_lock(&option_lock);
    bool retval = use_automatic_start;
    pthread_mutex_unlock(&option_lock);
    return retval;
}

bool AppOption::isAutostartCurrentListOnly()
{
    pthread_mutex_lock(&option_lock);
    bool retval = autostartModCurrentListOnly;
    pthread_mutex_unlock(&option_lock);
    return retval;
}

bool AppOption::ret_use_ignore_error_item()
{
    pthread_mutex_lock(&option_lock);
    bool retval = use_ignore_error_item;
    pthread_mutex_unlock(&option_lock);

    return retval;
}

bool AppOption::ret_confirm_clear()
{
    pthread_mutex_lock(&option_lock);
    bool retval = confirm_clear;
    pthread_mutex_unlock(&option_lock);

    return retval;
}

bool AppOption::ret_confirm_delete_list()
{
    pthread_mutex_lock(&option_lock);
    bool retval = confirm_delete_list;
    pthread_mutex_unlock(&option_lock);

    return retval;
}

bool AppOption::ret_confirm_exit()
{
    pthread_mutex_lock(&option_lock);
    bool retval = confirm_exit;
    pthread_mutex_unlock(&option_lock);

    return retval;
}

bool AppOption::ret_confirm_clearlog()
{
    pthread_mutex_lock(&option_lock);
    bool retval = confirm_clearlog;
    pthread_mutex_unlock(&option_lock);

    return retval;
}

bool AppOption::ret_use_servertemplate()
{
    pthread_mutex_lock(&option_lock);
    bool retval = use_servertemplate;
    pthread_mutex_unlock(&option_lock);

    return retval;
}

bool AppOption::ret_use_commandlist()
{
    pthread_mutex_lock(&option_lock);
    bool retval = use_commandlist;
    pthread_mutex_unlock(&option_lock);

    return retval;
}

bool AppOption::ret_use_size_human_readable()
{
    pthread_mutex_lock(&option_lock);
    bool retval = use_size_human_readable;
    pthread_mutex_unlock(&option_lock);

    return retval;
}

bool AppOption::ret_use_autosave()
{
    pthread_mutex_lock(&option_lock);
    bool retval = use_autosave;
    pthread_mutex_unlock(&option_lock);

    return retval;
}

int AppOption::getAutosaveInterval()
{
    pthread_mutex_lock(&option_lock);
    int retval = autosave_interval;
    pthread_mutex_unlock(&option_lock);

    return retval;
}

int AppOption::ret_autosave_tag()
{
    pthread_mutex_lock(&option_lock);
    int retval = autosave_tag;
    pthread_mutex_unlock(&option_lock);

    return retval;
}

bool AppOption::isStartTimerEnabled()
{
    pthread_mutex_lock(&option_lock);
    bool retval = startTimerEnabled;
    pthread_mutex_unlock(&option_lock);

    return retval;
}

bool AppOption::isStopTimerEnabled()
{
    pthread_mutex_lock(&option_lock);
    bool retval = stopTimerEnabled;
    pthread_mutex_unlock(&option_lock);

    return retval;
}

bool AppOption::ret_timer_start_all_list()
{
    pthread_mutex_lock(&option_lock);
    bool retval = timer_start_all_list;
    pthread_mutex_unlock(&option_lock);

    return retval;
}

time_t AppOption::ret_timer_start_time()
{
    pthread_mutex_lock(&option_lock);
    time_t retval = timerdata.ret_start_time();
    pthread_mutex_unlock(&option_lock);

    return retval;
}

time_t AppOption::ret_timer_stop_time()
{
    pthread_mutex_lock(&option_lock);
    time_t retval = timerdata.ret_stop_time();
    pthread_mutex_unlock(&option_lock);

    return retval;
}

int AppOption::ret_timer_hour_start()
{
    pthread_mutex_lock(&option_lock);
    int retval = timerdata.ret_hour_start();
    pthread_mutex_unlock(&option_lock);

    return retval;
}

int AppOption::ret_timer_min_start()
{
    pthread_mutex_lock(&option_lock);
    int retval = timerdata.ret_min_start();
    pthread_mutex_unlock(&option_lock);

    return retval;
}

int AppOption::ret_timer_hour_stop()
{
    pthread_mutex_lock(&option_lock);
    int retval = timerdata.ret_hour_stop();
    pthread_mutex_unlock(&option_lock);

    return retval;
}

int AppOption::ret_timer_min_stop()
{
    pthread_mutex_lock(&option_lock);
    int retval = timerdata.ret_min_stop();
    pthread_mutex_unlock(&option_lock);

    return retval;
}

void AppOption::Update_timer_start()
{
    pthread_mutex_lock(&option_lock);
    timerdata.Update_start_time();
    pthread_mutex_unlock(&option_lock);
}

void AppOption::Update_timer_stop()
{
    pthread_mutex_lock(&option_lock);
    timerdata.Update_stop_time();
    pthread_mutex_unlock(&option_lock);
}

int AppOption::ret_history_limit()
{
    pthread_mutex_lock(&option_lock);
    int retval = history_limit;
    pthread_mutex_unlock(&option_lock);
    return retval;
}

int AppOption::getSpeedLimitMax()
{
    pthread_mutex_lock(&option_lock);
    int retval = speedLimit;
    pthread_mutex_unlock(&option_lock);
    return retval;
}

bool AppOption::ret_use_arb_command()
{
    pthread_mutex_lock(&option_lock);
    bool retval = use_arb_command;
    pthread_mutex_unlock(&option_lock);

    return retval;
}

bool AppOption::ret_use_arb_command_timer()
{
    pthread_mutex_lock(&option_lock);
    bool retval = use_arb_command_timer;
    pthread_mutex_unlock(&option_lock);

    return retval;
}

string AppOption::ret_arb_command()
{
    pthread_mutex_lock(&option_lock);
    string retval = arb_command;
    pthread_mutex_unlock(&option_lock);

    return retval;
}

bool AppOption::ret_use_quit_program()
{
    pthread_mutex_lock(&option_lock);
    bool retval = use_quit_program;
    pthread_mutex_unlock(&option_lock);

    return retval;
}

void AppOption::setStatusIconDir(const string &statusIconDir_in)
{
    statusIconDir = statusIconDir_in;
}

string AppOption::getStatusIconDir()
{
    pthread_mutex_lock(&option_lock);
    string retval =  statusIconDir;
    pthread_mutex_unlock(&option_lock);
    return retval;
}

bool AppOption::isForceDownloadNowEnabled()
{
    pthread_mutex_lock(&option_lock);
    bool retval = useForceDownloadNow;
    pthread_mutex_unlock(&option_lock);

    return retval;
}

bool AppOption::isDirectPastingFromBasketEnabled()
{
    pthread_mutex_lock(&option_lock);
    bool retval = directPastingFromBasket;
    pthread_mutex_unlock(&option_lock);

    return retval;
}
