# -*- makefile -*-
#
# $Id$
#
TEMPLATE = app
CONFIG += debug warn_on qt exceptions thread
INCLUDEPATH += ../engine/
DEFINES += QT_FATAL_ASSERT

# Input
HEADERS +=	INetDJClient.h NetDJClient.h \
		ILogListener.h NetLogListener.h
SOURCES	+= 	main.cpp \
		NetDJClient.cpp \
		NetLogListener.cpp
