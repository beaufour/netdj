// -*- c++ -*-
/**
 * \file Collection.h
 * Class Collection.
 *
 * $Id$
 *
 */

#ifndef __COLLECTION_H__
#define __COLLECTION_H__

#include <string>
#include "Song.h"
#include "StdException.h"
#include "XMLOutput.h"

namespace NetDJ
{

  /**
   * Exception thrown by GetNextSong if Collection is empty.
   */
  class EmptyCollection : public StdException {
  public:
    EmptyCollection(const std::string aStr);
  };


  /**
   * Interface for collections of songs.
   *
   * Goals: Handle everything about a collection of songs
   * - Identifier (string)
   * - Textual description
   * - Number of songs
   * - Retrieve next song to play
   * - Retrieve song at specific position
   * - Retrieve list of all songs?
   */
  class Collection : public XMLOutput {
  public:
    /**
     * Get the identifier for the collection.
     *
     * @return        The identifier
     */
    virtual std::string GetIdentifier() const = 0;
    
    /**
     * Get the description for the collection.
     *
     * @return        The description
     */
    virtual std::string GetDescription() const = 0;
    
    
    /**
     * Get whether songs from this collection should be deleted from the
     * filesystem after being played.
     *
     * @return        Delete after play?
     */
    virtual bool GetDeleteAfterPlay() const = 0;
    
    
    /**
     * Get number of songs in collection
     *
     * @return        The number of songs
     */
    virtual int Size() const = 0;
    
    
    /**
     * Update the collection (new/deleted songs, etc.)
     */
    virtual void Update() = 0;
    
    /**
     * Get next song to play from collection
     *
     * @return        The next song
     */
    virtual Song GetNextSong() = 0;
    
    
    /**
     * Get a song from the list.
     *
     * \note Hmmm, would like to use operator[], but how do you call
     * that function from a class funtion (in asXML?).
     *
     * @param pos     The song number to get
     */
    virtual const Song& GetSong(const int aPos) const = 0;
    
    
    /**
     * Add a song to the list.
     *
     * @param sng     The song to add
     */
    virtual bool AddSong(const Song& aSong) = 0;
  };
}

#endif
