/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/**
 * \file IAccessChecker.h
 * Interface IAccessChecker
 *
 * $Id$
 *
 */

#ifndef __IACCESSCHECKER_H__
#define __IACCESSCHECKER_H__

#include <qstring.h>

namespace NetDJ
{

  /**
   * Access control interface.
   */
  class IAccessChecker
  {
  public:
    /**
     * Initializes class, must be called before class is used.
     *
     * @return                  Initialization succesfull?
     */
    virtual bool Init() = 0;
    
    /**
     * Check access for a given level for a given user.
     *
     * @param aUsername         The name of the user
     * @param aPassword         The password for the user
     * @param aLevel            The level to check for
     * @return                  Has user access?
     */
    virtual bool HasAccess(const QString& aUsername,
                           const QString& aPassword,
                           const unsigned int aLevel) = 0;
  };
}

#endif
