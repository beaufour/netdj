/**
 * \file Collection_Songlist.cpp
 * Class Collection_Songlist.
 *
 * $Id$
 *
 */

#include "Collection_Songlist.h"
#include "util.h"

using namespace std;

Collection_Songlist::Collection_Songlist(const string id, const string descr,
					 const bool isq)
  : Collection(id, descr), isQueue(isq)
{
}

Collection_Songlist::~Collection_Songlist() {

}

int
Collection_Songlist::Size() const {
  QMutexLocker locker(&mutex);

  return Songlist.size();
}

Song
Collection_Songlist::GetNextSong() {
  QMutexLocker locker(&mutex);

  if (!Songlist.size()) {
    throw EmptyCollection("Cannot get a song, collection is empty");
  }

  if (isQueue) {
    Song sng = Songlist[0];
    Songlist.pop_front();
    return sng;
  } else {
    return Songlist[get_rand(Songlist.size())];
  }
}

const Song&
Collection_Songlist::GetSong(const int pos) const {
  QMutexLocker locker(&mutex);
  
  return Songlist[pos];
}

bool
Collection_Songlist::AddSong(const Song& sng) {
  Songlist.push_back(sng);
  return true;
}

void
Collection_Songlist::Update() {
  
}
