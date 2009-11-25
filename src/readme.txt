Release notes for MPRDB 1.131.0 Experimental
============================================
These release notes describe changes relative to Martin's Dungeon Bash 1.130.2
Experimental.

Feature changes
---------------

+ The player character now has a "profession". Each profession has a number of
special abilities unique to that profession. The "active" abilities are
triggered using the number keys 0-9.

+ The Show 'E'quipment command now exists; this command lists the items
currently equipped by the player character.

- Movement using the number keys is no longer supported. These keys have been
reassigned as the triggers for the player character's profession-specific
abilities.

Bug fixes
---------

Levextra structures are now correctly preserved in the saved-game file.

Message printout suppresion now works correctly.

Architecture changes
--------------------

The Coord constants (the directions and NOWHERE) have been moved to the
"libmrl" namespace.

Obj_handle and Mon_handle objects now hold a 64-bit unsigned integer used as
the lookup key for a std::map. As a result, the game engine can now
accommodate more than 100 instantiated objects and 100 instantiated monsters.

The game engine can now accommodate persistent dungeon levels. Please note
that all instantiated persistent dungeon levels are held in memory during game
play.

The game engine now supports the creation and manipulation of monsters and
objects on levels other than the one pointed to by "currlev".

The game engine now handles persistent effects in a more "agnostic" way; in
principle monsters could now be made victims of (for example) the "leadfoot"
curse.

