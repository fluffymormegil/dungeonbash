/* monsters.cc
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

#define MONSTERS_C

#include "dunbash.hh"
#include "objects.hh"
#include "monsters.hh"
#include "pobjid.hh"
#include "pmonid.hh"

std::map<uint64_t, Mon *> monsters;
uint64_t next_mon_handle = 1ull;

static int reject_mon(int pm, int depth);

void monsters_init(void)
{
}

int summoning(libmrl::Coord c, int how_many, Level *lptr)
{
    int i;
    libmrl::Coord delta;
    libmrl::Coord testpos;
    int tryct;
    Mon_handle mon;
    int created = 0;
    int pmon;
    if (!lptr)
    {
        lptr = currlev;
    }
    for (i = 0; i < how_many; i++)
    {
	for (tryct = 0; tryct < 20; tryct++)
	{
	    delta.y = zero_die(3) - 1;
	    delta.x = zero_die(3) - 1;
            testpos = delta + c;
	    if ((!terrain_data[lptr->terrain_at(testpos)].impassable) &&
		(lptr->monster_at(testpos) == NO_MONSTER) &&
		(testpos != u.pos))
	    {
		pmon = get_random_pmon(lptr->self.level);
		if (pmon_is_magician(pmon))
		{
		    /* Never summon magicians! */
		    continue;
		}
		mon = create_mon(NO_PM, testpos);
		if (mon.valid())
		{
		    created++;
		    break;
		}
	    }
	}
    }
    if ((created > 0) && (lptr == currlev))
    {
	map_updated = 1;
	display_update();
    }
    return created;
}

int ood(int depth, int power, int ratio)
{
    return (depth - power + ratio - 1) / ratio;
}

int get_random_pmon(int depth)
{
    int tryct;
    int pm;
    for (tryct = 0; tryct < 200; tryct++)
    {
	pm = zero_die(PM_COUNT);
	if (reject_mon(pm, depth))
	{
	    pm = NO_PM;
	    continue;
	}
	break;
    }
    return pm;
}

Mon_handle create_zombie(int corpse_mon, libmrl::Coord c, Level *lptr)
{
    Mon_handle mon = create_mon(PM_ZOMBIE, c, lptr);
    if (mon.valid())
    {
        Mon *mptr = mon.snapv();
        mptr->meta = corpse_mon;
    }
    return mon;
}

Mon_handle get_free_mon(void)
{
    Mon_handle tmp(next_mon_handle);
    ++next_mon_handle;
    return tmp;
}

Mon_handle create_mon(int pm_idx, libmrl::Coord c, Level *lptr)
{
    Mon_handle mon;
    Mon *mptr;
    if (!lptr)
    {
        lptr = currlev;
    }
    if (lptr->monster_at(c).valid())
    {
	print_msg(MSGCHAN_INTERROR, "Attempt to create monster at occupied space %d %d", c.y, c.x);
	return NO_MONSTER;
    }
    if (pm_idx == NO_PM)
    {
	pm_idx = get_random_pmon(lptr->self.level);
        if (pm_idx == NO_PM)
        {
            return NO_MONSTER;
        }
    }
    mon = get_free_mon();
    mptr = new Mon;
    monsters[mon.value] = mptr;
    mptr->self = mon;
    mptr->lev = lptr->self;
    mptr->mon_id = pm_idx;
    mptr->pos = c;
    mptr->ai_lastpos = libmrl::NOWHERE;
    mptr->hpmax = permons[pm_idx].hp + ood(mptr->lev.level, permons[pm_idx].power, 1);
    mptr->hpcur = mptr->hpmax;
    mptr->mtohit = permons[pm_idx].melee.acc + ood(mptr->lev.level, permons[pm_idx].power, 3);
    mptr->defence = permons[pm_idx].defence + ood(mptr->lev.level, permons[pm_idx].power, 3);
    mptr->mdam = permons[pm_idx].melee.dam + ood(mptr->lev.level, permons[pm_idx].power, 5);
    if (permons[pm_idx].ranged.acc != -1)
    {
        mptr->rtohit = permons[pm_idx].ranged.acc + ood(mptr->lev.level, permons[pm_idx].power, 3);
        mptr->rdam = permons[pm_idx].ranged.dam + ood(mptr->lev.level, permons[pm_idx].power, 5);
    }
    else
    {
        mptr->rtohit = -1;
        mptr->rdam = -1;
    }
    mptr->awake = 0;
    if (pmon_always_female(pm_idx))
    {
        mptr->sex = Csex_female;
    }
    else if (pmon_always_male(pm_idx))
    {
        mptr->sex = Csex_male;
    }
    else if (pmon_always_neuter(pm_idx))
    {
        mptr->sex = Csex_neuter;
    }
    else if (pmon_always_other(pm_idx))
    {
        mptr->sex = Csex_other;
    }
    else
    {
        mptr->sex = zero_die(2) ? Csex_male : Csex_female;
    }
    if (mptr->mon_id == PM_ZOMBIE)
    {
        mptr->meta = PM_HUMAN;
    }
    if (pmon_is_greater_demon(pm_idx))
    {
        mptr->name = demon_get_name();
    }
    lptr->denizens.insert(mon);
    lptr->set_mon_at(c, mon);
    if (currlev == lptr)
    {
        newsym(c);
    }
    return mon;
}

void death_drop(Mon_handle mon)
{
    Mon *mptr = mon.snapv();
    int pm = mptr->mon_id;
    int pm2;
    Obj_handle k = NO_OBJECT;
    libmrl::Coord pos;
    if ((permons[pm].flags & PMF_MEATY) && !(permons[pm].flags & PMF_NOCORPSE))
    {
        pos = get_obj_scatter(mptr->pos);
        if (pos == libmrl::NOWHERE)
        {
            return;
        }
        switch (permons[pm].species)
        {
        case Species_own:
            pm2 = pm;
            break;
        case Species_elf:
            pm2 = PM_ELF;
            break;
        case Species_human:
            pm2 = PM_HUMAN;
            break;
        case Species_giant:
            pm2 = PM_GIANT;
            break;
        default:
            pm2 = pm;
        }
        create_corpse(pm2, pos);
        if ((pos != mptr->pos) && pos_visible(pos))
        {
            newsym(pos);
        }
    }
    pos = get_obj_scatter(mptr->pos);
    if (pos == libmrl::NOWHERE)
    {
        return;
    }
    switch (pm)
    {
    case PM_GOBLIN:
	if (!zero_die(4))
	{
	    k = create_obj(PO_DAGGER, 1, 0, pos);
	}
	break;
    case PM_THUG:
    case PM_GOON:
	if (!zero_die(4))
	{
	    k = create_obj(PO_MACE, 1, 0, pos);
	}
	else if (!zero_die(3))
	{
	    k = create_obj(PO_LEATHER_ARMOUR, 1, 0, pos);
	}
	break;
    case PM_HUNTER:
	if (!zero_die(6))
	{
	    k = create_obj(PO_BOW, 1, 0, pos);
	}
	break;
    case PM_DUELLIST:
	if (!zero_die(6))
	{
	    k = create_obj(PO_LONG_SWORD, 1, 0, pos);
	}
	break;
    case PM_WIZARD:
	if (!zero_die(4))
	{
	    k = create_obj_class(POCLASS_SCROLL, 1, 0, pos);
	}
	else if (!zero_die(3))
	{
	    k = create_obj_class(POCLASS_POTION, 1, 0, pos);
	}
        break;
    case PM_WARLORD:
	if (!zero_die(3))
	{
	    k = create_obj(PO_RUNESWORD, 1, 0, pos);
	}
        break;
    default:
	break;
    }
    if (k.valid() && pos_visible(pos))
    {
        newsym(pos);
    }
    map_updated = 1;
}

bool Mon::can_pass(libmrl::Coord c) const
{
    if (currlev->outofbounds(c))
    {
	return false;
    }
    if (currlev->monster_at(c).valid())
    {
	return false;
    }
    if (c == u.pos)
    {
	/* Sanity check! */
	return false;
    }
    if (mon_id == PM_WRAITH)
    {
	/* Wraiths can walk through walls. */
	return true;
    }
    if (terrain_data[currlev->terrain_at(c)].impassable)
    {
	return false;
    }
    return true;
}

void Mon::get_name(std::string *dest, int article, bool shortname) const
{
    char numbuf[64];
    if (!permons[mon_id].name)
    {
        snprintf(numbuf, 64, "GROB THE VOID (%d)", mon_id);
        (*dest) = numbuf;
    }
    else
    {
        if (name)
        {
            (*dest) = name;
            if (shortname)
            {
                return;
            }
            else
            {
                (*dest) += " ";
                article = 1;
            }
        }
        int pm = (mon_id == PM_ZOMBIE) ? meta : mon_id;
        switch (article)
        {
        case 0:
            (*dest) += (is_vowel(permons[pm].name[0]) ? "an " : "a ");
            break;
        case 1:
            (*dest) += "the ";
            break;
        case 2:
            (*dest) += (is_vowel(permons[pm].name[0]) ? "An " : "A ");
            break;
        case 3:
            (*dest) += "The ";
            break;
        }
        (*dest) += permons[pm].name;
        if (mon_id == PM_ZOMBIE)
        {
            (*dest) += " zombie";
        }
    }
}

void heal_mon(Mon_handle mon, int amount, int cansee)
{
    Mon * mptr = mon.snapv();
    std::string name;
    mptr->get_name(&name, 3);
    if (amount > (mptr->hpmax - mptr->hpcur))
    {
	amount = mptr->hpmax - mptr->hpcur;
    }
    if (amount > 0)
    {
        if (cansee)
        {
            print_msg(0, "%s looks healthier.", name.c_str());
        }
        mptr->hpcur += amount;
    }
}

int knock_mon(Mon_handle mon, libmrl::Coord step, int force, bool by_you)
{
    if (!mon.valid())
    {
        print_msg(MSGCHAN_INTERROR, "attempt to knockback invalid monster handle.");
        return -1;
    }
    Mon *mptr = mon.snapv();
    libmrl::Coord pos = mptr->pos + step;
    /*
     * The force parameter will determine how "big" a monster an attack can
     * knock back. For now, we aren't going to use it.
     */
    if (mptr->can_pass(pos))
    {
        // Push the monster back and return 1.
        move_mon(mon, pos);
        return 1;
    }
    else
    {
        // Obstruction
        return 2;
    }
}

bool damage_mon(Mon_handle mon, int amount, int by_you)
{
    Mon *mptr = mon.snapv();
    libmrl::Coord pos = mptr->pos;
    std::string name;
    if (by_you)
    {
        if (mptr->in_fov())
        {
            mptr->get_name(&name, 1);
        }
        else
        {
            name = "something";
        }
    }
    else
    {
        mptr->get_name(&name, 2);
    }
    if (amount >= mptr->hpcur)
    {
        if (by_you)
        {
            print_msg(0, "You kill %s!", name.c_str());
            gain_experience(permons[mptr->mon_id].exp);
        }
        else if (mptr->in_fov())
        {
            print_msg(0, "%s dies.", name.c_str());
        }
        death_drop(mon);
        release_monster(mon);
        newsym(pos);
	map_updated = 1;
	display_update();
        return true;
    }
    else
    {
	mptr->hpcur -= amount;
        return false;
    }
}

int reject_mon(int pm, int depth)
{
    if ((permons[pm].power > depth) || (zero_die(100) < permons[pm].rarity))
    {
	return 1;
    }
    return 0;
}

int teleport_mon_to_you(Mon_handle mon)
{
    Mon *mptr = mon.snapv();
    int tryct;
    libmrl::Coord delta;
    libmrl::Coord pos;
    int success = 0;
    for (tryct = 0; tryct < 40; tryct++)
    {
        do
        {
            delta.y = zero_die(3) - 1;
            delta.x = zero_die(3) - 1;
        } while ((delta.y == 0) && (delta.x == 0));
        pos = u.pos;
        pos += delta;
        if (mptr->can_pass(pos))
        {
            success = 1;
            break;
        }
    }
    if (success)
    {
        std::string namestr;
        move_mon(mon, pos);
        mptr->get_name(&namestr, 2);
        print_msg(0, "%s appears in a puff of smoke.", namestr.c_str());
        return 1;
    }
    return 0;
}

int teleport_mon(Mon_handle mon)
{
    int rval = -1;
    int cell_try;
    Mon *mptr = mon.snapv();
    libmrl::Coord pos;
    if (mptr)
    {
        for (cell_try = 0; cell_try < 400; cell_try++)
        {
            pos.y = exclusive_flat(0, currlev->height - 1);
            pos.x = exclusive_flat(0, currlev->width - 1);
            if ((currlev->monster_at(pos) == NO_MONSTER) &&
                mptr->can_pass(pos))
            {
                move_mon(mon, pos);
                rval = 0;
                break;
            }
        }
    }
    return rval;
}

void move_mon(Mon_handle mon, libmrl::Coord pos, Level *lptr)
{
    // TODO account for the monster being moved into hostile terrain by a
    // player action and dying before they can get out of it. (I don't want
    // to penalize fighters for Slamming monsters into lava.)
    Mon *mptr = mon.snapv();
    if (!lptr)
    {
        lptr = currlev;
    }
    if (pos == mptr->pos)
    {
        print_msg(MSGCHAN_INTERROR, "Warning: moving mon %lld to its own position!", mon.value);
        return;
    }
    Mon_handle foo = currlev->monster_at(mptr->pos);
    if (!mptr->can_pass(pos))
    {
	print_msg(MSGCHAN_INTERROR, "Warning: mon %lld could not pass %d, %d.", mon.value, pos.y, pos.x);
	return;
    }
    if (!(mon == foo))
    {
	print_msg(MSGCHAN_INTERROR, "Monster map array in disorder...");
	press_enter();
	return;
    }
    currlev->set_mon_at(mptr->pos, NO_MONSTER);
    newsym(mptr->pos);
    mptr->pos = pos;
    currlev->set_mon_at(mptr->pos, mon);
    newsym(mptr->pos);
    display_update();
}

void summon_mon_near(int pm_idx, libmrl::Coord pos, Level *lptr)
{
    libmrl::Coord pos2;
    Mon_handle mon;
    pos2 = get_mon_scatter(pos, lptr);
    if (pos == libmrl::NOWHERE)
    {
        return;
    }
    mon = create_mon(pm_idx, pos2, lptr);
    if ((lptr == currlev) && !mon.valid())
    {
        print_msg(0, "You feel lonely.");
    }
}

bool update_mon(Mon_handle mon)
{
    int cansee;
    bool wiped = false;
    Mon *mptr = mon.snapv();
    if (!mptr)
    {
        print_msg(MSGCHAN_INTERROR, "FATAL: null pointer.");
        press_enter();
        abort();
    }
    mptr->last_update = game_tick;
    if (mptr->hpcur < mptr->hpmax)
    {
	cansee = mptr->in_fov();
	switch (mptr->mon_id)
	{
	case PM_TROLL:
	    if (!(game_tick % 10))
	    {
		if (cansee)
		{
		    print_msg(0, "The troll regenerates.");
		}
		heal_mon(mon, one_die(3) + 3, 0);
	    }
	    break;

	case PM_ZOMBIE:
	    /* Zombies don't recover from their injuries. */
	    break;

	default:
	    if (!(game_tick % 20))
	    {
		heal_mon(mon, 1, cansee);
	    }
	    break;
	}
    }
    if (!mptr->perseffs.empty())
    {
        wiped = false;
#ifdef DEBUG_PEFFS
        print_msg(0, "Monster %p has %d persistent effects.", mptr->perseffs.size());
        press_enter();
#endif
        std::list<Perseff_data>::iterator peff_iter;
        std::list<Perseff_data>::iterator peff_next;
        Status_flags new_status;
        new_status.clear_all();
        for (peff_iter = mptr->perseffs.begin();
             peff_iter != mptr->perseffs.end();
             peff_iter = peff_next)
        {
            peff_next = peff_iter;
            ++peff_next;
            if (!perseff_meta[(*peff_iter).flavour].ontological_inertia &&
                !(*peff_iter).by_you && !(*peff_iter).caster.valid())
            {
                mptr->perseffs.erase(peff_iter);
                continue;
            }
            if ((*peff_iter).duration > 0)
            {
                (*peff_iter).duration--;
            }
            if ((*peff_iter).duration)
            {
                /* Act on the debuff */
                new_status.set_flag((*peff_iter).flavour);
            }
            else
            {
                mptr->perseffs.erase(peff_iter);
            }
        }
        mptr->status = new_status;
        for (peff_iter = mptr->perseffs.begin();
             peff_iter != mptr->perseffs.end();
             peff_iter = peff_next)
        {
            wiped = mptr->suffer(*peff_iter);
            if (wiped)
            {
                break;
            }
        }
    }
    return wiped;
}

libmrl::Coord get_mon_scatter(libmrl::Coord pos, Level *lptr)
{
    libmrl::Coord delta;
    int tryct = 0;
    if (!lptr)
    {
        lptr = currlev;
    }
    while (((lptr->monster_at(pos).valid()) ||
            (pos == u.pos) ||
            terrain_data[lptr->terrain_at(pos)].impassable ||
            terrain_data[lptr->terrain_at(pos)].feature) &&
           (tryct < 100))
    {
        delta.y = zero_die(3) - 1;
        delta.x = zero_die(3) - 1;
        // Don't try to scatter through a wall.
        if (int(delta) && !(terrain_data[lptr->terrain_at(pos + delta)].impassable))
        {
            ++tryct;
            pos += delta;
        }
    }
    if (tryct >= 100)
    {
        return libmrl::NOWHERE;
    }
    return pos;
}

void release_monster(Mon_handle mon)
{
    Mon *mptr = mon.snapv();
    if (!mptr)
    {
        print_msg(MSGCHAN_INTERROR, "releasing null monster");
        press_enter();
        abort();
    }
    Level *lptr = mptr->lev.snapv();
    if (!lptr)
    {
        print_msg(MSGCHAN_INTERROR, "levelsnap failed");
        press_enter();
        abort();
    }
    std::set<Mon_handle>::iterator iter = lptr->denizens.find(mon);
    if (iter == lptr->denizens.end())
    {
        print_msg(MSGCHAN_INTERROR, "iter == lptr->denizens.end()");
        press_enter();
        abort();
    }
    else
    {
        lptr->denizens.erase(iter);
    }
    lptr->set_mon_at(mptr->pos, NO_MONSTER);
    if (mptr->current_path)
    {
        mptr->discard_path();
    }
    if (mptr->name)
    {
        free(mptr->name);
        mptr->name = 0;
    }
    delete mptr;
    std::map<uint64_t, Mon *>::iterator miter = monsters.find(mon.value);
    if (miter == monsters.end())
    {
        print_msg(MSGCHAN_INTERROR, "miter == monsters.end() before erase from monsters");
        press_enter();
        abort();
    }
    monsters.erase(miter);
    miter = monsters.find(mon.value);
    if (miter != monsters.end())
    {
        print_msg(MSGCHAN_INTERROR, "miter != monsters.end()");
        press_enter();
        abort();
    }
    mptr = mon.snapv();
    if (mptr)
    {
        print_msg(MSGCHAN_INTERROR, "mptr non-null");
        press_enter();
        abort();
    }
}
/* monsters.cc */
