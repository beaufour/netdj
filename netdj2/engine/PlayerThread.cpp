/**
 * \file PlayerThread.cpp
 * class PlayerThread
 *
 * $Id$
 *
 */
#include "PlayerThread.h"

#include <iostream>
#include <vector>

#include "config.h"
#include "Collection.h"
#include "Collections.h"
#include "Shout.h"
#include "Song.h"
#include "StdException.h"

using namespace std;

PlayerThread::PlayerThread(Collections* aCols, unsigned int aStackSize,
			   QObject* aParent)
  : QObject(aParent, "PlayerThread"),
    QThread(aStackSize),
    mStopPlayer(false), mSkipSong(false), mCols(aCols)
{
}

PlayerThread::~PlayerThread()
{
}

bool
PlayerThread::GetNextSong() {
  QMutexLocker lock(&mSongMutex);

  return mCols->GetNextSong(mCurrentSong, &mCurrentCollection);
}

void
PlayerThread::GetCurrentSong(Song& aSong, string& aColId) const {
  QMutexLocker lock(&mSongMutex);

  aSong = mCurrentSong;
  aColId = mCurrentCollection->GetIdentifier();
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
    cout << "-----=====> Setup libshout" << endl;
    Shout shout(NETDJ_CONF.GetString("STREAM_NAME"),
		NETDJ_CONF.GetString("STREAM_GENRE"),
		NETDJ_CONF.GetString("STREAM_DESCR"));
    shout.SetPublic(NETDJ_CONF.GetBool("STREAM_PUBLIC"));
    shout.SetUser(NETDJ_CONF.GetString("STREAM_USER"));
    shout.SetHost(NETDJ_CONF.GetString("STREAM_HOST"));
    shout.SetPort(NETDJ_CONF.GetInteger("STREAM_PORT"));
    shout.SetPassword(NETDJ_CONF.GetString("STREAM_PASSWD"));
    shout.SetMount(NETDJ_CONF.GetString("STREAM_MOUNT"));
    shout.SetFormat(SHOUT_FORMAT_MP3);
    shout.SetProtocol(SHOUT_PROTOCOL_HTTP);
    
    cout << "-----=====> Connect to icecast" << endl;
    shout.Connect();
    
    cout << endl << "-----=====> MAIN LOOP" << endl;
    
    unsigned char buf[4096];
    int bytes;

    while (true) {
      cout << "-----=====> Get next song" << endl;

      /* Check whether we have found a song */
      if (!GetNextSong()) {
	cout << "Hmmmm, no files to play" << endl;
	cout << "...waiting a wee time" << endl;
	/** \todo Can we do something better than just waiting 10 secs? */
	sleep(10);
	continue;
      }

      cout << "** Song info " << endl;
      cout << "UNID :      " << mCurrentSong.GetUNID() << endl;
      cout << "Type :      " << mCurrentSong.GetSongType() << endl;
      cout << "Filename :  " << mCurrentSong.GetFilename() << endl;
      cout << "Collection: " << mCurrentCollection->GetIdentifier() << endl;
      
      if (mCurrentSong.GetSongType() != SongType_MP3) {
	cout << "INVALID SONGTYPE!" << endl;
	continue;
      }
      
      try {
	const SongInfo* info = mCurrentSong.GetSongInfo();
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

      cout << "-----=====> Send song to icecast" << endl;
      QFile file(mCurrentSong.GetFilename());
      if (!file.open(IO_ReadOnly)) {
	continue;
      }
      
      /* Send song */
      shout.SetSongName(mCurrentSong.GetFilename());
      while ((bytes = file.readBlock((char*) buf, sizeof(buf))) > 0
	     && mStopPlayer == false && mSkipSong == false) {
	shout.Send(buf, bytes);
	shout.Sleep();
      }
      
      /* Close file */
      file.close();


      /* Delete file, if collection is request-queue */
      if (mCurrentCollection->GetDeleteAfterPlay()) {
	/** \todo Delete file! */
	cout << "TODO: delete file" << endl;
      }

      if (mStopPlayer) {
	break;
      }
      if (mSkipSong) {
	mSkipSong = false;
	cout << "Skipping current song!" << endl;
      }
    } // Main loop
  }
  catch (StdException &e) {
    cout << "[" << e.GetType() << "] " << e.what() << endl;
  }
  catch (exception& e) {
    cout << "[std::exception] " << e.what() << endl;
  }
  catch (...) {
    cout << "Unknown exception!!" << endl;
    }
  cout << "Player thread stopped!" << endl;
}
