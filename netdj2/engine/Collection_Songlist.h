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
#include "Song.h"
#include <deque>
#include <qmutex.h>


/**
 * Base class for collections maintained by a list of songs.
 */
class Collection_Songlist : public CollectionBase {
protected:
  /** List of songs managed by collection */
  std::deque<Song> mSonglist;

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
   * @param id      The identifier
   * @param descr   The description
   * @param isq     Is the songlist a queue?
   */
  Collection_Songlist(const std::string aId, const std::string aDescr,
		      const bool aIsQ);

  /** Destructor */
  ~Collection_Songlist();

  virtual void Update();
  int Size() const;
  Song GetNextSong();
  const Song& GetSong(const int aPos) const;
  bool AddSong(const Song& aSong);
};

#endif
