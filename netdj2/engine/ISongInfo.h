// -*- c++ -*-
/**
 * \file ISongInfo.h
 * Interface ISongInfo.
 *
 * $Id$
 *
 */

#ifndef __ISONGINFO_H__
#define __ISONGINFO_H__

#include <qstring.h>

#include "StdException.h"
#include "IXMLOutput.h"

namespace NetDJ
{
  
  /**
   * Defines the different media types
   */
  typedef enum {
    MediaType_Unknown,
    MediaType_MP3,
    MediaType_OGG
  } MediaType_t;
  
  
  /**
   * Exception thrown by ISongInfo and sub-classes when no ISongInfo can
   * be retrieved from a song.
   */
  class NoSongInfo : public StdException {
  public:
    /**
     * Constructor.
     *
     * @param aStr     Eventual error message
     */
    NoSongInfo(std::string aStr);
  };
  

  /**
   * Interface for song information
   *
   * It is assumed that the information does not change, so all
   * information needs only to be fetched once, i.e. can be cached.
   */
  class ISongInfo : public IXMLOutput {
  public:
    /**
     * Get song description.
     *
     * @return        mDescription
     */
    virtual QString GetDescription() const = 0;
    
    /**
     * Get song artist
     *
     * @return        mArtist
     */
    virtual QString GetArtist() const = 0;
    
    /**
     * Get song album
     *
     * @return        mAlbum
     */
    virtual QString GetAlbum() const = 0;
    
    /**
     * Get song title
     *
     * @return        mTitle
     */
    virtual QString GetTitle() const = 0;
    
    /**
     * Get song genre
     *
     * @return       mGenre 
     */
    virtual QString GetGenre() const = 0;
    
    /**
     * Get song track number
     *
     * @return       mTrack
     */
    virtual u_int32_t GetTrack() const = 0;
    
    /**
     * Get song release year
     *
     * @return       mYear
     */
    virtual u_int32_t GetYear() const = 0;
    
    /**
     * Get song size (in bytes)
     *
     * @return       mSize
     */
    virtual u_int32_t GetSize() const = 0;
    
    /**
     * Get song owner
     *
     * @return       mOwner
     */
    virtual QString GetOwner() const = 0;
    
    /**
     * Get song length, in seconds
     *
     * @return       mSeconds
     */
    virtual u_int32_t GetLength() const = 0;

    /**
     * Get media type of song (MP3, OGG, ...)
     */
    virtual MediaType_t GetMediaType() const = 0;
  };
}

#endif
