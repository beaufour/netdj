// -*- c++ -*-
/**
 * \file ISong.h
 * Interface ISong.
 *
 * $Id$
 *
 */

#ifndef __ISONG_H__
#define __ISONG_H__

#include "SongInfo.h"
#include "IXMLOutput.h"

namespace NetDJ
{
  
  /**
   * Interface for songs.
   *
   * As collections can contain thousands of songs, this classes implementing
   * this should be kept to the absolutely minimum. All retrieval and storage
   * of informations about Title, Artist, etc. should be done by other
   * classes.
   */
  class ISong : public IXMLOutput {
  public:
    /**
     * Get unique identificator (UNID).
     *
     * Note that UNIDs are only unique in their contained Collection
     *
     * @return       UNID
     */
    virtual u_int32_t GetUNID() const = 0;
    
    /**
     * Get the type of the media (MP3, OGG, WAV, etc.)
     * 
     * @return        The media type.
     */
    virtual MediaType_t GetMediaType() const = 0;
    
    /**
     * Get detailed information about the song.
     *
     * @note Ownership is held by ISong.
     *
     * @return        SongInfo structure
     */
    virtual const SongInfo* GetSongInfo() const = 0;
  };
}

#endif
