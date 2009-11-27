/* combat.hh
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

#ifndef COMBAT_HH
#define COMBAT_HH

#ifndef DUNBASH_HH
#include "dunbash.hh"
#endif

#include "monsters.hh"

#define agility_modifier() (u.status.test_flag(Perseff_wither_curse) ? (u.agility / 10) : (u.agility / 5))
#define EVASION_PRESCALE        100
#define EVASION_POSTSCALE       200
/* XXX combat.c data and funcs */
extern int player_attack(libmrl::Coord dir);
extern int mhitu(Mon_handle mon, Damtyp dtyp);
extern int uhitm(Mon_handle mon);
extern int mshootu(Mon_handle mon, Damtyp dtyp);
extern int ushootm(libmrl::Coord dir);
extern void renew_combat_timer(void);
extern bool player_in_combat(void);

#endif

/* combat.hh */
