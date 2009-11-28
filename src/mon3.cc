/* mon3.cc
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

#define MON3_C
#include "dunbash.hh"
#include "monsters.hh"
#include "pmonid.hh"
#include "objects.hh"
#include "bmagic.hh"
#include "combat.hh"

int weaker_demon(int pm_num)
{
    switch (pm_num)
    {
    case PM_FLAYER:
        return PM_LASHER;
    case PM_DOMINATOR:
        return PM_FLAYER;
    case PM_FESTERING_HORROR:
        return PM_FOETID_OOZE;
    case PM_DEFILER:
        return PM_FESTERING_HORROR;
    case PM_IRONGUARD:
        return PM_IRON_SNAKE;
    case PM_IRON_LORD:
        return PM_IRONGUARD;
    default:
        return NO_PM;
    }
}

bool Mon::apply_effect(Perseff_data &peff)
{
    peff.victim = self;
    status.set_flag(peff.flavour);
    perseffs.push_back(peff);
    return false;
}

bool Mon::suffer(Perseff_data& peff)
{
    std::string victim_name;
    get_name(&victim_name, 1, false);
    switch (peff.flavour)
    {
    case Perseff_bitter_chill:
        print_msg(0, "Bitter cold freezes %s.\n", victim_name.c_str());
        return damage_mon(self, one_die(peff.power), peff.by_you);

    case Perseff_searing_flames:
        print_msg(0, "Searing flames burn %s.\n", victim_name.c_str());
        return damage_mon(self, one_die(peff.power), peff.by_you);

    case Perseff_tentacle_embrace:
        /* rather theoretical since blue-on-blue melee isn't supported yet and
         * the player character hasn't got tentacles. */
        break;

    default:
        print_msg(MSGCHAN_INTERROR, "Monster has debuff of invalid/unimplemented type %d\n", peff.flavour);
        break;
    }
    return false;
}

void aggravate_monsters(Level *lptr)
{
    std::set<Mon_handle>::iterator iter;
    for (iter = lptr->denizens.begin(); iter != lptr->denizens.end(); ++iter)
    {
        iter->snapv()->awake = true;
    }
}

/* mon3.cc */
