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

bool out_of_violence(int required)
{
    if (u.violence < required)
    {
        print_msg(0, "You are not inspired to such extreme Violence.");
        return 0;
    }
}

int uberserk(void)
{
    if (out_of_violence(25))
    {
        return 0;
    }
    if ((u.berserk_tick + BERSERK_COOLDOWN) < game_tick)
    {
        print_msg(0, "You must wait another %d turns before going berserk again.",
                  200 - (game_tick - u.berserk_tick));
        return 0;
    }
    Perseff_data peff = 
    {
        Perseff_berserk, 10, 20, true, true
    }:
    u.apply_effect(peff);
    if (!u.status.test_flag(Perseff_berserk))
    {
        print_msg(0, "Something inhibits your rage.");
        return 0;
    }
    print_msg(0, "You go berserk!");
    u.violence = 0;
    return 1;
}

int usmash(void)
{
}

int uwhirl(void)
{
    Mon_handle mons[8];
    int i;
    int j;
    if (u.weapon.valid() && (u.weapon.snapv()->durability < 10))
    {
        print_msg(0, "Your weapon is too badly damaged to consider that manoeuvre.");
        return 0;
    }
    if (out_of_violence(30))
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
        tmp = currlev->monster_at[u.pos + spiral_path[i]];
        if (tmp.valid())
        {
            mons[j++] = tmp;
        }
    }
    for (i = 0; i < 8; ++i)
    {
    }
}

/* prof-fighter.cc */
