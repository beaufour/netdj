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

Collection_Songlist_Dir::Collection_Songlist_Dir(const string aId,
						 const string aDescr,
						 const string aDir,
						 const bool aIsQ,
						 const bool aDelete)
  : Collection_Songlist(aId, aDescr, aIsQ),
    mDirectory(aDir),
    mDeleteAfterPlay(aDelete)
{
  Update();
}

Collection_Songlist_Dir::~Collection_Songlist_Dir()
{
}

bool
Collection_Songlist_Dir::GetDeleteAfterPlay() const
{
  return mDeleteAfterPlay;
}

void
Collection_Songlist_Dir::Update()
{
  /* Temporary storage for new list */
  deque<Song> newlist;

  // Open directory
  QDir dir(mDirectory, QString::null, QDir::Time | QDir::Reversed,
	   QDir::Files | QDir::Readable);
  if (!dir.exists()) {
    throw StdException("Could not open directory '" + mDirectory + "'!");
  }

  // Iterate over files in directory
  const QFileInfoList* list = dir.entryInfoList();
  QFileInfoListIterator it(*list);
  QFileInfo* fi;

  /** \bug Something is wrong with the timestamp handling!! */

  while ((fi = it.current()) != 0 ) {
    try {
      if (mIsQueue) {
	if (fi->lastModified() >= mLastTimeStamp) {
	  QMutexLocker locker(&mMutex);
	  mSonglist.push_back(Song(fi->filePath().ascii()));
	  mLastTimeStamp = fi->lastModified();
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
  mLastTimeStamp.addSecs(1);
  
  /* Swap content */
  if (!mIsQueue) {
    QMutexLocker locker(&mMutex);
    mSonglist.swap(newlist);
  }
}
