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
  mutable pthread_mutex_t mutex;

  void lock() const;
  void unlock() const;

public:
  Lockable();
  ~Lockable();
};

#endif
