/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/**
 * \file NetDJClient.h
 * class NetDJClient
 *
 * $Id$
 *
 */

#ifndef __NETDJCLIENT_H__
#define __NETDJCLIENT_H__

#include "INetDJClient.h"

namespace NetDJ 
{
  class NetDJClient : public INetDJClient
  {
    Q_OBJECT

  signals:
    void SongPlaying(const ISong* aSong, const ICollection* aCol);
    
    /** Receiving a 'quit' command */
    void Quit(const QString& aUsername);
    
    /** Receiving a 'skip' command */
    void Skip(const QString& aUsername);
    
    /** The PlayerThread starts */
    void PlayerStart();
    
    /** The PlayerThread stops */
    void PlayerStop();
    
    /** A new client connects */
    void ClientNew();
    
    /** A client connection is closed */
    void ClientClose();
    
    /** A generic message */
    void Message(const QString& aMsg, const unsigned int aLevel, LogService::EntryClass_t aClass);

  public slots:
    void NewLogEntry(const QDomElement* aEntry);
  };
}

#endif
