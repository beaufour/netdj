/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/**
 * \file main.cpp
 * DB Insert, a util to insert songs into the NetDJ song database
 *
 * $Id$
 *
 *  (c) 2004, Allan Beaufour Larsen <allan@beaufour.dk>
 *
 * @todo Enable user to specify database, username, and password.
 */

#include <qapplication.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qsqldatabase.h>
#include <qtextstream.h>

#include <tag.h>
#include <fileref.h>
#include <tstring.h>

/** The usage explanation for the program */
const char* gUsage = "./db_insert [-l] <filename>";

/** Standard output */
QTextStream out;

/** Standard error */
QTextStream err;

/**
 * Adds a single file to the database
 */
bool
add_file(const QString& aFilename)
{
  out << "Inserting file: " << aFilename
      << " ..."
      << endl;

  TagLib::FileRef tfile(aFilename);

  /*
  mAlbum = tfile.tag()->album().to8Bit();
  mTitle = tfile.tag()->title().to8Bit();
  mArtist = tfile.tag()->artist().to8Bit();
  mGenre = tfile.tag()->genre().to8Bit();
  mTrack = tfile.tag()->track();
  mYear = tfile.tag()->year();
  mLength = tfile.audioProperties()->length();
  */

  return true;
}

/**
 * Initialization and command line parsing.
 */
int
main(int argc, char* argv[])
{
  QApplication app(argc, argv, false);

  QFile ofile;
  ofile.open(IO_WriteOnly, stdout);
  out.setDevice(&ofile);

  QFile efile;
  efile.open(IO_WriteOnly, stderr);
  err.setDevice(&efile);

  QString filename;
  bool islist = false;
  switch (app.argc()) {
    case 2:
      filename = app.argv()[1];
      break;

    case 3:
      if (QString(app.argv()[1]) == "-l") {
	islist = true;
	filename = app.argv()[2];
	break;
      }

    default:
      err << gUsage << endl;
      return -1;
  }
  
  out << "Creating connection to database" << endl;
  QSqlDatabase* db = QSqlDatabase::addDatabase("QMYSQL3");
  if (!db) {
    err << "Could not create database connection" << endl;
    return -2;
  }
  db->setDatabaseName("netdj");

  if (!db->open()) {
    err << "Could not open database: " << db->lastError().driverText() << endl;
    return -3;
  }
  

  QFileInfo finfo(filename);  
  if (finfo.isDir()) {
    out << "Will include all files from directory" << endl;
  } else if (finfo.isFile()) {
    if (islist) {      
      out << "Will read filenames from list in file" << endl;
    } else {
      out << "Will insert single file" << endl;
      add_file(filename);
    }
  } else {
    err << gUsage << endl;
    return -4;
  }

  return 0;
}
