/**
 * \file StdException.cpp
 * Class StdException.
 *
 * $Id$
 *
 */

#include "StdException.h"

using namespace std;

StdException::StdException(string aDescr)
  : mDescription(aDescr)
{
}

StdException::~StdException() throw()
{
}

const char*
StdException::what() const throw() {
  return mDescription.c_str();
}

