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
#include <qhttp.h>

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
 * The main server process.
 *
 * Handles all commands from users.
 *
 * @todo Implement Digest authentication (http://rfc.sunsite.dk/rfc/rfc2617.html)
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

private:
  /**
   * Handles commands from clients.
   * It throws CMD_Unauthorized and CMD_Invalid.
   *
   * @param aStream           Stream to write results to
   * @param aHeader           The request header
   */
  void HandleCommand(QTextStream& aStream,
                     const QHttpRequestHeader& aHeader);

  /**
   * Check user authorization for a given level.
   * 
   * For now, the aAuthString is just a HTTP Basic authentication string.
   *
   * @param aLevel            Authorization level
   * @param aAuthString       User authorization string
   * @return                  Is user authorized?
   */
  bool CheckAuthorization(unsigned int aLevel, QString aAuthString);

  /**
   * Command: Show help.
   *
   * @param aStream           The stream to send output to
   */
  void CmdHelp(QTextStream& aStream);
  
  /**
   * Command: Skip current song.
   *
   * @param aStream           The stream to send output to
   */
  void CmdSkip(QTextStream& aStream);
  
  /**
   * Command: Shutdown NetDJ
   *
   * @param aStream           The stream to send output to
   */
  void CmdShutdown(QTextStream& aStream);
  
  /**
   * Command: Get index file, information about current song etc.
   *
   * @param aStream           The stream to send output to
   */
  void CmdIndex(QTextStream& aStream);
  
signals:
  /** Emitted on receiving a 'quit' command */
  void SigQuit();
  
  /** Emitted on receiving a 'skip' command */
  void SigSkip();
  
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
