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
  std::string tags[6];

  std::string find_style(int);

public:
  ID3Tag() {};

  bool InitFromFile(const std::string);
  
  std::string GetTitle() const { return tags[0]; };
  std::string GetArtist() const { return tags[1]; };
  std::string GetAlbum() const { return tags[2]; };
  std::string GetYear() const { return tags[3]; };
  std::string GetNote() const { return tags[4]; };
  std::string GetStyle() const { return tags[5]; };
};

#endif
