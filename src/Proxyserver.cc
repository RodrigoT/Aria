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

// $Id: Proxyserver.cc,v 1.2 2001/02/22 09:36:51 tujikawa Exp $

#include "Proxyserver.h"

Proxyserver::Proxyserver()
{
  server = "";
  port = 0;
}

Proxyserver::Proxyserver(const string& server_in, int port_in)
{
  server = server_in;
  port = port_in;
}

Proxyserver::Proxyserver(const Proxyserver& entry)
{
  server = entry.ret_Server();
  port = entry.ret_Port();
}

const string& Proxyserver::ret_Server() const
{
  return server;
}

int Proxyserver::ret_Port() const
{
  return port;
}

void Proxyserver::set_Server(const string& server_in)
{
  server = server_in;
}

void Proxyserver::set_Port(int port_in)
{
  port = port_in;
}
