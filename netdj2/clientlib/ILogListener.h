/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/**
 * \file ILogListener.h
 * class ILogListener
 *
 * $Id$
 *
 */

#ifndef __ILOGLISTENER_H__
#define __ILOGLISTENER_H__

#include <qobject.h>

class QDomElement;

namespace NetDJ
{
  /**
   * Pure virual interface for a NetDJ log listener
   */
  class ILogListener : public QObject
  {
    Q_OBJECT

  signals:
    virtual void SigLogEntry(const QDomElement* aEntry) = 0;
  };
}

#endif
