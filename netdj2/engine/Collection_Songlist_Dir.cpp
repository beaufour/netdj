/**
 * \file Collection_Songlist_Dir.cpp
 * Class Collection_Songlist_Dir.
 *
 * $Id$
 *
 */

#include "Collection_Songlist_Dir.h"
#include <qdir.h>
#include <qptrlist.h>

using namespace std;

Collection_Songlist_Dir::Collection_Songlist_Dir(const string id,
						 const string descr,
						 const string dir,
						 const bool isq)
  : Collection_Songlist(id, descr, isq), Directory(dir)
{
  Update();
}

Collection_Songlist_Dir::~Collection_Songlist_Dir() {

}

void
Collection_Songlist_Dir::Update() {
  /* Temporary storage for new list */
  deque<Song> newlist;

  // Open directory
  QDir dir(Directory, QString::null, QDir::Time | QDir::Reversed,
	   QDir::Files | QDir::Readable);
  if (!dir.exists()) {
    throw StdException("Could not open directory '" + Directory + "'!");
  }

  // Iterate over files in directory
  const QFileInfoList* list = dir.entryInfoList();
  QFileInfoListIterator it(*list);
  QFileInfo* fi;

  /** \bug Something is wrong with the timestamp handling!! */

  while ((fi = it.current()) != 0 ) {
    try {
      if (isQueue) {
	if (fi->lastModified() >= LastTimeStamp) {
	  QMutexLocker locker(&mutex);
	  Songlist.push_back(Song(fi->filePath().ascii()));
	  LastTimeStamp = fi->lastModified();
	}
      } else { /* Not a queue */
	newlist.push_back(Song(fi->filePath().ascii()));
      }
      ++it;
    }
    catch (...) {
      qWarning("Could not add '%s' to collection '%s'!", fi->filePath().ascii(), GetIdentifier().c_str());
    }
  }

  /**
   * Add a second to timestamp, or last file(s) will show up in next
   * iteration.
   *
   * \note Can I do something smarter than add one second to the
   * timestamp?
   */
  LastTimeStamp.addSecs(1);
  
  /* Swap content */
  if (!isQueue) {
    QMutexLocker locker(&mutex);
    Songlist.swap(newlist);
  }
}
