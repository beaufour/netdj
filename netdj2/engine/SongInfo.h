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
 * Handles all information about the song: Title, artist, etc.
 */
class SongInfo {
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
  std::string GetDescription();

  /**
   * Get song artist
   *
   * @return        Artist
   */
  std::string GetArtist();

  /**
   * Get song album
   *
   * @return        Album
   */
  std::string GetAlbum();

  /**
   * Get song title
   *
   * @return        Title
   */
  std::string GetTitle();

  /**
   * Get song genre
   *
   * @return       Genre 
   */
  std::string GetGenre();

  /**
   * Get song track number
   *
   * @return       Track
   */
  u_int32_t GetTrack();

  /**
   * Get song release year
   *
   * @return       Year
   */
  u_int32_t GetYear();

  /**
   * Create XML structure for object.
   *
   * @param doc     The DomDocument to use as root
   * @return        The XML structure
   */
  QDomElement asXML(QDomDocument& doc);
};

#endif
