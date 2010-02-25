Release notes for MPRDB 1.131.0 Experimental
============================================
These release notes describe changes relative to Martin's Dungeon Bash 1.130.2
Experimental.

Feature changes
---------------

+ The player character now has a "profession". Each profession has a number of
special abilities unique to that profession. The "active" abilities are
triggered using the number keys 0-9. At present, only the Fighter profession
is supported.

+ The Show 'E'quipment command now exists; this command lists the items
currently equipped by the player character.

- Movement using the number keys is no longer supported. These keys have been
reassigned as the triggers for the player character's profession-specific
abilities.

+ Pants!

Bug fixes
---------

Levextra structures are now correctly preserved in the saved-game file.

Message printout suppresion now works correctly.

Architecture changes
--------------------
Too many to conveniently enumerate. Thanks are due to Stefan O'Rear for his
significant improvements to the display/UI architecture.

