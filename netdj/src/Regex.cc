/*
 *  Regex.h
 * 
 *  Allan Beaufour Larsen <allan@beaufour.dk>
 *
 *  $Id$
 */

#include "Regex.h"
Regex::Regex(const string &regstr, int cflags = 0) {
  Compile(regstr, cflags);
}

void
Regex::Compile(const string &regstr, int cflags = 0) {
  regcomp(&preg, regstr.c_str(), cflags);
}

bool
Regex::Match(const string &str, vector<string> &res, int eflags = 0) {
  const int MAX_MATCH = 20;
  regmatch_t pmatch[MAX_MATCH];

  if (regexec(&preg, str.c_str(), MAX_MATCH, pmatch, eflags) == 0) {
    for (int i = 1; i < MAX_MATCH; ++i) {
      if (pmatch[i].rm_so != -1) {
	res.push_back(str.substr(pmatch[i].rm_so, pmatch[i].rm_eo));
      }
    }
    return true;
  } else {
    return false;
  }
}
