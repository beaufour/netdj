/*
 *  File.cc
 * 
 *  Allan Beaufour Larsen <allan@beaufour.dk>
 *
 *  $Id$
 */

#include "File.h"

// Provides basename
#include <libgen.h>

bool
File::update_stat() {
  if (name.size() != 0 && stat(name.c_str(), &stat_buf) == 0 && S_ISREG(stat_buf.st_mode)) {
    return true;
  } else {
    memset(&stat_buf, 0, sizeof(stat_buf));
    return false;
  };
};

bool
File::SetName(const string _name) {
  name = _name;
  return update_stat();
}

string
File::GetDirname() const {
  return name.substr(0, name.find_last_of('/'));
};

string
File::GetFilename() const {
  return name.substr(name.find_last_of('/') + 1);
};

bool
File::GetID3Info(ID3Tag* &_id3) {
  if (!validid3) {
    validid3 = true;
    id3.InitFromFile(name);
  }
  _id3 = &id3;
  return validid3;
}
