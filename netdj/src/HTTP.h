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


//////////////////////////////////////////////////
// HTTP                                         //
//////////////////////////////////////////////////
class HTTP {
private:
  std::map<std::string, std::string> headers;
  std::string body;
  static Regex headerreg;
  std::string packet;

  int ParseHeaders(const std::string&, int);

protected:
  virtual int ParseCommand(const std::string&, int) = 0;
  virtual void ClearCommand() = 0;
  virtual void CreateCommand(std::string&) = 0;

public:
  HTTP();
  virtual ~HTTP() {};

  bool Parse(const std::string&, const bool = false);
  bool GetHeader(const std::string&, std::string&) const;
  void SetHeader(const std::string&, const std::string&);

  void CreatePacket();
  const std::string& Packet() const;
  void SetBody(const std::string&);

  void Clear();
  void ClearBody();
};


//////////////////////////////////////////////////
// HTTPRequest                                  //
//////////////////////////////////////////////////
class HTTPRequest : public HTTP {
private:
  std::string command;
  std::string URI;
  std::string version;

  std::string name;
  std::map<std::string,std::string> params;

  static Regex comreg;
  static Regex paramreg;

  int ParseCommand(const std::string&, int);
  void ClearCommand();
  void CreateCommand(std::string&);

public:
  HTTPRequest() : HTTP() {};

  std::string GetCommand() const {return command;};
  std::string GetURI() const {return URI;};
  std::string GetVersion() const {return version;};
  std::string GetURIResource() const {return name;};

  std::string GetURIName() const {return name;};
  bool GetURIParam(const std::string&, std::string&) const;
};


//////////////////////////////////////////////////
// HTTPResponse                                 //
//////////////////////////////////////////////////
class HTTPResponse : public HTTP {
private:
  std::string version;
  int status;
  std::string description;

  static Regex comreg;

  int ParseCommand(const std::string&, int);
  void ClearCommand();
  void CreateCommand(std::string&);

public:
  HTTPResponse() : HTTP(), status(0) {};
  HTTPResponse(int, const std::string& = "");

  std::string GetVersion() const {return version;};
  int GetStatus() const {return status;};
  std::string GetDescription() const {return description;};

  void SetVersion(const std::string& st) {version = st;};
  void SetDescription(const std::string& st) {description = st;};
  void SetStatus(int i) {version = i;};
};

#endif
