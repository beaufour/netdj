/**
 * \file PlayerThread.cpp
 * class PlayerThread
 *
 * $Id$
 *
 */
#include "PlayerThread.h"

#include <iostream>
#include <vector>

#include "Collection_Songlist_File.h"
#include "Collection_Songlist_Dir.h"
#include "Song.h"
#include "StdException.h"
#include "Shout.h"

using namespace std;

PlayerThread::PlayerThread(unsigned int stackSize)
  : QThread(stackSize), StopPlayer(false), SkipSong(false) {

}

void
PlayerThread::Stop() {
  StopPlayer = true;
}

void
PlayerThread::Skip() {
  SkipSong = true;
}

void
PlayerThread::run() {
  /** \todo Free up collections after use! */
  vector<Collection*> cols;

  try {
    cout << "-----=====> Open collections" << endl;
    cols.push_back(new Collection_Songlist_Dir("request", "Requests", "/tmp/netdj_request/", true));
    cols.push_back(new Collection_Songlist_File("share", "Shares", "mp3.list"));

    cout << "-----=====> Setup libshout" << endl;
    Shout shout("NetDJ", "Mixed", "NetDJ streaming channel");
    shout.SetPublic(false);
    shout.SetFormat(SHOUT_FORMAT_MP3);
    shout.SetUser("source");
    shout.SetProtocol(SHOUT_PROTOCOL_HTTP);
    shout.SetHost("localhost");
    shout.SetPort(8000);
    shout.SetPassword("hackme");
    shout.SetMount("netdj");
    
    cout << "-----=====> Connect to icecast" << endl;
    shout.Connect();
    
    cout << endl << "-----=====> MAIN LOOP" << endl;
    
    unsigned char buf[4096];
    int bytes;
    Song song;
    string colid;
    vector<Collection*>::const_iterator curcol;

    while (true) {
      cout << "-----=====> Get next song" << endl;

      for (curcol = cols.begin();
	   curcol != cols.end();
	   ++curcol) {
	try {
	  (*curcol)->Update();
	  song = (*curcol)->GetNextSong();
	  break;
	}
	catch (EmptyCollection &e) {
	}
      }

      /* Check whether we have found a song */
      if (curcol == cols.end()) {
	cout << "Hmmmm, no files to play" << endl;
	cout << "...waiting a wee time" << endl;
	/** \todo Can we do something better than just waiting 10 secs? */
	sleep(10);
	continue;
      }

      cout << "** Song info " << endl;
      cout << "UNID :     " << song.GetUNID() << endl;
      cout << "Type :     " << song.GetSongType() << endl;
      cout << "Filename : " << song.GetFilename() << endl;
      cout << "Collection:" << (*curcol)->GetIdentifier() << endl;
      
      if (song.GetSongType() != SongType_MP3) {
	cout << "INVALID SONGTYPE!" << endl;
	continue;
      }
      
      try {
	const SongInfo* info = song.GetSongInfo();
	if (info) {
	  cout << "** Song content info" << endl;
	  cout << "Description:  " << info->GetDescription() << endl;
	  cout << "Album:        " << info->GetAlbum() << endl;
	  cout << "Title:        " << info->GetTitle() << endl;
	  cout << "Artist:       " << info->GetArtist() << endl;
	  cout << "Genre:        " << info->GetGenre() << endl;
	  cout << "Year:         " << info->GetYear() << endl;
	  cout << "Track:        " << info->GetTrack() << endl;
	  cout << "Size:         " << info->GetSize() << endl;
	  cout << "Owner:        " << info->GetOwner() << endl;
	}
	
      }
      catch (NoSongInfo& h) {
	cout << "No header info" << endl;
      }

      cout << "-----=====> Send song to icecast" << endl;
      QFile file(song.GetFilename());
      if (!file.open(IO_ReadOnly)) {
	continue;
      }
      
      /* Send song */
      shout.SetSongName(song.GetFilename());
      while ((bytes = file.readBlock((char*) buf, sizeof(buf))) > 0
	     && StopPlayer == false && SkipSong == false) {
	shout.Send(buf, bytes);
	shout.Sleep();
      }
      
      /* Close file */
      file.close();
      
      if (StopPlayer) {
	break;
      }
      if (SkipSong) {
	SkipSong = false;
	cout << "Skipping current song!" << endl;
      }
    } // Main loop
  }
  catch (ShoutException& e) {
    cout << "[ShoutException] " << e.what() << endl;
  }
  catch (StdException &e) {
    cout << "[StdException] " << e.what() << endl;
  }
  catch (exception& e) {
    cout << "[std::exception] " << e.what() << endl;
  }
  catch (...) {
    cout << "Unknown exception!!" << endl;
    }
  cout << "Player thread stopped!" << endl;
}
