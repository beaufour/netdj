/*
 *  AccessConf.h
 * 
 *  Allan Beaufour Larsen <allan@beaufour.dk>
 *
 *  $Id$
 */

#ifndef __ACCESSCONF_H__
#define __ACCESSCONF_H__

#include <map>
#include <string>
#include <fstream>
#include "Lockable.h"
#include "Regex.h"

class AccessConf : public Lockable {
 private:
  map<string, string> users;

 public:
  AccessConf() : Lockable() {};

  void ReadFile(const string&);
  bool IsAccessAllowed(const string&, string* = NULL);
};

#endif
