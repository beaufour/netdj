// -*- c++ -*-
/**
 * \file SongInfo_File.h
 * Class SongInfo_File.
 *
 * $Id$
 *
 */

#ifndef __SONGINFO_FILE_H__
#define __SONGINFO_FILE_H__

#include "SongInfo.h"
#include <string>

namespace NetDJ
{

  /**
   * Handles song informations for a file, that is fills out Owner and
   * Size.
   */
  class SongInfo_File : public SongInfo {
  protected:
    /** The filename of the file */
    std::string mFilename;
    
  public:
    /**
     * Constructor.
     *
     * @param fname   The filename of the file
     */
    SongInfo_File(const std::string aName);

    
    /** Destructor */
    virtual ~SongInfo_File();
  };
}

#endif
