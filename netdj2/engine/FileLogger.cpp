/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/**
 * \file FileLogger.cpp
 * class FileLogger
 *
 * $Id$
 *
 */

#include "FileLogger.h"

#include <qdom.h>

using namespace std;

FileLogger::FileLogger(const QString aFilename,
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
  
}

FileLogger::~FileLogger()
{
  mStream << flush;
}

void
FileLogger::NewLogEntry(const QDomElement* aEntry, const unsigned int aLevel)
{
  if (aLevel > mLevel)
    return;

  mStream << *aEntry;
}
