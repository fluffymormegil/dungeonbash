/* pmon1.h
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

#ifndef PMON1_HH
#define PMON1_HH

#ifndef INDIE_HH
#include "indie.hh"
#endif

#define PMF_RESIST_FIRE 0x00000001
#define PMF_RESIST_COLD 0x00000002
#define PMF_RESIST_ELEC 0x00000004
#define PMF_RESIST_ACID 0x00000008
#define PMF_RESIST_POIS 0x00000010
#define PMF_RESIST_NECR 0x00000020
#define PMF_ALWAYS_OTHER      0x00001000
#define PMF_ALWAYS_FEMALE     0x00002000
#define PMF_ALWAYS_MALE       0x00004000
#define PMF_ALWAYS_NEUTER     0x00008000
#define PMF_UNDEAD      0x00010000
#define PMF_DEMONIC     0x00020000
#define PMF_ETHEREAL    0x00040000
#define PMF_GDEMON      0x00080000 /* Greater demons get generated names */
#define PMF_MEATY       0x00100000 /* Not just corporeal, but MADE OF MEAT */
#define PMF_SKELETAL    0x00200000 /* Made of bones */
#define PMF_NOCORPSE    0x00400000 /* Leaves neither corpse nor bones */
#define PMF_MAGICIAN    0x01000000
#define PMF_ARCHER      0x02000000
#define PMF_BREATHER    0x04000000
#define PMF_ARMED       0x08000000 /* uses a weapon */
#define PMF_SMART       0x10000000
#define PMF_STUPID      0x20000000
#define PMF_TELE_HARASS 0x40000000 /* may teleport away after hitting player */
#define PMF_PEACEFUL    0x80000000 /* never acts */

enum Pmon_species
{
    Species_own,
    Species_human,
    Species_elf,
    Species_giant
};

enum Mon_maux
{
    Maux_none, // is zero rather than -1 because it makes my life easier
    Maux_poison_body,
    Maux_poison_agil,
    Maux_drain_body,
    Maux_drain_agil,
    Maux_drink_blood,
    Maux_fever,
    Maux_decay,
    Maux_shieldbreaker,
    Maux_hentacle,
    Maux_ignite,
    Maux_total
};

enum Mon_raux
{
    Raux_none,
    Raux_poison_body,
    Raux_poison_agil,
    Raux_drain_body,
    Raux_drain_agil,
    Raux_fever,
    Raux_total
};

struct Mon_mattk
{
    int acc;
    int dam;
    int auxchance; // out of 100
    Mon_maux auxtyp;
    int aux_strength;
};

struct Mon_rattk
{
    int acc;
    int dam;
    Damtyp dt;
    const char *verb;
    int auxchance;
    Mon_raux auxtyp;
    int aux_strength;
};

#define PMSYM_LESSER_DEMON '4'
#define PMSYM_MIDDLE_DEMON '3'
#define PMSYM_GREATER_DEMON '2'
#define PMSYM_ARCHDEVIL '1'
#define PMSYM_HUMAN '@'
#define PMSYM_CANINE 'c'
#define PMSYM_ELF 'e'
#define PMSYM_FIGHTER 'f'
#define PMSYM_GOBLIN 'g'
#define PMSYM_HUNTER 'h'
#define PMSYM_NEWT 'n'
#define PMSYM_PRIEST 'p'
#define PMSYM_RODENT 'r'
#define PMSYM_SNAKE 's'
#define PMSYM_THUG 't'
#define PMSYM_WIZARD 'w'
#define PMSYM_ZOMBIE 'z'
#define PMSYM_CENTAUR 'C'
#define PMSYM_DRAGON 'D'
#define PMSYM_GIANT 'H'
#define PMSYM_ICE_MONSTER 'I'
#define PMSYM_LICH 'L'
#define PMSYM_TROLL 'T'
#define PMSYM_VAMPIRE 'V'
#define PMSYM_WRAITH 'W'

struct Permon {
    const char *name;
    const char *plural;
    const char *description;
    Pmon_species species;
    char sym;
    int colour;
    unsigned rarity;         /* Chance in 100 of being thrown back and regen'd. */
    unsigned power;          /* Used to determine OOD rating. */
    /* All OOD-improved stats cap out at base + (power * base) */
    int hp;             /* Improved by OOD rating at 1:1. */
    Mon_mattk melee;
    Mon_rattk ranged;
    int defence;        /* Improved by OOD rating at 1:3. */
    int armour;
    int exp;            /* Unaffected by OOD rating. */
    Creature_speed speed;          /* 0 = slow; 1 = normal; 2 = quick */
    uint32_t flags;          /* resistances, AI settings, etc. */
};

extern Permon permons[];
extern const int PM_COUNT;

#define NO_PM (-1)

#endif

/* pmon1.h */

