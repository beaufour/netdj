/**
 * \file HttpServer.cpp
 * class HttpServer
 *
 * $Id$
 *
 */

#include "HttpServer.h"

#include <qsocket.h>
#include <qregexp.h>
#include <qdom.h>

#include "Song.h"

using namespace std;

HttpError::HttpError(string err)
  : StdException(err) {
}

HttpServer::HttpServer(Q_UINT16 port, int backlog,
		       PlayerThread* pl, QObject* parent)
  : QServerSocket(port, backlog, parent), Player(pl) {
  if (!ok()) {
    throw HttpError("Could not listen to port!");
  }
}

void
HttpServer::newConnection(int socket) {
  // When a new client connects, the server constructs a QSocket and all
  // communication with the client is done over this QSocket. QSocket
  // works asynchronouslyl, this means that all the communication is done
  // in the two slots readClient() and discardClient().
  QSocket* s = new QSocket( this );
  connect( s, SIGNAL(readyRead()), this, SLOT(readClient()) );
  connect( s, SIGNAL(delayedCloseFinished()), this, SLOT(discardClient()) );
  s->setSocket( socket );
}

void
HttpServer::readClient() {
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
      os << "HTTP/1.0 200 Ok\r\n"
	"Content-Type: text/html; charset=\"utf-8\"\r\n"
	"\r\n"
	"<h1>Quitting</h1>\n";
      socket->close();
      emit cmdQuit();
    } else if (tokens[0] == "SKIP") {
      QTextStream os(socket);
      os.setEncoding(QTextStream::UnicodeUTF8);
      os << "HTTP/1.0 200 Ok\r\n"
	"Content-Type: text/html; charset=\"utf-8\"\r\n"
	"\r\n"
	"<h1>Skipping</h1>\n";
      Player->Skip();
    } else if (tokens[0] == "GET") {
      QTextStream os(socket);
      os.setEncoding(QTextStream::UnicodeUTF8);
      os << "HTTP/1.0 200 Ok\r\n"
	"Content-Type: text/xml; charset=\"utf-8\"\r\n"
	"\r\n";
     
      QDomDocument doc("NetDJ");
      QDomElement root = doc.createElement("currentsong");
      Song cursong;
      string curcol;
      Player->GetCurrentSong(cursong, curcol);
      root.setAttribute("collection", curcol);
      cursong.asXML(doc, root);
      doc.appendChild(root);
      os << doc.toString() << endl;

    }
  }
}

void
HttpServer::discardClient() {
  QSocket* socket = (QSocket*)sender();
  delete socket;
}
