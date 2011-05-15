/* perseff.cc - persistent effects for Martin's Dungeon Bash
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

#define PERSEFF_CC

#include "dunbash.hh"
#include "perseff.hh"

Perseff_metadata perseff_meta[Total_perseffs] =
{
    {
        "Might", "Increases the physical damage done by the creature.",
        true, true, Stack_renew
    },
    {
        "Fell Dominion", "Guides the creature's actions with a malign intellect.",
        true, true, Stack_renew
    },
    {
        "Berserker Rage", "Increases the Fighter's physical damage per attack.",
        true, false, Stack_none
    },
    {
        "Martial Trance", "Increases the Fighter's accuracy.",
        true, false, Stack_none
    },
    {
        "Soul of Iron", "Increases the Fighter's resistance to magic.",
        true, false, Stack_none
    },
    {
        "Smash", "This turn's attack autohits for double damage.",
        true, false, Stack_none
    },
    {
        "Aspect of Severity", "Increases the Preacher's physical damage at the expense of weaker healing.",
        true, false, Stack_none
    },
    {
        "Aspect of Mercy", "Increases the Preacher's healing ability at the expense of weaker attacks.",
        true, false, Stack_none
    },
    {
        "Song of Death", "Adds Death-type damage to the Thanatophile's melee attacks.",
        true, false, Stack_none
    },
    {
        "Assassin Soul", "Conceals the Thanatophile from the sight of prying eyes.",
        true, false, Stack_none
    },
    {
        "Bitter Chill", "Does Cold damage to the affected creature every turn.",
        false, true, Stack_independent
    },
    {
        "Searing Flames", "Does Fire damage to the affected creature every turn.",
        false, true, Stack_independent
    },
    {
        "Curse of Leaden Limbs", "Reduces the affected creature's action speed.",
        false, false, Stack_renew
    },
    {
        "Curse of Withering", "Debilitates the affected creature, making it clumsy and easily struck.",
        false, false, Stack_renew
    },
    {
        "Curse of Shattered Armour", "Temporarily renders the affected creature's armour both ineffective and fragile.",
        false, false, Stack_renew
    },
    {
        "Binding Chains", "Temporarily prevents the affected creature from leaving its current square.",
        false, false, Stack_renew
    },
    {
        "Tentacle Embrace", "Holds the affected creature in place.",
        false, false, Stack_renew
    },
};

int Perseff_data::conflicts(const Perseff_data& other) const
{
    switch (flavour)
    {
    default:
        return 0;

    case Perseff_bitter_chill:
        if (other.flavour == Perseff_searing_flames)
        {
            return (power < other.power) ? -1 : ((power == other.power) ? 2 : 1);
        }
        return 0;

    case Perseff_searing_flames:
        if (other.flavour == Perseff_bitter_chill)
        {
            return (power < other.power) ? -1 : ((power == other.power) ? 2 : 1);
        }
        return 0;

    case Perseff_protection:
        if ((other.flavour == Perseff_wither_curse) ||
            (other.flavour == Perseff_leadfoot_curse) ||
            (other.flavour == Perseff_armourmelt_curse))
        {
            return 1;
        }
        return 0;

    case Perseff_leadfoot_curse:
    case Perseff_armourmelt_curse:
    case Perseff_wither_curse:
        if (other.flavour == Perseff_protection)
        {
            return -1;
        }
        return 0;
    }
}

void Perseff_data::extend_using(const Perseff_data& other)
{
    int p, d;
    p = std::max(power, other.power);
    d = power * duration + other.power * other.duration;
    if ( power < other.power)
    {
        caster = other.caster;
        by_you = other.by_you;
    }
    power = p;
    duration = d / p;
}

void Perseff_data::renew_using(const Perseff_data& other)
{
    if (power < other.power)
    {
        *this = other;
    }
    else
    {
        // Weak effects will not effectively renew strong ones that aren't on
        // the edge of expiry.
        duration = std::max((other.duration * other.power) / power, duration);
    }
}

/* perseff.cc */
