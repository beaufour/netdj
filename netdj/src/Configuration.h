/*
 *  Configuration.h
 * 
 *  Allan Beaufour Larsen <allan@beaufour.dk>
 *
 *  $Id$
 */

#ifndef __CONFIGURATION_H__
#define __CONFIGURATION_H__

#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <cstdlib>
#include "Regex.h"

class Configuration {
private:
  map<string, string> conflist;

public:
  void ReadFile(string);
  string GetString(string);
  int GetInteger(string);
  bool GetBool(string);
};

#endif
