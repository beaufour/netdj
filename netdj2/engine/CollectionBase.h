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

namespace NetDJ
{

  /**
   * Base implementation of ICollection.
   */
  class CollectionBase : public ICollection
  {
  private:
    /** The next UNID */
    u_int32_t mNextUNID;
    
  protected:
    /** Identifier for the collection, short id */
    std::string mIdentifier;
    
    /** The description, long textual*/
    std::string mDescription;

    /** Get a UNID */
    u_int32_t GetNewUNID();

  public:
    CollectionBase(const std::string aId, const std::string aDescr);
    virtual ~CollectionBase();
    
    // ICollection
    std::string GetIdentifier() const;
    std::string GetDescription() const;

    // IXMLOutput
    QDomElement* AsXML(QDomDocument* aDocument) const;
  };
}

#endif
