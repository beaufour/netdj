/**
 * \file CollectionBase.cpp
 * Class CollectionBase.
 *
 * $Id$
 *
 */

#include "CollectionBase.h"

#include <qdom.h>

#include "ISong.h"

using namespace std;
using namespace NetDJ;

EmptyCollection::EmptyCollection(const std::string aStr)
  : StdException(aStr, "EmptyCollection")
{
}


CollectionBase::CollectionBase(const string aId, const string aDescr)
  : mNextUNID(0), mIdentifier(aId), mDescription(aDescr)
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

u_int32_t
CollectionBase::GetUNID()
{  
  return ++mNextUNID;
}

bool
CollectionBase::GetDeleteAfterPlay() const
{
  return false;
}


QDomElement*
CollectionBase::AsXML(QDomDocument* aDocument) const
{
  /* Main node */
  QDomElement* col = new QDomElement();
  *col = aDocument->createElement("collection");
  col->setAttribute("id", GetIdentifier());

  /* Description */
  QDomElement col_descr = aDocument->createElement("description");
  col->appendChild(col_descr);
  QDomText col_descr_text = aDocument->createTextNode(GetDescription());
  col_descr.appendChild(col_descr_text);

  /* Size */
  QDomElement col_size = aDocument->createElement("size");
  col->appendChild(col_size);
  QString col_size_str;
  col_size_str.setNum(Size());
  QDomText col_size_text = aDocument->createTextNode(col_size_str);
  col_size.appendChild(col_size_text);
  

  /* Songs */
  QDomElement* song;
  for (int i = 0; i < Size(); ++i) {
    QDomElement elem = aDocument->createElement("entry");
    elem.setAttribute("position", i);
    song = GetSong(i)->AsXML(aDocument);
    elem.appendChild(*song);
    col->appendChild(elem);
    delete song;
  }

  return col;
}
