SUBDIRS = config doc src util

EXTRA_DIST = \
	AUTHORS COPYING INSTALL README TODO gnut.patch netdj.spec gnutrc_sample

.PHONY:	ChangeLog

ChangeLog:
	./cvs2cl.pl --tags --branches --revisions --day-of-week --prune --fsf -U AUTHORS -W 3600

