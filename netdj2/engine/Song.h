// -*- c++ -*-
/**
 * \file Song.h
 * Class Song.
 *
 * $Id$
 *
 */

#ifndef __SONG_H__
#define __SONG_H__

#include <string>
#include <qfileinfo.h>

#include "SongInfo.h"

/**
 * Defines the different song types known by the class.
 */
typedef enum {
  SongType_Unknown,
  SongType_MP3
} SongType;


/**
 * Main class for songs.
 *
 * As collections can contain thousands of songs, this class should be
 * kept to the absolutely minimum. All retrieval and storage of
 * informations about Title, Artist, etc. should be done by other
 * classes.
 *
 * 
 *
 * \note Is specialized for songs on file. Could/should be generalized
 * so remote/DB-based files can be handled to.
 */
class Song {
private:
  /** Next unique ID to assign */
  static u_int32_t NextUNID;

  /** The filename */
  std::string Filename;

  /** Unique ID (application-wise) */
  u_int32_t UNID;

  /**
   * Assign an UNID to Song.
   */
  void AssignUNID();

public:
  /** Constructor */
  Song();

  /**
   * Constructor.
   *
   * @param fname   The filename of the song
   */
  Song(const std::string fname);

  /**
   * Get unique identificator.
   *
   * @return       UNID
   */
  u_int32_t GetUNID() const;

  /**
   * Get the filename of the song
   * 
   * @return        The filname
   */
  std::string GetFilename() const;

  /**
   * Get the type of the song (MP3, OGG, WAV, etc.)
   * 
   * @return        The song type.
   */
  SongType GetSongType() const;

  /**
   * Get detailed information about the song.
   *
   * @return        New SongInfo structure
   */
  SongInfo* GetSongInfo() const;
};

#endif
