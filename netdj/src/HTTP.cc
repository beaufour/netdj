/*
 *  HTTP.cc
 * 
 *  Allan Beaufour Larsen <allan@beaufour.dk>
 *
 *  $Id$
 */

#include "HTTP.h"

//////////////////////////////////////////////////
// HTTP                                         //
//////////////////////////////////////////////////
Regex HTTP::headerreg("^([^\r]+) *: *([^\r]+)\r\n");

int
HTTP::ParseHeaders(const string& st, int pos) {
  vector<string> line;
  int newpos;

  headers.clear();

  // Parse headers
  while ((newpos = headerreg.Match(st.substr(pos), line))) {
    headers[line[0]] = line[1];
    pos += newpos;
    line.clear();
  };

  return pos;
}

bool
HTTP::Parse(const string& st, const bool savebody = false) {
  int pos;

  if ((pos = ParseCommand(st, 0))) {
    if ((pos = ParseHeaders(st, pos)) && savebody) {

      // An empty line should follow headers
      if (st.substr(pos + 1, 2) == "\r\n") {
	body = st.substr(pos + 3);
      } else {
	pos = 0;
      };
    }
  }

  return pos ? true : false;
};

bool
HTTP::GetHeader(const string& st, string &res) const {
  map<string, string>::const_iterator it = headers.find(st);
  if (it != headers.end()) {
    res = it->second;
    return true;
  }
  return false;
}

//////////////////////////////////////////////////
// HTTPRequest                                  //
//////////////////////////////////////////////////
Regex HTTPRequest::comreg("^([^ ]+) +([^ ]+) +HTTP/([^\r]+)\r\n");
Regex HTTPRequest::paramreg("([^=&]+)(=([^&]*))?&?");

#include <iostream>

int
HTTPRequest::ParseCommand(const string& st, int pos) {
  vector<string> line;
  int newpos;
  vector<string> pmat;
  // Parse Command
  if ((newpos = comreg.Match(st.substr(pos), line))) {
    command = line[0];
    URI = line[1];
    version = line[2];
  };

  // Parse URI
  string::size_type qpos;
  int npos;
  params.clear();
  if ((qpos = URI.find('?')) != URI.npos) {
    name = URI.substr(0, qpos);
    ++qpos;
    while ((npos = paramreg.Match(URI.substr(qpos), pmat))) {
      if (pmat.size() > 2) {
	params[pmat[0]] = pmat[2];
      } else {
	params[pmat[0]] = "";
      }
      pmat.clear();
      qpos += npos;
    }
  } else {
    name = URI;
  }


  return pos + newpos;
}

bool
HTTPRequest::GetURIParam(const string& st, string& res) const {
  map<string, string>::const_iterator it = params.find(st);
  if (it != params.end()) {
    res = it->second;
    return true;
  }
  return false;
}
