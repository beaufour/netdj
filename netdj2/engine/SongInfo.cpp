/**
 * \file SongInfo.cpp
 * Class SongInfo.
 *
 * $Id$
 *
 */

#include "SongInfo.h"
#include <qfileinfo.h>

using namespace std;

NoSongInfo::NoSongInfo(string str)
  : StdException(str)
{
}


SongInfo::SongInfo()
  : Track(0), Year(0)
{
}

SongInfo::~SongInfo() {
}

string
SongInfo::GetDescription() {
  return Description;
}

string
SongInfo::GetArtist() {
  return Artist;
}

string
SongInfo::GetAlbum() {
  return Album;
}

string
SongInfo::GetTitle() {
  return Title;
}

string
SongInfo::GetGenre() {
  return Genre;
}

u_int32_t
SongInfo::GetTrack() {
  return Track;
}

u_int32_t
SongInfo::GetYear() {
  return Year;
}
