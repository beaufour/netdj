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

#include <qdom.h>
#include <qhttp.h>
#include <qmutex.h>
#include <qobject.h>
#include <qptrdict.h>

#include "Client.h"
#include "StdException.h"

class QDomElement;

namespace NetDJ
{

  class ServerSocket;
  class IAccessChecker;
  class ICollection;
  class ISong;
  class Collections;

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
   * @todo Implement StartNetwork and StopNetwork, including signals.
   *
   * @todo Implement Digest authentication (http://rfc.sunsite.dk/rfc/rfc2617.html)
   
   * @todo Implement SigCommand signal instead of using Message()
   */
  class Server : public QObject
  {
    Q_OBJECT
    
  private:
    /** The current server socket */
    ServerSocket* mServerSocket;
    
    /** List of currently connected clients */
    QPtrDict<Client> mClients;
    
    /** The access checker used */
    IAccessChecker* mAccessChecker;
    
    /** Document used for XML representation */
    QDomDocument mDocument;

    /** Document used for representation of currently playing song */
    QDomElement* mSongElement;

    /** Mutex used to lock mSongElement */
    mutable QMutex mSongMutex;
    
    /** Pointer to song Collections */
    Collections* mCols;

    /**
     * Get a copy of the currently playing song document
     */
    QDomElement* GetSong();
    
    /**
     * Handles commands from clients.
     * It throws CMD_Unauthorized and CMD_Invalid.
     *
     * @param aStream           Stream to write results to
     * @param aHeader           The request header
     * @return                  Close connection?
     */
    bool HandleCommand(QTextStream& aStream,
                       const QHttpRequestHeader& aHeader);
    
    /**
     * Check user authorization for a given level.
     * 
     * For now, the aAuthString is just a HTTP Basic authentication string.
     *
     * @param aLevel            Authorization level
     * @param aAuthString       User authorization string
     * @param aUserName         Returns the user name
     * @return                  Is user authorized?
     */
    bool CheckAuthorization(unsigned int aLevel,
                            QString aAuthString,
                            QString& aUserName);
    
    /**
     * Command: Show help.
     *
     * @param aStream           The stream to send output to
     */
    void CmdHelp(QTextStream& aStream);
    
    /**
     * Command: Enable continous logging for caller
     *
     * @param aStream           The stream to send output to
     */
    void CmdLog(QTextStream& aStream);
    
    /**
     * Command: Skip current song.
     *
     * @param aStream           The stream to send output to
     * @param aUsername         The user that ran the command
     */
    void CmdSkip(QTextStream& aStream, const QString& aUsername);
    
    /**
     * Command: Shutdown NetDJ
     *
     * @param aStream           The stream to send output to
     * @param aUsername         The user that ran the command
     */
    void CmdShutdown(QTextStream& aStream, const QString& aUsername);
    
    /**
     * Command: Get current song playing
     *
     * @param aStream           The stream to send output to
     */
    void CmdCurrent(QTextStream& aStream);
    
    /**
     * Command: Get the request list
     *
     * @param aStream           The stream to send output to
     */
    void CmdRequests(QTextStream& aStream);
    
  public:
    /**
     * Constructor.
     *
     * @param aCols             A pointer to the collections
     * @param aPort             The port number to listen on
     * @param aBackLog          The backlog of connections to keep
     * @param aParent           The parent QObject
     */
    Server(Collections* aCols, int aPort, int aBackLog, QObject* aParent = 0);
    
    /** Destructor */
    ~Server();
    
  signals:
    /** Emitted on receiving a 'quit' command */
    void SigQuit(const QString& aUsername);
    
    /** Emitted on receiving a 'skip' command */
    void SigSkip(const QString& aUsername);
    
    /** Emitted when a new client connects */
    void SigClientNew();
    
    /** Emitted when a client connection is closed */
    void SigClientClose();  
    
    /** Emits a general information message */
    void SigMessage(const QString& aMsg, const unsigned int aLevel);
    
    /** Emits an exception */
    void SigException(const QString& aName, const QString& aMsg);
    
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

    /**
     * Called when a new song is playing.
     *
     * @param aSong             The song
     * @param aCol              The collection it comes from
     */
    void SongPlaying(const ISong* aSong, const ICollection* aCol);

    /** Called when a new log entry is made */
    void NewLogEntry(const QDomElement* aEntry, const unsigned int aLevel);
  };
}

#endif
