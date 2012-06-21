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

// $Id: SumInfo.cc,v 1.1 2001/04/24 16:08:19 tujikawa Exp $

#include "SumInfo.h"

SumInfo::SumInfo()
{
  pthread_mutex_init(&si_lock, NULL);
  all_reset();
}

SumInfo::~SumInfo()
{
}


void SumInfo::all_reset()
{
  pthread_mutex_lock(&si_lock);
  n_download = 0;
  n_stop = 0;
  n_ready = 0;
  n_split = 0;
  n_locked = 0;
  n_complete = 0;
  n_error = 0;
  pthread_mutex_unlock(&si_lock);
}

void SumInfo::inc_download()
{
  pthread_mutex_lock(&si_lock);
  ++n_download;
  pthread_mutex_unlock(&si_lock);
}

void SumInfo::dec_download()
{
  pthread_mutex_lock(&si_lock);
  if(n_download > 0) --n_download;
  pthread_mutex_unlock(&si_lock);
}

void SumInfo::set_diff_download(int diff)
{
  pthread_mutex_lock(&si_lock);
  n_download += diff;
  if(n_download < 0) n_download = 0;
  pthread_mutex_unlock(&si_lock);
}

int SumInfo::ret_download()
{
  pthread_mutex_lock(&si_lock);
  int retval = n_download;
  pthread_mutex_unlock(&si_lock);
  return retval;
}

void SumInfo::inc_error()
{
  pthread_mutex_lock(&si_lock);
  ++n_error;
  pthread_mutex_unlock(&si_lock);
}

void SumInfo::dec_error()
{
  pthread_mutex_lock(&si_lock);
  if(n_error > 0) --n_error;
  pthread_mutex_unlock(&si_lock);
}

void SumInfo::set_diff_error(int diff)
{
  pthread_mutex_lock(&si_lock);
  n_error += diff;
  if(n_error < 0) n_error = 0;
  pthread_mutex_unlock(&si_lock);
}

int SumInfo::ret_error()
{
  pthread_mutex_lock(&si_lock);
  int retval = n_error;
  pthread_mutex_unlock(&si_lock);
  return retval;

}

void SumInfo::inc_stop()
{
  ++n_stop;
}

void SumInfo::dec_stop()
{
  if(n_stop > 0) --n_stop;
}

void SumInfo::set_diff_stop(int diff)
{
  n_stop += diff;
  if(n_stop < 0) n_stop = 0;
}

int SumInfo::ret_stop()
{
  return n_stop;
}

void SumInfo::inc_ready()
{
  ++n_ready;
}

void SumInfo::dec_ready()
{
  if(n_ready > 0) --n_ready;
}

void SumInfo::set_diff_ready(int diff)
{
  n_ready += diff;
  if(n_ready < 0) n_ready = 0;
}

int SumInfo::ret_ready()
{
  return n_ready;
}

void SumInfo::inc_split()
{
  ++n_split;
}

void SumInfo::dec_split()
{
  if(n_split > 0) --n_split;
}

void SumInfo::set_diff_split(int diff)
{
  n_split += diff;
  if(n_split < 0) n_split = 0;
}

int SumInfo::ret_split()
{
  return n_split;
}

void SumInfo::inc_locked()
{
  ++n_locked;
}

void SumInfo::dec_locked()
{
  if(n_locked > 0) --n_locked;
}

void SumInfo::set_diff_locked(int diff)
{
  n_locked += diff;
  if(n_locked < 0) n_locked = 0;
}

int SumInfo::ret_locked()
{
  return n_locked;
}

void SumInfo::inc_complete()
{
  ++n_complete;
}

void SumInfo::dec_complete()
{
  if(n_complete > 0) --n_complete;
}

void SumInfo::set_diff_complete(int diff)
{
  n_complete += diff;
  if(n_complete < 0) n_complete = 0;
}

int SumInfo::ret_complete()
{
  return n_complete;
}

void SumInfo::print_sum() const
{
  cerr << "download: " << n_download << '\n';
  cerr << "error:    " << n_error << '\n';
  //cerr << "stop:     " << n_stop << '\n';
  //cerr << "complete: " << n_compelete << '\n';
  //cerr << "ready:    " << n_ready << '\n';
  //cerr << "split:    " << n_split << '\n';
  //cerr << "locked:   " << n_locked << '\n';
  cerr << endl;
}
