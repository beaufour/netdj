/**
 * \file Shout.cpp
 * Class Shout.
 *
 * $Id$
 *
 */

#include "Shout.h"

using namespace std;


ShoutException::ShoutException(const string msg)
  : StdException(msg) {
  
}


Shout_Metadata::Shout_Metadata() {
  metadata = shout_metadata_new();
  if (!metadata) {
    throw ShoutException("[shout_metadata_new] Could not create new metadata_t!");
  }
}

Shout_Metadata::~Shout_Metadata() {
  if (metadata) {
    shout_metadata_free(metadata);
  }
}

void
Shout_Metadata::Add(const string name, const string val) {
  if (shout_metadata_add(metadata, name.c_str(), val.c_str()) != SHOUTERR_SUCCESS) {
    throw ShoutException("[shout_metadata_add] Could not add metadata '" + name + "' = '"+ val +"'");
  }
}

void
Shout::Init() {
  shout_init();

  shout = shout_new();
  if (!shout) {
    throw ShoutException("[shout_new] Could not create shout_t!");
  }
}

Shout::Shout()
  : shout(0)
{
  Init();
}

Shout::Shout(const string name, const string genre,
	     const string descr)
  : shout(0)
{

  Init();

  SetName(name);
  SetGenre(genre);
  SetDescription(descr);
}

Shout::~Shout() {
  if (shout) {
    shout_close(shout);
    shout_free(shout);
  }
  shout_shutdown();
}

inline void
Shout::ErrorWrapper(const string func, const int err) {
  if ((err) != SHOUTERR_SUCCESS) {
    throw ShoutException("[" + func + "] " + shout_get_error(shout));
  } 
}

void
Shout::Connect() {
  ErrorWrapper("shout_open", shout_open(shout));
}

void
Shout::Disconnect() {
  ErrorWrapper("shout_close", shout_close(shout));
}

void
Shout::SetSongName(const string name) {
  Shout_Metadata meta;
  meta.Add("song", name.c_str());

  ErrorWrapper("[shout_set_metadata]", shout_set_metadata(shout, meta.metadata));
}

void
Shout::Send(const unsigned char *data, size_t len) {
  ErrorWrapper("shout_send", shout_send(shout, data, len));
}

void
Shout::Sleep() throw() {
  shout_sync(shout);
}

int
Shout::GetDelay() throw() {
  return shout_delay(shout);
}

void
Shout::SetHost(const string host) {
  ErrorWrapper("shout_set_host", shout_set_host(shout, host.c_str()));
}

void
Shout::SetPort(const unsigned short port) {
  ErrorWrapper("shout_set_port", shout_set_port(shout, port));
}

void
Shout::SetMount(const string mount) {
  ErrorWrapper("shout_set_mount", shout_set_mount(shout, mount.c_str()));

}

void
Shout::SetUser(const string uname) {
  ErrorWrapper("shout_set_user", shout_set_user(shout, uname.c_str()));
}

void
Shout::SetPassword(const string pass) {
  ErrorWrapper("shout_set_password", shout_set_password(shout, pass.c_str()));
}

void
Shout::SetFormat(const unsigned int format) {
  ErrorWrapper("shout_set_format", shout_set_format(shout, format));
}

void
Shout::SetProtocol(const unsigned int prot) {
  ErrorWrapper("shout_protocol", shout_set_protocol(shout, prot));
}

void
Shout::SetPublic(const bool pub) {
  ErrorWrapper("shout_set_public", shout_set_public(shout, pub));
}

void
Shout::SetName(const string name) {
  ErrorWrapper("shout_set_name", shout_set_name(shout, name.c_str()));
}

void
Shout::SetGenre(const string genre) {
  ErrorWrapper("shout_set_genre", shout_set_genre(shout, genre.c_str()));
}

void
Shout::SetDescription(const string descr) {
  ErrorWrapper("shout_set_description", shout_set_description(shout, descr.c_str()));
}

