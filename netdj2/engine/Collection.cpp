/**
 * \file Collection.cpp
 * Class Collection.
 *
 * $Id$
 *
 */

#include "Collection.h"

using namespace std;

EmptyCollection::EmptyCollection(const std::string str)
  : StdException(str) {

}


Collection::Collection(const string id, const string descr)
  : Identifier(id), Description(descr)
{
}


Collection::~Collection() {
}


string
Collection::GetIdentifier() const {
  return Identifier;
}


string
Collection::GetDescription() const {
  return Description;
}


void
Collection::asXML(QDomDocument& doc, QDomElement& root) const {
  /* Main node */
  QDomElement col = doc.createElement("collection");
  col.setAttribute("id", GetIdentifier());
  root.appendChild(col);

  /* Description */
  QDomElement col_descr = doc.createElement("description");
  col.appendChild(col_descr);
  QDomText col_descr_text = doc.createTextNode(GetDescription());
  col_descr.appendChild(col_descr_text);

  /* Size */
  QDomElement col_size = doc.createElement("size");
  col.appendChild(col_size);
  QString col_size_str;
  col_size_str.setNum(Size());
  QDomText col_size_text = doc.createTextNode(col_size_str);
  col_size.appendChild(col_size_text);
  

  /* Songs */
  for (int i = 0; i < Size(); ++i) {
    QDomElement elem = doc.createElement("entry");
    elem.setAttribute("position", i);
    col.appendChild(elem);
    GetSong(i).asXML(doc, elem);
  }
}
