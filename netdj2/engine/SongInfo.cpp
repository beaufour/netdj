/**
 * \file SongInfo.cpp
 * Class SongInfo.
 *
 * $Id$
 *
 */

#include "SongInfo.h"

using namespace std;

NoSongInfo::NoSongInfo(string str)
  : StdException(str)
{
}


SongInfo::SongInfo()
  : Track(0), Year(0), Size(0)
{
}

SongInfo::~SongInfo() {
}

string
SongInfo::GetDescription() const {
  return Description;
}

string
SongInfo::GetArtist() const {
  return Artist;
}

string
SongInfo::GetAlbum() const {
  return Album;
}

string
SongInfo::GetTitle() const {
  return Title;
}

string
SongInfo::GetGenre() const {
  return Genre;
}

u_int32_t
SongInfo::GetTrack() const {
  return Track;
}

u_int32_t
SongInfo::GetYear() const {
  return Year;
}

u_int32_t
SongInfo::GetSize() const {
  return Size;
}

string
SongInfo::GetOwner() const {
  return Owner;
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
SongInfo::asXML(QDomDocument& doc, QDomElement& root) const {
  const int s_num = 6;
  const string* s_list[s_num] = {&Description, &Artist, &Album, &Title, &Genre, &Owner};
  const string s_list_names[s_num] = {"description", "artist", "album", "title", "genre", "owner"};
  const int i_num = 3;
  const u_int32_t* i_list[i_num] = {&Track, &Year, &Size};
  const string i_list_names[i_num] = {"track", "year", "size"};

  for (int i = 0; i < s_num; ++i) {
    if (*s_list[i] != "") {
      QDomElement elem = doc.createElement(s_list_names[i]);
      QDomText text = doc.createTextNode(*s_list[i]);
      elem.appendChild(text);
      root.appendChild(elem);
    }
  }

  QString tmpstr;
  for (int i = 0; i < i_num; ++i) {
    if (*i_list[i] != 0) {
      QDomElement elem = doc.createElement(i_list_names[i]);
      tmpstr.setNum(*i_list[i]);
      QDomText text = doc.createTextNode(tmpstr);
      elem.appendChild(text);
      root.appendChild(elem);
    }
  }
}
