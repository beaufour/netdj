/*
 *  util.h
 * 
 *  Allan Beaufour Larsen <allan@beaufour.dk>
 *
 *  $Id$
 */

#ifndef __UTIL_H__
#define __UTIL_H__

// Provides string-container
#include <string>

// Provides rand, srand and system
#include <cstdlib>

// Provides file-handling
#include <sys/stat.h>

// Provides basename
#include <libgen.h>

// Provides time_t
#include <ctime>

int get_rand(const int);
bool file_exists(const string&, struct stat*);
time_t file_mtime(const string&);
off_t file_size(const string&);
void error(const char*, const bool = true);
string get_filename(const string&);
string get_dirname(const string&);
string base64_decode(const char*);

#endif
