Release Notes for Martin's Dungeon Bash v1.130.1 Experimental
=============================================================

These notes cover both the 1.130.0->1.130.1 bug fixes and the
1.129.1->1.130.0 amendments.

Installation
------------

The official distribution of MPRDB 1.130.1 Experimental is a source-only
gzipped tar archive.

To build the single-user version of MPRDB on Linux, simply invoke
    make all
in the source directory.

Game-mechanical Updates
-----------------------

The adventurer can now take two actions in one turn if the first action
is a normal move ending adjacent to a monster and the second is a melee
attack.  Future versions will make taking advantage of this feature
important against some monsters.

There are now five supported speeds for creatures, ranging from 1/3 to
5/3 of the player's base speed. (A sixth speed of 6/3 is on hold pending
me finding the effort to tweak the speed system.)

The adventurer now only consumes food on game ticks where normal speed
creatures get to act.

New monster spells
------------------

Chainstrike: Lashes at the adventurer with barbed chains.

Shackle: Roots the adventurer to the spot.

Infrastructure
--------------

The game now has proper support for shared playgrounds on multi-user
POSIX-style systems. This support requires the game binary to be
installed setuid a suitable user. IMPORTANT NOTE: this feature has not
been given a security audit.

The game now has support for per-user and system-wide configuration
files. The following configuration variables are supported:

System-wide

    On multi-user systems, system-wide configuration variables are read
    from the file /etc/dungeonbash/config and handled as follows:

    system_playground - specifies the directory used to store saved
    games, high score files, etc.

    always_fsync - panders to the Theodore T'sos of this world by
    allowing them to stipulate that the game should fsync() files after
    updating them. If you turn this feature on, you're a paranoid moron.

Per-user

    User configuration parameters are read from the file
    $HOME/.dunbashrc and handled as follows:

    suppress_boringfail, suppress_fluff, suppress_mon_alert,
    suppress_numeric, suppress_taunt - disable various non-essential
    message channels. [all default to false]

    name_prompt - if set, causes the game to prompt the player for their
    choice of name. [defaults to true]

    levelup_wait - if set, the game will wait for the player to press
    ENTER before resuming game computation after gaining a level.
    [Defaults to true]

    save_wait - if set, the game will wait for the player to press ENTER
    before exiting after saving the game state. [Defaults to true]

    reload_wait - if set, the game will wait for the player to press
    ENTER before resuming game computation after reloading saved state.
    [Defaults to false]

    name - specifies the default name to be used for the player's
    character.  [defaults to Ingold]

    language - specifies the player's preferred language for in-game
    messages.  Currently, only English (en) is supported, so this
    setting is ignored. [defaults to en]

    preferred_display - specifies the player's preferred display
    interface method. Doesn't currently do anything. [defaults
    to tty]

    inventory_colours - colours items in the player's inventory
    according to their official "quality". [defaults to false]

