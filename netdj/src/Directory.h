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
#include "File.h"
#include "util.h"

// STL containers and such
#include <deque>
#include <vector>
#include <string>
#include <algorithm>

// Provides directory-handling
#include <sys/types.h>
#include <dirent.h>

class Directory : public Lockable {
private:
  typedef std::deque<File> list_t;

  std::string shname, descr;
  std::string dirname;
  int nextfileid;
  time_t lastupdate, lastplay;
  list_t list;
  bool playlist;
  pthread_mutex_t mutex;
  pthread_mutexattr_t mutexattr;
  bool fromfile;

  void Update();

public:
  Directory(char* _shname, char* _descr, bool _playlist = true)
    : Lockable(),
      shname(_shname), descr(_descr), nextfileid(0),
      lastupdate(time_t(0)), lastplay(time_t(0)), playlist(_playlist) {};
  
  void SetDirname(const std::string&, bool = false);
  const std::string& GetDirname() const;

  void SetLastupdate(const time_t);
  time_t GetNextTimestamp() const;

  bool Empty();
  bool GetSong(File&);
  void AddSong(const std::string&);
  int GetSize();
  int GetEntries(std::vector<File>&, unsigned int) const;

  const std::string& GetShortname() const;
  const std::string& GetDescription() const;
};

#endif
