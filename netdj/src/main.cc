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

// HACK to correct c++ "bug" in shout.h
#define namespace nspace
#include <shout/shout.h>

// Provides cout and manipulation of same
#include <iostream>
#include <iomanip>

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

// Provides wait, fork and unlink
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
AccessConf acc;
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
  statefile += CONF_STATEFILENAME;

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
  string currsong;
  bool   prevdelete;
  string prevsong;

public:
  TCurrentSong() : Lockable() {};

  void Get(string& str) {
    lock();
    str = currsong;
    unlock();
  }
  
  void Set(const string& str, const bool del) {
    lock();
    if (config.GetBool("DELETE_PLAYED") && prevdelete && prevsong.size()) {
      if (unlink(prevsong.c_str())) {
	cout << "Couldn't delete '" << prevsong << "'!" << endl;
	screen_flush();
      }
    }
    prevsong = currsong;
    prevdelete = currdelete;
    
    currsong = str;
    currdelete = del;
    unlock();
  }

  void ModifyCurrent(const string& str, const bool del) {
    lock();
    currsong = str;
    currdelete = del;
    unlock();
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
    screen_flush();
    stop_player = true;
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
      currentsong.Set("", false);
      screen_flush();
      sleep(30);
    } else {
      if (fobj.Exists()) {
	currentsong.Set(fobj.GetName(), i == 1 ? true : false);
	cout << endl << "  Playing '" << fobj.GetFilename()
	     << "' [" << *(lists[i]->GetShortname()) << "]" << endl;
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
	}
      }
    }
  }

  if (config.GetBool("STREAM")) {
    shout_update_metadata(&conn, NULL);
    shout_disconnect(&conn);
  }

  // Clear currentsong and end thread
  currentsong.Set("", false); 
  pthread_exit(NULL);
}


void kill_current() {
  if (config.GetBool("STREAM")) {
    // This should be packed nicely into a semaphore...
    next_song = true;
  } else if (playercmd_pid) {
    kill(playercmd_pid, SIGINT);
  }
}


////////////////////////////////////////
// COMMANDS
typedef void (*COMFUNC)(char*);

int com_add(char*);
int com_del(char*);
int com_help(char*);
int com_info(char*);
int com_list(char*);
int com_lists(char*);
int com_move(char*);
int com_next(char*);
int com_start(char*);
int com_stop(char*);
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
  {"del",      com_del,      false, "Delete (!!!) current song from disk"},
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
  {"weblock",  com_weblock,  false, "Lock webacces"},
  {"webstart", com_webstart, false, "Start web_thread"},
  { (char*) NULL, (rl_icpfunc_t*) NULL, false, (char*) NULL}
};

int
com_add(char* arg) {
  string tmp;
  if (*arg) {
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
com_del(char* arg) {
  string songname;
  currentsong.Get(songname);
  if (remove(songname.c_str()) == -1) {
    cout << "  Error: " << strerror(errno) << endl;
  } else {
    cout << "  OK, it's gone!" << endl;
    kill_current();
  }
  return 0;
};

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
  ID3Tag *id3tag = NULL;
  time_t filetime;
  string songname;
  File fobj;

  currentsong.Get(songname);
  if (fobj.SetName(songname)) {
    filetime = fobj.GetMtime();
    cout << "  " << songname << endl;
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
  }
  return 0;
};

int
com_list(char* arg) {
  unsigned int l = atoi(arg);
  vector<File> songs;

  if (l >= 0 && l < listnum) {
    cout << "  [" << *(lists[l]->GetShortname()) << "]:" << endl;
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
	 << "  " << *(dir->GetDescription())
	 << " [" << *(dir->GetShortname()) << "]"
	 << endl;
  }
  return 0;
}

int
com_move(char* arg) {
  string songname;
  string newpath;
  currentsong.Get(songname);
  File songfile(songname);

  if (songfile.Exists()) {
    newpath = *(share.GetDirname());
    newpath += songfile.GetFilename();
    
    cout << "  Moving '" << songname << "' to '"
	 << newpath << "'" << endl;
    if (rename(songname.c_str(), newpath.c_str()) == -1) {
      cout << "  Error: " << strerror(errno) << endl;
    } else {
      currentsong.ModifyCurrent(newpath, false);
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
com_stop(char* arg) {
  stop_player = true;
  kill_current();
  return 0;
};

int
com_start(char* arg) {
  stop_player = false;
  pthread_create(&player_t, NULL, &player_thread, NULL);
  return 0;
};

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
find_command(char* name, bool admin) {
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

    // XML-answer
    const char xbuf1[] =
      "HTTP/1.1 200 OK\r\n"
      "Server: NetDJ\r\n"
      "Connection: close\r\n"
      "Content-Type: text/xml\r\n"
      "\r\n"
      "<?xml version=\"1.0\"?>\n"
      "<netdj id=\"netdj\" xmlns:html=\"http://www.w3.org/1999/xhtml\"\n"
      "                    xmlns:xlink=\"http://www.w3.org/1999/xlink\">\n";

    const char xbuf2[] =
      "</netdj>\n";

    string xbuf;


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
      "    - [<A HREF=\"/cgi-bin/next\">Skip song</A>]\n"
      "    - [<A HREF=\"/cgi-bin/stop\">Stop player</A>]\n"
      "    - [<A HREF=\"/cgi-bin/start\">Start player</A>]\n"
      "    <P>\n"
      "    <STRONG>Next songs in cache:</STRONG><BR>\n";

    const char hbuf3[] =
      "    <P>\n"
      "    <STRONG>Playlists:</STRONG><BR>\n"
      "    <TABLE BORDER=1>\n"
      "      <TR><TD><B>Size</B></TD><TD><B>Name</B></TD><TD><B>Description</B></TD></TR>\n";

    const char hbuf4[] =
      "    </TABLE>\n"
      "    <P>\n"
      "    Add to <A HREF=\"javascript:addNetscapePanel();\">Netscape sidebar</A>"
      " - <A HREF=\"/\" REL=\"sidebar\" TITLE=\"NetDJ\">Opera panel</A>\n"
      "  </BODY>\n"
      "</HTML>\n";

    string hbuf;


    // TEXT-answer
    const char tbuf[] =
      "HTTP/1.1 200 OK\r\n"
      "Server: NetDJ\r\n"
      "Connection: close\r\n"
      "Content-Type: text/ascii\r\n"
      "\r\n";

    // COMMAND-answer
    const char cbuf[] =
      "HTTP/1.1 302 Found\r\n"
      "Connection: close\r\n"
      "Server: NetDJ\r\n"
      "Location:/\r\n"
      "\r\n";

    const char abuf[] =
      "HTTP/1.1 401 Authorization Required\r\n"
      "Server: NetDJ\r\n"
      "WWW-Authenticate: Basic realm=\"NetDJ\"\r\n"
      "Connection: close\r\n"
      "Content-Type: text/html; charset=iso-8859-1\r\n"
      "\r\n"
      "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n"
      "<HTML>\n"
      "  <HEAD>\n"
      "    <TITLE>NetDJ</TITLE>\n"
      "  </HEAD>\n"
      "  <BODY>\n"
      "    Authorization required!\n"
      "  </BODY>\n"
      "</HTML>\n";

    string songname, o_hsongname, o_xsongname;
    string cfilename;
    string tmpstr;
    string cmdbyuser;
    vector<File> songs;
    char *spos, *apos;
    COMMAND *com;
    Directory* dir;
    char tmpint[10];
    char tmplint[20];
    struct pollfd pf;
    pf.events = POLLIN;
    int z;
    unsigned char ch;
    ID3Tag *id3tag = NULL;

    // I know, a hack - but this file name isn't that plausible is it?
    o_xsongname = o_hsongname = "///---///";

    cout << endl << "  HTTPThread: Accepting new connections" << endl;
    screen_flush();

    while (1 < 2) {
      newsock = accept(http_sock,  (struct sockaddr *) &newsin, &len);
      if (newsock != -1) {
	//	cout << endl << "  HTTPThread: Got connectionrequest" << endl;
	//	screen_flush();

	currentsong.Get(songname);
	// Get filename (without .-ending)
	cfilename = File(songname.substr(0, songname.find_last_of('.'))).GetFilename();

	// Receive something from the client ...
	pf.fd = newsock;
	if (poll(&pf, 1, 1000) > 0) {
	  int recno = recv(newsock, rbuf, sizeof(rbuf) - 1, 0);
	  rbuf[recno] = '\0';
	  
	  //#define _DEBUG_HTTP
	  
#ifdef _DEBUG_HTTP
	  cout << endl << "  HTTPThread: Received = " << endl << rbuf << endl;
	  screen_flush();
#endif
	  
	  if (!http_locked && (spos = strstr(rbuf, "GET /cgi-bin/"))) {
#ifdef _DEBUG_HTTP
	    cout  << endl<< "  HTTPThread: COMMAND: " << spos << endl;
	    screen_flush();
#endif
	    
	    // Check authorization
	    if ((apos = strstr(rbuf, "Authorization: Basic "))) {
	      apos += 21;
	      *strchr(apos, '\n') = '\0';
	      tmpstr = base64_decode(apos);
#ifdef _DEBUG_HTTP
	      cout << "  HTTPThread: Authorization: " << tmpstr << endl;
	      screen_flush();
#endif
	      if (acc.IsAccessAllowed(tmpstr, &cmdbyuser)) {
		// Seperate command from rest of request
		*strchr(spos + 13, ' ') = '\0';
		com = find_command(spos + 13, false);
		if (com) {
		  cout << endl << "  HTTP-CMD: '" << cmdbyuser
		       << "' requested '"
		       << (spos + 13) << "'" << endl;
		  ((*(com->func)) ((char*) NULL));
		}
		// Send redirect to '/'
		send(newsock, cbuf, sizeof(cbuf), 0);
	      } else {
		// Wrong password, send authorization request
		send(newsock, abuf, sizeof(abuf), 0);
	      }
	    } else {
	      // Didn't send credentials, send authorization request
	      send(newsock, abuf, sizeof(abuf), 0);
	    }
	  } else if ((spos = strstr(rbuf, "GET /"))) {
#ifdef _DEBUG_HTTP
	    cout  << endl<< "  HTTPThread: Send INDEX" << endl;
	    screen_flush();
#endif
	    if (strncmp(spos + 5, "index.txt", 9) == 0) {
	      ///////////////////
	      // Send TEXT answer
	      //////////////////
	      send(newsock, tbuf, sizeof(tbuf) - 1, 0);
	      send(newsock, cfilename.c_str(), cfilename.size(), 0);

	    } else if (strncmp(spos + 5, "index.xml", 9) == 0) {
	      //////////////////
	      // Send XML answer
	      //////////////////
	      if (o_xsongname != songname) {
		xbuf = xbuf1;
		xbuf += "  <currentsong>\n";
		xbuf += "     <description>" + cfilename + "</description>\n";
#ifdef HAVE_ID3LIB  
		tag.Link(songname.c_str());
		xbuf += "     <artist>";
		if ((id3frame = tag.Find(ID3FID_LEADARTIST)) ||
		    (id3frame = tag.Find(ID3FID_BAND))       ||
		    (id3frame = tag.Find(ID3FID_CONDUCTOR))  ||
		    (id3frame = tag.Find(ID3FID_COMPOSER))) {
		  xbuf += id3frame->GetField(ID3FN_TEXT)->GetRawText();
		}
		xbuf += "</artist>\n";
		xbuf += "     <album>";
		if ((id3frame = tag.Find(ID3FID_ALBUM))) {
		  xbuf += id3frame->GetField(ID3FN_TEXT)->GetRawText();
		};
		xbuf += "</album>\n";
		xbuf += "     <title>";
		if ((id3frame = tag.Find(ID3FID_TITLE))) {
		  xbuf += id3frame->GetField(ID3FN_TEXT)->GetRawText();
		};
		xbuf += "</title>\n";
		xbuf += "     <comment>";
		if ((id3frame = tag.Find(ID3FID_COMMENT))) {
		  xbuf += id3frame->GetField(ID3FN_TEXT)->GetRawText();
		};	
		xbuf += "</comment>\n";
		tag.Clear();
#endif
		xbuf += "  </currentsong>\n";
		songs.clear();
		lists[1]->GetEntries(songs, 10);
		z = 0;
		for (vector<File>::iterator it = songs.begin();
		     it != songs.end();
		     ++it) {
		  xbuf += "  <song id=\"";
		  sprintf(tmpint, "%d", ++z);
		  xbuf += tmpint;
		  xbuf += "\">\n";
		  xbuf += "    <unid>";
		  sprintf(tmpint, "%d", it->GetId());
		  xbuf += tmpint;
		  xbuf += "</unid>\n";
		  xbuf += "    <size>";
		  sprintf(tmplint, "%ld", it->GetSize());
		  xbuf += tmplint;
		  xbuf += "</size>\n";
		  xbuf += "    <description>" + it->GetFilename() + "</description>\n";
		  it->GetID3Info(id3tag);
		  xbuf += "     <artist>";
		  xbuf += id3tag->GetArtist();
		  xbuf += "</artist>\n";
		  xbuf += "     <album>";
		  xbuf += id3tag->GetAlbum();
		  xbuf += "</album>\n";
		  xbuf += "     <title>";
		  xbuf += id3tag->GetTitle();
		  xbuf += "</title>\n";
		  xbuf += "     <comment>";
		  xbuf += id3tag->GetNote();
		  xbuf += "</comment>\n";
		  xbuf += "     <year>";
		  xbuf += id3tag->GetYear();
		  xbuf += "</year>\n";
		  xbuf += "     <style>";
		  xbuf += id3tag->GetStyle();
		  xbuf += "</style>\n";
		  xbuf += "  </song>\n";
		}
		for (unsigned int i = 0; i < listnum; ++i) {
		  dir = lists[i];
		  xbuf += "  <list>\n";
		  sprintf(tmpint, "%d", dir->GetSize());
		  xbuf += "     <size>";
		  xbuf += tmpint;
		  xbuf += "</size>\n";
		  xbuf += "     <id>" + *(dir->GetShortname()) + "</id>\n";
		  xbuf += "     <description>" + *(dir->GetDescription()) + "</description>\n";
		  xbuf += "  </list>\n";
		}
		xbuf += xbuf2;
		o_xsongname = songname;
	      }
	      // Replace illegal characters
  	      for (string::iterator it = xbuf.begin();
  		   it != xbuf.end();
  		   ++it) {
		ch = (unsigned char) *it;
  		if (ch == '&' || ch == '`' ||
		    (ch > 127 || ch < 32 && ch != '\n' && ch != '\r')) {
  		  *it = ' ';
  		}
  	      }
	      send(newsock, xbuf.c_str(), xbuf.size(), 0);

	    } else {
	      ///////////////////
	      // Send HTML answer
	      ///////////////////
	      if (o_hsongname != songname) {
		hbuf = hbuf1 + cfilename + hbuf2;
		songs.clear();
		lists[1]->GetEntries(songs, 10);
		for (vector<File>::iterator it = songs.begin();
		     it != songs.end();
		     ++it) {
		  hbuf += "    " + it->GetFilename() + "<BR>\n";
		}
		hbuf += hbuf3;
		for (unsigned int i = 0; i < listnum; ++i) {
		  dir = lists[i];
		  hbuf += "      <TR><TD>";
		  sprintf(tmpint, "%d", dir->GetSize());
		  hbuf += tmpint;
		  hbuf += "</TD><TD>";
		  hbuf += *(dir->GetShortname());
		  hbuf += "</TD><TD>";
		  hbuf += *(dir->GetDescription());
		  hbuf += "</TD></TR>\n";
		}
		hbuf += hbuf4;
		o_hsongname = songname;
	      }
	      send(newsock, hbuf.c_str(), hbuf.size(), 0);
	    }
	  }
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

  // Construct dotfile-path
  char* home = getenv("HOME");
  string dotfile;
  if (home) {
    dotfile = home;
    if (dotfile[dotfile.size() - 1] != '/') {
      dotfile += '/';
    }
  }

  // Read configuration
  config.ReadFile(dotfile + CONF_DOTFILENAME);

  // Read users
  acc.ReadFile(dotfile + CONF_USERFILENAME);

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
