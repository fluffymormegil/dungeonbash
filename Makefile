# Top-level Makefile for Dungeon Bash

GAME=dungeonbash
SRCGEN=srcgen
MAJVERS=1
MINVERS=131
PATCHVERS=0
PRERELEASE=
# To make a prerelease package, define the next rule
#PRERELEASE=+pr1
ARCHIVEDIR=$(GAME)-$(MAJVERS).$(MINVERS).$(PATCHVERS)$(PRERELEASE)
ARCHIVEMANIFEST=Makefile *.mk README man src html share skel

# The os.mk, dirs.mk, and permissions.mk files are where configuration should
# be set up.
include os.mk
include dirs.mk
include permissions.mk
include features.mk

all:
	(cd src && make $(GAME)$(EXECUTABLE_SUFFIX) )

install: all
	install -D -o $(games_user) -g $(games_group) -m $(suid_bin_mode) src/$(GAME)$(EXECUTABLE_SUFFIX) $(DESTDIR)$(games_exec_prefix)/$(GAME)$(EXECUTABLE_SUFFIX)
	install -D -m $(man_mode) man/$(GAME).6 $(DESTDIR)$(man6dir)/$(GAME).6
	install -D -m $(man_mode) man/dungeonbashrc.5 $(DESTDIR)$(man5dir)/dungeonbashrc.5
	install -d -o $(games_user) -g $(games_group) -m $(PLAYGROUNDMODE) $(DESTDIR)$(PLAYGROUND)
	install -d -o $(games_user) -g $(games_group) -m $(PLAYGROUNDMODE) $(DESTDIR)$(PLAYGROUND)/save
	install -d -m $(GLOBALCFGDIRMODE) $(DESTDIR)$(GLOBALCFGDIR)

# srcarch builds a source code archive suitable for distribution
srcarch: distclean
	mkdir $(ARCHIVEDIR)
	cp -R $(ARCHIVEMANIFEST) $(ARCHIVEDIR)
	tar cvzf $(ARCHIVEDIR).tar.gz $(ARCHIVEDIR)
	-rm -rf $(ARCHIVEDIR)

# deb builds the packages dungeonbash and dungeonbash-data
deb: 

# distclean tidies up all generated files.
distclean:
	(cd src && make distclean)
	-rm -rf $(ARCHIVEDIR)

clean:
	(cd src && make clean)

# vim:noexpandtab:ts=8
