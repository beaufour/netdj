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
    std::string password;

  public:
    User() : level(0) {};
    User(int _level, std::string _password)
      : level(_level), password(_password) {};

    void SetInfo(const int _level, const std::string _password) {
      level = _level;
      password = _password;
    }

    std::string GetPassword() const {return password;};
    int GetLevel() const {return level;};
  };

  std::map<std::string, User> users;

  Configuration *Conf;

 public:
  AccessConf(Configuration *c) : Lockable(), Conf(c) {};

  void ReadFile();
  bool IsAccessAllowed(const std::string&, int, std::string* = NULL) const;
};

#endif
