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

/**
 * Exception thrown by GetNextSong if Collection is empty.
 */
class EmptyCollection : public StdException {
public:
  EmptyCollection(const std::string str);
};

/**
 * Base class for collections of songs.
 *
 * Goals: Handle everything about a collection of songs
 * - Identifier (string)
 * - Textual description
 * - Number of songs
 * - Retrieve next song to play
 * - Retrieve list of all songs
 */
class Collection {
protected:
  /** Identifier for the collection, short id */
  std::string Identifier;


  /** The description, long textual*/
  std::string Description;


public:
  /**
   * Constructor.
   *
   * @param id      The identifier
   * @param descr   The description
   */
  Collection(const std::string id, const std::string descr);

  /**
   * Destructor.
   */
  virtual ~Collection();

  /**
   * Get the identifier for the collection.
   *
   * @return        The identifier
   */
  std::string GetIdentifier() const;


  /**
   * Get the description for the collection.
   *
   * @return        The description
   */
  std::string GetDescription() const;


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
   * @param pos     The song number to get
   */
  virtual Song& operator[] (const int pos) = 0;


  /**
   * Add a song to the list.
   *
   * @param sng     The song to add
   */
  virtual bool AddSong(const Song& sng) = 0;
};

#endif
