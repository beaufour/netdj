/**
 * \file StdException.cpp
 * Class StdException.
 *
 * $Id$
 *
 */

#include "StdException.h"

using namespace std;

StdException::StdException(string descr)
  : Description(descr) 
{
}

StdException::~StdException() throw()
{
}

const char*
StdException::what() const throw() {
  return Description.c_str();
}

