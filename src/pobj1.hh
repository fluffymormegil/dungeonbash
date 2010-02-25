/* pobj1.h
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

#ifndef POBJ1_H
#define POBJ1_H

#ifndef INDIE_HH
#include "indie.hh"
#endif

enum Item_quality 
{
    Itemqual_bad,
    Itemqual_normal,
    Itemqual_good,
    Itemqual_great,
    Itemqual_excellent
};

/* XXX enum poclass_num */
/* Categories of permanent object. */
enum Poclass_num {
    POCLASS_NONE = 0,
    POCLASS_WEAPON, POCLASS_POTION,
    POCLASS_SCROLL, POCLASS_ARMOUR, POCLASS_RING,
    POCLASS_FOOD, POCLASS_MISC, POCLASS_WAND,
    POCLASS_CARRION, POCLASS_COIN,
    NUM_OF_POCLASSES
};

#define POSYM_MELEE_WEAPON ')'
#define POSYM_RANGED_WEAPON '('
#define POSYM_POTION '!'
#define POSYM_SCROLL '?'
#define POSYM_ARMOUR '['
#define POSYM_RING '='
#define POSYM_FOOD '%'
#define POSYM_MISC '*'
#define POSYM_WAND '/'
#define POSYM_CARRION '&'
#define POSYM_COIN '$'

/* Many of these flags are "reserved for future expansion". */
#define POF_WEAR_SHOES 0x00000001u /* armours your feet */
#define POF_WEAR_PANTS 0x00000002u /* armours your legs */
#define POF_WEAR_SHIRT 0x00000004u /* armours your torso */
#define POF_WEAR_GLOVES 0x00000010u /* armours your hands */
#define POF_WEAR_SHOULDERS 0x00000020u /* armours your shoulders */
#define POF_WEAR_HAT 0x00000040u /* armours your head */
#define POF_ARTIFACT 0x80000000u /* untouchable infallible divine */

struct Permobj {
    const char name[48];
    const char plural[48];
    const char description[160];
    Poclass_num poclass;
    Item_quality qual;
    int rarity;	/* Chance in 100 of being thrown away and regen'd. */
    int sym;
    int colour;
    int power;	/* AC for armour; damage for weapons; colour/title for
		 * scrolls and potions and rings and such. */
    bool known;	/* Set to 1 for items recognised at startup. Updated
		 * during play when items identified. */
    int depth;	/* If greater than 1, this item cannot be given out
		 * by get_random_pobj() before the specified depth. */
    /* The flag field is used for things like "is this an artifact?", "what is
     * this object's wearmask", etc. */
    uint32_t flags;
};

extern Permobj permobjs[];
extern const int PO_COUNT;

#define NO_POBJ (-1)
#endif

/* pobj1.h */

