/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/**
 * \file FileLogger.cpp
 * class FileLogger
 *
 * $Id$
 *
 */

#include "FileLogger.h"
#include "LogService.h"

#include <qdom.h>

using namespace std;
using namespace NetDJ;

FileLogger::FileLogger(LogService* aLogService,
                       const QString aFilename,
                       const unsigned int aLevel,
                       QObject* aParent)
  : QObject(aParent, "FileLogger"),
    mLevel(aLevel),
    mFile(aFilename),
    mStream(&mFile)
{
  if (aFilename == "-") {
    mFile.open(IO_WriteOnly, stdout);
  } else {
    mFile.open(IO_WriteOnly);
  }
  if (aLogService) {
    connect(aLogService, SIGNAL(NewLogEntry(const QDomElement*, const unsigned int)),
            this,       SLOT(NewLogEntry(const QDomElement*, const unsigned int)));
  }
}

FileLogger::~FileLogger()
{
  mStream << flush;
  mFile.close();
}

void
FileLogger::NewLogEntry(const QDomElement* aEntry, const unsigned int aLevel)
{
  if (aLevel > mLevel)
    return;

  mStream << *aEntry;
}
