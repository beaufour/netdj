/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/**
 * \file Client.h
 * class Client
 *
 * $Id$
 *
 */

#ifndef __CLIENT_H__
#define __CLIENT_H__

class QSocket;

/**
 * This class will contain all the information about each currently connected client.
 */
class Client 
{
private:
  static int mIDCount;
  int mID;
  QSocket* mSocket;
  
public:
  /**
   * Constructor, takes ownership of the socket.
   *
   * @param aSocket           The network socket for the client
   */
  Client(QSocket* aSocket);

  /** Destructor */
  ~Client();

  /**
   * Get the network socket for the client
   *
   * @return                  The network socket
   */
  QSocket* GetSocket();

  /**
   * Get the unique client number.
   *
   * @return                  The client id
   */
  int GetID() const;
};

#endif
