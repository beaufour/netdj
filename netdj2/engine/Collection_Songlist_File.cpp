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

#include <qtextstream.h>
#include <qfile.h>
#include <qfileinfo.h>

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
  /* Check whether file has changed */
  QFile file(mFilename);
  QFileInfo finfo(file);
  if (finfo.lastModified() <= mFiledate) {
    return;
  }

  mFiledate = finfo.lastModified();
  
  /* Temporary storage for new list */
  deque<ISong*> newlist;

  if (!finfo.isFile() || !finfo.isReadable() || !file.open(IO_ReadOnly)) {
    throw StdException("Could not open file '" + mFilename + "' for reading!");
  } else {
    QTextStream is (&file);
    QString line;
    while (!is.atEnd()) {
      line = is.readLine();
      if (line.isEmpty() || line[0] == '#') {
	continue;
      }
      try {
	newlist.push_back(new SongFile(line, GetNewUNID()));
      }
      catch (...) {
	qWarning("Could not add '%s' to collection '%s'!", line.latin1(), GetIdentifier().latin1());
      }
    }
  }
  
  /* Swap content */
  QMutexLocker locker(&mMutex);
  mSonglist.swap(newlist);
}
