/* cloud.cc - clouds for Martin's Dungeon Bash
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

const Cloud no_cloud = { Total_clouds, 0 };

void encounter_cloud(Cloud cld, bool announce_cloud)
{
    switch (cld.flavour)
    {
    case Cloud_fog:
        print_msg(0, "You are in a bank of fog.\n");
        break;
    case Cloud_ice:
        print_msg(0, "You are in a cloud of bitterly cold fog.\n");
        break;
    case Cloud_fire:
        print_msg(0, "You are in a cloud of searing flames.\n");
        break;
    case Cloud_poison:
        print_msg(0, "You are in a cloud of noxious gas.\n");
        break;
    case Cloud_abyssal:
        print_msg(0, "You are in a cloud of miasmic foulness.\n");
        break;
    }
}

/* cloud.cc */
