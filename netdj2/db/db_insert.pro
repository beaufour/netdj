# -*- makefile -*-
#
# $Id$
#
TEMPLATE = app
CONFIG += debug warn_on qt exceptions
INCLUDEPATH += . /usr/include/taglib /usr/local/include
LIBS += -ltag -lmysqlclient
DEFINES += QT_FATAL_ASSERT

# Input
SOURCES	+= 	main.cpp
