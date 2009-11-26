prefix=/usr/local
exec_prefix=$(prefix)/bin
games_exec_prefix=$(prefix)/games
vardir=/var/local
games_vardir=/var/local/games
mandir=$(prefix)/man
man1dir=$(mandir)/man1
man2dir=$(mandir)/man2
man3dir=$(mandir)/man3
man4dir=$(mandir)/man4
man5dir=$(mandir)/man5
man6dir=$(mandir)/man6
man7dir=$(mandir)/man7
man8dir=$(mandir)/man8
syscfg_dir=/etc
# Multiuser installatiosn need the following paths defined
PLAYGROUND=$(games_vardir)/$(GAME)
GLOBALCFGDIR=$(syscfg_dir)/$(GAME)/v$(MAJVERS).$(MINVERS)
