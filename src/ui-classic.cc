/* ui-classic.cc
 * 
 * Copyright 2005-2010 Martin Read and Stefan O'Rear
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

#define UI_CLASSIC_CC
#include "dunbash.hh"
#include "monsters.hh"
#include "objects.hh"
#include "player.hh"
#include "display.hh"
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <libmormegil/stlprintf.hh>
#include "cfgfile.hh"

/* Prototypes for static funcs */
static int object_char(int object_id);
static int monster_char(int monster_id);
static int terrain_char(Terrain_num terrain_type);
static void print_help_en_GB(void);
static void prepend_item_colour(Obj const *optr, std::string& str);
static void append_slot_maybe(Obj_handle oh, std::string& str);

static int terrain_char(Terrain_num terrain_type)
{
    return (terrain_data[terrain_type].colour << 8) | terrain_data[terrain_type].symbol;
}

static int monster_char(int monster_id)
{
    return (permons[monster_id].sym) | (permons[monster_id].colour << 8);
}

static int object_char(int object_id)
{
    return permobjs[object_id].sym | (permobjs[object_id].colour << 8);
}

void touch_back_buffer(void)
{
    libmormegil::Coord c;
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

void newsym(libmormegil::Coord c)
{
    int ch;

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
            back_buffer[c.y][c.x] = '*' | (projectile_colour << 8);
        }
        else if (c == u.pos)
        {
            back_buffer[c.y][c.x] = '@' | (you_colour << 8);
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

/* extern funcs */

void ui_init(void)
{
}

void press_enter(void)
{
    int ch;
    print_msg(0, "Press RETURN or SPACE to continue");
    while (1)
    {
        ch = get_silent();
        if ((ch == ' ') || (ch == '\n') || (ch == '\r'))
        {
            break;
        }
    }
}

void print_msg(int channel, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    std::string msg = libmormegil::vstlprintf(fmt, ap);
    va_end(ap);

    message_line(!suppressions[channel], msg, 0);
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

void print_equipped(void)
{
    std::string namestr;
    int i;
    print_msg(0, "You have the following items equipped:");
    if (u.weapon.valid())
    {
        u.weapon.snapc()->get_name(&namestr);
        i = get_inventory_slot(u.weapon);
        prepend_item_colour(u.weapon.snapc(), namestr);
        append_slot_maybe(u.weapon, namestr);
        print_msg(0, "%c) %s", 'a' + i, namestr.c_str());
    }
    if (u.armour.valid())
    {
        u.armour.snapc()->get_name(&namestr);
        i = get_inventory_slot(u.armour);
        prepend_item_colour(u.armour.snapc(), namestr);
        append_slot_maybe(u.armour, namestr);
        print_msg(0, "%c) %s", 'a' + i, namestr.c_str());
    }
    if (u.ring.valid())
    {
        u.ring.snapc()->get_name(&namestr);
        i = get_inventory_slot(u.ring);
        prepend_item_colour(u.ring.snapc(), namestr);
        append_slot_maybe(u.ring, namestr);
        print_msg(0, "%c) %s", 'a' + i, namestr.c_str());
    }
}

void prepend_item_colour(Obj const *optr, std::string& str)
{
    if (fruit_salad_inventory)
    {
        std::string colour = "";
        switch (optr->quality())
        {
        case Itemqual_bad:
            colour = "<red>";
            break;
        case Itemqual_normal:
            break;
        case Itemqual_good:
            colour = "<lgreen>";
            break;
        case Itemqual_great:
            colour = "<lblue>";
            break;
        case Itemqual_excellent:
            colour = "<purple>";
            break;
        }
        str = colour + str;
    }
}

void append_slot_maybe(Obj_handle oh, std::string& str)
{
    if (u.ring == oh)
    {
        str += "<lgrey> (on finger)";
    }
    else if (u.weapon == oh)
    {
        str += "<lgrey> (in hand)";
    }
    else if (u.armour == oh)
    {
        str += "<lgrey> (being worn)";
    }
}

void print_inv(Poclass_num filter)
{
    int i;
    std::string namestr;
    Obj const *optr;
    for (i = 0; i < 19; i++)
    {
        optr = u.inventory[i].snapc();
        if (optr && ((filter == POCLASS_NONE) || (permobjs[optr->obj_id].poclass == filter)))
        {
            u.inventory[i].snapc()->get_name(&namestr);
            prepend_item_colour(u.inventory[i].snapc(), namestr);
            append_slot_maybe(u.inventory[i], namestr);
            print_msg(0, "%c) %s", 'a' + i, namestr.c_str());
            // XXX avoid coloring the letter
        }
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
    std::string prompt = libmormegil::stlprintf("What do you want to %s? ", action);
    ch = (message_line(true, prompt, 1))[0];
    switch (ch)
    {
    case '-':
        if (accept_blank)
        {
            return -2;
        }
    case 'x':
    case '\x1b':
    case ' ':
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

int select_dir(libmormegil::Offset *psign, bool silent)
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
        ch = get_silent();
        switch (ch)
        {
        case 'h':
            *psign = dunbash::WEST;
            done = 1;
            break;
        case 'j':
            *psign = dunbash::SOUTH;
            done = 1;
            break;
        case 'k':
            *psign = dunbash::NORTH;
            done = 1;
            break;
        case 'l':
            *psign = dunbash::EAST;
            done = 1;
            break;
        case 'y':
            *psign = dunbash::NORTHWEST;
            done = 1;
            break;
        case 'u':
            *psign = dunbash::NORTHEAST;
            done = 1;
            break;
        case 'b':
            *psign = dunbash::SOUTHWEST;
            done = 1;
            break;
        case 'n':
            *psign = dunbash::SOUTHEAST;
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

Game_cmd get_command(void)
{
    int ch;
    int done = 0;
    while (!done)
    {
        ch = get_silent();
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
        case '1':
            return PROFCMD_1;
        case '2':
            return PROFCMD_2;
        case '3':
            return PROFCMD_3;
        case '4':
            return PROFCMD_4;
        case '5':
            return PROFCMD_5;
        case '6':
            return PROFCMD_6;
        case '7':
            return PROFCMD_7;
        case '8':
            return PROFCMD_8;
        case '9':
            return PROFCMD_9;
        case '0':
            return PROFCMD_0;
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

void pressanykey(void)
{
    message_line(true, "Press any key to continue...", 0);
    get_silent();
}

int getYN(const char *msg)
{
    int ch;
    print_msg(MSGCHAN_PROMPT, "%s", msg);
    ch = message_line(true, "Press capital Y to confirm, any other key to cancel", 1)[0];
    if (ch == 'Y')
    {
        return 1;
    }
    return 0;
}

int getyn(const char *msg)
{
    int ch;
    ch = message_line(true, msg, 1)[0];
    while (1)
    {
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
            ch = message_line(true, "Invalid response. Press y or n (ESC or space to cancel)", 1)[0];
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
    print_msg(0, "g   pick up an item (comma also works)");
    print_msg(0, "d   drop an item");
    print_msg(0, "e   eat something edible");
    print_msg(0, ">   go down stairs");
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

void animate_projectile(libmormegil::Coord pos, Dbash_colour col)
{
    if (!pos_visible(pos))
    {
        return;
    }
    projectile_colour = col;
    last_projectile_pos = curr_projectile_pos;
    curr_projectile_pos = pos;
    if (last_projectile_pos != dunbash::NOWHERE)
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
    curr_projectile_pos = dunbash::NOWHERE;
    newsym(last_projectile_pos);
    last_projectile_pos = dunbash::NOWHERE;
    display_update();
}

void farlook(void)
{
    libmormegil::Coord mappos = u.pos;
    libmormegil::Offset step;
    std::string name;
    bool done = false;
    int i;

    print_msg(MSGCHAN_PROMPT, "Use the movement keys to move the cursor.");
    print_msg(MSGCHAN_PROMPT, "Press '.' to examine a square, ESC/SPACE to finish.");
    cursor_highlight(mappos);
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

            if (!cursor_highlight(mappos + step))
            {
                continue;
            }

            mappos = mappos + step;
        }
    }
    print_msg(MSGCHAN_PROMPT, "Done.");
}

int get_smite_target(libmormegil::Coord *ppos, bool must_be_visible)
{
    libmormegil::Coord mappos = u.pos;
    libmormegil::Offset step;
    std::string name;
    bool done = false;
    int i;

    print_msg(MSGCHAN_PROMPT, "Use the movement keys to move the cursor.");
    print_msg(MSGCHAN_PROMPT, "Use '.'/ENTER to select a square, ESC/SPACE to cancel.");
    cursor_highlight(mappos);
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
                print_msg(MSGCHAN_PROMPT, "Invalid target location");
                i = -1;
                done = true;
            }
            else if (must_be_visible && !pos_visible(mappos))
            {
                print_msg(MSGCHAN_PROMPT, "You can't see that square to target it.");
            }
            else
            {
                i = 0;
                done = true;
            }
        }
        else
        {
            if (currlev->outofbounds(mappos + step))
            {
                continue;
            }

            if (!cursor_highlight(mappos + step))
            {
                continue;
            }

            mappos = mappos + step;
        }
    }
    return (i == -1) ? -1 : 0;
}

void print_version(void)
{
    print_msg(0, "You are using Martin's Dungeon Bash version %s", LONG_VERSION);
}

/* ui-classic.cc */
