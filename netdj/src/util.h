/*
 *  util.h
 * 
 *  Allan Beaufour Larsen <allan@beaufour.dk>
 *
 *  $Id$
 */

#ifndef __UTIL_H__
#define __UTIL_H__

// Provides string-container
#include <string>

// Provides rand, srand and system
#include <cstdlib>

int get_rand(const int);
void error(const char*, const bool = true);
string base64_decode(const char*);

#endif
