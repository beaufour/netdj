/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/**
 * \file Client.cpp
 * class Client
 *
 * $Id$
 *
 */

#include <qsocket.h>

#include "Client.h"

using namespace std;
using namespace NetDJ;

int Client::mIDCount = 0;

Client::Client(QSocket* aSocket)
  : mSocket(aSocket), mLogLevel(0)
{
}

Client::~Client()
{
  if (mSocket) {
    delete mSocket;
  }
}

QSocket*
Client::GetSocket()
{
  return mSocket;
}

int
Client::GetID() const
{
  return mID;
}

unsigned int
Client::GetLogLevel() const
{
  return mLogLevel;
}

void
Client::SetLogLevel(unsigned int aLevel)
{
  mLogLevel = aLevel;
}

