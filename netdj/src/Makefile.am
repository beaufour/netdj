READLINE_LIB =
HISTORY_LIB =

bin_PROGRAMS = netdj

netdj_LDADD = $(READLINE_LIB) $(HISTORY_LIB) -lshout

netdj_SOURCES = main.cc \
		util.cc util.o \
		Regex.cc Regex.h \
		Configuration.cc Configuration.h \
		Directory.cc Directory.h \
		AccessConf.cc AccessConf.h \
		Lockable.cc Lockable.h \
		File.cc File.h \
		ID3Tag.cc ID3Tag.h
