// -*- c++ -*-
/**
 * \file HttpServer.h
 * class HttpServer
 *
 * $Id$
 *
 */

#ifndef __HTTPSERVER_H__
#define __HTTPSERVER_H__

#include <qserversocket.h>

#include "PlayerThread.h"
#include "StdException.h"

/**
 * Exception thrown by HttpServer.
 */
class HttpError : public StdException {
public:
  /**
   * Constructor.
   *
   * @param err     The errormessage
   */
  HttpError(const std::string err);
};

/**
 * The HTTP-server.
 */
class HttpServer : public QServerSocket {
  Q_OBJECT
private:
  /** Pointer to the PlayerThread */
  PlayerThread* Player;

public:
  /**
   * Constructor.
   *
   * @param port    The port number to listen on
   * @param backlog The backlog of connections to keep
   * @param pl      The PlayerThread to communicate with
   * @param parent  The parent QObject
   */
  HttpServer(Q_UINT16 port, int backlog,
	     PlayerThread* pl, QObject* parent = 0);

  /**
   * Called when a new connection is received.
   *
   * @param socket  The new connection socket.
   */
  void newConnection(int socket);

signals:
  void cmdQuit();
  
private slots:
  /** Called when data is ready to be read from client */
  void readClient();

  /** Called when connection is terminated */
  void discardClient();
};

#endif
