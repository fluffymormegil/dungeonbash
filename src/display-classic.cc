/* display.cc
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

#define DISPLAY_C
#include "dunbash.hh"
#include "monsters.hh"
#include "objects.hh"
#include "player.hh"
#include <curses.h>
#include <stdio.h>
#include <panel.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>

WINDOW *status_window;
WINDOW *world_window;
WINDOW *message_window;
PANEL *status_panel;
PANEL *world_panel;
PANEL *message_panel;

FILE *msglog_fp;

// Message channel suppression
bool suppressions[] =
{
    false, false, false, false,
    false, false, false, false,
    false, true
};

bool fruit_salad_inventory;

libmrl::Coord last_projectile_pos = libmrl::NOWHERE;
libmrl::Coord curr_projectile_pos = libmrl::NOWHERE;
Dbash_colour projectile_colour = DBCLR_L_GREY;
int projectile_delay = 40;

int wall_colour;
int you_colour;
int status_updated;
int map_updated;
int show_terrain;
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

chtype back_buffer[MAX_DUN_HEIGHT][MAX_DUN_WIDTH];
chtype front_buffer[DISP_HEIGHT][DISP_WIDTH];

/* Prototypes for static funcs */
static chtype object_char(int object_id);
static chtype monster_char(int monster_id);
static chtype terrain_char(Terrain_num terrain_type);
static void draw_status_line(void);
static void draw_world(void);
static void print_help_en_GB(void);

/* Static funcs */
static void draw_status_line(void)
{
    mvwprintw(status_window, 0, 0, "%-16.16s", u.name);
    mvwprintw(status_window, 0, 17, "HP: %3d/%3d", u.hpcur, u.hpmax);
    mvwprintw(status_window, 0, 30, "Depth: %d", u.lev.level);
    mvwprintw(status_window, 0, 47, "Bod: %2d/%2d", u.body - u.bdam, u.body);
    mvwprintw(status_window, 0, 62, "Gold: %d", u.gold);
    mvwprintw(status_window, 1, 0, "Def/Eva: %2d/%2d", u.defence, u.evasion);
    mvwprintw(status_window, 1, 19, "Food: %6d", u.food);
    mvwprintw(status_window, 1, 62, "Exp: %2d/%7d", u.level, u.experience);
    mvwprintw(status_window, 1, 47, "Agi: %2d/%2d", u.agility - u.adam, u.agility);
}

static chtype terrain_char(Terrain_num terrain_type)
{
    return colour_attrs[terrain_data[terrain_type].colour] | terrain_data[terrain_type].symbol;
}

static chtype monster_char(int monster_id)
{
    return (permons[monster_id].sym) | colour_attrs[permons[monster_id].colour];
}

static chtype object_char(int object_id)
{
    return permobjs[object_id].sym | colour_attrs[permobjs[object_id].colour];
}

void touch_back_buffer(void)
{
    libmrl::Coord c;
    for (c.y = 0; c.y < MAX_DUN_HEIGHT; c.y++)
    {
        for (c.x = 0; c.x < MAX_DUN_WIDTH; c.x++)
        {
            newsym(c);
        }
    }
    map_updated = 1;
    hard_redraw = 1;
}

void newsym(libmrl::Coord c)
{
    chtype ch;

    if ((c.y < 0) || (c.x < 0) || (c.y >= MAX_DUN_HEIGHT) || (c.x >= MAX_DUN_WIDTH))
    {
        return;
    }
    ch = back_buffer[c.y][c.x];
    if ((c.y >= currlev->height) || (c.x >= currlev->width))
    {
        back_buffer[c.y][c.x] = ' ';
    }
    else
    {
        Mon_handle mon = currlev->monster_at(c);
        Mon *mptr = mon.snapv();
        if (c == curr_projectile_pos)
        {
            back_buffer[c.y][c.x] = '*' | colour_attrs[projectile_colour];
        }
        else if (c == u.pos)
        {
            back_buffer[c.y][c.x] = '@' | colour_attrs[you_colour];
        }
        else if (!show_terrain && mptr && mon_visible(mon))
        {
            back_buffer[c.y][c.x] = monster_char(mptr->mon_id);
        }
        else if (currlev->flags_at(c) & MAPFLAG_EXPLORED)
        {
            if (!show_terrain && currlev->object_at(c).valid())
            {
                back_buffer[c.y][c.x] = object_char(currlev->object_at(c).otyp());
            }
            else
            {
                back_buffer[c.y][c.x] = terrain_char(currlev->terrain_at(c));
            }
        }
        else
        {
            back_buffer[c.y][c.x] = ' ';
        }
    }
    if (ch != back_buffer[c.y][c.x])
    {
        map_updated = 1;
    }
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
                mvwaddch(world_window, i, j, back_buffer[y][x]);
                front_buffer[i][j] = back_buffer[y][x];
            }
        }
    }
}

/* extern funcs */

void press_enter(void)
{
    int ch;
    print_msg(0, "Press RETURN or SPACE to continue");
    while (1)
    {
        ch = wgetch(message_window);
        if ((ch == ' ') || (ch == '\n') || (ch == '\r'))
        {
            break;
        }
    }
}

void display_update(void)
{
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

int display_init(void)
{
    int i, j;
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
            back_buffer[i][j] = ' ';
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
    wall_colour = DBCLR_BROWN;
    you_colour = DBCLR_WHITE;
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
    scrollok(message_window, TRUE);
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
    return 0;
}

int read_input(char *buffer, int length)
{
    echo();
    display_update();
    buffer[0] = '\0';
    wgetnstr(message_window, buffer, length);
    noecho();
    return strlen(buffer);
}

void print_msg(int channel, const char *fmt, ...)
{
    va_list ap;
    va_list ap2;
    /* For now, assume (1) that the player will never be so inundated
     * with messages that it's dangerous to let them just fly past (2)
     * that messages will be of sane length and nicely formatted. THIS
     * IS VERY BAD CODING PRACTICE! */
    /* Note that every message forces a call to display_update().
     * Events that cause changes to the map or the player should flag
     * the change before calling printmsg. */
    if (!suppressions[channel])
    {
        va_start(ap, fmt);
        vw_printw(message_window, fmt, ap);
        wprintw(message_window, "\n");
        va_end(ap);
    }
    if (msglog_fp)
    {
        va_start(ap2, fmt);
        vfprintf(msglog_fp, fmt, ap);
        fprintf(msglog_fp, "\n");
        va_end(ap2);
    }
    display_update();
}

void show_discoveries(void)
{
    int i, j;
    print_msg(0, "You recognise the following items:");
    for (i = 0, j = 1; i < PO_COUNT; i++)
    {
        if (permobjs[i].known)
        {
            print_msg(0, "%s", permobjs[i].name);
            j++;
        }
        if (j == 19)
        {
            press_enter();
            j = 0;
        }
    }
}

void print_inv(Poclass_num filter)
{
    int i;
    std::string namestr;
    Obj const *optr;
    for (i = 0; i < 19; i++)
    {
        wattrset(message_window, 0);
        optr = u.inventory[i].snapc();
        if (optr && ((filter == POCLASS_NONE) || (permobjs[optr->obj_id].poclass == filter)))
        {
            if (fruit_salad_inventory)
            {
                switch (optr->quality())
                {
                case Itemqual_bad:
                    wattrset(message_window, colour_attrs[DBCLR_RED]);
                    break;
                case Itemqual_normal:
                    wattrset(message_window, 0);
                    break;
                case Itemqual_good:
                    wattrset(message_window, colour_attrs[DBCLR_GREEN]);
                    break;
                case Itemqual_great:
                    wattrset(message_window, colour_attrs[DBCLR_L_BLUE]);
                    break;
                case Itemqual_excellent:
                    wattrset(message_window, colour_attrs[DBCLR_PURPLE]);
                    break;
                }
            }
            u.inventory[i].snapc()->get_name(&namestr);
            if (u.ring == u.inventory[i])
            {
                namestr += " (on finger)";
            }
            else if (u.weapon == u.inventory[i])
            {
                namestr += " (in hand)";
            }
            else if (u.armour == u.inventory[i])
            {
                namestr += " (being worn)";
            }
            print_msg(0, "%c) %s", 'a' + i, namestr.c_str());
            // XXX avoid coloring the letter
        }
        wattrset(message_window, 0);
    }
}

int inv_select(Poclass_num filter, const char *action, int accept_blank)
{
    int selection;
    int ch;
    int i;
    int items = 0;
    for (i = 0; i < 19; i++)
    {
        if ((u.inventory[i].valid()) && ((filter == POCLASS_NONE) || (permobjs[u.inventory[i].otyp()].poclass == filter)))
        {
            items++;
        }
    }
    if (items == 0)
    {
        print_msg(MSGCHAN_PROMPT, "You have nothing to %s.", action);
        return -1;
    }
    print_msg(MSGCHAN_PROMPT, "Items available to %s", action);
    print_inv(filter);
    if (accept_blank)
    {
        print_msg(MSGCHAN_PROMPT, "-) no item");
    }
    print_msg(MSGCHAN_PROMPT, "[ESC/SPACE to cancel]");
tryagain:
    print_msg(MSGCHAN_PROMPT, "What do you want to %s? ", action);
    ch = wgetch(message_window);
    switch (ch)
    {
    case '-':
        if (accept_blank)
        {
            print_msg(MSGCHAN_PROMPT, "");
            return -2;
        }
    case 'x':
    case '\x1b':
    case ' ':
        print_msg(MSGCHAN_PROMPT, "");
        print_msg(MSGCHAN_PROMPT, "Never mind.");
        return -1;
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
    case 'g':
    case 'h':
    case 'i':
    case 'j':
    case 'k':
    case 'l':
    case 'm':
    case 'n':
    case 'o':
    case 'p':
    case 'q':
    case 'r':
    case 's':
        /* I am assuming that we're in a place where the character
         * set is a strict superset of ASCII. If we're not, the
         * following code may break. */
        selection = ch - 'a';
        if ((u.inventory[selection].valid()) && ((filter == POCLASS_NONE) || (permobjs[u.inventory[selection].otyp()].poclass == filter)))
        {
            return selection;
        }
        /* Fall through */
    default:
        print_msg(MSGCHAN_PROMPT, "Bad selection");
        goto tryagain;
    }
}

int select_dir(libmrl::Coord *psign, bool silent)
{
    int ch;
    int done = 0;
    if (!silent)
    {
        print_msg(MSGCHAN_PROMPT, "Select a direction with movement keys.");
        print_msg(MSGCHAN_PROMPT, "[ESC or space to cancel].");
    }
    while (!done)
    {
        ch = wgetch(message_window);
        switch (ch)
        {
        case 'h':
            *psign = libmrl::WEST;
            done = 1;
            break;
        case 'j':
            *psign = libmrl::SOUTH;
            done = 1;
            break;
        case 'k':
            *psign = libmrl::NORTH;
            done = 1;
            break;
        case 'l':
            *psign = libmrl::EAST;
            done = 1;
            break;
        case 'y':
            *psign = libmrl::NORTHWEST;
            done = 1;
            break;
        case 'u':
            *psign = libmrl::NORTHEAST;
            done = 1;
            break;
        case 'b':
            *psign = libmrl::SOUTHWEST;
            done = 1;
            break;
        case 'n':
            *psign = libmrl::SOUTHEAST;
            done = 1;
            break;
        case '\n':
        case '.':
            psign->y = 0;
            psign->x = 0;
            done = 1;
            break;
        case '\x1b':
        case ' ':
            return -1;	/* cancelled. */
        default:
            if (!silent)
            {
                print_msg(MSGCHAN_PROMPT, "Bad direction (use movement keys; ESC or space to cancel.)");
            }
            break;
        }
    }
    return 0;
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

Game_cmd get_command(void)
{
    int ch;
    int done = 0;
    while (!done)
    {
        ch = wgetch(message_window);
        switch (ch)
        {
        case 'a':
            return ATTACK;
        case 'z':
            return ZAP_WAND;
        case 'A':
            return ACTIVATE_MISC;
        case 'v':
            return VOCALIZE_WORD;
        case '0':
        case ',':
        case 'g':
            return GET_ITEM;
        case 'd':
            return DROP_ITEM;
        case 'D':
            return DUMP_CHARA;
        case 'S':
            return SAVE_GAME;
        case 'X':
            return QUIT;
        case 'i':
            return SHOW_INVENTORY;
        case 'I':
            return INSPECT_ITEM;
        case 'E':
            return SHOW_EQUIPPED;
        case ';':
            return FARLOOK;
        case ':':
            return FLOORLOOK;
        case '#':
            return SHOW_TERRAIN;
        case '\\':
            return SHOW_DISCOVERIES;
        case '\x12':
            return RNG_TEST;
        case 'h':
            return MOVE_WEST;
        case 'j':
            return MOVE_SOUTH;
        case 'k':
            return MOVE_NORTH;
        case 'l':
            return MOVE_EAST;
        case 'y':
            return MOVE_NW;
        case 'u':
            return MOVE_NE;
        case 'b':
            return MOVE_SW;
        case 'n':
            return MOVE_SE;
        case 'H':
            return FARMOVE_WEST;
        case 'J':
            return FARMOVE_SOUTH;
        case 'K':
            return FARMOVE_NORTH;
        case 'L':
            return FARMOVE_EAST;
        case 'Y':
            return FARMOVE_NW;
        case 'U':
            return FARMOVE_NE;
        case 'B':
            return FARMOVE_SW;
        case 'N':
            return FARMOVE_SE;
        case 'q':
            return QUAFF_POTION;
        case 'r':
            return READ_SCROLL;
        case 'w':
            return WIELD_WEAPON;
        case 'W':
            return WEAR_ARMOUR;
        case 'T':
            return TAKE_OFF_ARMOUR;
        case 'P':
            return PUT_ON_RING;
        case 'R':
            return REMOVE_RING;
        case '?':
            return GIVE_HELP;
        case 'V':
            return PRINT_VERSION;
        case '<':
            return GO_UP_STAIRS;
        case '>':
            return GO_DOWN_STAIRS;
        case 'e':
            return EAT_FOOD;
        case '.':
            return STAND_STILL;
        case '\x04':
            return WIZARD_DESCEND;
        case '\x05':
            return WIZARD_LEVELUP;
        case '\x10':
            return WIZARD_DUMP_PERSEFFS;
        case '\x0f':
            return WIZARD_CURSE_ME;
        case '\x14':
            return WIZARD_TELEPORT;
        }
    }
    return NO_CMD;
}

int display_shutdown(void)
{
    display_update();
    clear();
    refresh();
    endwin();
    return 0;
}

void pressanykey(void)
{
    print_msg(MSGCHAN_PROMPT, "Press any key to continue.");
    wgetch(message_window);
}

int getYN(const char *msg)
{
    int ch;
    print_msg(MSGCHAN_PROMPT, "%s", msg);
    print_msg(MSGCHAN_PROMPT, "Press capital Y to confirm, any other key to cancel");
    ch = wgetch(message_window);
    if (ch == 'Y')
    {
        return 1;
    }
    return 0;
}

int getyn(const char *msg)
{
    int ch;
    print_msg(MSGCHAN_PROMPT, "%s", msg);
    while (1)
    {
        ch = wgetch(message_window);
        switch (ch)
        {
        case 'y':
        case 'Y':
            return 1;
        case 'n':
        case 'N':
            return 0;
        case '\x1b':
        case ' ':
            return -1;
        default:
            print_msg(MSGCHAN_PROMPT, "Invalid response. Press y or n (ESC or space to cancel)");
        }
    }
}

void print_help(void)
{
    print_help_en_GB();
}

static void print_help_en_GB(void)
{
    print_msg(0, "MOVEMENT");
    print_msg(0, "y  k  u");
    print_msg(0, " \\ | /");
    print_msg(0, "  \\|/");
    print_msg(0, "h--*--l");
    print_msg(0, "  /|\\");
    print_msg(0, " / | \\");
    print_msg(0, "b  j  n");
    print_msg(0, "Attack monsters in melee by bumping into them.");
    print_msg(0, "Doors do not have to be opened before you go through.");
    print_msg(0, "Turn on NUM LOCK to use the numeric keypad for movement.");
    print_msg(0, "Capitals HJKLYUBN move in the corresponding direction"
              "until something interesting happens or is found.");
    pressanykey();
    print_msg(0, "");
    print_msg(0, "ACTIONS");
    print_msg(0, "a   make an attack (used to fire bows)");
    print_msg(0, "P   put on a ring");
    print_msg(0, "R   remove a ring");
    print_msg(0, "W   wear armour");
    print_msg(0, "T   take off armour");
    print_msg(0, "r   read a scroll");
    print_msg(0, "w   wield a weapon");
    print_msg(0, "q   quaff a potion");
    print_msg(0, "z   zap a wand");
    print_msg(0, "A   activate a miscellaneous item");
    print_msg(0, "g   pick up an item (also 0 or comma)");
    print_msg(0, "d   drop an item");
    print_msg(0, "e   eat something edible");
    print_msg(0, ">   go down stairs");
    print_msg(0, "5   do nothing (wait until next action)");
    print_msg(0, ".   do nothing (wait until next action)");
    pressanykey();
    print_msg(0, "");
    print_msg(0, "OTHER COMMANDS");
    print_msg(0, "S   save and exit");
    print_msg(0, "X   quit without saving");
    print_msg(0, "i   print your inventory");
    print_msg(0, "I   examine an item you are carrying");
    print_msg(0, "E   show your equipped items");
    print_msg(0, "#   show underlying terrain of occupied squares");
    print_msg(0, "\\   list all recognised items");
    print_msg(0, "D   dump your character's details to <name>.dump");
    print_msg(0, "?   print this message");
    print_msg(0, "Control-W    print information about this program's absence of warranty.");
    print_msg(0, "Control-D    print information about redistributing this program.");
    pressanykey();
    print_msg(0, "");
    print_msg(0, "SYMBOLS - you and your surroundings");
    print_msg(0, "@   you");
    print_msg(0, ".   floor");
    print_msg(0, "<   stairs up");
    print_msg(0, ">   stairs down");
    print_msg(0, "\"   a pool of liquid, possibly baleful");
    print_msg(0, "_   an altar");
    print_msg(0, "-   an anvil or other unobstructive fitting");
    print_msg(0, "|   a furnace or other obstructive fitting");
    print_msg(0, "#   wall");
    print_msg(0, "+   a door or tombstone");
    pressanykey();
    print_msg(0, "");
    print_msg(0, "SYMBOLS - treasure");
    print_msg(0, ")   a weapon");
    print_msg(0, "(   a missile weapon");
    print_msg(0, "[   a suit of armour");
    print_msg(0, "=   a ring");
    print_msg(0, "?   a scroll");
    print_msg(0, "!   a potion");
    print_msg(0, "%%   some food");
    print_msg(0, "&   corpses, severed body parts, etc.");
    print_msg(0, "/   a magic wand");
    print_msg(0, "*   a miscellaneous item");
    pressanykey();
    print_msg(0, "");
    print_msg(0, "Demons are represented as numbers.");
    print_msg(0, "Most other monsters are shown as letters.");
    print_msg(0, "");
    print_msg(0, "This is all the help you get. Good luck!");
}

void animate_projectile(libmrl::Coord pos, Dbash_colour col)
{
    if (!pos_visible(pos))
    {
        return;
    }
    projectile_colour = col;
    last_projectile_pos = curr_projectile_pos;
    curr_projectile_pos = pos;
    if (last_projectile_pos != libmrl::NOWHERE)
    {
        newsym(last_projectile_pos);
    }
    newsym(curr_projectile_pos);
    display_update();
    usleep(projectile_delay * 1000);
}

void projectile_done(void)
{
    last_projectile_pos = curr_projectile_pos;
    curr_projectile_pos = libmrl::NOWHERE;
    newsym(last_projectile_pos);
    last_projectile_pos = libmrl::NOWHERE;
    display_update();
}

void farlook(void)
{
    libmrl::Coord screenpos = { 10, 10 };
    libmrl::Coord mappos = u.pos;
    libmrl::Coord step;
    std::string name;
    bool done = false;
    int i;

    print_msg(MSGCHAN_PROMPT, "Use the movement keys to move the cursor.");
    print_msg(MSGCHAN_PROMPT, "Press '.' to examine a square, ESC/SPACE to finish.");
    wmove(world_window, screenpos.y, screenpos.x);
    wrefresh(world_window);
    while (!done)
    {
        i = select_dir(&step, true);
        if (i == -1)
        {
            done = true;
        }
        else if ((step.y == 0) && (step.x == 0))
        {
            if (currlev->outofbounds(mappos))
            {
                print_msg(MSGCHAN_PROMPT, "The Outer Darkness.");
            }
            else if (!(currlev->flags_at(mappos) & MAPFLAG_EXPLORED))
            {
                print_msg(MSGCHAN_PROMPT, "Unexplored territory");
            }
            else
            {
                Mon_handle mh = currlev->monster_at(mappos);
                Obj_handle oh = currlev->object_at(mappos);
                if (mappos == u.pos)
                {
                    print_msg(MSGCHAN_PROMPT, "An unfortunate adventurer");
                }
                if (mh.valid() && mon_visible(mh))
                {
                    //describe_monster(currlev->monster_at(mappos));
                    mh.snapc()->get_name(&name, 0);
                    print_msg(MSGCHAN_PROMPT, "%s", name.c_str());
                }
                if (oh.valid())
                {
                    oh.snapc()->get_name(&name);
                    print_msg(MSGCHAN_PROMPT, "%s", name.c_str());
                }
                print_msg(MSGCHAN_PROMPT, "%s", terrain_data[currlev->terrain_at(mappos)].name);
            }
            if (wizard_mode)
            {
                print_msg(0, "%d %d: flags %8.8x", mappos.y, mappos.x, currlev->flags_at(mappos));
            }
        }
        else
        {
            if (currlev->outofbounds(mappos + step))
            {
                continue;
            }
            libmrl::Coord tmp_spos = screenpos + step;
            if ((tmp_spos.y < 0) || (tmp_spos.x < 0) ||
                (tmp_spos.y >= DISP_HEIGHT) || (tmp_spos.x >= DISP_WIDTH))
            {
                continue;
            }
            screenpos = tmp_spos;
            mappos = mappos + step;
            wmove(world_window, screenpos.y, screenpos.x);
            wrefresh(world_window);
        }
    }
    print_msg(MSGCHAN_PROMPT, "Done.");
}

void get_smite_target(libmrl::Coord *ppos)
{
    libmrl::Coord screenpos = { 10, 10 };
    libmrl::Coord mappos = u.pos;
    libmrl::Coord step;
    std::string name;
    bool done = false;
    int i;

    print_msg(MSGCHAN_PROMPT, "Use the movement keys to move the cursor.");
    print_msg(MSGCHAN_PROMPT, "Use '.'/ENTER to select a square, ESC/SPACE to cancel.");
    wmove(world_window, screenpos.y, screenpos.x);
    wrefresh(world_window);
    while (!done)
    {
        i = select_dir(&step, true);
        if (i == -1)
        {
            done = true;
        }
        else if ((step.y == 0) && (step.x == 0))
        {
            if (currlev->outofbounds(mappos))
            {
                print_msg(MSGCHAN_PROMPT, "The Outer Darkness.");
            }
            else if (!(currlev->flags_at(mappos) & MAPFLAG_EXPLORED))
            {
                print_msg(MSGCHAN_PROMPT, "Unexplored territory");
            }
            else
            {
                Mon_handle mh = currlev->monster_at(mappos);
                Obj_handle oh = currlev->object_at(mappos);
                if (mappos == u.pos)
                {
                    print_msg(MSGCHAN_PROMPT, "An unfortunate adventurer");
                }
                else if (mh.valid() & mon_visible(mh))
                {
                    //describe_monster(currlev->monster_at(mappos));
                    mh.snapc()->get_name(&name, 0);
                    print_msg(MSGCHAN_PROMPT, "%s", name.c_str());
                }
                if (oh.valid())
                {
                    oh.snapc()->get_name(&name);
                    print_msg(MSGCHAN_PROMPT, "%s", name.c_str());
                }
                print_msg(MSGCHAN_PROMPT, "%s", terrain_data[currlev->terrain_at(mappos)].name);
            }
        }
        else
        {
            if (currlev->outofbounds(mappos + step))
            {
                continue;
            }
            libmrl::Coord tmp_spos = screenpos + step;
            if ((tmp_spos.y < 0) || (tmp_spos.x < 0) ||
                (tmp_spos.y >= DISP_HEIGHT) || (tmp_spos.x >= DISP_WIDTH))
            {
                continue;
            }
            screenpos = tmp_spos;
            mappos = mappos + step;
            wmove(world_window, screenpos.y, screenpos.x);
            wrefresh(world_window);
        }
    }
    print_msg(MSGCHAN_PROMPT, "Done.");
}

void print_version(void)
{
    print_msg(0, "You are using Martin's Dungeon Bash version %s", LONG_VERSION);
}

/* display.cc */
