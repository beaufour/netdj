/**
 * \file Collection_Songlist_Dir.cpp
 * Class Collection_Songlist_Dir.
 *
 * $Id$
 *
 */

#include "Collection_Songlist_Dir.h"

#include "SongFile.h"

#include <qdir.h>
#include <qptrlist.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>

using namespace std;
using namespace NetDJ;

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

void
Collection_Songlist_Dir::Update()
{
  /* Temporary storage for new list */
  deque<ISong*> newlist;

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

  SongFile* newsong;
  QDateTime fDate;
  while ((fi = it.current()) != 0 ) {
    try {
      if (fi->isSymLink()) {
	// If it is a symlink we have to lstat it...
	struct stat buffer;
	int status = lstat(fi->filePath().latin1(), &buffer);
	if (status == -1) {
	  continue; 
	}
	fDate.setTime_t(buffer.st_ctime);
      } else {
	fDate = fi->lastModified();
      }
      if (fDate >= mLastTimeStamp || !mIsQueue) {
	QMutexLocker locker(&mMutex);
	newsong = new SongFile(fi->filePath(), GetNewUNID());
	Q_CHECK_PTR(newsong);
	newsong->SetDeleteAfterPlay(mDeleteAfterPlay);
	mLastTimeStamp = fDate;
	if (mIsQueue) {
	  mSonglist.push_back(newsong);
	} else {
	  newlist.push_back(newsong);
	}
      }
      ++it;
    }
    catch (...) {
      qWarning("Could not add '%s' to collection '%s'!", fi->filePath().ascii(), GetIdentifier().latin1());
    }
  }

  /**
   * Add a second to timestamp, or last file(s) will show up in next
   * iteration.
   *
   * @note Can I do something smarter than add one second to the
   * timestamp?
   */
  mLastTimeStamp = mLastTimeStamp.addSecs(1);
  
  /* Swap content */
  if (!mIsQueue) {
    QMutexLocker locker(&mMutex);
    mSonglist.swap(newlist);
  }
}
