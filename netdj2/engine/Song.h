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
#include "XMLOutput.h"

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
class Song : public XMLOutput {
private:
  /** Next unique ID to assign */
  static u_int32_t NextUNID;

  /** The filename */
  std::string Filename;

  /** Pointer to SongInfo structure, may be 0 */
  mutable SongInfo* sInfo;

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

  /** Destructor */
  virtual ~Song();

  /**
   * Copy constructor.
   *
   * @param s2      New song content.
   */
  Song(const Song& s2);

  /**
   * Assignment. 
   *
   * @param s2      New song content.
   * @return        Reference to self.
   */
  Song& operator=(const Song& s2);

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
   * NB! Ownership is held by Song.
   *
   * @return        SongInfo structure
   */
  const SongInfo* GetSongInfo() const;

  void asXML(QDomDocument& doc, QDomElement& root) const;
};

#endif