/* -*- C++ -*-
 *
 *  HTTP.h
 * 
 *  Allan Beaufour Larsen <allan@beaufour.dk>
 *  
 *  $Id$
 */

#ifndef __HTTP_H__
#define __HTTP_H__

#include <map>
#include <string>
#include "Regex.h"

class HTTP {
private:
  map<string, string> headers;
  string body;
  static Regex headerreg;

  int ParseHeaders(const string&, int);

protected:
  virtual int ParseCommand(const string&, int) = 0;

public:
  HTTP() {};
  virtual ~HTTP() {};

  bool Parse(const string&, const bool = false);
  bool GetHeader(const string&, string&) const;
};


class HTTPRequest : public HTTP {
private:
  string command;
  string URI;
  string version;

  string name;
  map<string,string> params;

  static Regex comreg;
  static Regex paramreg;

  int ParseCommand(const string&, int);

public:
  HTTPRequest() : HTTP() {};

  string GetCommand() const {return command;};
  string GetURI() const {return URI;};
  string GetVersion() const {return version;};
  string GetURIResource() const {return name;};

  string GetURIName() const {return name;};
  bool GetURIParam(const string&, string&) const;
};

#endif
