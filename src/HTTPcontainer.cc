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

// $Id: HTTPcontainer.cc,v 1.15 2001/10/17 13:06:34 tujikawa Exp $

// implementation of class HTTPcontainer
#include "HTTPcontainer.h"

// constructor
HTTPcontainer::HTTPcontainer()
{
  etag = "";
  contenttype = "";
  contentlength = 0;
  keepAliveFlag = false;
  compressedFlag = false;
}

// destructor
HTTPcontainer::~HTTPcontainer()
{
}

int HTTPcontainer::Parse_HTTP_header(const HTTPHeaderList& httpheaderlist)
{
  int retval = 1;//HTTPCON_NORMAL;

  HTTPHeaderList::const_iterator hl_itr;
  for(hl_itr = httpheaderlist.begin(); hl_itr != httpheaderlist.end(); ++hl_itr) {
    if(casecomp(hl_itr->ret_Item(), "Content-Length")) {
      // fixed 2001/5/27
      // ����
      // ��� Content-Length �ե�����ɤ���� Content-Range ��������
      // ���������Ȳ��ꤷ�Ƥ��ޤ�����������ϲ���Ǥ���ޤ�����
      // �դν�����������륵���Ф⤢�ꡢ�ޤ� Content-Length �ʤ���
      // Content-Range �Τ����������Τ⤢��ޤ����쥸�塼��򥵥ݡ���
      // ���ʤ������ФϤ��εդǤ���
      // ��β����� contentlength �� 0 (Content-Range �ˤ�ꡢ
      // �ͤ���������Ƥ��ʤ�) �Ȥ���, Content-Length ���ͤ����������
      // ������ΤǤ����°פǤϤ���ޤ�������������ư��ޤ���
      //
      // ��������� Content-Length �ϻĤ�ΥХ��ȿ� (����������� ?)
      // ���֤��Τ��Ф�, Content-Range �Τ���Ͼ�˥ե�����Υ�������
      // �֤����ȤǤ���
      // �����ϻĤ�Х��ȿ��Ȥߤʤ��������Ƥ��ޤ��� (Content-Range �ե������
      // ��̵�뤷�Ƥ����Ȥ������ȤǤ�)�� ��������Content-Length �ʤ���
      // Content-Range �Τ��֤������Ф����뤳�Ȥ��Τ�ޤ�����
      // ������ Content-Range ����Х��ȿ����ꡢ��˥ե������������
      // �Ȥ��ƽ�������褦������ޤ�������Content-Range, Content-Length
      // �ν�����������Ⱦ㳲��������Ȥ����꤬������ޤ�����
      // �ʤˤϤȤ⤢�졢������Ϥ��ޤ�����
      if(ret_ContentLength() == 0) {
	set_ContentLength(hl_itr->ret_Arg());
      }
    } else if(casecomp(hl_itr->ret_Item(), "ETag")) {
      set_ETag(hl_itr->ret_Arg());
    } else if(casecomp(hl_itr->ret_Item(), "Content-MD5")) {
      set_MD5(hl_itr->ret_Arg());
    } else if(casecomp(hl_itr->ret_Item(), "Content-Type")) {
      set_ContentType(hl_itr->ret_Arg());
    } else if(casecomp(hl_itr->ret_Item(), "status")) {
      set_HTTP_Status(hl_itr->ret_Arg());
    } else if(casecomp(hl_itr->ret_Item(), "Content-Range")) {
      // must deal with '*'. fix this
      string arg = hl_itr->ret_Arg();
      unsigned int slash_pos = arg.find('/');
      if(slash_pos != string::npos && slash_pos+1 < arg.size()) {
	set_ContentLength(arg.substr(slash_pos+1));
      }
    } else if(casecomp(hl_itr->ret_Item(), "Location")) {
      switch(ret_HTTP_Status()) {
      case MultipleChoices:
      case MovedPermanently:
      case Found:
      case SeeOther:
	set_Location(hl_itr->ret_Arg());
	retval = 1;
	break;
      case UseProxy:
	return -1;//throw HTTPCON_EUNSUPP;
      default:
	break;
      }
    } else if(casecomp(hl_itr->ret_Item(), "Set-Cookie")) {
      add_Cookie_string(hl_itr->ret_Arg());
    } else if(casecomp(hl_itr->ret_Item(), "Content-Location")) {
      set_contentLocation(hl_itr->ret_Arg());
    } else if(casecomp(hl_itr->ret_Item(), "Connection")) {
      if(casefind(hl_itr->ret_Arg(), "Keep-Alive") != string::npos) {
	setKeepAliveEnabled(true);
      }
    } else if(casecomp(hl_itr->ret_Item(), "Content-Encoding")) {
      if(casefind(hl_itr->ret_Arg(), "gzip") != string::npos
	 || casefind(hl_itr->ret_Arg(), "deflate") != string::npos) {
	setCompressEnabled(true);
      }
    } else if(casecomp(hl_itr->ret_Item(), "Transfer-Encoding")) {
      setTransferEncoding(hl_itr->ret_Arg());
    }
  }
  return retval;
}

void HTTPcontainer::setTransferEncoding(const string& transEnc)
{
  transferEncoding = transEnc;
}

const string& HTTPcontainer::getTransferEncoding()
{
  return transferEncoding;
}

void HTTPcontainer::setKeepAliveEnabled(bool toggle)
{
  keepAliveFlag = toggle;
}

void HTTPcontainer::setCompressEnabled(bool toggle)
{
  compressedFlag = toggle;
}

bool HTTPcontainer::isCompressEnabled()
{
  return compressedFlag;
}

int HTTPcontainer::ret_HTTP_Status() const
{
  return status;
}

unsigned int HTTPcontainer::ret_ContentLength() const
{
  return contentlength;
}

void HTTPcontainer::set_ContentLength(const string& clength_str)
{
  contentlength = strtol(clength_str.c_str(), (char**)NULL, 10);
}

void HTTPcontainer::set_ETag(const string& etag_in)
{
  etag = etag_in;
}

void HTTPcontainer::set_Location(const string& location_in)
{
  location = location_in;
}

const list<string>& HTTPcontainer::ret_Cookie_list() const
{
  return cookie_list;
}

const string& HTTPcontainer::ret_contentLocation() const {
  return contentLocation;
}

void HTTPcontainer::add_Cookie_string(const string& cookie_string)
{
  cookie_list.push_back(cookie_string);
}

void HTTPcontainer::set_contentLocation(const string& content_string) {
  contentLocation = content_string;
}

const string& HTTPcontainer::ret_Location() const
{
  return(location);
}

void HTTPcontainer::set_ContentType(const string& contenttype_in)
{
  contenttype = contenttype_in;
}

void HTTPcontainer::set_HTTP_Status(const string& status_str)
{
  status = strtol(status_str.c_str(), (char**)NULL, 10);
}

void HTTPcontainer::set_MD5(const string& md5Crypted)
{
  string md5Decrypted = Base64::decode(md5Crypted);
  char hexDigest[32];
  
  for(int i = 0; i < 16; ++i) {
    sprintf(hexDigest+i*2, "%02x", (unsigned char)md5Decrypted.at(i));
  }
  md5str = hexDigest;
}

const string& HTTPcontainer::ret_MD5() const
{
  return md5str;
}
