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

SongInfoFile::SongInfoFile(const QString& aFilename)
{
  // Get file information
  QFileInfo qinfo(aFilename);

  mOwner = qinfo.owner().ascii();
  mSize = qinfo.size();


  // Get tag information with TagLib
  TagLib::FileRef tfile(aFilename.ascii());

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
    mDescription = qinfo.ascii();
  }



}

MediaType_t
SongInfo_File::GetMediaType() const {
  // Filename must include ".mp3".
  // Tried using QFileInfo, but
  // f.x. "..mp3" returned ".mp3" instead of "mp3".
  if (mFilename.size() > 4) {
    char ending[5];
    strcpy(ending, mFilename.substr(mFilename.size() - 4).c_str());
    ending[1] = tolower(ending[1]);
    ending[2] = tolower(ending[2]);
    if (strstr(ending, ".mp3") != 0) {
      return MediaType_MP3;
    }
  }

  return MediaType_Unknown;
}

