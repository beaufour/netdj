/*
 *  Locakable.cc
 * 
 *  Allan Beaufour Larsen <allan@beaufour.dk>
 *
 *  $Id$
 */

#include "Lockable.h"

Lockable::Lockable() {
  pthread_mutex_init(&mutex, NULL);
};


Lockable::~Lockable() {
  pthread_mutex_destroy(&mutex);
};


void
Lockable::lock() const {
  pthread_mutex_lock(&mutex);
};


void
Lockable::unlock() const {
  pthread_mutex_unlock(&mutex);
};

