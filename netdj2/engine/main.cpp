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

// Command line handling
#include <popt.h>

// setuid, etc.
#include <pwd.h>
#include <grp.h>

// Signal handling
#include <csignal>

// QT Main App
#include <qapplication.h>

// NetDJ includes
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

QApplication* mainApp = 0;

void sig_handler(int signal)
{
  QString signum;
  signum.setNum(signal);
  gLogger.LogMessage("Received signal " + signum + ", quitting", 0);
  mainApp->quit();
}

void
myMessageOutput(QtMsgType aType, const char *aMsg)
{
  switch (aType) {
    case QtDebugMsg:
      gLogger.LogMessage(QString("[DEBUG]:") + aMsg, 200);
      break;
    case QtWarningMsg:
      gLogger.LogMessage(QString("[WARNING]:") + aMsg, 30);
      break;
    case QtFatalMsg:
      gLogger.LogMessage(QString("FATAL ERROR: ") + aMsg, 0);
      cerr << "FATAL ERROR: " <<  aMsg << endl;
      abort();                    // deliberately core dump
  }
}

int
mainloop(QApplication* aApp)
{
  /* Create logger service */
  FileLogger flog(&gLogger, "-", 999, aApp);

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
                                        gConfig.GetString("SHARE_DIR"));
  Q_CHECK_PTR(newcol);
  cols.AddCollection(newcol);
  
  gLogger.LogMessage("Initializing player", 10);
  PlayerThread* playerthread = new PlayerThread(&cols, 0, aApp);
  Q_CHECK_PTR(playerthread);

  // Connect player to gLogger
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
  Server* server = new Server(&cols, gConfig.GetInteger("SERVER_PORT"), 5, aApp);
  Q_CHECK_PTR(server);

  // Connect server to gLogger
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
                   aApp,   SLOT(quit()));

  // Connect playerthread and server
  QObject::connect(server,       SIGNAL(SigSkip(const QString&)),
                   playerthread, SLOT(Skip()));
  QObject::connect(playerthread, SIGNAL(SigSongPlaying(const ISong*, const ICollection*)),
                   server,       SLOT(SongPlaying(const ISong*, const ICollection*)));
  

  // Connect gLogger to server
  QObject::connect(&gLogger, SIGNAL(NewLogEntry(const QDomElement*, const unsigned int)),
                   server,   SLOT(NewLogEntry(const QDomElement*, const unsigned int)));


  // Starting playerthread
  playerthread->start(QThread::HighPriority);

  // Main application loop
  aApp->exec();

  // Main application loop has ended
  playerthread->Stop();

  /* Wait for thread to quit */
  playerthread->wait();

  return 0;  
}


int
main(int argc, char* argv[])
{
  /* Use verbose terminate handler, prints out name of exception,
     etc. */
  std::set_terminate (__gnu_cxx::__verbose_terminate_handler);

  // @todo Catch ctrl-c and call quit()
  
  QApplication app( argc, argv );
  mainApp = &app;
  
  /* Seed random number generator */
  srand(time(0));
  
  /* Install Qt message handler */
  qInstallMsgHandler(myMessageOutput);

  // Parse options
  int daemon = 0;
  int dump = 0;
  int port = 0;
  char *config = 0;
  
  struct poptOption optionsTable[] = {
    {"daemon", 'd', 0, &daemon, 0, "run in background", 0},
    {"config", 'c', POPT_ARG_STRING, config, 0, "use specific config file", 0},
    {"port",   'p', POPT_ARG_INT, &port, 0, "server TCP port", 0},
    {"dump",     0, 0, &dump, 0, "dump configuration and exit", 0},
    POPT_AUTOHELP
    {0, 0, 0, 0, 0, 0, 0}
  };

  poptContext poptCon = poptGetContext(0, app.argc(), (const char**) app.argv(), optionsTable, 0);
  if (poptGetNextOpt(poptCon) < -1) {
    poptPrintHelp(poptCon, stdout, 0);
    return -1;
  }

  // Read configuration
  // @todo: Get popt to accept string arguments....
  if (!gConfig.Init(QString::null)) {
    cerr << "FATAL ERROR! Could not get configuration!\n" << endl;
    return -1;
  }
  poptFreeContext(poptCon);

  if (port) {
    gConfig.SetInteger("SERVER_PORT", port);
  }

  if (daemon) {
    gConfig.SetBool("DAEMON_MODE", true);
  }

  if (dump) {
    gConfig.Dump();
    return -1;
  }

  if (gConfig.GetBool("DAEMON_MODE")) {
    int pid = fork();
    if (pid == -1) {
      cerr << "Hmmm, couldn't fork into the background?!" << std::endl;
      cerr << "  " << strerror(errno) << std::endl;
      exit (-1);
    }
    
    if (pid) {
      // cout << "Spawned daemon (pid " << pid << ")" << std::endl;
      exit(0);
    }
  }

  // If forked, daemon continues here
  setpgrp();
  // Change to other uid/gid
  if (gConfig.GetBool("DAEMON_CHANGE_USER")) {
    if (gConfig.GetString("DAEMON_GROUP").isEmpty() ||
        gConfig.GetString("DAEMON_USER").isEmpty()) {
      cerr << "You need to set DAEMON_GROUP and DAEMON_USER to make DAEMON_CHANGE_USER work"
           << endl;
      return -1;
    }
    
    struct group *gr = getgrnam(gConfig.GetString("DAEMON_GROUP").latin1());
    if (!gr || !setgid(gr->gr_gid)) {
      cerr << "Could not change to group='"
           << gConfig.GetString("DAEMON_GROUP") << "'!"
           << endl;
      return -1;
    }  
    struct passwd *pw = getpwnam(gConfig.GetString("DAEMON_USER").latin1());
    if (!pw || !setuid(pw->pw_uid)) {
      cerr << "Could not change to user='"
           << gConfig.GetString("DAEMON_USER") << "'!"
           << endl;
      return -1;
    }

    // No unwanted input/output, please
    freopen("/dev/null", "r", stdin);
    freopen("/dev/null", "w", stdout);
    freopen("/dev/null", "w", stderr);
  }

  // Install signal handler
  signal(SIGHUP, sig_handler);
  signal(SIGINT, sig_handler);
  signal(SIGTERM, sig_handler);

  return mainloop(&app);
}
