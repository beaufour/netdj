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
  NoSongInfo(std::string str);
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
  std::string Description;

  /** Song artist */
  std::string Artist;

  /** From album */
  std::string Album;

  /** Song title */
  std::string Title;

  /** Song genre */
  std::string Genre;

  /** Song track */
  u_int32_t Track;

  /** Song release year */
  u_int32_t Year;

  /** Song size (in bytes) */
  u_int32_t Size;

  /** Song owner */
  std::string Owner;

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
   * @return        Description
   */
  std::string GetDescription() const;

  /**
   * Get song artist
   *
   * @return        Artist
   */
  std::string GetArtist() const;

  /**
   * Get song album
   *
   * @return        Album
   */
  std::string GetAlbum() const;

  /**
   * Get song title
   *
   * @return        Title
   */
  std::string GetTitle() const;

  /**
   * Get song genre
   *
   * @return       Genre 
   */
  std::string GetGenre() const;

  /**
   * Get song track number
   *
   * @return       Track
   */
  u_int32_t GetTrack() const;

  /**
   * Get song release year
   *
   * @return       Year
   */
  u_int32_t GetYear() const;

  /**
   * Get song size (in bytes)
   *
   * @return       Size
   */
  u_int32_t GetSize() const;

  /**
   * Get song owner
   *
   * @return       Owner
   */
  std::string GetOwner() const;

  void asXML(QDomDocument& doc, QDomElement& root) const;
};

#endif
