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
