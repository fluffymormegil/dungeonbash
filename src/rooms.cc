/* rooms.cc
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
#include "rooms.hh"
#include "monsters.hh"
#include "objects.hh"
#include "pmonid.hh"

#include <string.h>

char const * const room_flavour_strings[] =
{
    "a nondescript room",
    "a treasure zoo",
    "a smithy",
    "a crypt",
    "an abominable shrine",
    "a barracks",
    "an alchemical laboratory",
    "a library",
    "a magician's workroom"
};

void Levext_rooms::excavate_normal_room(int rnum)
{
    libmrl::Coord topleft = bounds[rnum][0];
    libmrl::Coord botright = bounds[rnum][1];
    libmrl::Coord c;
    print_msg(0,"room bounds %d: x %d %d y %d %d", rnum, topleft.x, botright.x, topleft.y, botright.y);
    for (c.y = topleft.y + 1; c.y < botright.y; c.y++)
    {
        for (c.x = topleft.x + 1; c.x < botright.x; c.x++)
	{
	    parent->set_terrain(c, FLOOR);
	    parent->set_region(c, rnum);
	}
    }
    for (c.y = topleft.y; c.y <= botright.y; c.y++)
    {
        c.x = topleft.x;
        parent->set_region(c, rnum);
        c.x = botright.x;
        parent->set_region(c, rnum);
    }
    for (c.x = topleft.x; c.x <= botright.x; c.x++)
    {
        c.y = topleft.y;
        parent->set_region(c, rnum);
        c.y = botright.y;
        parent->set_region(c, rnum);
    }
}

void Levext_rooms::excavate_shrine(int rnum)
{
    libmrl::Coord topleft = bounds[rnum][0];
    libmrl::Coord botright = bounds[rnum][1];
    libmrl::Coord c = { (topleft.y + botright.y) / 2, (topleft.x + botright.x) / 2 };
    excavate_normal_room(rnum);
    switch (zoo_style)
    {
    case ZOO_SHRINE_FIRE:
        parent->set_terrain(c + libmrl::NORTHWEST, LAVA_POOL);
        parent->set_terrain(c + libmrl::NORTHEAST, LAVA_POOL);
        parent->set_terrain(c + libmrl::SOUTHWEST, LAVA_POOL);
        parent->set_terrain(c + libmrl::SOUTHEAST, LAVA_POOL);
        parent->set_terrain(c + libmrl::NORTH, RED_FLOOR);
        parent->set_terrain(c + libmrl::EAST, RED_FLOOR);
        parent->set_terrain(c + libmrl::WEST, RED_FLOOR);
        parent->set_terrain(c + libmrl::SOUTH, RED_FLOOR);
        break;
    case ZOO_SHRINE_IRON:
        parent->set_terrain(c + libmrl::NORTHWEST, IRON_WALL);
        parent->set_terrain(c + libmrl::NORTHEAST, IRON_WALL);
        parent->set_terrain(c + libmrl::SOUTHWEST, IRON_WALL);
        parent->set_terrain(c + libmrl::SOUTHEAST, IRON_WALL);
        parent->set_terrain(c + libmrl::NORTH, IRON_FLOOR);
        parent->set_terrain(c + libmrl::EAST, IRON_FLOOR);
        parent->set_terrain(c + libmrl::WEST, IRON_FLOOR);
        parent->set_terrain(c + libmrl::SOUTH, IRON_FLOOR);
        break;
    case ZOO_SHRINE_BONE:
        parent->set_terrain(c + libmrl::NORTHWEST, BONE_WALL);
        parent->set_terrain(c + libmrl::NORTHEAST, BONE_WALL);
        parent->set_terrain(c + libmrl::SOUTHWEST, BONE_WALL);
        parent->set_terrain(c + libmrl::SOUTHEAST, BONE_WALL);
        parent->set_terrain(c + libmrl::NORTH, BONE_FLOOR);
        parent->set_terrain(c + libmrl::EAST, BONE_FLOOR);
        parent->set_terrain(c + libmrl::WEST, BONE_FLOOR);
        parent->set_terrain(c + libmrl::SOUTH, BONE_FLOOR);
        break;
    case ZOO_SHRINE_DECAY:
        // putresecent slime implemented as acid
        parent->set_terrain(c + libmrl::NORTHWEST, ACID_POOL);
        parent->set_terrain(c + libmrl::NORTHEAST, ACID_POOL);
        parent->set_terrain(c + libmrl::SOUTHWEST, ACID_POOL);
        parent->set_terrain(c + libmrl::SOUTHEAST, ACID_POOL);
        break;
    case ZOO_SHRINE_FLESH:
        // bathing-pools
        parent->set_terrain(c + libmrl::NORTHWEST, WATER_POOL);
        parent->set_terrain(c + libmrl::NORTHEAST, WATER_POOL);
        parent->set_terrain(c + libmrl::SOUTHWEST, WATER_POOL);
        parent->set_terrain(c + libmrl::SOUTHEAST, WATER_POOL);
        parent->set_terrain(c + libmrl::NORTH, SKIN_FLOOR);
        parent->set_terrain(c + libmrl::EAST, SKIN_FLOOR);
        parent->set_terrain(c + libmrl::WEST, SKIN_FLOOR);
        parent->set_terrain(c + libmrl::SOUTH, SKIN_FLOOR);
        break;
    default:
        print_msg(MSGCHAN_INTERROR, "excavate_shrine() called on level with non-shrine zoo_style");
        return;
    }
    parent->set_terrain(c, ALTAR);
}

void Levext_rooms::excavate_morgue(int rnum)
{
    libmrl::Coord topleft = bounds[rnum][0];
    libmrl::Coord botright = bounds[rnum][1];
    libmrl::Coord c;
    excavate_normal_room(rnum);
    for (c.y = topleft.y + 2; c.y < botright.y - 2; ++(c.y))
    {
        for (c.x = topleft.x + 2; c.x < botright.x - 2; c.x += 2)
        {
            parent->set_terrain(c, TOMBSTONE);
        }
    }
}

void Levext_rooms::excavate_smithy(int rnum)
{
    libmrl::Coord topleft = bounds[rnum][0];
    libmrl::Coord botright = bounds[rnum][1];
    libmrl::Coord c = { (topleft.y + botright.y) / 2, (topleft.x + botright.x) / 2 };
    libmrl::Coord c2;
    excavate_normal_room(rnum);
    parent->set_terrain(c, ANVIL);
    do
    {
        c2 = get_obj_scatter(c + libmrl::SOUTHEAST, parent);
    } while ((c.y <= topleft.y) || (c.y >= botright.y) ||
             (c.x <= topleft.x) || (c.x >= botright.x));
    parent->set_terrain(c2, FURNACE);
}

void Levext_rooms::add_random_room(int yseg, int xseg)
{
    int roomidx = (yseg * 3) + xseg;
    libmrl::Coord centre;
    libmrl::Coord topleft;
    libmrl::Coord botright;
    int ht;
    int wd;
    centre.y = (parent->height - 2) / 6 + yseg * ((parent->height - 2) / 3);
    centre.x = (parent->width - 2) / 6 + xseg * ((parent->width - 2) / 3);
    ht = libmrl::max(6, 2 + dice(2, ROOM_HT_DELTA));
    wd = libmrl::max(5, 2 + dice(2, ROOM_WD_DELTA));
    topleft.y = centre.y - ROOM_HT_DELTA - 1 + zero_die(((ROOM_HT_DELTA + 1) * 2) - ht);
    topleft.x = centre.x - ROOM_WD_DELTA - 1 + zero_die(((ROOM_WD_DELTA + 1) * 2) - wd);
    botright.y = topleft.y + ht;
    botright.x = topleft.x + wd;
    topleft.y = std::max(topleft.y, yseg * (parent->height / 3));
    topleft.x = std::max(topleft.x, xseg * (parent->width / 3));
    botright.y = std::min(botright.y, (yseg + 1) * (parent->height / 3));
    botright.x = std::min(botright.x, (xseg + 1) * (parent->width / 3));
    bounds[roomidx][0] = topleft;
    bounds[roomidx][1] = botright;
    segsused[yseg * 3 + xseg] = 1;
}

void Levext_rooms::excavate_room(int roomidx)
{
    if (roomidx == zoo_room)
    {
        excavate_zoo_room();
    }
    else
    {
        roomflav[roomidx] = (zero_die(6) ? Rflav_dull : Rflav_barracks);
        excavate_normal_room(roomidx);
    }
}

void Levext_rooms::excavate_zoo_room()
{
    switch (zoo_style)
    {
    case ZOO_MORGUE:
        roomflav[zoo_room] = Rflav_treasure_zoo;
        excavate_morgue(zoo_room);
        break;
    case ZOO_SHRINE_FIRE:
    case ZOO_SHRINE_IRON:
    case ZOO_SHRINE_BONE:
    case ZOO_SHRINE_DECAY:
    case ZOO_SHRINE_FLESH:
        roomflav[zoo_room] = Rflav_shrine;
        excavate_shrine(zoo_room);
        break;
    case ZOO_TREASURE:
        roomflav[zoo_room] = Rflav_treasure_zoo;
        excavate_normal_room(zoo_room);
        break;
    case ZOO_SMITHY:
        roomflav[zoo_room] = Rflav_smithy;
        excavate_smithy(zoo_room);
        break;
    default:
        roomflav[zoo_room] = Rflav_dull;
        excavate_normal_room(zoo_room);
        break;
    }
}
void Levext_rooms::link_rooms(int r1, int r2)
{
    libmrl::Coord pos[4];
    int i;
    libmrl::Coord end1;
    libmrl::Coord posts[4];
    libmrl::Coord end2;
    libmrl::Coord mid1;
    libmrl::Coord mid2;
    /* Update the linkage matrix. */
    linkage[r1][r2] = 1;
    linkage[r2][r1] = 1;
    for (i = 0; i < MAX_ROOMS; i++)
    {
	if ((i == r1) || (i == r2))
	{
	    continue;
	}
	if ((linkage[r1][i] > 0) && !linkage[r2][i])
	{
	    linkage[r2][i] = 2;
	    linkage[i][r2] = 2;
	}
	if ((linkage[r2][i] > 0) && !linkage[r1][i])
	{
	    linkage[r1][i] = 2;
	    linkage[i][r1] = 2;
	}
    }
    /* Take a copy of the corners of the rooms, and use them to find an entry
     * and exit point. */
    pos[0] = bounds[r1][0];
    pos[1] = bounds[r1][1];
    pos[2] = bounds[r2][0];
    pos[3] = bounds[r2][1];
    /* Now generate the corridor. */
    if ((r1 % 3) == (r2 % 3))
    {
	if (pos[1].y < pos[2].y)
	{
            end1.y = pos[1].y;
            end2.y = pos[2].y;
	}
	else
	{
            end1.y = pos[0].y;
            end2.y = pos[3].y;
	}
        do
        {
            end1.x = exclusive_flat(pos[0].x, pos[1].x);
        } while (parent->flags_at(end1) & MAPFLAG_NOPIERCE);
        do
        {
            end2.x = exclusive_flat(pos[2].x, pos[3].x);
        } while (parent->flags_at(end2) & MAPFLAG_NOPIERCE);
        mid1.y = exclusive_flat(end1.y, end2.y);
        mid1.x = end1.x;
        mid2.y = mid1.y;
        mid2.x = end2.x;
        posts[0] = end1 + libmrl::EAST;
        posts[1] = end1 + libmrl::WEST;
        posts[2] = end2 + libmrl::EAST;
        posts[3] = end2 + libmrl::WEST;
    }
    else
    {
	if (pos[1].x < pos[2].x)
	{
            end1.x = pos[1].x;
            end2.x = pos[2].x;
	}
	else
	{
            end1.x = pos[0].x;
            end2.x = pos[3].x;
	}
        do
        {
            end1.y = exclusive_flat(pos[0].y, pos[1].y);
        } while (parent->flags_at(end1) & MAPFLAG_NOPIERCE);
        do
        {
            end2.y = exclusive_flat(pos[2].y, pos[3].y);
        } while (parent->flags_at(end2) & MAPFLAG_NOPIERCE);
        mid1.x = exclusive_flat(end1.x, end2.x);
        mid1.y = end1.y;
        mid2.x = mid1.x;
        mid2.y = end2.y;
        posts[0] = end1 + libmrl::SOUTH;
        posts[1] = end1 + libmrl::NORTH;
        posts[2] = end2 + libmrl::SOUTH;
        posts[3] = end2 + libmrl::NORTH;
    }
    parent->set_flag_at(end1, MAPFLAG_NOPIERCE);
    parent->set_flag_at(end2, MAPFLAG_NOPIERCE);
    parent->set_flag_at(posts[0], MAPFLAG_NOPIERCE);
    parent->set_flag_at(posts[1], MAPFLAG_NOPIERCE);
    parent->set_flag_at(posts[2], MAPFLAG_NOPIERCE);
    parent->set_flag_at(posts[3], MAPFLAG_NOPIERCE);
    excavate_corridor_segment(end1, mid1, zero_die(10), false);
    excavate_corridor_segment(mid1, mid2, false, false);
    excavate_corridor_segment(mid2, end2, false, zero_die(10));
}

void Levext_rooms::put_stairs(void)
{
    Level_tag below = parent->self;
    Level_tag above = parent->self;
    below.level++;
    above.level--;
    dstairs_pos.y = exclusive_flat(bounds[dstairs_room][0].y, bounds[dstairs_room][1].y);
    dstairs_pos.x = exclusive_flat(bounds[dstairs_room][0].x, bounds[dstairs_room][1].x);
    parent->set_terrain(dstairs_pos, STAIRS_DOWN);
    parent->exit_modes[dstairs_pos] = Leventry_stairs_dn_1;
    parent->entries[Leventry_stairs_up_1] = dstairs_pos;
    parent->exit_dests[Leventry_stairs_dn_1] = below;
    ustairs_pos.y = exclusive_flat(bounds[ustairs_room][0].y, bounds[ustairs_room][1].y);
    ustairs_pos.x = exclusive_flat(bounds[ustairs_room][0].x, bounds[ustairs_room][1].x);
    parent->set_terrain(ustairs_pos, STAIRS_UP);
    parent->exit_modes[ustairs_pos] = Leventry_stairs_dn_1;
    parent->entries[Leventry_stairs_dn_1] = ustairs_pos;
    parent->exit_dests[Leventry_stairs_up_1] = above;
}

int edge_rooms[4] = { 1, 3, 5, 7 };
int corners[4][2] = { { 0, 2 }, { 0, 6 }, { 2, 8 }, { 6, 8 } };

void Levext_rooms::excavate(void)
{
    int i;
    parent = parent;
    /*
     * For now, stick with the policy that we have nine rooms. This will need
     * rejigging when I get round to making the number of rooms variable.
     */
    actual_rooms = MAX_ROOMS;
    // Select dstairs room, ustairs room, zoo room
    dstairs_room = zero_die(actual_rooms);
    do
    {
        ustairs_room = zero_die(actual_rooms);
    } while (ustairs_room == dstairs_room);
    do
    {
        zoo_room = zero_die(actual_rooms);
    } while ((zoo_room == ustairs_room) ||
             (zoo_room == dstairs_room));
    if ((parent->self.level > 2) && !zero_die(10))
    {
        zoo_style = ZOO_TREASURE;
    }
    else if ((parent->self.level > 4) && !zero_die(9))
    {
        zoo_style = ZOO_MORGUE;
    }
    else if ((parent->self.level > 6) && !zero_die(4))
    {
        switch (zero_die(std::min(parent->self.level - 6, 5)))
        {
        case 0:
            zoo_style = ZOO_SHRINE_FLESH;
            break;
        case 1:
            zoo_style = ZOO_SHRINE_DECAY;
            break;
        case 2:
            zoo_style = ZOO_SHRINE_IRON;
            break;
        case 3:
            zoo_style = ZOO_SHRINE_FIRE;
            break;
        case 4:
            zoo_style = ZOO_SHRINE_BONE;
            break;
        }
    }
    else if (!zero_die(3))
    {
        zoo_style = ZOO_SMITHY;
    }
    else
    {
        zoo_style = NO_ZOO;
    }
    /* Add rooms */
    for (i = 0; i < actual_rooms; i++)
    {
	add_random_room(i / 3, i % 3);
        excavate_room(i);
    }
    /* Add corridors */
    /* Link the centre room to an edge room. */
    link_rooms(4, edge_rooms[zero_die(4)]);
    /* And to another; if we're already linked, don't bother. */
    i = zero_die(4);
    if (linkage[4][edge_rooms[i]] == 0)
    {
	link_rooms(4, edge_rooms[i]);
    }
    /* Link each edge room to one of its corner rooms. */
    for (i = 0; i < 4; i++)
    {
	link_rooms(edge_rooms[i], corners[i][zero_die(2)]);
    }
    /* At this point, 1-2 edge rooms and their attached corner rooms
     * have linkage to the centre. */
    /* Link each edge room to its unlinked corner if it is not 2-linked
     * to the centre. */
    for (i = 0; i < 4; i++)
    {
	if (!linkage[4][edge_rooms[i]])
	{
	    if (linkage[edge_rooms[i]][corners[i][0]])
	    {
		link_rooms(edge_rooms[i], corners[i][1]);
	    }
	    else
	    {
		link_rooms(edge_rooms[i], corners[i][0]);
	    }
	}

    }
    /* Link each corner room to its unlinked edge if that edge is not
     * 2-linked to the centre.  If we still haven't got centre
     * connectivity for the edge room, connect the edge to the centre. */
    for (i = 0; i < 4; i++)
    {
	if (!linkage[4][edge_rooms[i]])
	{
	    if (!linkage[edge_rooms[i]][corners[i][0]])
	    {
		link_rooms(edge_rooms[i], corners[i][0]);
	    }
	    if (!linkage[edge_rooms[i]][corners[i][1]])
	    {
		link_rooms(edge_rooms[i], corners[i][1]);
	    }
	}
	if (!linkage[4][edge_rooms[i]])
	{
	    link_rooms(edge_rooms[i], 4);
	}
    }
    /* Just for safety's sake: Now we know all edges are attached,
     * make sure all the corners are. (Previously, it was possible
     * for them not to be. I know, because I met such a level :) */
    for (i = 3; i > -1; i--)
    {
        if ((!linkage[4][corners[i][0]]) &&
            (linkage[edge_rooms[i]][corners[i][0]] != 1))
	{
	    link_rooms(edge_rooms[i], corners[i][0]);
	}
        if ((!linkage[4][corners[i][1]]) &&
            (linkage[edge_rooms[i]][corners[i][1]] != 1))
	{
	    link_rooms(edge_rooms[i], corners[i][1]);
	}
    }
    /* Add the stairs */
    put_stairs();
}

void Levext_rooms::populate_treasure_zoo()
{
    int mons;
    int items;
    int tries;
    Mon_handle mon;
    Obj_handle obj;
    libmrl::Coord pos;
    /* A treasure zoo should get nine monsters and nine items. */
    for (mons = 0; mons < 9; mons++)
    {
	for (tries = 0; tries < 200; tries++)
	{
	    pos.y = exclusive_flat(bounds[zoo_room][0].y, bounds[zoo_room][1].y);
	    pos.x = exclusive_flat(bounds[zoo_room][0].x, bounds[zoo_room][1].x);
	    if (parent->monster_at(pos) == NO_MONSTER)
	    {
		mon = create_mon(NO_PM, pos, parent);
		if (mon.valid())
		{
		    break;
		}
	    }
	}
    }
    for (items = 0; items < 9; items++)
    {
	for (tries = 0; tries < 200; tries++)
	{
	    pos.y = exclusive_flat(bounds[zoo_room][0].y, bounds[zoo_room][1].y);
	    pos.x = exclusive_flat(bounds[zoo_room][0].x, bounds[zoo_room][1].x);
	    if (parent->object_at(pos) == NO_OBJECT)
	    {
		obj = create_obj(NO_POBJ, 1, 0, pos, parent);
		if (obj.valid())
		{
		    break;
		}
	    }
	}
    }
}

void Levext_rooms::populate_shrine()
{
    libmrl::Coord topleft = bounds[zoo_room][0];
    libmrl::Coord botright = bounds[zoo_room][1];
    libmrl::Coord c = { (topleft.y + botright.y) / 2, (topleft.x + botright.x) / 2 };
    switch (zoo_style)
    {
    case ZOO_SHRINE_FIRE:
        create_mon(PM_FIRE_PRIEST, c, parent);
        break;
    case ZOO_SHRINE_DECAY:
        create_mon(PM_SLIME_PRIEST, c, parent);
        break;
    case ZOO_SHRINE_IRON:
        create_mon(PM_IRON_PRIEST, c, parent);
        break;
    case ZOO_SHRINE_FLESH:
        create_mon(PM_SYBARITE, c, parent);
        break;
    case ZOO_SHRINE_BONE:
        create_mon(PM_DEATH_PRIEST, c, parent);
        break;
    default:
        print_msg(MSGCHAN_INTERROR, "error: attempt to populate_shrine() on non-shrine level.");
        break;
    }
}

void Levext_rooms::populate_morgue()
{
    // TODO decide if graveyards should have any monsters in them.
    return;
}

void Levext_rooms::populate_smithy()
{
    libmrl::Coord topleft = bounds[zoo_room][0];
    libmrl::Coord botright = bounds[zoo_room][1];
    libmrl::Coord c = { (topleft.y + botright.y) / 2, (topleft.x + botright.x) / 2 };
    if (zero_die(parent->self.level) > 3)
    {
        create_mon(PM_SMITH, c, parent);
    }
}

libmrl::Coord Levext_rooms::get_room_cell(int room) const
{
    libmrl::Coord tmp;
    tmp.y = exclusive_flat(bounds[room][0].y, bounds[room][1].y);
    tmp.x = exclusive_flat(bounds[room][0].x, bounds[room][1].x);
    return tmp;
}

int Levext_rooms::get_levgen_mon_spot(libmrl::Coord *ppos) const
{
    /* Get a vacant floor cell that isn't in the treasure zoo. */
    int room_try;
    int cell_try;
    libmrl::Coord trypos = libmrl::NOWHERE;
    int room;
    for (room_try = 0; room_try < (MAX_ROOMS * 2); room_try++)
    {
	room = zero_die(actual_rooms);
	if (room == zoo_room)
	{
	    continue;
	}
	for (cell_try = 0; cell_try < 200; cell_try++)
	{
            trypos = get_room_cell(room);
            Terrain_num t = parent->terrain_at(trypos);
	    if ((parent->monster_at(trypos).valid()) ||
                terrain_data[t].impassable ||
                terrain_data[t].feature)
	    {
                trypos = libmrl::NOWHERE;
		continue;
	    }
	    break;
	}
	break;
    }
    if (trypos == libmrl::NOWHERE)
    {
	return -1;
    }
    *ppos = trypos;
    return 0;
}

void Levext_rooms::populate_zoo_room(void)
{
    switch (zoo_style)
    {
    case ZOO_TREASURE:
        populate_treasure_zoo();
        break;
    case ZOO_SMITHY:
        populate_smithy();
        break;
    case ZOO_MORGUE:
        populate_morgue();
        break;
    case ZOO_SHRINE_IRON:
    case ZOO_SHRINE_FIRE:
    case ZOO_SHRINE_BONE:
    case ZOO_SHRINE_DECAY:
    case ZOO_SHRINE_FLESH:
        populate_shrine();
        break;
    default:
        break;
    }
}

void Levext_rooms::populate(void)
{
    int i;
    int j;
    libmrl::Coord pos;
    int ic;
    /* Check for a "treasure zoo" */
    populate_zoo_room();
    /* Generate some random monsters */
    for (i = 0; i < 10; i++)
    {
	j = get_levgen_mon_spot(&pos);
	if (j == -1)
	{
	    continue;
	}
	create_mon(NO_PM, pos, parent);
    }
    ic = 3 + parent->self.level;
    if (ic > 40)
    {
	/* Never create more than 40 items. */
	ic = 40;
    }
    /* Generate some random treasure */
    for (i = 0; i < ic; i++)
    {
	j = get_levgen_mon_spot(&pos);
	if (j == -1)
	{
	    continue;
	}
	create_obj(NO_POBJ, 1, 0, pos, parent);
    }
}

libmrl::Coord Levext_rooms::get_injection_point(Leventry_mode mode) const
{
    return ustairs_pos;
}

Levext_rooms::Levext_rooms() : Levextra(),
    actual_rooms(MAX_ROOMS),
    zoo_room(NO_ROOM),
    dstairs_room(NO_ROOM),
    ustairs_room(NO_ROOM)
{
    int i, j;
    parent = 0;
    for (i = 0; i < MAX_ROOMS; ++i)
    {
        for (j = 0; j < MAX_ROOMS; ++j)
        {
            linkage[i][j] = 0;
        }
        segsused[i] = false;
        bounds[i][0] = libmrl::NOWHERE;
        bounds[i][1] = libmrl::NOWHERE;
    }
}

/* This function sets the appropriate NOPIERCE flag for the squares adjacent
 * to the corridor, but does not check such flags itself, nor does it set the
 * NOPIERCE flags adjacent to the ends of the segments. As such, it is the
 * caller's responsibility to perform any "survey" work required before
 * deciding where to excavate a corridor segment. */
void Levext_rooms::excavate_corridor_segment(libmrl::Coord c1, libmrl::Coord c2, bool door1, bool door2)
{
    libmrl::Coord sgn = libmrl::sign(c2 - c1);
    libmrl::Coord pos;
    libmrl::Coord side1, side2;
    int sideflags;
    if (sgn.y)
    {
        side1.y = 0;
        side1.x = 1;
        side2.y = 0;
        side2.x = -1;
        sideflags = MAPFLAG_NOPIERCE_NS;
    }
    else
    {
        side1.x = 0;
        side1.y = 1;
        side2.x = 0;
        side2.y = -1;
        sideflags = MAPFLAG_NOPIERCE_EW;
    }
    parent->set_terrain(c1, door1 ? DOOR : FLOOR);
    parent->set_terrain(c2, door2 ? DOOR : FLOOR);
    for (pos = c1 + sgn; pos != c2; pos += sgn)
    {
        parent->set_terrain(pos, FLOOR);
        parent->set_flag_at(pos + side1, sideflags);
        parent->set_flag_at(pos + side2, sideflags);
    }
}

int Levext_rooms::enter_region(libmrl::Coord c)
{
    if (parent->region_at(c) != -1)
    {
        if (roomflav[parent->region_at(c)] != Rflav_dull)
        {
            print_msg(MSGCHAN_FLUFF, "You enter %s.", room_flavour_strings[roomflav[parent->region_at(c)]]);
        }
    }
    return 1;
}

int Levext_rooms::leave_region(libmrl::Coord c)
{
    if (parent->region_at(c) != -1)
    {
        if (roomflav[parent->region_at(c)] != Rflav_dull)
        {
            print_msg(MSGCHAN_FLUFF, "You leave %s.", room_flavour_strings[roomflav[parent->region_at(c)]]);
        }
    }
    return 1;
}

/* rooms.cc */
