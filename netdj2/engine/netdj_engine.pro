# -*- makefile -*-
#
# $Id$
#
TEMPLATE = app
CONFIG += debug warn_on qt exceptions thread
INCLUDEPATH += . /usr/include/taglib /usr/local/include
LIBS += -ltag -lshout -lpopt # -lefence
DEFINES += QT_FATAL_ASSERT

# Input
HEADERS	+= 	IAccessChecker.h ICollection.h ISong.h ISongInfo.h IXMLOutput.h \
		SongFile.h SongInfoBase.h SongInfoFile.h \
		CollectionBase.h Collection_Songlist.h \
		Collection_Songlist_File.h Collection_Songlist_Dir.h \
		Collections.h Client.h ServerSocket.h \
		StdException.h Shout.h PlayerThread.h Server.h \
		Configuration.h AccessChecker.h util.h config.h \
		LogService.h FileLogger.h

SOURCES	+= 	main.cpp \
		SongFile.cpp SongInfoBase.cpp SongInfoFile.cpp \
		CollectionBase.cpp Collection_Songlist.cpp \
		Collection_Songlist_File.cpp Collection_Songlist_Dir.h \
		Collections.cpp Client.cpp ServerSocket.cpp \
		StdException.cpp Shout.cpp PlayerThread.cpp Server.cpp \
		Configuration.cpp AccessChecker.cpp util.cpp \
		LogService.cpp FileLogger.cpp
