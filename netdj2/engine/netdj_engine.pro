# -*- makefile -*-
#
# $Id$
#
TEMPLATE = app
CONFIG += debug warn_on qt exceptions
INCLUDEPATH += .
LIBS += -lid3 -lshout -lefence

# Input
HEADERS	+= 	Song.h SongInfo.h SongInfo_mp3.h \
		Collection.h Collection_Songlist.h \
		Collection_Songlist_File.h Collection_Songlist_Dir.h \
		StdException.h Shout.h PlayerThread.h HttpServer.h \
		util.h

SOURCES	+= 	main.cpp \
		Song.cpp SongInfo.cpp SongInfo_mp3.cpp \
		Collection.cpp Collection_Songlist.cpp \
		Collection_Songlist_File.cpp Collection_Songlist_Dir.h \
		StdException.cpp Shout.cpp PlayerThread.cpp HttpServer.cpp \
		util.cpp
