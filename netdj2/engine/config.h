/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/**
 * \file config.h
 * Global defines and variables
 *
 * $Id$
 *
 */

#ifndef __CONFIG_H__
#define __CONFIG_H__

#define NETDJ_ETCDIR "/etc"
#define NETDJ_CONF_FILENAME "netdj"

#include "Configuration.h"
#include "LogService.h"

namespace NetDJ {
  extern Configuration gConfig;
  extern LogService gLogger;
}

#endif
