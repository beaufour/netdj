/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/**
 * \file ServerSocet.h
 * class ServerSocket
 *
 * $Id$
 *
 */

#ifndef SERVERSOCKET_H
#define SERVERSOCKET_H

// Qt includes
#include <qserversocket.h>

// Forward declarations
class QSocket;

namespace NetDJ
{

  class ServerSocket : public QServerSocket
  {
    Q_OBJECT
    
  public:
    ServerSocket(int aPort, int aBackLog, QObject* aParent = 0, const char* aName = 0);
    ~ServerSocket();
    
    void newConnection(int aSocketFD);
    
  signals:
    void newClient(QSocket* aSocket);
  };
}

#endif
