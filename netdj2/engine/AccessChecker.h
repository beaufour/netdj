/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/**
 * \file AccessChecker.h
 * Class SimpleAccessChecker.
 *
 * $Id$
 *
 */

#ifndef __ACCESSCHECKER_H__
#define __ACCESSCHECKER_H__

#include <qstring.h>
#include <qmap.h>

#include "IAccessChecker.h"

namespace NetDJ
{

  /**
   * Implementation of IAccessChecker, a simple access control mechanism.
   *
   * Reads a file with userid:passwd:level, and uses this information to
   * authenticate with.
   */
  class SimpleAccessChecker : public IAccessChecker 
  {
  private:
    /** The name of the file used */
    QString mFilename;
    
    // Structure used for each user
    typedef struct
    {
      QString mPassword;
      unsigned int mLevel;
    } userinfo_t;
    
    QMap<QString, userinfo_t> mUsers;
    
  public:
    SimpleAccessChecker(const QString& aFilename);
    virtual ~SimpleAccessChecker();
    
    // IAccessChecker
    bool Init();
    bool HasAccess(const QString& aUsername,
                   const QString& aPassword,
                   const unsigned int aLevel);
    
  };
}

#endif
