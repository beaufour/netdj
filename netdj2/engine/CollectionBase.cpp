/**
 * \file CollectionBase.cpp
 * Class CollectionBase.
 *
 * $Id$
 *
 */

#include "CollectionBase.h"

using namespace std;

EmptyCollection::EmptyCollection(const std::string aStr)
  : StdException(aStr)
{
}


CollectionBase::CollectionBase(const string aId, const string aDescr)
  : mIdentifier(aId), mDescription(aDescr)
{
}


CollectionBase::~CollectionBase()
{
}


string
CollectionBase::GetIdentifier() const
{
  return mIdentifier;
}


string
CollectionBase::GetDescription() const
{
  return mDescription;
}

bool
CollectionBase::GetDeleteAfterPlay() const
{
  return false;
}


void
CollectionBase::asXML(QDomDocument& aDoc, QDomElement& aRoot) const
{
  /* Main node */
  QDomElement col = aDoc.createElement("collection");
  col.setAttribute("id", GetIdentifier());
  aRoot.appendChild(col);

  /* Description */
  QDomElement col_descr = aDoc.createElement("description");
  col.appendChild(col_descr);
  QDomText col_descr_text = aDoc.createTextNode(GetDescription());
  col_descr.appendChild(col_descr_text);

  /* Size */
  QDomElement col_size = aDoc.createElement("size");
  col.appendChild(col_size);
  QString col_size_str;
  col_size_str.setNum(Size());
  QDomText col_size_text = aDoc.createTextNode(col_size_str);
  col_size.appendChild(col_size_text);
  

  /* Songs */
  for (int i = 0; i < Size(); ++i) {
    QDomElement elem = aDoc.createElement("entry");
    elem.setAttribute("position", i);
    col.appendChild(elem);
    GetSong(i).asXML(aDoc, elem);
  }
}
