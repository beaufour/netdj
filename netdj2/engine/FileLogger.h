/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/**
 * \file FileLogger.h
 * class FileLogger
 *
 * $Id$
 *
 */

#ifndef __FILELOGGER_H__
#define __FILELOGGER_H__

#include <qfile.h>
#include <qobject.h>
#include <qstring.h>
#include <qtextstream.h>

class QDomElement;

namespace NetDJ
{

  class LogService;
  
  class FileLogger : public QObject
  {
    Q_OBJECT

  private:
    unsigned int mLevel;
    
    QFile mFile;
    
    QTextStream mStream;
    
  public:
    FileLogger(LogService* aLogService,
               const QString aFilename,
               const unsigned int aLevel,
               QObject* aParent = 0);
    ~FileLogger();
    
  public slots:
    void NewLogEntry(const QDomElement* aEntry, const unsigned int aLevel);
  };
}

#endif
