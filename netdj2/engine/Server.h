/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/**
 * \file Server.h
 * class Server
 *
 * $Id$
 *
 */

#ifndef __SERVER_H__
#define __SERVER_H__

#include <qptrdict.h>
#include <qobject.h>

#include "Client.h"
#include "StdException.h"

class ServerSocket;
class PlayerThread;

/**
 * Maximum bytes allowed in a request
 * @todo Move this somewhere else!!
 */

const unsigned int MAX_REQUEST_SIZE = 8192;

/**
 * Exception thrown by Server.
 */
class ServerError : public StdException
{
public:
  /**
   * Constructor.
   *
   * @param aErr              The errormessage
   */
  ServerError(const std::string aErr);
};

/**
 * The server.
 */
class Server : public QObject
{
  Q_OBJECT

private:
  /* The current server socket */
  ServerSocket* mServerSocket;

  /* List of currently connected clients */
  QPtrDict<Client> mClients;

public:
  /**
   * Constructor.
   *
   * @param aPort             The port number to listen on
   * @param aBackLog          The backlog of connections to keep
   * @param aParent           The parent QObject
   */
  Server(int aPort, int aBackLog, QObject* aParent = 0);

signals:
  /** Emitted on receiving a 'quit' command */
  void CmdQuit();
  
  /** Emitted on receiving a 'skip' command */
  void CmdSkip();
  
private slots:
  /** Called when data is ready to be read from client */
  void ReadClient();

  /** Called when connection is terminated */
  void ClientClosed();
  
  /**
   * Called when a new client connects
   *
   * @param aSocket           The network socket for the client
   */
  void NewClient(QSocket* aSocket);
};

#endif
