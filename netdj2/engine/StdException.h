// -*- c++ -*-
/**
 * \file StdException.h
 * Class StdException.
 *
 * $Id$
 *
 */

#include <string>
#include <exception>

#ifndef __STDEXCEPTION_H__
#define __STDEXCEPTION_H__

/**
 * Basic exception class.
 */
class StdException : public std::exception {
protected:
  std::string mDescription;
public:
  StdException(std::string aDescr);
  ~StdException() throw();

  virtual const char* what() const throw();
};

#endif
