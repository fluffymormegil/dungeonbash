/* hci-common.cc
 * 
 * Copyright 2010 Martin Read and Stefan O'Rear
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

#define HCI_COMMON_CC
#include "dunbash.hh"
#include "player.hh"
#include <curses.h>
#include <stdio.h>
#include <panel.h>
#include <string.h>

#include <vector>

libmormegil::Coord last_projectile_pos = dunbash::NOWHERE;
libmormegil::Coord curr_projectile_pos = dunbash::NOWHERE;
Dbash_colour projectile_colour = DBCLR_L_GREY;
int projectile_delay = 40;
int you_colour = DBCLR_WHITE;

bool fruit_salad_inventory = false;
bool show_terrain = false;

bool suppressions[] =
{
    false, false, false, false,
    false, false, false, false,
    false, true
};

FILE *msglog_fp;

const char *colour_names[15] =
{
    "lgrey", "dgrey", "red", "blue", "green", "purple", "brown", "cyan",
    "white", "lred", "lblue", "lgreen", "lpurple", "yellow", "lcyan"
};

/* extern funcs */

int display_init_common(void)
{
    //int i, j;
#ifdef LOG_MESSAGES
#ifdef MULTIUSER
    user_permissions();
#endif
    msglog_fp = fopen("msglog.txt", "a");
    fprintf(msglog_fp, "-----------\nTimestamp %#Lx\n", uint64_t(time(0)));
#ifdef MULTIUSER
    game_permissions();
#endif
#endif
    /* TODO handle initializing the appropriate UI and display. */
    return 0;
}

int display_shutdown_common(void)
{
    /* TODO handle shutting down the appropriate UI and display. */
    return 0;
}

/* hci-common.cc */
