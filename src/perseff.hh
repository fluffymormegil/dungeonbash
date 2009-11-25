/* perseff.hh - Persistent effects in Martin's Dungeon Bash
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

#ifndef PERSEFF_HH
#define PERSEFF_HH

#ifndef MONSTERS_HH
#include "monsters.hh"
#endif

enum Persistent_effect
{
    // Buffs: monster
    Perseff_might, Perseff_fell_dominion,
    // Buffs: fighter
    Perseff_berserk, Perseff_martial_trance, Perseff_iron_soul,
    // Buffs: preacher
    Perseff_aspect_severity, Perseff_aspect_mercy,
    // Buffs: thanatophile
    Perseff_death_song, Perseff_assassin_soul,
    // Debuffs: elemental
    Perseff_bitter_chill, Perseff_searing_flames,
    // Debuffs: curses
    Perseff_leadfoot_curse, Perseff_wither_curse,
    Perseff_armourmelt_curse,
    // Debuffs: physical
    Perseff_binding_chains, Perseff_tentacle_embrace,
    // fencepost / count
    Total_perseffs
};

enum Eff_stacking_mode
{
    Stack_none,
    Stack_independent,
    Stack_renew,
    Stack_extend
};

struct Perseff_metadata
{
    const char *name;
    const char *description;
    bool is_buff;
    // See http://tvtropes.org/pmwiki.php/Main/NoOntologicalInertia for an
    // explanation of the next flag's name.
    bool ontological_inertia;
    Eff_stacking_mode stacking_mode;
};

struct Perseff_data
{
    Persistent_effect flavour;
    int power;
    int duration;
    bool by_you; // if true, ignore the caster field
    bool on_you; // if true, ignore the victim field
    Mon_handle caster;
    Mon_handle victim;
};

struct Status_flags
{
    enum
    {
        Status_flag_units = 2
    };
    enum Flag
    {
        // Bonuses
        Bonus_might, Bonus_fell_dom, Bonus_berserk, Bonus_tranced,
        Bonus_resistant, Bonus_severe, Bonus_merciful, Bonus_balanced,
        Bonus_deathsong, Bonus_assassin,
        // Maluses
        Malus_freezing, Malus_burning,
        Malus_hentacled, Malus_shackled, Malus_armourmelted, Malus_leadfooted,
        Malus_withered
    };
    uint32_t data[Status_flag_units];
    void clear_all()
    {
        int i;
        for (i = 0; i < Status_flag_units; ++i)
        {
            data[Status_flag_units] = 0u;
        }
    }
    bool test_flag(Flag tf) const
    {
        int f = tf;
        return !!(data[f >> 5] & (1 << (f & 31)));
    }
    bool set_flag(Flag tf)
    {
        int f = tf;
        return !!(data[f >> 5] & (1 << (f & 31)));
    }
};

#endif

/* perseff.hh */
