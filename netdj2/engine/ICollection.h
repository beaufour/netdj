// -*- c++ -*-
/**
 * \file ICollection.h
 * Interface ICollection.
 *
 * $Id$
 *
 */

#ifndef __ICOLLECTION_H__
#define __ICOLLECTION_H__

#include <string>
#include "StdException.h"
#include "IXMLOutput.h"

namespace NetDJ
{

  class ISong;

  /**
   * Exception thrown by GetNextSong if ICollection is empty.
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
  class ICollection : public IXMLOutput {
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
    virtual ISong* GetNextSong() = 0;
    
    
    /**
     * Get a song from the list.
     *
     * @note Hmmm, would like to use operator[], but how do you call
     * that function from a class funtion (in asXML?).
     *
     * @param aPos     The song number to get
     */
    virtual const ISong* GetSong(const int aPos) const = 0;
    
    
    /**
     * Add a song to the list.
     *
     * @param sng     The song to add
     */
    virtual bool AddSong(ISong* aSong) = 0;
  };
}

#endif
