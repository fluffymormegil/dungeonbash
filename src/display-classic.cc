// display-classic.cc
// 
// Copyright 2005-2009 Martin Read
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

#define DISPLAY_CC
#include "dunbash.hh"
#include "player.hh"
#include <curses.h>
#include <stdio.h>
#include <panel.h>
#include <string.h>

#include <vector>

WINDOW *status_window;
WINDOW *world_window;
WINDOW *message_window;
PANEL *status_panel;
PANEL *world_panel;
PANEL *message_panel;

int dgamelaunch_karma;

int status_updated;
int map_updated;
int hard_redraw;

/* If your terminal defaults to black text on a white background instead of
 * light grey text on a black background, this will fuck up. */
chtype colour_attrs[15] =
{
    0,
    COLOR_PAIR(DBCLR_D_GREY) | A_BOLD,
    COLOR_PAIR(DBCLR_RED),
    COLOR_PAIR(DBCLR_BLUE),
    COLOR_PAIR(DBCLR_GREEN),
    COLOR_PAIR(DBCLR_PURPLE),
    COLOR_PAIR(DBCLR_BROWN),
    COLOR_PAIR(DBCLR_CYAN),
    A_BOLD,
    COLOR_PAIR(DBCLR_RED) | A_BOLD,
    COLOR_PAIR(DBCLR_BLUE) | A_BOLD,
    COLOR_PAIR(DBCLR_GREEN) | A_BOLD,
    COLOR_PAIR(DBCLR_PURPLE) | A_BOLD,
    COLOR_PAIR(DBCLR_BROWN) | A_BOLD,
    COLOR_PAIR(DBCLR_CYAN) | A_BOLD
};

#define DISP_HEIGHT 21
#define DISP_WIDTH 21

#define MSGLINES 21

// The screen will be redrawn from scratch once every this-many updates, to
// prevent view corruption with simple clients like dgamelaunch and termcast.
#define DGL_MAX_KARMA 200

int back_buffer[MAX_DUN_HEIGHT][MAX_DUN_WIDTH];
int front_buffer[DISP_HEIGHT][DISP_WIDTH];

std::vector<std::string> messages;

/* Prototypes for static funcs */
static void draw_status_line(void);
static void draw_world(void);

/* Static funcs */
static void draw_status_line(void)
{
    mvwprintw(status_window, 0, 0, "%-16.16s", u.name);
    mvwprintw(status_window, 0, 17, "HP: %3d/%3d", u.hpcur, u.hpmax);
    mvwprintw(status_window, 0, 30, "Depth: %d", u.lev.level);
    mvwprintw(status_window, 0, 47, "Bod: %2d/%2d", u.body - u.bdam, u.body);
    mvwprintw(status_window, 0, 62, "Gold: %d", u.gold);
    mvwprintw(status_window, 1, 0, "Def/Eva: %2d/%2d", u.defence, u.evasion);
    mvwprintw(status_window, 1, 16, "Food: %6d", u.food);
    mvwprintw(status_window, 1, 30, "%s: %4d/%4d", mana_nouns[u.job], u.mpcur, u.mpmax);
    mvwprintw(status_window, 1, 52, "Agi: %2d/%2d", u.agility - u.adam, u.agility);
    mvwprintw(status_window, 1, 64, "Exp: %2d/%7d", u.level, u.experience);
}

static void draw_world(void)
{
    int i;
    int j;
    int x;
    int y;

    for (i = 0; i < 21; i++)
    {
        y = u.pos.y + i - 10;
        for (j = 0; j < 21; j++)
        {
            x = u.pos.x + j - 10;
            if ((y < 0) || (x < 0) ||
                (y >= DUN_HEIGHT) || (x >= DUN_WIDTH))
            {
                if ((front_buffer[i][j] != ' ') || hard_redraw)
                {
                    mvwaddch(world_window, i, j, ' ');
                }
                front_buffer[i][j] = ' ';
            }
            else if (hard_redraw || (front_buffer[i][j] != back_buffer[y][x]))
            {
                int rend = back_buffer[y][x];
                wattrset(world_window, colour_attrs[rend >> BB_COLOR_SHIFT]);
                mvwaddch(world_window, i, j, rend & BB_CHAR_MASK);
                front_buffer[i][j] = rend;
            }
        }
    }
}

/* extern funcs */

void full_redraw(void)
{
    // This will probably cause lossage on non-ANSI terminals, but dgamelaunch
    // and termcast look for this exact string so there aren't much options.
    fputs("\e[2J", stdout);
    clearok(curscr, 1);
    dgamelaunch_karma = DGL_MAX_KARMA;
}

void display_update(void)
{
    if ((--dgamelaunch_karma) == 0)
        full_redraw();

    if (status_updated)
    {
        status_updated = 0;
        draw_status_line();
    }
    if (map_updated)
    {
        map_updated = 0;
        draw_world();
    }
    update_panels();
    doupdate();
}

int display_init_classic(void)
{
    int i, j;

    for (i = 0; i < DISP_HEIGHT; ++i)
    {
        for (j = 0; j < DISP_WIDTH; ++j)
        {
            front_buffer[i][j] = ' ';
        }
    }
    initscr();
    for (i = 0; i < DUN_HEIGHT; ++i)
    {
        for (j = 0; j < DUN_WIDTH; ++j)
        {
            back_buffer[i][j] = ' ' | (DBCLR_L_GREY << 8);
        }
    }
    initscr();
    noecho();
    cbreak();
    start_color();
    init_pair(DBCLR_BROWN, COLOR_YELLOW, COLOR_BLACK);
    init_pair(DBCLR_RED, COLOR_RED, COLOR_BLACK);
    init_pair(DBCLR_GREEN, COLOR_GREEN, COLOR_BLACK);
    init_pair(DBCLR_BLUE, COLOR_BLUE, COLOR_BLACK);
    init_pair(DBCLR_D_GREY, COLOR_BLACK, COLOR_BLACK);
    init_pair(DBCLR_PURPLE, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(DBCLR_CYAN, COLOR_CYAN, COLOR_BLACK);
    /* OK. We want a 21x21 viewport (player at centre), a 21x58 message
     * window, and a 2x80 status line. */
    status_window = newwin(2, 80, 22, 0);
    status_panel = new_panel(status_window);
    world_window = newwin(21, 21, 0, 0);
    world_panel = new_panel(world_window);
    message_window = newwin(21, 58, 0, 22);
    message_panel = new_panel(message_window);
    wclear(status_window);
    wclear(world_window);
    wclear(message_window);
    scrollok(status_window, FALSE);
    scrollok(world_window, FALSE);
    scrollok(message_window, FALSE);
    idcok(status_window, FALSE);
    idcok(world_window, FALSE);
    idcok(message_window, FALSE);
    idlok(status_window, FALSE);
    idlok(world_window, FALSE);
    idlok(message_window, FALSE);
    mvwprintw(world_window, 6, 5, "  Martin's");
    mvwprintw(world_window, 7, 5, "Dungeon Bash");
    mvwprintw(world_window, 9, 5, "Version %s", LONG_VERSION);
    wmove(message_window, 0, 0);
    map_updated = FALSE;
    status_updated = FALSE;
    update_panels();
    doupdate();
    dgamelaunch_karma = DGL_MAX_KARMA;
    return 0;
}

static void update_message(unsigned int line)
{
    std::string msg;

    if (line < messages.size())
    {
        msg = messages[line];
    }

    wmove(message_window, line, 0);

    size_t i = 0;
    size_t max = msg.size();

    while (i < max)
    {
        size_t close;

        if (msg[i] != '<')
        {
            goto literal;
        }

        close = msg.find('>', i + 1);

        if (close == std::string::npos)
        {
            goto literal;
        }

        for (int cand = 0; cand < 15; ++cand)
        {
            if (msg.compare(i + 1, close - i - 1, colour_names[cand]) == 0)
            {
                wattrset(message_window, colour_attrs[cand]);
                i = close;
                goto next;
            }
        }
literal:
        waddch(message_window, msg[i]);
next:
        ++i;
    }
    wattrset(message_window, 0);

    wclrtoeol(message_window);
}

std::string get_reply(int mid, unsigned nmax)
{
    std::string& out( messages[mid] );
    std::string buf;
    int keep = out.size();

    while(1)
    {
        out.erase(keep);

        for (unsigned i = 0; i < buf.size(); ++i)
        {
            if (buf[i] == '<')
            {
                out += "<<lgrey>";
            }
            else
            {
                out += buf[i];
            }
        }

        update_message(mid);
        int key = wgetch(message_window);

        if (isprint(key) && buf.size() < nmax)
        {
            buf.push_back(key);
        }
        else if (key == 8 || key == 127)
        {
            buf.resize(buf.size() ? buf.size() - 1 : 0);
        }
        else if (key == '\r' || key == '\n')
        {
            return buf;
        }
    }
}

/* For now, assume (1) that the player will never be so inundated
 * with messages that it's dangerous to let them just fly past (2)
 * that messages will be of sane length and nicely formatted. THIS
 * IS VERY BAD CODING PRACTICE! */
/* Note that every visible message forces a call to display_update().  Events
 * that cause changes to the map or the player should flag the change before
 * emitting messages. */
std::string message_line(bool visible, const std::string& msg, int expected)
{
    std::string ret;

    if (visible)
    {
        if (messages.size() == MSGLINES)
        {
            // Scroll off the oldest message
            messages.erase(messages.begin());

            for (int i = 0; i < MSGLINES - 1; ++i)
            {
                update_message(i);
            }
        }

        int msg_num = messages.size();
        messages.push_back(msg);
        update_message(msg_num);

        if (expected == 1)
        {
            int key = wgetch(message_window);

            ret = static_cast<char>(key);

            if (key < 32)
            {
                messages[msg_num] += '^';
                messages[msg_num] += static_cast<char>(key + '@');
            }
            else if (key == 127)
            {
                messages[msg_num] += "^?";
            }
            else if (key >= 128 && key < 160)
            {
                messages[msg_num] += '~';
                messages[msg_num] += static_cast<char>(key - 128 + '@');
            }
            else
            {
                messages[msg_num] += key;
            }

            update_message(msg_num);
        }
        else if (expected)
        {
            ret = get_reply(msg_num, expected);
        }

        display_update();
    }

    if (msglog_fp)
    {
        fputs(msg.c_str(), msglog_fp);
        fputs(ret.c_str(), msglog_fp);
        fputs("\n", msglog_fp);
    }

    return ret;
}

bool get_interrupt(void)
{
    bool ret = false;
    nodelay(message_window, TRUE);
    if (wgetch(message_window) >= 0)
    {
        ret = true;
    }
    nodelay(message_window, FALSE);
    return ret;
}

char get_silent(void)
{
    return wgetch(message_window);
}

int display_shutdown(void)
{
    display_update();
    clear();
    refresh();
    endwin();
    return 0;
}

bool cursor_highlight(libmormegil::Coord onto)
{
    if ((onto.y - u.pos.y) < -10 || (onto.y - u.pos.y) > 10 ||
        (onto.x - u.pos.x) < -10 || (onto.x - u.pos.x) > 10)
    {
        return false;
    }

    wmove(world_window, onto.y - u.pos.y + 10,
                        onto.x - u.pos.x + 10);
    wrefresh(world_window);

    return true;
}

// display-classic.cc
