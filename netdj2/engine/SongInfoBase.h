// -*- c++ -*-
/**
 * \file SongInfoBase.h
 * Class SongInfoBase.
 *
 * $Id$
 *
 */

#ifndef __SONGINFOBASE_H__
#define __SONGINFOBASE_H__

#include "ISongInfo.h"

#include <qdom.h>

namespace NetDJ
{  
  /**
   * Implementation of ISongInfo that handles storage of song information.
   *
   * It does not fill out the values, this need to be done by subclasses.
   */
  class SongInfoBase : public ISongInfo {
  private:
    /** Tells whether the XML information is valid */
    bool mXMLValid;

    /** Document used to store song information in */
    mutable QDomDocument mDoc;

    /** The main information node */
    mutable QDomElement  mInfoNode;

    /** Creates the XML representation of the class */
    void CreateXML() const;
    
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

    /** The media type */
    MediaType_t mMediaType;
    
  public:
    /** Constructor. */
    SongInfoBase();
    
    /** Destructor */
    virtual ~SongInfoBase();

    // ISong
    std::string GetDescription() const;
    std::string GetArtist() const;
    std::string GetAlbum() const;
    std::string GetTitle() const;
    std::string GetGenre() const;
    u_int32_t GetTrack() const;
    u_int32_t GetYear() const;
    u_int32_t GetSize() const;
    std::string GetOwner() const;
    u_int32_t GetLength() const;
    MediaType_t GetMediaType() const;

    // IXMLOutput
    QDomElement* AsXML(QDomDocument* aDocument) const;
  };
}

#endif
