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
#include "LogService.h"
#include "PlayerThread.h"
#include "Server.h"

Configuration NETDJ_CONF;

using namespace std;

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
  
  QApplication app( argc, argv );
  
  /* Seed random number generator */
  srand(time(0));
  
  /* Install Qt message handler */
  qInstallMsgHandler(myMessageOutput);

  cout << "Reading configuration" << endl;
  if (!NETDJ_CONF.Init()) {
    cerr << "FATAL ERROR! Could not get configuration!\n" << endl;
    return -1;
  }

  /* Create logger service */
  LogService logger(&app);

  cout << "Initializing song collections" << endl;
  Collections cols;
  Collection* newcol = new Collection_Songlist_Dir("request", "Requests", "/tmp/netdj_request/", true);
  Q_CHECK_PTR(newcol);
  cols.AddCollection(newcol);
  newcol = new Collection_Songlist_File("share", "Shares", "mp3.list");
  Q_CHECK_PTR(newcol);
  cols.AddCollection(newcol);
  
  cout << "Initializing player" << endl;
  PlayerThread* playerthread = new PlayerThread(&cols, 0, &app);
  Q_CHECK_PTR(playerthread);

  // Connect logger
  QObject::connect(playerthread, SIGNAL(SigSongPlaying(const Song&, const Collection*)), &logger, SLOT(LogSongPlaying(const Song&, const Collection*)));
  QObject::connect(playerthread, SIGNAL(SigStart()), &logger, SLOT(LogPlayerStart()));
  QObject::connect(playerthread, SIGNAL(SigStop()), &logger, SLOT(LogPlayerStop()));

  cout << "Initializing server" << endl;
  Server* server = new Server(7676, 5, &app);
  Q_CHECK_PTR(server);

  // Connect logger
  QObject::connect(server, SIGNAL(SigQuit()), &logger, SLOT(LogQuit()));
  QObject::connect(server, SIGNAL(SigSkip()), &logger, SLOT(LogSkip()));
  QObject::connect(server, SIGNAL(SigClientNew()), &logger, SLOT(LogClientNew()));
  QObject::connect(server, SIGNAL(SigClientClose()), &logger, SLOT(LogClientClose()));

  // Connect quit signal to application
  QObject::connect(server, SIGNAL(SigQuit()), qApp, SLOT(quit()));

  // Connect playerthread and server
  QObject::connect(server, SIGNAL(SigSkip()), playerthread, SLOT(Skip()));
  QObject::connect(playerthread, SIGNAL(SigSongPlaying(const Song&, const Collection*)), server, SLOT(SongPlaying(const Song&, const Collection*)));
  
  cout << "Starting player" << endl;
  playerthread->start(QThread::HighPriority);

  // Main application loop
  app.exec();

  // Main application loop has ended
  cout << "Stopping player" << endl;
  playerthread->Stop();

  /* Wait for thread to quit */
  playerthread->wait();
  cout << "main() finished!" << endl;
}
