#ifndef SERVERSOCKET_H
#define SERVERSOCKET_H

// Qt includes
#include <qserversocket.h>

// Forward declarations
class QSocket;

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

#endif
