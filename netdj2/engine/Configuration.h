
/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/**
 * \file Configuration.h
 * class Configuration
 *
 * $Id$
 *
 */

#ifndef __CONFIGURATION_H__
#define __CONFIGURATION_H__

#include <qmap.h>
#include <qstring.h>

namespace NetDJ
{
  
  /**
   * Configuration file manager.
   */
  class Configuration {
  private:
    /** Map that holds the configuration, name/value pairs */
    QMap<QString, QString> mConfig;
    
  public:
    bool Init();  
    
    QString GetString(const QString&) const;
    int GetInteger(const QString&) const;
    bool GetBool(const QString&) const;
    
    void SetString(const QString&, const QString&);
    void SetBool(const QString&, bool);
    void SetInteger(const QString&, int);
  };
}

#endif
