// -*- c++ -*-
/**
 * \file Shout.h
 * Class Shout.
 *
 * $Id$
 *
 */

#ifndef __SHOUT_H__
#define __SHOUT_H__

#include <string>
#include <shout/shout.h>

#include "StdException.h"

/**
 * Exception thrown by functions of Shout.
 */
class ShoutException : public StdException {
public:
  /**
   * Constructor.
   *
   * @param msg     The errormessage
   */
  ShoutException(const std::string aMsg);
};

class Shout;

/**
 *
 */
class Shout_Metadata {
private:
  /** The metadata handle */
  shout_metadata_t* mMetaData;

public:
  /** Constructor */
  Shout_Metadata();

  /** Destructor */
  ~Shout_Metadata();

  /**
   * Add name, value pair.
   *
   * "song" seems to be one name....
   *
   * @param name    The name
   * @param value   The value
   */
  void Add(const std::string aName, const std::string aValue);

  friend class Shout;
};



/**
 * Class to handle an IceCast stream through the libshout library.
 *
 */
class Shout {
protected:
  /** The connection handle */
  shout_t* mShout;

  /**
   * Error wrapper function. Checks the number and throws a
   * ShoutException with the error message if err is not
   * SHOUTERR_SUCCESS.
   *
   * @param func    The function returning the error
   * @param err     The error number
   */
  inline void ErrorWrapper(const std::string aFunc, const int aErr);

  /** Initialize libshout */
  void Init();

public:
  /**
   * Constructor.
   */
  Shout();

  /**
   * Constructor.
   *
   * @param name    The name of the stream (not the mountpoint!)
   * @param genre   The genre of music played
   * @param descr   Description of the stream
   */
  Shout(const std::string aName, const std::string aGenre,
	const std::string aDescr);


  /** Destructor */
  ~Shout();

  /** Connect to icecast server */
  void Connect();

  /** Disconnect from icecast server */
  void Disconnect();

  /**
   * Stream data (music) to icecast server.
   *
   * @param data    The data to send
   * @param len     The length of the data
   */
  void Send(const unsigned char *aData, size_t aLen);

  /** Sleep until it is time to send more data */
  void Sleep() throw();

  /**
   * Get the amount of time (in ms) the caller should wait before
   * sending data again.
   *
   * @return        The delay in ms
   */
  int GetDelay() throw();

  /**
   * Set the hostname of the icecast server to connect to.
   *
   * @param host    The hostname
   */
  void SetHost(const std::string aHost);

  /**
   * Set the port number for the icecast server
   *
   * @param port    The port number
   */
  void SetPort(const unsigned short aPort);

  /**
   * Set the mountpoint for the stream (ie. the URL)
   *
   * @param mount   The mountpoint
   */
  void SetMount(const std::string aMount);

  /**
   * Set the username to authenticate with.
   *
   * @param aUserName The username
   */
  void SetUser(const std::string aUserName);

  /**
   * Set the password to authenticate with. 
   *
   * @param pass    The password
   */
  void SetPassword(const std::string aPass);

  /**
   * Set the stream format (ie. Vorbis or MP3)
   *
   * @param format  The format (Vorbis = 0, MP3 = 1)
   */
  void SetFormat(const unsigned int aFormat);

  /**
   * Set the protocol to use to communicate with icecast server (HTTP,
   * XAudioCast or Icy)
   *
   * @param prot    The protocol type.
   */
  void SetProtocol(const unsigned int aProt);

  /**
   * Set whether the stream is public or not.
   *
   * @param pub     Is stream public?
   */
  void SetPublic(const bool aPublic);

  /**
   * Set the name of the stream (not mountpoint!)
   *
   * @param name    The name
   */
  void SetName(const std::string aName);

  /**
   * Set the genre of music played
   *
   * @param genre   The genre
   */
  void SetGenre(const std::string aGenre);

  /**
   * Set the textual description of the stream
   *
   * @param descr   The description
   */
  void SetDescription(const std::string aDescr);

  /**
   * Set the songname of the current song.
   *
   * @param name    The name of the song.
   */
  void SetSongName(const std::string aName);

};

#endif
