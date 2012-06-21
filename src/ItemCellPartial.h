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

// $Id: ItemCellPartial.h,v 1.1 2001/05/04 12:19:56 tujikawa Exp $

#ifndef _ITEMCELLPARTIAL_H_
#define _ITEMCELLPARTIAL_H_
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include "aria.h"
#include "utils.h"
#include "ItemCell.h"

class ItemCellPartial : public ItemCell {
private:
  unsigned int start_range;
  unsigned int end_range;
  unsigned int order;
  unsigned int total_size;
  //string filename_partial;
  ItemCell *boss;
public:
  enum SplitStatusType {
    PARTIAL_NORMAL,
    PARTIAL_NOINDEX,
    PARTIAL_CHANGED
  };

  ItemCellPartial(const string& url,
		   const URLcontainer& urlcon,
		   const Options& options_ptr,
		   const string& initial_log,
		   ItemCell *itemcell_parent,
		   unsigned int order,
		   unsigned int start_pos,
		   unsigned int end_pos
		   );
  virtual ~ItemCellPartial();
  /*
  string             ret_Filename() const;
  string             ret_Filename_opt() const;
  */
  void               set_Filename(const string& filename);
  void               set_Filename_opt(const string& filename);

  bool               Is_Partial() const;
  unsigned int       ret_Order() const;
  ItemCell::DownloadStatusType Post_process();
  unsigned int       ret_Start_range() const;
  unsigned int       ret_End_range() const;
  ItemCell *ret_Boss();

  void WriteSplitInfo(const string& filename);
  ItemCell::DownloadStatusType ItemCellPartial::Download_Main();

  SplitStatusType SplitNumberChanged(const string& filename);
};
#endif // _ITEMCELLPARTIAL_H_
