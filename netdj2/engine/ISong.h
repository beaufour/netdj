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

#include "IXMLOutput.h"

#include <qstring.h>

namespace NetDJ
{
  class ISongInfo;
  
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
     * Get detailed information about the song.
     *
     * @note Ownership is held by ISong.
     *
     * @return        SongInfo structure
     */
    virtual const ISongInfo* GetSongInfo() const = 0;

    /**
     * Get the filename of the song
     *
     * @return        The filename
     */
    virtual QString GetFilename() const = 0;

    /**
     * Should file be deleted _from filesystem_ after being played?
     *
     * I guess this will only be used for the request queue.
     */
    virtual bool GetDeleteAfterPlay() const = 0;
  };
}

#endif
