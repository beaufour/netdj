// -*- c++ -*-
/**
 * \file SongInfo_mp3.h
 * Class SongInfo_mp3.
 *
 * $Id$
 *
 */

#ifndef __SONGINFO_MP3_H__
#define __SONGINFO_MP3_H__

#include "SongInfo.h"
#include "StdException.h"

/**
 * SongInfo class for MP3-files.
 */
class SongInfo_mp3 : public SongInfo {
public:
  /** Constructor. */
  SongInfo_mp3(const std::string fname);

  /** Destructor */
  ~SongInfo_mp3();
};

#endif
