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
  bool validid3;
  ID3Tag id3;
  mutable struct stat stat_buf;
  mutable bool stat_valid;

  bool update_stat() const;
  void check_stat() const;

 public:
  File ();
  File (std::string _name);

  int GetId() const;
  void SetId(int _id);

  bool Exists();
  time_t GetMtime() const;
  time_t GetCtime() const;
  off_t GetSize() const;

  std::string GetOwner();

  // Name operations
  bool SetName(const std::string);
  std::string GetName() const;
  std::string GetDirname() const;
  std::string GetFilename() const;
  std::string GetFilenameNoType() const;

  bool GetID3Info(ID3Tag const*&);

  // Disk operations
  bool Delete();
  bool Rename(const std::string&);
  bool Symlink(const std::string&) const;
  
  bool operator< (const File& f2) const;
};

#endif
