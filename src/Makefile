# Makefile for Dungeon Bash

OBJS=astar.o bmagic.o cfgfile.o cloud.o combat.o display.o hexpi.o loadsave.o main.o map.o misc.o mon1.o mon2.o mon3.o obj1.o obj2.o permobj.o permons.o pmon2.o radiance.o rng.o rooms.o spiralpath.o u.o vector.o vision.o

GAME=dungeonbash
SRCGEN=srcgen
MAJVERS=1
MINVERS=131
PATCHVERS=0

# The following definitions are used for Unixy systems where the administrator
# wants to have a shared playground. You will need to tweak them to match your
# system, and also settings in dbashcfg.hh, if you want to produce a multi-user
# install.
include dirs.mk
include permissions.mk
PLAYGROUND=$(games_vardir)/$(GAME)
PLAYGROUNDMODE=0755
GLOBALCFGDIR=$(syscfg_dir)/$(GAME)/v$(MAJVERS).$(MINVERS)
GLOBALCFGDIRMODE=0755

# If you are using a compiler other than g++, or building against a C runtime
# other than (e)glibc, you may well need to review the compiler options in
# CXXFLAGS.

CXXFLAGS=-c -g -Wall -Wwrite-strings -Wredundant-decls -DMAJVERS=$(MAJVERS) \
	-DMINVERS=$(MINVERS) -DPATCHVERS=$(PATCHVERS) -D_POSIX_SOURCE \
	-DPLAYGROUND=\"$(PLAYGROUND)\" -DGLOBALCFGDIR=\"$(GLOBALCFGDIR)\"
LINKFLAGS=-lpanel -lncurses -g

all: $(GAME)

install: all
	install -D -o $(games_user) -g $(games_group) -m $(suid_bin_mode) $(GAME) $(DESTDIR)$(games_exec_prefix)/$(GAME)
	install -D -m $(man_mode) man/$(GAME).6 $(DESTDIR)$(man6dir)/$(GAME).6
	install -D -m $(man_mode) man/dungeonbashrc.5 $(DESTDIR)$(man5dir)/dungeonbashrc.5
	install -d -o $(games_user) -g $(games_group) -m $(PLAYGROUNDMODE) $(DESTDIR)$(PLAYGROUND)
	install -d -o $(games_user) -g $(games_group) -m $(PLAYGROUNDMODE) $(DESTDIR)$(PLAYGROUND)/save
	install -d -m $(GLOBALCFGDIRMODE) $(DESTDIR)$(GLOBALCFGDIR)

$(GAME): $(OBJS)
	$(CXX) $(OBJS) $(LINKFLAGS) -o $(GAME)

$(SRCGEN): srcgen.o pmparse.o poparse.o
	$(CXX) $^ -o $@

archive: distclean
	(cd .. && tar cvzf dungeonbash-$(MAJVERS).$(MINVERS).$(PATCHVERS).tar.gz -h dungeonbash-$(MAJVERS).$(MINVERS).$(PATCHVERS)/ )

distclean: clean
	-rm -f dunbash.log msglog.txt

clean:
	-rm -f *.o $(GAME) $(SRCGEN) dunbash.sav.gz permons.cc pmonid.hh permobj.cc pobjid.hh core spiralpath.cc

astar.o: astar.cc monsters.hh dunbash.hh dbashcfg.hh

bmagic.o: bmagic.cc dunbash.hh bmagic.hh pmonid.hh pobjid.hh dbashcfg.hh

cfgfile.o: cfgfile.cc cfgfile.hh

combat.o: combat.cc combat.hh dunbash.hh pmonid.hh pobjid.hh dbashcfg.hh

display.o: display.cc dunbash.hh dbashcfg.hh pobjid.hh pmonid.hh

loadsave.o: loadsave.cc dunbash.hh monsters.hh objects.hh rooms.hh player.hh loadsave.hh

main.o: main.cc combat.hh dunbash.hh monsters.hh pobjid.hh dbashcfg.hh

map.o: map.cc dunbash.hh dbashcfg.hh

mon1.o: mon1.cc dunbash.hh monsters.hh pmonid.hh pobjid.hh dbashcfg.hh pmonid.hh

mon2.o: mon2.cc dunbash.hh bmagic.hh monsters.hh dbashcfg.hh pmonid.hh

mon3.o: mon3.cc dunbash.hh monsters.hh dbashcfg.hh pmonid.hh

obj1.o: obj1.cc dunbash.hh objects.hh pobjid.hh dbashcfg.hh

obj2.o: obj2.cc  dunbash.hh objects.hh pobjid.hh pmonid.hh dbashcfg.hh

permobj.o: permobj.cc indie.hh dbashcfg.hh

permobj.cc pobjid.hh: permobj.txt $(SRCGEN)
	./$(SRCGEN) -o permobj.txt

permons.o: permons.cc dunbash.hh dbashcfg.hh

permons.cc pmonid.hh: permons.txt $(SRCGEN)
	./$(SRCGEN) -m permons.txt

pmon2.o: pmon2.cc dunbash.hh monsters.hh dbashcfg.hh pmonid.hh

rooms.o: rooms.cc dunbash.hh monsters.hh objects.hh pobjid.hh pmonid.hh rooms.hh

spiralpath.o: spiralpath.cc

u.o: u.cc combat.hh dunbash.hh pmonid.hh pobjid.hh player.hh dbashcfg.hh

vector.o: vector.cc dunbash.hh dbashcfg.hh

vision.o: vision.cc dunbash.hh player.hh monsters.hh dbashcfg.hh

srcgen.o: srcgen.cc pobj1.hh pmon1.hh dbashcfg.hh

spiralpath.cc: $(SRCGEN)
	./$(SRCGEN) -s

# vim:noexpandtab:ts=8
