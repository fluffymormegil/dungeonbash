/* display.hh
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

#ifndef UI_HH
#define UI_HH

#include "coord.hh"
#include <string>

#define MSGCHAN_DEFAULT 0
#define MSGCHAN_TAUNT 1
#define MSGCHAN_NUMERIC 2
#define MSGCHAN_BORINGFAIL 3
#define MSGCHAN_MINORFAIL 4
#define MSGCHAN_PROMPT 5
#define MSGCHAN_FLUFF 6
#define MSGCHAN_MON_ALERT 7
#define MSGCHAN_INTERROR 8
#define MSGCHAN_DEBUG 9

/* XXX ui.c data and funcs */
extern void ui_init(void);
extern void print_msg(int channel, const char *fmt, ...);
extern void print_help(void);
extern void print_version(void);
extern void newsym(libmrl::Coord c);
extern void touch_back_buffer(void);
extern void print_inv(Poclass_num filter);
extern void print_equipped();
extern int inv_select(Poclass_num filter, const char *action, int accept_blank);
extern Game_cmd get_command(void);
extern int select_dir(libmrl::Coord *psign, bool silent = false);
extern int getYN(const char *msg);
extern int getyn(const char *msg);
extern void press_enter(void);
extern void pressanykey(void);
extern void show_discoveries(void);
extern void animate_projectile(libmrl::Coord pos, Dbash_colour col = DBCLR_L_GREY);
extern void projectile_done(void);
extern void farlook(void);

/* "Show the player the terrain only" flag. */
extern int show_terrain;

#endif
/* ui.hh */
