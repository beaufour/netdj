// -*- c++ -*-
/**
 * \file Collection_Songlist_File.h
 * Class Collection_Songlist_File.
 *
 * $Id$
 *
 */

#ifndef __COLLECTION_SONGLIST_FILE_H__
#define __COLLECTION_SONGLIST_FILE_H__

#include "Collection_Songlist.h"

#include <qdatetime.h>

namespace NetDJ
{

  /**
   * Class for collections maintained by a list of songs, read from a
   * file.
   */
  class Collection_Songlist_File : public Collection_Songlist {
  protected:
    /** Filename with the file list */
    std::string mFilename;

    /** Date of file at last read */
    QDateTime mFiledate;
    
  public:
    /**
     * Constructor.
     * 
     * @param aId      The identifier
     * @param aDescr   The description
     * @param aName    The filename containing the list of songs
     */
    Collection_Songlist_File(const std::string aId, const std::string aDescr,
			     const std::string aName);
    
    /** Destructor */
    ~Collection_Songlist_File();
    
    void Update();
  };
}

#endif
