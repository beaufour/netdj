/**
 * \file PlayerThread.cpp
 * class PlayerThread
 *
 * $Id$
 *
 */
#include "PlayerThread.h"

#include "config.h"
#include "Collection.h"
#include "Collections.h"
#include "Shout.h"
#include "Song.h"
#include "StdException.h"

using namespace std;
using namespace NetDJ;

PlayerThread::PlayerThread(Collections* aCols, unsigned int aStackSize,
			   QObject* aParent)
  : QObject(aParent, "Player"),
    QThread(aStackSize),
    mStopPlayer(false), mSkipSong(false), mCols(aCols)
{
}

PlayerThread::~PlayerThread()
{
}

void
PlayerThread::Stop() {
  mStopPlayer = true;
}

void
PlayerThread::Skip() {
  mSkipSong = true;
}

void
PlayerThread::run() {
  try {
    emit SigStart();
    
    Shout shout(gConfig.GetString("STREAM_NAME"),
		gConfig.GetString("STREAM_GENRE"),
		gConfig.GetString("STREAM_DESCR"));
    shout.SetPublic(gConfig.GetBool("STREAM_PUBLIC"));
    shout.SetUser(gConfig.GetString("STREAM_USER"));
    shout.SetHost(gConfig.GetString("STREAM_HOST"));
    shout.SetPort(gConfig.GetInteger("STREAM_PORT"));
    shout.SetPassword(gConfig.GetString("STREAM_PASSWD"));
    shout.SetMount(gConfig.GetString("STREAM_MOUNT"));
    shout.SetFormat(SHOUT_FORMAT_MP3);
    shout.SetProtocol(SHOUT_PROTOCOL_HTTP);
    
    shout.Connect();
    
    unsigned char buf[4096];
    int bytes;

    Song currentSong;
    const Collection* currentCollection;

    while (true) {
      /* Check whether we have found a song */
      if (!mCols->GetNextSong(currentSong, &currentCollection)) {
	emit SigMessage("I have no files to play...", 10);
	/** @todo Can we do something better than just waiting 10 secs? */
	sleep(10);
	continue;
      }
      emit SigSongPlaying(currentSong, currentCollection);

      if (currentSong.GetSongType() != SongType_MP3) {
	emit SigMessage("Invalid songtype, skipping!", 10);
	continue;
      }

      /*
      try {
	const SongInfo* info = currentSong.GetSongInfo();
	if (info) {
	  cout << "** Song content info" << endl;
	  cout << "Description:  " << info->GetDescription() << endl;
	  cout << "Album:        " << info->GetAlbum() << endl;
	  cout << "Title:        " << info->GetTitle() << endl;
	  cout << "Artist:       " << info->GetArtist() << endl;
	  cout << "Genre:        " << info->GetGenre() << endl;
	  cout << "Year:         " << info->GetYear() << endl;
	  cout << "Track:        " << info->GetTrack() << endl;
	  cout << "Length:       " << (info->GetLength()/60)
	       << ":" << (info->GetLength() - (info->GetLength()/60) * 60)<< endl;
	  cout << "Size:         " << info->GetSize() << endl;
	  cout << "Owner:        " << info->GetOwner() << endl;
	}
	
      }
      catch (NoSongInfo& h) {
	cout << "No header info" << endl;
      }
      */

      QFile file(currentSong.GetFilename());
      if (!file.open(IO_ReadOnly)) {
	continue;
      }
      
      /* Send song */
      shout.SetSongName(currentSong.GetFilename());
      while ((bytes = file.readBlock((char*) buf, sizeof(buf))) > 0
	     && mStopPlayer == false && mSkipSong == false) {
	shout.Send(buf, bytes);
	shout.Sleep();
      }
      
      /* Close file */
      file.close();


      /* Delete file, if collection is request-queue */
      if (currentCollection->GetDeleteAfterPlay()) {
	/** @todo Delete file! */
	emit SigMessage("TODO: delete file", 0);
      }

      if (mStopPlayer) {
	break;
      }
      if (mSkipSong) {
	mSkipSong = false;
      }
    } // Main loop
  }
  catch (StdException &e) {
    emit SigException(e.GetType(), e.what());
  }
  catch (exception& e) {
    emit SigException("[std::exception] ", e.what());
  }
  catch (...) {
    emit SigException("(unknown)", "(unknown)");
  }

  emit SigStop();
}
