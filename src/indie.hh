/* indie.hh
 * 
 * Copyright 2009 Martin Read
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef INDIE_HH
#define INDIE_HH

#include <stdint.h>
#include "coord.hh"

/* This file contains universal definitions for things that don't change
 * when the monster or item databases do. */

/* XXX enum damtyp - types of damage. */
enum Damtyp {
    DT_PHYS = 0, DT_COLD, DT_FIRE, DT_NECRO, DT_ELEC, DT_POISON, DT_ACID, DT_COUNT
};

/* XXX enum terrain_num */
enum Terrain_num {
    // Walls
    WALL, BONE_WALL, IRON_WALL, RED_WALL, SKIN_WALL,
    // Floors
    FLOOR, BONE_FLOOR, IRON_FLOOR, RED_FLOOR, SKIN_FLOOR,
    // Doors
    DOOR,
    // Levelportals
    STAIRS_DOWN, STAIRS_UP,
    // Fittings
    ALTAR, FURNACE, ANVIL, TOMBSTONE,
    // Hazardous terrain
    WATER_POOL, ACID_POOL, LAVA_POOL
};

/* XXX enum death */
/* Sadly, there are not yet 52 kinds of way to die, only four: killed by
 * an arbitrary string, killed by a monster, drained of body by an arbitrary
 * string, and drained of agility by an arbitrary string. */
enum Death {
    DEATH_KILLED, DEATH_KILLED_MON, DEATH_BODY, DEATH_AGILITY
};

/* XXX enum game_cmd - player actions. */
enum Game_cmd {
    NO_CMD, DROP_ITEM, SAVE_GAME, SHOW_INVENTORY,
    MOVE_WEST, MOVE_SOUTH, MOVE_NORTH, MOVE_EAST,
    MOVE_NW, MOVE_NE, MOVE_SW, MOVE_SE,
    FARMOVE_WEST, FARMOVE_SOUTH, FARMOVE_NORTH, FARMOVE_EAST,
    FARMOVE_NW, FARMOVE_NE, FARMOVE_SW, FARMOVE_SE,
    QUAFF_POTION, READ_SCROLL, ZAP_WAND, ACTIVATE_MISC, WIELD_WEAPON,
    WEAR_ARMOUR, TAKE_OFF_ARMOUR, PUT_ON_RING, REMOVE_RING, ATTACK,
    VOCALIZE_WORD, GET_ITEM, GO_UP_STAIRS, GO_DOWN_STAIRS, STAND_STILL,
    EAT_FOOD, DUMP_CHARA, INSPECT_ITEM, FARLOOK, FLOORLOOK, QUIT, GIVE_HELP,
    SHOW_TERRAIN, SHOW_EQUIPPED, SHOW_DISCOVERIES, PRINT_VERSION,
    CHAT_WITH,
    PROFCMD_0, PROFCMD_1, PROFCMD_2, PROFCMD_3, PROFCMD_4,
    PROFCMD_5, PROFCMD_6, PROFCMD_7, PROFCMD_8, PROFCMD_9,
    // Commands past this point are intended for debugging use.
    RNG_TEST, WIZARD_LEVELUP, WIZARD_DESCEND, WIZARD_TELEPORT,
    WIZARD_DUMP_PERSEFFS, WIZARD_CURSE_ME
};

inline bool was_move_command(int i)
{
    return (i >= MOVE_WEST) && (i <= FARMOVE_SE);
}

#define is_vowel(ch) (((ch) == 'a') || ((ch) == 'e') || ((ch) == 'i') || ((ch) == 'o') || ((ch) == 'u'))

#endif

/* Player resistance masks */

#define RESIST_MASK_TEMPORARY	0x0000FFFFu
#define RESIST_MASK_PERM_EQUIP	0xFFFF0000u
#define RESIST_RING	0x00010000u
#define RESIST_ARMOUR	0x00020000u

/* Game-internal colour codes */

enum Dbash_colour
{
    DBCLR_L_GREY, DBCLR_D_GREY, DBCLR_RED, DBCLR_BLUE,
    DBCLR_GREEN, DBCLR_PURPLE, DBCLR_BROWN, DBCLR_CYAN,
    DBCLR_WHITE, DBCLR_L_RED, DBCLR_L_BLUE, DBCLR_L_GREEN,
    DBCLR_L_PURPLE, DBCLR_YELLOW, DBCLR_L_CYAN
};

enum Creature_speed
{
    SPEED_VERY_SLOW, SPEED_SLOW, SPEED_NORMAL, SPEED_FAST,
    SPEED_VERY_FAST, SPEED_ULTRAFAST
};

struct Direction_data
{
    libmrl::Coord delta;
    libmrl::Coord sign;
    bool meleerange;
    bool oncardinal;
};

enum Dungeon_num
{
    Dungeon_main, Dungeon_necropolis, Dungeon_inferno,
    Dungeon_torment, Dungeon_iron_halls, Dungeon_hellmire,
    Total_dungeons
};

enum Player_profession
{
    Prof_fighter, Prof_preacher, Prof_thanatophile,
    Total_professions
};

enum Critter_sex
{
    Csex_neuter, Csex_female, Csex_male, Csex_other
};

// for now I am keeping this to major-body-part anatomy. Organs can wait for a
// highly hypothetical "gorebash" or "hentaibash" fork.
enum Anatomy_bits
{
    Anat_head, Anat_torso, // Fundamentals of meaty critters
    Anat_left_leg, Anat_right_leg, // bipeds
    Anat_LF_leg, Anat_RF_leg, Anat_LR_leg, Anat_RR_leg, // quadruped legs
    Anat_left_arm, Anat_right_arm, // manuals
    Anat_left_wing, Anat_right_wing // avians, chiropterans, etc.
};

enum Caster_fluff
{
    Cfluff_curse, Cfluff_incant, Total_caster_fluffs
};

typedef uint32_t Anatomy;
#define anatomy_mask(i) (1 << i)

#define COMPASS_POINTS  8
/* indie.h */
