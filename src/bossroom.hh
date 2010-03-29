/* bossroom.hh - Boss rooms for Martin's Dungeon Bash
 * 
 * Copyright 2009-2010 Martin Read
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

#ifndef BOSSROOM_HH
#define BOSSROOM_HH

#ifndef MONSTERS_HH
#include "monsters.hh"
#endif

#ifndef ROOMS_HH
#include "rooms.hh"
#endif

struct Boss_spec
{
    Terrain_num x_terrain;
    Terrain_num hash_terrain;
    Terrain_num dot_terrain;
    Terrain_num tilde_terrain;
    int num_pmons[10];
    int num_pobjs[10];
    Terrain_num num_terrs[10];
};

struct Bossroom
{
    Level_tag parent;
    int region_num;
    Mon_handle boss_mon;
    std::set<Mon_handle> add_mons;
    std::set<Obj_handle> decor_objs;
    virtual void excavate() = 0;
    // base_rset deletes monsters and corpses.
    void base_reset();
    // extended_reset 
    void (*extended_reset)();
};

struct Levext_rooms_boss : public Levext_rooms
{
    enum Boss_flavour
    {
        Boss_lord, // Physical combatant with bodyguards
        Boss_sculptor, // Caster who animates golems during fight
        Total_bosses
    };
    Mon_handle boss;
    std::set<Mon_handle> guards;
    virtual void add_random_room(int yseg, int xseg);
    virtual void excavate_zoo_room(void);
    virtual int leave_region(libmrl::Coord c);
    virtual int enter_region(libmrl::Coord c);
    virtual void populate_zoo_room(void);
};

#endif

/* bossroom.hh */
