// -*- c++ -*-
/**
 * \file PlayerThread.h
 * class PlayerThread
 *
 * $Id$
 *
 */

#ifndef __PLAYERTHREAD_H__
#define __PLAYERTHREAD_H__

#include <qthread.h>
#include <qobject.h>

#include "Client.h"
#include "Song.h"

class Collections;
class Collection;

/**
 * The thread handling the actual playing of files through the icecast
 * server.
 */
class PlayerThread : public QObject,
		     public QThread		     
{
private:
  Q_OBJECT

  /** If this is set to true the thread will exit */
  bool mStopPlayer;

  /** If this is set to true the current song will be skipped */
  bool mSkipSong;

  /** Pointer to song Collections */
  Collections* mCols;

public:
  /**
   * Constructor.
   *
   * @param aCols             Pointer to Collections
   * @param aStackSize        The stacksize for the thread
   * @patam aParent           The parent object
   */
  PlayerThread(Collections* aCols, unsigned int aStackSize = 0,
	       QObject* aParent = 0);

  ~PlayerThread();
  
  /** Main function of the thread. */
  void run();

  /** Stop the thread */
  void Stop();

signals:
  /*
   * Emitted when a new song is put on
   *
   * @param aSong             The song
   * @param aCol              The collection it comes from
   */
  void SigSongPlaying(const Song& aSong, const Collection* aCol);

  /** Emitted when the player starts */
  void SigStart();
  
  /** Emitted when the player stops */
  void SigStop();
  
  /** Emits a general information message */
  void SigMessage(const QString& aMsg, const unsigned int aLevel);

public slots:
  /** Skip current song */
  void Skip();
};

#endif
