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

// $Id: Basket.cc,v 1.13 2001/11/28 15:28:38 tujikawa Exp $

#include "Basket.h"
#include <iostream>
#include "PasteWindow.h"
#include "ItemOption.h"
#include "AppOption.h"

extern PasteWindow *g_pasteWindow;
extern AppOption *g_appOption;
extern ItemOption *g_itemOption;

extern bool Create_new_item(string url, bool onPaste = false, const string& dir = "", const string& referer = "");

static bool inMotionFlag = false;
static int inWinX, inWinY;

static void dnd_receive_event_cb(GtkWidget  *w,
				 GdkDragContext *context,
				 int x,
				 int y,
				 GtkSelectionData *data,
				 unsigned int info,
				 unsigned int time) {
  if(g_appOption->isDirectPastingFromBasketEnabled()) {
    Create_new_item((char *)data->data);
  } else if(g_pasteWindow->addURL((char *)data->data, false)) {
    g_pasteWindow->show();
  }
}

static gboolean hide_cb(Basket *basket)
{
  basket->hide();

  return TRUE;
}

static gboolean motion_notify_event_cb(GtkWidget *w,
				       GdkEventMotion *event,
				       Basket *basket) {
  if(inMotionFlag) {
    int ptrX, ptrY;
    GdkModifierType modmask;
    gdk_window_get_pointer((GdkWindow *)NULL,
			   &ptrX,
			   &ptrY,
			   &modmask);
    int X = ptrX-inWinX;
    int Y = ptrY-inWinY;
    basket->setGeometry(X, Y);
    
    gdk_window_move(w->window, X, Y);
  }
  return TRUE;
}


static void WINDOW_RAISE(GtkWidget *window) {
    gdk_window_raise(window->window);
    gdk_window_show(window->window);
}

static gboolean button_press_event_cb(GtkWidget *w,
				      GdkEventButton *event,
				      Basket *basket) {
  GtkWidget *toplevel = basket->getMainWindow();
  if(event->type == GDK_2BUTTON_PRESS) {
    if(basket->isMainWindowVisible()) {
      // gdk(gtk?) 2.0 has usuful function gdk_widget_iconify() :)
      basket->setMainWindowVisibleFlag(false);
      Display *display = GDK_WINDOW_XDISPLAY(toplevel->window);
      XIconifyWindow(display, GDK_WINDOW_XWINDOW(toplevel->window), DefaultScreen (display));
    } else {
      basket->setMainWindowVisibleFlag(true);

      WINDOW_RAISE(toplevel);
      if(g_pasteWindow->isVisible()) {
	WINDOW_RAISE(g_pasteWindow->getWindow());
      }
      // this don't work because its window is modal. fix this
      if(g_itemOption->isVisible()) {
	WINDOW_RAISE(g_itemOption->getWindow());
      }
    }
  } else if(event->button == 3 && event->state & GDK_SHIFT_MASK) {
    basket->hide();
    basket->toggleTitleBar();
    basket->show();
  } else {
    inMotionFlag = true;
    int rootX, rootY;
    gdk_window_get_position(w->window, &rootX, &rootY);
    int ptrX, ptrY;
    GdkModifierType modmask;
    gdk_window_get_pointer((GdkWindow *)NULL,
			   &ptrX,
			   &ptrY,
			   &modmask);
    inWinX = ptrX-rootX;
    inWinY = ptrY-rootY;
  }
  return TRUE;
}

static gboolean button_release_event_cb(GtkWidget *w,
					GdkEventButton *event,
					gpointer data) {
  inMotionFlag = false;

  return TRUE;
}

Basket::Basket(GtkWidget *toplevel_in) {
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_wmclass(GTK_WINDOW(window), "ariaBasket", "AriaBasket");
  // set the default size of window
  gtk_window_set_default_size(GTK_WINDOW(window), SIZE_X, SIZE_Y);
  gtk_widget_set_events(window,
			gtk_widget_get_events(window) |
			GDK_POINTER_MOTION_MASK |
			GDK_BUTTON_PRESS_MASK |
			GDK_BUTTON_RELEASE_MASK);

  basketVBox = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(window), basketVBox);
  gtk_widget_show(basketVBox);
  gtk_widget_set_usize(basketVBox, 0, 0);

  label = gtk_label_new("Aria's Basket");
  gtk_widget_set_usize(label, SIZE_X, SIZE_Y);
  gtk_box_pack_start(GTK_BOX(basketVBox), label, FALSE, FALSE, 0);
  toplevel = toplevel_in;
  gtk_widget_show(label);

  // set call back functions
  gtk_signal_connect_object(GTK_OBJECT(window),
			    "delete_event",
			    GTK_SIGNAL_FUNC(hide_cb),
			    (GtkObject *)this);
  gtk_signal_connect(GTK_OBJECT(window),
		     "motion_notify_event",
		     GTK_SIGNAL_FUNC(motion_notify_event_cb),
		     (void *)this);
  gtk_signal_connect(GTK_OBJECT(window),
		     "button_press_event",
		     GTK_SIGNAL_FUNC(button_press_event_cb),
		     (void *)this);
  gtk_signal_connect(GTK_OBJECT(window),
		     "button_release_event",
		     GTK_SIGNAL_FUNC(button_release_event_cb),
		     NULL);
  gtk_signal_connect(GTK_OBJECT(window),
		     "focus-out-event",
		     GTK_SIGNAL_FUNC(button_release_event_cb),
		     NULL);
  pixmap = NULL;

  Setup_dnd(window, dnd_receive_event_cb);
  visibleFlag = false;
  titleVisibleFlag = false;

  gtk_widget_realize(window);

  gdk_window_set_decorations(window->window, (GdkWMDecoration)0);
  //gdk_window_set_title(window->window, "POPUP");
  //gdk_window_move(window->window, 0, 0);

  //gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(toplevel));

  // initialize geometry information
  geometry.x = 0;
  geometry.y = 0;
}

Basket::~Basket() {
  gtk_widget_destroy(window);
}

void Basket::toggleTitleBar() {
//    gtk_widget_unrealize(window);
//    gtk_widget_realize(window);
  if(titleVisibleFlag) {
    titleVisibleFlag = false;
    gdk_window_set_decorations(window->window, (GdkWMDecoration)0);
  } else {
    titleVisibleFlag = true;
    gdk_window_set_decorations(window->window, GDK_DECOR_TITLE);
  }
}

void Basket::show() {
  visibleFlag = true;

  int rootX, rootY;
  gdk_window_get_size(GDK_ROOT_PARENT(), &rootX, &rootY);

  if(geometry.x >= rootX
     || geometry.x < 0) {
    geometry.x = 0;
  }
  if(geometry.y >= rootY
     || geometry.y < 0) {
    geometry.y = 0;
  }

  gtk_widget_set_uposition(window, geometry.x, geometry.y);
  gtk_widget_show(window);

  XEvent xev;
  xev.type = ClientMessage;
  xev.xclient.window = GDK_WINDOW_XWINDOW(window->window);
  xev.xclient.message_type = XInternAtom(GDK_DISPLAY(), "_WIN_LAYER", false);
  xev.xclient.format = 32;
  xev.xclient.data.l[0] = 6;
  XSendEvent(GDK_DISPLAY(), GDK_ROOT_WINDOW(), false,
	     SubstructureNotifyMask, (XEvent *)&xev);

  //gdk_window_move(window->window, geometry.x, geometry.y);


}

void Basket::hide() {
  visibleFlag = false;
  gtk_widget_hide(window);
}

bool Basket::isVisible() {
  return visibleFlag;
}

void Basket::setMainWindowVisibleFlag(bool flag) {
  mainWindowVisibleFlag = flag;
}

bool Basket::isMainWindowVisible() const {
  return mainWindowVisibleFlag;
}

GtkWidget *Basket::getMainWindow() {
  return toplevel;
}

bool Basket::hasFocus() {
  return GTK_WIDGET_HAS_FOCUS(window);
}

void Basket::setGeometry(int x, int y)
{
  geometry.x = x;
  geometry.y = y;
}

void Basket::getGeometry(int& x, int& y)
{
  x = geometry.x;
  y = geometry.y;
}

void Basket::setPixmap(GdkPixbuf *basketPixmap, GdkBitmap *basketBitmap)
{
  if(basketPixmap != NULL &&
     basketBitmap != NULL) {
    if(pixmap == NULL) {
      gtk_widget_hide(label);
//      pixmap = gtk_pixmap_new(basketPixmap, basketBitmap);
	  pixmap = gtk_image_new_from_pixbuf(basketPixmap);
      gtk_box_pack_start(GTK_BOX(basketVBox), pixmap, FALSE, FALSE, 0);
      gtk_widget_shape_combine_mask(window, basketBitmap, 0, 0);
      gtk_widget_show(pixmap);
    } else {
      gtk_widget_hide(pixmap);
      gtk_widget_destroy(pixmap);
      pixmap = gtk_image_new_from_pixbuf(basketPixmap);
      gtk_box_pack_start(GTK_BOX(basketVBox), pixmap, FALSE, FALSE, 0);
      gtk_widget_shape_combine_mask(window, basketBitmap, 0, 0);
      gtk_widget_show(pixmap);
    }
    gtk_window_set_default_size(GTK_WINDOW(window), pixmap->requisition.width, pixmap->requisition.height);
    gdk_window_resize(window->window,
		      pixmap->requisition.width,
		      pixmap->requisition.height);
  } else {
    if(pixmap != NULL) {
      gtk_widget_hide(pixmap);
      gtk_widget_destroy(pixmap);
      pixmap = NULL;
      gtk_widget_show(label);
      gtk_widget_shape_combine_mask(window, NULL, 0, 0);
      gdk_window_resize(basketVBox->window,
			SIZE_X,
			SIZE_Y);
      gdk_window_resize(window->window,
			SIZE_X,
			SIZE_Y);

      gtk_window_set_default_size(GTK_WINDOW(window), SIZE_X, SIZE_Y);
    }
  }
  if(geometry.x < 0) {
    geometry.x = 0;
  }
  if(geometry.y < 0) {
    geometry.y = 0;
  }
  gdk_window_move(window->window, geometry.x, geometry.y);
}
