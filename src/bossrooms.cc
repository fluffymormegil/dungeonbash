/* bossroom.cc - Boss rooms for Martin's Dungeon Bash
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

#define BOSSROOM_CC

#include "dunbash.hh"
#include "rooms.hh"
#include "bossroom.hh"

/* Boss rooms for different styles of level have different geometric
 * considerations. For *all* boss rooms, we have the geometric constraint that
 * the layout must interdict LOS from outside the region.
 *
 * For a Classic Rooms level, we divide the level into nine 14x14 chunks. The
 * last row and column are interdicted, so the boss room has to be 12x12
 * including its perimeter wall.
 *
 * ######+#####
 * #...+..#...#
 * #...####...#
 * #..........#
 * ###......#+#
 * +.#......#.#
 * #.#......#.+
 * #+#......###
 * #..........#
 * #...####...#
 * #...#..+...#
 * #####+######
 */

const char boss_template[12][13] =
{
    "XXXXXX#XXXXX",
    "X...+..#...X",
    "X...####...X",
    "X..........X",
    "X##......#+X",
    "#.#......#.X",
    "X.#......#.#",
    "X+#......##X",
    "X..........X",
    "X...####...X",
    "X...#..+...X",
    "XXXXX#XXXXXX",
};

void Levext_rooms_boss::populate(void)
{
    // Populate the rest of the level first.
    Levext_rooms::populate();
    // Now do the boss room.
}

void Levext_rooms_boss::excavate_zoo_room(void)
{
    /* TODO implement boss level zoo room excavation. */
    int yseg = zoo_room / 3;
    int xseg = zoo_room % 3;
    libmrl::Coord topleft;
    libmrl::Coord botright;
    libmrl::Coord c;
    if ((parent->height < 42) || (parent->width < 42))
    {
        print_msg(MSGCHAN_INTERROR, "fatal error: boss room generation on Classic Rooms level smaller than 42x42.");
        press_enter();
        abort();
    }
    /* the following is a temporary hack. */
    topleft.y = (yseg * (parent->height / 3)) + 1;
    topleft.x = (xseg * (parent->width / 3)) + 1;
    botright.y = (yseg * (parent->height / 3)) + 12;
    botright.x = (xseg * (parent->width / 3)) + 12;
    for (c.y = topleft.y; c.y < botright.y; ++(c.y))
    {
        for (c.x = topleft.x; c.x < botright.x; ++(c.x))
        {
            switch (boss_template[c.y - topleft.y][c.x - topleft.x])
            {
            case 'X':
                parent->set_flag_at(c, MAPFLAG_NOPIERCE);
                parent->set_terrain(c, IRON_WALL);
                break;
            case '#':
                parent->set_terrain(c, IRON_WALL);
                break;
            case '.':
                parent->set_terrain(c, IRON_FLOOR);
                break;
            case '+':
                parent->set_terrain(c, DOOR);
                break;
            default:
                print_msg(MSGCHAN_INTERROR, "internal error: garbage character in boss room template.");
                parent->set_terrain(c, IRON_FLOOR);
                break;
            }
        }
    }
}

void Levext_rooms_boss::add_random_room(int yseg, int xseg)
{
    int roomidx = (yseg * 3) + xseg;
    libmrl::Coord topleft;
    libmrl::Coord botright;
    if (roomidx != zoo_room)
    {
        Levext_rooms::add_random_room(yseg, xseg);
        return;
    }
    segsused[yseg * 3 + xseg] = 1;
    topleft.y = 1 + yseg * (parent->height / 3);
    botright.y = (yseg + 1) * (parent->height / 3) - 1;
    topleft.x = 1 + xseg * (parent->width / 3);
    botright.x = (xseg + 1) * (parent->width / 3) - 1;
    bounds[roomidx][0] = topleft;
    bounds[roomidx][1] = botright;
}

int Levext_rooms_boss::leave_region(libmrl::Coord c)
{
    if (parent->region_at(c) == zoo_room)
    {
        // react to bossroom departure
        return 1;
    }
    else
    {
        return Levext_rooms::leave_region(c);
    }
}

int Levext_rooms_boss::enter_region(libmrl::Coord c)
{
    if (parent->region_at(c) == zoo_room)
    {
        // react to bossroom departure
        return 1;
    }
    else
    {
        return Levext_rooms::enter_region(c);
    }
}

/* bossrooms.cc */
