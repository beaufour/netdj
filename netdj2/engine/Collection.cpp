/**
 * \file Collection.cpp
 * Class Collection.
 *
 * $Id$
 *
 */

#include "Collection.h"

using namespace std;

EmptyCollection::EmptyCollection(const std::string aStr)
  : StdException(aStr) {

}


Collection::Collection(const string aId, const string aDescr)
  : mIdentifier(aId), mDescription(aDescr)
{
}


Collection::~Collection() {
}


string
Collection::GetIdentifier() const {
  return mIdentifier;
}


string
Collection::GetDescription() const {
  return mDescription;
}


void
Collection::asXML(QDomDocument& aDoc, QDomElement& aRoot) const {
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
