// -*- c++ -*-
/**
 * \file SongInfoFile.h
 * Class SongInfoFile.
 *
 * $Id$
 *
 */

#ifndef __SONGINFOFILE_H__
#define __SONGINFOFILE_H__

#include "SongInfoBase.h"

#include <qstring.h>

namespace NetDJ
{

  /**
   * Implementation of ISongInfo for file based songs.
   *
   * Uses Taglib to get the information,
   * @see http://developer.kde.org/~wheeler/taglib.html
   */
  class SongInfoFile : public SongInfoBase {
  public:
    /**
     * Constructor.
     *
     * @param aFilename       The filename of the file
     */
    SongInfoFile(const QString& aFilename);
  };
}

#endif
