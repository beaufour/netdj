/*
 *  ID3Tag.h
 * 
 *  Allan Beaufour Larsen <allan@beaufour.dk>
 *
 *  $Id$
 */

#ifndef  __ID3TAG_H__
#define __ID3TAG_H__

#include <cstdio>

class ID3Tag {
private:
  string title;
  string artist;
  string album;
  string year;
  string note;
  string style;

public:
  ID3Tag();

  InitFromFile(const string);
  
  string GetTitle() const { return title; };
  string GetArtist() const { return artist; };
  string GetAlbum() const { return album; };
  string GetYear() const { return year; };
  string GetNote() const { return note; };
  string GetStyle() const { return style; };
};

#endif
