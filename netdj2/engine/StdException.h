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

namespace NetDJ 
{
  
  /**
   * Basic exception class.
   *
   * @todo Somehow hook up StdException to the logger, to all exceptions are
   * logged? It could be a mess, since that may also throw an exception...
   */
  class StdException : public std::exception {
  protected:
    /** The description of the exception that happened  */
    std::string mDescription;
    
    /** The name of this type of exception */
    std::string mType;
    
  public:
  StdException(std::string aDescr, std::string aType = "StdException");
    ~StdException() throw();

    const std::string GetType() const;
    
    virtual const char* what() const throw();
  };
}

#endif
