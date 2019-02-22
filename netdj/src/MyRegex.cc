/*
 *  Regex.h
 *
 *  Allan Beaufour Larsen <allan@beaufour.dk>
 *
 *  $Id$
 */

#include "MyRegex.h"

using namespace std;

MyRegex::MyRegex(const string &regstr, int cflags) {
  Compile(regstr, cflags);
}

void
MyRegex::Compile(const string &regstr, int cflags) {
  regcomp(&preg, regstr.c_str(),  cflags ? cflags : REG_EXTENDED);
}

int
MyRegex::Match(const string &str, vector<string> &res, int eflags) {
  const int MAX_MATCH = 20;
  int i;
  regmatch_t pmatch[MAX_MATCH];

  if (regexec(&preg, str.c_str(), MAX_MATCH, pmatch, eflags) == 0) {
    for (i = 1; i < MAX_MATCH; ++i) {
      if (pmatch[i].rm_so == -1) {
	break;
      }
      res.push_back(str.substr(pmatch[i].rm_so, pmatch[i].rm_eo - pmatch[i].rm_so));
    }
    return pmatch[0].rm_eo;
  } else {
    return 0;
  }
}
