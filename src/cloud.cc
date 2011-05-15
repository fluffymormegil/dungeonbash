// cloud.cc - clouds for Martin's Dungeon Bash
// 
// Copyright 2009 Martin Read
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#define CLOUD_CC
#include "dunbash.hh"
#include "cloud.hh"

// intended mechanics:
//
//   A cloud affects its victims on the tick they enter it. It then affects
//   them on any *future* normal-speed tick they finish in it.
//
//   All clouds block LOS.
//
//   fog cloud: harmless
//   fire cloud: does 1d4 fire damage.
//   ice cloud: does 1d4 cold damage.
//   abyssal cloud: does 1d10 death damage.
//
// Spawning a fire cloud into an ice cloud (or v.v.) produces a fog cloud.
// Fire and ice clouds overwrite fog. Abyssal clouds overwrite all others and
// cannot be overwritten.
//
// TODO actually use the intensity field.
//
// TODO actually time clouds out.

const Cloud no_cloud = { Total_clouds, 0 };

void encounter_cloud(Cloud cld, bool entering)
{
    if ((u.next_cloud_tick > game_tick) && !entering)
    {
        return;
    }
    u.next_cloud_tick = get_next_tick(SPEED_NORMAL);
    switch (cld.flavour)
    {
    case Cloud_fog:
        print_msg(0, "You are in a bank of fog.");
        break;
    case Cloud_ice:
        print_msg(0, "You are in a cloud of bitterly cold fog.");
        if (!player_resists_dtype(DT_COLD))
        {
            damage_u(one_die(4), DEATH_KILLED, "a cloud of freezing fog");
        }
        break;
    case Cloud_fire:
        print_msg(0, "You are in a cloud of searing flames.");
        if (!player_resists_dtype(DT_FIRE))
        {
            damage_u(one_die(4), DEATH_KILLED, "a cloud of searing flames");
        }
        break;
    case Cloud_poison:
        print_msg(0, "You are in a cloud of noxious gas.");
        if (!player_resists_dtype(DT_POISON))
        {
            damage_u(one_die(4), DEATH_KILLED, "a cloud of poison gas");
        }
        break;
    case Cloud_abyssal:
        print_msg(0, "You are in a cloud of abyssal energies.");
        if (!player_resists_dtype(DT_NECRO))
        {
            damage_u(one_die(10), DEATH_KILLED, "a cloud of abyssal energies");
        }
        break;
    default:
        print_msg(0, "You are in a cloud of program bugs and unimplemented features.");
        break;
    }
}

Cloud resolve_conflict(const Cloud& left, const Cloud& right)
{
    Cloud cld;
    /* if the clouds are identical, merge them. */
    if (left.flavour == right.flavour)
    {
        cld.flavour = left.flavour;
        cld.intensity = std::max(left.intensity, right.intensity);
        cld.duration = left.duration * left.intensity + right.duration * right.intensity;
        cld.duration /= cld.intensity;
        cld.by_you = left.by_you || right.by_you;
        return cld;
    }
    /* Abyssal stomps all */
    if (left.flavour == Cloud_abyssal)
    {
        return left;
    }
    if (right.flavour == Cloud_abyssal)
    {
        return right;
    }
    /* Fog yields to all */
    if (left.flavour == Cloud_fog)
    {
        return right;
    }
    if (right.flavour == Cloud_fog)
    {
        return left;
    }
    /* Fire stomps what remains, except that it merges with ice to make fog. */
    if (left.flavour == Cloud_fire)
    {
        switch (right.flavour)
        {
        case Cloud_ice:
            cld.flavour = Cloud_fog;
            cld.intensity = 1;
            cld.duration = 5 + zero_die(6);
            cld.by_you = left.by_you || right.by_you;
            return cld;
        default:
            return left;
        }
    }
    /* Ice stomps what remains, except that it merges with fire to make fog. */
    if (left.flavour == Cloud_ice)
    {
        switch (right.flavour)
        {
        case Cloud_fire:
            cld.flavour = Cloud_fog;
            cld.intensity = 1;
            cld.duration = 5 + zero_die(6);
            cld.by_you = left.by_you || right.by_you;
            return cld;
        default:
            return left;
        }
    }
    return left;
}

bool mon_endure_cloud(Mon_handle mon, Cloud cld)
{
    // TODO implement monsters enduring clouds
    int next_tick = get_next_tick(SPEED_NORMAL);
}

bool put_cloud(Level *lptr, libmormegil::Coord pos, Cloud cld)
{
    // TODO implement placement of clouds
    Cloud existing;
    if (lptr->outofbounds(pos))
    {
        return false;
    }
    existing = lptr->cloud_at(pos);
    if (existing != no_cloud)
    {
        lptr->set_cloud_at(pos, resolve_conflict(cld, existing));
    }
    else
    {
        lptr->set_cloud_at(pos, cld);
    }
    return true;
}

bool move_cloud(Level *lptr, libmormegil::Coord old_cloudpos, libmormegil::Coord new_cloudpos)
{
    // TODO implement movement of clouds
}

// cloud.cc
