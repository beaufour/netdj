/**
 * \file Collection_Songlist_File.cpp
 * Class Collection_Songlist_File.
 *
 * $Id$
 *
 */

#include "Collection_Songlist_File.h"
#include "SongFile.h"

#include "StdException.h"

#include <qapplication.h>
#include <fstream>
#include <cstdlib>

using namespace std;
using namespace NetDJ;

Collection_Songlist_File::Collection_Songlist_File(const string aId,
						   const string aDescr,
						   const string aName)
  : Collection_Songlist(aId, aDescr, false), mFilename(aName) {

  Update();
}

Collection_Songlist_File::~Collection_Songlist_File() {

}


void
Collection_Songlist_File::Update() {
  /* Temporary storage for new list */
  deque<ISong*> newlist;

  /* Open file */
  ifstream flist(mFilename.c_str());
  if (!flist) {
    throw StdException("Could not open file '" + mFilename + "'!");
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
      newlist.push_back(new SongFile(line, GetUNID()));
    }
    catch (...) {
      qWarning("Could not add '%s' to collection '%s'!", line, GetIdentifier().c_str());
    }
  }

  /* Swap content */
  QMutexLocker locker(&mMutex);
  mSonglist.swap(newlist);
}
