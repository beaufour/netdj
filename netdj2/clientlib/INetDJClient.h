/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/**
 * \file INetDJClient.h
 * class INetDJClient
 *
 * $Id$
 *
 */

#ifndef __INETDJCLIENT_H__
#define __INETDJCLIENT_H__

#include <qobject.h>
#include <qstring.h>

#include "LogService.h"

class QDomElement;

namespace NetDJ
{
  class ISong;
  class ICollection;

  /**
   * Pure virual interface for NetDJ client.
   *
   * It is the reverse of the LogService class; Instead of receiving the
   * commands, and emitting a entry signal, it receives the entry signal and
   * emits commands.
   */
  class INetDJClient : public QObject
  {
    Q_OBJECT

  signals:
    virtual void SongPlaying(const ISong* aSong, const ICollection* aCol) = 0;
    
    /** Receiving a 'quit' command */
    virtual void Quit(const QString& aUsername) = 0;
    
    /** Receiving a 'skip' command */
    virtual void Skip(const QString& aUsername) = 0;
    
    /** The PlayerThread starts */
    virtual void PlayerStart() = 0;
    
    /** The PlayerThread stops */
    virtual void PlayerStop() = 0;
    
    /** A new client connects */
    virtual void ClientNew() = 0;
    
    /** A client connection is closed */
    virtual void ClientClose() = 0;
    
    /** A generic message */
    virtual void Message(const QString& aMsg, const unsigned int aLevel, LogService::EntryClass_t aClass) = 0;

  public slots:
    virtual void NewLogEntry(const QDomElement* aEntry) = 0;
  };
}

#endif
