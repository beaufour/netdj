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

// Provides fopen, fread and fclose
#include <cstdio>

typedef struct id3tag_t {
	char		magic[3];
	char		songname[30];
	char		artist[30];
	char		album[30];
	char		year[4];
	char		note[30];
	unsigned char	style;
} id3tag_t;

class File {
 protected:
  int id;
  string name;
  struct stat stat_buf;
  bool validid3;
  id3tag_t id3;

  bool update_stat();

 public:
  File ()
    : name(""), validid3(false) { update_stat(); };
  File (string _name)
    : name(_name), validid3(false) { update_stat(); };

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

  bool GetID3Info(id3tag_t&);
  
  bool operator< (const File& f2) const {
    return GetCtime() < f2.GetCtime();
  }
};

#endif
