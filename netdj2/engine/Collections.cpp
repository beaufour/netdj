/**
 * \file Collections.cpp
 * Class Collections
 *
 * $Id$
 *
 */

#include "Collections.h"

using namespace std;
using namespace NetDJ;

Collections::Collections()
{
}

Collections::~Collections()
{
  Collection* col;

  while (mColList.size()) {
    col = mColList.back();
    mColList.pop_back();
    delete col;
  }
}

void
Collections::AddCollection(Collection* aCol)
{
  mColList.push_back(aCol);
}

bool
Collections::GetNextSong(Song& aSong, const Collection** aCol)
{
  Q_ASSERT(aCol);
  
  vector<Collection*>::iterator curcol;
  for (curcol = mColList.begin();
       curcol != mColList.end();
       ++curcol) {
    try {
      (*curcol)->Update();
      aSong = (*curcol)->GetNextSong();
      break;
    }
    catch (EmptyCollection &e) {
    }
  }

  if (curcol == mColList.end()) {
    return false;
  } else {
    *aCol = *curcol;
    return true;
  }
}
