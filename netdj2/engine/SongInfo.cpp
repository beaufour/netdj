/**
 * \file SongInfo.cpp
 * Class SongInfo.
 *
 * $Id$
 *
 */

#include "SongInfo.h"

using namespace std;

NoSongInfo::NoSongInfo(string aStr)
  : StdException(aStr)
{
}


SongInfo::SongInfo()
  : mTrack(0), mYear(0), mSize(0), mLength(0)
{
}

SongInfo::~SongInfo() {
}

string
SongInfo::GetDescription() const {
  return mDescription;
}

string
SongInfo::GetArtist() const {
  return mArtist;
}

string
SongInfo::GetAlbum() const {
  return mAlbum;
}

string
SongInfo::GetTitle() const {
  return mTitle;
}

string
SongInfo::GetGenre() const {
  return mGenre;
}

u_int32_t
SongInfo::GetTrack() const {
  return mTrack;
}

u_int32_t
SongInfo::GetYear() const {
  return mYear;
}

u_int32_t
SongInfo::GetSize() const {
  return mSize;
}

string
SongInfo::GetOwner() const {
  return mOwner;
}

u_int32_t
SongInfo::GetLength() const {
  return mLength;
}

/**
   \todo The XML created for SongInfo really needs to be cached, and
    not recreated all the time. Problem is that QDomElement does not
    have an asString() and everything "belongs" to the QDomDocument
    owned by the caller.

    \note SongInfo::asXML assumes that Year, Track, and Size == 0 is
    empty values.
*/
void
SongInfo::asXML(QDomDocument& aDoc, QDomElement& aRoot) const {
  const int s_num = 6;
  const string* s_list[s_num] = {&mDescription, &mArtist, &mAlbum, &mTitle, &mGenre, &mOwner};
  const string s_list_names[s_num] = {"description", "artist", "album", "title", "genre", "owner"};
  const int i_num = 4;
  const u_int32_t* i_list[i_num] = {&mTrack, &mYear, &mSize, &mLength};
  const string i_list_names[i_num] = {"track", "year", "size", "length"};

  for (int i = 0; i < s_num; ++i) {
    if (*s_list[i] != "") {
      QDomElement elem = aDoc.createElement(s_list_names[i]);
      QDomText text = aDoc.createTextNode(*s_list[i]);
      elem.appendChild(text);
      aRoot.appendChild(elem);
    }
  }

  QString tmpstr;
  for (int i = 0; i < i_num; ++i) {
    if (*i_list[i] != 0) {
      QDomElement elem = aDoc.createElement(i_list_names[i]);
      tmpstr.setNum(*i_list[i]);
      QDomText text = aDoc.createTextNode(tmpstr);
      elem.appendChild(text);
      aRoot.appendChild(elem);
    }
  }
}
