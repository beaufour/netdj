/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/**
 * \file main.cpp
 * Global initialization and startup.
 *
 * $Id$
 *
 *  (c) 2004, Allan Beaufour Larsen <allan@beaufour.dk>
 */

#include <iostream>

// Fork
#include <sys/types.h>
#include <unistd.h>

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

  /* Fork code
  int pid = fork();
  if (pid == -1) {
    std::cout << "Hmmm, couldn't fork into the background?!" << std::endl;
    std::cout << "  " << strerror(errno) << std::endl;
    exit (-1);
  }

  if (pid) {
    std::cout << "Spawned daemon (pid " << pid << ")" << std::endl;
    exit(0);
  }

  // Daemon
  setpgid(0, 0);
  freopen("/dev/null", "r", stdin);
  freopen("/var/log/netdj", "w", stdout);
  freopen("/dev/null", "w", stderr);
  std::cout << "NetDJ v" << VERSION << " starting up." << std::endl;  

  */


  /* Create logger service */
  FileLogger flog(&gLogger, "-", 999, &app);

  gLogger.LogMessage("Initializing song collections", 10);
  Collections cols;
  ICollection* newcol = new Collection_Songlist_Dir("request",
                                                    "Requests",
                                                    gConfig.GetString("REQUEST_DIR"),
                                                    true,
                                                    true);
  Q_CHECK_PTR(newcol);
  cols.AddCollection(newcol);
  newcol = new Collection_Songlist_File("share",
                                        "Shares",
                                        "mp3.list");
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
  QObject::connect(playerthread,  SIGNAL(SigSongPlaying(const ISong*, const ICollection*)),
                   &gLogger,      SLOT(LogSongPlaying(const ISong*, const ICollection*)));
  QObject::connect(playerthread,  SIGNAL(SigStart()),
                   &gLogger,      SLOT(LogPlayerStart()));
  QObject::connect(playerthread,  SIGNAL(SigStop()),
                   &gLogger,      SLOT(LogPlayerStop()));

  gLogger.LogMessage("Initializing server", 10);
  Server* server = new Server(&cols, 7676, 5, &app);
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
  QObject::connect(playerthread, SIGNAL(SigSongPlaying(const ISong*, const ICollection*)),
                   server,       SLOT(SongPlaying(const ISong*, const ICollection*)));
  

  // Starting playerthread
  playerthread->start(QThread::HighPriority);

  // Main application loop
  app.exec();

  // Main application loop has ended
  playerthread->Stop();

  /* Wait for thread to quit */
  playerthread->wait();
}
