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

#include <qdom.h>
#include <qhttp.h>
#include <qregexp.h>
#include <qsocket.h>
#include <qurl.h>

using namespace std;

ServerError::ServerError(string aErr)
  : StdException(aErr, "ServerError") {
}

const char* HTTP_200       = "HTTP/1.0 200 Ok\r\n";
const char* HTTP_404       = "HTTP/1.0 404 Not Found\r\n";
const char* HTTP_501       = "HTTP/1.0 501 Not Implemented\r\n";
const char* HTTP_HTML      = "Content-Type: text/html; charset=\"utf-8\"\r\n";
const char* HTTP_XML       = "Content-Type: text/xml; charset=\"utf-8\"\r\n";
const char* HTTP_DATASTART = "\r\n";

const char* HTML_200_START =
  "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n"
  "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\" \"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">\n"
  "<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\">\n"
  "<head>\n"
  "  <title>NetDJ</title>\n"
  "</head>\n"
  "<body>\n";

const char* HTML_200_END =
  "</body>\n"
  "</html>\n";

const char* HTML_404 = 
  "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n"
  "<HTML>\n"
  "<HEAD>\n"
  "<TITLE>404 Not found</TITLE>\n"
  "</HEAD>\n"
  "<BODY BGCOLOR=\"#FFFFFF\">\n"
  "Sorry, the requested URI cannot be found.\n"
  "</BODY>\n"
  "</HTML>\n";

const char* HTML_501 = 
  "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n"
  "<HTML>\n"
  "<HEAD>\n"
  "<TITLE>501 Not Implemented</TITLE>\n"
  "</HEAD>\n"
  "<BODY BGCOLOR=\"#FFFFFF\">\n"
  "Sorry, the server only supports GET.\n"
  "</BODY>\n"
  "</HTML>\n";

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



// DEBUG DEBUG DEBUG
#include <iostream>



void
Server::ReadClient()
{
  // This slot is called when the client sent data to the server. The
  // server looks if it was a get request and sends a very simple HTML
  // document back.
  QSocket* socket = (QSocket*)sender();

  QString input;
  while (input.length() < MAX_REQUEST_SIZE && socket->canReadLine()) {
    input += socket->readLine();
  }

  cout << "Got request from client, size = " << input.length() << endl;
  
  QHttpRequestHeader header(input);
  cout << "Header: " << endl;
  cout << "\t Method:  " << header.method() << endl;
  cout << "\t Path:    " << header.path() << endl;
  cout << "\t Version: " << header.majorVersion() << "." << header.minorVersion() << endl;
  
  QTextStream os(socket);
  os.setEncoding(QTextStream::UnicodeUTF8);
  if (header.method() == "GET") {
    cout << "GET request: " << endl;
    QString path = header.path();
    QUrl url(QUrl("http://127.0.0.1/"), header.path());
    cout << "\t Path:    " << url.path() << endl;
    cout << "\t Query:   " << url.query() << endl;
    
    if (url.path() == "/" || url.path() == "/index.xml") {
      QDomDocument doc("NetDJ");
      QDomElement root = doc.createElement("currentsong");
      Song cursong;
      string curcol;
      // TODO: Listen for PlaySong signal!
      // mPlayer->GetCurrentSong(cursong, curcol);
      root.setAttribute("collection", curcol);
      cursong.asXML(doc, root);
      doc.appendChild(root);

      os << HTTP_200
         << HTTP_XML
         << HTTP_DATASTART
         << doc.toString()
         << endl;
    } else if (url.path() == "/cgi-bin/next") {
      os << HTTP_200
         << HTTP_HTML
         << HTTP_DATASTART
         << HTML_200_START
         << "<p>Skipping song</p>\n"
         << HTML_200_END;

      emit CmdSkip();
    } else if (url.path() == "/cgi-bin/quit") {
      os << HTTP_200
         << HTTP_HTML
         << HTTP_DATASTART
         << HTML_200_START
         << "<p>Shutting down NetDJ</p>\n"
         << HTML_200_END;
      emit CmdQuit();
    } else {
      os << HTTP_404
         << HTTP_HTML
         << HTTP_DATASTART
         << HTML_404;
    }
  } else {
    os << HTTP_501
       << HTTP_HTML
       << HTTP_DATASTART
       << HTML_501;
  }

  // Close connection
  socket->close();
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
