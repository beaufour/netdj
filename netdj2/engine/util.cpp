/**
 * \file util.cpp
 * Misc. utility functions.
 *
 * $Id$
 *
 */

#include "util.h"

// Provides rand, srand and system
#include <cstdlib>
#include <iostream>

using namespace std;

int
get_rand(const int aMax) {
  return (int) ((float) aMax * rand() / (RAND_MAX + 1.0));
}

