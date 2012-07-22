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

// $Id: aria.h,v 1.25 2001/10/01 12:45:26 tujikawa Exp $

#ifndef _ARIA_H_
#define _ARIA_H_

#include <stdio.h>
#include <gtk/gtk.h>
#include <gtk/gtk.h>
#include <string.h>
#include <pthread.h>
#include "config.h"

#define RETRYWAIT 10 // wait between retries(sec)
#define MAXTHREAD 30 // maximum number of threads
#define THREADLIMIT 10
#define MAXSPLIT 10
#define SPLITLIMIT 30
#define MAXRETRY 100
#define MAXRETRYPB 100
#define MAXRETRYINTERVAL 120
#define MAXTIMEOUT 120
#define MAXRECURSION 100
#define MAXAUTOSAVEINTERVAL 60
#define MAXLIST 100
#define MAXSPEEDLIMIT 200
#define DEFAULTSPEEDLIMIT 80
#define MAXHISTORY 10000
#define DEFAULTHISTORY 1000
#define MWWIDTH 700
#define MWHEIGHT 480

// definition for the columns of clist
// TODO: To enumerate inside ListEntry.h !
#define COL_ICON 0
#define COL_FILENAME 1
#define COL_CURSIZE 2
#define COL_TOTSIZE 3
#define COL_PROGRESS 4
#define COL_SPEED 5
#define COL_RTIME 6
#define COL_RETRY 7
#define COL_REC 8
#define COL_STATUS 9
#define COL_CRC 10
#define COL_MD5 11
#define COL_SAVE 12
#define COL_URL 13
#define TOTALCOL 14

#ifdef ENABLE_NLS
#  include <libintl.h>
#  define _(STRING) gettext(STRING)
#else
#  define _(STRING) STRING
#endif

#define DEFAULT_LIST_NAME _("New list")

// status icon (penguin like creature, you know)
enum {
  ICON_READY,
  ICON_STOP,
  ICON_COMPLETE,
  ICON_DOWNLOAD,
  ICON_ERROR,
  ICON_DIVIDE,
  ICON_LOCK,
  ICON_TOTAL
};

#endif // _ARIA_H_
