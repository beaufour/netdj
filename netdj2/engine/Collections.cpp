/**
 * \file Collections.cpp
 * Class Collections
 *
 * $Id$
 *
 */

#include "Collections.h"

using namespace std;

Collections::Collections() {

}

Collections::~Collections() {
  Collection* col;

  while (mColList.size()) {
    col = mColList.back();
    mColList.pop_back();
    delete col;
  }
}

void
Collections::AddCollection(Collection* aCol) {
  mColList.push_back(aCol);
}

bool
Collections::GetNextSong(Song& song, std::string& aColId) {
  vector<Collection*>::iterator curcol;
  for (curcol = mColList.begin();
       curcol != mColList.end();
       ++curcol) {
    try {
      (*curcol)->Update();
      song = (*curcol)->GetNextSong();
      break;
    }
    catch (EmptyCollection &e) {
    }
  }

  if (curcol == mColList.end()) {
    return false;
  } else {
    aColId = (*curcol)->GetIdentifier();
    return true;
  }
}
