# -*- makefile -*-
#
# $Id$
#
TEMPLATE = app
CONFIG += debug warn_on qt exceptions thread
INCLUDEPATH += . /usr/include/taglib /usr/local/include
LIBS += -ltag -lshout # -lefence
DEFINES += QT_FATAL_ASSERT

# Input
HEADERS	+= 	Song.h SongInfo.h SongInfo_File.h SongInfo_File_mp3.h \
		Collection.h CollectionBase.h Collection_Songlist.h \
		Collection_Songlist_File.h Collection_Songlist_Dir.h \
		Collections.h Client.h ServerSocket.h \
		StdException.h Shout.h PlayerThread.h Server.h \
		Configuration.h AccessChecker.h util.h config.h

SOURCES	+= 	main.cpp \
		Song.cpp SongInfo.cpp SongInfo_File.cpp SongInfo_File_mp3.cpp \
		CollectionBase.cpp Collection_Songlist.cpp \
		Collection_Songlist_File.cpp Collection_Songlist_Dir.h \
		Collections.cpp Client.cpp ServerSocket.cpp \
		StdException.cpp Shout.cpp PlayerThread.cpp Server.cpp \
		Configuration.cpp AccessChecker.cpp util.cpp
