SUBDIRS = src config util

EXTRA_DIST = \
	AUTHORS COPYING INSTALL README TODO

.PHONY:	ChangeLog

ChangeLog:
	./cvs2cl.pl --tags --branches --revisions --day-of-week --prune --fsf -U AUTHORS -W 3600

