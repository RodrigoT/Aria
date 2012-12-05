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

// $Id: TimerData.cc,v 1.6 2002/10/01 15:32:00 tujikawa Exp $
#include <iostream>
#include "TimerData.h"

TimerData::TimerData(int hour_start_in,
                     int min_start_in,
                     int hour_stop_in,
                     int min_stop_in)
{
    set_start_time(hour_start_in, min_start_in);
    set_stop_time(hour_stop_in, min_stop_in);
}

TimerData::TimerData()
{
    set_start_time(0, 0);
    set_stop_time(0, 0);
}

TimerData::~TimerData()
{
}

void TimerData::adjustStopTimeBasedOnStartTime()
{
    /*
    if((int)(t_stop-t_start) < 0) {
      time_t baseTime = t_start;
      struct tm *tmst = localtime(&baseTime);

      ++(tmst->tm_mday);

      tmst->tm_hour = hour_stop;
      tmst->tm_min = min_stop;
      tmst->tm_sec = 0;

      t_stop = mktime(tmst);
    }
    */
}

time_t TimerData::get_correct_time(int hour, int min, bool force_next_day)
{
    time_t curtime = time(NULL);
    struct tm *tmst = localtime(&curtime);

    if (force_next_day || tmst->tm_hour > hour || (tmst->tm_hour == hour && tmst->tm_min > min)) {
        // set next day
        //cerr << "update" << endl;
        ++(tmst->tm_mday);
    }
    tmst->tm_hour = hour;
    tmst->tm_min = min;
    tmst->tm_sec = 0;
    //cerr << "C" << curtime << endl;
    //cerr << "T" << mktime(tmst) << endl;
    return mktime(tmst);
}

void TimerData::set_start_time(int hour, int min)
{
    hour_start = hour;
    min_start = min;
    t_start = get_correct_time(hour, min);
}

void TimerData::set_stop_time(int hour, int min)
{
    hour_stop = hour;
    min_stop = min;

    t_stop = get_correct_time(hour, min);
}

void TimerData::Update_start_time()
{
    t_start = get_correct_time(hour_start, min_start, true);
}

void TimerData::Update_stop_time()
{
    t_stop = get_correct_time(hour_stop, min_stop, true);
}

time_t TimerData::ret_start_time()
{
    return t_start;
}

time_t TimerData::ret_stop_time()
{
    return t_stop;
}

int TimerData::ret_hour_start()
{
    return hour_start;
}

int TimerData::ret_min_start()
{
    return min_start;
}

int TimerData::ret_hour_stop()
{
    return hour_stop;
}

int TimerData::ret_min_stop()
{
    return min_stop;
}

