/**
 * \file Song.cpp
 * Class Song.
 *
 * $Id$
 *
 */

#include "Song.h"
#include "SongInfo_File_mp3.h"

#include <ctype.h>
#include <cstring>

using namespace std;

u_int32_t Song::sNextUNID = 0;

Song::Song()
  : mSongInfo(0) {
  
}

Song::Song(const string aName)
  : mFilename(aName), mSongInfo(0)
{
  AssignUNID();
}


Song::~Song() {
  if (mSongInfo) {
    delete mSongInfo;
  }
}

Song::Song(const Song& aSong2)
  : XMLOutput(), mFilename(aSong2.mFilename), mSongInfo(0), mUNID(aSong2.mUNID) {
}

Song&
Song::operator=(const Song& aSong2) {
  mFilename = aSong2.mFilename;
  mUNID = aSong2.mUNID;
  mSongInfo = 0;
  return *this;
}

void
Song::AssignUNID() {
  mUNID = sNextUNID++;
}

u_int32_t
Song::GetUNID() const {
  return mUNID;
}

string
Song::GetFilename() const {
  return mFilename;
}

SongType
Song::GetSongType() const {
  // Filename must include ".mp3".
  // Tried using QFileInfo, but
  // f.x. "..mp3" returned ".mp3" instead of "mp3".
  if (mFilename.size() > 4) {
    char ending[5];
    strcpy(ending, mFilename.substr(mFilename.size() - 4).c_str());
    ending[1] = tolower(ending[1]);
    ending[2] = tolower(ending[2]);
    if (strstr(ending, ".mp3") != 0) {
      return SongType_MP3;
    }
  }

  return SongType_Unknown;
}

const SongInfo*
Song::GetSongInfo() const {
  if (!mSongInfo) {
    switch (GetSongType()) {
    case SongType_MP3:
      mSongInfo = new SongInfo_File_mp3(mFilename);
      Q_CHECK_PTR(mSongInfo);
      break;
      
    default:
      break;
    }
  }

  return mSongInfo;
}


void
Song::asXML(QDomDocument& aDoc, QDomElement& aRoot) const {
  QDomElement song = aDoc.createElement("song");
  aRoot.appendChild(song);
  song.setAttribute("unid", GetUNID());

  QDomElement fname = aDoc.createElement("filename");
  song.appendChild(fname);
  QDomText fname_text = aDoc.createTextNode(GetFilename());
  fname.appendChild(fname_text);

  
  if (GetSongInfo()) {
    mSongInfo->asXML(aDoc, song);
  }
}
