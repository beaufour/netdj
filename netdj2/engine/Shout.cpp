/**
 * \file Shout.cpp
 * Class Shout.
 *
 * $Id$
 *
 */

#include "Shout.h"

using namespace std;


ShoutException::ShoutException(const string aMsg)
  : StdException(aMsg) {
  
}


Shout_Metadata::Shout_Metadata() {
  mMetaData = shout_metadata_new();
  if (!mMetaData) {
    throw ShoutException("[shout_metadata_new] Could not create new metadata_t!");
  }
}

Shout_Metadata::~Shout_Metadata() {
  if (mMetaData) {
    shout_metadata_free(mMetaData);
  }
}

void
Shout_Metadata::Add(const string aName, const string aVal) {
  if (shout_metadata_add(mMetaData, aName.c_str(), aVal.c_str()) != SHOUTERR_SUCCESS) {
    throw ShoutException("[shout_metadata_add] Could not add metadata '" + aName + "' = '"+ aVal +"'");
  }
}

void
Shout::Init() {
  shout_init();

  mShout = shout_new();
  if (!mShout) {
    throw ShoutException("[shout_new] Could not create shout_t!");
  }
}

Shout::Shout()
  : mShout(0)
{
  Init();
}

Shout::Shout(const string aName, const string aGenre,
	     const string aDescr)
  : mShout(0)
{

  Init();

  SetName(aName);
  SetGenre(aGenre);
  SetDescription(aDescr);
}

Shout::~Shout() {
  if (mShout) {
    shout_close(mShout);
    shout_free(mShout);
  }
  shout_shutdown();
}

inline void
Shout::ErrorWrapper(const string aFunc, const int aErr) {
  if ((aErr) != SHOUTERR_SUCCESS) {
    throw ShoutException("[" + aFunc + "] " + shout_get_error(mShout));
  } 
}

void
Shout::Connect() {
  ErrorWrapper("shout_open", shout_open(mShout));
}

void
Shout::Disconnect() {
  ErrorWrapper("shout_close", shout_close(mShout));
}

void
Shout::SetSongName(const string aName) {
  Shout_Metadata meta;
  meta.Add("song", aName.c_str());

  ErrorWrapper("[shout_set_metadata]", shout_set_metadata(mShout, meta.mMetaData));
}

void
Shout::Send(const unsigned char *aData, size_t aLen) {
  ErrorWrapper("shout_send", shout_send(mShout, aData, aLen));
}

void
Shout::Sleep() throw() {
  shout_sync(mShout);
}

int
Shout::GetDelay() throw() {
  return shout_delay(mShout);
}

void
Shout::SetHost(const string aHost) {
  ErrorWrapper("shout_set_host", shout_set_host(mShout, aHost.c_str()));
}

void
Shout::SetPort(const unsigned short aPort) {
  ErrorWrapper("shout_set_port", shout_set_port(mShout, aPort));
}

void
Shout::SetMount(const string aMount) {
  ErrorWrapper("shout_set_mount", shout_set_mount(mShout, aMount.c_str()));

}

void
Shout::SetUser(const string aUserName) {
  ErrorWrapper("shout_set_user", shout_set_user(mShout, aUserName.c_str()));
}

void
Shout::SetPassword(const string aPass) {
  ErrorWrapper("shout_set_password", shout_set_password(mShout, aPass.c_str()));
}

void
Shout::SetFormat(const unsigned int aFormat) {
  ErrorWrapper("shout_set_format", shout_set_format(mShout, aFormat));
}

void
Shout::SetProtocol(const unsigned int aProt) {
  ErrorWrapper("shout_protocol", shout_set_protocol(mShout, aProt));
}

void
Shout::SetPublic(const bool aPublic) {
  ErrorWrapper("shout_set_public", shout_set_public(mShout, aPublic));
}

void
Shout::SetName(const string aName) {
  ErrorWrapper("shout_set_name", shout_set_name(mShout, aName.c_str()));
}

void
Shout::SetGenre(const string aGenre) {
  ErrorWrapper("shout_set_genre", shout_set_genre(mShout, aGenre.c_str()));
}

void
Shout::SetDescription(const string aDescr) {
  ErrorWrapper("shout_set_description", shout_set_description(mShout, aDescr.c_str()));
}

