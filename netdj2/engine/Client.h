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

namespace NetDJ
{

  /**
   * This class will contain all the information about each currently connected client.
   */
  class Client 
  {
  private:
    static int mIDCount;
    int mID;
    QSocket* mSocket;
    int mLogLevel;
    
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
     * Get log level
     *
     * @return                  Log level
     */
    int GetLogLevel() const;
    
    /**
     * Set log level
     *
     * @param aLevel            Log level
     */
    void SetLogLevel(int aLevel);
    
    /**
     * Get the unique client number.
     *
     * @return                  The client id
     */
    int GetID() const;
  };
}

#endif
