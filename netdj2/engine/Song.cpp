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

Song::Song()
  : sInfo(0) {
  
}

Song::Song(const string fname)
  : Filename(fname), sInfo(0)
{
  AssignUNID();
}


Song::~Song() {
  if (sInfo) {
    delete sInfo;
  }
}

Song::Song(const Song& s2)
  : XMLOutput(), Filename(s2.Filename), sInfo(0), UNID(s2.UNID) {
}

Song&
Song::operator=(const Song& s2) {
  Filename = s2.Filename;
  UNID = s2.UNID;
  sInfo = 0;
  return *this;
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

const SongInfo*
Song::GetSongInfo() const {
  if (!sInfo) {
    switch (GetSongType()) {
    case SongType_MP3:
      sInfo = new SongInfo_File_mp3(Filename);
      break;
      
    default:
      break;
    }
  }

  return sInfo;
}


void
Song::asXML(QDomDocument& doc, QDomElement& root) const {
  QDomElement song = doc.createElement("song");
  root.appendChild(song);
  song.setAttribute("unid", GetUNID());

  QDomElement fname = doc.createElement("filename");
  song.appendChild(fname);
  QDomText fname_text = doc.createTextNode(GetFilename());
  fname.appendChild(fname_text);

  
  if (GetSongInfo()) {
    sInfo->asXML(doc, song);
  }
}
