// -*- c++ -*-
/**
 * \file Collections.h
 * Class Collections
 *
 * $Id$
 *
 */

#ifndef __COLLECTIONS_H__
#define __COLLECTIONS_H__

#include <vector>

#include <qstring.h>

namespace NetDJ
{
  class ICollection;
  class ISong;
  
  /**
   * Handles a collection of Collection(s).
   *
   * It handles how to get the next song from the right Collection. For
   * the moment it is just a prioritzed list, where the songs are taken
   * from the first non-empty Collection.
   *
   * The collections are prioritized according to the order they are
   * added. First one added is the highest.
   *
   * In the future it could have different collections with a equal
   * priority/probability of being chosen.
   *
   */
  class Collections {
  private:
    /** The list of Collections */
    std::vector<ICollection*> mColList;
    
  public:
    /** Constructor. */
    Collections();
    
    /**
     * Destructor.
     *
     * delete's the Collections handled.
     */
    ~Collections();
    
    /**
     * Add a new Collection.
     *
     * @param aCol              The new Collection to add.
     *
     */
    void AddCollection(ICollection* aCol);

    ICollection* GetCollection(const QString& aName);
    
    /**
     * Get the next song to play.
     *
     * @param aSong             The next song.
     * @param aCol              The collection the song is retrieved from
     * @return                  Song succesfully retrieved?
     */
    bool GetNextSong(ISong** aSong, const ICollection** aCol);
  };
}

#endif
