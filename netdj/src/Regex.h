/*
 *  Regex.h
 * 
 *  Allan Beaufour Larsen <allan@beaufour.dk>
 *
 *  $Id$
 */

#ifndef __REGEX_H__
#define __REGEX_H__

#include <sys/types.h>
#include <regex.h>
#include <vector>
#include <string>

class Regex {
private:
  regex_t preg;

public:
  Regex(const std::string&, int = 0);

  void Compile(const std::string&, int = 0);
  int Match(const std::string&, std::vector<std::string>&, int = 0);
};

#endif
