/* ui-sdl.cc
 * 
 * Copyright 2010 Martin Read
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

#define UI_SDL_CC
#include "dunbash.hh"
#include "monsters.hh"
#include "objects.hh"
#include "player.hh"
#include "display.hh"
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include "stlprintf.hh"

/* Prototypes for static funcs */

void newsym(libmrl::Coord c)
{
    /* TODO write SDL newsym() */
}

/* extern funcs */

void press_enter(void)
{
    /* TODO write SDL press_enter() */
}

void show_discoveries(void)
{
    /* TODO write SDL show_discoveries() */
}

void print_inv(Poclass_num filter)
{
    /* TODO write SDL print_inv() */
}

int inv_select(Poclass_num filter, const char *action, int accept_blank)
{
    /* TODO write SDL inv_select() */
}

int select_dir(libmrl::Coord *psign, bool silent)
{
    /* TODO write SDL select_dir() */
    return 0;
}

Game_cmd get_command(void)
{
    /* TODO write SDL get_command() */
    return NO_CMD;
}

void pressanykey(void)
{
    /* TODO write SDL pressanykey() */
    return;
}

int getYN(const char *msg)
{
    /* TODO write SDL getYN() */
}

int getyn(const char *msg)
{
    /* TODO write SDL getyn() ? */
}

void animate_projectile(libmrl::Coord pos, Dbash_colour col)
{
    /* TODO write SDL projectile animation */
}

void projectile_done(void)
{
    /* TODO write SDL projectile animation cleanup */
}

void farlook(void)
{
    /* TODO write SDL farlook interface */
}

int get_smite_target(libmrl::Coord *ppos, bool must_be_visible)
{
    /* TODO write SDL smite targeter */
}

/* ui-sdl.cc */
