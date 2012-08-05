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

// $Id: CommandList.cc,v 1.7 2002/03/16 14:13:00 tujikawa Exp $

#include "CommandList.h"

CommandList::CommandList()
{
  Command dummy;
  command_list.push_back(dummy);
  pthread_mutex_init(&command_list_lock, NULL);
}

CommandList::~CommandList()
{
  pthread_mutex_destroy(&command_list_lock);
}

const Command& CommandList::search(const string& filename)
{
  pthread_mutex_lock(&command_list_lock);
  for(vector<Command>::const_iterator itr = command_list.begin();
      itr != command_list.end(); ++itr) {
    if(itr->Is_valid() && itr->Is_in_extensions(filename)) {
      pthread_mutex_unlock(&command_list_lock);
      return *itr;
    }
  }
  pthread_mutex_unlock(&command_list_lock);
  return command_list.back();
}

bool CommandList::Is_reserved(const string& tag)
{

  if(tag == "<identifier>" || tag == "</identifier>" ||
     tag == "<target-extensions>" || tag == "</target-extensions>" ||
     tag == "<execute>" || tag == "</execute>" ||
     tag == "<comment>" || tag == "</comment>" ||
     tag == "<command>" || tag == "</command>" ||
     tag == "<execute-status>" || tag == "</execute-status>") {
    return true;
  } else {
    return false;
  }
}

bool CommandList::Read_from_file(const string& filename)
{
  ifstream infile;
  string line;
  string tag;
  StringHash command_map;

  try{
    pthread_mutex_lock(&command_list_lock);
    command_list.clear();
    infile.open(filename.c_str(), ios::in);//ios::skipws|ios::in);
    if(infile.bad()) throw EOPEN;

    while(infile.good()) {
      try {
	tag = get_next_tag(infile);
	if(!Is_reserved(tag)) {
	  throw tag;
	}
      } catch (int err) {
	break;
      }
      if(tag == "<command>") {
	command_map.clear();
	while(1) {
	  tag = get_next_tag(infile);
	  if(!Is_reserved(tag)) {
	    throw tag;
	  }
	  //cerr << tag << endl;
	  if(!infile.good()) {
	    cerr << "in getting tag in <command>" << endl;
	    throw TAGPARSE_GETTAG_EOF;
	  }
	  if(tag == "</command>") {
	    string extensions = command_map.get("<target-extensions>");
	    string name = command_map.get("<identifier>");
	    string commandline = command_map.get("<execute>");
	    string commandstatus = command_map.get("<execute-status>");
	    string comment = command_map.get("<comment>");
	    Command command(name, comment, commandline, commandstatus, extensions);
	    if(!command.bad()) {
	      command_list.push_back(command);
	    } else {
	      cerr << "error occurred in command list '" << command_map.get("<identifier>") << "' in file command.aria" << endl;
	      cerr << "discard this command entry" << endl;
	    }
	    break;
	  } else {
	    string value = get_value(infile, tag);
	    command_map.add(tag, value);
	  }
	}
      } else {
	throw tag;
      }
    }
    Command dummy;
    command_list.push_back(dummy);
    pthread_mutex_unlock(&command_list_lock);
    return true;
  } catch (int err) {
    switch(err) {
    case TAGPARSE_GETTAG_EOF:
      cerr << "in tag: " << tag << endl;
      break;
    case TAGPARSE_GETVALUE_EOF:
      cerr << "in getting value in tag: " << tag << endl;
      break;
    case TAGPARSE_UCOM_EOF:
      cerr << "unterminated comment tag" << endl;
      break;
    default:
      cerr << "io error; cannot open file command.aria" << endl;
      break;
    }
  } catch (ErrorType err) {
    cerr << "can't open file" << endl;
  } catch (string tag) {
    cerr << "invalid tag : " << tag << endl;
  }
  cerr << "error occurred in command '" << command_map.get("<identifier>") << "' in file command.aria" << endl;
  Command dummy;
  command_list.push_back(dummy);
  pthread_mutex_unlock(&command_list_lock);
  return(false);
}

vector<Command>& CommandList::ret_command_list()
{
  return command_list;
}

void CommandList::set_valid_safely(vector<Command>::iterator com_itr, bool flag)
{
  pthread_mutex_lock(&command_list_lock);
  com_itr->set_valid(true);
  pthread_mutex_unlock(&command_list_lock);
}

void CommandList::update_validity(GList *selection)
{
  pthread_mutex_lock(&command_list_lock);
  for(vector<Command>::iterator com_itr = command_list.begin();
      com_itr != command_list.end(); ++com_itr) {
    com_itr->set_valid(false);
  }
  while(selection) {
    int rowindex = GPOINTER_TO_UINT(selection->data);
    command_list[rowindex].set_valid(true);
    selection = g_list_next(selection);
  }
  pthread_mutex_unlock(&command_list_lock);
}
