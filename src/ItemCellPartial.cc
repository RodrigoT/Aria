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

// $Id: ItemCellPartial.cc,v 1.5 2002/03/16 14:13:00 tujikawa Exp $

#include "ItemCellPartial.h"
#include "RetrieveHTTP2.h"
#include "RetrieveFTP2.h"
#ifdef HAVE_OPENSSL
#include "RetrieveHTTPS2.h"
#endif // HAVE_OPENSSL

ItemCellPartial::ItemCellPartial(const string& url_in,
				 const URLcontainer& urlcon_in,
				 const Options& options_in,
				 const string& initial_log,
				 ItemCell *itemcell_boss,
				 unsigned int order_in,
				 unsigned int start_range_in,
				 unsigned int end_range_in)
  : ItemCell(url_in, urlcon_in, options_in, initial_log)
{
  order = order_in;
  start_range = start_range_in;
  end_range = end_range_in;
  boss = itemcell_boss;

  //filename_partial = urlcon_in.ret_Filename()+"."+itos(order);
  set_Filename(urlcon_in.ret_Filename()+"."+itos(order));
}

ItemCellPartial::~ItemCellPartial()
{
}

ItemCell::DownloadStatusType ItemCellPartial::Download_Main()
{
  Retrieve *retr_obj = 0;

  if(ret_URL_Container().ret_Protocol() == "http:" ||
     (ret_URL_Container().ret_Protocol() == "ftp:" &&
     ret_Options().ret_use_ftp_proxy() &&
     ret_Options().ret_use_ftp_proxy_via_http() &&
     !ret_Options().ret_ftp_proxy().ret_Server().empty())) {
    retr_obj = new RetrieveHTTP2(this);
  }
#ifdef HAVE_OPENSSL
  else if(ret_URL_Container().ret_Protocol() == "https:") {
    retr_obj = new RetrieveHTTPS2(this);
  }
#endif // HAVE_OPENSSL
  else if(ret_URL_Container().ret_Protocol() == "ftp:") {
    retr_obj = new RetrieveFTP2(this);
  }
  DownloadStatusType retval = retr_obj->Download_Main();
  /*
  if(retval == DLSUCCESS || retval == DLSUCCESSALR) {
    if(retr_obj->Post_process() != ItemCell::DLSUCCESS) {//crc check and etc
      retval = ItemCell::DLERROR;
      set_Command(ItemCell::DLERROR);// fixed 2001/3/16
      set_Status(ItemCell::ITEM_CRCERROR);
    }
  }
  */
  delete retr_obj;

  return retval;
}

unsigned int ItemCellPartial::ret_Order() const
{
  return order;
}

bool ItemCellPartial::Is_Partial() const
{
  return true;
}

void ItemCellPartial::set_Filename(const string& filename_in)
{
  ItemCell::set_Filename(filename_in+'.'+itos(order));
}

void ItemCellPartial::set_Filename_opt(const string& filename_in)
{
  ItemCell::set_Filename_opt(filename_in+'.'+itos(order));
}
/*
string ItemCellPartial::ret_Filename() const
{
  return ret_Filename();
}

string ItemCellPartial::ret_Filename_opt() const
{
  return ret_Filename_opt();
}
*/
unsigned int ItemCellPartial::ret_Start_range() const
{
  return start_range;
}
unsigned int ItemCellPartial::ret_End_range() const
{
  return end_range;
}

ItemCell::DownloadStatusType ItemCellPartial::Post_process()
{
  DownloadStatusType retval = DLSUCCESS;

  //pthread_mutex_lock(&itemlistlock); //mod 2001/4/11
  boss->get_Options_Lock(); //mod 2001/4/11
  boss->Remove_worker(this);
  if(boss->No_more_worker()) {
    boss->set_Status(ITEM_READY_CONCAT);
    boss->Send_status();
  }
  //pthread_mutex_unlock(&itemlistlock); //mod 2001/4/11
  boss->release_Options_Lock(); //mod 2001/4/11
  return retval;
}

void ItemCellPartial::WriteSplitInfo(const string& filename)
{
  ofstream indexfile(filename.c_str(), ios::out);
  if(indexfile.bad()) {
    cerr << "index file status:bad" << endl;
    return;
  } else {
    indexfile << start_range << '\t' << end_range;
  }
}

ItemCellPartial::SplitStatusType ItemCellPartial::SplitNumberChanged(const string& filename)
{
  struct stat file_stat;
  if(!stat(filename.c_str(), &file_stat)) {
    if(S_ISREG(file_stat.st_mode)) {
      // read file
      ifstream indexfile(filename.c_str(), ios::in);
      if(indexfile.bad()) return PARTIAL_NOINDEX;
      unsigned int start_range_in, end_range_in;
      indexfile >> start_range_in >> end_range_in;
      indexfile.close();
      if(start_range == start_range_in && end_range == end_range_in) {
	return PARTIAL_NORMAL;
      } else {
	WriteSplitInfo(filename);
	return PARTIAL_CHANGED;
      }
    } else {
      return PARTIAL_NOINDEX;
    }
  } else {
    WriteSplitInfo(filename);
    return PARTIAL_NOINDEX;
  }
}


ItemCell *ItemCellPartial::ret_Boss()
{
  return boss;
}
