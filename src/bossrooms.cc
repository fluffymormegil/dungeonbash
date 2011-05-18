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

#define BOSSROOMS_CC

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
        PM_GOBLIN, PM_GOBLIN, PM_GOBLIN, -1, -1
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
    int yseg = zoo_room / 3;
    int xseg = zoo_room % 3;
    libmormegil::Coord topleft;
    libmormegil::Coord botright;
    libmormegil::Coord c;
    int i;
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
        spec = goblin_chieftain_boss;
    }
    else if (parent->self.level == 10)
    {
        /* level 10 will be a shrine of evil */
    }
    else
    {
        /* Otherwise we get a golem foundry. */
    }
    /* the following is a temporary hack. */
    topleft.y = (yseg * (parent->height / 3)) + 1;
    topleft.x = (xseg * (parent->width / 3)) + 1;
    botright.y = (yseg * (parent->height / 3)) + 12;
    botright.x = (xseg * (parent->width / 3)) + 12;
    bounds[zoo_room][0] = topleft;
    bounds[zoo_room][1] = botright;
    for (i = 0; i < 10; ++i)
    {
        num_posns[i] = dunbash::NOWHERE;
    }
    for (c.y = topleft.y; c.y <= botright.y; ++(c.y))
    {
        for (c.x = topleft.x; c.x <= botright.x; ++(c.x))
        {
            parent->set_region(c, zoo_room);
            int ch = boss_template[c.y - topleft.y][c.x - topleft.x];
            switch (ch)
            {
            case 'X':
                parent->set_flag_at(c, MAPFLAG_NOPIERCE);
                parent->set_terrain(c, spec.x_terrain);
                break;
            case '#':
                parent->set_terrain(c, spec.hash_terrain);
                break;
            case '.':
                parent->set_terrain(c, spec.dot_terrain);
                break;
            case '~':
                parent->set_terrain(c, spec.tilde_terrain);
                break;
            case '+':
                parent->set_terrain(c, DOOR);
                break;
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9': 
                /* what's that? your platform's native character set doesn't
                 * store the ten standard numerals as a dense sequence?  Even
                 * IBM use ASCII these days, for crying out louud! */
                parent->set_terrain(c, spec.num_terrs[ch - '0']);
                num_posns[ch - '0'] = c;
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
    libmormegil::Coord topleft;
    libmormegil::Coord botright;
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

int Levext_rooms_boss::leave_region(libmormegil::Coord c)
{
    if (parent->region_at(c) == zoo_room)
    {
        // react to bossroom departure
        if (boss.snapv())
        {
            /* Boss hasn't been killed, so mock the craven hero and despawn
             * the denizens. */
            mock_coward();
            while (!guards.empty())
            {
                release_monster(*guards.begin());
                guards.erase(guards.begin());
            }
            release_monster(boss);
            boss = NO_MONSTER;
            libmormegil::Coord c;
            /* Cleanup: purge clouds. No, this is not "OP". Everything else
             * resets, after all. */
            for (c.y = bounds[zoo_room][0].y; c.y <= bounds[zoo_room][1].y; ++(c.y))
            {
                for (c.x = bounds[zoo_room][0].x; c.x <= bounds[zoo_room][1].x; ++(c.x))
                {
                    parent->clear_cloud_at(c);
                }
            }
        }
        else
        {
            cleared = true;
        }
        return 1;
    }
    else
    {
        return Levext_rooms::leave_region(c);
    }
}

int Levext_rooms_boss::enter_region(libmormegil::Coord c)
{
    int i;
    if ((parent->region_at(c) == zoo_room) && !cleared)
    {
        /* initialize monsters. */
        boss = create_mon(spec.num_pmons[0], num_posns[0], parent);
        for (i = 1; i < 10; ++i)
        {
            if (spec.num_pmons[i] == -1)
            {
                continue;
            }
            Mon_handle mon = create_mon(spec.num_pmons[i], num_posns[i], parent);
            mon.snapv()->no_exp = true;
            guards.insert(mon);
        }
        return 1;
    }
    else
    {
        return Levext_rooms::enter_region(c);
    }
}

void Levext_rooms_boss::mock_coward() const
{
    print_msg(0, "\"Coward! Wretch! Craven! Stand and fight!\"");
    print_msg(0, "You sense that your foes will recover while");
    print_msg(0, "their leader remains alive.");
}

/* bossrooms.cc */
