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

SongInfo_File::SongInfo_File(const string fname)
  : Filename(fname)
{
  QFileInfo qinfo(Filename);

  Owner = qinfo.owner().ascii();
  Size = qinfo.size();
}


SongInfo_File::~SongInfo_File() {

}
