// -*- c++ -*-
/**
 * \file SongFile.h
 * Class SongFile.
 *
 * $Id$
 *
 */

#ifndef __SONGFILE_H__
#define __SONGFILE_H__

#include <string>
#include <qfileinfo.h>

#include "ISong.h"

namespace NetDJ
{
  
  /**
   * Implementation of ISong for songs stored in files
   */
  class SongFile : public ISong
  {
  private:
    /** The filename */
    std::string mFilename;
    
    /** Pointer to SongInfo structure, may be 0 */
    mutable SongInfo* mSongInfo;
    
    /** Unique ID (application-wise) */
    u_int32_t mUNID;
    
  public:
    /**
     * Constructor.
     *
     * @param fname   The filename of the song
     */
    SongFile(const std::string fname,  const u_int32_t aUNID);
    
    /** Destructor */
    ~SongFile();
    
    /**
     * Copy constructor.
     *
     * @param s2      New song content.
     */
    SongFile(const SongFile& aSong2);
    
    /**
     * Assignment. 
     *
     * @param s2      New song content.
     * @return        Reference to self.
     */
    SongFile& operator=(const SongFile& aSong2);

    // ISong
    u_int32_t GetUNID() const;
    MediaType_t GetMediaType() const;
    const SongInfo* GetSongInfo() const;    

    // IXMLOutput
    const QDomElement& AsXML() const;
  };
}

#endif
