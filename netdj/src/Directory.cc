/*
 *  Directory.cc
 * 
 *  Allan Beaufour Larsen <allan@beaufour.dk>
 *
 *  $Id$
 */

#include "Directory.h"

////////////////////////////////////////
// PRIVATE
void
Directory::Update() {
  if (!dirname.empty()) {
    DIR *dir_ptr = opendir(dirname.c_str());
    if (dir_ptr == NULL) {
      error("  Can't open directory!");
    }
    struct dirent *dirent_ptr = readdir(dir_ptr);
    struct stat stat_buf;
    string fname;
    char ending[5];
    list_t::iterator it_from = list.end();
    while (dirent_ptr != NULL) {
      fname = dirname + string(dirent_ptr->d_name);
      if (file_exists(fname, &stat_buf)) {
	if (stat_buf.st_ctime > lastupdate) {
	  strcpy(ending, fname.substr(fname.size()-4).c_str());
	  ending[1] = tolower(ending[1]);
	  ending[2] = tolower(ending[2]);
	  if (strstr(ending, ".mp3") != 0) {
	    list.push_back(File(fname, stat_buf.st_ctime));
	  }
	}
      }
      dirent_ptr = readdir(dir_ptr);
    }
    closedir(dir_ptr);
    // Sort new entries in FIFO-chosen lists (eg. playlist)
    if (playlist) {
      ::sort (it_from, list.end());
    }
    lastupdate = time(NULL);
  };
}


////////////////////////////////////////
// PUBLIC
void
Directory::SetDirname(const string &_dirname) {
  dirname = _dirname;
  // Add slash to end of directory name
  if (dirname[dirname.size() - 1] != '/') {
    dirname += '/';
  }
}

const string*
Directory::GetDirname() {
  return &dirname;
}


void
Directory::SetLastupdate(const time_t _lastupdate) {
  lastupdate = _lastupdate;
}


time_t
Directory::GetNextTimestamp() {
  // Only makes sense for non-playlists...
  return list.front().timestamp;
}


bool
Directory::Empty() {
  bool tmp;
  lock();
  Update();
  tmp = list.empty();
  unlock();
  return tmp;
}
  

void
Directory::GetSong(string &str) {
  lock();
  Update();
  if (list.empty()) {
    if (!str.empty()) str = "";
  } else if (playlist) {
    // Songs are chosen FIFO-order
    str = list.front().name;
    list.pop_front();
  } else {
    // Songs are chosen randomly
    str = list[get_rand(list.size())].name;
  }
  unlock();
}

void
Directory::AddSong(const string &str) {
  lock();
  if (!str.empty()) {
    list.push_back(File(str, time_t(0)));
  }
  unlock();
}

int
Directory::GetSize() {
  int tmp;
  lock();
  Update();
  tmp = list.size();
  unlock();
  return tmp;
}

int
Directory::GetEntries(vector<string> &vect, unsigned int max) {
  unsigned int i = 0;
  lock();
  if (playlist) {
    for (; (i < max) && i < list.size(); ++i) {
      vect.push_back(list[i].name);
    }
  }
  unlock();
  return i;
}

const string*
Directory::GetShortname() {
  return &shname;
}

const string*
Directory::GetDescription() {
  return &descr;
}
