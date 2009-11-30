/* obj2.cc
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

#define OBJ2_CC

#include "dunbash.hh"
#include "objects.hh"
#include "monsters.hh"
#include "pobjid.hh"
#include "pmonid.hh"
#include "radiance.hh"
#include "vision.hh"

int zapeff_frost(libmrl::Coord pos)
{
    // wallwalking mons are immune to frost zaps (the only current wallwalking
    // mon is immune to cold *anyway*, but whatever).
    int rv = 0;
    if (terrain_data[currlev->terrain_at(pos)].opaque)
    {
        rv = 2;
    }
    // TODO test and refine balance of frost bolts
    Mon_handle mon = currlev->monster_at(pos);
    if (mon.valid())
    {
        rv = 1;
        Mon *mptr = mon.snapv();
        // for now, make this an autohit effect that autofails if the victim
        // resists cold.
        if  (pmon_resists_cold(mptr->mon_id))
        {
            std::string namestr;
            mptr->get_name(&namestr, 3, true);
            print_msg(0, "%s seems unbothered by the cold.", namestr.c_str());
        }
        else
        {
            std::string namestr;
            mptr->get_name(&namestr, 1, true);
            print_msg(0, "Bitter cold saps %s's strength.", namestr.c_str());
            Perseff_data peff = 
            {
                Perseff_bitter_chill, 4, 10, true, false
            };
            mptr->apply_effect(peff);
        }
    }
    return rv;
}

/***************************************/
struct Corpseblast_data
{
    int max;
};

bool corpseblast_func(libmrl::Coord c, void *data)
{
    if (c == u.pos)
    {
        int dmg;
        Corpseblast_data *deref = (Corpseblast_data *) data;
        dmg = one_die(deref->max);
        print_msg(0, "You are blasted by the exploding corpse.");
        damage_u(dmg, DEATH_KILLED, "an exploding corpse");
        return false;
    }
    else if (currlev->monster_at(c).valid())
    {
        int dmg;
        Corpseblast_data *deref = (Corpseblast_data *) data;
        std::string namestr;
        currlev->monster_at(c).snapc()->get_name(&namestr, 3);
        namestr += " is blasted.";
        dmg = one_die(deref->max);
        print_msg(0, "%s", namestr.c_str());
        damage_mon(currlev->monster_at(c), dmg, true);
        return true;
    }
    return false;
}

int zapeff_corpse_explosion(libmrl::Coord pos)
{
    Obj_handle obj = currlev->object_at(pos);
    Obj *optr = obj.snapv();
    Mon_handle mon = currlev->monster_at(pos);
    Mon *mptr = mon.snapv();
    int rv = 0;
    bool b;
    if (mptr && !(pmon_is_ethereal(mptr->mon_id)) &&
        (pmon_is_undead(mptr->mon_id) ||
         pmon_is_death_demon(mptr->mon_id)))
    {
        rv = 1;
        std::string m_namestr;
        Corpseblast_data data;
        mptr->get_name(&m_namestr, 3, true);
        print_msg(0, "Arcane energy erupts from %s!", m_namestr.c_str());
        if (mptr->mon_id == PM_ZOMBIE)
        {
            data.max = libmrl::min(permons[mptr->meta].hp, 20);
        }
        else
        {
            data.max = libmrl::min(permons[mptr->mon_id].hp, 20);
        }
        Square_radiance corpseblast =
        {
            { { 0 } }, pos, 2, block_vision
        };
        irradiate_square(&corpseblast);
        b = spiral_square(&corpseblast, corpseblast_func, &data);
    }
    else if (optr && (optr->obj_id == PO_CORPSE))
    {
        rv = 1;
        std::string namestr;
        Corpseblast_data data;
        optr->get_name(&namestr);
        data.max = libmrl::min(permons[optr->meta].hp, 20);
        print_msg(0, "Arcane energy detonates %s!", namestr.c_str());
        release_obj(obj);
        newsym(pos);
        map_updated = true;
        Square_radiance corpseblast =
        {
            { { 0 } }, pos, 2, block_vision
        };
        irradiate_square(&corpseblast);
        b = spiral_square(&corpseblast, corpseblast_func, &data);
    }
    else if (terrain_data[currlev->terrain_at(pos)].opaque)
    {
        rv = 2;
    }
    return rv;
}

/***************************************/
struct Shatter_data
{
    int max;
    const char *name;
};

bool terrainblast_func(libmrl::Coord c, void *data)
{
    if (c == u.pos)
    {
        int dmg;
        Shatter_data *deref = (Shatter_data *) data;
        dmg = one_die(deref->max);
        print_msg(0, "You are blasted by the exploding %s.", deref->name);
        damage_u(dmg, DEATH_KILLED, "flying debris");
        return false;
    }
    else if (currlev->monster_at(c).valid())
    {
        int dmg;
        Shatter_data *deref = (Shatter_data *) data;
        std::string namestr;
        currlev->monster_at(c).snapc()->get_name(&namestr, 3);
        namestr += " is blasted.";
        dmg = one_die(deref->max);
        print_msg(0, "%s", namestr.c_str());
        damage_mon(currlev->monster_at(c), dmg, true);
        return true;
    }
    return false;
}

int zapeff_shattering(libmrl::Coord pos)
{
    int rv = 0;
    bool remove = false;
    Terrain_num remove_result;
    Terrain_num contents;
    Square_radiance terrainsquare =
    {
        { { 0 } }, pos, 0, block_vision
    };
    Shatter_data data;
    contents = currlev->terrain_at(pos);
    switch (contents)
    {
    case FURNACE:
        rv = 1;
        remove = true;
        terrainsquare.radius = 3;
        data.max = 30;
        remove_result = IRON_FLOOR;
        data.name = "furnace";
        break;
    case ANVIL:
        rv = 1;
        remove = true;
        remove_result = currlev->base_floor();
        terrainsquare.radius = 2;
        data.max = 20;
        data.name = "anvil";
        remove = true;
        break;
    case DOOR:
        rv = 1;
        remove = true;
        remove_result = currlev->base_floor();
        data.max = 10;
        data.name = "door";
        terrainsquare.radius = 1;
        break;
    default:
        if (terrain_data[contents].impassable)
        {
            rv = 2;
        }
        else
        {
            Obj_handle obj = currlev->object_at(pos);
            Obj *optr = obj.snapv();
            if (optr)
            {
                // TODO object destruction for brittle items.
            }
        }
        break;
    }
    if (rv == 1)
    {
        if (remove)
        {
            currlev->set_terrain(pos, remove_result);
            newsym(pos);
            map_updated = 1;
            do_vision();
        }
        print_msg(0, "The %s explodes!", data.name);
        terrainsquare.array[10][10] = true;
        irradiate_square(&terrainsquare);
        spiral_square(&terrainsquare, terrainblast_func, &data);
    }
    return rv;
}

int zap_wand(Obj_handle obj)
{
    Obj *optr = obj.snapv();
    libmrl::Coord pos;
    libmrl::Coord dir;
    int i;
    int rv = 0;
    if (optr->durability)
    {
        i = select_dir(&dir);
        if (i != -1)
        {
            if ((dir.y == 0) && (dir.x == 0))
            {
                print_msg(MSGCHAN_MINORFAIL, "You think zapping yourself would be unwise.");
            }
            else
            {
                rv = 1;
                for (pos = u.pos + dir; u.pos.distance(pos) < 10; pos += dir)
                {
                    switch (optr->obj_id)
                    {
                    case PO_WAND_OF_CORPSE_EXPLOSION:
                        i = zapeff_corpse_explosion(pos);
                        if (i == 1)
                        {
                            identify_pobj(PO_WAND_OF_CORPSE_EXPLOSION);
                        }
                        break;
                    case PO_WAND_OF_FROST:
                        identify_pobj(PO_WAND_OF_FROST);
                        i = zapeff_frost(pos);
                        break;
                    case PO_WAND_OF_SHATTERING:
                        i = zapeff_shattering(pos);
                        if (i == 1)
                        {
                            identify_pobj(PO_WAND_OF_SHATTERING);
                        }
                        break;
                    default:
                        print_msg(MSGCHAN_INTERROR, "ERROR: Zapping nonwand.");
                        i = 1;
                        break;
                    }
                    if (i != 0)
                    {
                        break;
                    }
                }
            }
        }
        damage_obj(obj);
    }
    else
    {
        // We shouldn't be here, since wands disintegrate on depletion.
        rv = 1;
        print_msg(0, "The wand sputters a little, then fizzles out.");
    }
    return rv;
}

int activate_misc(Obj_handle obj)
{
    Obj *optr = obj.snapv();
    switch (optr->obj_id)
    {
    case PO_LICH_SKULL:
        print_msg(0, "You look into the skull's empty eye sockets, and see bone.");
        return 0;

    case PO_ORNATE_EBONY_CUBE:
        print_msg(0, "You puzzle briefly over the ebony and brass cube.");
        return 0;

    case PO_LAMEN_OF_TORMENT:
        print_msg(0, "You shudder at the obscenity of the manhide badge.");
        return 0;

    case PO_LAMEN_OF_DEATH:
        print_msg(0, "You admire the intricately carved bone badge.");
        return 0;

    default:
        print_msg(MSGCHAN_INTERROR, "ERROR: Activating nonmisc.");
        break;
    }
    return 0;
}

/* obj2.cc */
