/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/**
 * \file Server.cpp
 * class Server
 *
 * $Id$
 *
 */

#include <qdom.h>
#include <qhttp.h>
#include <qregexp.h>
#include <qsocket.h>
#include <qurl.h>

#include "config.h"
#include "AccessChecker.h"
#include "Collection.h"
#include "Server.h"
#include "ServerSocket.h"
#include "Song.h"
#include "StdException.h"
#include "util.h"

using namespace std;

ServerError::ServerError(string aErr)
  : StdException(aErr, "ServerError") {
}

const char* HTTP_200       = "HTTP/1.0 200 Ok\r\n";
const char* HTTP_401       = "HTTP/1.0 401 Unauthorized\r\n"
                             "WWW-Authenticate: Basic realm=\"NetDJ\"\r\n";
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

const char* HTML_401 = 
  "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n"
  "<HTML>\n"
  "<HEAD>\n"
  "<TITLE>404 Unathorized</TITLE>\n"
  "</HEAD>\n"
  "<BODY BGCOLOR=\"#FFFFFF\">\n"
  "You need proper authentication to access this!\n"
  "</BODY>\n"
  "</HTML>\n";

const char* HTML_404 = 
  "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n"
  "<HTML>\n"
  "<HEAD>\n"
  "<TITLE>404 Not found</TITLE>\n"
  "</HEAD>\n"
  "<BODY BGCOLOR=\"#FFFFFF\">\n"
  "The requested command is invalid.\n"
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

  // Initialize Access Controll
  QString Filename = NETDJ_CONF.GetString("CONFIG_DIR");
  Filename += Filename.isEmpty() ? "" : "/";
  Filename += NETDJ_CONF.GetString("USER_LIST_FILE");
  mAccessChecker = new SimpleAccessChecker(Filename);
  if (!mAccessChecker->Init()) {
    throw ServerError("Could not initialize Access Checker");
  }

  // Setup server socket
  mServerSocket = new ServerSocket(aPort, aBackLog, this, "ServerSocket");
  Q_CHECK_PTR(mServerSocket);
  
  if (!mServerSocket->ok()) {
    throw ServerError("Could not listen to port!");
  }
  connect(mServerSocket, SIGNAL(newClient(QSocket*)), this, SLOT(NewClient(QSocket*)));
}

Server::~Server()
{
  if (mServerSocket) {
    delete mServerSocket;
  }
  if (mAccessChecker) {
    delete mAccessChecker;
  }
}

void
Server::NewClient(QSocket* aSocket)
{
  // Insert new client
  Client* client = new Client(aSocket);
  Q_CHECK_PTR(client);
  
  mClients.insert(aSocket, client);

  // Connect slots
  connect(aSocket, SIGNAL(readyRead()), this, SLOT(ReadClient()));
  connect(aSocket, SIGNAL(connectionClosed()), this, SLOT(ClientClosed()));
}

/**
 * Exception thrown by HandleCommand() when it encounters an invalid command.
 */
class CMDInvalid 
{
public:
  CMDInvalid() {};
};

/**
 * Exception thrown by HandleCommand() when a user is not authorized to
 * perform a commands.
 */
class CMDUnauthorized
{
public:
  CMDUnauthorized() {};
};

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

  QHttpRequestHeader header(input);

  QTextStream os(socket);
  os.setEncoding(QTextStream::UnicodeUTF8);
  if (header.method() == "GET") {
    try {
      HandleCommand(os, header);
    }
    catch (CMDUnauthorized& e) {
      os << HTTP_401
         << HTTP_HTML
         << HTTP_DATASTART
         << HTML_401
         << endl;
    }
    catch (CMDInvalid& e) {
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

void
Server::SongPlaying(const Song& aSong, const Collection* aCol)
{
  QMutexLocker lock(&mSongMutex);

  mCurrentSong = aSong;
  mCurrentCol = aCol;
}

void
Server::GetSong(Song& aSong, const Collection** aCol)
{
  QMutexLocker lock(&mSongMutex);

  aSong = mCurrentSong;
  *aCol = mCurrentCol;
}

/********** COMMAND HANDLER ************/
/** Command types */
enum cmdtype_t {
  CMD_HELP,
  CMD_INDEX,
  CMD_SKIP,
  CMD_SHUTDOWN,
  CMD_NULL
};


/** Contains information about a command */
typedef struct 
{
  const cmdtype_t mType;
  const char* mName;
  const unsigned int mAuthLevel;
  const char* mDescription;
} command_t;

/** List of available commands */
const command_t gCommands[] = {
  {CMD_HELP,     "/help",       0, "This page"},
  {CMD_INDEX,    "/index.xml",  0, "XML: The current song playing and the requested songs pending"},
  {CMD_SKIP,     "/skip",     200, "Skip the currently playing song"},
  {CMD_SHUTDOWN, "/shutdown", 500, "Shutdown NetDJ"},
  {CMD_NULL,     (char*) 0,     0, (char*) 0}
};

void
Server::HandleCommand(QTextStream& aStream, const QHttpRequestHeader& aHeader)
{
  // Analyze command
  const QUrl url(QUrl("http://127.0.0.1/"), aHeader.path());

  QString cmdstr = url.path();
  // Shortcuts for help for ease and backward compatibility
  if (cmdstr == "/") {
    cmdstr = "/help";
  }
  if (cmdstr == "/index.html") {
    cmdstr = "/help";
  }
  
  int i;
  for (i = 0; gCommands[i].mName && gCommands[i].mName != cmdstr; ++i) {
  }
  command_t cmd = gCommands[i];
  if (cmd.mType == CMD_NULL) {
    throw CMDInvalid();
  }

  // Check Auth
  if (!CheckAuthorization(cmd.mAuthLevel, aHeader.value("Authorization"))) {
    throw CMDUnauthorized();
  }

  // If/when commands with structures are needed header needs to be unpacked.

  // Call command
  switch (cmd.mType) {
    case CMD_HELP:
      CmdHelp(aStream);
      break;

    case CMD_INDEX:
      CmdIndex(aStream);
      break;

    case CMD_SHUTDOWN:
      CmdShutdown(aStream);
      break;

    case CMD_SKIP:
      CmdSkip(aStream);
      break;

    default:
      throw new ServerError("Shoot the programmer, HandleCommand() gets an unknown command type!\n");
      break;
  }
}

bool
Server::CheckAuthorization(unsigned int aLevel, QString aAuthString)
{
  if (aLevel == 0) {
    return true;
  } else {
    if (aAuthString.isEmpty() || !aAuthString.startsWith("Basic ")) {
      return false;
    }
    QString auth = base64_decode(aAuthString.mid(6).ascii());

    QString user = auth.section(':', 0, 0);
    QString pass = auth.section(':', 1, 1);
    
    return mAccessChecker->HasAccess(user, pass, aLevel);
  }
}

/************* COMMANDS **************/
void
Server::CmdHelp(QTextStream& aStream)
{
  aStream << HTTP_200
          << HTTP_HTML
          << HTTP_DATASTART
          << HTML_200_START
          << "<table>\n"
          << "  <tr><th>Name</th><th>Description</th></tr>\n";

  for (int i = 0; gCommands[i].mName; ++i) {
    aStream << "  <tr><td><a href=\""
            << gCommands[i].mName
            << "\">"
            << gCommands[i].mName
            << "</a></td><td>"
            << gCommands[i].mDescription
            << "</td></tr>\n";
  }

  aStream << "</table>\n"
          << HTML_200_END;
  
}

void
Server::CmdIndex(QTextStream& aStream)
{
  Song cursong;
  const Collection* curcol;
  GetSong(cursong, &curcol);
  
  QDomDocument doc("NetDJ");
  QDomElement root = doc.createElement("currentsong");
  root.setAttribute("collection", mCurrentCol->GetIdentifier());
  mCurrentSong.asXML(doc, root);
  doc.appendChild(root);
  
  aStream << HTTP_200
          << HTTP_XML
          << HTTP_DATASTART
          << doc.toString()
          << endl;
}

void
Server::CmdSkip(QTextStream& aStream)
{
  aStream << HTTP_200
          << HTTP_HTML
          << HTTP_DATASTART
          << HTML_200_START
          << "<p>Skipping song</p>\n"
          << HTML_200_END;
  
  emit SigSkip();
}

void
Server::CmdShutdown(QTextStream& aStream)
{
  aStream << HTTP_200
          << HTTP_HTML
          << HTTP_DATASTART
          << HTML_200_START
          << "<p>Shutting down NetDJ!</p>\n"
          << HTML_200_END;
  
  emit SigQuit();
}

