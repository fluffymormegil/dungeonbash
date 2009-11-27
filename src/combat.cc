/* combat.cc
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
#include "combat.hh"
#include "perseff.hh"
#include "objects.hh"
#include "pobjid.hh"
#include "pmonid.hh"
#include "monsters.hh"

int combat_timer = 0;

void renew_combat_timer(void)
{
    combat_timer = 10;
}

void step_combat_timer(void)
{
    if (combat_timer > 0)
    {
        --combat_timer;
    }
}

void flush_combat_timer(void)
{
    combat_timer = 0;
}

bool player_in_combat(void)
{
    return (combat_timer > 0);
}

int player_attack(libmrl::Coord dir)
{
    renew_combat_timer();
    if (u.weapon.valid() && ((u.weapon.snapc()->obj_id == PO_BOW) || (u.weapon.snapc()->obj_id == PO_CROSSBOW)))
    {
	ushootm(dir);
    }
    else
    {
        dir += u.pos;
        Mon_handle mh = currlev->monster_at(dir);
        if (mh.valid())
        {
            uhitm(mh);
        }
        else
        {
            print_msg(MSGCHAN_MINORFAIL, "You attack empty air.\n");
        }
    }
    return 2;
}

int uhitm(Mon_handle mon)
{
    Mon *mp;
    Obj *wep;
    Permobj *pwep;
    Obj *ring;
    Permobj *pring;
    int tohit;
    int damage;
    int healing;

    mp = mon.snapv();
    mp->notice_you(true);
    tohit = zero_die(u.agility + u.level);
    if (tohit < mp->defence)
    {
        print_msg(0, "You miss.\n");
        return 0;	/* Missed. */
    }
    std::string victim_name;
    mp->get_name(&victim_name, 1, false);
    print_msg(0, "You hit %s.\n", victim_name.c_str());
    wep = u.weapon.snapv();
    ring = u.ring.snapv();
    if (wep)
    {
        pwep = permobjs + wep->obj_id;
        damage = one_die(pwep->power) + (u.net_body() / 10);
    }
    else
    {
        damage = u.net_body() / 10;
    }
    if (ring)
    {
        pring = permobjs + ring->obj_id;
        switch (ring->obj_id)
        {
        case PO_FIRE_RING:
            if (!pmon_resists_fire(mp->mon_id))
            {
                if (!pring->known)
                {
                    pring->known = 1;
                }
                print_msg(0, "Your ring burns %s!\n", victim_name.c_str());
                damage += (damage + 1) / 2 + dice(2, 4);
            }
            break;
        case PO_VAMPIRE_RING:
            if (!pmon_is_undead(mp->mon_id))
            {
                if (!pring->known)
                {
                    pring->known = 1;
                }
                print_msg(0, "Your ring drains %s!\n", victim_name.c_str());
                damage += (damage + 3) / 4 + dice(2, 4);
                healing = (damage + 5) / 6;
                heal_u(healing, 0, 1);
            }
            break;
        case PO_FROST_RING:
            if (!pmon_resists_cold(mp->mon_id))
            {
                if (!pring->known)
                {
                    pring->known = 1;
                }
                print_msg(0, "Your ring freezes %s!\n", victim_name.c_str());
                damage += (damage + 2) / 3 + dice(1, 6);
            }
        }
    }
    print_msg(MSGCHAN_NUMERIC, "You do %d damage.\n", damage);
    damage_mon(mon, damage, 1);
    if (u.weapon.valid())
    {
        damage_obj(u.weapon);
    }
    return 1;	/* Hit. */
}

int ushootm(libmrl::Coord dir)
{
    /* Propagate a missile in direction (sy,sx). Attack first target in
     * LOF. */
    int tohit;
    int range;
    libmrl::Coord pos = u.pos;
    int done = 0;
    Mon *mptr;
    Obj *wep;
    Permobj *pwep;
    int damage;
    int rv = 0;
    std::string victim_name;
    renew_combat_timer();
    wep = u.weapon.snapv();
    pwep = permobjs + wep->obj_id;
    damage = one_die(pwep->power);
    pos += dir;
    range = 1;
    for ( ; !done; (pos += dir), ++range)
    {
        Mon_handle mh;
        animate_projectile(pos, DBCLR_L_GREY);
        mh = currlev->monster_at(pos);
        mptr = mh.snapv();
        if (mptr)
        {
            done = true;
            tohit = zero_die(u.agility + u.level - (range >> 1));
            mptr->get_name(&victim_name, 1, false);
            if (range == 1)
            {
                /* Shooting at point-blank is tricky */
                tohit = (tohit + 1) / 2;
            }
            if (tohit >= mptr->defence)
            {
                if (mon_visible(currlev->monster_at(pos)))
                {
                    print_msg(0, "You hit %s.\n", victim_name.c_str());
                    print_msg(MSGCHAN_NUMERIC, "You do %d damage.\n", damage);
                }
                damage_mon(currlev->monster_at(pos), damage, 1);
                rv = 1;
                done = true;
            }
            else
            {
                print_msg(MSGCHAN_MINORFAIL, "You miss %s.\n", victim_name.c_str());
                done = true;
            }
        }
        else if ((currlev->terrain_at(pos) == WALL) || (currlev->terrain_at(pos) == DOOR))
        {
            print_msg(MSGCHAN_BORINGFAIL, "Your %s hits the %s.\n", (wep->obj_id == PO_BOW) ? "arrow" : "bolt", (currlev->terrain_at(pos) == WALL) ? "wall" : "door");
            done = true;
        }
    }
    projectile_done();
    damage_obj(u.weapon);
    return rv;
}

int mhitu(Mon_handle mon, Damtyp dtype)
{
    int tohit;
    int damage;
    bool unaffected;
    std::string attackername;
    std::string Attackername;
    Mon *mptr = mon.snapv();
    renew_combat_timer();
    mptr->get_name(&attackername, 1);
    mptr->get_name(&Attackername, 3);
    tohit = zero_die(mptr->mtohit + 5);
    if (tohit < u.defence)
    {
        /* Note: Yes, all attacks can damage your armour. Deal. */
        if ((u.armour.valid()) && (tohit > agility_modifier()))
        {
            /* Monster hit your armour. */
            print_msg(0, "Your armour %s %s's blow.\n", zero_die(2) ? "absorbs" : "deflects", attackername.c_str());
            damage_obj(u.armour);
        }
        else
        {
            print_msg(0, "%s misses you.\n", Attackername.c_str());
        }
        return 0;
    }
    damage = one_die(mptr->mdam);
    unaffected = player_resists_dtype(dtype);
    print_msg(0, "%s hits you.\n", Attackername.c_str());
    if ((u.armour.valid()) && u.status.test_flag(Perseff_armourmelt_curse) && (!zero_die(3)))
    {
        /* If you're subject to armourmelt, it is decreed that one
         * hit in three tears bits off your dust-weak armour. */
        damage_obj(u.armour);
    }
test_unaffected:
    if (unaffected)
    {
        switch (dtype)
        {
        case DT_PHYS:
            print_msg(MSGCHAN_INTERROR, "Can't happen: player resisting physical damage\n");
            unaffected = false;
            /* Turn off the player's resistance, because they're
             * not supposed to have it! */
            u.resistances[DT_PHYS] = 0;
            goto test_unaffected;
        case DT_FIRE:
            print_msg(0, "The flames seem pleasantly warm.\n");
            if (unaffected & RESIST_RING)
            {
                print_msg((permobjs[PO_FIRE_RING].known ? MSGCHAN_TAUNT : 0), "Your ring flashes red.\n");
                permobjs[PO_FIRE_RING].known = 1;
            }
            break;
        case DT_COLD:
            print_msg(0, "Its touch seems pleasantly cool.\n");
            if (unaffected & RESIST_RING)
            {
                print_msg((permobjs[PO_FIRE_RING].known ? MSGCHAN_TAUNT : 0), "Your ring flashes blue.\n");
                permobjs[PO_FROST_RING].known = 1;
            }
            break;
        case DT_NECRO:
            print_msg(0, "Its touch makes you feel no deader.\n");
            if (unaffected & RESIST_RING)
            {
                print_msg((permobjs[PO_FIRE_RING].known ? MSGCHAN_TAUNT : 0), "Your ring shrieks.\n");
                permobjs[PO_VAMPIRE_RING].known = 1;
            }
            break;
        case DT_ELEC:
            print_msg(0, "Tingly blue lightning plays across your skin.\n");
            break;
        case DT_POISON:
            print_msg(0, "You feel faintly nauseous.\n");
            break;
        default:
            print_msg(MSGCHAN_INTERROR, "Can't happen: bogus damage type.\n");
            break;
        }
    }
    else
    {
        switch (dtype)
        {
        default:
        case DT_PHYS:
            break;
        case DT_FIRE:
            print_msg(0, "You are engulfed in flames.\n");
            break;
        case DT_COLD:
            print_msg(0, "You are covered in frost.\n");
            break;
        case DT_NECRO:
            print_msg(0, "You feel your life force slipping away.\n");
            break;
        case DT_ELEC:
            print_msg(0, "Electricity shoots through you.\n");
            break;
        case DT_POISON:
            print_msg(0, "You feel sick.\n");
            break;
        }
        print_msg(MSGCHAN_NUMERIC, "You take %d damage.\n", damage);
        if ((mptr->mon_id == PM_VAMPIRE) && !player_resists_dtype(DT_NECRO))
        {
            heal_mon(mon, damage * 2 / 5, 1);
        }
        else if ((tohit - u.defence >= 5) && (mptr->mon_id == PM_SNAKE) &&
                 !player_resists_dtype(DT_POISON))
        {
            drain_body(1, "snake venom", 0);
        }
        damage_u(damage, DEATH_KILLED_MON, permons[mptr->mon_id].name);
        display_update();
        if (pmon_is_tele_harasser(mptr->mon_id) &&
            !zero_die(8 - permons[mptr->mon_id].speed))
        {
            teleport_mon(mon);
            print_msg(0, "%s teleports away!\n", Attackername.c_str());
        }
    }
    return 1;
}

int mshootu(Mon_handle mon, Damtyp dtyp)
{
    Mon *mptr;
    Mon *bystander;
    libmrl::Coord pos;
    Direction_data dirdata;
    bool done;
    int unaffected = false;
    int tohit;
    int damage;
    int defence;
    int rv = 0;
    Dbash_colour col;
    std::string attackername;
    std::string Attackername;
    mptr = mon.snapv();
    mptr->get_name(&attackername, 1);
    mptr->get_name(&Attackername, 3);
    pos = mptr->pos;

    compute_directions(u.pos, pos, &dirdata);
    /* Don't get the bonus that applies to melee attacks. */
    tohit = zero_die(mptr->rtohit);
    if (dtyp == DT_PHYS)
    {
        print_msg(0, "%s %s at you!\n", Attackername.c_str(), permons[mptr->mon_id].ranged.verb);
    }
    else
    {
        print_msg(0, "%s %s %s at you!\n", Attackername.c_str(), permons[mptr->mon_id].ranged.verb, damtype_names[dtyp]);
    }
    if ((dtyp == DT_NECRO) || (dtyp == DT_ELEC))
    {
        /* Use agility-based defence for necromantic blasts and lightning
         * bolts */
        defence = u.evasion;
    }
    else
    {
        defence = u.defence;
    }
    switch (dtyp)
    {
    case DT_PHYS:
    default:
        col = DBCLR_L_GREY;
        break;
    case DT_COLD:
        col = DBCLR_WHITE;
        break;
    case DT_ELEC:
        col = DBCLR_L_BLUE;
        break;
    case DT_NECRO:
        col = DBCLR_D_GREY;
        break;
    case DT_FIRE:
        col = DBCLR_L_RED;
        break;
    case DT_POISON:
        col = DBCLR_L_GREEN;
        break;
    }
    /* Move projectile one square before looking for targets. */
    for ((done = false), (pos += dirdata.sign);
         !done;
         pos += dirdata.sign)
    {
        animate_projectile(pos, col);
        if ((currlev->terrain_at(pos) == WALL) || (currlev->terrain_at(pos) == DOOR))
        {
            done = true;
        }
        if (pos == u.pos)
        {
            if (tohit >= defence)
            {
                done = true;
                print_msg(0, "It hits you!\n");
                unaffected = player_resists_dtype(dtyp);
                if (unaffected)
                {
                    /* For now, resistant armours are always known, so
                     * we only need to check for identification of rings. */
                    if (unaffected & RESIST_RING)
                    {
                        switch (dtyp)
                        {
                        case DT_COLD:
                            print_msg((permobjs[PO_FROST_RING].known ? MSGCHAN_TAUNT : 0), "Your ring flashes blue.\n");
                            permobjs[PO_FROST_RING].known = 1;
                            break;
                        case DT_FIRE:
                            print_msg((permobjs[PO_FIRE_RING].known ? MSGCHAN_TAUNT : 0), "Your ring flashes red.\n");
                            permobjs[PO_FIRE_RING].known = 1;
                            break;
                        case DT_NECRO:
                            print_msg((permobjs[PO_VAMPIRE_RING].known ? MSGCHAN_TAUNT : 0), "Your ring shrieks.\n");
                            permobjs[PO_VAMPIRE_RING].known = 1;
                            break;
                        default:
                            break;
                        }
                    }
                }
                else
                {
                    damage = one_die(mptr->rdam);
                    print_msg(MSGCHAN_NUMERIC, "You take %d damage.\n", damage);
                    damage_u(damage, DEATH_KILLED_MON, permons[mptr->mon_id].name);
                }
                display_update();
                rv = 1;
            }
            else
            {
                print_msg(0, "It misses you.\n");
            }
        }
        else if (currlev->monster_at(pos).valid())
        {
            done = true;
            bystander = currlev->monster_at(pos).snapv();
            switch (dtyp)
            {
            case DT_COLD:
                if (pmon_resists_cold(bystander->mon_id))
                {
                    unaffected = true;
                }
                else
                {
                    unaffected = false;
                }
                break;
            case DT_FIRE:
                if (pmon_resists_fire(bystander->mon_id))
                {
                    unaffected = true;
                }
                else
                {
                    unaffected = false;
                }
                break;
            case DT_NECRO:
                if (pmon_is_undead(bystander->mon_id))
                {
                    unaffected = true;
                }
                else
                {
                    unaffected = false;
                }
                break;
            default:
                unaffected = false;
                break;
            }
            if (tohit >= bystander->defence)
            {
                damage = one_die(mptr->rdam);
                damage_mon(currlev->monster_at(pos), dtyp, 0);
            }
        }
    }
    projectile_done();
    return rv;
}

/* combat.c */
