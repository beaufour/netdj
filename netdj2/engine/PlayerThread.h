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

public:
  /**
   * Constructor.
   *
   * @param stackSize    The stacksize for the thread
   */
  PlayerThread(unsigned int stackSize = 0);

  /** Main function of the thread. */
  void run();

  /** Stop the thread */
  void Stop();

  /** Skip current song */
  void Skip();
 
};

#endif
