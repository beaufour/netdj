/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/**
 * \file SongInfo_File_mp3.cpp
 * Class SongInfo_File_mp3.
 *
 * $Id$
 *
 */

#include <cstdlib>
#include <qfileinfo.h>
#include <tag.h>
#include <fileref.h>
#include <tstring.h>

#include "SongInfo_File_mp3.h"

using namespace std;

SongInfo_File_mp3::SongInfo_File_mp3(const string aName)
  : SongInfo_File(aName)
{
  /* Initialize */
  TagLib::FileRef tfile(mFilename.c_str());

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
    mDescription = QFileInfo(mFilename).baseName().ascii();
  }
}


SongInfo_File_mp3::~SongInfo_File_mp3() {
}
