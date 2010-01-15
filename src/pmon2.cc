/* pmon2.cc
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

#define PMON2_CC
#include "dunbash.hh"
#include "monsters.hh"
#include "pmonid.hh"

bool pmon_resists_necro(int pm)
{
    return !!(permons[pm].flags & PMF_RESIST_NECR);
}

bool pmon_resists_elec(int pm)
{
    return !!(permons[pm].flags & PMF_RESIST_ELEC);
}

bool pmon_resists_acid(int pm)
{
    return !!(permons[pm].flags & PMF_RESIST_ACID);
}

bool pmon_resists_fire(int pm)
{
    return !!(permons[pm].flags & PMF_RESIST_FIRE);
}

bool pmon_resists_cold(int pm)
{
    return !!(permons[pm].flags & PMF_RESIST_COLD);
}

bool pmon_is_undead(int pm)
{
    return !!(permons[pm].flags & PMF_UNDEAD);
}

bool pmon_is_stupid(int pm)
{
    return !!(permons[pm].flags & PMF_STUPID);
}

bool pmon_is_smart(int pm)
{
    return !!(permons[pm].flags & PMF_SMART);
}

bool pmon_is_magician(int pm)
{
    return !!(permons[pm].flags & PMF_MAGICIAN);
}

bool pmon_is_archer(int pm)
{
    return !!(permons[pm].flags & PMF_ARCHER);
}

bool pmon_is_greater_demon(int pm)
{
    return !!(permons[pm].flags & PMF_GDEMON);
}

bool pmon_is_ethereal(int pm)
{
    return !!(permons[pm].flags & PMF_ETHEREAL);
}

bool pmon_is_demon(int pm)
{
    return !!(permons[pm].flags & PMF_DEMONIC);
}

bool pmon_is_tele_harasser(int pm)
{
    return !!(permons[pm].flags & PMF_TELE_HARASS);
}

bool pmon_is_fire_demon(int pm)
{
    return (pm == PM_HELLHOUND) || (pm == PM_SCORCHER) || (pm == PM_IMMOLATOR);
}

bool pmon_is_slime_demon(int pm)
{
    return (pm == PM_FOETID_OOZE) || (pm == PM_FESTERING_HORROR) || (pm == PM_DEFILER);
}

bool pmon_is_flesh_demon(int pm)
{
    return (pm == PM_LASHER) || (pm == PM_FLAYER) || (pm == PM_DOMINATOR);
}

bool pmon_is_iron_demon(int pm)
{
    return (pm == PM_IRON_SNAKE) || (pm == PM_IRONGUARD) || (pm == PM_IRON_LORD);
}

bool pmon_is_death_demon(int pm)
{
    return (pm == PM_FLYING_SKULL) || (pm == PM_REAPER) || (pm == PM_DEATHLORD);
}

bool pmon_is_peaceful(int pm)
{
    return !!(permons[pm].flags & PMF_PEACEFUL);
}

bool pmon_always_female(int pm)
{
    return !!(permons[pm].flags & PMF_ALWAYS_FEMALE);
}

bool pmon_always_other(int pm)
{
    return !!(permons[pm].flags & PMF_ALWAYS_OTHER);
}

bool pmon_always_neuter(int pm)
{
    return !!(permons[pm].flags & PMF_ALWAYS_NEUTER);
}

bool pmon_always_male(int pm)
{
    return !!(permons[pm].flags & PMF_ALWAYS_MALE);
}

/* pmon2.cc */
