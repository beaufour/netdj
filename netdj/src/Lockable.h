/*
 *  Lockable.h
 * 
 *  Allan Beaufour Larsen <allan@beaufour.dk>
 *
 *  $Id$
 */

#ifndef  __LOCKABLE_H__
#define __LOCKABLE_H__

#include <pthread.h>

class Lockable {
protected:
  pthread_mutex_t mutex;

  void lock();
  void unlock();

public:
  Lockable();
  ~Lockable();
};

#endif
