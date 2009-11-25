/* vision.cc
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

#include "dunbash.hh"
#include "radiance.hh"
#include "vision.hh"
#include "monsters.hh"
#include <string.h>
//#include "dungeon.h"


Square_radiance vismap = {
    {},
    { 0, 0 },
    10,
    block_vision
};

bool block_vision(libmrl::Coord pos)
{
    if (currlev->monster_at(pos).valid())
    {
        return false;
    }
    if (pos == u.pos)
    {
        return false;
    }
    return terrain_data[currlev->terrain_at(pos)].opaque;
}

bool pos_visible(libmrl::Coord pos)
{
    libmrl::Coord delta;
    libmrl::Coord adelta;
    bool vis;
    delta = pos - u.pos;
    adelta = libmrl::abs(delta);
    vis = (((adelta.y > 10) || (adelta.x > 10)) ? false : vismap.array[10 + delta.y][10 + delta.x]);
    return vis;
}

bool mon_visible(Mon_handle mon)
{
    Mon const *mptr = mon.snapc();
    return mptr ? mptr->in_fov() : false;
}

bool Mon::in_fov(void) const
{
    return pos_visible(pos);
}

void do_vision(void)
{
    libmrl::Coord indices;
    libmrl::Coord cell;
    vismap.origin = u.pos;
    memset(vismap.array, 0, sizeof vismap.array);
    vismap.array[10][10] = 1;
    irradiate_square(&vismap);
    for ((indices.y = 0), cell.y = u.pos.y - 10; indices.y < 21; ++(indices.y), ++(cell.y))
    {
        for ((indices.x = 0), cell.x = u.pos.x - 10; indices.x < 21; ++(indices.x), ++(cell.x))
        {
            if (!currlev->outofbounds(cell))
            {
                // cheap hack to undraw monsters
                if (vismap.array[indices.y][indices.x])
                {
                    if (!(currlev->flags_at(cell) & MAPFLAG_EXPLORED))
                    {
                        currlev->set_flag_at(cell, MAPFLAG_EXPLORED);
                        newsym(cell);
                    }
                    else if (currlev->object_at(cell).valid())
                    {
                        newsym(cell);
                    }
                }
                Mon *mptr = currlev->monster_at(cell).snapv();
                if (mptr)
                {
                    if (vismap.array[indices.y][indices.x])
                    {
                        mptr->ai_lastpos = u.pos;
                    }
                    newsym(cell);
                }
            }
        }
    }
}

/* vision.c */
