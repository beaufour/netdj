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
#include "Collections.h"
#include "Collection_Songlist_File.h"
#include "Collection_Songlist_Dir.h"

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
  /* Use verbose terminate handler, prints out name of exception,
     etc. */
  std::set_terminate (__gnu_cxx::__verbose_terminate_handler);

  QApplication app( argc, argv );

  /* Seed random number generator */
  srand(time(0));

  /* Install Qt message handler */
  qInstallMsgHandler( myMessageOutput );

  cout << "Initializing song collections" << endl;
  Collections cols;
  cols.AddCollection(new Collection_Songlist_Dir("request", "Requests", "/tmp/netdj_request/", true));
  cols.AddCollection(new Collection_Songlist_File("share", "Shares", "mp3.list"));

  cout << "Starting player" << endl;
  PlayerThread* playerthread = new PlayerThread(&cols);
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
