/*
 *  NetDJ
 * 
 *  Allan Beaufour Larsen <allan@beaufour.dk>
 *
 *  $Id$
 */

// Own headers
#include "config.h"
#include "util.h"
#include "Lockable.h"
#include "Configuration.h"
#include "AccessConf.h"
#include "Directory.h"
#include "File.h"
#include "ID3Tag.h"
#include "HTTP.h"

// HACK to correct c++ "bug" in shout.h
#ifdef HAVE_LIBSHOUT
#  define namespace nspace
#  include <shout/shout.h>
#endif

// Provides cout and manipulation of same
#include <iostream>
#include <iomanip>

// Provides file streams
#include <fstream>

// Provides stdout and stderr
#include <cstdio>

// Provides time_t time()
#include <ctime>

// Provides poll()
#include <sys/poll.h>

// GNU Readline
#ifdef USE_READLINE
extern "C" {
# include <readline/readline.h>
#  ifdef HAVE_READLINE_HISTORY
#    include <readline/history.h>
#  endif
}
#endif

// Provides strcmp and strerror
#include <cstring>

// Provides errno
#include <errno.h>

// POSIX threads
#include <pthread.h>
#include <signal.h>

// Provides wait, fork
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#undef HAVE_ID3LIB

void
screen_flush() {
  // Seems like there are problems with this and old readline libraries...
#ifdef USE_READLINE_XX
  rl_forced_update_display();
#else
  cout << endl << "NetDJ> " << flush;
#endif
}

Directory request("request", "Requested songs");
Directory cache  ("cache",   "Cache-dir");
Directory share  ("share",   "Share-dir", false);
const unsigned int listnum = 3;
Directory *lists[] = {&request, &cache, &share};

Configuration config;
AccessConf acc(&config);
bool http_locked;

void* http_thread(void*);
pthread_t http_t;


////////////////////////////////////////
// SAVE/LOAD state

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// Holds the full path to the statefile
string statefile;

void
savestate() {
  // Errors are not considered fatal here, because we're exiting.
  int fd;
  time_t upd = cache.GetNextTimestamp();

  cout << "Saving state to '" << statefile << "'." << endl;

  fd = open(statefile.c_str(), O_WRONLY | O_CREAT | O_TRUNC,  S_IWUSR | S_IRUSR);
  if (fd == -1) {
    error(strerror(errno), false);
  } else if (write(fd, (void*) &upd, sizeof(time_t)) == -1) {
    error(strerror(errno), false);
  } else if (close(fd) == -1) {
    error(strerror(errno), false);
  }
}

void
loadstate() {
  // Construct full path to statefile: $(HOME)/$(CONF_STATEFILENAME)
  char* home = getenv("HOME");
  if (home) {
    statefile = home;
    if (statefile[statefile.size() - 1] != '/') {
      statefile += '/';
    }
  }
  statefile += CONF_DIRNAME;
  statefile += CONF_STATEFILENAME;

  cout << "Reading state from '" << statefile << "'." << endl;

  int fd;
  fd = open(statefile.c_str(), O_RDONLY);
  if (fd != -1) {
    time_t upd;
    if (read(fd, (void*) &upd, sizeof(time_t)) == -1) {
      error(strerror(errno));
    }
    // If we don't subtract one,
    // the oldest file won't be included in next update
    cache.SetLastupdate(upd - 1);
    if (close(fd) == -1) {
      error(strerror(errno));
    }
  }
}

////////////////////////////////////////
// PLAYER

class TCurrentSong : public Lockable {
private:
  bool   currdelete;
  File currfile;
  File prevfile;
  bool   prevdelete;

  void InternalSet(const File& fil, const bool del) {
    lock();
    if (config.GetBool("DELETE_PLAYED") && prevdelete) {
      if (!prevfile.Delete()) {
	cout << "Couldn't delete '" << prevfile.GetName() << "'!" << endl;
	screen_flush();
      }
    }
    prevfile = currfile;
    prevdelete = currdelete;
    
    currfile = fil;
    currdelete = del;
    unlock();
  }


public:
  TCurrentSong() : Lockable() {};

  void Get(File& fil) {
    lock();
    fil = currfile;
    unlock();
  }
  
  void Set(const File& fil, const bool del) {
    InternalSet(fil, del);
  }

  void Clear() {
    InternalSet(File(), false);
  }

  bool RenameCurrent(const string& newpath, const bool del) {
    bool res = false;
    lock();
    if (currfile.Rename(newpath)) {
      res = true;
      currdelete = del;
    };
    unlock();
  return res;
  };
};

TCurrentSong currentsong;
int playercmd_pid = 0;
bool stop_player  = false;
bool next_song = false;
pthread_t player_t;

void*
player_thread(void*) {
  unsigned int i;
  File fobj;

  stop_player = false;
  next_song = false;

  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

  // Init shoutcast stuff
#ifdef HAVE_LIBSHOUT
  shout_conn_t conn;
  unsigned char buff[4096];
  long read;
  FILE *musicfile;

  shout_init_connection(&conn);
  
  // Setup stream
  char stream_ip[16];
  char stream_mount[256];
  char stream_passwd[256];

  strncpy(stream_ip, config.GetString("STREAM_IP").c_str(), sizeof(stream_ip));
  strncpy(stream_mount, config.GetString("STREAM_MOUNT").c_str(), sizeof(stream_mount));
  strncpy(stream_passwd, config.GetString("STREAM_PASSWD").c_str(), sizeof(stream_passwd));

  conn.ip = stream_ip;
  conn.port = config.GetInteger("STREAM_PORT");
  conn.mount = stream_mount;
  conn.password = stream_passwd;
  conn.name = "NetDJ";
  conn.genre = "Mixed";
  conn.description = "NetDJ streaming channel";

  if (config.GetBool("STREAM") && !shout_connect(&conn)) {
    cout << endl << "  Couldn't connect to server!" << endl;
#else
  // Sanitycheck, when libshout isn't in binary
  if (config.GetBool("STREAM")) {
    cout << endl
	 << "  Shoutcast-support isn't included in this binary!" << endl
	 << "  Set STREAM=false in configuration to use player instead" << endl;
#endif
    screen_flush();
    stop_player = true;
  }
  
  
  // Open logfile
  ofstream logfile;
  if (config.GetBool("PLAYER_LOG")) {
    string logfilename = config.GetString("$$CONFDIR") + CONF_LOGFILENAME;
    cout << endl << "  Logging to '" << logfilename << "'" << endl;
    logfile.open(logfilename.c_str(), ios::app);
    if (!logfile.is_open()) {
      cout << endl << "  Sorry, couldn't open logfile!" << endl;
      config.SetBool("PLAYER_LOG", false);
    }
    screen_flush();
  }
  
  while (!stop_player) {
    // Get next song
    next_song = false;
    for (i = 0; i < listnum; ++i) {
      if (lists[i]->GetSong(fobj))
	break;
    }
    if (i == listnum) {
      cout << endl << "  Hmmm, no files to play..." << endl;
      currentsong.Clear();
      screen_flush();
      sleep(30);
    } else {
      if (fobj.Exists()) {
	currentsong.Set(fobj, i == 1 ? true : false);
	cout << endl << "  Playing '" << fobj.GetFilename()
	     << "' [" << lists[i]->GetShortname() << "]" << endl;
	screen_flush();
	
	if (!config.GetBool("STREAM")) {
	  // Play song with player program
	  playercmd_pid = fork();
	  if (playercmd_pid == -1) {
	    error(strerror(errno));
	  }
	  
	  if (playercmd_pid == 0) {
	    // Skip all output from player
	    freopen("/dev/null", "w", stdout);
	    freopen("/dev/null", "w", stderr);
	    // Start player
	    execlp(config.GetString("PLAYER").c_str(),
		   "mpg123",
		   "-q",
		   "--aggressive",
		   fobj.GetName().c_str(),
		   NULL);
	  } else {
	    wait(NULL);
	  }
#ifdef HAVE_LIBSHOUT
	} else {
	  musicfile = fopen(fobj.GetName().c_str(), "r");
	  shout_update_metadata(&conn, fobj.GetFilename().c_str());
	  while (!stop_player && !next_song) {
	    read = fread(buff, 1, 4096, musicfile);
	    if (read > 0) {
	      if (!shout_send_data(&conn, buff, read)) {
		cout << endl << "  Send error:" << conn.error << "..." << endl;
		screen_flush();
		break;
	      }
	    } else {
	      break;
	    }
	    shout_sleep(&conn);
	  }
	  fclose(musicfile);
#endif
	}
	if (logfile.is_open()) {
	  logfile << "\"" << fobj.GetFilenameNoType() << "\","
		  << ((stop_player || next_song) ? 1 : 0) << ",\""
		  << lists[i]->GetShortname() << "\""
		  << endl;
	}
      }
    }
  }

#ifdef HAVE_LIBSHOUT
  // Disconnect from streaming server
  if (config.GetBool("STREAM")) {
    shout_update_metadata(&conn, NULL);
    shout_disconnect(&conn);
  }
#endif

  // Close logfile
  if (logfile.is_open()) {
    logfile.close();
  }

  // Clear currentsong and end thread
  currentsong.Clear();
  pthread_exit(NULL);
}


void kill_current() {
  // This should be packed nicely into a semaphore...
  next_song = true;
  if (!config.GetBool("STREAM") && playercmd_pid) {
    kill(playercmd_pid, SIGINT);
  }
}


////////////////////////////////////////
// COMMANDS
typedef void (*COMFUNC)(char*);

int com_add(char*);
int com_help(char*);
int com_info(char*);
int com_list(char*);
int com_lists(char*);
int com_move(char*);
int com_next(char*);
int com_start(char*);
int com_stop(char*);
int com_version(char*);
int com_weblock(char*);
int com_webstart(char*);

int
com_yeah(char* arg) {
  cout << "  Coming soon (tm)" << endl;
  return 0;
}

#ifndef HAVE_ICPFUNC
typedef int rl_icpfunc_t (char *);
#endif

typedef struct {
  char* name;
  rl_icpfunc_t* func;
  bool userfunc;
  char* doc;
} COMMAND;

COMMAND commands[] = {
  {"add",      com_add,      false, "Add song to list"},
  {"help",     com_help,     false, "This listing"},
  {"info",     com_info,     false, "Get information about the current song"},
  {"list",     com_list,     false, "Show next 10 entries in LIST"},
  {"lists",    com_lists,    false, "Show status for lists"},
  {"move",     com_move,     false, "Move current song to share"},
  {"next",     com_next,     true,  "Skip to next song"},
  {"otw",      com_yeah,     false, "(!) Show song(s) currently being downloaded"},
  {"save",     com_yeah,     false, "(!) Save current song to share"},
  {"start",    com_start,    true,  "Start player"},
  {"stop",     com_stop,     true,  "Stop player"},
  {"version",  com_version,  false, "Show program version"},
  {"weblock",  com_weblock,  false, "Lock webacces"},
  {"webstart", com_webstart, false, "Start web_thread"},
  { (char*) NULL, (rl_icpfunc_t*) NULL, false, (char*) NULL}
};

bool check_web(char*& arg) {
  if (arg && *arg && arg[0] == '}') {
    arg++;
    return true;
  } else {
    return false;
  }
}

int
com_add(char* arg) {
  string tmp;
  check_web(arg);
  if (arg && *arg) {
    if (arg[0] != '/') {
      tmp = getenv("PWD");
      if (tmp[tmp.size() - 1] != '/') {
	tmp += '/';
      }
      tmp += arg;
    } else {
      tmp = arg;
    }
    cout << "  Adding '" << tmp << "' to request-list" << endl;
    request.AddSong(tmp);
  } else {
    cout << "  Usage: add <filename>" << endl;
  };
  return 0;
}

int
com_help(char* arg) {
  for (int i = 0; commands[i].name; ++i)
    cout << "  " << setw(10) << commands[i].name
	 << "   " << commands[i].doc << endl;
  cout << endl;
  cout << "  " << setw(10) << "quit"
       << "   " << "Quit program" << endl;
  return 0;
};

int
com_info(char* arg) {
  ID3Tag const *id3tag = NULL;
  time_t filetime;
  File fobj;

  currentsong.Get(fobj);
  filetime = fobj.GetMtime();
  cout << "  " << fobj.GetName() << endl;
  cout << "    " << setprecision(2)
       << ((float) fobj.GetSize() / (1024 * 1024)) << " MB - "
       << ctime(&filetime) << endl;
  if (fobj.GetID3Info(id3tag)) {
    cout << "    Artist:  " << id3tag->GetArtist() << endl;
    cout << "    Album:   " << id3tag->GetAlbum() << endl;
    cout << "    Title:   " << id3tag->GetTitle() << endl;
    cout << "    Year:    " << id3tag->GetYear() << endl;
    cout << "    Style:   " << id3tag->GetStyle() << endl;
    cout << "    Comment: " << id3tag->GetNote() << endl;
  }
  return 0;
};

int
com_list(char* arg) {
  check_web(arg);
  unsigned int l = arg ? atoi(arg) : 0;
  vector<File> songs;

  if (l >= 0 && l < listnum) {
    cout << "  [" << lists[l]->GetShortname() << "]:" << endl;
    lists[l]->GetEntries(songs, 10);
    for (vector<File>::iterator it = songs.begin();
	 it != songs.end();
	 ++it) {
      cout << "    " << it->GetFilename() << endl;
    }
  }
  return 0;
}

int
com_lists(char* arg) {
  Directory* dir;
  for (unsigned int i = 0; i < listnum; ++i) {
    dir = lists[i];
    cout << setw(7) << dir->GetSize()
	 << "  " << dir->GetDescription()
	 << " [" << dir->GetShortname() << "]"
	 << endl;
  }
  return 0;
}

int
com_move(char* arg) {
  string songname;
  string newpath;
  File songfile;
  currentsong.Get(songfile);

  if (songfile.Exists()) {
    newpath = share.GetDirname();
    newpath += songfile.GetFilename();
    
    cout << "  Moving '" << songfile.GetName() << "' to '"
	 << newpath << "'" << endl;
    if (currentsong.RenameCurrent(newpath, false)) {
      cout << "  Error! " << endl;
    }
  } else {
    cout << "  Error: File doesn't exist..." << endl;
  }
  return 0;
}

int
com_next(char* arg) {
  cout << "  Skipping current song..." << endl;
  kill_current();
  usleep(500);
  return 0;
};

int
com_start(char* arg) {
  stop_player = false;
  pthread_create(&player_t, NULL, &player_thread, NULL);
  return 0;
};

int
com_stop(char* arg) {
  stop_player = true;
  kill_current();
  return 0;
};

int
com_version(char* arg) {
  cout << "  " << PKGVER << endl
       << "  CVS (main.cc):"
       << "$Id$" << endl;

  cout << "Shoucast-support: " <<
#ifdef HAVE_LIBSHOUT
    "Yes"
#else
    "No"
#endif
       << endl;

  return 0;
}

int
com_weblock(char* arg) {
  http_locked = !http_locked;
  cout << "  WebAccess " << (http_locked ? "OFF" : "ON") << endl;
  return 0;
}

int
com_webstart(char* arg) {
  pthread_create(&http_t, NULL, &http_thread, NULL);
  return 0;
}


COMMAND*
find_command(const char* name, bool admin) {
  for (int i = 0; commands[i].name; ++i)
    if (strcmp(name, commands[i].name) == 0 && (commands[i].userfunc || admin))
      return (&commands[i]);
  return ((COMMAND*) NULL);
}

void
execute_line(char* line) {
  char* arg = line;

  // Seperate command and arguments
  while (*arg != '\0' && *arg != ' ')
    ++arg;
  if ((*arg) != '\0') {
    *arg = '\0';
    ++arg;
  }
  // Strip leading and trailing spaces in argumentstring
  while (*arg && *arg == ' ') {
    ++arg;
  }
  char* argend = arg;
  while (*argend) {
    ++argend;
  }
  // OK, here comes the quick and dirty solution (it's late!)
  // Problem puts a \0 for every space - should only put one.
  --argend;
  while (argend > arg && *argend == ' ') {
    *argend = '\0';
    --argend;
  }

  // Find command and execute it
  COMMAND* com = find_command(line, true);
  if (!com) {
    cout << "  Hmmm, did you really mean '" << line << "'?" << endl;
  } else {
    ((*(com->func)) (arg));
  }
}


////////////////////////////////////////
// HTTP
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int http_sock = -1;

void
http_cleanup(void*) {
  if (http_sock != -1) {
    close(http_sock);
  }
  cout << endl << "  HTTPThread: Exiting" << endl;
  screen_flush();
}

void*
http_thread(void*) {
  struct sockaddr_in sin;
  const bool tval = true;

  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
  pthread_cleanup_push(&http_cleanup, NULL);

  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons(config.GetInteger("HTTP_PORT"));
  sin.sin_family = AF_INET;

  http_sock = socket(PF_INET, SOCK_STREAM, 0);
  setsockopt(http_sock, SOL_SOCKET, SO_REUSEADDR, (void*) &tval, sizeof(tval));

  if (http_sock == -1) {
    cout << endl << "  HTTPThread: " << strerror(errno) << endl;
    screen_flush();
  } else if (bind(http_sock, (struct sockaddr *) &sin, sizeof(struct sockaddr_in)) == -1) {
    cout << endl << "  HTTPThread: " << strerror(errno) << endl;
    screen_flush();
  } else if (listen(http_sock, 5) == -1) {
    cout << endl << "  HTTPThread: " << strerror(errno) << endl;
    screen_flush();
  } else {
    int newsock;
    struct sockaddr_in newsin;
    socklen_t len;
    char rbuf[4096];

    // HTML-answer
    const char hbuf1[] =
      "HTTP/1.1 200 OK\r\n"
      "Server: NetDJ\r\n"
      "Connection: close\r\n"
      "Content-Type: text/html; charset=iso-8859-1\r\n"
      "\r\n"
      "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n"
      "<HTML>\n"
      "  <HEAD>\n"
      "    <META HTTP-EQUIV=\"Pragma\" CONTENT=\"no-cache\">\n"
      "    <META HTTP-EQUIV=\"Refresh\" CONTENT=\"30;URL=/\">\n"
      "    <STYLE TYPE=\"text/css\">\n"
      "      body.dummy {}\n"
      "      body {\n"
      "        font-size: 10pt;\n"
      "      }\n"
      "      table {\n"
      "        font-size: 10pt;\n"
      "      }\n"
      "      form {\n"
      "        font-size: 10pt;\n"
      "      }\n"
      "    </STYLE>\n"
      "    <TITLE>NetDJ</TITLE>\n"
      "  </HEAD>\n"
      "  <BODY>\n"
      "    <SCRIPT LANGUAGE=\"JavaScript\" TYPE=\"text/javascript\">\n"
      "      function addNetscapePanel() {\n"
      "        if ((typeof window.sidebar == \"object\")\n"
      "            && (typeof window.sidebar.addPanel == \"function\"))\n"
      "          {\n" 
      "            window.sidebar.addPanel (\"NetDJ\", window.location,\"\");\n"
      "          }\n"
      "      }\n"
      "    </SCRIPT>\n"
      "    <A HREF=\"http://netdj.sourceforge.net\">NetDJ</A> is currently playing:\n"
      "    <P>\n"
      "    <FORM TARGET=\"_CONTENT\" ACTION=\"http://www.google.com/search\">\n"
      "      <INPUT maxLength=256 size=55 name=q value=\"";
    const char hbuf2[] =
      "\">\n"
      "      <INPUT TYPE=SUBMIT VALUE=\"Google Search\" NAME=btnG>\n"
      "    </FORM>\n"
      "    <P>\n"
      "    [<A HREF=\"/\">Refresh</A>]\n"
      "    - [<A HREF=\"/cgi-bin/next?id=";

    const char hbuf3[] =
      "\">Skip song</A>]\n"
      "    - [<A HREF=\"/cgi-bin/stop\">Stop player</A>]\n"
      "    - [<A HREF=\"/cgi-bin/start\">Start player</A>]\n"
      "    <P>\n"
      "    <STRONG>Next songs in cache:</STRONG><BR>\n";

    const char hbuf4[] =
      "    <P>\n"
      "    <STRONG>Playlists:</STRONG><BR>\n"
      "    <TABLE BORDER=1>\n"
      "      <TR><TD><B>Size</B></TD><TD><B>Name</B></TD><TD><B>Description</B></TD></TR>\n";

    const char hbuf5[] =
      "    </TABLE>\n"
      "    <P>\n"
      "    Add to <A HREF=\"javascript:addNetscapePanel();\">Netscape sidebar</A>"
      " - <A HREF=\"/\" REL=\"sidebar\" TITLE=\"NetDJ\">Opera panel</A>\n"
      "  </BODY>\n"
      "</HTML>\n";

    string hbuf;

    // text/xml packet
    HTTPResponse HTTPxml(200, "text/xml");
    HTTPxml.SetHeader("Connection", "Close");
    string xbuf;
    
    // text/ascii packet
    HTTPResponse HTTPtext(200, "text/ascii");
    HTTPtext.SetHeader("Connection", "Close");

    // Redirection packet
    HTTPResponse HTTPredirect(302, "");
    HTTPredirect.SetHeader("Connection", "Close");
    HTTPredirect.SetHeader("Location", "/");
    HTTPredirect.CreatePacket();

    // Authorization packet
    HTTPResponse HTTPauth(401, "text/html; charset=iso-8859-1");
    HTTPauth.SetHeader("Connection", "Close");
    HTTPauth.SetHeader("WWW-Authenticate", "Basic realm=\"NetDJ\"");
    HTTPauth.SetBody(
      "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n"
      "<HTML>\n"
      "  <HEAD>\n"
      "    <TITLE>NetDJ</TITLE>\n"
      "  </HEAD>\n"
      "  <BODY>\n"
      "    Authorization required!\n"
      "  </BODY>\n"
      "</HTML>\n");
    HTTPauth.CreatePacket();

    File songfile;
    string o_hsongname, o_xsongname;
    string cfilename;
    string tmpstr;
    string cmdbyuser;
    vector<File> songs;
    COMMAND *com;
    Directory* dir;
    char tmpint[10];
    char tmplint[20];
    struct pollfd pf;
    pf.events = POLLIN;
    int songsgot, songno;
    unsigned char ch;
    const ID3Tag* id3tag = NULL;
    HTTPRequest req;
    string auth;

    // I know, a hack - but this file name isn't that plausible is it?
    o_xsongname = o_hsongname = "///---///";

    cout << endl << "  HTTPThread: Accepting new connections" << endl;
    screen_flush();
    while (1 < 2) {
      len = sizeof(struct sockaddr_in);
      newsock = accept(http_sock,  (struct sockaddr *) &newsin, &len);
      if (newsock != -1) {
	//	cout << endl << "  HTTPThread: Got connectionrequest" << endl;
	//	screen_flush();
	
	currentsong.Get(songfile);
	// Get filename (without ending type)
	cfilename = songfile.GetFilenameNoType();
	
	// Receive something from the client ...
	pf.fd = newsock;
	if (poll(&pf, 1, 1000) > 0) {
	  int recno = recv(newsock, rbuf, sizeof(rbuf) - 1, 0);
	  rbuf[recno] = '\0';
	  
	  // #define _DEBUG_HTTP
	  
#ifdef _DEBUG_HTTP
	  cout << endl << "  HTTPThread: Received = " << endl << rbuf << endl;
	  screen_flush();
#endif
	  
	  if (!http_locked) {
	    if (req.Parse(rbuf) && req.GetCommand() == "GET") {
	      // Commands
	      if (req.GetURIName().substr(0, 9) == "/cgi-bin/") {
#ifdef _DEBUG_HTTP
		cout  << endl<< "  HTTPThread: COMMAND: " << req.GetURI() << endl;
		screen_flush();
#endif
		
		// Check authorization
		if (req.GetHeader("Authorization", auth) && auth.substr(0, 6) == "Basic ") {
		  tmpstr = base64_decode(auth.substr(6).c_str());
#ifdef _DEBUG_HTTP
		  cout << "  HTTPThread: '" << auth.substr(6) << "'" << endl;
		  cout << "  HTTPThread: Authorization: " << tmpstr << endl;
		  screen_flush();
#endif
		  if (acc.IsAccessAllowed(tmpstr, Level_PowerUser, &cmdbyuser)) {
		    // Seperate command from rest of request
		    com = find_command(req.GetURIName().substr(9).c_str(), false);
		    if (com) {
		      cout << endl << "  HTTP-CMD: '" << cmdbyuser
			   << "' requested '"
			   << req.GetURIName().substr(9) << "'" << endl;
		      ((*(com->func)) ((char*) NULL));
		    }
		    // Send redirect to '/'
		    send(newsock, HTTPredirect.Packet().c_str(), HTTPredirect.Packet().size(), 0);
		  } else {
		    // Wrong password, send authorization request
		    send(newsock, HTTPauth.Packet().c_str(), HTTPauth.Packet().size(), 0);
		  }
		} else {
		  // Didn't send credentials, send authorization request
		  send(newsock, HTTPauth.Packet().c_str(), HTTPauth.Packet().size(), 0);
		}
	      } else if (req.GetURIName() == "/index.txt") {
		///////////////////
		// Send TEXT answer
		//////////////////
		HTTPtext.SetBody(cfilename);
		HTTPtext.CreatePacket();
		send(newsock, HTTPtext.Packet().c_str(), HTTPtext.Packet().size(), 0);
		
	      } else if (req.GetURIName() == "/index.xml") {
		//////////////////
		// Send XML answer
		//////////////////
		if (o_xsongname != songfile.GetName()) {
		  xbuf = 
		    "<?xml version=\"1.0\"?>\n"
		    "<netdj id=\"netdj\" xmlns:html=\"http://www.w3.org/1999/xhtml\"\n"
		    "                  xmlns:xlink=\"http://www.w3.org/1999/xlink\">\n";

		  xbuf += "  <currentsong>\n";
		  xbuf += "    <unid>0</unid>\n";
		  xbuf += "    <listid>current</listid>\n";
		  xbuf += "    <size>";
		  sprintf(tmplint, "%ld", songfile.GetSize());
		  xbuf += tmplint;
		  xbuf += "</size>\n";
		  xbuf += "    <description>" + songfile.GetFilenameNoType() + "</description>\n";
		  songfile.GetID3Info(id3tag);
		  xbuf += "    <artist>" +  id3tag->GetArtist() + "</artist>\n";
		  xbuf += "    <album>" + id3tag->GetAlbum() + "</album>\n";
		  xbuf += "    <title>" + id3tag->GetTitle() + "</title>\n";
		  xbuf += "    <comment>" + id3tag->GetNote() + "</comment>\n";
		  xbuf += "    <year>" + id3tag->GetYear() + "</year>\n";
		  xbuf += "    <style>" + id3tag->GetStyle() + "</style>\n";
		  xbuf += "  </currentsong>\n";
		  songsgot = songno = 0;
		  for (unsigned int listno = 0; listno < listnum; ++listno) {
		    songs.clear();
		    songsgot += lists[listno]->GetEntries(songs, 10 - songsgot);
		    for (vector<File>::iterator it = songs.begin();
			 it != songs.end();
			 ++it) {
		      xbuf += "  <song id=\"";
		      sprintf(tmpint, "%d", ++songno);
		      xbuf += tmpint;
		      xbuf += "\">\n";
		      xbuf += "    <unid>";
		      sprintf(tmpint, "%d", it->GetId());
		      xbuf += tmpint;
		      xbuf += "</unid>\n";
		      xbuf += "    <listid>" + lists[listno]->GetShortname() + "</listid>\n";
		      xbuf += "    <size>";
		      sprintf(tmplint, "%ld", it->GetSize());
		      xbuf += tmplint;
		      xbuf += "</size>\n";
		      xbuf += "    <description>" + it->GetFilenameNoType() + "</description>\n";
		      it->GetID3Info(id3tag);
		      xbuf += "    <artist>" + id3tag->GetArtist() + "</artist>\n";
		      xbuf += "    <album>" + id3tag->GetAlbum() + "</album>\n";
		      xbuf += "    <title>" + id3tag->GetTitle() + "</title>\n";
		      xbuf += "    <comment>" + id3tag->GetNote() + "</comment>\n";
		      xbuf += "    <year>" + id3tag->GetYear() + "</year>\n";
		      xbuf += "    <style>" + id3tag->GetStyle() + "</style>\n";
		      xbuf += "  </song>\n";
		    }
		    if (songsgot >= 10) {
		      break;
		    }
		  } // iterate through lists
		  for (unsigned int i = 0; i < listnum; ++i) {
		    dir = lists[i];
		    xbuf += "  <list>\n";
		    sprintf(tmpint, "%d", dir->GetSize());
		    xbuf += "    <size>";
		    xbuf += tmpint;
		    xbuf += "</size>\n";
		    xbuf += "    <id>" + dir->GetShortname() + "</id>\n";
		    xbuf += "    <description>" + dir->GetDescription() + "</description>\n";
		    xbuf += "  </list>\n";
		  }
		  xbuf += "</netdj>\n";
		  o_xsongname = songfile.GetName();
		  // Replace illegal characters
		  for (string::iterator it = xbuf.begin();
		       it != xbuf.end();
		       ++it) {
		    ch = (unsigned char) *it;
		    if ((ch == '&' || ch == '`' || ch > 127 || ch < 32)
			&& ch != '\n' && ch != '\r') {
		      *it = ' ';
		    }
		  }
		  HTTPxml.SetBody(xbuf);
		  HTTPxml.CreatePacket();
		}
		send(newsock, HTTPxml.Packet().c_str(), HTTPxml.Packet().size(), 0);
	      } else {
		///////////////////
		// Send HTML answer
		///////////////////
		if (o_hsongname != songfile.GetName()) {
		  hbuf = hbuf1 + cfilename + hbuf2;
		  sprintf(tmpint, "%d", songfile.GetId());
		  hbuf += tmpint;
		  hbuf += hbuf3;
		  songs.clear();
		  lists[1]->GetEntries(songs, 10);
		  for (vector<File>::iterator it = songs.begin();
		       it != songs.end();
		       ++it) {
		    hbuf += "    " + it->GetFilenameNoType() + "<BR>\n";
		  }
		  hbuf += hbuf4;
		  for (unsigned int i = 0; i < listnum; ++i) {
		    dir = lists[i];
		    hbuf += "      <TR><TD>";
		    sprintf(tmpint, "%d", dir->GetSize());
		    hbuf += tmpint;
		    hbuf += "</TD><TD>";
		    hbuf += dir->GetShortname();
		    hbuf += "</TD><TD>";
		    hbuf += dir->GetDescription();
		    hbuf += "</TD></TR>\n";
		  }
		  hbuf += hbuf5;
		  o_hsongname = songfile.GetName();
		}
		send(newsock, hbuf.c_str(), hbuf.size(), 0);
	      }
	    } // req.Parse()
	  } // !http_locked
	}
	close(newsock);
      }
    }
  }
    pthread_exit(NULL);
    // An extremely weird bug in the pthread_cleanup_push macro needs
    // an extra }
  }
}


////////////////////////////////////////
// MAIN
int
main(int argc, char* argv[]) {
  cout << "Welcome to NetDJ v" << VERSION << endl;

  // Read configuration
  config.ReadFile();

  // Read users
  acc.ReadFile();

  // Load saved state from disk
  loadstate();

  // Init rand()
  srand(time(NULL));

  // Init playlists
  cache.SetDirname(config.GetString("CACHE_DIR"));
  share.SetDirname(config.GetString("SHARE_DIR"));

  // Start player-thread
  if (config.GetBool("PLAYER_START")) {
    pthread_create(&player_t, NULL, &player_thread, NULL);
  };

  http_locked = config.GetBool("WEB_LOCKED");

  // Start http-thread
  if (config.GetBool("HTTP_START")) {
    com_webstart(NULL);
  };

  // Give the threads a chance to start before starting UI
  usleep(500);

  string songname;
  bool done = false;
#ifdef USE_READLINE
  char* line = (char*) NULL;
#else
  char line[255];
#endif

  // UI-loop
  do {
#ifdef USE_READLINE
    if (line) {
      free(line);
    }
    line = readline("NetDJ> ");
#else
    cout << "NetDJ> " << flush;
    cin.getline(line, sizeof(line));
#endif
    if (line && *line ) {
#ifdef USE_READLINE
#   ifdef HAVE_READLINE_HISTORY
      add_history(line);
#  endif
#endif
      if (strncmp(line, "quit", 4) == 0) {
	done = true;
      } else {
	execute_line(line);
      }
    }
  } while (!done);

  // Save state to disk
  savestate();

  cout << "Stopping player" << endl;
  stop_player = true;
  kill_current();
  pthread_cancel(player_t);
  cout << "Stopping HTTP" << endl;
  pthread_cancel(http_t);

  return 0;
}
