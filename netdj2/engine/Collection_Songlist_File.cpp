/**
 * \file Collection_Songlist_File.cpp
 * Class Collection_Songlist_File.
 *
 * $Id$
 *
 */

#include "Collection_Songlist_File.h"
#include "StdException.h"

#include <qapplication.h>
#include <fstream>
#include <cstdlib>

using namespace std;

Collection_Songlist_File::Collection_Songlist_File(const string id,
						   const string descr,
						   const string fname)
  : Collection_Songlist(id, descr, false), Filename(fname) {

  Update();
}

Collection_Songlist_File::~Collection_Songlist_File() {

}


void
Collection_Songlist_File::Update() {
  /* Temporary storage for new list */
  deque<Song> newlist;

  /* Open file */
  ifstream flist(Filename.c_str());
  if (!flist) {
    throw StdException("Could not open file '" + Filename + "'!");
  }

  /* Read file */
  char line[4096];
  while (flist) {
    flist.getline(line, sizeof(line));
    // Ignore empty lines
    if (line[0] == 0) {
      continue;
    }
    try {
      newlist.push_back(Song(line));
    }
    catch (...) {
      qWarning("Could not add '%s' to collection '%s'!", line, GetIdentifier().c_str());
    }
  }

  /* Swap content */
  QMutexLocker locker(&mutex);
  Songlist.swap(newlist);
}
