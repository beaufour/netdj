/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/**
 * \file LogService.cpp
 * class LogService
 *
 * $Id$
 *
 */

#include "LogService.h"

#include <qdom.h>

#include "ISong.h"
#include "ICollection.h"

using namespace NetDJ;

LogService::LogService(QObject* aParent)
  : QObject(aParent, "LogService"),
    mLogCount(0),
    mDocument("log")
{
}

void
LogService::Emit(QDomElement& aEntry, const int aLevel)
{
  QMutexLocker lock(&mEmitMutex);

  aEntry.setAttribute("id", ++mLogCount);
  emit NewLogEntry(&aEntry, aLevel);
}

void
LogService::CreateEntry(QDomElement& aEntry, const int aLevel, const QString aName)
{
  aEntry = mDocument.createElement("entry");

  aEntry.setAttribute("sender", sender() ? sender()->name() : "(none)");
  aEntry.setAttribute("name", aName);
  aEntry.setAttribute("level", aLevel);
}

void
LogService::SimpleEntry(const int aLevel, const QString aName)
{
  QDomElement e;
  CreateEntry(e, aLevel, aName);
  Emit(e, aLevel);
}


void
LogService::LogSongPlaying(const ISong* aSong, const ICollection* aCol)
{
  QDomElement e;
  CreateEntry(e, 10, "SongPlaying");

  QDomElement* song = aSong->AsXML(&mDocument);
  song->setAttribute("collection", aCol->GetIdentifier());
  e.appendChild(*song);
  delete song;
  
  Emit(e, 10);
}

void
LogService::LogQuit(const QString& aUsername)
{
  QDomElement e;
  CreateEntry(e, 0, "Shutdown");
  QDomElement uNode = mDocument.createElement("user");
  e.appendChild(uNode);
  QDomText uName = mDocument.createTextNode(aUsername);
  uNode.appendChild(uName);
  Emit(e, 0);
}
  
void
LogService::LogSkip(const QString& aUsername)
{
  QDomElement e;
  CreateEntry(e, 20, "Skip");
  QDomElement uNode = mDocument.createElement("user");
  e.appendChild(uNode);
  QDomText uName = mDocument.createTextNode(aUsername);
  uNode.appendChild(uName);
  Emit(e, 20);
}

void
LogService::LogClientNew()
{
  SimpleEntry(100, "ClientNew");
}  

void
LogService::LogClientClose()
{
  SimpleEntry(100, "ClientClose");
}

void
LogService::LogPlayerStart()
{
  SimpleEntry(30, "PlayerStart");
}

void
LogService::LogPlayerStop()
{
  SimpleEntry(30, "PlayerStop");
}

void
LogService::LogMessage(const QString& aMsg, const unsigned int aLevel)
{
  QDomElement e;
  CreateEntry(e, aLevel, "Message");
  QDomText msg = mDocument.createTextNode(aMsg);
  e.appendChild(msg);
  Emit(e, aLevel);
}

void
LogService::LogException(const QString& aName, const QString& aMsg)
{
  QDomElement e;
  CreateEntry(e, 0, "Exception");
  e.setAttribute("class", aName);
  QDomText msg = mDocument.createTextNode(aMsg);
  e.appendChild(msg);
  Emit(e, 0);
}

