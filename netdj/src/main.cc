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

// Provides wait and fork
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

// ID3-handling
#ifdef HAVE_ID3LIB
#  include <id3/tag.h>
#  include <id3/misc_support.h>
#endif

void
screen_flush() {
#ifdef USE_READLINE
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

class TCurrentSong : public Lockable{
private:
  string songname;

public:
  TCurrentSong() : Lockable() {};

  void Get(string& str) {
    lock();
    str = songname;
    unlock();
  }
  
  void Set(const string& str) {
    lock();
    songname = str;
    unlock();
  }
};

TCurrentSong currentsong;
int playercmd_pid = 0;
bool stop_player  = false;
pthread_t player_t;

void*
player_thread(void*) {
  unsigned int i;
  string filename;

  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

  while (!stop_player) {
    filename = "";
    i = 0;
    while (filename.empty() && i < listnum) {
      lists[i]->GetSong(filename);
      ++i;
    }
    --i;
    if (filename.empty()) {
      cout << endl << "  Hmmm, no files to play..." << endl;
      screen_flush();
      sleep(30);
    } else {
      if (file_exists(filename.c_str(), NULL)) {
	currentsong.Set(filename);
	cout << endl << "  Playing '" << get_filename(filename)
	     << "' [" << *(lists[i]->GetShortname()) << "]" << endl;
	screen_flush();
	
	// Play song
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
		 filename.c_str(),
		 NULL);
	} else {
	  wait(NULL);
	}
      }
    }
  }

  // Clear currentsong and end thread
  currentsong.Set(""); 
  pthread_exit(NULL);
}


void kill_current() {
  if (playercmd_pid) {
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
  {"add",     com_add,     false, "Add song to list"},
  {"del",     com_del,     false, "Delete (!!!) current song from disk"},
  {"help",    com_help,    false, "This listing"},
  {"info",    com_info,    false, "Get information about the current song"},
  {"list",    com_list,    false, "Show next 10 entries in LIST"},
  {"lists",   com_lists,   false, "Show status for lists"},
  {"move",    com_move,    false, "Move current song to share"},
  {"next",    com_next,    true,  "Skip to next song"},
  {"otw",     com_yeah,    false, "(!) Show song(s) currently being downloaded"},
  {"save",    com_yeah,    false, "(!) Save current song to share"},
  {"start",   com_start,   true,  "Start player"},
  {"stop",    com_stop,    true,  "Stop player"},
  {"weblock", com_weblock, false, "Lock webacces"},
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
#ifdef HAVE_ID3LIB  
  ID3_Tag tag;
#endif
  time_t filetime;
  string songname;

  currentsong.Get(songname);
  if (!songname.empty()) {
    filetime = file_mtime(songname);
    cout << "  " << songname << endl;
    cout << "    " << setprecision(2)
         << ((float) file_size(songname) / (1024 * 1024)) << " MB - "
         << ctime(&filetime) << endl;
#ifdef HAVE_ID3LIB  
    tag.Link(songname.c_str());
    if (tag.Size()) {
      cout << "    Artist:  " << ID3_GetArtist(&tag) << endl;
      cout << "    Album:   " << ID3_GetAlbum(&tag) << endl;
      cout << "    Title:   " << ID3_GetTitle(&tag) << endl;
      cout << "    Comment: " << ID3_GetComment(&tag) << endl;
    }
    tag.Clear();
#endif
  }
  return 0;
};

int
com_list(char* arg) {
  unsigned int l = atoi(arg);
  vector<string> songs;

  if (l >= 0 && l < listnum) {
    cout << "  [" << *(lists[l]->GetShortname()) << "]:" << endl;
    lists[l]->GetEntries(songs, 10);
    for (vector<string>::iterator it = songs.begin();
	 it != songs.end();
	 ++it) {
      cout << "    " << (*it) << endl;
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
  newpath = *(share.GetDirname());
  newpath += get_filename(songname);

  cout << "  Moving '" << songname << "' to '"
       << newpath << "'" << endl;
  if (rename(songname.c_str(), newpath.c_str()) == -1) {
    cout << "  Error: " << strerror(errno) << endl;
  } else {
    currentsong.Set(newpath);
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

void*
http_thread(void*) {
  int sock;
  struct sockaddr_in sin;
  const bool tval = true;

  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons(config.GetInteger("HTTP_PORT"));
  sin.sin_family = AF_INET;

  sock = socket(PF_INET, SOCK_STREAM, 0);
  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void*) &tval, sizeof(tval));

  if (sock == -1) {
    cout << endl << "  HTTPThread: " << strerror(errno) << endl;
    screen_flush();
  } else if (bind(sock, (struct sockaddr *) &sin, sizeof(struct sockaddr_in)) == -1) {
    cout << endl << "  HTTPThread: " << strerror(errno) << endl;
    screen_flush();
  } else if (listen(sock, 5) == -1) {
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
      "<netdj>\n";

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
      "    Currently playing:\n"
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
    vector<string> songs;
    char *spos, *apos;
    COMMAND *com;
    Directory* dir;
    char tmpsize[10];
    struct pollfd pf;
    pf.events = POLLIN;

    // I know, a hack - but this file name isn't that plausible is it?
    o_xsongname = o_hsongname = "///---///";

    cout << endl << "  HTTPThread: Accepting new connections" << endl;
    screen_flush();

    while (1 < 2) {
      newsock = accept(sock,  (struct sockaddr *) &newsin, &len);
      if (newsock != -1) {
	//	cout << endl << "  HTTPThread: Got connectionrequest" << endl;
	//	screen_flush();

	currentsong.Get(songname);
	// Get filename (without .-ending)
	cfilename = get_filename(songname.substr(0, songname.find_last_of('.')));

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
	      if (acc.IsAccessAllowed(tmpstr)) {
		// Seperate command from rest of request
		*strchr(spos + 13, ' ') = '\0';
		com = find_command(spos + 13, false);
		if (com) {
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
		xbuf += "  <currentsong>\n    <description>" + cfilename + "</description>\n  </currentsong>\n";
		songs.clear();
		lists[1]->GetEntries(songs, 10);
		for (vector<string>::iterator it = songs.begin();
		     it != songs.end();
		     ++it) {
		  xbuf += "  <song>\n";
		  xbuf += "    <description>" + get_filename(*it) + "</description>\n";
		  xbuf += "  </song>\n";
		}
		for (unsigned int i = 0; i < listnum; ++i) {
		  dir = lists[i];
		  xbuf += "  <list>\n";
		  sprintf(tmpsize, "%d", dir->GetSize());
		  xbuf += "     <size>";
		  xbuf += tmpsize;
		  xbuf += "</size>\n";
		  xbuf += "     <shortname>" + *(dir->GetShortname()) + "</shortname>\n";
		  xbuf += "     <description>" + *(dir->GetDescription()) + "</description>\n";
		  xbuf += "  </list>\n";
		}
		xbuf += xbuf2;
		o_xsongname = songname;
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
		for (vector<string>::iterator it = songs.begin();
		     it != songs.end();
		     ++it) {
		  hbuf += "    " + get_filename(*it) + "<BR>\n";
		}
		hbuf += hbuf3;
		for (unsigned int i = 0; i < listnum; ++i) {
		  dir = lists[i];
		  hbuf += "      <TR><TD>";
		  sprintf(tmpsize, "%d", dir->GetSize());
		  hbuf += tmpsize;
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
  
  cout << "  HTTPThread: Exiting" << endl;
  screen_flush();
    
  pthread_exit(NULL);
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
  pthread_t http_t;
  if (config.GetBool("HTTP_START")) {
    pthread_create(&http_t, NULL, &http_thread, NULL);
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

  cout << "Stopping player" << endl;
  stop_player = true;
  kill_current();
  pthread_cancel(player_t);
  cout << "Stopping HTTP" << endl;
  pthread_cancel(http_t);

  // Save state to disk
  savestate();

  return 0;
}
