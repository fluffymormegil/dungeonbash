/* prof-fighter.cc - fighter professional abilities for Martin's Dungeon Bash
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

#define PROF_FIGHTER_CC
#include "dunbash.hh"
#include "combat.hh"
#include "radiance.hh"

const int fighter_cooldowns[] = 
{
    0, // no ability 0 yet
    30,
    5,
    0,
    200
};

const int fighter_costs[] =
{
    0, // no ability 0 yet
    30,
    20,
    30,
    25
};

bool out_of_violence(Player *ptmp, int required)
{
    if (ptmp->mpcur < required)
    {
        print_msg(0, "You are not inspired to such extreme Violence.");
        return true;
    }
    return false;
}

int do_fighter_slam(Player *ptmp)
{
    if (out_of_violence(ptmp, fighter_costs[Fighter_slam]))
    {
        return 0;
    }
    int cooldown_multiplier = 1;
    if (ptmp->cooldowns[Fighter_slam])
    {
        print_msg(0, "Your are not ready to slam another foe (%d ticks to go)", ptmp->cooldowns[Fighter_slam]);
        return 0;
    }
    // get an adjacent monster.
    libmrl::Coord step;
    int i;
    Mon_handle mon;
    i = select_dir(&step);
    if (i == -1)
    {
        return 0;
    }
    mon = currlev->monster_at(ptmp->pos + step);
    if (mon.valid())
    {
        // slam is autohit, so is a good way to deal with a monster you've got
        // pinned against a wall.
        i = knock_mon(mon, step, ptmp->body, true);
        switch (i)
        {
        case 0: // You bounced off.
            print_msg(0, "You bounce uselessly off your foe. Your shoulder hurts now.");
            // take 1d10 damage, non-lethal.
            damage_u(libmrl::min(ptmp->hpcur - 1, one_die(10)), DEATH_KILLED, "program bug");
            cooldown_multiplier = 2;
            break;
        case 1: // Successful slam.
            move_player(step);
            print_msg(0, "You force your foe back!");
            break;
        case 2: // Bounced them off the wall.
            print_msg(0, "Your foe is slammed into the scenery!");
            damage_mon(mon, one_die(ptmp->body), true);
            break;
        case 3:
            print_msg(0, "That foe is not accessible.");
            return 0;
        default:
            print_msg(MSGCHAN_INTERROR, "Invalid knock_mon result %d\n", i);
            break;
        }
    }
    else
    {
        // this will want revising if I implement invisibility.
        print_msg(0, "There is no monster there.");
        return 0;
    }
    ptmp->spellcast(fighter_costs[Fighter_slam], Fighter_slam, fighter_cooldowns[Fighter_slam] * cooldown_multiplier);
    return 1;
}

int do_fighter_berserk(Player *ptmp)
{
    if (out_of_violence(ptmp, fighter_costs[Fighter_berserk]))
    {
        return 0;
    }
    if (ptmp->cooldowns[Fighter_berserk])
    {
        print_msg(0, "You are too tired to go berserk (%d ticks to go)", ptmp->cooldowns[Fighter_berserk]);
        return 0;
    }
    Perseff_data peff = 
    {
        Perseff_berserk, 10, 20, true, true
    };
    ptmp->apply_effect(peff);
    if (!ptmp->status.test_flag(Perseff_berserk))
    {
        print_msg(0, "Something inhibits your rage.");
        return 0;
    }
    print_msg(0, "You go berserk!");
    ptmp->spellcast(ptmp->mpcur, Fighter_berserk, fighter_cooldowns[Fighter_berserk]);
    return 1;
}

int do_fighter_smash(Player *ptmp)
{
    if (out_of_violence(ptmp, fighter_costs[Fighter_smash]))
    {
        return 0;
    }
    if (ptmp->cooldowns[Fighter_smash])
    {
        print_msg(0, "You are not ready to make such a powerful blow (%d ticks to go)", ptmp->cooldowns[Fighter_smash]);
        return 0;
    }
    // Get a target.
    libmrl::Coord step;
    int i;
    Mon_handle mon;
    i = select_dir(&step);
    if (i == -1)
    {
        return 0;
    }
    mon = currlev->monster_at(ptmp->pos + step);
    if (mon.valid())
    {
        uhitm(mon);
    }
    else
    {
        // this will want revising if I implement invisibility.
        print_msg(0, "There is no monster there.");
        return 0;
    }
    ptmp->spellcast(fighter_costs[Fighter_smash], Fighter_smash, fighter_cooldowns[Fighter_smash]);
    return 1;
}

int do_fighter_whirl(Player *ptmp)
{
    Mon_handle mons[8];
    int i;
    int j;
    // If your weapon could theoretically break as a result, you can't whirlwind.
    if (ptmp->weapon.valid() && (ptmp->weapon.snapv()->durability < 9))
    {
        print_msg(0, "Your weapon is too badly damaged to consider that manoeuvre.");
        return 0;
    }
    if (ptmp->cooldowns[Fighter_whirlwind])
    {
        print_msg(0, "You feel too dizzy to execute that manoeuvre.");
        return 0;
    }
    if (out_of_violence(ptmp, fighter_costs[Fighter_whirlwind]))
    {
        return 0;
    }
    for (i = 0; i < 8; ++i)
    {
        mons[i] = Mon_handle(0);
    }
    for ((i = 1), (j = 0); i < 9; ++i)
    {
        Mon_handle tmp;
        tmp = currlev->monster_at(ptmp->pos + spiral_path[i]);
        if (tmp.valid())
        {
            mons[j++] = tmp;
        }
    }
    if (j == 0)
    {
        print_msg(0, "There is nobody for you to hit.");
        return 0;
    }
    ptmp->spellcast(fighter_costs[Fighter_whirlwind], Fighter_whirlwind, fighter_cooldowns[Fighter_whirlwind]);
    for (i = 0; (i < 8) && mons[i].valid(); ++i)
    {
        uhitm(mons[i]);
        if (game_finished)
        {
            break;
        }
    }
    return 1;
}

/* prof-fighter.cc */
