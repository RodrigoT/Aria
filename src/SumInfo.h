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

// $Id: SumInfo.h,v 1.2 2001/09/11 13:13:32 tujikawa Exp $

#ifndef _SUMINFO_H_
#define _SUMINFO_H_
#include <iostream>
#include "aria.h"

using namespace std;

class SumInfo
{
private:
  int n_download;
  int n_error;
  int n_stop;
  int n_ready;
  int n_split;
  int n_locked;
  int n_complete;
  pthread_mutex_t si_lock;
public:
  SumInfo();
  ~SumInfo();

  void all_reset();

  void inc_download();
  void dec_download();
  void set_diff_download(int diff);
  int ret_download();

  void inc_error();
  void dec_error();
  void set_diff_error(int diff);
  int ret_error();

  void inc_stop();
  void dec_stop();
  void set_diff_stop(int diff);
  int ret_stop();

  void inc_ready();
  void dec_ready();
  void set_diff_ready(int diff);
  int ret_ready();

  void inc_split();
  void dec_split();
  void set_diff_split(int diff);
  int ret_split();

  void inc_locked();
  void dec_locked();
  void set_diff_locked(int diff);
  int ret_locked();

  void inc_complete();
  void dec_complete();
  void set_diff_complete(int diff);
  int ret_complete();

  void print_sum() const;
};
#endif // _SUMINFO_H_
