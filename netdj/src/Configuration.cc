/*
 *  Configuration.cc
 * 
 *  Allan Beaufour Larsen <allan@beaufour.dk>
 *
 *  $Id$
 */

#include "Configuration.h"

#include <vector>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include "Regex.h"
#include "config.h"

const unsigned int CONFSIZE = 20 * 2;
const char *CONF[CONFSIZE] = {"HTTP_PORT", "7676",
			      "PLAYER", "/usr/bin/mpg123",
			      "CACHE_DIR", "/usr/share/mp3/cache",
			      "SHARE_DIR", "/usr/share/mp3/share",
			      "REQUEST_DIR", "/usr/share/mp3/request",
			      "WEB_LOCKED", "false",
			      "HTTP_START", "false",
			      "HTTP_REDIRECT", "",
			      "PLAYER_START", "true",
			      "PLAYER_LOG", "false",
                              "DELETE_PLAYED", "false",
                              "STREAM", "false",
                              "STREAM_IP", "127.0.0.1",
                              "STREAM_PORT", "8000",
                              "STREAM_MOUNT", "netdj",
                              "STREAM_PASSWD", "",
			      "DAEMON_MODE", "false",
			      "DAEMON_CHANGEUSER", "false",
			      "DAEMON_UID", "",
			      "DAEMON_GID", ""};

void
Configuration::ReadFile() {
  // Construct dotfile-path
  char* home = getenv("HOME");
  string dotpath;
  if (home) {
    dotpath = home;
    if (dotpath[dotpath.size() - 1] != '/') {
      dotpath += '/';
    }
  }
  dotpath += CONF_DIRNAME;
  if (dotpath[dotpath.size() - 1] != '/') {
    dotpath += '/';
  }

  // Open file in user's home directory
  string fullname = dotpath + CONF_FILENAME;
  ifstream conf(fullname.c_str());
  if (!conf.is_open()) {
    // Not found, try global directory
    fullname = NETDJ_ETCDIR;
    fullname += "/";
    fullname += CONF_FILENAME;
    conf.open(fullname.c_str());
  }

  // Insert default values
  for (unsigned int i = 0; i < CONFSIZE; i += 2) {
    conflist[CONF[i]] = CONF[i+1];
  }

  // Read file
  cout << "Reading configuration entries from '" << fullname << "':" << endl;
  Regex reg("([^ ]+) *= *([^ ]+)");
  char line[255];
  vector<string> splitline;
  while (conf.getline(line, sizeof(line))) {
    splitline.clear();
    if (line[0] != '#' && reg.Match(line, splitline)) {
      cout << "  " << splitline[0] << " = " << splitline[1] << endl;
      conflist[splitline[0]] = splitline[1];
    }
  }
  
  // Save location of configuration files
  conflist["$$CONFDIR"] = dotpath;
  conflist["$$CONFFILE"] = fullname;
}

string
Configuration::Get(const string &str) const {
  map<string, string>::const_iterator it = conflist.find(str);
  if (it != conflist.end()) {
    return it->second;
  } else {
    return "";
  }
}

string
Configuration::GetString(const string &str) const {
  return Get(str);
}

int
Configuration::GetInteger(const string &str) const {
  return atoi(Get(str).c_str());
}

bool
Configuration::GetBool(const string &str) const {
  return (GetInteger(str) == 1 || Get(str) == "true");
}

void
Configuration::SetString(const string &str, const string &val) {
  conflist[str] = val;
};

void
Configuration::SetBool(const string &str, bool val) {
  conflist[str] = val ? "true" : "false";
}

void
Configuration::SetInteger(const string &str, int val) {
  char tmpint[10];
  sprintf(tmpint, "%d", val);
  conflist[str] = tmpint;
}
