/* bossrooms.cc - Boss rooms for Martin's Dungeon Bash
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

#define BOSSROOM_CC

#include "dunbash.hh"
#include "rooms.hh"
#include "bossrooms.hh"
#include "pmonid.hh"

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
    "X.4.####.5.X",
    "X..........X",
    "X##.~~~~.#+X",
    "#.#.~01~.#.X",
    "X.#.~23~.#.#",
    "X+#.~~~~.##X",
    "X..........X",
    "X.6.####.7.X",
    "X...#..+...X",
    "XXXXX#XXXXXX",
};

const char corner_shrine_template[12][13] =
{
    "XXXXXXXXXXXX",
    "X..........X",
    "X........_.X",
    "X..........X",
    "X....~~~~~~X",
    "X..........X",
    "X....~..###X",
    "X....~..+..X",
    "X....~..##.X",
    "X....~...#+X",
    "X....~...#.#",
    "XXXXXXXXXX#X",
};

const char edge_shrine_template[12][13] =
{
    "XXXXXXXXXX#X",
    "X....~...#.X",
    "X....~...#+X",
    "X....~...+.X",
    "X....~...#+X",
    "X....~...#.#",
    "X_.......#+X",
    "X....~...#.X",
    "X....~...+.X",
    "X....~...#+X",
    "X....~...#.X",
    "XXXXXXXXXX#X",
};

void Levext_rooms_boss::populate_zoo_room(void)
{
}

Boss_spec goblin_chieftain_boss =
{
    // X # . ~
    WALL, WALL, FLOOR, FLOOR,
    // monsters at the numbered positions
    {
        PM_GOBLIN_CHIEFTAIN, PM_GOBLIN, PM_GOBLIN, PM_GOBLIN, PM_GOBLIN,
        PM_GOBLIN, PM_GOBLIN, PM_GOBLIN, PM_GOBLIN, PM_GOBLIN
    },
    // objects at the numbered positions
    {
        -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1,
    },
    // terrains at the numbered positions
    {
        FLOOR, FLOOR, FLOOR, FLOOR, FLOOR,
        FLOOR, FLOOR, FLOOR, FLOOR, FLOOR
    }
};

void Levext_rooms_boss::excavate_zoo_room(void)
{
    /* TODO implement boss level zoo room excavation. */
    int yseg = zoo_room / 3;
    int xseg = zoo_room % 3;
    Terrain_num terr[8];
    int pms[8];
    libmrl::Coord topleft;
    libmrl::Coord botright;
    libmrl::Coord c;
    Terrain_num tilde_terrain = FLOOR;
    if ((parent->height < 42) || (parent->width < 42))
    {
        print_msg(MSGCHAN_INTERROR, "fatal error: boss room generation on Classic Rooms level smaller than 42x42.");
        press_enter();
        abort();
    }
    /* Select bossroom subflavour; we will have separate code later but for
     * now it gets embedded in this function in one big blob */
    if (parent->self.level == 5)
    {
        /*  */
    }
    else if (parent->self.level == 10)
    {
        /* level 10 will be a shrine of evil */
    }
    /* the following is a temporary hack. */
    topleft.y = (yseg * (parent->height / 3)) + 1;
    topleft.x = (xseg * (parent->width / 3)) + 1;
    botright.y = (yseg * (parent->height / 3)) + 12;
    botright.x = (xseg * (parent->width / 3)) + 12;
    bounds[zoo_room][0] = topleft;
    bounds[zoo_room][1] = botright;
    print_msg(0, "room bounds %d: x %d %d y %d %d", zoo_room, topleft.x, botright.x, topleft.y, botright.y);
    for (c.y = topleft.y; c.y <= botright.y; ++(c.y))
    {
        for (c.x = topleft.x; c.x <= botright.x; ++(c.x))
        {
            int ch = boss_template[c.y - topleft.y][c.x - topleft.x];
            switch (ch)
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
            case '0':
                parent->set_terrain(c, terr[0]);
                break;
            case '1':
                parent->set_terrain(c, terr[1]);
                break;
            case '2':
                parent->set_terrain(c, terr[2]);
                break;
            case '3':
                parent->set_terrain(c, terr[3]);
                break;
            case '4':
                parent->set_terrain(c, terr[4]);
                break;
            case '5':
                parent->set_terrain(c, terr[5]);
                break;
            case '6':
                parent->set_terrain(c, terr[6]);
                break;
            case '7':
                parent->set_terrain(c, terr[7]);
                break;
            case '8':
                parent->set_terrain(c, terr[8]);
                break;
            case '9':
                parent->set_terrain(c, terr[9]);
                break;
            case '~':
                parent->set_terrain(c, tilde_terrain);
                break;
            default:
                print_msg(MSGCHAN_INTERROR, "internal error: garbage character '%c' (hex %2.2x) in boss room template.", ch, ch & 0xff);
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
        // react to bossroom entry
        return 1;
    }
    else
    {
        return Levext_rooms::enter_region(c);
    }
}

/* bossrooms.cc */
