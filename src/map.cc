/* map.cc
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

#include "dunbash.hh"
#include "monsters.hh"
#include "objects.hh"
#include "rooms.hh"
#include "loadsave.hh"

#include <string.h>
#include <limits.h>
uint32_t depth = 1;
uint32_t dungeon = 0;
Level *currlev = 0;
std::map<Level_tag, Level *> levels;
const Level_tag no_level = { Total_dungeons, UINT_MAX };

bool save_on_stairs;

Level::Level(int h, int w) : height(h), width(w)
{
    build();
}

Level::~Level()
{
    release();
}

void Level::release(void)
{
    int i;
    std::set<Obj_handle>::iterator oiter;
    std::set<Mon_handle>::iterator miter;
    while ((oiter = booty.begin()) != booty.end())
    {
        release_obj(*oiter);
    }
    while ((miter = denizens.begin()) != denizens.end())
    {
        release_monster(*miter);
    }
    for (i = 0; i < height; ++i)
    {
        free(mobjs[i]);
        free(mmons[i]);
        free(terrain[i]);
        free(rnums[i]);
        free(astar_invoc_num[i]);
        free(astar_considered[i]);
        free(came_from[i]);
        free(gscores[i]);
        free(mflags[i]);
    }
    free(mobjs);
    free(mmons);
    free(terrain);
    free(rnums);
    free(astar_invoc_num);
    free(astar_considered);
    free(came_from);
    free(gscores);
    free(mflags);
    delete levextra;
    levextra = 0;
}

void Level::leave(void)
{
}

void make_new_level(Level_tag lt)
{
    Level *lptr;
    lptr = new Level(DUN_HEIGHT, DUN_WIDTH);
    lptr->self = lt;
    levels[lptr->self] = lptr;
    lptr->excavate();
    lptr->populate();
}

void Level::excavate(void)
{
    levextra->excavate();
}

void Level::build(void)
{
    int i;
    int j;

    levtype = LEVEL_ROOMS;
    mobjs = (Obj_handle **) calloc(height, sizeof (Obj_handle *));
    mmons = (Mon_handle **) calloc(height, sizeof (Mon_handle *));
    rnums = (int **) calloc(height, sizeof (int *));
    terrain = (Terrain_num **) calloc(height, sizeof (Terrain_num *));
    astar_invoc_num = (uint32_t **) calloc(height, sizeof (uint32_t *));
    astar_considered = (uint32_t **) calloc(height, sizeof (uint32_t *));
    gscores = (uint32_t **) calloc(height, sizeof (uint32_t *));
    mflags = (uint32_t **) calloc(height, sizeof (uint32_t *));
    came_from = (libmrl::Coord **) calloc(height, sizeof (libmrl::Coord *));
    for (i = 0; i < height; ++i)
    {
        mobjs[i] = (Obj_handle *) calloc(width, sizeof (Obj_handle ));
        mmons[i] = (Mon_handle *) calloc(width, sizeof (Mon_handle ));
        rnums[i] = (int *) calloc(width, sizeof (int ));
        terrain[i] = (Terrain_num *) calloc(width, sizeof (Terrain_num ));
        astar_invoc_num[i] = (uint32_t *) calloc(width, sizeof (uint32_t ));
        astar_considered[i] = (uint32_t *) calloc(width, sizeof (uint32_t ));
        gscores[i] = (uint32_t *) calloc(width, sizeof (uint32_t ));
        mflags[i] = (uint32_t *) calloc(width, sizeof (uint32_t ));
        came_from[i] = (libmrl::Coord *) calloc(width, sizeof (libmrl::Coord ));
        for (j = 0; j < width; ++j)
        {
            mobjs[i][j] = NO_OBJECT;
            mmons[i][j] = NO_MONSTER;
            rnums[i][j] = NO_REGION;
            terrain[i][j] = WALL;
        }
    }
    levextra = new Levext_rooms();
    levextra->parent = this;
}

void Level::populate(void)
{
    levextra->populate();
}

void inject_player(Level *lptr, Leventry_mode lem)
{
    Entry_citer eciter = lptr->entries.find(lem);
    if (eciter == lptr->entries.end())
    {
        // This mode doesn't have a defined destination in the specified
        // level. Invoke levextra's injection point algorithm.
        u.pos = lptr->levextra->get_injection_point(lem);
    }
    else
    {
        u.pos = eciter->second;
    }
    reloc_player(u.pos);
}

void go_to_level(Level_tag lt, Leventry_mode lem)
{
    u.lev = lt;
    Level_iter liter = levels.find(lt);
    if (liter != levels.end())
    {
        currlev = liter->second;
    }
    else
    {
        make_new_level(lt);
        currlev = levels[lt];
    }
    inject_player(currlev, lem);

    if (save_on_stairs)
    {
        save_game();
    }

    touch_back_buffer();
    display_update();
}

void depart_level(Leventry_mode lem)
{
    // TODO implement reaction APIs for dungeon levels
}

/* map.cc */
