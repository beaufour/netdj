// -*- c++ -*-
/**
 * \file SongInfo_File.h
 * Class SongInfo_File.
 *
 * $Id$
 *
 */

#ifndef __SONGINFO_FILE_H__
#define __SONGINFO_FILE_H__

#include "SongInfo.h"
#include <string>

/**
 * Handles song informations for a file, that is fills out Owner and
 * Size.
 */
class SongInfo_File : public SongInfo {
protected:
  /** The filename of the file */
  std::string Filename;

public:
  /**
   * Constructor.
   *
   * @param fname   The filename of the file
   */
  SongInfo_File(const std::string fname);


  /** Destructor */
  virtual ~SongInfo_File();
};

#endif
