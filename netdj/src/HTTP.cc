/*
 *  HTTP.cc
 * 
 *  Allan Beaufour Larsen <allan@beaufour.dk>
 *
 *  $Id$
 */

#include "HTTP.h"
#include "config.h"

// Provides atoi
#include <cstdlib>

// Provides sprintf
#include <cstdio>

//////////////////////////////////////////////////
// HTTP                                         //
//////////////////////////////////////////////////
Regex HTTP::headerreg("^([^\r]+) *: *([^\r]+)\r\n");

HTTP::HTTP() {
  SetHeader("Server", PKGVER);
}

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

void
HTTP::Clear() {
  ClearCommand();
  ClearBody();
}

void
HTTP::ClearBody() {
  body = "";
}

bool
HTTP::GetHeader(const string& st, string &res) const {
  map<string, string>::const_iterator it = headers.find(st);
  if (it != headers.end()) {
    res = it->second;
    return true;
  }
  return false;
}

void
HTTP::SetHeader(const string& hdr, const string& val) {
  headers[hdr] = val;
}

void
HTTP::CreatePacket() {
  // Create Command first
  CreateCommand(packet);

  // Add contentlength
  if (body.size()) {
    char tmplen[10];
    sprintf(tmplen, "%d", body.size());
    headers["Content-Length"] = tmplen;
  }

  // Add headers
  for (map<string,string>::const_iterator it = headers.begin();
       it != headers.end();
       ++it) {
    packet += it->first + ": " + it->second + "\r\n";
  }

  // Empty line between header and body
  packet += "\r\n";

  // And finnaly the body
  packet += body;
}

const string&
HTTP::Packet() const {
  return packet;
}

void
HTTP::SetBody(const string &str) {
  body = str;
}

//////////////////////////////////////////////////
// HTTPRequest                                  //
//////////////////////////////////////////////////
Regex HTTPRequest::comreg("^([^ ]+) +([^ ]+) +HTTP/([^\r]+)\r\n");
Regex HTTPRequest::paramreg("([^=&]+)(=([^&]*))?&?");

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

void
HTTPRequest::ClearCommand() {
  command = URI = version = "";
  params.clear();
}

void
HTTPRequest::CreateCommand(string &str) {
  str = command + " " + URI + " HTTP/" + version + "\r\n";
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

//////////////////////////////////////////////////
// HTTPResponse                                 //
//////////////////////////////////////////////////
Regex HTTPResponse::comreg("^HTTP/([^ ]+) +([[:digit:]]+) *(.*)\r\n");

HTTPResponse::HTTPResponse(int _status, const string &cont_type = "")
  : HTTP(), version("1.1"), status(_status) {
  if (cont_type.size()) {
    SetHeader("Content-Type", cont_type);
  }
}

int
HTTPResponse::ParseCommand(const string& st, int pos) {
  vector<string> line;
  int newpos;
  // Parse Command
  if ((newpos = comreg.Match(st.substr(pos), line))) {
    version = line[0];
    status = atoi(line[1].c_str());
    description = line[2];
  };

  return pos + newpos;
}

void
HTTPResponse::ClearCommand() {
  version = description = "";
  status = 0;
}

void
HTTPResponse::CreateCommand(string &str) {
  char tmpstat[10];
  sprintf(tmpstat, "%d", status);
  str = "HTTP/" + version + " " + tmpstat + " " + description + "\r\n";
}
