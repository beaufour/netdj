/**
 * \file SongInfoBase.cpp
 * Class SongInfoBase.
 *
 * $Id$
 *
 */

#include "SongInfoBase.h"

using namespace NetDJ;

NoSongInfo::NoSongInfo(std::string aStr)
  : StdException(aStr, "NoSongInfo")
{
}


SongInfoBase::SongInfoBase()
  : mXMLValid(false),mDocument("SongInfoBase"), 
    mTrack(0), mYear(0), mSize(0), mLength(0), mMediaType(MediaType_Unknown)
{
}

SongInfoBase::~SongInfoBase()
{
}

QString
SongInfoBase::GetDescription() const
{
  return mDescription;
}

QString
SongInfoBase::GetArtist() const
{
  return mArtist;
}

QString
SongInfoBase::GetAlbum() const
{
  return mAlbum;
}

QString
SongInfoBase::GetTitle() const
{
  return mTitle;
}

QString
SongInfoBase::GetGenre() const
{
  return mGenre;
}

u_int32_t
SongInfoBase::GetTrack() const
{
  return mTrack;
}

u_int32_t
SongInfoBase::GetYear() const
{
  return mYear;
}

u_int32_t
SongInfoBase::GetSize() const
{
  return mSize;
}

QString
SongInfoBase::GetOwner() const
{
  return mOwner;
}

u_int32_t
SongInfoBase::GetLength() const
{
  return mLength;
}

MediaType_t
SongInfoBase::GetMediaType() const
{
  return mMediaType;
}

QDomElement*
SongInfoBase::AsXML(QDomDocument* aDocument) const
{
  if (!mXMLValid) {
    CreateXML();
    mXMLValid = true;
  }

  QDomElement* info = new QDomElement();
  *info = aDocument->importNode(mInfoNode, true).toElement();

  return info;
}

/**
 * @note Assumes that 0 or "" == "no info"
 */
void
SongInfoBase::CreateXML() const
{
  // Create main node
  mInfoNode = mDocument.createElement("songinfo");
  mDocument.appendChild(mInfoNode);

  // Handle mediatype
  QDomElement elem = mDocument.createElement("mediatype");
  QDomText text;
  switch (mMediaType) {
    case MediaType_MP3:
      text = mDocument.createTextNode("MP3");
      break;
      
    case MediaType_OGG:
      text = mDocument.createTextNode("OGG");
      break;
      
    default:
      text = mDocument.createTextNode("unknown");
      break;
      
  }  
  elem.appendChild(text);
  mInfoNode.appendChild(elem);

  // Handle string types
  const int s_num = 6;
  const QString* s_list[s_num] = {&mDescription, &mArtist, &mAlbum, &mTitle, &mGenre, &mOwner};
  const QString s_list_names[s_num] = {"description", "artist", "album", "title", "genre", "owner"};
  for (int i = 0; i < s_num; ++i) {
    if (*s_list[i] != "") {
      QDomElement elem = mDocument.createElement(s_list_names[i]);
      QDomText text = mDocument.createTextNode(*s_list[i]);
      elem.appendChild(text);
      mInfoNode.appendChild(elem);
    }
  }

  // Handle numeric types
  const int i_num = 4;
  const u_int32_t* i_list[i_num] = {&mTrack, &mYear, &mSize, &mLength};
  const QString i_list_names[i_num] = {"track", "year", "size", "length"};
  QString tmpstr;
  for (int i = 0; i < i_num; ++i) {
    if (*i_list[i] != 0) {
      QDomElement elem = mDocument.createElement(i_list_names[i]);
      tmpstr.setNum(*i_list[i]);
      QDomText text = mDocument.createTextNode(tmpstr);
      elem.appendChild(text);
      mInfoNode.appendChild(elem);
    }
  }
}
