/**
 * \file StdException.cpp
 * Class StdException.
 *
 * $Id$
 *
 */

#include "StdException.h"

using namespace std;
using namespace NetDJ;

StdException::StdException(string aDescr, string aType)
  : mDescription(aDescr), mType(aType)
{
}

StdException::~StdException() throw()
{
}

const string
StdException::GetType() const
{
  return mType;
}


const char*
StdException::what() const throw() {
  return mDescription.c_str();
}

