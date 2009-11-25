/* bmagic.h
 * 
 * Copyright 2005 Martin Read
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

#ifndef BMAGIC_H
#define BMAGIC_H

/* XXX DATA TYPES XXX */

enum Monspell {
	MS_REJECT = -1,		/* Rejection tag. */
	/* "Melee" attacks */
	MS_STRIKE_STAFF,	/* Wizard */
	MS_NECRO_STAFF,		/* Lich */
	MS_CHILLING_TOUCH,	/* Master Lich */
	/* Ranged Attacks */
	MS_LIGHTNING,	/* Wizard */
	MS_NECRO_BOLT,	/* Lich */
	MS_NECRO_SMITE,	/* Master Lich - no cardinal alignment needed */
	MS_FIRE_COLUMN,	/* Immolator */
	MS_CORRUPTION,	/* Defiler */
        MS_CHAINSTRIKE, /* Dominator */
	/* Curses */
	MS_CURSE_ARMOURMELT,	/* All cursers */
	MS_CURSE_LEADFOOT,	/* All cursers */
	MS_CURSE_WITHERING,	/* Master Lich and Defiler only */
        MS_SHACKLE,             /* dominator only */
	/* Evasion */
	MS_TELEPORT_ESCAPE,	/* Wizard, Archmage, Master Lich */
	MS_TELEPORT_AND_SUMMON,	/* Archmage */
	MS_TELEPORT_ASSAULT,	/* Wizard, Archmage, Master Lich */
        /* Other */
        MS_ANIMATE_DEAD,        /* Lich, Master Lich, Deathlord */
};

extern int use_black_magic(Mon_handle mon);
extern void mon_curses(Mon_handle mon);
extern void malignant_aura(void);

#endif

/* dunbash.h */
