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
#include <qmutex.h>
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

  /** Mutex protecting CurrentSong */
  mutable QMutex mSongMutex;

  /** The song currently playing */
  Song mCurrentSong;

  /** The collection the currently playing song is from */
  const Collection* mCurrentCollection;

  /** Get next song to play */
  bool GetNextSong();

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

  /**
   * Get the currently playing Song and Collection.
   *
   * @param aSong             Current song
   * @param aColId            Current collection.
   */
  void GetCurrentSong(Song& aSong, std::string& aColId) const;

public slots:
  /** Skip current song */
  void Skip();
};

#endif
