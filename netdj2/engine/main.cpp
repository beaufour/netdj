/*
 (c) 2004, Allan Beaufour Larsen <allan@beaufour.dk>

  main.cpp - Global initialization and startup.

*/

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cstdlib>

#include <qapplication.h>

#include "PlayerThread.h"
#include "HttpServer.h"

using namespace std;

void
myMessageOutput(QtMsgType type, const char *msg ) {
  switch (type) {
  case QtDebugMsg:
    fprintf(stderr, "Debug: %s\n", msg);
    break;
  case QtWarningMsg:
    fprintf(stderr, "Warning: %s\n", msg);
    break;
  case QtFatalMsg:
    fprintf(stderr, "Fatal: %s\n", msg);
    abort();                    // deliberately core dump
  }
}

int
main(int argc, char* argv[]) {
  QApplication app( argc, argv );
  /* Use verbose terminate handler, prints out name of exception,
     etc. */
  std::set_terminate (__gnu_cxx::__verbose_terminate_handler);

  /* Seed random number generator */
  srand(time(0));

  /* Install Qt message handler */
  qInstallMsgHandler( myMessageOutput );

  cout << "Starting player" << endl;
  PlayerThread* playerthread = new PlayerThread();
  playerthread->start(QThread::HighPriority);

  HttpServer* httpd = new HttpServer(7676, 5, playerthread);
  QObject::connect(httpd, SIGNAL(cmdQuit()), qApp, SLOT(quit()));
  app.exec();

  cout << "Stopping player" << endl;
  playerthread->Stop();

  /* Wait for thread to quit */
  playerthread->wait();
  cout << "main() finished!" << endl;
}
