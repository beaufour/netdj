/**
 * \file SongInfoFile.cpp
 * Class SongInfoFile.
 *
 * $Id$
 *
 */

#include "SongInfoFile.h"

#include <qfileinfo.h>

#include <tag.h>
#include <fileref.h>
#include <tstring.h>

using namespace NetDJ;

/**
 * @todo How do I get mediatype from Taglib?
 */
SongInfoFile::SongInfoFile(const QString& aFilename)
{
  // Get file information
  QFileInfo qinfo(aFilename);

  if (!qinfo.isReadable()) {
    return;
  }

  mOwner = qinfo.owner();
  mSize = qinfo.size();

  // Get tag information with TagLib
  TagLib::FileRef tfile(aFilename);

  // TODO: Use unicode in to8Bit() call?
  mAlbum = tfile.tag()->album().to8Bit();
  mTitle = tfile.tag()->title().to8Bit();
  mArtist = tfile.tag()->artist().to8Bit();
  mGenre = tfile.tag()->genre().to8Bit();
  mTrack = tfile.tag()->track();
  mYear = tfile.tag()->year();
  mLength = tfile.audioProperties()->length();

  if (mTitle != "" && mArtist != "") {
    mDescription = mArtist + " - " + mTitle;
  } else {
    /* Set description to file basename */
    mDescription = qinfo.fileName();
  }

  // Get mediatype
  // Tried using QFileInfo, but
  // f.x. "..mp3" returned ".mp3" instead of "mp3".
  if (aFilename.right(3).lower() == "mp3") {
    mMediaType = MediaType_MP3;
  }
}
