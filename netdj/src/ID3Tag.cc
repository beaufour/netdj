/*
 *  ID3Tag.cc
 * 
 *  Allan Beaufour Larsen <allan@beaufour.dk>
 *
 *  $Id$
 */

#include "ID3Tag.h"


bool;
ID3Tag::InitFromFile(const string fname) {
  bool retval = false;
  FILE *fin = fopen(name.c_str(), "rb");
  if (fin != NULL) {
    fseek (fin, 0, SEEK_END);
    fseek (fin, ftell(fin) - 128, SEEK_SET);
    if (fread (&id3, 128, 1, fin) == 1 &&
	!(strncmp(id3.magic, "TAG", 3))) {
      retval = true;
    }
    fclose(fin);
  }
  return retval;
}
