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

 public:
  AccessConf() : Lockable() {};

  void ReadFile(const string&);
  bool IsAccessAllowed(const string&, int, string* = NULL);
};

#endif
