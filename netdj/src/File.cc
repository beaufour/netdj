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

// Provides rename
#include <cstdio>

// Provides getpwnam
#include <pwd.h>
#include <sys/types.h>

using namespace std;

bool
File::update_stat() const {
  if (name.size() != 0 && stat(name.c_str(), &stat_buf) == 0 && !S_ISDIR(stat_buf.st_mode)) {
    stat_valid = true;
  } else {
    memset(&stat_buf, 0, sizeof(stat_buf));
    stat_valid = false;
  };
  return stat_valid;
};

void
File::check_stat() const {
  if (!stat_valid) {
    update_stat();
  }
}

File::File ()
  : name(""), validid3(false) {
};


File::File (std::string _name)
    : name(_name), validid3(false) {
};

int
File::GetId() const {
  return id;
};

void
File::SetId(int _id) {
  id = _id;
};

bool
File::Exists() {
  return update_stat();
};

time_t
File::GetMtime() const {
  check_stat();
  return stat_buf.st_mtime;
};

time_t
File::GetCtime() const {
  check_stat();
  return stat_buf.st_ctime;
};

off_t
File::GetSize() const {
  check_stat();
  return stat_buf.st_size;
};

bool
File::SetName(const string _name) {
  name = _name;
  return update_stat();
}

string
File::GetName() const {
  return name;
};

string
File::GetDirname() const {
  return name.substr(0, name.find_last_of('/'));
};

string
File::GetFilename() const {
  return name.substr(name.find_last_of('/') + 1);
};

string
File::GetFilenameNoType() const {
  string tmpstr = GetFilename();
  return tmpstr.substr(0, tmpstr.find_last_of('.'));
}

bool
File::GetID3Info(ID3Tag const * &_id3) {
  if (!validid3) {
    validid3 = true;
    id3.InitFromFile(name);
  }
  _id3 = &id3;
  return validid3;
}

bool
File::Delete() {
  return !unlink(name.c_str());
}

bool
File::Rename(const string& newpath) {
  if (!rename(name.c_str(), newpath.c_str())) {
    name = newpath;
    return true;
  } else {
    return false;
  }
}

bool
File::Symlink(const string &linkname) const {
  return !symlink(name.c_str(), linkname.c_str());
}

string
File::GetOwner() {
  struct passwd *pw;

  check_stat();
  pw = getpwuid(stat_buf.st_uid);
  if (pw) {
    return string(pw->pw_name);
  } else {
    return string("");
  }
}

bool
File::operator< (const File& f2) const {
    return GetCtime() < f2.GetCtime();
}
