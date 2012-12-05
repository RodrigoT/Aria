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

// $Id: Cookie.cc,v 1.8 2001/11/04 10:18:07 tujikawa Exp $

#include "Cookie.h"

Cookie::Cookie()
{
    valid = false;
}

Cookie::~Cookie()
{
}

bool Cookie::Is_valid() const
{
    return valid;
}

void Cookie::all_clear()
{
    key_value_map.clear();
    domain = "";
    path = "";
    valid = false;
}

time_t Cookie::interpret_date(string date_gmt)
{
    struct tm time_st;

    if (date_gmt.empty()) return -1;

    string token = Token_splitter(date_gmt, ", ");
    if (token == "Sun" || token == "Sunday") {
        time_st.tm_wday = 0;
    } else if (token == "Mon" || token == "Monday") {
        time_st.tm_wday = 1;
    } else if (token == "Tue" || token == "Tuesday") {
        time_st.tm_wday = 2;
    } else if (token == "Wed" || token == "Wednesday") {
        time_st.tm_wday = 3;
    } else if (token == "Thu" || token == "Thursday") {
        time_st.tm_wday = 4;
    } else if (token == "Fri" || token == "Friday") {
        time_st.tm_wday = 5;
    } else if (token == "Sat" || token == "Saturday") {
        time_st.tm_wday = 6;
    } else {
        time_st.tm_wday = 0;
    }

    token = Token_splitter(date_gmt, ",- ");
    time_st.tm_mday = stoi(token);
    token = Token_splitter(date_gmt, ",- ");
    if (token == "Jan") {
        time_st.tm_mon = 0;
    } else if (token == "Feb") {
        time_st.tm_mon = 1;
    } else if (token == "Mar") {
        time_st.tm_mon = 2;
    } else if (token == "Apr") {
        time_st.tm_mon = 3;
    } else if (token == "May") {
        time_st.tm_mon = 4;
    } else if (token == "Jun") {
        time_st.tm_mon = 5;
    } else if (token == "Jul") {
        time_st.tm_mon = 6;
    } else if (token == "Aug") {
        time_st.tm_mon = 7;
    } else if (token == "Sep") {
        time_st.tm_mon = 8;
    } else if (token == "Oct") {
        time_st.tm_mon = 9;
    } else if (token == "Nov") {
        time_st.tm_mon = 10;
    } else if (token == "Dec") {
        time_st.tm_mon = 12;
    } else {
        time_st.tm_mon = 0;
    }
    token = Token_splitter(date_gmt, ",- ");
    int year = stoi(token);
    if (year < 100) {
        time_st.tm_year = stoi(token) + 100;
    } else {
        time_st.tm_year = stoi(token) - 1900;
    }
    token = Token_splitter(date_gmt, ":");
    time_st.tm_hour = stoi(token);
    token = Token_splitter(date_gmt, ":");
    time_st.tm_min =  stoi(token);
    token = Token_splitter(date_gmt, " ");
    time_st.tm_sec = stoi(token);

    time_t time_val = mktime(&time_st);// mktime assume time_st as GMT
    // fix this by adding offset time

#ifdef HAVE_TIMEZONE
    time_val -= timezone;
#else
    {
        time_t lt;
        struct tm *ltm;
        lt = time(NULL);
        ltm = localtime(&lt);

        time_val += ltm->tm_gmtoff;
    }
#endif

    //char *ast = ctime(&time_val);
    //cerr << ast << endl;

    return time_val;
}

bool Cookie::Parse(string cookie_string)
{
    if (cookie_string.empty()) return false;
    string key;
    string value;
    time_t expires = -1;
    while (cookie_string.size()) {
        string exp = Token_splitter(cookie_string, ";");
        string item = Token_splitter(exp, " =");
        if (item == "expires" || item == "Expires") {
            expires = interpret_date(exp);
        } else if (item == "path" || item == "Path") {
            path = exp;
        } else if (item == "domain" || item == "Domain") {
            domain = exp;
        } else {
            value = exp;
            key = item;
        }
    }
    pair<string, time_t> p(value, expires);
    key_value_map[key] = p;
    valid = true;
    return true;
}

bool Cookie::Is_alive(const string &key, const string &host, const string &dir)
{
    return valid && !Is_expired(key) && Is_in_path(dir) && Is_in_domain(host);
}

bool Cookie::Is_expired(const string &key)
{
    if (key_value_map[key].second <= 0 || time(NULL) - key_value_map[key].second < 0) return false;
    else return true;
}

bool Cookie::Is_expired(time_t expire_time) const
{
    if (expire_time <= 0 || time(NULL) - expire_time < 0) return false;
    else return true;
}

bool Cookie::Is_in_path(const string &dir) const
{
    if (dir.empty() || dir.find(path.c_str(), 0, path.size()) != string::npos) return true;
    else
        return false;
}

bool Cookie::Is_in_domain(const string &host) const
{
    if ((host.find(domain.c_str(), 0, domain.size()) != string::npos) || domain.empty()) return true;
    else return false;
}

const map<string, pair<string, time_t> > &Cookie::ret_key_value_map() const
{
    return key_value_map;
}

string Cookie::ret_names(const string &domain, const string &path) const
{
    string retstring;

    if (!Is_in_domain(domain) || !Is_in_path(path)) return "";

    for (map<string, pair<string, time_t> >::const_iterator itr = key_value_map.begin();
            itr != key_value_map.end(); ++itr) {
        if (!Is_expired(itr->second.second)) {
            retstring += itr->first + "=" + itr->second.first + "; ";

        }
    }
    return retstring;
}

const string &Cookie::ret_domain() const
{
    return domain;
}

void Cookie::set_domain(const string &domain_in)
{
    domain = domain_in;
}

const string &Cookie::ret_path() const
{
    return path;
}

void Cookie::merge(const Cookie &cookie)
{
    for (map<string, pair<string, time_t> >::const_iterator keypair = cookie.ret_key_value_map().begin();
            keypair != cookie.ret_key_value_map().end(); ++keypair) {
        key_value_map[keypair->first] = keypair->second;
    }
}
