/**
 * \file ServerSocket.cpp
 * class ServerSocket
 *
 * $Id$
 *
 */

#include <qsocket.h>
#include "ServerSocket.h"

ServerSocket::ServerSocket(int aPort, int aBackLog, QObject* aParent, const char* aName)
  : QServerSocket(aPort, aBackLog, aParent, aName)
{
}

////////////////////////////////////////////////////////////////////////////////


ServerSocket::~ServerSocket()
{
}

////////////////////////////////////////////////////////////////////////////////

void
ServerSocket::newConnection(int aSocketFD)
{
  QSocket* socket = new QSocket(parent());
  Q_CHECK_PTR(socket);
  
  socket->setSocket(aSocketFD);

  emit newClient(socket);
}
