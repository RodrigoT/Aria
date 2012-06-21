//
//  Aria - yet another download tool
//  Copyright (C) 2000, 2001, 2002 Tatsuhiro Tsujikawa
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

// $Id: TimerData.h,v 1.3 2002/01/20 14:41:58 tujikawa Exp $

#ifndef _TIMERDATA_H_
#define _TIMERDATA_H_
#include <time.h>

class TimerData {
private:
  int hour_start;
  int min_start;
  int hour_stop;
  int min_stop;

  time_t t_start;
  time_t t_stop;
public:
  TimerData(int hour_start, int min_start, int hour_stop, int min_stop);
  TimerData();
  ~TimerData();

  int ret_hour_start();
  int ret_min_start();
  int ret_hour_stop();
  int ret_min_stop();

  void set_start_time(int hour_start, int min_start);
  void set_stop_time(int hour_stop, int min_stop);
  void adjustStopTimeBasedOnStartTime();
  time_t get_correct_time(int hour, int min, bool force_next_day = false);

  time_t ret_start_time();
  time_t ret_stop_time();
  void Update_start_time();
  void Update_stop_time();
};
#endif // _TIMERDATA_H_
