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

// This file is intended to be natural language agnostic.  It contains the
// code for displaying the screen, but no game logic; it could be replaced
// with a tiles version, or something.

// There is some text in here, but no grammar logic; gettext or equivalent
// would work fine.

// In addition to calling of the functions, this file uses global state;
// the status line takes values from &u, the map view takes information
// from back_buffer.

#ifndef DISPLAY_HH
#define DISPLAY_HH

#include "coord.hh"
#include <string>

/* XXX display.c data and funcs */
extern std::string message_line(bool vs, const std::string& msg, int expected = 0);
extern int display_init(void);
extern void display_update(void);
extern void full_redraw(void);
extern int display_shutdown(void);
extern bool get_interrupt(void);
extern char get_silent(void);
extern bool cursor_highlight(libmrl::Coord mappos);

/* The back buffer represents the character's entire map knowledge.  It
   is currently represented using colored characters; this is Less Than Ideal.

   You should fill this in before setting map_updated.
 */
#define BB_CHAR_MASK   0xFF
#define BB_COLOR_SHIFT 8
extern int back_buffer[MAX_DUN_HEIGHT][MAX_DUN_WIDTH];

/* "I've changed things that need to be redisplayed" flags. */
extern int hard_redraw;
extern int status_updated;
extern int map_updated;

#endif
/* display.hh */
