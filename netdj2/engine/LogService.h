/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/**
 * \file LogService.h
 * class LogService
 *
 * $Id$
 *
 */

#ifndef __LOGSERVICE_H__
#define __LOGSERVICE_H__

#include <qobject.h>
#include <qdom.h>

#include "Song.h"

class Collection;

/**
 * A service that listens for signals from other objects, and creates log
 * entries.
 *
 * The LogService class is responsible for converting the native signals to an
 * XML document, and emit a NewLogEntry() with the XML document that
 * interested parties can listen on.
 *
 * Each log entry is given a level, where 0 is the most important
 * level. Listeners can use this to filter out entries.
 *
 * All entries have a
 * - unique id
 * - timestamp
 * - level
 */
class LogService : public QObject
{
  Q_OBJECT

private:
  /** The number of log entries */
  unsigned int mLogCount;

  /** The DOM document containing the log entries */
  QDomDocument mDocument;

  /** Create a new entry */
  void CreateEntry(QDomElement& aEntry, const int aLevel, const QString aType);

  /** Emit a simple entry */
  void SimpleEntry(const int aLevel, const QString aType);
  
public:
  /** Constructor */
  LogService(QObject* aParent = 0);

public slots:
  /**
   * Called when a new song is playing.
   *
   * @param aSong             The song
   * @param aCol              The collection it comes from
   */
  void LogSongPlaying(const Song& aSong, const Collection* aCol);

  /** Receiving a 'quit' command */
  void LogQuit();
  
  /** Receiving a 'skip' command */
  void LogSkip();

  /** The PlayerThread starts */
  void LogPlayerStart();
  
  /** The PlayerThread stops */
  void LogPlayerStop();

  /** A new client connects */
  void LogClientNew();
  
  /** A client connection is closed */
  void LogClientClose();

  /** A generic message */
  void LogMessage(const QString& aMsg, const unsigned int aLevel);

signals:
  /**
   * Emitted when a new log entry is registered.
   *
   * NB. aLevel seems unecessary since it is also in the aEntry, but it is a
   * lot quicker filtering on an integer, than parsing through a QDomElement.
   *
   * @param aEntry            The content of the log entry, in XML
   * @param aLevel            The log entry level
   */
  void NewLogEntry(const QDomElement* aEntry, const unsigned int aLevel);
};

#endif
