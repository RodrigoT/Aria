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

// $Id: Basket.h,v 1.5 2001/10/12 16:47:38 tujikawa Exp $

#ifndef _BASKET_H_
#define _BASKET_H_

#include "gui_utils.h"
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <X11/Xlib.h>

class Basket {
private:
  enum BasketSize {
    SIZE_X = 100,
    SIZE_Y = 50
  };
  GtkWidget *window; // base widget
  GtkWidget *toplevel; // main window of Aria
  GtkWidget *basketVBox;
  GtkWidget *pixmap;
  GtkWidget *label;

  bool visibleFlag;
  bool mainWindowVisibleFlag;
  bool titleVisibleFlag;

  struct Geometry {
    int x;
    int y;
  } geometry;

public:
  Basket(GtkWidget *toplevel); // create basket window
  ~Basket(); // destroy basket window

  void setGeometry(int x, int y);
  void getGeometry(int& x, int& y);
  void setPixmap(GdkPixmap *basketPixmap, GdkBitmap *basketBitmap);
  void show(); // show basket window
  void hide(); // hide basket window
  bool isVisible();
  void setMainWindowVisibleFlag(bool flag);
  bool isMainWindowVisible() const;
  GtkWidget *getMainWindow();
  void toggleTitleBar();
  bool hasFocus();
};
#endif // _BASKET_H_
