/**
 *  Configuration.cc
 * 
 *  Allan Beaufour Larsen <allan@beaufour.dk>
 *
 *  $Id$
 */

#include "config.h"
#include "Configuration.h"

#include <cstdlib>
#include <iostream>

#include <qfile.h>
#include <qfileinfo.h>
#include <qtextstream.h>

using namespace std;
using namespace NetDJ;

typedef struct 
{
  const char* mName;
  const char* mValue;
} defvalue_t;

const defvalue_t gDefValues[] = {
  {"SERVER_PORT",       "7676"},
  {"SHARE_DIR",         "/usr/share/mp3/share"},
  {"REQUEST_DIR",       "/usr/share/mp3/request"},
  {"USER_LIST_FILE",    "netdj-users"},
  {"UPDATE_BEFORE_TRANSMIT",    "false"},
  {"STREAM_HOST",       "127.0.0.1"},
  {"STREAM_PORT",       "8000"},
  {"STREAM_MOUNT",      "netdj"},
  {"STREAM_USER",       "source"},  
  {"STREAM_PASSWD",     "hackme"},  
  {"STREAM_NAME",       "NetDJ"},
  {"STREAM_GENRE",      "Mixed"},
  {"STREAM_DESCR",      "NetDJ Streaming Channel"},
  {"STREAM_PUBLIC",     "false"},
  {"DAEMON_MODE",       "false"},
  {"DAEMON_CHANGE_USER", "false"},
  {"DAEMON_GROUP",      "nobody"},
  {"DAEMON_USER",       "nobody"},
  {0,                   0}
};

bool
Configuration::Init(const QString& aFilename) {
  // Fill in default values
  for (int i = 0; gDefValues[i].mName; ++i) {
    mConfig[gDefValues[i].mName] = gDefValues[i].mValue;
  }

  // Try to find config-file
  QFile file(QString(".") + NETDJ_CONF_FILENAME);
  if (!file.open(IO_ReadOnly)) {
    file.setName(QString(getenv("HOME")) + "/." + NETDJ_CONF_FILENAME);
    if (!file.open(IO_ReadOnly)) {
      file.setName(QString(NETDJ_ETCDIR) + "/" + NETDJ_CONF_FILENAME);
      if (!file.open(IO_ReadOnly)) {
	cout << "Could not find configuration file (."
	     << NETDJ_CONF_FILENAME
	     << " | ~/." << NETDJ_CONF_FILENAME
	     << " | " << NETDJ_ETCDIR << "/" << NETDJ_CONF_FILENAME
	     << "), using defaults!!!" << endl;
	return false;
      }
    }
  }
  
  // Read file
  QTextStream is (&file);
  int i = 1;
  QString line;
  QString name, value;
  while (!is.atEnd()) {
    line = is.readLine();
    if (line.isEmpty() || line[0] == '#') {
      continue;
    }
    name = line.section('=', 0, 0).stripWhiteSpace();
    value = line.section('=', 1).stripWhiteSpace();
    if (!name.isEmpty() && !value.isEmpty()) {
      mConfig[name] = value;
    } else {
      cerr << "Error parsing line " << i
	   << ": " << line
	   << endl;      
    }
    
    ++i;
  }
  file.close();

  // Insert configuration dir
  mConfig["CONFIG_DIR"] = QFileInfo(file).dirPath();

  return true;
}

void
Configuration::Dump() const
{
  for (QMap<QString, QString>::const_iterator it = mConfig.begin();
       it != mConfig.end();
       ++it) {
    cout << it.key() << " = "
	 << (it.data().isEmpty() ? "<null>" : it.data())
	 << endl;
  }
}

QString
Configuration::GetString(const QString &str) const {
  return mConfig[str];
}

int
Configuration::GetInteger(const QString &str) const {
  return mConfig[str].toInt();
}

bool
Configuration::GetBool(const QString &str) const {
  return (GetInteger(str) == 1 || mConfig[str] == "true");
}

void
Configuration::SetString(const QString &str, const QString &val) {
  mConfig[str] = val;
};

void
Configuration::SetBool(const QString &str, bool val) {
  mConfig[str] = val ? "true" : "false";
}

void
Configuration::SetInteger(const QString &str, int val) {
  mConfig[str].setNum(val);
}
