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

  while (ColList.size()) {
    col = ColList.back();
    ColList.pop_back();
    delete col;
  }
}

void
Collections::AddCollection(Collection* col) {
  ColList.push_back(col);
}

bool
Collections::GetNextSong(Song& song, std::string& colid) {
  vector<Collection*>::iterator curcol;
  for (curcol = ColList.begin();
       curcol != ColList.end();
       ++curcol) {
    try {
      (*curcol)->Update();
      song = (*curcol)->GetNextSong();
      break;
    }
    catch (EmptyCollection &e) {
    }
  }

  if (curcol == ColList.end()) {
    return false;
  } else {
    colid = (*curcol)->GetIdentifier();
    return true;
  }
}
