/**
 * \file SongInfo_File.cpp
 * Class SongInfo_File.
 *
 * $Id$
 *
 */

#include "SongInfo_File.h"
#include <qfileinfo.h>

using namespace std;

SongInfo_File::SongInfo_File(const string aName)
  : mFilename(aName)
{
  QFileInfo qinfo(mFilename);

  mOwner = qinfo.owner().ascii();
  mSize = qinfo.size();
}


SongInfo_File::~SongInfo_File() {

}
