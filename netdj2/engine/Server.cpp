/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/**
 * \file Server.cpp
 * class Server
 *
 * $Id$
 *
 */

#include "Server.h"
#include "ServerSocket.h"
#include "Song.h"
#include "StdException.h"

#include <qsocket.h>
#include <qregexp.h>
#include <qdom.h>

using namespace std;

const char* HTTP_404 =
  "HTTP/1.0 404 Not Found\r\n"
  "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n"
  "<HTML>\n"
  "<HEAD>\n"
  "<TITLE>404 Not found</TITLE>\n"
  "</HEAD>\n"
  "<BODY BGCOLOR=\"#FFFFFF\">\n"
  "Sorry, the requested document cannot be found!\n"
  "</BODY>\n"
  "</HTML>\n";
const char* HTTP_200       = "HTTP/1.0 200 Ok\r\n";
const char* HTTP_HTML      = "Content-Type: text/html; charset=\"utf-8\"\r\n";
const char* HTTP_XML       = "Content-Type: text/xml; charset=\"utf-8\"\r\n";
const char* HTTP_DATASTART = "\r\n";

ServerError::ServerError(string aErr)
  : StdException(aErr) {
}

Server::Server(int aPort, int aBackLog, QObject* aParent)
  : QObject(aParent, "Server")
{
  // Automatically delete contained pointers on delete
  mClients.setAutoDelete(true);

  // Setup server socket
  mServerSocket = new ServerSocket(aPort, aBackLog, this, "ServerSocket");
  Q_CHECK_PTR(mServerSocket);
  
  if (!mServerSocket->ok()) {
    throw ServerError("Could not listen to port!");
  }
  connect(mServerSocket, SIGNAL(newClient(QSocket*)), this, SLOT(NewClient(QSocket*)));
}

void
Server::NewClient(QSocket* aSocket)
{
  qDebug("NewClient()");
  
  // Insert new client
  Client* client = new Client(aSocket);
  Q_CHECK_PTR(client);
  
  mClients.insert(aSocket, client);

  // Connect slots
  connect(aSocket, SIGNAL(readyRead()), this, SLOT(ReadClient()));
  connect(aSocket, SIGNAL(connectionClosed()), this, SLOT(ClientClosed()));
}

void
Server::ReadClient()
{
  // This slot is called when the client sent data to the server. The
  // server looks if it was a get request and sends a very simple HTML
  // document back.
  QSocket* socket = (QSocket*)sender();
  if (socket->canReadLine()) {
    QStringList tokens = QStringList::split(QRegExp("[ \r\n][ \r\n]*"),
					    socket->readLine());
    if (tokens[0] == "QUIT") {
      QTextStream os(socket);
      os.setEncoding(QTextStream::UnicodeUTF8);
      os << HTTP_200
	 << HTTP_HTML
	 << HTTP_DATASTART
	 << "<h1>Quitting</h1>\n";
      socket->close();
      emit CmdQuit();

    } else if (tokens[0] == "SKIP") {
      QTextStream os(socket);
      os.setEncoding(QTextStream::UnicodeUTF8);
      os << HTTP_200
	 << HTTP_HTML
	 << HTTP_DATASTART
	 << "<h1>Skipping</h1>\n";
      emit CmdSkip();

    } else if (tokens[0] == "GET") {
      QTextStream os(socket);
      os.setEncoding(QTextStream::UnicodeUTF8);
      os << HTTP_200
	 << HTTP_XML
	 << HTTP_DATASTART;
     
      QDomDocument doc("NetDJ");
      QDomElement root = doc.createElement("currentsong");
      Song cursong;
      string curcol;
      // TODO: Listen for PlaySong signal!
      // mPlayer->GetCurrentSong(cursong, curcol);
      root.setAttribute("collection", curcol);
      cursong.asXML(doc, root);
      doc.appendChild(root);
      os << doc.toString() << endl;
    }
  }
}

void
Server::ClientClosed()
{
  qDebug("client disconnected");

  QObject* sender = const_cast<QObject*>(QObject::sender());
  QSocket* socket = static_cast<QSocket*>(sender);

  // Disconnect
  socket->disconnect();

  // Remove from client list
  mClients.remove(socket);
}
