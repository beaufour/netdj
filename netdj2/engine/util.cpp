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
get_rand(const int max) {
  return (int) ((float) max * rand() / (RAND_MAX + 1.0));
}

