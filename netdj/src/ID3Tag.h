/*
 *  ID3Tag.h
 * 
 *  Allan Beaufour Larsen <allan@beaufour.dk>
 *
 *  $Id$
 */

#ifndef  __ID3TAG_H__
#define __ID3TAG_H__

#include <string>

class ID3Tag {
private:
  string tags[6];

  string find_style(int);

public:
  ID3Tag() {};

  bool InitFromFile(const string);
  
  string GetTitle() const { return tags[0]; };
  string GetArtist() const { return tags[1]; };
  string GetAlbum() const { return tags[2]; };
  string GetYear() const { return tags[3]; };
  string GetNote() const { return tags[4]; };
  string GetStyle() const { return tags[5]; };
};

#endif
