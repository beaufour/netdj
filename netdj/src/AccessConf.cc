/*
 *  AccessConf.cc
 * 
 *  Allan Beaufour Larsen <allan@beaufour.dk>
 *
 *  $Id$
 */

#include "AccessConf.h"

#include <iostream>
#include "Regex.h"
#include "config.h"

using namespace std;

void
AccessConf::ReadFile() {
  // Open configuration files
  string fullname = Conf->GetString("$$CONFDIR") + CONF_USERFILENAME;
  ifstream conf(fullname.c_str());
  if (!conf.is_open()) {
    // Not found, try global directory
    fullname = NETDJ_ETCDIR;
    fullname += "/";
    fullname +=  CONF_USERFILENAME;
    conf.clear();
    conf.open(fullname.c_str());
  }

  cout << "Reading user entries from '" << fullname << "':" << endl;
  Regex reg("^([^:]+):([[:digit:]]+):(.+)$");
  vector<string> splitline;
  char line[255];
  User user;

  lock();
  // Clear list
  users.clear();

  // Read list from file
  while (conf.getline(line, sizeof(line))) {
    splitline.clear();
    if (line[0] != '#' && reg.Match(line, splitline)) {
      //      cout << "  " << splitline[0] << ", " << splitline[1] << endl;
      user.SetInfo(atoi(splitline[1].c_str()), splitline[2]);
      users[splitline[0]] = user;
    }
  }
  unlock();
}

bool
AccessConf::IsAccessAllowed(const string& str, int acclevel, string* userstr) const {
  bool res = false;
  Regex reg("([^:]+):(.+)");
  vector<string> splitline;
  map<string, User>::const_iterator user;

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
