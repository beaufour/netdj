/**
 * \file Collections.cpp
 * Class Collections
 *
 * $Id$
 *
 */

#include "Collections.h"
#include "ICollection.h"

#include <qglobal.h>

using namespace std;
using namespace NetDJ;

Collections::Collections()
{
}

Collections::~Collections()
{
  ICollection* col;

  while (mColList.size()) {
    col = mColList.back();
    mColList.pop_back();
    delete col;
  }
}

void
Collections::AddCollection(ICollection* aCol)
{
  mColList.push_back(aCol);
}

bool
Collections::GetNextSong(ISong** aSong, const ICollection** aCol)
{
  Q_ASSERT(aCol);
  Q_ASSERT(aSong);
  
  vector<ICollection*>::iterator curcol;
  for (curcol = mColList.begin();
       curcol != mColList.end();
       ++curcol) {
    try {
      (*curcol)->Update();
      *aSong = (*curcol)->GetNextSong();
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
