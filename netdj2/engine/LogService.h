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

#include <qdom.h>
#include <qmutex.h>
#include <qobject.h>
#include <qptrlist.h>

namespace NetDJ 
{

  class ICollection;
  class ISong;

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
   *
   * The service only emits signals in the parent context, if other threads
   * post signals on this service they are queued, and an event (QEvent::User
   * + 1) is posted to itself. Upon receiving the event (in the parent
   * context) the queued entries are processed.
   *
   * @note The class is reentrant, but it is important that a NewLogEntry()
   * signal does not spawn another call to LogService in the same
   * context. This will make Emit() wait for itself.
   */
  class LogService : public QObject
  {
    Q_OBJECT

  public:
    /** The different entry classes */
    enum EntryClass_t {
      ENTRY_CRIT,
      ENTRY_WARN,
      ENTRY_INFO
    };

  private:
    /** The number of log entries */
    unsigned int mLogCount;

    /** Emit() locks on this one */
    QMutex mEmitMutex;
    
    /** The DOM document containing the log entries */
    QDomDocument mDocument;
    
    /** The thread owning the class */
    int mOwnerThread;

    /**
     * A queue for delayed entries, that is entries not comming from the main
     * application context
     */
    QPtrList<QDomElement> mEntryQueue;

    /** Create a new entry */
    void CreateEntry(QDomElement& aEntry, const QString aName);

    /** Emit a simple entry */
    void SimpleEntry(const int aLevel, const QString aName, EntryClass_t aClass);

    /** Emits the NewLogEntry signal _serialized_ */
    void Emit(QDomElement& e, const int aLevel, EntryClass_t aClass);
    
  public:
    /** Constructor */
    LogService(QObject* aParent = 0);

    /** Event handler */
    bool event(QEvent* aEvent);

  public slots:
    /**
     * Called when a new song is playing.
     *
     * @param aSong             The song
     * @param aCol              The collection it comes from
     */
    void LogSongPlaying(const ISong* aSong, const ICollection* aCol);
    
    /** Receiving a 'quit' command */
    void LogQuit(const QString& aUsername);
    
    /** Receiving a 'skip' command */
    void LogSkip(const QString& aUsername);
    
    /** The PlayerThread starts */
    void LogPlayerStart();
    
    /** The PlayerThread stops */
    void LogPlayerStop();
    
    /** A new client connects */
    void LogClientNew();
    
    /** A client connection is closed */
    void LogClientClose();
    
    /** A generic message */
    void LogMessage(const QString& aMsg, const unsigned int aLevel, LogService::EntryClass_t aClass);
    
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
}

#endif
