/*
 *  Directory.cc
 * 
 *  Allan Beaufour Larsen <allan@beaufour.dk>
 *
 *  $Id$
 */

#include "Directory.h"

#include <iostream>
#include <errno.h>
#include <cstring>

////////////////////////////////////////
// PRIVATE
void
Directory::Update() {
  File fobj;
  if (!dirname.empty()) {
    DIR *dir_ptr = opendir(dirname.c_str());
    if (dir_ptr == NULL) {
      cout << "Problems with '" << dirname << "':" << endl
	   << strerror(errno)
	   << endl;

      error("  Can't open directory!");
    }
    struct dirent *dirent_ptr = readdir(dir_ptr);
    string fname;
    char ending[5];
    list_t::iterator it_from;
    bool it_invalid = true;
    if (list.size()) {
      it_from = --list.end();
      it_invalid = false;
    };
    while (dirent_ptr != NULL) {
      fname = dirname + string(dirent_ptr->d_name);
      if (fobj.SetName(fname)) {
	if (fobj.GetCtime() > lastupdate) {
	  strcpy(ending, fname.substr(fname.size()-4).c_str());
	  ending[1] = tolower(ending[1]);
	  ending[2] = tolower(ending[2]);
	  if (strstr(ending, ".mp3") != 0) {
	    fobj.SetId(nextfileid++);
	    list.push_back(fobj);
	  }
	}
      }
      dirent_ptr = readdir(dir_ptr);
    }
    closedir(dir_ptr);

    if (it_invalid) {
      it_from = list.begin();
    } else {
      ++it_from;
    }

    // Sort new entries in FIFO-chosen lists (eg. playlist)
    if (playlist) {
      //      ::sort (it_from, list.end());
      // Having troubles with the iterators, I think.
      // Trying to sort the whole list instead ... not clever...
      ::sort(list.begin(), list.end());
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

const string&
Directory::GetDirname() const {
  return dirname;
}


void
Directory::SetLastupdate(const time_t _lastupdate) {
  lastupdate = _lastupdate;
}


time_t
Directory::GetNextTimestamp() const {
  // Only makes sense for non-playlists...
  return list.front().GetCtime();
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
  

bool
Directory::GetSong(File &fobj) {
  bool retval = true;
  lock();
  Update();
  if (list.empty()) {
    retval = false;
  } else if (playlist) {
    // Songs are chosen FIFO-order
    fobj = list.front();
    list.pop_front();
  } else {
    // Songs are chosen randomly
    fobj = list[get_rand(list.size())];
  }

  unlock();
  return retval;
}

void
Directory::AddSong(const string &str) {
  lock();
  File fobj(str);
  if (fobj.Exists()) {
    fobj.SetId(nextfileid++);
    list.push_back(fobj);
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
Directory::GetEntries(vector<File> &vect, unsigned int max) const {
  unsigned int i = 0;
  lock();
  if (playlist) {
    for (; (i < max) && i < list.size(); ++i) {
      vect.push_back(list[i]);
    }
  }
  unlock();
  return i;
}

const string&
Directory::GetShortname() const {
  return shname;
}

const string&
Directory::GetDescription() const {
  return descr;
}
