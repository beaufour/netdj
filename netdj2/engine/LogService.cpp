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


#include <iostream>


LogService::LogService(QObject* aParent)
  : QObject(aParent, "LogService"),
    mLogCount(0)
{
}

void
LogService::LogSongPlaying(const Song& aSong, const Collection* aCol)
{
  printf("LOG: LogSongPlaying\n");
}

void
LogService::LogQuit()
{
  printf("LOG: LogQuit\n");
}
  
void
LogService::LogSkip()
{
  printf("LOG: LogSkip\n");
}

void
LogService::LogClientNew()
{
  printf("LOG: LogClientNew\n");
}  

void
LogService::LogClientClose()
{
  printf("LOG: LogClientClose\n");
}

void
LogService::LogPlayerStart()
{
  printf("LOG: LogPlayerStart\n");
}

void
LogService::LogPlayerStop()
{
  printf("LOG: LogPlayerStop\n");
}

