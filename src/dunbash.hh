/* dunbash.hh
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

#ifndef DUNBASH_HH
#define DUNBASH_HH

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "dbashcfg.hh"

#ifndef COORD_HH
#include "coord.hh"
#endif

#ifndef INDIE_HH
#include "indie.hh"
#endif

#ifndef POBJ1_HH
#include "pobj1.hh"
#endif

#ifndef PMON1_HH
#include "pmon1.hh"
#endif

enum Astar_states
{
    ASTAR_UNCONSIDERED,
    ASTAR_OPEN,
    ASTAR_CLOSED,
    ASTAR_REJECTED,
};

#define MAPFLAG_EXPLORED 0x00000001
#define MAPFLAG_NOPIERCE 0x00000002
#define MAPFLAG_NOPIERCE_EW 0x00000004
#define MAPFLAG_NOPIERCE_NS 0x00000008

enum Level_type
{
    LEVEL_ROOMS,
    LEVEL_ROOMS_BOSS,
    LEVEL_CAVE,
    LEVEL_NECROPOLIS,
    LEVEL_INFERNO,
    TOTAL_LEVEL_STYLES
};

// Standard dimensions of a level
#define DUN_WIDTH 42
#define DUN_HEIGHT 42
// Maximum dimensions of a level, relied on by display.cc
#define MAX_DUN_WIDTH 84
#define MAX_DUN_HEIGHT 84
#define ROOM_HT_DELTA 4
#define ROOM_WD_DELTA 4

struct Level;
extern Level *currlev;

enum Leventry_mode
{
    // allow for up to three prime staircases
    Leventry_stairs_dn_1,
    Leventry_stairs_dn_2,
    Leventry_stairs_dn_3,
    Leventry_stairs_up_1,
    Leventry_stairs_up_2,
    Leventry_stairs_up_3,
    // hell entry and exit modes are special
    Leventry_portal_to_hell,
    Leventry_portal_from_hell,
    // total-count value
    Total_leventries
};

struct Level_tag
{
    Dungeon_num dungeon;
    int level;
    Level *snapv() const;
    Level const *snapc() const;
    bool operator < (const Level_tag& right) const
    {
        return (dungeon < right.dungeon) || ((dungeon == right.dungeon) && (level < right.level));
    }
    bool operator == (const Level_tag& right) const
    {
        return (dungeon == right.dungeon) && (level == right.level);
    }
    bool operator != (const Level_tag& right) const
    {
        return (dungeon != right.dungeon) || (level != right.level);
    }
};

extern const Level_tag no_level;

struct Levextra
{
    Level *parent;
    bool overridden_monsel;

    virtual void excavate(void) = 0;
    virtual void populate(void) = 0;
    virtual libmrl::Coord get_injection_point(Leventry_mode mode = Leventry_stairs_dn_1) const = 0;
    bool has_monsel_override(void) const { return overridden_monsel; }
    // Reaction functions 
    virtual void react_to_profession(libmrl::Coord c, Player_profession prof, int prof_cmd)
    {
    }
    // returning one signifies succes.
    virtual int pre_leave_region(libmrl::Coord c) { return 1; }
    virtual int leave_region(libmrl::Coord c) { return 1; }
    virtual int enter_region(libmrl::Coord c) { return 1; }
};

struct Region
{
    virtual bool contains(libmrl::Coord c) const = 0;
    virtual bool linked_to(int rnum) const = 0;
    virtual libmrl::Coord random_point() const = 0;
    virtual ~Region() { }
};

#define NO_REGION -1
#include "monsters.hh"
#include "objects.hh"
#include "cloud.hh"

struct Level
{
    int height;
    int width;
    int levtype;
    Level_tag self;
    // This map tells us what mode we enter the destination level by...
    std::map<libmrl::Coord, Leventry_mode> exit_modes;
    // ... and this one tells us where the destination level *is*...
    std::map<Leventry_mode, Level_tag> exit_dests;
    // ... and this one tells us where we arrive when we enter *this* level by
    // a certain mode.
    std::map<Leventry_mode, libmrl::Coord> entries;
    std::set<Mon_handle> denizens;
    std::set<Obj_handle> booty;
    Obj_handle **mobjs;
    Mon_handle **mmons;
    Terrain_num **terrain;
    std::map<libmrl::Coord, Cloud> clouds;
    int32_t **rnums; /* region numbers */
    uint32_t **astar_invoc_num;
    uint32_t **astar_considered;
    libmrl::Coord **came_from;
    uint32_t **gscores;
    uint32_t **mflags;
    Levextra *levextra;
    // ctor/dtor
    Level() { }
    Level(Level_tag lt, int ht, int wd);
    ~Level();
    // construction/destruction helpers
    void build(void);
    void excavate(void);
    void populate(void);
    void leave(void);
    void release(void);
    // getters/setters
    bool outofbounds(libmrl::Coord c) const { return (c.y < 0) || (c.x < 0) || (c.y >= height) || (c.x >= width); }
    Terrain_num terrain_at(libmrl::Coord c) const { return terrain[c.y][c.x]; }
    void set_terrain(libmrl::Coord c, Terrain_num t) const { terrain[c.y][c.x] = t; }
    uint32_t flags_at(libmrl::Coord c) const { return mflags[c.y][c.x]; }
    void set_flag_at(libmrl::Coord c, uint32_t f) const { mflags[c.y][c.x] |= f; }
    void clear_flag_at(libmrl::Coord c, uint32_t f) const { mflags[c.y][c.x] &= ~f; }
    Mon_handle monster_at(libmrl::Coord c) const { return mmons[c.y][c.x]; }
    Obj_handle object_at(libmrl::Coord c) const { return mobjs[c.y][c.x]; }
    void set_mon_at(libmrl::Coord c, Mon_handle m) { mmons[c.y][c.x] = m; }
    void set_obj_at(libmrl::Coord c, Obj_handle o) { mobjs[c.y][c.x] = o; }
    Cloud cloud_at(libmrl::Coord c) const { std::map<libmrl::Coord, Cloud>::const_iterator iter = clouds.find(c); if (iter != clouds.end()) { return iter->second; } return no_cloud; }
    void set_cloud_at(libmrl::Coord c, Cloud const& cld) { clouds[c] = cld; }
    void clear_cloud_at(libmrl::Coord c) { std::map<libmrl::Coord, Cloud>::iterator iter = clouds.find(c); if (iter != clouds.end()) { clouds.erase(iter); } }
    int region_at(libmrl::Coord c) const { return rnums[c.y][c.x]; }
    void set_region(libmrl::Coord c, int r) const { rnums[c.y][c.x] = r; }
    int as_gscore(libmrl::Coord c) const { return gscores[c.y][c.x]; }
    int as_invoc(libmrl::Coord c) const { return astar_invoc_num[c.y][c.x]; }
    libmrl::Coord as_came_from(libmrl::Coord c) const { return came_from[c.y][c.x]; }
    int as_considered(libmrl::Coord c) const { return astar_considered[c.y][c.x]; }
    void as_considered(libmrl::Coord c, int val) { astar_considered[c.y][c.x] = val; }
    void set_as_gscore(libmrl::Coord c, int val) { gscores[c.y][c.x] = val; }
    void set_as_invoc(libmrl::Coord c, int val) { astar_invoc_num[c.y][c.x] = val; }
    void set_as_came_from(libmrl::Coord c, libmrl::Coord val) { came_from[c.y][c.x] = val; }
    void set_as_considered(libmrl::Coord c, Astar_states val) { astar_considered[c.y][c.x] = val; }
    Terrain_num base_floor(void) const { return FLOOR; }
    Terrain_num base_wall(void) const { return WALL; }
    // event reactions
    int pre_leave_region(libmrl::Coord c) { return (levextra ? levextra->pre_leave_region(c) : 1); }
    int leave_region(libmrl::Coord c) { return (levextra ? levextra->leave_region(c) : 1); }
    int enter_region(libmrl::Coord c) { return (levextra ? levextra->enter_region(c) : 1); }
};

typedef std::map<libmrl::Coord, Leventry_mode>::iterator Exitmode_iter;
typedef std::map<libmrl::Coord, Leventry_mode>::const_iterator Exitmode_citer;
typedef std::map<Leventry_mode, Level_tag>::iterator Exitdest_iter;
typedef std::map<Leventry_mode, Level_tag>::const_iterator Exitdest_citer;
typedef std::map<Leventry_mode, libmrl::Coord>::iterator Entry_iter;
typedef std::map<Leventry_mode, libmrl::Coord>::const_iterator Entry_citer;

extern std::map<Level_tag, Level *> levels;
typedef std::map<Level_tag, Level *>::const_iterator Level_citer;
typedef std::map<Level_tag, Level *>::iterator Level_iter;

inline Level *Level_tag::snapv() const
{
    std::map<Level_tag, Level *>::iterator iter = levels.find(*this);
    return (iter == levels.end()) ? 0 : iter->second;
}

inline Level const *Level_tag::snapc() const
{
    std::map<Level_tag, Level *>::iterator iter = levels.find(*this);
    return (iter == levels.end()) ? 0 : iter->second;
}

typedef int (*Mon_select_fun)(int levnum);

struct Dungeon_desc
{
    char const *name;
    bool persistent_levels;
    Mon_select_fun mon_selector;
};

extern Dungeon_desc dungeon_specs[];

struct Terrain_desc
{
    char const *name;
    bool opaque;        // blocks LOS
    bool impassable;    // blocks movement and projectiles
    bool feature;       // is a dungeon feature
    bool hostile;       // is hostile
    char symbol;        // symbol for text-mode display
    Dbash_colour colour;        // colour for text-mode display
};

extern const Terrain_desc terrain_data[];

struct Damage_desc
{
    int values[DT_COUNT];
    int sum;
    bool by_you;
    Mon_handle damagee;
    Mon_handle damager;
    bool sum_cached;
};

/* XXX misc.c data and funcs */
extern const char *damtype_names[DT_COUNT];
extern void get_iso_8601_time(std::string& dest);

/* XXX objects.c data and funcs */
extern void flavours_init(void);

/* XXX rng.c data and funcs */
#define RNG_MAX 0xFFFFFFFFu
extern uint32_t rng_state[5];
extern uint32_t saved_state[5];
extern uint32_t rng(void);
extern void rng_init(void);

/* XXX vector.c data and funcs */
extern void compute_directions(libmrl::Coord c1, libmrl::Coord c2, Direction_data *dir_data);

extern bool pos_visible(libmrl::Coord pos);

extern void ptac_init(void);
extern char *demon_get_name(void);
extern const char *numberwords[40];

/* XXX main.cc data and funcs */
Creature_speed get_tick_speed(uint32_t tick);
uint32_t get_next_tick(Creature_speed cspd);
#ifdef MULTIUSER
extern int user_permissions(void);
extern int game_permissions(void);
#endif
extern int exclusive_flat(int lower, int upper); /* l+1 ... u-1 */
extern int inclusive_flat(int lower, int upper); /* l ... u */
extern int one_die(int sides);	/* 1..n */
extern int dice(int count, int sides);
extern int zero_die(int sides);	/* 0..n-1 */
extern int do_command(Game_cmd command);
extern unsigned int convert_range(int dy, int dx);
extern bool game_finished;
extern uint32_t game_tick;
extern bool wizard_mode;
extern int game_uid;
extern int user_uid;
extern int action_speed;

/* XXX map.c data and funcs*/
extern void make_new_level(void);
extern void depart_level(Leventry_mode lem);
extern void go_to_level(Level_tag lt, Leventry_mode lem);
extern void inject_player(Level *lptr, Leventry_mode lem);
extern int get_room_x(int room);
extern int get_room_y(int room);
extern void room_reset(void);

#ifndef DISPLAY_H
#include "display.hh"
#endif

#ifndef UI_H
#include "ui.hh"
#endif

#ifndef PLAYER_H
#include "player.hh"
#endif

#endif

/* dunbash.hh */
