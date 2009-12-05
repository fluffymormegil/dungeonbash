/* player.hh
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

#ifndef PLAYER_H
#define PLAYER_H

#include "perseff.hh"

#define INVENTORY_SIZE 19
/* XXX struct Player */
struct Player {
    char name[17];	/* including '\0' the fencepost. */
    libmrl::Coord pos;  /* position within the level. */
    Level_tag lev;
    Critter_sex sex;
    Player_profession job;
    int body;		/* determines mace to-hit, melee damage, max 99 */
    int bdam;		/* current level of temporary body drain. */
    int agility;	/* determines sword, dagger, missile to-hit, max 99 */
    int adam;		/* current level of temporary agility drain. */
    int hpmax;		/* Max hit points; max of 999. */
    int hpcur;		/* Current hit points; <= 0 is dead. */
    int mpmax;          /* Max mana */
    int mpcur;          /* Max mana */
    int32_t food;		/* Current nutrition in body; < 0 is hungry. */
    uint32_t experience;	/* Experience points earned. */
    int defence;	/* To-hit target number for monsters */
    int evasion;	/* To-hit target number for armour-ignoring attacks */
    int speed;
    uint32_t resistances[DT_COUNT];	/* Resistances to damage types. */
    int level;	/* Each level gets you +1 body, +1 agility, +1 random
                   point, and +(10+body/10) hit points */
    int gold;
    Obj_handle inventory[INVENTORY_SIZE];	/* 19 inventory slots, leaving room for a prompt */
    Obj_handle weapon;		/* For now, you can only wield one weapon. */
    Obj_handle armour;		/* For now, you can only wear one item of armour. */
    Obj_handle ring;		/* For now, you can only wear one magic ring. */
    bool farmoving;
    libmrl::Coord farmove_direction;
    // Persistent effect state
    std::list<Perseff_data> perseffs;
    Status_flags status;
    // Persistent effect application
    void apply_effect(Perseff_data& peff);
    bool suffer(Perseff_data& peff);
    void resolve_dispel(std::list<Perseff_data>::iterator peff_iter);

    // computed-value functions
    int net_body() const { return body - bdam; }
    int net_agility() const { return agility - adam; }
    bool test_mobility(bool noisy = false) const;
    int on_remove(bool force = false);
};

/* XXX u.c data and funcs */
extern void u_init(void);
extern void write_char_dump(void);
extern int do_death(Death d, const char *what);
extern void heal_u(int amount, int boost, int loud);
extern void disturb_u(void);
extern int damage_u(int amount, Death d, const char *what);
extern int gain_body(int amount, int loud);
extern int gain_agility(int amount, int loud);
extern int drain_body(int amount, const char *what, int permanent);
extern int drain_agility(int amount, const char *what, int permanent);
extern void gain_experience(int amount);
extern unsigned int lev_threshold(int level);
extern int farmove_player(libmrl::Coord direction);
extern int move_player(libmrl::Coord step);
extern int reloc_player(libmrl::Coord target, bool override = false);
extern void recalc_defence(void);
extern int teleport_u(void);
extern void update_player(void);
extern int player_resists_dtype(Damtyp dtype);
extern void look_at_floor(void);
extern bool player_next_to_mon(void);
extern int get_inventory_slot(Obj_handle oh);

extern Player u;

#define inyourroom(mpos) ((currlev->region_at(u.pos) != -1) && (currlev->region_at(u.pos) == currlev->region_at(mpos)))
#define couldseemon(mpos) (inyourroom((mpos)) || (u.pos.distance((mpos)) < 2))

#endif

/* player.hh */
