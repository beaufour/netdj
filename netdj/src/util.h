/*  -*- c++ -*-
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

int get_rand(const int);
void error(const char*, const bool = true);
std::string base64_decode(const char*);

#endif
