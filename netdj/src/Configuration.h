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
  map<string, string> conflist;

  string Get(const string&) const;

public:
  void ReadFile();

  string GetString(const string&) const;
  int GetInteger(const string&) const;
  bool GetBool(const string&) const;

  void SetString(const string&, const string&);
  void SetBool(const string&, bool);
  void SetInteger(const string&, int);
};

#endif
