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
#include "Song.h"
#include "Collections.h"

/**
 * The thread handling the actual playing of files through the icecast
 * server.
 */
class PlayerThread : public QThread {
private:
  /** If this is set to true the thread will exit */
  bool StopPlayer;

  /** If this is set to true the current song will be skipped */
  bool SkipSong;

  /** Pointer to song Collections */
  Collections* Cols;

  /** Mutex protecting CurrentSong */
  mutable QMutex SongMutex;

  /** The song currently playing */
  Song CurrentSong;

  /** The collection the currently playing song is from */
  std::string CurrentCollection;

  /** Get next song to play */
  bool GetNextSong();

public:
  /**
   * Constructor.
   *
   * @param cols         Point to Collections
   * @param stackSize    The stacksize for the thread
   */
  PlayerThread(Collections* cols, unsigned int stackSize = 0);

  /** Main function of the thread. */
  void run();

  /** Stop the thread */
  void Stop();

  /** Skip current song */
  void Skip();

  /**
   * Get the currently playing Song and Collection.
   *
   * @param song    Current song
   * @param colid   Current collection.
   */
  void GetCurrentSong(Song& song, std::string& colid) const;
};

#endif
