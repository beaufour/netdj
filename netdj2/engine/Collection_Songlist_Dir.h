// -*- c++ -*-
/**
 * \file Collection_Songlist_Dir.h
 * Class Collection_Songlist_Dir.
 *
 * $Id$
 *
 */

#ifndef __COLLECTION_SONGLIST_DIR_H__
#define __COLLECTION_SONGLIST_DIR_H__

#include "Collection_Songlist.h"

/**
 * Class for collections maintained by a list of songs, read from a
 * directory.
 */
class Collection_Songlist_Dir : public Collection_Songlist {
protected:
  /** The directory with the songs */
  std::string Directory;

  /**
   * Date/time of last file added to songlist
   *
   * Used when isQueue == true, to only add newer files.
   */
  QDateTime LastTimeStamp;

public:
  /**
   * Constructor.
   * 
   * @param id      The identifier
   * @param descr   The description
   * @param dir     The directory containing songs
   * @param isq     Is the songlist a queue?
   */
  Collection_Songlist_Dir(const std::string id, const std::string descr,
			  const std::string dir, const bool isq);

  /** Destructor */
  ~Collection_Songlist_Dir();

  void Update();
};

#endif
