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

#include "Song.h"
#include "Collection.h"

using namespace std;

LogService::LogService(QObject* aParent)
  : QObject(aParent, "LogService"),
    mLogCount(0),
    mDocument("log")
{
}

void
LogService::CreateEntry(QDomElement& aEntry, const int aLevel, const QString aType)
{
  aEntry = mDocument.createElement("entry");

  aEntry.setAttribute("sender", sender() ? sender()->name() : "(none)");
  aEntry.setAttribute("type", aType);
  aEntry.setAttribute("level", aLevel);
  aEntry.setAttribute("id", ++mLogCount);
}

void
LogService::SimpleEntry(const int aLevel, const QString aType)
{
  QDomElement e;
  CreateEntry(e, aLevel, aType);
  emit NewLogEntry(&e, aLevel);
}


void
LogService::LogSongPlaying(const Song& aSong, const Collection* aCol)
{
  SimpleEntry(10, "SongPlaying");
}

void
LogService::LogQuit()
{
  SimpleEntry(0, "Quit");
}
  
void
LogService::LogSkip()
{
  SimpleEntry(20, "Skip");
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
  emit NewLogEntry(&e, aLevel);
}

