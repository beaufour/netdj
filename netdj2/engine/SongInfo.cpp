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

void
SongInfo::asXML(QDomDocument& doc, QDomElement& root) const {
  const int s_num = 5;
  const string* s_list[s_num] = {&Artist, &Album, &Title, &Genre, &Owner};
  const string s_list_names[s_num] = {"Artist", "Album", "Title", "Genre", "Owner"};
  const int i_num = 3;
  const u_int32_t* i_list[i_num] = {&Track, &Year, &Size};
  const string i_list_names[i_num] = {"Track", "Year", "Size"};

  for (int i = 0; i < s_num; ++i) {
    QDomElement elem = doc.createElement(s_list_names[i]);
    QDomText text = doc.createTextNode(*s_list[i]);
    elem.appendChild(text);
    root.appendChild(elem);
  }

  QString tmpstr;
  for (int i = 0; i < i_num; ++i) {
    QDomElement elem = doc.createElement(i_list_names[i]);
    tmpstr.setNum(*i_list[i]);
    QDomText text = doc.createTextNode(tmpstr);
    elem.appendChild(text);
    root.appendChild(elem);
  }

  /*
  info.setAttribute("Artist", Artist);
  info.setAttribute("Album", Album);
  info.setAttribute("Title", Title);
  info.setAttribute("Genre", Genre);
  info.setAttribute("Track", Track);
  info.setAttribute("Year", Year);
  info.setAttribute("Size", Size);
  info.setAttribute("Owner", Owner);
  */
}
