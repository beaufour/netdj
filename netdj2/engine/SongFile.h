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

#include <qstring.h>
#include <qfileinfo.h>

#include "ISong.h"

namespace NetDJ
{

  class ISongInfo;
  
  /**
   * Implementation of ISong for songs stored in files
   */
  class SongFile : public ISong
  {
  private:
    /** The filename */
    QString mFilename;
    
    /** Pointer to SongInfo structure, may be 0 */
    mutable ISongInfo* mSongInfo;
    
    /** Unique ID (application-wise) */
    u_int32_t mUNID;

    /** Should song be deleted from filesystem after being played? */
    bool mDeleteAfterPlay;
    
  public:
    /**
     * Constructor.
     *
     * @param aNname   The filename of the song
     * @param aUNID    The unique ID of the file
     */
    SongFile(const QString& aNname,  const u_int32_t aUNID);
    
    /** Destructor */
    virtual ~SongFile();
    
    /**
     * Copy constructor.
     *
     * @param aSong2      New song content.
     */
    SongFile(const SongFile& aSong2);
    
    /**
     * Assignment. 
     *
     * @param aSong2      New song content.
     * @return            Reference to self.
     */
    SongFile& operator=(const SongFile& aSong2);

    /**
     * Set mDeleteAfterPlay.
     *
     * @param aVal            The value
     */
    void SetDeleteAfterPlay(bool aVal);
    
    // ISong
    u_int32_t GetUNID() const;
    const ISongInfo* GetSongInfo() const;    
    QString GetFilename() const;
    bool GetDeleteAfterPlay() const;


    // IXMLOutput
    QDomElement* AsXML(QDomDocument* aDocument) const;
  };
}

#endif
