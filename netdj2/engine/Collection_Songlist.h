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

#include "Collection.h"
#include "Song.h"
#include <deque>
#include <qmutex.h>


/**
 * Base class for collections maintained by a list of songs.
 */
class Collection_Songlist : public Collection {
protected:
  /** List of songs managed by collection */
  std::deque<Song> Songlist;

  /**
   * Determines whether collecgion is treated like a queue: Any new
   * songs in the directory are just added to the list when Update
   * is called.
   */
  bool isQueue;

  /** Mutex used for serialization */
  mutable QMutex mutex;

public:
  /**
   * Constructor.
   * 
   * @param id      The identifier
   * @param descr   The description
   * @param isq     Is the songlist a queue?
   */
  Collection_Songlist(const std::string id, const std::string descr,
		      const bool isq);

  /** Destructor */
  ~Collection_Songlist();

  virtual void Update();
  int Size() const;
  Song GetNextSong();
  Song& operator[] (const int pos);
  bool AddSong(const Song& sng);
};

#endif
