/* prof-thanato.cc - thanatophile professional abilities for Martin's Dungeon Bash
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

#define PROF_THANATO_CC
#include "dunbash.hh"
#include "combat.hh"
#include "radiance.hh"

const int thanato_cooldowns[] =
{
    0
};

const int thanato_costs[] =
{
    0
};

bool out_of_power(Player *ptmp, int required)
{
    if (ptmp->mpcur < required)
    {
        print_msg(0, "You are not sufficiently in touch with Death to do that.");
        return true;
    }
    return false;
}

int do_assassin_soul(Player *ptmp)
{
    if (out_of_power(ptmp, thanato_costs[Thanato_assassin_soul]))
    {
        return 0;
    }
    if (ptmp->cooldowns[Thanato_assassin_soul])
    {
        // no direct cooldown, so give the player an unsettling message.
        print_msg(0, "You feel oddly unprepared to veil yourself in darkness.");
        return 0;
    }
    if (ptmp->in_combat())
    {
        print_msg(0, "Death's eye is on you and your deeds. This is no time to hide.");
        return 0;
    }
    return 1;
}

int do_death_song(Player *ptmp)
{
    // Death Song is free to activate, but shuts down if you go OOM, and
    // casting it locks out your power regen for 50 turns.
    if (out_of_power(ptmp, thanato_costs[Thanato_assassin_soul]))
    {
        return 0;
    }
    // As with Assassin Soul, Death Song normally has no cooldown.
    if (ptmp->cooldowns[Thanato_death_song])
    {
        print_msg(0, "You have a sore throat.");
        return 0;
    }
    return 1;
}

int do_life_leech(Player *ptmp)
{
    return 0;
}


int do_thanato_explosion(Player *ptmp)
{
    // TODO Implement thanatophile corpse explosion
    // XXX This should be a smite-targeted effect.
    // XXX Incomplete remains generate smaller blasts.
    return 0;
}

/* prof-thanato.cc */
