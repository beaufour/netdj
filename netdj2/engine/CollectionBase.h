// -*- c++ -*-
/**
 * \file CollectionBase.h
 * Class CollectionBase.
 *
 * $Id$
 *
 */

#ifndef __COLLECTIONBASE_H__
#define __COLLECTIONBASE_H__

#include <string>

#include "ICollection.h"
#include "Song.h"
#include "StdException.h"
#include "XMLOutput.h"

namespace NetDJ
{

  /**
   * Base implementation of Collection.
   */
  class CollectionBase : public Collection
  {
  protected:
    /** Identifier for the collection, short id */
    std::string mIdentifier;
    
    /** The description, long textual*/
    std::string mDescription;
    
  public:
    CollectionBase(const std::string aId, const std::string aDescr);
    virtual ~CollectionBase();
    
    std::string GetIdentifier() const;
    std::string GetDescription() const;
    bool GetDeleteAfterPlay() const;
    void asXML(QDomDocument& aDoc, QDomElement& aRoot) const;
  };
}

#endif
