/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/**
 * \file AccessChecker.cpp
 * class SimpleAccessChecker
 *
 * $Id$
 *
 */

#include "AccessChecker.h"

#include "qfile.h"
#include "qtextstream.h"

#include <iostream>

using namespace std;

SimpleAccessChecker::SimpleAccessChecker(const QString& aFilename)
  : mFilename(aFilename)
{
}

SimpleAccessChecker::~SimpleAccessChecker()
{
}

bool
SimpleAccessChecker::Init()
{
  QFile file(mFilename);
  if (!file.open(IO_ReadOnly)) {
    return false;
  }
  
  QTextStream is(&file);
  int i = 1;
  QString line, name, password, level;
  while (!is.atEnd()) {
    line = is.readLine();
    if (line.isEmpty() || line[0] == '#') {
      continue;
    }
    name = line.section(':', 0, 0).stripWhiteSpace();
    password = line.section(':', 1, 1).stripWhiteSpace();
    level = line.section(':', 2).stripWhiteSpace();
    if (!name.isEmpty() && !password.isEmpty() && !level.isEmpty()) {
      mUsers[name].mPassword = password;
      mUsers[name].mLevel = level.toInt();
    } else {
      /** @todo Logger? */
      cerr << "Error parsing line " << i
	   << ": " << line
	   << endl;      
    }
    
    ++i;
  }
  file.close();

  return true;
}

bool
SimpleAccessChecker::HasAccess(const QString& aUsername,
			       const QString& aPassword,
			       const unsigned int aLevel)
{
    if (mUsers.contains(aUsername) &&
      mUsers[aUsername].mPassword == aPassword &&
      mUsers[aUsername].mLevel >= aLevel) {
    return true;
  }
  
  return false;      
}

