// -*- c++ -*-
/**
 * \file SongInfo_File_mp3.h
 * Class SongInfo_File_mp3.
 *
 * $Id$
 *
 */

#ifndef __SONGINFO_FILE_MP3_H__
#define __SONGINFO_FILE_MP3_H__

#include "SongInfo_File.h"

/**
 * SongInfo class for MP3-files.
 *
 * @todo Using TagLib, this class in fact handles multiple formats..., should be changed.
 */
class SongInfo_File_mp3 : public SongInfo_File {
public:
  /** Constructor. */
  SongInfo_File_mp3(const std::string aName);

  /** Destructor */
  ~SongInfo_File_mp3();
};

#endif
