/*
 *  AccessConf.cc
 * 
 *  Allan Beaufour Larsen <allan@beaufour.dk>
 *
 *  $Id$
 */

#include "AccessConf.h"

#include "Regex.h"

void
AccessConf::ReadFile(const string& fname) {
  // userid:userlevel:password
  Regex reg("^([^:]+):([[:digit:]]+):(.+)$");
  ifstream conf(fname.c_str());
  vector<string> splitline;
  char line[255];
  User user;

  cout << "Reading user entries: " << endl;
  
  lock();
  while (conf.getline(line, sizeof(line))) {
    splitline.clear();
    if (line[0] != '#' && reg.Match(line, splitline)) {
      cout << "  " << splitline[0] << ", " << splitline[1] << endl;
      user.SetInfo(atoi(splitline[1].c_str()), splitline[2]);
      users[splitline[0]] = user;
    }
  }
  unlock();
}

bool
AccessConf::IsAccessAllowed(const string& str, int acclevel, string* userstr = NULL) {
  bool res = false;
  Regex reg("([^:]+):(.+)");
  vector<string> splitline;
  map<string, User>::iterator user;

  lock();
  if (reg.Match(str, splitline)) {
    if (userstr) {
      *userstr = splitline[0];
    }
    if ((user = users.find(splitline[0])) != users.end()) {
      res = ((user->second.GetPassword() == splitline[1])
	     && (user->second.GetLevel() >= acclevel));
    }
  }
  unlock();

  return res;
}
