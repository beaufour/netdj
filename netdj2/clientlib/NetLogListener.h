/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/**
 * \file NetLogListener.h
 * class NetLogListener
 *
 * $Id$
 *
 */

#ifndef __NETLOGLISTENER_H__
#define __NETLOGLISTENER_H__

#include "ILogListener.h"

#include <qstring.h>

namespace NetDJ
{
  /**
   * Pure virual interface for a NetDJ log listener
   */
  class NetLogListener : public ILogListener
  {
    Q_OBJECT

  public:
    /** Constructor */
    NetLogListener(const QString aHost, const unsigned int aPort);

  signals:
    void SigLogEntry(const QDomElement* aEntry);
  };
}

#endif
