/* u.cc
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
#include "combat.hh"
#include "objects.hh"
#include "pobjid.hh"
#include "player.hh"
#include "vision.hh"
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <stdio.h>
#include "cfgfile.hh"

Player u;
bool levelup_wait;
bool name_prompt;

void recalc_defence(void)
{
    int i;
    Obj const *armptr = u.armour.snapc();
    Obj const *ringptr = u.ring.snapc();
    for (i = 0; i < DT_COUNT; i++)
    {
	u.resistances[i] &= RESIST_MASK_TEMPORARY;
    }
    u.speed = (u.status.test_flag(Perseff_wither_curse)) ? SPEED_SLOW : SPEED_NORMAL;
    int defshift = !!u.status.test_flag(Perseff_leadfoot_curse) + !!u.status.test_flag(Perseff_wither_curse);
    u.defence = u.net_agility() / 5;
    u.defence >>= defshift;
    u.evasion = u.net_agility();
    if (armptr)
    {
	u.defence += u.status.test_flag(Perseff_armourmelt_curse) ? 0 : permobjs[armptr->obj_id].power;
        u.evasion *= EVASION_PRESCALE - evasion_penalty(u.armour);
	switch (armptr->obj_id)
	{
	case PO_DRAGONHIDE_ARMOUR:
	case PO_METEORIC_PLATE_ARMOUR:
	    u.resistances[DT_FIRE] |= RESIST_ARMOUR;
	    break;
	case PO_ROBE_OF_SWIFTNESS:
	    u.speed++;
	    break;
	default:
	    break;
	}
    }
    else
    {
        u.evasion *= EVASION_PRESCALE;
    }
    u.evasion >>= defshift;
    u.evasion /= EVASION_POSTSCALE;
    if (ringptr)
    {
	switch (ringptr->obj_id)
	{
	case PO_FIRE_RING:
	    u.resistances[DT_FIRE] |= RESIST_RING;
	    break;
	case PO_FROST_RING:
	    u.resistances[DT_COLD] |= RESIST_RING;
	    break;
	case PO_VAMPIRE_RING:
	    u.resistances[DT_NECRO] |= RESIST_RING;
	    break;
	}
    }
    status_updated = 1;
    display_update();
}

int farmove_player(libmrl::Coord direction)
{
    libmrl::Coord c = u.pos;
    c += direction;
    u.farmove_direction = direction;
    u.farmoving = true;
    int i = move_player(direction);
    return i;
}

int move_player(libmrl::Coord step)
{
    libmrl::Coord c = u.pos;
    c += step;
    if ((c.y < 0) || (c.y >= DUN_HEIGHT) ||
	(c.x < 0) || (c.x >= DUN_WIDTH))
    {
	print_msg(MSGCHAN_INTERROR, "Attempted move out of bounds.\n");
        disturb_u();
	return 0;	/* No movement. */
    }
    if (currlev->monster_at(c).valid())
    {
        disturb_u();
        if (u.farmoving)
        {
            return 0;
        }
        Obj const *optr = u.weapon.snapc();
	if (optr)
	{
	    if ((optr->obj_id == PO_BOW) ||
		(optr->obj_id == PO_CROSSBOW))
	    {
		print_msg(MSGCHAN_MINORFAIL, "You can't use that weapon in melee!\n");
		return -1;
	    }
	}
	return player_attack(step);
    }
    /* Now that we know it isn't an attack attempt, check for shackling and
     * impassable terrain.
     */
    if (u.shackled)
    {
        print_msg(MSGCHAN_MINORFAIL, "Your legs are chained to the spot.\n");
        disturb_u();
        return 0;
    }
    if (terrain_data[currlev->terrain_at(c)].impassable)
    {
	if (!u.farmoving)
        {
            print_msg(MSGCHAN_BORINGFAIL, "You cannot go there.\n");
        }
        disturb_u();
        return 0;
    }
    return reloc_player(c);
}

void encounter_terrain(void);

int reloc_player(libmrl::Coord c, bool override)
{
    libmrl::Coord old = u.pos;

    if (!override)
    {
        if (currlev->region_at(c) != currlev->region_at(old))
        {
            // will the level let you leave?
            if (!currlev->pre_leave_region(old))
            {
                // no.
                return 0;
            }
            // yes. do whatever happens.
            currlev->leave_region(old);
            if (game_finished)
            {
                // you died to the leave-reaction
                return 0;
            }
        }
    }
    u.pos = c;
    newsym(old);
    newsym(u.pos);
    do_vision();
    map_updated = 1;
    status_updated = 1;
    if ((terrain_data[currlev->terrain_at(u.pos)].feature) ||
        (terrain_data[currlev->terrain_at(u.pos)].hostile) ||
        (currlev->object_at(u.pos).valid()))
    {
        disturb_u();
    }
    if (currlev->region_at(c) != currlev->region_at(old))
    {
        currlev->enter_region(c);
    }
    look_at_floor();
    encounter_terrain();
    display_update();
    return 1;
}

void encounter_terrain(void)
{
    Terrain_num terr = currlev->terrain_at(u.pos);
    Cloud cld = currlev->cloud_at(u.pos);
    if (terrain_data[terr].hostile)
    {
    }
    if (cld != no_cloud)
    {
        encounter_cloud(cld);
    }
}

void look_at_floor(void)
{
    std::string itemname;
    const Obj *optr = currlev->object_at(u.pos).snapc();
    if (optr)
    {
        optr->get_name(&itemname);
	print_msg(0,"You see here %s.\n", itemname.c_str());
    }
    int t = currlev->terrain_at(u.pos);
    if (terrain_data[t].feature)
    {
        print_msg(0, "There is a%s %s here.\n", is_vowel(terrain_data[t].name[0]) ? "n" : "",
                  terrain_data[t].name);
    }
}

int gain_body(int amount, int loud)
{
    if (amount < 1)
    {
	print_msg(MSGCHAN_INTERROR, "Absurd body gain %d\n", amount);
    }
    if (u.body < 99)
    {
	if (u.body + amount > 99)
	{
	    amount = 99 - u.body;
	}
	u.body += amount;
	status_updated = 1;
	if (loud)
	{
	    print_msg(0, "You feel stronger!\n");
	}
	else
	{
	    display_update();
	}
	return amount;
    }
    else
    {
	print_msg(0, "You feel disappointed.\n");
	return 0;
    }
}

int drain_body(int amount, const char *what, int permanent)
{
    if (!amount)
    {
        return 0;
    }
    disturb_u();
    print_msg(0, "You feel weaker!\n");
    if (permanent)
    {
	u.body -= amount;
    }
    else
    {
	u.bdam += amount;
    }
    status_updated = 1;
    if ((u.body - u.bdam) < 0)
    {
	print_msg(0, "Your heart is too weak to beat.\n");
	return do_death(DEATH_BODY, what);
    }
    display_update();
    return 0;
}

int gain_agility(int amount, int loud)
{
    if (amount < 1)
    {
	print_msg(MSGCHAN_INTERROR, "Absurd agility gain %d\n", amount);
    }
    if (u.agility < 99)
    {
	if (u.agility + amount > 99)
	{
	    amount = 99 - u.agility;
	}
	u.agility += amount;
	status_updated = 1;
	recalc_defence();
	if (loud)
	{
	    print_msg(0, "You feel more agile!\n");
	}
	else
	{
	    display_update();
	}
	return amount;
    }
    else
    {
	print_msg(0, "You feel disappointed.\n");
	return 0;
    }
}

int drain_agility(int amount, const char *what, int permanent)
{
    if (!amount)
    {
        return 0;
    }
    disturb_u();
    print_msg(0, "You feel clumsy!\n");
    if (permanent)
    {
	u.agility -= amount;
    }
    else
    {
	u.adam += amount;
    }
    status_updated = 1;
    if ((u.agility - u.adam) < 0)
    {
	print_msg(0, "You forget how to breathe.\n");
	return do_death(DEATH_AGILITY, what);
    }
    recalc_defence();
    return 0;
}

int damage_u(int amount, Death d, const char *what)
{
    if (!amount)
    {
        print_msg(MSGCHAN_INTERROR, "How odd. You just took 0 damage.\n");
        return 0;
    }
    disturb_u();
    u.hpcur -= amount;
    status_updated = 1;
    if (u.hpcur < 0)
    {
	u.hpcur = 0;
	return do_death(d, what);
    }
    else
    {
        display_update();
    }
    return 0;
}

void heal_u(int amount, int boost, int loud)
{
    if (u.hpcur + amount > u.hpmax)
    {
	if (boost)
	{
	    u.hpmax++;
	}
	amount = u.hpmax - u.hpcur;
    }
    u.hpcur += amount;
    /* Touch the status line */
    status_updated = 1;
    if (loud)
    {
	/* Tell the player how much better he feels. */
	if (u.hpcur == u.hpmax)
	{
	    print_msg(0, "You feel great.\n");
	}
	else
	{
	    print_msg(0, "You feel %sbetter.\n", amount > 10 ? "much " : "");
	}
    }
    else
    {
	/* Update the display. */
	display_update();
    }
    return;
}

int do_death(Death d, const char *what)
{
    FILE *fp;
    int really = 0;
    int fd;
    std::string filename;

    if (wizard_mode)
    {
	really = getyn("Really die? ");
	if (really != 1)
	{
	    u.hpcur = u.hpmax;
	    u.adam = 0;
	    u.bdam = 0;
	    status_updated = 1;
	    print_msg(0, "\nYou survived that attempt on your life.\n");
	    return 0;
	}
    }
    if (!wizard_mode)
    {
#ifdef MULTIUSER
        filename = PLAYGROUND;
        filename += "/dunbash.log";
        game_permissions();
#else
        filename = "dunbash.log";
#endif
        fd = open(filename.c_str(), O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (fd != -1)
        {
            fp = fdopen(fd, "a");
            if (fp)
            {
                std::string datestamp;
                get_iso_8601_time(datestamp);
                fprintf(fp, "%s:%d:%s:%s:%d:%d:%d:%d:%d\n", u.name, d, what, datestamp.c_str(), game_tick, u.experience, u.gold, u.lev.dungeon, u.lev.level);
                fflush(fp);
                fclose(fp);
            }
        }
    }
    print_msg(0, "THOU ART SLAIN!\n");
    game_finished = 1;
    switch (d)
    {
    case DEATH_KILLED:
        print_msg(0, "You were killed by %s.\n", what);
        break;
    case DEATH_KILLED_MON:
        print_msg(0, "You were killed by a nasty %s.\n", what);
        break;
    case DEATH_BODY:
	print_msg(0, "Your heart was stopped by %s.\n", what);
	break;
    case DEATH_AGILITY:
	print_msg(0, "Your nerves were destroyed by %s.\n", what); 
	break;
    }
    print_msg(0, "Your game lasted %d ticks.\n", game_tick);
    print_msg(0, "You killed monsters worth %d experience.\n", u.experience);
    print_msg(0, "You found %d pieces of gold.\n", u.gold);
    press_enter();

    return 1;
}

void write_char_dump(void)
{
    FILE *fp;
    int i;
    std::string filename;
    std::string itemname;
    filename = u.name;
    filename += ".dump";
#ifdef MULTIUSER
    user_permissions();
#endif
    fp = fopen(filename.c_str(), "w");
    if (fp == NULL)
    {
	print_msg(MSGCHAN_INTERROR, "Couldn't create dump file. Dump failed.\n");
	return;
    }
    fprintf(fp, "%s, level %d adventurer (%d XP)\n", u.name, u.level, u.experience);
    fprintf(fp, "%d gold pieces collected.\n", u.gold);
    fprintf(fp, "%d of %d hit points.\n", u.hpcur, u.hpmax);
    fprintf(fp, "Body %d (%d damage).\n", u.body, u.bdam);
    fprintf(fp, "Agility %d (%d damage).\n", u.agility, u.adam);
    fprintf(fp, "Defence %d.\n", u.defence);
    fprintf(fp, "Inventory:\n");
    for (i = 0; i < 19; i++)
    {
	if (u.inventory[i].valid())
	{
            u.inventory[i].snapc()->get_name(&itemname);
            fprintf(fp, "%s\n", itemname.c_str());
	}
    }
    fflush(fp);
    fclose(fp);
#ifdef MULTIUSER
    game_permissions();
#endif
}

void u_init(void)
{
    char * hasslash = NULL;
    u.name[16] = '\0';
    u.name[0] = '\0';
    if (name_prompt)
    {
        do {
            print_msg(0, "What is your name, stranger?\n");
            read_input(u.name, 16);
            hasslash = strchr(u.name, '/');
            /* Now that we create a named dump file, we must not
             * permit the player's name to contain a slash, colon,
             * or backslash. */
            if (hasslash)
            {
                print_msg(0, "No slashes permitted.\n");
                continue;
            }
            hasslash = strchr(u.name, '\\');
            if (hasslash)
            {
                print_msg(0, "No backslashes permitted.\n");
                continue;
            }
            hasslash = strchr(u.name, ':');
            if (hasslash)
            {
                print_msg(0, "No colons permitted.\n");
                continue;
            }
        } while (hasslash != NULL);
    }
    if (strlen(u.name) == 0)
    {
        u.name[16] = '\0';
        strncpy(u.name, configured_name.c_str(), 16);
    }
    u.lev.dungeon = Dungeon_main;
    u.lev.level = 1;
    u.body = 10;
    u.bdam = 0;
    u.agility = 10;
    u.adam = 0;
    u.hpmax = 20;
    u.hpcur = 20;
    u.experience = 0;
    u.level = 1;
    u.food = 2000;
    u.inventory[0] = create_obj(PO_DAGGER, 1, 1, libmrl::NOWHERE);
    if (!u.inventory[0].valid())
    {
	print_msg(MSGCHAN_INTERROR, "Couldn't create dagger!\n");
    }
    u.inventory[1] = create_obj(PO_IRON_RATION, 1, 1, libmrl::NOWHERE);
    if (!u.inventory[1].valid())
    {
	print_msg(MSGCHAN_INTERROR, "Couldn't create ration!\n");
    }
    u.weapon = u.inventory[0];
    u.ring = NO_OBJECT;
    u.armour = NO_OBJECT;
    recalc_defence();
}

unsigned lev_threshold(int level)
{
    if (level < 10)
    {
	return 20u * (1 << (level - 1));
    }
    if (level < 20)
    {
	return 10000u * (level - 9);
    }
    if (level < 30)
    {
	return 100000u * (level - 18);
    }
    return INT_MAX;
}

void gain_experience(int amount)
{
    int hpgain;
    int bodygain;
    int agilgain;
    u.experience += amount;
    status_updated = 1;
    if (u.experience > lev_threshold(u.level))
    {
	u.level++;
	print_msg(0, "You gained a level!\n");
	if (!zero_die(2))
	{
	    bodygain = gain_body(2, 0);
	    agilgain = gain_agility(1, 0);
	}
	else
	{
	    bodygain = gain_body(1, 0);
	    agilgain = gain_agility(2, 0);
	}
	print_msg(MSGCHAN_NUMERIC, "You gained %d body and %d agility.\n", bodygain, agilgain);
	hpgain = u.body / 10 + 10;
	if (u.hpmax + hpgain > 999)
	{
	    hpgain = 999 - u.hpmax;
	}
	if (hpgain > 0)
	{
            /* Gaining a level effectively heals you. */
	    u.hpcur += hpgain;
	    u.hpmax += hpgain;
	    status_updated = 1;
	    print_msg(MSGCHAN_NUMERIC, "You gained %d hit points.\n", hpgain);
	}
        if (levelup_wait)
        {
            press_enter();
        }
    }
    else
    {
	display_update();
    }
}

int teleport_u(void)
{
    int cell_try;
    libmrl::Coord pos;
    disturb_u();
    for (cell_try = 0; cell_try < 400; ++cell_try)
    {
        pos.y = exclusive_flat(0, currlev->height - 1);
        pos.x = exclusive_flat(0, currlev->width - 1);
        if ((currlev->monster_at(pos) == NO_MONSTER) && (!terrain_data[currlev->terrain_at(pos)].impassable) && (pos != u.pos))
        {
            print_msg(0, "You are whisked away!\n");
            reloc_player(pos);
            return 0;
        }
    }
    print_msg(MSGCHAN_MINORFAIL, "You feel briefly dislocated.\n");
    return -1;
}

void Player::apply_effect(Perseff_data& peff)
{
    peff.on_you = true;
    perseffs.push_back(peff);
}

void Player::suffer(Perseff_data& peff)
{
    switch (peff.flavour)
    {
    case Perseff_bitter_chill:
        print_msg(0, "Bitter cold numbs your flesh.\n");
        damage_u(one_die(peff.power), DEATH_KILLED, "bitter cold");
        break;
    case Perseff_searing_flames:
        print_msg(0, "Searing flames burn your flesh.\n");
        damage_u(one_die(peff.power), DEATH_KILLED, "searing flames");
        break;
    default:
        break;
    }
}

void update_player(void)
{
    if ((!(game_tick % 5)) && (u.food >= 0) && (u.hpcur < u.hpmax))
    {
	/* Heal player for one hit point; do not allow HP gain,
	 * and don't say anything. */
	heal_u(1, 0, 0);
    }
    else if (!(game_tick % 60) && (u.hpcur < u.hpmax * 3 / 4))
    {
	/* Hungry player heals much, much slower, and cannot regain
	 * all their hit points. */
	heal_u(1, 0, 0);
    }
    /* Once you hit the nutrition endstop, your ring of regeneration stops
     * working, and like normal regen, it won't raise you above 75% HP if
     * your food counter is negative. */
    if (((game_tick % 10) == 5) &&
	(u.ring.otyp() == PO_REGENERATION_RING) &&
	(u.hpcur < ((u.food >= 0) ? u.hpmax : ((u.hpmax * 3) / 4))) &&
	(u.food >= -1950))
    {
	/* Heal player for 1d3 hit points; do not allow HP gain,
	 * and don't say anything apart from the regen ring message. */
	print_msg(MSGCHAN_FLUFF, "Your ring pulses soothingly.\n");
	heal_u(one_die(3), 0, 0);
	permobjs[PO_REGENERATION_RING].known = 1;
    }
    if (u.food >= -1950)
    {
        // Base food use is 1 on SPEED_NORMAL or slower ticks, 0 on SPEED_FAST
        // or faster ticks.
	int food_use = (action_speed <= SPEED_NORMAL) ? 1 : 0;
	int squeal = 0;
	if ((u.ring.otyp() == PO_REGENERATION_RING) && !(game_tick % 2) && (u.food >= -1950))
	{
	    /* If you are still less hungry than -1950 nutrition,
	     * use one more food every second game tick if you are
	     * wearing a ring of regeneration. */
	    food_use++;
	}
	if ((u.food >= 100) && (u.food - food_use < 100))
	{
	    squeal = 1;
	}
	if ((u.food >= 0) && (u.food < food_use))
	{
	    squeal = 2;
	}
	u.food -= food_use;
	status_updated = 1;
	switch (squeal)
	{
	case 0:
	default:
	    break;
	case 1:
	    print_msg(0, "You are getting quite hungry.\n");
	    break;
	case 2:
	    print_msg(0, "You are feeling hunger pangs, and will recover\nmore slowly from your injuries.\n");
	    break;
	}
    }
    if (!u.perseffs.empty())
    {
        bool wiped = false;
        std::list<Perseff_data>::iterator peff_iter;
        std::list<Perseff_data>::iterator peff_next;
        Status_flags saved_status = u.status;
        u.status.clear_all();
        for (peff_iter = u.perseffs.begin();
             peff_iter != u.perseffs.end();
             peff_iter = peff_next)
        {
            peff_next = peff_iter;
            ++peff_next;
            if ((*peff_iter).duration > 0)
            {
                (*peff_iter).duration--;
            }
            if ((*peff_iter).duration)
            {
                /* Act on the debuff */
                u.status.set_flag((*peff_iter).flavour);
                u.suffer(*peff_iter);
            }
            else
            {
                switch (peff_iter->flavour)
                {
                case Perseff_bitter_chill:
                    print_msg(0, "The bitter chill subsides.\n");
                    break;

                case Perseff_searing_flames:
                    print_msg(0, "The flames around you subside.\n");
                    break;

                case Perseff_leadfoot_curse:
                    print_msg(0, "You shed your feet of lead.\n");
                    break;

                case Perseff_wither_curse:
                    print_msg(0, "Your limbs straighten.\n");
                    break;

                case Perseff_armourmelt_curse:
                    print_msg(0, "Your armour looks less fragile now.\n");
                    break;

                case Perseff_binding_chains:
                    print_msg(0, "The chains binding you disintegrate.\n");
                    break;
                    
                case Perseff_tentacle_embrace:
                    print_msg(0, "Your tentacular ordeal is over.\n");
                    break;

                default:
                    break;
                }
                u.perseffs.erase(peff_iter);
                recalc_defence();
            }
            if (wiped)
            {
                break;
            }
        }
    }
    do_vision();
    display_update();
}

int player_resists_dtype(Damtyp dtype)
{
    return u.resistances[dtype];
}

void disturb_u(void)
{
    u.farmoving = false;
    u.farmove_direction = libmrl::NOWHERE;
}

bool player_next_to_mon(void)
{
    libmrl::Coord c;
    for (c.y = -1; c.y <= 1; ++(c.y))
    {
        for (c.x = -1; c.x <= 1; ++(c.x))
        {
            if (!int(c))
            {
                continue;
            }
            if (currlev->monster_at(u.pos + c).valid())
            {
                return true;
            }
        }
    }
    return false;
}

void describe_profession(Player_profession prof)
{
    switch (prof)
    {
    case Prof_fighter:
        print_msg(0,
                  "As a fighter, you have an instinct for weaponry and an\n"
                  "unparallelled talent for physical violence. The Violence\n"
                  "score on your status bar is kind of like 'mana' for your\n"
                  "special abilities; it accumulates whenever a monster\n"
                  "attempts to do you harm, and is spent when you use your\n"
                  "active abilities.\n\n");
        print_msg(0,
                  "   Vigour - This passive ability means you gain two Body\n"
                  "   and one Agility per level.\n\n");
        print_msg(0,
                  "1: Whirlwind - This active ability attacks all adjacent\n"
                  "   monsters at once. Costs 30 Violence and has a 20-turn\n"
                  "   cooldown timer.\n\n");
        print_msg(0,
                  "2: Slam - This active ability knocks a monster backwards\n"
                  "   one square, moves you into the square it was knocked\n"
                  "   out of, and stuns it for one turn. Costs 30 Violence\n"
                  "   and has a 5-turn cooldown timer.\n\n");
        print_msg(0,
                  "3: Smash - Attack an adjacent monster for double damage.\n"
                  "   Costs 20 Violence and has no cooldown timer.\n\n");
        print_msg(0,
                  "4: Berserker Rage - Increases your physical damage done\n"
                  "   by 50%% for 20 turns. Empties your Violence pool and\n"
                  "   has a 200-turn cooldown timer.\n\n");
        break;

    case Prof_preacher:
        break;

    case Prof_thanatophile:
        print_msg(0,
                  "The thanatophile is obsessed with death. Not the daed,\n"
                  "*death*. (S)he kills to make death.\n\n");
        print_msg(0,
                  "   Fell Spirit - This passive ability means you gain one\n"
                  "   Body, one Agility, and a ten-point increase to your\n"
                  "   Power pool per level.\n\n");
        print_msg(0,
                  "1: Assassin Soul - When you activate this ability, the\n"
                  "   darkness of your soul will hide you from the senses\n"
                  "   of those around you. Attacking an enemy will break\n"
                  "   the concealment immediately. You cannot activate\n"
                  "   this power if you have engaged in combat recently,\n"
                  "   and some monsters will see straight through it. You\n"
                  "   do not regenerate Power while this ability is active.\n\n");
        print_msg(0,
                  "2: Death Song - While this ability is active, your melee\n"
                  "   attacks do bonus Death damage based on your level. Each\n"
                  "   melee attack while this ability is active consumes two\n"
                  "   points of power.\n\n");
        print_msg(0,
                  "3: Life Leech - This ability drains hit points from an\n"
                  "   adjacent target. Even undead and demons have something\n"
                  "   worth draining. Particularly powerful opponents may\n"
                  "   partially resist this effect.\n\n");
        print_msg(0,
                  "4: Corpse Explosion - Undeath offends you. Those granted\n"
                  "   the gift of Death should not be deprived of it. To this\n"
                  "   end, you have learned how to use your fell power to make\n"
                  "   the mortal remains of the recently slain explode with\n"
                  "   great violence.\n\n");
        break;
    }
}

/* u.cc */
