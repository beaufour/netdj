/*
 *  Directory.h
 * 
 *  Allan Beaufour Larsen <allan@beaufour.dk>
 *
 *  $Id$
 */

#ifndef  __DIRECTORY_H__
#define __DIRECTORY_H__

#include "Lockable.h"

// STL containers and such
#include <deque>
#include <vector>
#include <string>
#include <algorithm>

// Provides directory-handling
#include <sys/types.h>
#include <dirent.h>

// Provides file-handling
#include <sys/stat.h>
#include <unistd.h>

#include "util.h"

class Directory : public Lockable {
private:
  class File {
  public:
    string name;
    time_t timestamp;
    File (string _name, time_t _timestamp)
      : name(_name), timestamp(_timestamp) {}

    bool operator< (const File& f2) const {
      return timestamp < f2.timestamp;
    }
  };

  typedef deque<File> list_t;

  string shname, descr;
  string dirname;
  time_t lastupdate, lastplay;
  list_t list;
  bool playlist;
  pthread_mutex_t mutex;
  pthread_mutexattr_t mutexattr;

  void Update();

public:
  Directory(char* _shname, char* _descr, bool _playlist = true)
    : Lockable(),
      shname(_shname), descr(_descr),
      lastupdate(time_t(0)), lastplay(time_t(0)), playlist(_playlist) {};
  
  void SetDirname(const string&);
  const string* GetDirname();

  void SetLastupdate(const time_t);
  time_t GetNextTimestamp();

  bool Empty();
  void GetSong(string&);
  void AddSong(const string&);
  int GetSize();
  int GetEntries(vector<string>&, unsigned int);

  const string* GetShortname();
  const string* GetDescription();
};

#endif
