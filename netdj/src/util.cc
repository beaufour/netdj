/*
 *  util.cc
 * 
 *  Allan Beaufour Larsen <allan@beaufour.dk>
 *
 *  $Id$
 */

#include "util.h"


// Provides rand, srand and system
#include <cstdlib>
#include <iostream>

using namespace std;

int
get_rand(const int max) {
  return (int) ((float) max * rand() / (RAND_MAX + 1.0));
}

void
error(const char* errmsg, const bool isfatal) {
  std::cout << "  ARGH: " << errmsg << std::endl;
  if (isfatal) {
    exit(100);
  }
}

// I'm not accountable, I stole this :)
string
base64_decode(const char *str) {
  const unsigned char alphabet[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  char inalphabet[256], decoder[256];
  int i, bits, char_count;
  string res;
  const char *ch;
  
  for (i = (sizeof alphabet) - 1; i >= 0 ; i--) {
    inalphabet[alphabet[i]] = 1;
    decoder[alphabet[i]] = i;
  }

  char_count = 0;
  bits = 0;
  for (ch = str; *ch; ++ch) {
    if (*ch == '=')
      break;
    if (! inalphabet[*ch])
      continue;
    bits += decoder[*ch];
    char_count++;
    if (char_count == 4) {
      res += (bits >> 16);
      res += ((bits >> 8) & 0xff);
      res += (bits & 0xff);
      bits = 0;
      char_count = 0;
    } else {
      bits <<= 6;
    }
  }
//    if (!*ch) {
//      if (char_count) {
      
//        cout << "  Base64 encoding incomplete: at least "
//  	   << ((4 - char_count) * 6)
//  	   << " bits truncated" << endl;
//        errors++;
//      }
//    } else { /* *ch == '=' */
    switch (char_count) {
//      case 1:
//        cout << "  Base64 encoding incomplete: at least 2 bits missing" << endl;
//        errors++;
//        break;
    case 2:
      res += (bits >> 10);
      break;
    case 3:
      res += (bits >> 16);
      res += ((bits >> 8) & 0xff);
      break;
    }
//    }
  return res;
}
