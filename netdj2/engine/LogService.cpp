/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/**
 * \file LogService.cpp
 * class LogService
 *
 * $Id$
 *
 */

#include "LogService.h"

#include <qapp.h>
#include <qdom.h>
#include <qdatetime.h>
#include <qthread.h>

#include "ISong.h"
#include "ICollection.h"

using namespace NetDJ;

/**
 * @note QThread::currentThread is only for internal Qt use, but as long as Qt
 * is running on Linux and using Posix threads, it should work I guess. But it
 * is not optimal. I need a way of determining the id of the current thread....
 */
LogService::LogService(QObject* aParent)
  : QObject(aParent, "LogService"),
    mLogCount(0),
    mDocument("log"),
    mOwnerThread((int) QThread::currentThread())
{
  mEntryQueue.setAutoDelete(true);
}

void
LogService::Emit(QDomElement& aEntry, const int aLevel, EntryClass_t aClass)
{

  aEntry.setAttribute("id", ++mLogCount);
  aEntry.setAttribute("timestamp", QDateTime::currentDateTime().toString(Qt::ISODate));
  aEntry.setAttribute("level", aLevel);
  QString class_str;
  switch (aClass) {
    case ENTRY_CRIT:
      class_str = "critical";
      break;
      
    case ENTRY_WARN:
      class_str = "warning";
      break;
      
    case ENTRY_INFO:
      class_str = "info";
      break;      
  }
  aEntry.setAttribute("class", class_str);

  QMutexLocker lock(&mEmitMutex);
  if ((int) QThread::currentThread() == mOwnerThread) {
    emit NewLogEntry(&aEntry, aLevel);
  } else {
    mEntryQueue.append(new QDomElement(aEntry));
    QApplication::postEvent(this, new QEvent(QEvent::User));
  }
}

bool
LogService::event(QEvent *aEvent)
{
  bool rv = false;
  if (aEvent && aEvent->type() == QEvent::User) { 
    QDomElement* entry;
    for (entry = mEntryQueue.first(); entry; entry = mEntryQueue.next()) {
      emit NewLogEntry(entry, entry->attribute("level", "0").toInt());
    }
    
    mEntryQueue.clear();
    rv = true;
  }
  return rv;
}

void
LogService::CreateEntry(QDomElement& aEntry, const QString aName)
{
  aEntry = mDocument.createElement("entry");

  aEntry.setAttribute("sender", sender() ? sender()->name() : "(none)");
  aEntry.setAttribute("name", aName);
}

void
LogService::SimpleEntry(const int aLevel, const QString aName, EntryClass_t aClass)
{
  QDomElement e;
  CreateEntry(e, aName);
  Emit(e, aLevel, aClass);
}


void
LogService::LogSongPlaying(const ISong* aSong, const ICollection* aCol)
{
  QDomElement e;
  CreateEntry(e, "SongPlaying");

  QDomElement* song = aSong->AsXML(&mDocument);
  song->setAttribute("collection", aCol->GetIdentifier());
  e.appendChild(*song);
  delete song;
  
  Emit(e, 10, ENTRY_INFO);
}

void
LogService::LogQuit(const QString& aUsername)
{
  QDomElement e;
  CreateEntry(e, "Shutdown");
  QDomElement uNode = mDocument.createElement("user");
  e.appendChild(uNode);
  QDomText uName = mDocument.createTextNode(aUsername);
  uNode.appendChild(uName);
  Emit(e, 0, ENTRY_CRIT);
}
  
void
LogService::LogSkip(const QString& aUsername)
{
  QDomElement e;
  CreateEntry(e, "Skip");
  QDomElement uNode = mDocument.createElement("user");
  e.appendChild(uNode);
  QDomText uName = mDocument.createTextNode(aUsername);
  uNode.appendChild(uName);
  Emit(e, 20, ENTRY_INFO);
}

void
LogService::LogClientNew()
{
  SimpleEntry(100, "ClientNew", ENTRY_INFO);
}  

void
LogService::LogClientClose()
{
  SimpleEntry(100, "ClientClose", ENTRY_INFO);
}

void
LogService::LogPlayerStart()
{
  SimpleEntry(30, "PlayerStart", ENTRY_INFO);
}

void
LogService::LogPlayerStop()
{
  SimpleEntry(30, "PlayerStop", ENTRY_CRIT);
}

void
LogService::LogMessage(const QString& aMsg, const unsigned int aLevel, EntryClass_t aClass)
{
  QDomElement e;
  CreateEntry(e, "Message");
  QDomText msg = mDocument.createTextNode(aMsg);
  e.appendChild(msg);
  Emit(e, aLevel, aClass);
}
