// -*- c++ -*-
/**
 * \file SongInfo.h
 * Class SongInfo.
 *
 * $Id$
 *
 */

#ifndef __SONGINFO_H__
#define __SONGINFO_H__

#include <string>
#include <sys/types.h>
#include <qdom.h>

#include "StdException.h"
#include "XMLOutput.h"

namespace NetDJ
{
  
  /**
   * Exception thrown by SongInfo and sub-classes when no SongInfo can
   * be retrieved from Song.
   */
  class NoSongInfo : public StdException {
  public:
    /**
     * Constructor.
     *
     * @param str     Eventual error message
     */
    NoSongInfo(std::string aStr);
  };
  

  /**
   * Base class for song information
   *
   * Handles all information about the song: Title, artist, etc. Should
   * be implemented by classes that fill in the values.
   *
   * It is assumed that the information does not change, so all
   * information needs only to be fetched once, i.e. can be cached.
   */
  class SongInfo : public XMLOutput {
  protected:
    /** Textual description of song (ie. "Aha - Take On Me") */
    std::string mDescription;
    
    /** Song artist */
    std::string mArtist;
    
    /** From album */
    std::string mAlbum;
    
    /** Song title */
    std::string mTitle;
    
    /** Song genre */
    std::string mGenre;
    
    /** Song track */
    u_int32_t mTrack;
    
    /** Song release year */
    u_int32_t mYear;
    
    /** Song size (in bytes) */
    u_int32_t mSize;
    
    /** Song owner */
    std::string mOwner;
    
    /** Song length, in seconds */
    u_int32_t mLength;
    
  public:
    /**
     * Constructor.
     */
    SongInfo();
    
    /** Destructor */
    virtual ~SongInfo();
    
    /**
     * Get song description.
     *
     * @return        mDescription
     */
    std::string GetDescription() const;
    
    /**
     * Get song artist
     *
     * @return        mArtist
     */
    std::string GetArtist() const;
    
    /**
     * Get song album
     *
     * @return        mAlbum
     */
    std::string GetAlbum() const;
    
    /**
     * Get song title
     *
     * @return        mTitle
     */
    std::string GetTitle() const;
    
    /**
     * Get song genre
     *
     * @return       mGenre 
     */
    std::string GetGenre() const;
    
    /**
     * Get song track number
     *
     * @return       mTrack
     */
    u_int32_t GetTrack() const;
    
    /**
     * Get song release year
     *
     * @return       mYear
     */
    u_int32_t GetYear() const;
    
    /**
     * Get song size (in bytes)
     *
     * @return       mSize
     */
    u_int32_t GetSize() const;
    
    /**
     * Get song owner
     *
     * @return       mOwner
     */
    std::string GetOwner() const;
    
    /**
     * Get song length, in seconds
     *
     * @return       mSeconds
     */
    u_int32_t GetLength() const;
    
    void asXML(QDomDocument& aDoc, QDomElement& aRoot) const;
  };
}

#endif
