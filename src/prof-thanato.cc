/* prof-thanato.cc - thanatophile profession for Martin's Dungeon Bash
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
#include "ui.hh"
#include "combat.hh"
#include "radiance.hh"
#include <limits.h>

const int thanato_cooldowns[] =
{
    0, // blank ability
    0, // assassiin soul can be freely toggled/detoggled
    0, // death song can be freely toggled/detoggled
    5, // you can't just stand there and spam life leech
    20 // corpse explosion should be a special effort
};

const int thanato_costs[] =
{
    0, // blank ability
    0, // AS toggle-on cost is zero
    0, // DS toggle-on cost is zero
    5, // constant cost for life leech
    20 // high constant cost for corpse explosion
};

int do_assassin_soul(Player *ptmp)
{
    if (ptmp->status.test_flag(Perseff_assassin_soul))
    {
        ptmp->dispel_effects(Perseff_assassin_soul, INT_MAX);
        // Detoggling takes no time.
        return 0;
    }
    // You can't go cloaked while singing.
    if (ptmp->status.test_flag(Perseff_death_song))
    {
        print_msg(0, "The song of Death is too loud to conceal.");
        return 0;
    }
    if (ptmp->check_mana(thanato_costs[Thanato_assassin_soul]))
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
    // Invoking the ability again detoggles it.
    if (ptmp->status.test_flag(Perseff_death_song))
    {
        ptmp->dispel_effects(Perseff_death_song, INT_MAX);
        // Detoggling takes no time.
        return 0;
    }

    // Death Song is free to activate, but shuts down if you go OOM, and
    // casting it locks out your power regen for 50 ticks.
    if (ptmp->check_mana(thanato_costs[Thanato_death_song]))
    {
        return 0;
    }
    // As with Assassin Soul, Death Song normally has no cooldown.
    if (ptmp->cooldowns[Thanato_death_song])
    {
        print_msg(0, "You have a sore throat.");
        return 0;
    }
    // Activating the ability voids cloak.
    if (ptmp->status.test_flag(Perseff_assassin_soul))
    {
        print_msg(0, "The song of Death breaks your cover.");
        ptmp->dispel_effects(Perseff_assassin_soul, INT_MAX);
    }
    Perseff_data peff = { Perseff_death_song, 1, -1, true, true };
    Perseff_data peff2 = { Perseff_suppress_mana_regen, 1, 50, true, true };
    ptmp->apply_effect(peff);
    ptmp->apply_effect(peff2);
    return 1;
}

int do_life_leech(Player *ptmp)
{
    // TODO Implement thanatophile life leech power
    if (ptmp->check_mana(thanato_costs[Thanato_life_leech]))
    {
        return 0;
    }
    if (ptmp->cooldowns[Thanato_life_leech])
    {
        print_msg(0, "You are not ready to receive more life essence.");
        return 0;
    }
    // Roughly speaking, anything capable of acting has life essence to drain.
    // Yes, this includes the undead. If they didn't have any life essence,
    // they would simply be the _dead_.
    //
    // There will be _consequences_ for leeching demons and undeads in future.
    // Whether they're worth it is a question for the player to answer.
    libmrl::Coord step;
    Mon_handle mon;
    int i;
    i = ptmp->get_adjacent_monster(&mon, &step);
    if (i == -1)
    {
        print_msg(0, "You steal no life.");
        return 0;
    }
    if (mon.valid())
    {
        // Life leech is autohit.
    }
    return 0;
}


int do_thanato_explosion(Player *ptmp)
{
    if (ptmp->check_mana(thanato_costs[Thanato_corpse_explosion]))
    {
        return 0;
    }
    if (ptmp->cooldowns[Thanato_corpse_explosion])
    {
        print_msg(0, "You are not ready to sunder the dead.");
        return 0;
    }
    // TODO Implement thanatophile corpse explosion
    // XXX This should be a smite-targeted effect.
    // XXX Incomplete remains generate smaller blasts.
    libmrl::Coord tgt;
    int i;
    i = get_smite_target(&tgt, true);
    if (i < 0)
    {
        print_msg(0, "Never mind.");
        return 0;
    }
    // We spiralpath from the target point all the way to range 10, building
    // a list of all corpses and a list of all corporeal undeads. The first
    // corpse or undead we find yields an explosion; the rest simply get
    // removed from play.
    std::list<Obj_handle> corpses;
    std::list<Mon_handle> undeads;
    return 0;
}

/* prof-thanato.cc */
