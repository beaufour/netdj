/*
 *  AccessConf.h
 * 
 *  Allan Beaufour Larsen <allan@beaufour.dk>
 *
 *  $Id$
 */

#ifndef __ACCESSCONF_H__
#define __ACCESSCONF_H__

#include <map>
#include <string>
#include <fstream>
#include "Lockable.h"
#include "Configuration.h"

class AccessConf : public Lockable {
 private:
  class User {
  private:
    int level;
    string password;

  public:
    User() : level(0) {};
    User(int _level, string _password)
      : level(_level), password(_password) {};

    void SetInfo(const int _level, const string _password) {
      level = _level;
      password = _password;
    }

    string GetPassword() const {return password;};
    int GetLevel() const {return level;};
  };

  map<string, User> users;

  Configuration *Conf;

 public:
  AccessConf(Configuration *c) : Lockable(), Conf(c) {};

  void ReadFile();
  bool IsAccessAllowed(const string&, int, string* = NULL) const;
};

#endif
