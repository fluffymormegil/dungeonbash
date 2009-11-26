# Top-level Makefile for Dungeon Bash

GAME=dungeonbash
SRCGEN=srcgen
MAJVERS=1
MINVERS=131
PATCHVERS=0
PRERELEASE=
# To make a prerelease package, define the next rule
#PRERELEASE=+pr1

# The os.mk, dirs.mk, and permissions.mk files are where configuration should
# be set up.
include os.mk
include dirs.mk
include permissions.mk

# The following definitions are used for Unixy systems where the administrator
# wants to have a shared playground. You will need to tweak them to match your
# system, and also settings in dbashcfg.hh, if you want to produce a multi-user
# install.
PLAYGROUND=$(games_vardir)/$(GAME)
PLAYGROUNDMODE=0755
GLOBALCFGDIR=$(syscfg_dir)/$(GAME)/v$(MAJVERS).$(MINVERS)
GLOBALCFGDIRMODE=0755

all:
	(cd src && make $(GAME) )

install: all
	install -D -o $(games_user) -g $(games_group) -m $(suid_bin_mode) src/$(GAME) $(DESTDIR)$(games_exec_prefix)/$(GAME)
	install -D -m $(man_mode) man/$(GAME).6 $(DESTDIR)$(man6dir)/$(GAME).6
	install -D -m $(man_mode) man/dungeonbashrc.5 $(DESTDIR)$(man5dir)/dungeonbashrc.5
	install -d -o $(games_user) -g $(games_group) -m $(PLAYGROUNDMODE) $(DESTDIR)$(PLAYGROUND)
	install -d -o $(games_user) -g $(games_group) -m $(PLAYGROUNDMODE) $(DESTDIR)$(PLAYGROUND)/save
	install -d -m $(GLOBALCFGDIRMODE) $(DESTDIR)$(GLOBALCFGDIR)

# archive builds a source code archive.
archive: distclean
	(cd .. && ln -s dungeonbash dungeonbash-$(MAJVERS).$(MINVERS).$(PATCHVERS)$(PRERELEASE) )
	(cd .. && tar cvzf dungeonbash-$(MAJVERS).$(MINVERS).$(PATCHVERS)$(PRERELEASE).tar.gz -h dungeonbash-$(MAJVERS).$(MINVERS).$(PATCHVERS)$(PRERELEASE)/* )

# distclean removes 
distclean:
	find . -name '*~' -print0 | xargs -0 rm
	find . -name '*.o' -print0 | xargs -0 rm

clean:
	(cd src && make clean)

# vim:noexpandtab:ts=8
