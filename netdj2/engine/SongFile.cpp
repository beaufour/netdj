/**
 * \file SongFile.cpp
 * Class SongFile.
 *
 * $Id$
 *
 */

#include "SongFile.h"
#include "SongInfoFile.h"

#include <qdom.h>

using namespace NetDJ;

SongFile::SongFile(const QString& aName, const u_int32_t aUNID)
  : mFilename(aName), mSongInfo(0), mUNID(aUNID)
{
}


SongFile::~SongFile()
{
  if (mSongInfo) {
    delete mSongInfo;
  }
}

SongFile::SongFile(const SongFile& aSong2)
  : ISong(), mFilename(aSong2.mFilename),
    mSongInfo(0), mUNID(aSong2.mUNID),
    mDeleteAfterPlay(false)
{
}

SongFile&
SongFile::operator=(const SongFile& aSong2)
{
  mFilename = aSong2.mFilename;
  mUNID = aSong2.mUNID;
  mSongInfo = 0;
  return *this;
}

u_int32_t
SongFile::GetUNID() const
{
  return mUNID;
}

const ISongInfo*
SongFile::GetSongInfo() const
{
  if (!mSongInfo) {
    mSongInfo = new SongInfoFile(mFilename);
    Q_CHECK_PTR(mSongInfo);
  }
  
  return mSongInfo;
}

QString
SongFile::GetFilename() const
{
  return mFilename;
}

bool
SongFile::GetDeleteAfterPlay() const
{
  return mDeleteAfterPlay;
}

void
SongFile::SetDeleteAfterPlay(bool aVal)
{
  mDeleteAfterPlay = aVal;
}

QDomElement*
SongFile::AsXML(QDomDocument* aDocument) const
{
  QDomElement* song = new QDomElement();
  Q_CHECK_PTR(song);
  
  *song = aDocument->createElement("song");
  song->setAttribute("unid", GetUNID());

  if (GetSongInfo()) {
    QDomElement* info = mSongInfo->AsXML(aDocument);
    song->appendChild(*info);
    delete info;
  }

  return song;
}
