/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/**
 * \file Server.cpp
 * class Server
 *
 * $Id$
 *
 */

#include <qdom.h>
#include <qregexp.h>
#include <qsocket.h>
#include <qurl.h>

#include "config.h"
#include "AccessChecker.h"
#include "Collections.h"
#include "ICollection.h"
#include "ISong.h"
#include "Server.h"
#include "ServerSocket.h"
#include "util.h"

using namespace std;
using namespace NetDJ;

ServerError::ServerError(string aErr)
  : StdException(aErr, "ServerError") {
}

const char* HTTP_200       = "HTTP/1.0 200 Ok\r\n";
const char* HTTP_401       = "HTTP/1.0 401 Unauthorized\r\n"
                             "WWW-Authenticate: Basic realm=\"NetDJ\"\r\n";
const char* HTTP_404       = "HTTP/1.0 404 Not Found\r\n";
const char* HTTP_500       = "HTTP/1.0 500 Internal Server Error\r\n";
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

const char* HTML_500 = 
  "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n"
  "<HTML>\n"
  "<HEAD>\n"
  "<TITLE>500 Internal Server Error</TITLE>\n"
  "</HEAD>\n"
  "<BODY BGCOLOR=\"#FFFFFF\">\n"
  "Sorry, something not quite good has happened in the server.\n"
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

Server::Server(Collections* aCols, int aPort, int aBackLog, QObject* aParent)
  : QObject(aParent, "Server"),
    mDocument("NetDJ"),
    mCols(aCols)
{
  // Automatically delete contained pointers on delete
  mClients.setAutoDelete(true);

  // Initialize Access Controll
  QString Filename = gConfig.GetString("CONFIG_DIR");
  Filename += Filename.isEmpty() ? "" : "/";
  Filename += gConfig.GetString("USER_LIST_FILE");

  mAccessChecker = new SimpleAccessChecker(Filename);
  Q_CHECK_PTR(mAccessChecker);  
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
  if (mSongElement) {
    delete mSongElement;
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
  connect(aSocket, SIGNAL(delayedCloseFinished()), this, SLOT(ClientClosed()));

  emit SigClientNew();
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
  QObject* sender = const_cast<QObject*>(QObject::sender());
  QSocket* socket = static_cast<QSocket*>(sender);

  QString input;
  while (input.length() < MAX_REQUEST_SIZE && socket->canReadLine()) {
    input += socket->readLine();
  }

  QHttpRequestHeader header(input);

  QTextStream os(socket);
  os.setEncoding(QTextStream::UnicodeUTF8);
  bool closeCon = true;
  if (header.method() == "GET") {
    try {
      closeCon = HandleCommand(os, header);
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

  if (closeCon) {
    socket->close();
  }
}

void
Server::ClientClosed()
{
  QObject* sender = const_cast<QObject*>(QObject::sender());
  QSocket* socket = static_cast<QSocket*>(sender);

  // Disconnect
  socket->disconnect();

  emit SigClientClose();

  // Remove from client list
  mClients.remove(socket);
}

void
Server::SongPlaying(const ISong* aSong, const ICollection* aCol)
{
  QMutexLocker lock(&mSongMutex);

  if (mSongElement) {
    delete mSongElement;
  }
  
  mSongElement = aSong->AsXML(&mDocument);
  mSongElement->setAttribute("collection", aCol->GetIdentifier());  
}

QDomElement*
Server::GetSong()
{
  QMutexLocker lock(&mSongMutex);

  QDomElement* newdoc = new QDomElement();
  Q_CHECK_PTR(newdoc);
  *newdoc = mSongElement->cloneNode(true).toElement();

  return newdoc;
}

void
Server::NewLogEntry(const QDomElement* aEntry, const unsigned int aLevel)
{
  ///
  /// @todo Implement log handling in Server
  QPtrDictIterator<Client> it(mClients);
  for (; it.current(); ++it) {
    if (it.current()->GetLogLevel() >= aLevel) {      
      QTextStream os(it.current()->GetSocket());
      os.setEncoding(QTextStream::UnicodeUTF8);
      os << *aEntry;
    }
  }
}


/********** COMMAND HANDLER ************/
/** Command types */
enum cmdtype_t {
  CMD_HELP,
  CMD_CURRENT,
  CMD_REQUESTS,
  CMD_LOG,
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
  {CMD_HELP,      "/help",          0, "This page"},
  {CMD_CURRENT,   "/current.xml",   0, "XML: The currently playing song"},
  {CMD_REQUESTS,  "/requests.xml",  0, "XML: The request list"},
  {CMD_LOG,       "/log",           0, "Get log messages (continuous)"},
  {CMD_SKIP,      "/skip",        200, "Skip the currently playing song"},
  {CMD_SHUTDOWN,  "/shutdown",    500, "Shutdown NetDJ"},
  {CMD_NULL,      (char*) 0,        0, (char*) 0}
};

bool
Server::HandleCommand(QTextStream& aStream, const QHttpRequestHeader& aHeader)
{
  // Analyze command
  const QUrl url(QUrl("http://127.0.0.1/"), aHeader.path());

  QString cmdstr = url.path();
  // Shortcuts for help for ease and backward compatibility
  if (cmdstr == "/" ||
      cmdstr == "/index.html" ||
      cmdstr == "/index.xml")
  {
    cmdstr = "/help";
  }
  
  int i;
  for (i = 0; gCommands[i].mName && gCommands[i].mName != cmdstr; ++i) {
  }
  command_t cmd = gCommands[i];
  if (cmd.mType == CMD_NULL) {
    emit SigMessage("Invalid command: " + cmdstr, 70, LogService::ENTRY_WARN);
    throw CMDInvalid();
  }

  // Check Auth
  QString uName;
  if (!CheckAuthorization(cmd.mAuthLevel, aHeader.value("Authorization"), uName)) {
    if (!uName.isEmpty()) {
      emit SigMessage("Unauthorized command: '" + cmdstr + "', user='" + uName + "'", 50, LogService::ENTRY_WARN);
    }
    
    throw CMDUnauthorized();
  }

  // If/when commands with structures are needed header needs to be unpacked.

  emit SigMessage("Command: '" + cmdstr + (uName.isEmpty() ? "" : "', user='" + uName + "'"), 150, LogService::ENTRY_INFO);
  bool closeCon = true;
  // Call command
  switch (cmd.mType) {
    case CMD_HELP:
      CmdHelp(aStream);
      break;

    case CMD_CURRENT:
      CmdCurrent(aStream);
      break;

    case CMD_REQUESTS:
      CmdRequests(aStream);
      break;

    case CMD_LOG:
      CmdLog(aStream);
      closeCon = false;
      break;

    case CMD_SHUTDOWN:
      CmdShutdown(aStream, uName);
      break;

    case CMD_SKIP:
      CmdSkip(aStream, uName);
      break;

    default:
      throw new ServerError("Shoot the programmer, HandleCommand() got an unknown command type!\n");
      break;
  }

  return closeCon;
}

bool
Server::CheckAuthorization(unsigned int aLevel, QString aAuthString, QString& aUserName)
{
  if (aLevel == 0) {
    return true;
  } else {
    if (aAuthString.isEmpty() || !aAuthString.startsWith("Basic ")) {
      return false;
    }
    QString auth = base64_decode(aAuthString.mid(6).ascii());

    aUserName = auth.section(':', 0, 0);
    QString pass = auth.section(':', 1, 1);
    
    return mAccessChecker->HasAccess(aUserName, pass, aLevel);
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
Server::CmdCurrent(QTextStream& aStream)
{
  QDomElement* songXML = GetSong();

  aStream << HTTP_200
          << HTTP_XML
          << HTTP_DATASTART
          << *songXML
          << endl;

  delete songXML;
}

void
Server::CmdRequests(QTextStream& aStream)
{
  ICollection* request = mCols->GetCollection("request");
  if (request) {
    QDomElement* xml = request->AsXML(&mDocument);

    aStream << HTTP_200
            << HTTP_XML
            << HTTP_DATASTART
            << *xml
            << endl;
    
    delete xml;
  } else {
    emit SigMessage("Could not retrieve request collection!", 10, LogService::ENTRY_CRIT);

    aStream << HTTP_500
            << HTTP_HTML
            << HTTP_DATASTART
            << HTML_500;
  }
}

void
Server::CmdLog(QTextStream& aStream)
{
  QObject* sender = const_cast<QObject*>(QObject::sender());
  QSocket* socket = static_cast<QSocket*>(sender);

  mClients[socket]->SetLogLevel(999);

  aStream << HTTP_200
          << HTTP_XML
          << HTTP_DATASTART
          << "<log state=\"on\"/>\n"
          << endl;
}

void
Server::CmdSkip(QTextStream& aStream, const QString& aUsername)
{
  aStream << HTTP_200
          << HTTP_HTML
          << HTTP_DATASTART
          << HTML_200_START
          << "<p>Skipping song</p>\n"
          << HTML_200_END;
  
  emit SigSkip(aUsername);
}

void
Server::CmdShutdown(QTextStream& aStream, const QString& aUsername)
{
  aStream << HTTP_200
          << HTTP_HTML
          << HTTP_DATASTART
          << HTML_200_START
          << "<p>Shutting down NetDJ!</p>\n"
          << HTML_200_END;
  
  emit SigQuit(aUsername);
}

