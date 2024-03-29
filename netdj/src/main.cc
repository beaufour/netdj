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

// Provides signal
#include <csignal>

// libshout
#include <shout/shout.h>

using namespace std;

////////////////////////////////////////
// GLOBALS
// Directories
Directory request("request", "Requested songs");
Directory cache  ("cache",   "Cache-dir");
Directory share  ("share",   "Share-dir", false);
const unsigned int listnum = 3;
Directory *lists[] = {&request, &cache, &share};

// Configs
Configuration config;
AccessConf acc(&config);

// HTTP-thread
bool http_locked;
void* http_thread(void*);
pthread_t http_t;

// Definition of userlevels
enum UserLevel {
  Level_Null  = 0,
  Level_User  = 50,
  Level_PowerUser = 100,
  Level_PrivUser = 200,
  Level_Admin = 1000
};


////////////////////////////////////////
// SCREEN_FLUSH
void
screen_flush() {
  // Seems like there are problems with this and old readline libraries...
#ifdef USE_READLINE_XX
  rl_forced_update_display();
#else
  //  if (!config.GetBool("DAEMON_MODE")) {
  //   cout << std::endl << "NetDJ> " << flush;
  // }
#endif
}

////////////////////////////////////////
// SAVE/LOAD state

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// Holds the full path to the statefile
std::string statefile;

void
savestate() {
  // Errors are not considered fatal here, because we're exiting.
  int fd;
  time_t upd = cache.GetNextTimestamp();

  std::cout << "Saving state to '" << statefile << "'." << std::endl;

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

  std::cout << "Reading state from '" << statefile << "'." << std::endl;

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
    if (prevdelete) {
      if (!prevfile.Delete()) {
	std::cout << "Couldn't delete '" << prevfile.GetName() << "'!" << std::endl;
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

  File Get() const {
    lock();
    File tmpobj = currfile;
    unlock();
    return tmpobj;
  }
  
  void Set(const File& fil, const bool del) {
    InternalSet(fil, del);
  }

  void Clear() {
    InternalSet(File(), false);
  }

  bool RenameCurrent(const std::string& newpath, const bool del) {
    bool res;
    lock();
    if ((res = currfile.Rename(newpath))) {
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
  bool delete_it;
  File fobj;

  FILE *musicfile;
  unsigned char buff[4096];
  long read;
  
  stop_player = false;
  next_song = false;
  
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
  
  // Init shoutcast stuff
  shout_init();
  shout_t* shout = shout_new();
  shout_metadata_t* metadata = shout_metadata_new();
  if (!shout || !metadata) {
    std::cout << std::endl << "  Couldn't create libshout structures!"
	      << shout_get_error(shout)
	      << std::endl;
    screen_flush();
    stop_player = true;
  }
  
  // Setup stream
  shout_set_user(shout, "source");
  shout_set_public(shout, 0);
  shout_set_format(shout, SHOUT_FORMAT_MP3);
  shout_set_protocol(shout, SHOUT_PROTOCOL_HTTP);
  shout_set_host(shout, config.GetString("STREAM_IP").c_str());
  shout_set_port(shout, config.GetInteger("STREAM_PORT"));
  shout_set_password(shout, config.GetString("STREAM_PASSWD").c_str());
  shout_set_mount(shout, config.GetString("STREAM_MOUNT").c_str());
  shout_set_name(shout, "NetDJ");
  shout_set_genre(shout, "Mixed");
  shout_set_description(shout, "NetDJ streaming channel");
  
  if (!stop_player && shout_open(shout) != SHOUTERR_SUCCESS) {
    std::cout << std::endl << "  Couldn't connect to icecast server: "
	      << shout_get_error(shout)
	      << std::endl;
    screen_flush();
    stop_player = true;
  }
  
  // Open logfile
  std::ofstream logfile;
  if (!stop_player && config.GetBool("PLAYER_LOG")) {
    std::string logfilename = config.GetString("$$CONFDIR") + CONF_LOGFILENAME;
    std::cout << std::endl << "  Logging to '" << logfilename << "'" << std::endl;
    logfile.open(logfilename.c_str(), ios::app);
    if (!logfile.is_open()) {
      std::cout << std::endl << "  Sorry, couldn't open logfile!" << std::endl;
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
      std::cout << std::endl << "  Hmmm, no files to play..." << std::endl;
      currentsong.Clear();
      screen_flush();
      sleep(30);
    } else {
      if (fobj.Exists() && fobj.GetName() != currentsong.Get().GetName()) {
	// This sucks, and should be part of the great
	// directory-reorganization scheme...
	switch(i) {
	  // Request
	case 0:
	  delete_it = true;
	  break;

	  // Cache
	case 1:
	  delete_it = config.GetBool("DELETE_PLAYED");
	  break;

	  // Share
	case 2:
	  delete_it = false;
	  break;
	
	  // What ever...
	default:
	  delete_it = false;
	  break;
	}
	currentsong.Set(fobj, delete_it);
	std::cout << std::endl << "  Playing '" << fobj.GetFilename()
		  << "' [" << lists[i]->GetShortname()
		  << ", " << fobj.GetOwner() << "]";
	std::cout << std::endl;
	screen_flush();
	
	// Stream song
	musicfile = fopen(fobj.GetName().c_str(), "r");
	if (musicfile) {
	  // TODO: Metadata
	  // shout_update_metadata(&conn, (char*) fobj.GetFilename().c_str());
	  while (!stop_player && !next_song) {
	    read = fread(buff, 1, 4096, musicfile);
	    if (read > 0) {
	      if (shout_send(shout, buff, read) != SHOUTERR_SUCCESS) {
		std::cout << std::endl
			  << "  Send error:" << shout_get_error(shout) << "..."
			  << std::endl;
		screen_flush();
		break;
	      }
	    } else {
	      break;
	    }
	    shout_sync(shout);
	  }
	  fclose(musicfile);
	} else {
	  // Couldn't open file
	  std::cout << std::endl
		    << "  Couldn't open '" << fobj.GetName() << "'"
		    << std::endl;
	  screen_flush();
	} // musicfile
	if (logfile.is_open()) {
	  logfile << time(NULL) << ","
		  << "\"" << fobj.GetFilenameNoType() << "\","
		  << ((stop_player || next_song) ? 1 : 0) << ",\""
		  << lists[i]->GetShortname() << "\""
		  << std::endl;
	}
      }
    }
  }
  
  // Disconnect from streaming server
  if (shout) {
    if (metadata) {
      shout_metadata_free(metadata);
    }
    shout_close(shout);
    shout_free(shout);
  }
  shout_shutdown();

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
int com_reload(char*);
int com_start(char*);
int com_stop(char*);
int com_version(char*);
int com_weblock(char*);
int com_webstart(char*);

int
com_yeah(char* arg) {
  std::cout << "  Coming soon (tm)" << std::endl;
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
  {"reload",   com_reload,   false, "Reload configuration"},
  {"save",     com_yeah,     false, "(!) Save current song to share"},
  {"start",    com_start,    true,  "Start player"},
  {"stop",     com_stop,     true,  "Stop player"},
  {"version",  com_version,  false, "Show program version"},
  {"weblock",  com_weblock,  false, "Lock webacces"},
  {"webstart", com_webstart, false, "Start web_thread"},
  { (char*) NULL, (rl_icpfunc_t*) NULL, false, (char*) NULL}
};

int
com_add(char* arg) {
  std::string tmp;
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
    File fobj(tmp);

    if (fobj.Symlink(request.GetDirname() + arg)) {
      std::cout << "  Added '" << tmp << "' to request-list" << std::endl;
    } else {
      std::cout << "  Couldn't add '" << tmp << "' to request-list" << std::endl;
    }
  } else {
    std::cout << "  Usage: add <filename>" << std::endl;
  };
  return 0;
}

int
com_help(char* arg) {
  for (int i = 0; commands[i].name; ++i)
    std::cout << "  " << setw(10) << commands[i].name
	 << "   " << commands[i].doc << std::endl;
  std::cout << std::endl;
  std::cout << "  " << setw(10) << "quit"
       << "   " << "Quit program" << std::endl;
  return 0;
};

int
com_info(char* arg) {
  ID3Tag const *id3tag = NULL;
  time_t filetime;
  File fobj;

  fobj = currentsong.Get();
  filetime = fobj.GetMtime();
  std::cout << "  " << fobj.GetName() << std::endl;
  std::cout << "    " << setprecision(2)
       << ((float) fobj.GetSize() / (1024 * 1024)) << " MB - "
       << ctime(&filetime) << std::endl;
  if (fobj.GetID3Info(id3tag)) {
    std::cout << "    Artist:  " << id3tag->GetArtist() << std::endl;
    std::cout << "    Album:   " << id3tag->GetAlbum() << std::endl;
    std::cout << "    Title:   " << id3tag->GetTitle() << std::endl;
    std::cout << "    Year:    " << id3tag->GetYear() << std::endl;
    std::cout << "    Style:   " << id3tag->GetStyle() << std::endl;
    std::cout << "    Comment: " << id3tag->GetNote() << std::endl;
  }
  return 0;
};

int
com_list(char* arg) {
  unsigned int l = arg ? atoi(arg) : 0;
  std::vector<File> songs;

  if (l >= 0 && l < listnum) {
    std::cout << "  [" << lists[l]->GetShortname() << "]:" << std::endl;
    lists[l]->GetEntries(songs, 10);
    for (std::vector<File>::iterator it = songs.begin();
	 it != songs.end();
	 ++it) {
      std::cout << "    " << it->GetFilename() << std::endl;
    }
  }
  return 0;
}

int
com_lists(char* arg) {
  Directory* dir;
  for (unsigned int i = 0; i < listnum; ++i) {
    dir = lists[i];
    std::cout << setw(7) << dir->GetSize()
	 << "  " << dir->GetDescription()
	 << " [" << dir->GetShortname() << "]"
	 << std::endl;
  }
  return 0;
}

int
com_move(char* arg) {
  std::string songname;
  std::string newpath;
  File songfile;
  songfile = currentsong.Get();

  if (songfile.Exists()) {
    newpath = share.GetDirname();
    newpath += songfile.GetFilename();
    
    std::cout << "  Moving '" << songfile.GetName() << "' to '"
	 << newpath << "'" << std::endl;
    if (currentsong.RenameCurrent(newpath, false)) {
      std::cout << "  Error! " << std::endl;
    }
  } else {
    std::cout << "  Error: File doesn't exist..." << std::endl;
  }
  return 0;
}

int
com_next(char* arg) {
  std::cout << "  Skipping current song..." << std::endl;
  kill_current();
  usleep(500);
  return 0;
};

int
com_reload(char* arg) {
  if (!strncmp(arg, "users", 5)) {
    std::cout << "Reloading users..." << std::endl;
    acc.ReadFile();
  } else if (!strncmp(arg, "config", 6)) {
    std::cout << "Reloading config (EXPERIMENTAL!)..." << std::endl;
    config.ReadFile();
  } else {
    std::cout << "  You have to give either 'users' or 'config' as parameter" << std::endl;
  }
  return 0;
}

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
  std::cout << "  " << PKGVER << std::endl
       << "  main.cc - CVS id  :"
       << " $Id$" << std::endl
       << "  main.cc - compiled: " << __DATE__ << " " << __TIME__ << std::endl
       << "  Shoutcast support: " << shout_version(0,0,0) << std::endl;

  return 0;
}

int
com_weblock(char* arg) {
  http_locked = !http_locked;
  std::cout << "  WebAccess " << (http_locked ? "OFF" : "ON") << std::endl;
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
    std::cout << "  Hmmm, did you really mean '" << line << "'?" << std::endl;
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
  std::cout << std::endl << "  HTTPThread: Exiting" << std::endl;
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
    std::cout << std::endl << "  HTTPThread: " << strerror(errno) << std::endl;
    screen_flush();
  } else if (bind(http_sock, (struct sockaddr *) &sin, sizeof(struct sockaddr_in)) == -1) {
    std::cout << std::endl << "  HTTPThread: " << strerror(errno) << std::endl;
    screen_flush();
  } else if (listen(http_sock, 5) == -1) {
    std::cout << std::endl << "  HTTPThread: " << strerror(errno) << std::endl;
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

    std::string hbuf;

    // text/xml packet
    HTTPResponse HTTPxml(200, "text/xml");
    HTTPxml.SetHeader("Connection", "Close");
    std::string xbuf;
    
    // text/ascii packet
    HTTPResponse HTTPtext(200, "text/ascii");
    HTTPtext.SetHeader("Connection", "Close");

    // Redirection packet
    HTTPResponse HTTPredirect(302, "");
    HTTPredirect.SetHeader("Connection", "Close");
    HTTPredirect.SetHeader("Location", config.GetString("HTTP_REDIRECT") == "" ? "/" : config.GetString("HTTP_REDIRECT"));
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
    std::string o_hsongname, o_xsongname;
    std::string cfilename;
    std::string tmpstr;
    std::string cmdbyuser;
    std::vector<File> songs;
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
    std::string auth;

    // I know, a hack - but this file name isn't that plausible is it?
    o_xsongname = o_hsongname = "///---///";

    std::cout << std::endl << "  HTTPThread: Accepting new connections" << std::endl;
    screen_flush();
    while (1 < 2) {
      len = sizeof(struct sockaddr_in);
      newsock = accept(http_sock,  (struct sockaddr *) &newsin, &len);
      if (newsock != -1) {
	//	std::cout << std::endl << "  HTTPThread: Got connectionrequest" << std::endl;
	//	screen_flush();
	
	songfile = currentsong.Get();
	// Get filename (without ending type)
	cfilename = songfile.GetFilenameNoType();
	
	// Receive something from the client ...
	pf.fd = newsock;
	if (poll(&pf, 1, 1000) > 0) {
	  int recno = recv(newsock, rbuf, sizeof(rbuf) - 1, 0);
	  rbuf[recno] = '\0';
	  
	  // #define _DEBUG_HTTP
	  
#ifdef _DEBUG_HTTP
	  std::cout << std::endl << "  HTTPThread: Received = " << std::endl << rbuf << std::endl;
	  screen_flush();
#endif
	  
	  if (!http_locked) {
	    if (req.Parse(rbuf) && req.GetCommand() == "GET") {
	      // Commands
	      if (req.GetURIName().substr(0, 9) == "/cgi-bin/") {
#ifdef _DEBUG_HTTP
		std::cout  << std::endl<< "  HTTPThread: COMMAND: " << req.GetURI() << std::endl;
		screen_flush();
#endif
		
		// Check authorization
		if (req.GetHeader("Authorization", auth) && auth.substr(0, 6) == "Basic ") {
		  tmpstr = base64_decode(auth.substr(6).c_str());
#ifdef _DEBUG_HTTP
		  std::cout << "  HTTPThread: '" << auth.substr(6) << "'" << std::endl;
		  std::cout << "  HTTPThread: Authorization: " << tmpstr << std::endl;
		  screen_flush();
#endif
		  if (acc.IsAccessAllowed(tmpstr, Level_PowerUser, &cmdbyuser)) {
		    // Seperate command from rest of request
		    com = find_command(req.GetURIName().substr(9).c_str(), false);
		    if (com) {
		      std::cout << std::endl << "  HTTP-CMD: '" << cmdbyuser
			   << "' requested '"
			   << req.GetURIName().substr(9) << "'" << std::endl;
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
		    for (std::vector<File>::iterator it = songs.begin();
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
		      xbuf += "    <owner>" + it->GetOwner() + "</owner>\n";
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
		  for (std::string::iterator it = xbuf.begin();
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
		if (config.GetString("HTTP_REDIRECT") != "") {
		  // Send redirect to '/'
		  send(newsock, HTTPredirect.Packet().c_str(), HTTPredirect.Packet().size(), 0);
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
		    for (std::vector<File>::iterator it = songs.begin();
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
  // The define is there to lure Emacs' indentation of the code...
#define XXLURE_INDENT {
  }
}


////////////////////////////////////////
// QUIT
int
quit() {
  // Save state to disk
  savestate();

  std::cout << "Stopping player" << std::endl;
  stop_player = true;
  kill_current();
  pthread_cancel(player_t);

  std::cout << "Stopping HTTP" << std::endl;
  pthread_cancel(http_t);
 
  return 0;
}


////////////////////////////////////////
// SIGNAL-HANDLING
void
sig_handler(int signum) {
  std::cout << "Caught signal " << signum << ", exiting." << std::endl;
  quit();
  exit(0);
}


////////////////////////////////////////
// MAIN
int
main(int argc, char* argv[]) {
  int pid = fork();
  if (pid == -1) {
    std::cout << "Hmmm, couldn't fork into the background?!" << std::endl;
    std::cout << "  " << strerror(errno) << std::endl;
    exit (-1);
  } else if (!pid) {
    // Daemon
    setpgid(0, 0);
    freopen("/dev/null", "r", stdin);
    freopen("/var/log/netdj", "w", stdout);
    freopen("/dev/null", "w", stderr);
    std::cout << "NetDJ v" << VERSION << " starting up." << std::endl;
    
    // Read configuration
    config.ReadFile();
    
    // Read users
    acc.ReadFile();
    
    // Load saved state from disk
    loadstate();
    
    // Init rand()
    srand(time(NULL));
    
    // Init playlists
    request.SetDirname(config.GetString("REQUEST_DIR"));
    cache.SetDirname(config.GetString("CACHE_DIR"));
    /* !!UGLY!! Should be configurable! */
    share.SetDirname(config.GetString("SHARE_DIR"), true);
    
    // Start player-thread
    if (config.GetBool("PLAYER_START")) {
      pthread_create(&player_t, NULL, &player_thread, NULL);
    };
    
    http_locked = config.GetBool("WEB_LOCKED");
    
    // Start http-thread
    if (config.GetBool("HTTP_START")) {
      com_webstart(NULL);
    };
    
    // Install signal-handler
    signal(SIGHUP, sig_handler);
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    // main loop
    while (1) {
      // Magic CORBA-thingy should enter here... :)
      sleep(999);
    }
    
  } else {
    std::cout << "Spawned daemon (pid " << pid << ")" << std::endl;
  }

  return 0;
}
