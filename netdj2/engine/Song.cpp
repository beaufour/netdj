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

u_int32_t Song::NextUNID = 0;

Song::Song() {
  
}

Song::Song(const string fname)
  : Filename(fname)
{
  AssignUNID();
}

void
Song::AssignUNID() {
  UNID = NextUNID++;
}

u_int32_t
Song::GetUNID() const {
  return UNID;
}

string
Song::GetFilename() const {
  return Filename;
}

SongType
Song::GetSongType() const {
  // Filename must include ".mp3".
  // Tried using QFileInfo, but
  // f.x. "..mp3" returned ".mp3" instead of "mp3".
  if (Filename.size() > 4) {
    char ending[5];
    strcpy(ending, Filename.substr(Filename.size() - 4).c_str());
    ending[1] = tolower(ending[1]);
    ending[2] = tolower(ending[2]);
    if (strstr(ending, ".mp3") != 0) {
      return SongType_MP3;
    }
  }

  return SongType_Unknown;
}

SongInfo*
Song::GetSongInfo() const {
  SongInfo* si = 0;

  switch (GetSongType()) {
  case SongType_MP3:
    si = new SongInfo_File_mp3(Filename);
    break;

  default:
    break;
  }

  return si;
}
