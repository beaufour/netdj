/* -*- c++ -*-
 *  Configuration.h
 * 
 *  Allan Beaufour Larsen <allan@beaufour.dk>
 *
 *  $Id$
 */

#ifndef __CONFIGURATION_H__
#define __CONFIGURATION_H__

#include <map>
#include <string>

class Configuration {
private:
  std::map<std::string, std::string> conflist;

  std::string Get(const std::string&) const;

public:
  void ReadFile();

  std::string GetString(const std::string&) const;
  int GetInteger(const std::string&) const;
  bool GetBool(const std::string&) const;

  void SetString(const std::string&, const std::string&);
  void SetBool(const std::string&, bool);
  void SetInteger(const std::string&, int);
};

#endif
