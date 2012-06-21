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

// $Id: ShortCutKey.h,v 1.9 2001/11/04 10:18:08 tujikawa Exp $

// File menu
#define SC_OPENURL 'L'
#define SCM_OPENURL GDK_CONTROL_MASK

#define SC_OPENCRC 'R'
#define SCM_OPENCRC GDK_CONTROL_MASK

#define SC_OPEN 'O'
#define SCM_OPEN GDK_CONTROL_MASK

#define SC_SAVE 'S'
#define SCM_SAVE GDK_CONTROL_MASK

#define SC_FINDHREF 'E'
#define SCM_FINDHREF GDK_CONTROL_MASK

#define SC_QUIT 'Q'
#define SCM_QUIT GDK_CONTROL_MASK

// Edit menu
#define SC_SELECT_ALL 'A'
#define SCM_SELECT_ALL GDK_CONTROL_MASK

#define SC_PASTEURL 'V'
#define SCM_PASTEURL GDK_CONTROL_MASK

#define SC_PASTEURLEXP 'B'
#define SCM_PASTEURLEXP GDK_CONTROL_MASK

#define SC_PASTECRC 'G'
#define SCM_PASTECRC GDK_CONTROL_MASK

#define SC_COPYITEM 'C'
#define SCM_COPYITEM GDK_MOD1_MASK

#define SC_CUTITEM 'X'
#define SCM_CUTITEM GDK_MOD1_MASK

#define SC_PASTEITEM 'V'
#define SCM_PASTEITEM GDK_MOD1_MASK

#define SC_SEARCH 'F'
#define SCM_SEARCH GDK_CONTROL_MASK

// Item menu
#define SC_NEWITEM 'N'
#define SCM_NEWITEM GDK_CONTROL_MASK

#define SC_LOCK 'L'
#define SCM_LOCK GDK_MOD1_MASK

#define SC_UNLOCK 'U'
#define SCM_UNLOCK GDK_MOD1_MASK

#define SC_MOVEUP '['
#define SCM_MOVEUP GDK_MOD1_MASK

#define SC_MOVEDOWN ']'
#define SCM_MOVEDOWN GDK_MOD1_MASK

#define SC_MOVETOP ','
#define SCM_MOVETOP GDK_MOD1_MASK

#define SC_MOVEBOTTOM '.'
#define SCM_MOVEBOTTOM GDK_MOD1_MASK

#define SC_LOCKERROR 'E'
#define SCM_LOCKERROR GDK_MOD1_MASK

#define SC_CLEARDOWNLOADED 'B'
#define SCM_CLEARDOWNLOADED GDK_MOD1_MASK

// List menu
#define SC_NEWLIST 'T'
#define SCM_NEWLIST GDK_CONTROL_MASK

#define SC_DELLIST 'W'
#define SCM_DELLIST GDK_CONTROL_MASK

#define SC_RENAMELIST GDK_F4
#define SCM_RENAMELIST 0

#define SC_MOVELEFT '['
#define SCM_MOVELEFT GDK_CONTROL_MASK

#define SC_MOVERIGHT ']'
#define SCM_MOVERIGHT GDK_CONTROL_MASK

#define SC_PREVIOUSLIST GDK_F2
#define SCM_PREVIOUSLIST 0

#define SC_NEXTLIST GDK_F3
#define SCM_NEXTLIST 0

#define SC_SHUFFLELIST GDK_F5
#define SCM_SHUFFLELIST 0
// Download menu
#define SC_START 'S'
#define SCM_START GDK_MOD1_MASK

#define SC_STOP 'P'
#define SCM_STOP GDK_MOD1_MASK

#define SC_DOWNLOADAGAIN 'T'
#define SCM_DOWNLOADAGAIN GDK_MOD1_MASK

#define SC_CLEAR 'D'
#define SCM_CLEAR GDK_MOD1_MASK

#define SC_CLEARWITHFILE 'D'
#define SCM_CLEARWITHFILE  GDK_MOD1_MASK|GDK_SHIFT_MASK

#define SC_START_ALL 'A'
#define SCM_START_ALL GDK_MOD1_MASK

#define SC_STOP_ALL 'W'
#define SCM_STOP_ALL GDK_MOD1_MASK

#define SC_CLEAR_ALL 'X'
#define SCM_CLEAR_ALL GDK_MOD1_MASK|GDK_CONTROL_MASK

#define SC_START_ALL_LIST 'A'
#define SCM_START_ALL_LIST GDK_MOD1_MASK|GDK_SHIFT_MASK

#define SC_STOP_ALL_LIST 'W'
#define SCM_STOP_ALL_LIST GDK_MOD1_MASK|GDK_SHIFT_MASK

#define SC_CLEAR_ALL_LIST 'X'
#define SCM_CLEAR_ALL_LIST GDK_MOD1_MASK|GDK_SHIFT_MASK

// Option menu
#define SC_ITEMOPTION 'O'
#define SCM_ITEMOPTION GDK_MOD1_MASK

#define SC_ITEMOPTION_DEFAULT_LIST 'Z'
#define SCM_ITEMOPTION_DEFAULT_LIST GDK_MOD1_MASK

#define SC_APPOPTION 'Y'
#define SCM_APPOPTION GDK_MOD1_MASK

#define SC_SETDEFAULT 'J'
#define SCM_SETDEFAULT GDK_MOD1_MASK

#define SC_SETDEFAULT_NOSAVEDIR 'K'
#define SCM_SETDEFAULT_NOSAVEDIR GDK_MOD1_MASK

#define SC_SETDEFAULT_ALL 'G'
#define SCM_SETDEFAULT_ALL GDK_MOD1_MASK

#define SC_SETDEFAULT_ALL_NOSAVEDIR 'H'
#define SCM_SETDEFAULT_ALL_NOSAVEDIR GDK_MOD1_MASK

#define SC_SHOWBASKET 'C'
#define SCM_SHOWBASKET GDK_CONTROL_MASK

#define SC_HISTORY 'I'
#define SCM_HISTORY GDK_CONTROL_MASK

// help menu
#define SC_ABOUT 'H'
#define SCM_ABOUT GDK_CONTROL_MASK
