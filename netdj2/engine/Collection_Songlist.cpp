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

Collection_Songlist::Collection_Songlist(const string aId, const string aDescr,
					 const bool aIsQ)
  : Collection(aId, aDescr), mIsQueue(aIsQ)
{
}

Collection_Songlist::~Collection_Songlist() {

}

int
Collection_Songlist::Size() const {
  QMutexLocker locker(&mMutex);

  return mSonglist.size();
}

Song
Collection_Songlist::GetNextSong() {
  QMutexLocker locker(&mMutex);

  if (!mSonglist.size()) {
    throw EmptyCollection("Cannot get a song, collection is empty");
  }

  if (mIsQueue) {
    Song sng = mSonglist[0];
    mSonglist.pop_front();
    return sng;
  } else {
    return mSonglist[get_rand(mSonglist.size())];
  }
}

const Song&
Collection_Songlist::GetSong(const int aPos) const {
  QMutexLocker locker(&mMutex);
  
  return mSonglist[aPos];
}

bool
Collection_Songlist::AddSong(const Song& aSng) {
  mSonglist.push_back(aSng);
  return true;
}

void
Collection_Songlist::Update() {
  
}
