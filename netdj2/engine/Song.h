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
 * Base class for songs (on file).
 *
 * Handles all information about the song, the file, and information
 * about the file.
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
   * Get information about the song.
   *
   * @return        New SongInfo structure
   */
  SongInfo* GetSongInfo() const;

  /**
   * Get information about the file.
   *
   * @return        New QFileInfo structure
   */
  QFileInfo* GetFileInfo() const;
};

#endif
