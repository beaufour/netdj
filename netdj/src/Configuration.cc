/*
 *  Configuration.cc
 * 
 *  Allan Beaufour Larsen <allan@beaufour.dk>
 *
 *  $Id$
 */

#include "Configuration.h"

const unsigned int CONFSIZE = 9 * 2;
const char *CONF[CONFSIZE] = {"HTTP_PORT", "7676",
			      "PLAYER", "/usr/bin/mpg123",
			      "CACHE_DIR", "/usr/share/mp3/cache",
			      "SHARE_DIR", "/usr/share/mp3/share",
			      "WEB_LOCKED", "false",
			      "HTTP_START", "false",
			      "PLAYER_START", "true",
                              "DELETE_PLAYED", "false",
                              "STREAM", "false"};

void
Configuration::ReadFile(string fname) {
  Regex reg("\\([^ ]*\\) *= *\\([^ ]*\\)");
  ifstream conf(fname.c_str());
  char line[255];
  vector<string> splitline;

  // Insert default values
  for (unsigned int i = 0; i < CONFSIZE; i += 2) {
    conflist[CONF[i]] = CONF[i+1];
  }

  cout << "Reading configuration entries:" << endl;
  while (conf.getline(line, sizeof(line))) {
    splitline.clear();
    if (line[0] != '#' && reg.Match(line, splitline)) {
      cout << "  " << splitline[0] << " = " << splitline[1] << endl;
      conflist[splitline[0]] = splitline[1];
    }
  }
}

string
Configuration::GetString(string str) {
  return conflist[str];
}

int
Configuration::GetInteger(string str) {
  return atoi(conflist[str].c_str());
}

bool
Configuration::GetBool(string str) {
  return (atoi(conflist[str].c_str()) == 1 || conflist[str] == "true");
}
