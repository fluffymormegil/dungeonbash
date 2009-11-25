/* pmon2.h
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

#ifndef PMON2_H
#define PMON2_H

#ifndef PMON1_H
#include "pmon1.hh"
#endif

extern bool pmon_is_archer(int pm);
extern bool pmon_is_magician(int pm);
extern bool pmon_is_smart(int pm);
extern bool pmon_is_stupid(int pm);
extern bool pmon_is_undead(int pm);
extern bool pmon_is_demon(int pm);
extern bool pmon_is_ethereal(int pm);
extern bool pmon_is_tele_harasser(int pm);
extern bool pmon_is_death_demon(int pm);
extern bool pmon_is_fire_demon(int pm);
extern bool pmon_is_flesh_demon(int pm);
extern bool pmon_is_slime_demon(int pm);
extern bool pmon_is_iron_demon(int pm);
extern bool pmon_is_greater_demon(int pm);
extern bool pmon_is_peaceful(int pm);
extern bool pmon_resists_cold(int pm);
extern bool pmon_resists_fire(int pm);
extern bool pmon_resists_elec(int pm);
extern bool pmon_resists_acid(int pm);
extern bool pmon_always_neuter(int pm);
extern bool pmon_always_male(int pm);
extern bool pmon_always_female(int pm);
extern bool pmon_always_other(int pm);

#endif

/* pmon2.h */

