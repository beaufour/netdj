// -*- c++ -*-
/**
 * \file Collection_Songlist.h
 * Class Collection_Songlist.
 *
 * $Id$
 *
 */

#ifndef __COLLECTION_SONGLIST_H__
#define __COLLECTION_SONGLIST_H__

#include "CollectionBase.h"

#include <deque>
#include <qmutex.h>

namespace NetDJ
{

  /**
   * Base class for collections maintained by a list of songs.
   */
  class Collection_Songlist : public CollectionBase {
  protected:
    /** List of songs managed by collection */
    std::deque<ISong*> mSonglist;
    
    /**
     * Determines whether collecgion is treated like a queue: Any new
     * songs in the directory are just added to the list when Update
     * is called.
     */
    bool mIsQueue;
    
    /** Mutex used for serialization */
    mutable QMutex mMutex;
    
  public:
    /**
     * Constructor.
     * 
     * @param aId      The identifier
     * @param aDescr   The description
     * @param aIsQ     Is the songlist a queue?
     */
    Collection_Songlist(const std::string aId, const std::string aDescr,
			const bool aIsQ);
    
    /** Destructor */
    ~Collection_Songlist();

    // ICollection
    virtual void Update();
    int Size() const;
    ISong* GetNextSong();
    const ISong* GetSong(const int aPos) const;
    bool AddSong(ISong* aSong);
  };
}

#endif
