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

#include "ID3Tag.h"

class File {
 protected:
  int id;
  std::string name;
  struct stat stat_buf;
  bool validid3;
  ID3Tag id3;

  bool update_stat();

 public:
  File ()
    : name(""), validid3(false) { update_stat(); };
  File (std::string _name)
    : name(_name), validid3(false) { update_stat(); };

  int GetId() const { return id; };
  void SetId(int _id) { id = _id; };

  bool Exists() { return update_stat(); };
  time_t GetMtime() const { return stat_buf.st_mtime; };
  time_t GetCtime() const { return stat_buf.st_ctime; };
  off_t GetSize() const { return stat_buf.st_size; };

  // Name operations
  bool SetName(const std::string);
  std::string GetName() const { return name; };
  std::string GetDirname() const;
  std::string GetFilename() const;
  std::string GetFilenameNoType() const;

  bool GetID3Info(ID3Tag const*&);

  // Disk operations
  bool Delete();
  bool Rename(const std::string&);
  bool Symlink(const std::string&) const;
  
  bool operator< (const File& f2) const {
    return GetCtime() < f2.GetCtime();
  }
};

#endif
