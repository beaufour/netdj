/*
 *  AccessConf.cc
 * 
 *  Allan Beaufour Larsen <allan@beaufour.dk>
 *
 *  $Id$
 */

#include "AccessConf.h"

void
AccessConf::ReadFile(const string& fname) {
  Regex reg("\\([^:]*\\):\\(.*\\)");
  ifstream conf(fname.c_str());
  vector<string> splitline;
  char line[255];

  cout << "Reading user entries: " << endl;
  
  lock();
  while (conf.getline(line, sizeof(line))) {
    splitline.clear();
    if (line[0] != '#' && reg.Match(line, splitline)) {
      cout << "  " << splitline[0] << " = ****** " << endl;
      users[splitline[0]] = splitline[1];
    }
  }
  unlock();
}

bool
AccessConf::IsAccessAllowed(const string& str) {
  bool res = false;
  Regex reg("\\([^:]*\\):\\(.*\\)");
  vector<string> splitline;

  lock();
  if (reg.Match(str, splitline)) {
    res = users[splitline[0]] == splitline[1];
  }
  unlock();

  return res;
}
