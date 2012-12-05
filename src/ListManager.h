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

// $Id: ListManager.h,v 1.8 2001/10/12 16:47:38 tujikawa Exp $

#ifndef _LISTMANAGER_H_
#define _LISTMANAGER_H_

#include <list>
#include "ListEntry.h"
#include "aria.h"
using namespace std;

class ListManager
{
private:
    list<ListEntry *> listentry_list;
    GtkWidget *baseNotebook;
    GdkPixbuf *statusIcon[ICON_TOTAL];
    GdkBitmap *statusIconMask[ICON_TOTAL];
public:
    ListManager();
    ~ListManager();

    GtkWidget *ret_baseNotebook();
    void Register(ListEntry *listentry);
    void Delete(int page_num);
    void DeleteCurrentList();
    bool Search(ListEntry *listentry);
    ListEntry *ret_Current_listentry();
    ListEntry *ret_nth_listentry(int page_num);
    ListEntry *getListEntryByName(string name);
    const list<ListEntry *> &ret_Listentry_list() const;
    list<string> getListNames() const;
    string getNewDefaultListName();
    bool checkDuplicatedName(string name);
    int ret_Length() const;
    int getPage(ListEntry *listentry);
    void showPage(ListEntry *listentry);
    void showPage(int page_num);
    void Move_left();
    void Move_right();
    bool Set_active_page();

    enum ListMoveType {
        LIST_MOVE_LEFT = -1,
        LIST_MOVE_RIGHT = 1
    };

    void Move_sub(ListMoveType mvtype);
    void Swap(int page_num1, int page_num2);
    void setStatusIcon(GdkPixbuf *pixmap[], GdkBitmap *bitmap[]);
};
#endif // _LISTMANAGER_H_
