/* monsters.hh
 * 
 * Copyright 2005-2009 Martin Read
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

#ifndef MONSTERS_HH
#define MONSTERS_HH

struct Mon;

#ifndef DUNBASH_HH
#include "dunbash.hh"
#endif

#ifndef PMON1_HH
#include "pmon1.hh"
#endif

#ifndef PMON2_HH
#include "pmon2.hh"
#endif

#ifndef ASTAR_HH
#include "astar.hh"
#endif

#include <string>
#include <map>
#include <stdint.h>

enum Ai_override
{
    AI_OVER_NONE, AI_OVER_STUPEFIED, AI_OVER_RAGING, AI_OVER_SMARTENED,
    TOTAL_AI_OVERS
};

enum Ai_mode
{
    AI_BANZAI, AI_ARCHER, AI_DODGER, AI_DRUNK, AI_STALK, AI_CHASE,
    TOTAL_AI_MODES
};

#define NO_MONSTER Mon_handle(0ull)

struct Mon_handle
{
    uint64_t value;
    Mon *snapv() const;
    Mon const *snapc() const;
    Mon_handle(uint64_t v = 0ull) : value(v) { }
    bool operator <(const Mon_handle& m) const { return value < m.value; }
    bool operator ==(const Mon_handle& m) const { return value == m.value; }
    bool valid() const { return value; }
};

#ifndef PERSEFF_HH
#include "perseff.hh"
#endif


/* XXX struct Mon */
struct Mon
{
    int mon_id;
    Mon_handle self;
    Level_tag lev;
    libmrl::Coord pos;
    libmrl::Coord ai_lastpos;
    int used;
    int hpmax;	/* Improved by OOD rating at 1:1. */
    int hpcur;	/* <= 0 is dead. */
    int mtohit;	/* Improved by OOD rating at 1:3. */
    int rtohit;	/* Improved by OOD rating at 1:3. */
    int defence;	/* Improved by OOD rating at 1:3. */
    int mdam;	/* Improved by OOD rating at 1:5. */
    int rdam;	/* Improved by OOD rating at 1:5. */
    int awake;
    int meta;
    Critter_sex sex;

    char *name;

    Astar_path *current_path;
    std::list<Perseff_data> perseffs;
    Status_flags status;

    Mon() : used(0), name(0), current_path(0) { }
    ~Mon() { if (name) delete[] name; if (current_path) delete current_path; }

    void find_astar_path(libmrl::Coord c);
    void discard_path();

    bool can_pass(libmrl::Coord c) const;
    bool will_pass(libmrl::Coord c) const
    {
        /* for now, the two are equivalent. */
        return can_pass(c);
    }
    bool in_fov() const;
    void get_name(std::string *dest, int article = 0, bool shortname = false) const;
    void curses() const;
    void incants() const;
    void notice_you(bool quiet = false);
    bool suffer(Perseff_data& peff); // true = mon dead
    bool apply_effect(Perseff_data& peff, int power, int duration);
};

#define AI_GUESS_RANGE 10
#define MAX_MONSTERS 100
extern std::map<uint64_t, Mon *> monsters;
extern uint64_t next_mon_handle;

inline Mon *Mon_handle::snapv() const
{
    std::map<uint64_t, Mon *>::iterator iter = monsters.find(value);
    return (iter == monsters.end()) ? 0 : iter->second;
}
inline Mon const *Mon_handle::snapc() const
{
    std::map<uint64_t, Mon *>::iterator iter = monsters.find(value);
    return (iter == monsters.end()) ? 0 : iter->second;
}

/* XXX monsters.cc data and funcs */
extern void monsters_init(void);
extern void update_mon(Mon_handle mon);
extern void mon_acts(Mon_handle mon);
extern void death_drop(Mon_handle mon);
extern void print_mon_name(Mon_handle mon, int article, bool shortname = false);
extern void summon_mon_near(int pm_idx, libmrl::Coord pos, Level *lptr = 0);
extern Mon_handle create_mon(int pm_idx, libmrl::Coord pos, Level *lptr = 0);
extern int summoning(libmrl::Coord c, int how_many, Level *lptr = 0);
extern int ood(int power, int ratio);
extern int get_random_pmon(int depth);
extern bool damage_mon(Mon_handle mon, int amount, int by_you);
extern bool mon_visible(Mon_handle mon);
extern void move_mon(Mon_handle mon, libmrl::Coord pos, Level *lptr = 0);
extern int teleport_mon(Mon_handle mon);	/* Randomly relocate monster. */
extern int teleport_mon_to_you(Mon_handle mon);	/* Relocate monster to your vicinity. */
extern void heal_mon(Mon_handle mon, int amount, int cansee);
extern libmrl::Coord get_mon_scatter(libmrl::Coord pos, Level *lptr = 0);
extern void release_monster(Mon_handle mon);
extern void aggravate_monsters(Level *lptr = 0);

/* XXX mon2.cc data and funcs */
extern void select_space(int *py, int *px, int dy, int dx, int selection_mode);

/* XXX mon3.cc data and funcs */
extern int weaker_demon(int pm_num);

/* XXX astar.cc funcs that explicitly depend on Mon */
Astar_path *find_astar_path(Mon *mptr, libmrl::Coord goal);
libmrl::Coord astar_advance(Mon *mptr);

#endif

/* monsters.hh */
