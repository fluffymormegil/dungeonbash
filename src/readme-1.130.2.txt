Release Notes for Martin's Dungeon Bash v1.130.2 Experimental
=============================================================

These notes cover the changes in 1.130.2 relative to 1.130.1.

Installation
------------

The official distribution of MPRDB 1.130.2 Experimental is a source-only
gzipped tar archive.

To build the single-user version of MPRDB on Linux, simply invoke
    make all
in the source directory.

Bug fixes
---------

1) There is now (minimal) checking for malformed saved-game files.

2) The released Makefile does not set -Werror in the flags to GCC.

3) The game will not crash if a bad monster handle is passed to teleport_mon.

Feature adjustments
-------------------

1) Imps may now teleport away after striking the player. (A general review of
   to-hit mechanics is required, but will have to wait.)

