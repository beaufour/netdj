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

#include <qdatetime.h>

namespace NetDJ
{

  /**
   * Class for collections maintained by a list of songs, read from a
   * directory.
   */
  class Collection_Songlist_Dir : public Collection_Songlist
  {
  protected:
    /** The directory with the songs */
    std::string mDirectory;
    
    /**
     * Date/time of last file added to songlist
     *
     * Used when isQueue == true, to only add newer files.
     */
    QDateTime mLastTimeStamp;
    
    /** Whether to delete files from file system after playing them */
    bool mDeleteAfterPlay;
    
  public:
    /**
     * Constructor.
     * 
     * @param aId               The identifier
     * @param aDescr            The description
     * @param aDir              The directory containing songs
     * @param aIsQ              Is the songlist a queue?
     * @param aDelete           Delete files after play?
     */
    Collection_Songlist_Dir(const std::string aId, const std::string aDescr,
			    const std::string aDir, const bool aIsQ,
			    const bool aDelete = false);
    
    /** Destructor */
    ~Collection_Songlist_Dir();

    // ICollection
    void Update();
  };
}

#endif
