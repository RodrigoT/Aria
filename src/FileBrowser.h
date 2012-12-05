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

// $Id: FileBrowser.h,v 1.3 2001/03/09 06:49:09 tujikawa Exp $

#ifndef _FILEBROWSER_H_
#define _FILEBROWSER_H_
#include <gtk/gtk.h>
#include <string>
#include "aria.h"
using namespace std;

class FileBrowser
{
private:
    GtkWidget *fs;
    GtkWindow *toplevel;

    unsigned int Ok_connection_id;
    unsigned int Cancel_connection_id;
public:
    FileBrowser(GtkWindow *toplevel_in);
    ~FileBrowser();

    void setup(const string &title_in,
               gboolean (*ok_SignalFunc)(GtkWidget *w, GtkWidget *fs),
               gboolean (*cancel_SignalFunc)(GtkWidget *w, GtkWidget *fs) = NULL);
    void set_ok_button_visible(bool isvisible);
    void set_cancel_button_visible(bool isvisible);
    void set_transient(GtkWindow *parent);
    void set_default_path(const string &path);
    void show();
    void hide();
};
#endif //_FILEBROWSER_H_
