# -*- makefile -*-
#
# $Id$
#
TEMPLATE = app
CONFIG += debug warn_on qt exceptions thread
INCLUDEPATH += . /usr/include/taglib /usr/local/include
LIBS += -ltag -lshout -lefence

# Input
HEADERS	+= 	Song.h SongInfo.h SongInfo_File.h SongInfo_File_mp3.h \
		Collection.h Collection_Songlist.h \
		Collection_Songlist_File.h Collection_Songlist_Dir.h \
		Collections.h \
		StdException.h Shout.h PlayerThread.h HttpServer.h \
		util.h

SOURCES	+= 	main.cpp \
		Song.cpp SongInfo.cpp SongInfo_File.cpp SongInfo_File_mp3.cpp \
		Collection.cpp Collection_Songlist.cpp \
		Collection_Songlist_File.cpp Collection_Songlist_Dir.h \
		Collections.cpp \
		StdException.cpp Shout.cpp PlayerThread.cpp HttpServer.cpp \
		util.cpp
