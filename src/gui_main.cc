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

// $Id: gui_main.cc,v 1.56 2002/12/18 15:41:05 tujikawa Exp $

#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "aria.h"
#include "utils.h"
#include "gui_utils.h"
#include "ItemCell.h"
#include "ItemOption.h"
#include "ItemList.h"
#include "ItemManager.h"
#include "ItemStatus.h"
#include "FileBrowser.h"
#include "Dialog.h"
#include "CtrlSocket.h"
#include "PasteWindow.h"
#include "SumInfo.h"
#include "Basket.h"
#include "pixmaps/progress_bar.h"
#include "pixmaps/speed.xpm"
#include "aria_mime.h"

using namespace std;

// external functions
extern gboolean File_quit_c(GtkWidget *w, gboolean (*Signal_Func)(GtkWidget *w, GtkWidget *window));
extern gboolean File_quit(GtkWidget *w, GtkWidget *unused);
extern void Create_file_selection_windows(GtkWidget *w);
extern void Create_file_menu(GtkWidget *toplevel, GtkWidget *menu_bar, GtkAccelGroup *accel_group);
extern void Create_edit_menu(GtkWidget *toplevel, GtkWidget *menu_bar, GtkAccelGroup *accel_group);
extern void Create_item_menu(GtkWidget *toplevel, GtkWidget *menu_bar, GtkAccelGroup *accel_group);
extern void Create_list_menu(GtkWidget *toplevel, GtkWidget *menu_bar, GtkAccelGroup *accel_group);
extern void Create_download_menu(GtkWidget *toplevel, GtkWidget *menu_bar, GtkAccelGroup *accel_group);
extern void Create_option_menu(GtkWidget *toplevel, GtkWidget *menu_bar, GtkAccelGroup *accel_group);
extern void Create_help_menu(GtkWidget *window, GtkWidget *menu_bar, GtkAccelGroup *accel_group);
extern GtkWidget *Create_toolbar(GtkWidget *toplevel);
extern void Create_popup_menu(GtkWidget *toplevel);
extern void Download_change_speed(float fspeed, ListEntry *listentry);
extern void Toolbar_set_thread_spin(int n_thread);

extern void Toolbar_set_sensitive__no_item_selected();
extern void Toolbar_set_sensitive__items_selected();
extern void Toolbar_set_sensitive__list_empty();
extern void Toolbar_set_sensitive__list_not_empty();
extern void Item_set_sensitive__no_item_selected();
extern void Item_set_sensitive__items_selected();
extern void Item_set_sensitive__list_empty();
extern void Item_set_sensitive__list_not_empty();
extern void Download_set_sensitive__no_item_selected();
extern void Download_set_sensitive__items_selected();
extern void Download_set_sensitive__list_empty();
extern void Download_set_sensitive__list_not_empty();
extern void Edit_set_sensitive__no_item_selected();
extern void Edit_set_sensitive__items_selected();

extern void Option_set_sensitive__no_item_selected();
extern void Option_set_sensitive__items_selected();
extern void Option_set_sensitive__list_empty();
extern void Option_set_sensitive__list_not_empty();
extern void Track_disable();

// global variables
extern int g_pipetogui[2];
extern AppOption *g_appOption;
//extern GtkWidget *g_popupMenu;
extern ListManager *g_listManager;
extern ItemOption *g_itemOption;
extern FileBrowser *g_cFileBrowser;
extern Dialog *g_cDialog;
extern CtrlSocket g_ctrlSock;
extern int g_threadLimit;
Basket *basket = NULL;

SumInfo g_summaryInfo;
GtkWidget *g_toplevel;
GtkWidget *g_consoleText;
GtkWidget *g_text;
GtkWidget *sg_sumInfoDlLabel;
GtkWidget *sg_sumInfoErrLabel;
//  GdkPixmap *g_statIcon[ICON_TOTAL];
//  GdkBitmap *g_statIconMask[ICON_TOTAL];
PasteWindow *g_pasteWindow;

//GdkBitmap *sg_progressBarMask[51];
GdkPixbuf *sg_progressBar[51];

static GtkWidget *sg_speedScale;
static GtkWidget *sg_speedScaleSpin;
static GtkStyle *sg_dlCListStyle;

static GtkWidget *sg_itemLogPage;
static GtkWidget *sg_textNotebook;
static GtkWidget *sg_vpaned;

void Set_sensitive__no_item_selected()
{
    gtk_widget_set_sensitive(sg_speedScale, FALSE);
    gtk_widget_set_sensitive(sg_speedScaleSpin, FALSE);
    Toolbar_set_sensitive__no_item_selected();
    Item_set_sensitive__no_item_selected();
    Download_set_sensitive__no_item_selected();
    Edit_set_sensitive__no_item_selected();
    Option_set_sensitive__no_item_selected();
    Edit_set_sensitive__no_item_selected();
}

void Set_sensitive__items_selected()
{
    gtk_widget_set_sensitive(sg_speedScale, TRUE);
    gtk_widget_set_sensitive(sg_speedScaleSpin, TRUE);
    Toolbar_set_sensitive__items_selected();
    Item_set_sensitive__items_selected();
    Download_set_sensitive__items_selected();
    Edit_set_sensitive__items_selected();
    Option_set_sensitive__items_selected();
    Edit_set_sensitive__items_selected();
}

void Set_sensitive__list_empty()
{
    Toolbar_set_sensitive__list_empty();
    Item_set_sensitive__list_empty();
    Download_set_sensitive__list_empty();
    Option_set_sensitive__list_empty();
}

void Set_sensitive__list_not_empty()
{
    Toolbar_set_sensitive__list_not_empty();
    Item_set_sensitive__list_not_empty();
    Download_set_sensitive__list_not_empty();
    Option_set_sensitive__list_not_empty();
}

void Set_suminfo_label()
{
    //string strSuminfo = "Downloading: "+itos(suminfo.ret_download())+" Error: "+itos(suminfo.ret_error());
    gtk_label_set_text(GTK_LABEL(sg_sumInfoDlLabel), (_("Downloading: ") + itos(g_summaryInfo.ret_download())).c_str());
    gtk_label_set_text(GTK_LABEL(sg_sumInfoErrLabel), (_(" Error: ") + itos(g_summaryInfo.ret_error())).c_str());
}

void CTRLSOCK_SEND_LS(int fd, const string &url_str)
{
    if (g_ctrlSock.Send_command(fd, CmdPacketHeader::CMD_DATA, (void *)url_str.c_str(), url_str.size() + 1) < 0) throw 0;
    if (g_ctrlSock.Recv_command(fd, NULL) < 0) throw 0;
}

// send list name
void Show_remote_current_listname(int fd, ListEntry *listentry = NULL)
{
    if (listentry == NULL) {
        listentry = g_listManager->ret_Current_listentry();
    }
    string name = _("#List name: ") + listentry->getName();
    if (g_ctrlSock.Send_command(fd, CmdPacketHeader::CMD_DATA, (void *)name.c_str(), name.size() + 1) < 0) throw 0;
    if (g_ctrlSock.Recv_command(fd, NULL) < 0) throw 0;
}

void Send_ls_item(int fd, int ls_flag)
{
    //pthread_mutex_lock(&itemlistlock);
    try {
        //for(list<ListEntry*>::const_iterator itr = list_manager->ret_Listentry_list().begin(); itr != list_manager->ret_Listentry_list().end(); ++itr) {
        //ListEntry *listentry = *itr;

        ListEntry *listentry = g_listManager->ret_Current_listentry();
        Show_remote_current_listname(fd, listentry);
        for (int rowindex = 0; rowindex < GTK_CLIST(listentry->ret_Dl_clist())->rows; ++rowindex) {
            ItemCell *itemcell = (ItemCell *)gtk_clist_get_row_data(GTK_CLIST(listentry->ret_Dl_clist()), rowindex);
            string url = itemcell->ret_URL();
            //Status (Ready/Error/Downloading/Stop/Complete/Lock)
            //| Split?(Yes/No/split Parts)
            //|/
            //++-=================================================================
            if (itemcell->Is_Partial()) {
                url = "p " + url;
            } else if (itemcell->ret_Status() == ItemCell::ITEM_DOWNLOAD_PARTIAL) {
                url = "y " + url;
            } else {
                url = "n " + url;
            }
            switch (itemcell->ret_Status()) {
                case ItemCell::ITEM_READY:
                case ItemCell::ITEM_READY_AGAIN:
                case ItemCell::ITEM_READY_CONCAT:
                case ItemCell::ITEM_DOWNLOAD_PARTIAL:
                    if (CtrlSocket::CTSOCK_LSREADY & ls_flag) {
                        CTRLSOCK_SEND_LS(fd, 'r' + url);
                    }
                    break;
                case ItemCell::ITEM_CRCERROR:
                case ItemCell::ITEM_EXECERROR:
                case ItemCell::ITEM_ERROR:
                    if (CtrlSocket::CTSOCK_LSFAILED & ls_flag) {
                        CTRLSOCK_SEND_LS(fd, 'e' + url);
                    }
                    break;
                case ItemCell::ITEM_STOP:
                    if (CtrlSocket::CTSOCK_LSSTOP & ls_flag) {
                        CTRLSOCK_SEND_LS(fd, 's' + url);
                    }
                    break;
                case ItemCell::ITEM_DOWNLOAD:
                case ItemCell::ITEM_INUSE:
                case ItemCell::ITEM_INUSE_AGAIN:
                case ItemCell::ITEM_INUSE_CONCAT:
                case ItemCell::ITEM_DOWNLOAD_AGAIN:
                case ItemCell::ITEM_DOWNLOAD_INTERNAL_AGAIN:
                    if (CtrlSocket::CTSOCK_LSGO & ls_flag) {
                        CTRLSOCK_SEND_LS(fd, 'd' + url);
                    }
                    break;
                case ItemCell::ITEM_COMPLETE:
                    if (CtrlSocket::CTSOCK_LSCOMP & ls_flag) {
                        CTRLSOCK_SEND_LS(fd, 'c' + url);
                    }
                    break;
                case ItemCell::ITEM_LOCK:
                    if (CtrlSocket::CTSOCK_LSLOCK & ls_flag) { // fixed 2001/3/8
                        CTRLSOCK_SEND_LS(fd, 'l' + url);
                    }
                    break;
                default:
                    if (CtrlSocket::CTSOCK_LSALL == ls_flag) { // fixed 2001/3/8
                        CTRLSOCK_SEND_LS(fd, '?' + url);
                    }
                    break;
            }
        }
    } catch (int err) {
    }
    //pthread_mutex_unlock(&itemlistlock);
}

// 色の取得
static GdkColor Get_color(MessageType reporttype)
{
    GdkColor color;

    switch (reporttype) {
        case MSG_DOWNLOAD_INFO:
        case MSG_SYS_INFO:
            color.red = 0;
            color.green = 0;
            color.blue = 0;
            break;
        case MSG_DOWNLOAD_SEND:
            //color.red = 0x8888;
            //color.green = 0x8888;
            //color.blue = 0x8888;
            color.red = 14417;
            color.green = 23592;
            color.blue = 19005;
            break;
        case MSG_DOWNLOAD_RECV:
            color.red = 34078;
            color.green = 14417;
            color.blue = 23592;
            break;
        case MSG_SYS_SUCCESS:
        case MSG_DOWNLOAD_SUCCESS:
            color.red = 0;
            color.green = 0;
            color.blue = 0xffff;
            break;
        case MSG_DOWNLOAD_ERROR:
        case MSG_SYS_ERROR:
            color.red = 0xffff;
            color.green = 0;
            color.blue = 0;
            break;
        default:
            color.red = 0;
            color.green = 0;
            color.blue = 0;
    }
    return color;
}

// アイテムのコンソール画面にreport_typeによって別のprefixを置く
//  static void TEXT_INSERT_PREFIX(MessageType report_type, GdkColor color)
//  {
//    return;
//    switch(report_type) {
//    case MSG_DOWNLOAD_SEND:
//      gtk_text_insert(GTK_TEXT(text), NULL, &color, NULL, _("SEND: "), -1);
//      break;
//    case MSG_DOWNLOAD_RECV:
//      gtk_text_insert(GTK_TEXT(text), NULL, &color, NULL, _("RECV: "), -1);
//      break;
//    case MSG_DOWNLOAD_INFO:
//      gtk_text_insert(GTK_TEXT(text), NULL, &color, NULL, _("INFO: "), -1);
//      break;
//    case MSG_DOWNLOAD_SUCCESS:
//      gtk_text_insert(GTK_TEXT(text), NULL, &color, NULL, _("SUCC: "), -1);
//      break;
//    case MSG_DOWNLOAD_ERROR:
//      gtk_text_insert(GTK_TEXT(text), NULL, &color, NULL, _("ERR: "), -1);
//      break;
//    default:
//      break;
//    }
//  }

// メニューバーの作成
static GtkWidget *Create_menu_bar(GtkWindow  *window)
{
    GtkAccelGroup *accel_group;
    GtkWidget *menu_bar;

    menu_bar = gtk_menu_bar_new();
//  gtk_menu_bar_set_shadow_type(GTK_MENU_BAR(menu_bar), GTK_SHADOW_ETCHED_IN);
    gtk_widget_show(menu_bar);

    accel_group = gtk_accel_group_new();
    gtk_window_add_accel_group(window, accel_group);

    Create_file_menu(GTK_WIDGET(window), menu_bar, accel_group);
    Create_edit_menu(GTK_WIDGET(window), menu_bar, accel_group);
    Create_item_menu(GTK_WIDGET(window), menu_bar, accel_group);
    Create_list_menu(GTK_WIDGET(window), menu_bar, accel_group);
    Create_download_menu(GTK_WIDGET(window), menu_bar, accel_group);
    Create_option_menu(GTK_WIDGET(window), menu_bar, accel_group);
    Create_help_menu(GTK_WIDGET(window), menu_bar, accel_group);
    return(menu_bar);
}


void Adjust_speed_scale(int max)
{
    GtkAdjustment *adj = gtk_range_get_adjustment(GTK_RANGE(sg_speedScale));
    adj->upper = (float)max;

    adj = gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(sg_speedScaleSpin));
    adj->upper = (float)max;

    gtk_range_set_adjustment(GTK_RANGE(sg_speedScale), GTK_ADJUSTMENT(adj));
//  gtk_range_clear_background(GTK_RANGE(sg_speedScale));
//  gtk_range_draw_background(GTK_RANGE(sg_speedScale));
}

void Show_download_log(ItemCell *itemcell)
{
    //static GdkColormap* cmap = gdk_colormap_get_system();
    string line;
    // get data associated to the row
    //ItemCell* itemcell = (ItemCell*)gtk_clist_get_row_data(GTK_CLIST(dl_clist), rowindex);

    if (itemcell->ret_Filename().empty()) {
        line = _("<directory>");
    } else {
        line = itemcell->ret_Filename();
    }
    gtk_notebook_set_tab_label_text(GTK_NOTEBOOK(sg_textNotebook),
                                    sg_itemLogPage,
                                    line.c_str());

    gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(g_text)), "", 0);

    // acquire lock
    itemcell->get_Log_Lock();
    ItemLogList::const_iterator itemlog_itr;
    //gtk_text_freeze(GTK_TEXT(g_text));
    for (itemlog_itr = itemcell->ret_Log_list().begin(); itemlog_itr != itemcell->ret_Log_list().end(); ++itemlog_itr) {
        GdkColor color = Get_color(itemlog_itr->ret_Logtype());
        //gtk_text_insert(GTK_TEXT(g_text), NULL, &color, NULL, itemlog_itr->ret_Log().c_str(), -1);
        //TODO: colores
        GtkTextIter lastpos;
        gtk_text_buffer_get_iter_at_offset(gtk_text_view_get_buffer(GTK_TEXT_VIEW(g_text)), &lastpos, -1);
        gtk_text_buffer_insert(gtk_text_view_get_buffer(GTK_TEXT_VIEW(g_text)), &lastpos, itemlog_itr->ret_Log().c_str(), -1);
        if (itemlog_itr->ret_Log().at(itemlog_itr->ret_Log().size() - 1) != '\n') {
            gtk_text_buffer_insert(gtk_text_view_get_buffer(GTK_TEXT_VIEW(g_text)), &lastpos, "\n", -1);
        }
    }
    //gtk_text_thaw(GTK_TEXT(g_text));
    //gtk_adjustment_set_value(GTK_TEXT_VIEW(g_text)->vadj, GTK_TEXT_VIEW(g_text)->vadj->upper+GTK_TEXT_VIEW(g_text)->vadj->page_size);
    // release lock
    itemcell->release_Log_Lock();
}

// コンソールに文字列logを追加
void Append_text(const ItemLogCell &itemlogcell, GtkWidget *text)
{
    /*  unsigned int insertionpoint = gtk_text_get_length(GTK_TEXT(text));
      gtk_text_set_point(GTK_TEXT(text), insertionpoint);
      GdkColor color = Get_color(itemlogcell.ret_Logtype());
      bool scroll_to_end = false;
      if(GTK_TEXT(text)->vadj->value+GTK_TEXT(text)->vadj->page_size >= GTK_TEXT(text)->vadj->upper) {
        scroll_to_end = true;
      }
      gtk_text_freeze(GTK_TEXT(text));
      const string& log = itemlogcell.ret_Log();
      gtk_text_insert(GTK_TEXT(text), NULL, &color, NULL, log.c_str(), -1);*/
    /*
    if(log.at(log.size()-1) != '\n') {
      gtk_text_insert(GTK_TEXT(text), NULL, &color, NULL, "\n", -1);
    }
    */
    /*gtk_text_thaw (GTK_TEXT (text));
    if(scroll_to_end) {
      gtk_adjustment_set_value(GTK_TEXT(text)->vadj, GTK_TEXT(text)->vadj->upper);
    }*/
    //TODO: colors and scroll
    const string &log = itemlogcell.ret_Log();
    GtkTextIter lastpos;
    gtk_text_buffer_get_iter_at_offset(gtk_text_view_get_buffer(GTK_TEXT_VIEW(text)), &lastpos, -1);
    gtk_text_buffer_insert(gtk_text_view_get_buffer(GTK_TEXT_VIEW(text)), &lastpos, log.c_str(), -1);
}

// g_pipetogui[0]に書き込まれたデータを読み込み,GUIに反映させる
static void Update_gui(void *w, int dummy_int, GdkInputCondition dummy_cond)
{
    StatusReport statusreport;
    Socket socket(g_pipetogui[0], Socket::DUPE);

    list<int> fdslist;
    if (socket.is_readready(0, fdslist) > 0) {
        if (!read(g_pipetogui[0], &statusreport, sizeof(StatusReport)))
            return;
        // if the message is from gui thread, then mutex_p of statusreport
        // must be 0(NULL) pointer
        if (statusreport.mutex_p == 0) {
            ItemStatus *itemstatus = (ItemStatus *)(statusreport.data);
            itemstatus->Update();
            delete itemstatus;
        } else {
            pthread_mutex_lock(statusreport.mutex_p);
            ItemStatus *itemstatus = (ItemStatus *)(statusreport.data);
            itemstatus->Update();
            delete itemstatus;
            pthread_cond_signal(statusreport.cond_p);
            pthread_mutex_unlock(statusreport.mutex_p);
        }
    }
}

static GtkWidget *Create_text_view_frame(GtkWidget *textWidget)
{
    // text ウィジェットのための箱
    GtkWidget *vbox = gtk_vbox_new(FALSE, 5);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
    gtk_widget_show(vbox);

    // GtkText ウィジェットを作成
    // スクロールウインドウを作成しそれをテキストに関連づける
    GtkWidget *scrolledWindow = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledWindow),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
    gtk_container_add(GTK_CONTAINER(vbox), scrolledWindow);
    gtk_widget_show(scrolledWindow);

    // テキスト入力を不可にする
    gtk_text_view_set_editable(GTK_TEXT_VIEW(textWidget), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textWidget), GTK_WRAP_WORD_CHAR);
    //gtk_text_view_set_word_wrap(GTK_TEXT_VIEW(textWidget), TRUE);
    // track text viewpoint
    //gtk_text_set_adjustments(GTK_TEXT(text), NULL, NULL);
    // テキストウィジェットの表示
    gtk_container_add(GTK_CONTAINER(scrolledWindow), textWidget);
    gtk_widget_show(textWidget);

    return(vbox);
}

// create item download log text view
static GtkWidget *Create_item_log_text()
{
    g_text = gtk_text_view_new();
    return(Create_text_view_frame(g_text));
}

// create system log text view
static GtkWidget *Create_console_log_text()
{
    g_consoleText = gtk_text_view_new();
    return(Create_text_view_frame(g_consoleText));
}

static GtkWidget *Create_text_notebook()
{
    // create notebook and add 2 pages to it
    sg_textNotebook = gtk_notebook_new();
    gtk_notebook_set_tab_pos(GTK_NOTEBOOK(sg_textNotebook), GTK_POS_BOTTOM);
    gtk_notebook_set_show_border(GTK_NOTEBOOK(sg_textNotebook), TRUE);
    gtk_widget_show(sg_textNotebook);

    // 1st page
    GtkWidget *tab1_label = gtk_label_new(_("Console"));
    gtk_notebook_append_page(GTK_NOTEBOOK(sg_textNotebook),
                             Create_console_log_text(),
                             tab1_label);
    // 2nd page
    GtkWidget *tab2_label = gtk_label_new(_("not selected"));
    gtk_notebook_append_page(GTK_NOTEBOOK(sg_textNotebook),
                             (sg_itemLogPage = Create_item_log_text()),
                             tab2_label);
    return(sg_textNotebook);
}

void dragged_to_download_list(GtkWidget  *w,
                              GdkDragContext *context,
                              int x,
                              int y,
                              GtkSelectionData *data,
                              unsigned int info,
                              unsigned int time)
{
    /*
    switch(info) {
    case MIME_URL:
      cerr << "MIME URL" << endl;
      break;
    case MIME_TEXT_PLAIN:
      cerr << "TEXT PLAIN" << endl;
      break;
    case MIME_TEXT_HTML:
      cerr << "TEXT HTML" << endl;
      break;
    default:
      cerr << "other" << endl;
      break;;
    }
    */
    // cerr << "in guimain" << (char*)data->data << endl;
    //cerr << GTK_PANED(sg_vpaned)->child1_size << endl;
    if (g_pasteWindow->addURL((char *)data->data, info)) {
        g_pasteWindow->show();
    }
}

static GtkWidget *Create_clist_notebook()
{
    g_listManager = new ListManager();

    return g_listManager->ret_baseNotebook();
}

gboolean File_quit_main(GtkWidget *w, gpointer data)
{
    if (g_appOption->ret_confirm_exit()) {
        g_cDialog->setup(_("Quit program"),
                         _("Are you sure to quit?"),
                         File_quit);
        g_cDialog->set_cancel_button_visible(false);
        g_cDialog->show();
    } else {
        File_quit(NULL, NULL);
    }
    return TRUE;
}

static void Create_progress_bar()
{
    const char **a[51];

    a[0] = progress0_xpm;

    a[1] = progress1_xpm;
    a[2] = progress2_xpm;
    a[3] = progress3_xpm;
    a[4] = progress4_xpm;
    a[5] = progress5_xpm;
    a[6] = progress6_xpm;
    a[7] = progress7_xpm;
    a[8] = progress8_xpm;
    a[9] = progress9_xpm;
    a[10] = progress10_xpm;
    a[11] = progress11_xpm;
    a[12] = progress12_xpm;
    a[13] = progress13_xpm;
    a[14] = progress14_xpm;
    a[15] = progress15_xpm;
    a[16] = progress16_xpm;
    a[17] = progress17_xpm;
    a[18] = progress18_xpm;
    a[19] = progress19_xpm;
    a[20] = progress20_xpm;
    a[21] = progress21_xpm;
    a[22] = progress22_xpm;
    a[23] = progress23_xpm;
    a[24] = progress24_xpm;
    a[25] = progress25_xpm;
    a[26] = progress26_xpm;
    a[27] = progress27_xpm;
    a[28] = progress28_xpm;
    a[29] = progress29_xpm;
    a[30] = progress30_xpm;
    a[31] = progress31_xpm;
    a[32] = progress32_xpm;
    a[33] = progress33_xpm;
    a[34] = progress34_xpm;
    a[35] = progress35_xpm;
    a[36] = progress36_xpm;
    a[37] = progress37_xpm;
    a[38] = progress38_xpm;
    a[39] = progress39_xpm;
    a[40] = progress40_xpm;
    a[41] = progress41_xpm;
    a[42] = progress42_xpm;
    a[43] = progress43_xpm;
    a[44] = progress44_xpm;
    a[45] = progress45_xpm;
    a[46] = progress46_xpm;
    a[47] = progress47_xpm;
    a[48] = progress48_xpm;
    a[49] = progress49_xpm;
    a[50] = progress50_xpm;

    for (int i = 0; i <= 50; ++i) {
        //GdkBitmap *mask;
        sg_progressBar[i] = gdk_pixbuf_new_from_xpm_data((const char **)a[i]);
        //sg_progressBarMask[i] = mask;
    }
}

static gboolean Speed_changed_cb(GtkWidget *w,  GdkEventButton *event,
                                 gpointer data)
{
    GtkAdjustment *adj = gtk_range_get_adjustment(GTK_RANGE(sg_speedScale));
    ListEntry *listentry = g_listManager->ret_Current_listentry();

    Download_change_speed(adj->value, listentry);

    return TRUE;
}

static gboolean SpeedScale_motionNotify_event_cb(GtkWidget *w,
                                                 GdkEventMotion *e,
                                                 gpointer data)
{
    GtkAdjustment *adj = gtk_range_get_adjustment(GTK_RANGE(sg_speedScale));

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(sg_speedScaleSpin), adj->value);
    return TRUE;
}

static void SpeedScaleSpin_changed_event_cb(GtkEditable *w,
                                            gpointer data)
{
    GtkAdjustment *adj = gtk_range_get_adjustment(GTK_RANGE(sg_speedScale));

    //adj->value = itemcell->ret_Options_opt().ret_speed_limit();
    adj->value = gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(sg_speedScaleSpin));
    //itemcell->release_Options_Lock();
    gtk_range_set_adjustment(GTK_RANGE(sg_speedScale), GTK_ADJUSTMENT(adj));

    Speed_changed_cb(sg_speedScale, NULL, NULL);
}

void Set_speed_scale(ItemCell *itemcell)
{
    //ListEntry *listentry = list_manager->ret_Current_listentry();

    //ItemCell* itemcell = (ItemCell*)gtk_clist_get_row_data(GTK_CLIST(dl_clist), rowindex);
    GtkAdjustment *adj = gtk_range_get_adjustment(GTK_RANGE(sg_speedScale));
    itemcell->get_Options_Lock();

    adj->value = itemcell->ret_Options_opt().ret_speed_limit();

    itemcell->release_Options_Lock();

    gtk_signal_handler_block_by_func(GTK_OBJECT(sg_speedScaleSpin),
                                     GTK_SIGNAL_FUNC(SpeedScaleSpin_changed_event_cb),
                                     NULL);

    gtk_range_set_adjustment(GTK_RANGE(sg_speedScale), GTK_ADJUSTMENT(adj));
//  gtk_range_clear_background(GTK_RANGE(sg_speedScale));
//  gtk_range_draw_background(GTK_RANGE(sg_speedScale));

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(sg_speedScaleSpin), adj->value);

    gtk_signal_handler_unblock_by_func(GTK_OBJECT(sg_speedScaleSpin),
                                       GTK_SIGNAL_FUNC(SpeedScaleSpin_changed_event_cb),
                                       NULL);
}

GtkWidget *Create_speed_limitter(GtkWidget *toplevel)
{
    GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(hbox);

    GtkWidget *ebox = gtk_event_box_new();
    gtk_widget_show(ebox);
    gtk_box_pack_start(GTK_BOX(hbox), ebox, FALSE, FALSE, 0);

    GtkStyle *style = gtk_widget_get_style(toplevel);
    GdkBitmap *icon_mask;
    GdkPixmap *icon = gdk_pixmap_create_from_xpm_d(toplevel->window, &icon_mask, &style->bg[GTK_STATE_NORMAL], (char **)speed_xpm);
    GtkWidget *pixmap = gtk_pixmap_new(icon, icon_mask);
    gtk_widget_show(pixmap);
    gtk_container_add(GTK_CONTAINER(ebox), pixmap);

    GtkTooltips *tooltip = gtk_tooltips_new();
    gtk_tooltips_enable(tooltip);
    gtk_tooltips_set_tip(tooltip,
                         ebox,
                         _("Limit download speed (in Kbytes/sec) (0.0: unlimited) "),
                         "");

    GtkObject *adj = gtk_adjustment_new(0,
                                        0,
                                        MAXSPEEDLIMIT,
                                        0.5,
                                        10.0,
                                        0);
    sg_speedScale = gtk_hscale_new(GTK_ADJUSTMENT(adj));
    gtk_scale_set_digits(GTK_SCALE(sg_speedScale), 1);
    //gtk_range_set_update_policy(GTK_RANGE(speed_scale), GTK_UPDATE_DISCONTINUOUS);

    //gtk_scale_set_value_pos(GTK_SCALE(sg_speedScale), GTK_POS_LEFT);
    gtk_scale_set_draw_value(GTK_SCALE(sg_speedScale), FALSE);
    {
        GtkTooltips *tooltip = gtk_tooltips_new();
        gtk_tooltips_enable(tooltip);
        gtk_tooltips_set_tip(tooltip,
                             sg_speedScale,
                             _("Limit download speed (in Kbytes/sec) (0.0: unlimited) "),
                             "");
    }
    gtk_widget_show(sg_speedScale);
    gtk_widget_set_usize(sg_speedScale, 300, -1);
    gtk_box_pack_start(GTK_BOX(hbox), sg_speedScale, FALSE, FALSE, 0);
    g_signal_connect(GTK_OBJECT(sg_speedScale),
                     "button-release-event",
                     GTK_SIGNAL_FUNC(Speed_changed_cb),
                     NULL);
    g_signal_connect(GTK_OBJECT(sg_speedScale),
                     "motion-notify-event",
                     GTK_SIGNAL_FUNC(SpeedScale_motionNotify_event_cb),
                     NULL);
    // spin box for better manipulation of speed limiter
    GtkObject *adjustment = gtk_adjustment_new(0,
                                               0,
                                               MAXSPEEDLIMIT,
                                               0.1,
                                               10,
                                               0);
    sg_speedScaleSpin = gtk_spin_button_new(GTK_ADJUSTMENT(adjustment),
                                            1.0,
                                            0);
    gtk_spin_button_set_digits(GTK_SPIN_BUTTON(sg_speedScaleSpin), 1);
    gtk_widget_set_usize(sg_speedScaleSpin, 60, -1);
    gtk_widget_show(sg_speedScaleSpin);
    gtk_box_pack_start(GTK_BOX(hbox), sg_speedScaleSpin, FALSE, FALSE, 0);

    g_signal_connect(GTK_OBJECT(sg_speedScaleSpin),
                     "changed",
                     GTK_SIGNAL_FUNC(SpeedScaleSpin_changed_event_cb),
                     NULL);

    // summary of download
    GtkWidget *frame = gtk_frame_new(NULL);
    gtk_widget_show(frame);
    gtk_box_pack_start(GTK_BOX(hbox), frame, TRUE, TRUE, 10);
    {
        GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
        gtk_widget_show(hbox);
        gtk_container_add(GTK_CONTAINER(frame), hbox);
        sg_sumInfoDlLabel = gtk_label_new("");
        gtk_label_set_justify(GTK_LABEL(sg_sumInfoDlLabel), GTK_JUSTIFY_LEFT);
        gtk_widget_show(sg_sumInfoDlLabel);
        gtk_box_pack_start(GTK_BOX(hbox), sg_sumInfoDlLabel, TRUE, TRUE, 10);

        sg_sumInfoErrLabel = gtk_label_new("");
        gtk_label_set_justify(GTK_LABEL(sg_sumInfoErrLabel), GTK_JUSTIFY_LEFT);
        gtk_widget_show(sg_sumInfoErrLabel);
        gtk_box_pack_start(GTK_BOX(hbox), sg_sumInfoErrLabel, TRUE, TRUE, 10);
    }
    return hbox;
}

#define FI_MAIN_WIDTH "%Main-Width:"
#define FI_MAIN_HEIGHT "%Main-Height:"
#define FI_PANED_POS "%Paned-Pos:"
#define FI_BASKET_X "%Basket-X:"
#define FI_BASKET_Y "%Basket-Y:"

extern ItemList *g_itemList;

bool save_gui_info(const string &filename)
{
    string filenameTemp = filename + ".temporary.working";

    ofstream outfile(filenameTemp.c_str(), ios::out);
    if (outfile.bad()) return false;

    outfile << FI_MAIN_WIDTH << ' ' << g_toplevel->allocation.width << endl;
    outfile << FI_MAIN_HEIGHT << ' ' << g_toplevel->allocation.height << endl;
    outfile << FI_PANED_POS << ' ' << GTK_PANED(sg_vpaned)->child1_size << endl;

    int basket_x = 0;
    int basket_y = 0;
    basket->getGeometry(basket_x, basket_y);
    if (basket_x < 0) basket_x = 0;
    if (basket_y < 0) basket_y = 0;
    outfile << FI_BASKET_X << ' ' << basket_x << endl;
    outfile << FI_BASKET_Y << ' ' << basket_y << endl;

    if (outfile.bad() || outfile.fail()) {
        return false;
    }
    if (rename(filenameTemp.c_str(), filename.c_str()) < 0) {
        return false;
    }

    return true;
}

bool restore_gui_info(const string &filename,
                      int &main_width, int &main_height, int &paned_pos,
                      int &basket_x, int &basket_y)
{
    /*
    main_width = 600;
    main_height = 800;
    paned_pos = 500;
    */
    ifstream infile(filename.c_str(), ios::in);
    map<string, string> guiInfoMap;
    if (infile.bad()) return false;

    if (!infile.good()) return false;
    while (infile.good()) {
        string line;
        getline(infile, line, '\n');
        string key = Token_splitter(line, " \t");
        guiInfoMap[key] = line;
    }

    int intTemp;
    if ((intTemp = stoi(guiInfoMap[FI_MAIN_WIDTH].c_str())) >= 0) {
        main_width = intTemp;
    }
    if ((intTemp = stoi(guiInfoMap[FI_MAIN_HEIGHT].c_str())) >= 0) {
        main_height = intTemp;
    }
    if ((intTemp = stoi(guiInfoMap[FI_PANED_POS].c_str())) >= 0) {
        paned_pos = intTemp;
    }

    // location of basket
    if ((intTemp = stoi(guiInfoMap[FI_BASKET_X])) >= 0) {
        basket_x = intTemp;
    }
    if ((intTemp = stoi(guiInfoMap[FI_BASKET_Y])) >= 0) {
        basket_y = intTemp;
    }

    // adjust paned height
    if (paned_pos > main_height) {
        paned_pos = main_height / 2;
    }

    return true;
}

static void window_focusInEvent_cb(GtkWidget *w,
                                   GdkEventFocus *event,
                                   gpointer data)
{
    //cerr << "expose event" << endl;
    basket->setMainWindowVisibleFlag(true);
}

/*
static void test2_func(GtkWidget *w,
		GdkEventFocus *event,
		gpointer data) {
  //cerr << "focus out event" << endl;
    if(basket->hasFocus()) {
      cerr << "true" << endl;
    }
    basket->setMainWindowVisibleFlag(false);
}
*/

static gboolean socketPoolRefresh(gpointer data)
{
    Socket::refreshSocketPool();
    return TRUE;
}

GtkWidget *GUI_main(int main_width, int main_height, int main_x, int main_y)
{
    GtkWidget *window;
    GtkWidget *vbox;

    int paned_pos;
    int main_width_new;
    int main_height_new;
    int basket_x;
    int basket_y;
    bool retval = restore_gui_info(g_itemList->ret_file_gui_info(),
                                   main_width_new, main_height_new, paned_pos,
                                   basket_x, basket_y);

    if (main_width != 0 && main_height != 0) {
        paned_pos = main_height / 2;
        if (!retval) {
            basket_x = 0;
            basket_y = 0;
        }
    } else if (retval) {
        main_width = main_width_new;
        main_height = main_height_new;
    } else {
        main_width = MWWIDTH;
        main_height = MWHEIGHT;
        paned_pos = main_height / 2;
        basket_x = 0;
        basket_y = 0;
    }

    // limit offset
    int rootX, rootY;
    gdk_window_get_size(GDK_ROOT_PARENT(), &rootX, &rootY);

    if (main_x >= rootX) main_x = 0;
    if (main_y >= rootY) main_y = 0;
    if (basket_x >= rootX) basket_x = 0;
    if (basket_y >= rootY) basket_y = 0;

    /*
    if(main_width == 0 || main_height == 0) {
      main_width = MWWIDTH;
      main_height = MWHEIGHT;

      if(!restore_gui_info(g_itemList->ret_file_gui_info(),
    		 main_width, main_height, paned_pos)) {
        main_width = MWWIDTH;
        main_height = MWHEIGHT;
        paned_pos = main_height/2;
      }
    } else {
      paned_pos = main_height/2;
    }
    */
    //トップレベルのウインドウの作成
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_toplevel = window;
    gtk_widget_set_name(window, "window");
    //set window size
    gtk_widget_set_usize(GTK_WIDGET(window), 320, 200);
    gtk_window_set_default_size(GTK_WINDOW(window), main_width, main_height);
    gtk_widget_set_uposition(window, main_x, main_y);
    //set window title

    gtk_window_set_title(GTK_WINDOW(window), ARIA_VERSION);

    g_signal_connect(GTK_OBJECT(window),
                     "delete_event",
                     GTK_SIGNAL_FUNC(File_quit_main),
                     NULL);

    g_signal_connect(GTK_OBJECT(window),
                     "focus-in-event",
                     GTK_SIGNAL_FUNC(window_focusInEvent_cb),
                     NULL);
    /*
    g_signal_connect(GTK_OBJECT(appWindow),
    	     "focus-out-event",
    	     GTK_SIGNAL_FUNC(test2_func),
    	     NULL);
    */
    gtk_widget_realize(window);

    // create vbox widget
    vbox = gtk_vbox_new(FALSE, 5);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    gtk_widget_show(vbox);

    // create menu bar
    gtk_box_pack_start(GTK_BOX(vbox), Create_menu_bar(GTK_WINDOW(window)), FALSE, TRUE, 0);

    // create tool bar
    GtkWidget *handlebox = gtk_handle_box_new ();
    gtk_widget_show(handlebox);
    gtk_box_pack_start(GTK_BOX(vbox), handlebox, FALSE, TRUE, 0);
    gtk_container_add(GTK_CONTAINER(handlebox), Create_toolbar(window));

    // create sg_vpaned widget
    sg_vpaned = gtk_vpaned_new();
    gtk_container_add(GTK_CONTAINER(vbox), sg_vpaned);
    //gtk_paned_set_handle_size(GTK_PANED(sg_vpaned), 10);
    //gtk_paned_set_gutter_size(GTK_PANED(sg_vpaned), 12);
    gtk_paned_set_position(GTK_PANED(sg_vpaned), paned_pos);
    gtk_widget_show(sg_vpaned);

    // create itemlist
    GtkWidget *lvbox = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(lvbox);
    gtk_box_pack_start(GTK_BOX(lvbox), Create_speed_limitter(window), FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(lvbox), Create_clist_notebook(), TRUE, TRUE, 0);
    gtk_paned_add1(GTK_PANED(sg_vpaned), lvbox);

    // create notebook and add 2 pages to it
    gtk_paned_add2(GTK_PANED(sg_vpaned), Create_text_notebook());

    GtkWidget *vsep = gtk_hseparator_new();
    gtk_widget_show(vsep);
    gtk_box_pack_start(GTK_BOX(vbox), vsep, false, false, 0);

    // dialogs commonly used
    g_cDialog = new Dialog(GTK_WINDOW(window));
    g_cFileBrowser = new FileBrowser(GTK_WINDOW(window));

    // show window

    Create_popup_menu(window);

    gtk_widget_show(window);

    Set_sensitive__no_item_selected();// fix this

    // create dummy clist and get style info from it
    GtkWidget *dummy_clist = gtk_clist_new(1);
    GtkStyle *style = gtk_widget_get_style(dummy_clist);
    sg_dlCListStyle = gtk_style_copy(style); // dl_clist_style is a global variable
    gtk_widget_destroy(dummy_clist);

    Create_progress_bar();

    basket = new Basket(window);
    basket->setMainWindowVisibleFlag(true);
    basket->setGeometry(basket_x, basket_y);

    // watch g_pipetogui[0] for incoming message
    gdk_input_add(g_pipetogui[0], GDK_INPUT_READ, Update_gui, NULL);

    // refresh socket pool periodically
    gtk_timeout_add(60000,
                    GtkFunction(socketPoolRefresh),
                    NULL);

    return window;
}
