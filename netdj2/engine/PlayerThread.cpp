/**
 * \file PlayerThread.cpp
 * class PlayerThread
 *
 * $Id$
 *
 */
#include "PlayerThread.h"

#include <qfile.h>
#include <qfileinfo.h>

#include "config.h"
#include "Collections.h"
#include "ICollection.h"
#include "ISong.h"
#include "Shout.h"
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

    ISong* currentSong;
    const ICollection* currentCollection;

    while (true) {
      /* Check whether we have found a song */
      if (!mCols->GetNextSong(&currentSong, &currentCollection)) {
	emit SigMessage("I have no files to play...", 10, LogService::ENTRY_WARN);
	/** @todo Can we do something better than just waiting 10 secs? */
	sleep(10);
	continue;
      }

      QFile file(currentSong->GetFilename());
      QFileInfo finfo (file);

      if (!finfo.exists() ||
	  !finfo.isReadable() ||
	  !finfo.isFile() ||
	  !file.open(IO_ReadOnly)
	) {
	emit SigMessage("Something is wrong with the file: " + currentSong->GetFilename(), 10, LogService::ENTRY_WARN);
	continue;
      }

      emit SigSongPlaying(currentSong, currentCollection);

      // @todo check song type
      emit SigMessage("Check song type!", 10, LogService::ENTRY_CRIT);
      
      /* Send song */
      // @todo use song info!
      shout.SetSongName(currentSong->GetFilename());
      while ((bytes = file.readBlock((char*) buf, sizeof(buf))) > 0
	     && mStopPlayer == false && mSkipSong == false) {
	shout.Send(buf, bytes);
	shout.Sleep();
      }
      
      /* Close file */
      file.close();

      /* Delete file? */
      if (currentSong->GetDeleteAfterPlay() &&
	  ! QFile::remove(currentSong->GetFilename())) {
	emit SigMessage("Could not delete file: " + currentSong->GetFilename(), 20, LogService::ENTRY_WARN);
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
    emit SigMessage(QString("EXCEPTION: [") + e.GetType() + "] " + e.what(), 0, LogService::ENTRY_CRIT);
  }
  catch (exception& e) {
    emit SigMessage(QString("EXCEPTION: [std::exception] ") + e.what(), 0, LogService::ENTRY_CRIT);
  }
  catch (...) {
    emit SigMessage(QString("EXCEPTION: (unknown)"), 0, LogService::ENTRY_CRIT);
  }

  emit SigStop();
}
