/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/**
 * \file main.cpp
 * Global initialization and startup.
 *
 * $Id$
 *
 *  (c) 2004, Allan Beaufour Larsen <allan@beaufour.dk>
 */

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cstdlib>

#include <qapplication.h>

#include "config.h"
#include "Collections.h"
#include "Collection_Songlist_File.h"
#include "Collection_Songlist_Dir.h"
#include "Configuration.h"
#include "FileLogger.h"
#include "LogService.h"
#include "PlayerThread.h"
#include "Server.h"

namespace NetDJ
{
  Configuration gConfig;
  LogService gLogger;
}

using namespace std;
using namespace NetDJ;

void
myMessageOutput(QtMsgType aType, const char *aMsg)
{
  switch (aType) {
    case QtDebugMsg:
      fprintf(stderr, "Debug: %s\n", aMsg);
      break;
    case QtWarningMsg:
      fprintf(stderr, "Warning: %s\n", aMsg);
      break;
    case QtFatalMsg:
      fprintf(stderr, "Fatal: %s\n", aMsg);
      abort();                    // deliberately core dump
  }
}

int
main(int argc, char* argv[])
{
  /* Use verbose terminate handler, prints out name of exception,
     etc. */
  std::set_terminate (__gnu_cxx::__verbose_terminate_handler);

  // @todo Catch ctrl-c and call quit()
  
  QApplication app( argc, argv );
  
  /* Seed random number generator */
  srand(time(0));
  
  /* Install Qt message handler */
  qInstallMsgHandler(myMessageOutput);

  cout << "Reading configuration" << endl;
  if (!gConfig.Init()) {
    cerr << "FATAL ERROR! Could not get configuration!\n" << endl;
    return -1;
  }

  // @todo parse command line options, should overrule configuration file.


  /* Create logger service */
  FileLogger flog(&gLogger, "-", 999, &app);

  gLogger.LogMessage("Initializing song collections", 10);
  Collections cols;
  Collection* newcol = new Collection_Songlist_Dir("request", "Requests", "/tmp/netdj_request/", true);
  Q_CHECK_PTR(newcol);
  cols.AddCollection(newcol);
  newcol = new Collection_Songlist_File("share", "Shares", "mp3.list");
  Q_CHECK_PTR(newcol);
  cols.AddCollection(newcol);
  
  gLogger.LogMessage("Initializing player", 10);
  PlayerThread* playerthread = new PlayerThread(&cols, 0, &app);
  Q_CHECK_PTR(playerthread);

  // Connect gLogger
  QObject::connect(playerthread,  SIGNAL(SigMessage(const QString&, const unsigned int)),
                   &gLogger,      SLOT(LogMessage(const QString&, const unsigned int)));
  QObject::connect(playerthread,  SIGNAL(SigException(const QString&, const QString&)),
                   &gLogger,      SLOT(LogException(const QString&, const QString&)));
  QObject::connect(playerthread,  SIGNAL(SigSongPlaying(const Song&, const Collection*)),
                   &gLogger,      SLOT(LogSongPlaying(const Song&, const Collection*)));
  QObject::connect(playerthread,  SIGNAL(SigStart()),
                   &gLogger,      SLOT(LogPlayerStart()));
  QObject::connect(playerthread,  SIGNAL(SigStop()),
                   &gLogger,      SLOT(LogPlayerStop()));

  gLogger.LogMessage("Initializing server", 10);
  Server* server = new Server(7676, 5, &app);
  Q_CHECK_PTR(server);

  // Connect gLogger
  QObject::connect(server,   SIGNAL(SigMessage(const QString&, const unsigned int)),
                   &gLogger, SLOT(LogMessage(const QString&, const unsigned int)));
  QObject::connect(server,   SIGNAL(SigException(const QString&, const QString&)),
                   &gLogger, SLOT(LogException(const QString&, const QString&)));
  QObject::connect(server,   SIGNAL(SigQuit(const QString&)),
                   &gLogger, SLOT(LogQuit(const QString&)));
  QObject::connect(server,   SIGNAL(SigSkip(const QString&)),
                   &gLogger, SLOT(LogSkip(const QString&)));
  QObject::connect(server,   SIGNAL(SigClientNew()),
                   &gLogger, SLOT(LogClientNew()));
  QObject::connect(server,   SIGNAL(SigClientClose()),
                   &gLogger, SLOT(LogClientClose()));

  // Connect quit signal to application
  QObject::connect(server, SIGNAL(SigQuit(const QString&)),
                   qApp,   SLOT(quit()));

  // Connect playerthread and server
  QObject::connect(server,       SIGNAL(SigSkip(const QString&)),
                   playerthread, SLOT(Skip()));
  QObject::connect(playerthread, SIGNAL(SigSongPlaying(const Song&, const Collection*)),
                   server,       SLOT(SongPlaying(const Song&, const Collection*)));
  

  // Starting playerthread
  playerthread->start(QThread::HighPriority);

  // Main application loop
  app.exec();

  // Main application loop has ended
  playerthread->Stop();

  /* Wait for thread to quit */
  playerthread->wait();
}
