/**
 * \file SongInfo_File_mp3.cpp
 * Class SongInfo_File_mp3.
 *
 * $Id$
 *
 */

#include <cstdlib>
#include <id3/tag.h>
#include <id3/misc_support.h>
#include <qfileinfo.h>

#include "SongInfo_File_mp3.h"

using namespace std;

// libmad for timing information?

SongInfo_File_mp3::SongInfo_File_mp3(const string fname)
  : SongInfo_File(fname)
{
  /* Initialize */
  ID3_Tag myTag;
  myTag.Link(Filename.c_str(), ID3TT_ALL);

  const Mp3_Headerinfo* mp3info = myTag.GetMp3HeaderInfo();
  if (mp3info) {
    auto_ptr<ID3_Tag::Iterator> iter(myTag.CreateIterator());
    if (!iter.get()) {
      throw StdException("Could not create ID3_Tag::ConstIterator!");
    }
    
    /* Iterate over frames */
    const ID3_Frame* frame = NULL;
    ID3_FrameID eFrameID;
    char *sText;
    string* element = 0;
    u_int32_t* n_element = 0;
    
    while (NULL != (frame = iter->GetNext())) {
      /* Get frame ID */
      eFrameID = frame->GetID();
      
      /* Assign pointer to apropriate variable */
      switch (eFrameID) {
      case ID3FID_ALBUM:
	element = &Album;
	break;
	
      case ID3FID_TITLE:
	element = &Title;
	break;
	
      case ID3FID_LEADARTIST:
	element = &Artist;
	break;
	
      case ID3FID_CONTENTTYPE:
	element = &Genre;
	break;
	
      case ID3FID_TRACKNUM:
	n_element = &Track;
	break;
	
      case ID3FID_YEAR:
	n_element = &Year;
	break;
	
      default:
	break;
      }
      
      /* Get string and assign (and convert for numeric values) */
      if (element || n_element) {
	sText = ID3_GetString(frame, ID3FN_TEXT);
	if (sText) {
	  if (element) {
	    *element = sText;
	  } else {
	    *n_element = atoi(sText);
	  }
	  delete [] sText;
	}
	element = 0;
	n_element = 0;
      }
    }
  }

  if (Title != "" && Artist != "") {
    Description = Artist + " - " + Title;
  } else {
    /* Set description to file basename */
    Description = QFileInfo(Filename).baseName().ascii();
  }
}


SongInfo_File_mp3::~SongInfo_File_mp3() {
}
