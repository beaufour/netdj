/*
 *  File.h
 * 
 *  Allan Beaufour Larsen <allan@beaufour.dk>
 *
 *  $Id$
 */

#ifndef  __FILE_H__
#define __FILE_H__

#include <string>

// Provides file-handling
#include <sys/stat.h>
#include <unistd.h>

// Provides time_t
#include <ctime>

// Provides basename
#include <libgen.h>

class File {
 protected:
  int id;
  string name;
  struct stat stat_buf;
  
  bool update_stat();

 public:
  File ()
    : name("") { update_stat(); };
  File (string _name)
    : name(_name) { update_stat(); };

  int GetId() const { return id; };
  void SetId(int _id) { id = _id; };

  bool Exists() { return update_stat(); };
  time_t GetMtime() const { return stat_buf.st_mtime; };
  time_t GetCtime() const { return stat_buf.st_ctime; };
  off_t GetSize() const { return stat_buf.st_size; };

  bool SetName(const string);
  string GetName() const { return name; };
  string GetDirname() const;
  string GetFilename() const;
  
  bool operator< (const File& f2) const {
    return GetCtime() < f2.GetCtime();
  }
};

#endif
