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
    mutable bool mXMLValid;

    /** Document used to store song information in */
    mutable QDomDocument mDocument;

    /** The main information node */
    mutable QDomElement mInfoNode;

    /** Creates the XML representation of the class */
    void CreateXML() const;
    
  protected:
    /** Textual description of song (ie. "Aha - Take On Me") */
    QString mDescription;
    
    /** Song artist */
    QString mArtist;
    
    /** From album */
    QString mAlbum;
    
    /** Song title */
    QString mTitle;
    
    /** Song genre */
    QString mGenre;
    
    /** Song track */
    u_int32_t mTrack;
    
    /** Song release year */
    u_int32_t mYear;
    
    /** Song size (in bytes) */
    u_int32_t mSize;
    
    /** Song owner */
    QString mOwner;
    
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
    QString GetDescription() const;
    QString GetArtist() const;
    QString GetAlbum() const;
    QString GetTitle() const;
    QString GetGenre() const;
    u_int32_t GetTrack() const;
    u_int32_t GetYear() const;
    u_int32_t GetSize() const;
    QString GetOwner() const;
    u_int32_t GetLength() const;
    MediaType_t GetMediaType() const;

    // IXMLOutput
    QDomElement* AsXML(QDomDocument* aDocument) const;
  };
}

#endif
