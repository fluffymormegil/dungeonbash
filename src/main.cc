/* main.cc
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

#include "dunbash.hh"
#include "objects.hh"
#include "perseff.hh"
#include "monsters.hh"
#include "combat.hh"
#include "pobjid.hh"
#include "loadsave.hh"
#include <list>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <limits.h>
#include "cfgfile.hh"

int user_uid;
int game_uid;
int run_delay;

std::string configured_name;
std::string configured_language;
std::string configured_system_playground(PLAYGROUND);
std::string preferred_display;

const libmrl::Coord libmrl::NOWHERE = { INT_MIN, INT_MIN };
const libmrl::Coord libmrl::NORTH = { -1, 0 };
const libmrl::Coord libmrl::WEST = { 0, -1 };
const libmrl::Coord libmrl::EAST = { 0, 1 };
const libmrl::Coord libmrl::SOUTH = { 1, 0 };
const libmrl::Coord libmrl::NORTHEAST = { -1, 1 };
const libmrl::Coord libmrl::NORTHWEST = { -1, -1 };
const libmrl::Coord libmrl::SOUTHEAST = { 1, 1 };
const libmrl::Coord libmrl::SOUTHWEST = { 1, -1 };

int select_and_use_item(Poclass_num pocl, const char *verb, Itemuse_fptr func);

const Terrain_desc terrain_data[] =
{
    {
        "stone wall", true, true, false, false, '#', DBCLR_BROWN
    },
    {
        "bone wall", true, true, false, false, '#', DBCLR_WHITE
    },
    {
        "iron wall", true, true, false, false, '#', DBCLR_CYAN
    },
    {
        "red stone wall", true, true, false, false, '#', DBCLR_RED
    },
    {
        "fleshy wall", true, true, false, false, '#', DBCLR_L_PURPLE
    },
    {
        "plain stone floor", false, false, false, false, '.', DBCLR_L_GREY
    },
    {
        "bone floor", false, false, false, false, '.', DBCLR_WHITE
    },
    {
        "iron floor", false, false, false, false, '.', DBCLR_CYAN
    },
    {
        "red stone floor", false, false, false, false, '.', DBCLR_RED
    },
    {
        "fleshy floor", false, false, false, false, '.', DBCLR_RED
    },
    {
        "door", true, false, true, false, '+', DBCLR_L_GREY
    },
    // Levelportals
    {
        "downward staircase", false, false, true, false, '>', DBCLR_L_GREY
    },
    {
        "upward staircase", false, false, true, false, '<', DBCLR_L_GREY
    },
    // Fittings
    {
        "altar", false, false, true, false, '_', DBCLR_L_GREY
    },
    {
        "furnace", false, true, true, false, '|', DBCLR_D_GREY
    },
    {
        "anvil", false, false, true, false, '-', DBCLR_D_GREY
    },
    {
        "tombstone", false, false, true, false, '+', DBCLR_D_GREY
    },
    // Pools are about mid-thigh depth. Water isn't hostile... yet.
    {
        "pool of water", false, false, true, false, '\"', DBCLR_BLUE
    },
    {
        "pool of acid", false, false, true, true, '\"', DBCLR_GREEN
    },
    {
        "pool of lava", false, false, true, true, '\"', DBCLR_RED
    }
};

static void new_game(void);
static void main_loop(void);
bool game_finished = false;
uint32_t game_tick = 0;
bool wizard_mode = WIZARD_MODE;

unsigned int convert_range(int dy, int dx)
{
    int ady, adx;
    ady = dy > 0 ? dy : -dy;
    adx = dx > 0 ? dx : -dx;
    if (ady > adx)
    {
	return ady;
    }
    return adx;
}

int exclusive_flat(int lower, int upper)
{
    if (lower < upper)
    {
        return lower + one_die(upper - lower - 1);
    }
    else
    {
        return upper + one_die(lower - upper - 1);
    }
}

int inclusive_flat(int lower, int upper)
{
    if (lower < upper)
    {
        return lower + zero_die(upper - lower + 1);
    }
    else
    {
        return upper + zero_die(lower - upper + 1);
    }
}

int one_die(int sides)
{
    int rval;
    if (sides < 2)
    {
	return 1;
    }
    rval = 1 + (rng() / ((RNG_MAX / sides) + 1));
    return rval;
}

int zero_die(int sides)
{
    int rval;
    if (sides < 2)
    {
	return 0;
    }
    rval = rng() / ((RNG_MAX / sides) + 1);
    return rval;
}

int dice(int count, int sides)
{
    int total = 0;
    for ( ; count > 0; count--)
    {
	total += one_die(sides);
    }
    return total;
}

void new_game(void)
{
    rng_init();
    u_init();
    flavours_init();
    Level_tag startpoint = { Dungeon_main, 1 };
    go_to_level(startpoint, Leventry_stairs_dn_1);
    status_updated = 1;
    map_updated = 1;
    hard_redraw = 1;
    print_msg(0, "Welcome to Martin's Infinite Dungeon.");
    print_msg(0, "Press '?' for help.");
}

int resolve_dance(Game_cmd cmd)
{
    int rv = 1;
    int i;
    libmrl::Coord stepdir = { 0, 0 };
    disturb_u();
    switch (cmd)
    {
    case MOVE_NORTH:
        stepdir = libmrl::NORTH;
        break;
    case MOVE_NE:
        stepdir = libmrl::NORTHEAST;
        break;
    case MOVE_EAST:
        stepdir = libmrl::EAST;
        break;
    case MOVE_SE:
        stepdir = libmrl::SOUTHEAST;
        break;
    case MOVE_SOUTH:
        stepdir = libmrl::SOUTH;
        break;
    case MOVE_SW:
        stepdir = libmrl::SOUTHWEST;
        break;
    case MOVE_WEST:
        stepdir = libmrl::WEST;
        break;
    case MOVE_NW:
        stepdir = libmrl::NORTHWEST;
        break;
    case ATTACK:
	i = select_dir(&stepdir);
        if (i == -1)
        {
            rv = 0;
        }
        break;
    case DUMP_CHARA:
    case SHOW_INVENTORY:
    case SHOW_EQUIPPED:
        // Status inspection commands 
        rv = 0;
        break;
    default:
        break;
    }
    if (int(stepdir) && currlev->monster_at(u.pos + stepdir).valid())
    {
        rv = player_attack(stepdir);
    }
    else
    {
        print_msg(0, "You stumble.");
    }
    return rv;
}

int do_command(Game_cmd cmd)
{
    int i;
    int j;
    libmrl::Coord step;
    std::string namestr;
    switch (cmd)
    {
    case VOCALIZE_WORD:
        return 0;
    case FARMOVE_NORTH:
	return farmove_player(libmrl::NORTH);
    case FARMOVE_SOUTH:
	return farmove_player(libmrl::SOUTH);
    case FARMOVE_EAST:
	return farmove_player(libmrl::EAST);
    case FARMOVE_WEST:
	return farmove_player(libmrl::WEST);
    case FARMOVE_NW:
	return farmove_player(libmrl::NORTHWEST);
    case FARMOVE_NE:
	return farmove_player(libmrl::NORTHEAST);
    case FARMOVE_SE:
	return farmove_player(libmrl::SOUTHEAST);
    case FARMOVE_SW:
	return farmove_player(libmrl::SOUTHWEST);

    case MOVE_NORTH:
	return move_player(libmrl::NORTH);
    case MOVE_SOUTH:
	return move_player(libmrl::SOUTH);
    case MOVE_EAST:
	return move_player(libmrl::EAST);
    case MOVE_WEST:
	return move_player(libmrl::WEST);
    case MOVE_NW:
	return move_player(libmrl::NORTHWEST);
    case MOVE_NE:
	return move_player(libmrl::NORTHEAST);
    case MOVE_SE:
	return move_player(libmrl::SOUTHEAST);
    case MOVE_SW:
	return move_player(libmrl::SOUTHWEST);

    case ATTACK:
	i = select_dir(&step);
	if (i != -1)
	{
	    return player_attack(step);
	}
	return 0;

    case GET_ITEM:
	if (currlev->object_at(u.pos).valid())
	{
	    attempt_pickup();
	    return 1;
	}
	else
	{
	    print_msg(MSGCHAN_MINORFAIL, "Nothing to get.");
	    return 0;
	}

    case WIELD_WEAPON:
	j = 0;
	i = inv_select(POCLASS_WEAPON, "wield", 1);
	if (i == -2)
	{
	    u.weapon = NO_OBJECT;
	    print_msg(0, "Weapon unwielded.");
	}
	else if (i >= 0)
	{
	    u.weapon = u.inventory[i];
	    j = 1;
            u.weapon.snapc()->get_name(&namestr);
	    print_msg(0, "Wielding %s.", namestr.c_str());
	}
	return j;

    case WEAR_ARMOUR:
	if (u.armour.valid())
	{
	    print_msg(MSGCHAN_MINORFAIL, "You are already wearing armour.");
	    return 0;
	}
	i = inv_select(POCLASS_ARMOUR, "wear", 0);
	if (i >= 0)
	{
	    u.armour = u.inventory[i];
	    permobjs[u.armour.otyp()].known = 1;
	    recalc_defence();
            u.armour.snapc()->get_name(&namestr);
	    print_msg(0, "Wearing %s.", namestr.c_str());
	    return 1;
	}
	return 0;
    case TAKE_OFF_ARMOUR:
	if (u.armour.valid())
	{
	    u.armour = NO_OBJECT;
	    recalc_defence();
	    print_msg(0, "You take off your armour.");
	    return 1;
	}
	else
	{
	    print_msg(0, "You aren't wearing any armour.");
	    return 0;
	}

    case GIVE_HELP:
	print_help();
	return 0;

    case READ_SCROLL:
        return select_and_use_item(POCLASS_SCROLL, "read", read_scroll);

    case GO_UP_STAIRS:
	if (currlev->terrain_at(u.pos) == STAIRS_UP)
        {
            print_msg(MSGCHAN_MINORFAIL, "The curse of the dungeon prevents you from ascending.");
        }
        else
        {
            print_msg(MSGCHAN_MINORFAIL, "There are no up stairs here.");
        }
        return 0;
    case GO_DOWN_STAIRS:
	if (currlev->terrain_at(u.pos) == STAIRS_DOWN)
	{
            Leventry_mode lem = currlev->exit_modes[u.pos];
            Level_tag lt = currlev->exit_dests[lem];
            depart_level(lem);
            go_to_level(lt, lem);
	}
	else
	{
	    print_msg(MSGCHAN_MINORFAIL, "There are no down stairs here.");
	}
	return 0;

    case STAND_STILL:
	return 1;

    case EAT_FOOD:
        return select_and_use_item(POCLASS_FOOD, "eat", eat_food);

    case QUAFF_POTION:
        return select_and_use_item(POCLASS_POTION, "quaff", quaff_potion);

    case ZAP_WAND:
        return select_and_use_item(POCLASS_WAND, "zap", zap_wand);

    case ACTIVATE_MISC:
        return select_and_use_item(POCLASS_MISC, "activate", activate_misc);

    case REMOVE_RING:
        return u.on_remove();

    case PUT_ON_RING:
	if (u.ring.valid())
	{
	    print_msg(MSGCHAN_MINORFAIL, "You are already wearing a ring.");
	    return 0;
	}
	i = inv_select(POCLASS_RING, "put on", 0);
	if (i >= 0)
	{
	    u.ring = u.inventory[i];
            u.ring.snapc()->get_name(&namestr);
            print_msg(0, "You put on %s.", namestr.c_str());
	    return 1;
	}
	return 0;

    case INSPECT_ITEM:
	i = inv_select(POCLASS_NONE, "inspect", 0);
	if ((i >= 0) && (u.inventory[i].valid()))
	{
	    describe_object(u.inventory[i]);
	}
	return 0;

    case FLOORLOOK:
        look_at_floor();
        return 0;
    case FARLOOK:
        farlook();
	return 0;
    case SHOW_DISCOVERIES:
	show_discoveries();
	return 0;
    case SHOW_TERRAIN:
	show_terrain = 1;
	map_updated = 1;
        touch_back_buffer();
	display_update();
	print_msg(0, "Display of monsters and objects suppressed.");
	press_enter();
	show_terrain = 0;
	map_updated = 1;
        touch_back_buffer();
	display_update();
	return 0;
    case RNG_TEST:
	{
	    int odds = 0;
	    int evens = 0;
	    for (i = 0; i < 100000; i++)
	    {
		if (zero_die(2))
		{
		    odds++;
		}
		else
		{
		    evens++;
		}
	    }
	    print_msg(0, "100k rolls: 0 %d, 1 %d", odds, evens);
	}
	print_msg(0, "1d2-1: %d %d %d %d %d %d %d %d", zero_die(2), zero_die(2), zero_die(2), zero_die(2), zero_die(2), zero_die(2), zero_die(2), zero_die(2));
	print_msg(0, "1d8-1: %d %d %d %d %d %d %d %d", zero_die(8), zero_die(8), zero_die(8), zero_die(8), zero_die(8), zero_die(8), zero_die(8), zero_die(8));
	print_msg(0, "1d32-1: %d %d %d %d %d %d %d %d", zero_die(32), zero_die(32), zero_die(32), zero_die(32), zero_die(32), zero_die(32), zero_die(32), zero_die(32));
	return 0;
    case DROP_ITEM:
	i = inv_select(POCLASS_NONE, "drop", 0);
	if (i >= 0)
	{
	    if ((u.inventory[i].valid()) &&
		((u.inventory[i] == u.ring) ||
		 (u.inventory[i] == u.armour)))
	    {
		print_msg(MSGCHAN_MINORFAIL, "You cannot drop something you are wearing.");
		return 0;
	    }
	    j = drop_obj(i);
	    if (j == -1)
	    {
		return 0;
	    }
	    return 1;
	}
	return 0;
    case DUMP_CHARA:
	write_char_dump();
	return 0;
    case SAVE_GAME:
	i = save_game();
	if (i == 0)
	{
	    print_msg(0, "Game saved; exiting.");
	    game_finished = true;
	}
	if (save_wait)
	{
	    press_enter();
	}
	return 0;
    case QUIT:
	j = getYN("Really quit?");
	if (j > 0)
	{
	    game_finished = true;
	    kill_game();
	}
	else
	{
	    print_msg(0, "Never mind.");
	}
	return 0;
    case SHOW_EQUIPPED:
        print_equipped();
        return 0;
    case SHOW_INVENTORY:
	print_msg(0, "You are carrying:");
	print_inv(POCLASS_NONE);
	return 0;

    case WIZARD_DUMP_PERSEFFS:
        if (wizard_mode)
        {
        for (std::list<Perseff_data>::iterator peff_iter = u.perseffs.begin();
             peff_iter != u.perseffs.end();
             ++peff_iter)
        {
            print_msg(0, "perseff flavour %d power %d duration %d by_you %d on_you %d",
                      peff_iter->flavour, peff_iter->power,
                      peff_iter->duration, peff_iter->by_you,
                      peff_iter->on_you);
        }
        }
        else
        {
            print_msg(MSGCHAN_MINORFAIL, "You aren't a wizard.");
        }
        return 0;

    case WIZARD_CURSE_ME:
        if (wizard_mode)
        {
            Perseff_data peff = 
            {
                Perseff_leadfoot_curse, 10, 100, true, true
            };
            u.apply_effect(peff);
            peff.flavour = Perseff_wither_curse;
            u.apply_effect(peff);
            peff.flavour = Perseff_armourmelt_curse;
            u.apply_effect(peff);
        }
        else
        {
            print_msg(MSGCHAN_MINORFAIL, "You aren't a wizard.");
        }
        return 0;
    case WIZARD_DESCEND:
        if (wizard_mode)
        {
            Leventry_mode lem = Leventry_stairs_dn_1;
            Level_tag lt = currlev->exit_dests[lem];
            depart_level(currlev->exit_modes[u.pos]);
            go_to_level(lt, lem);
        }
        else
        {
            print_msg(MSGCHAN_MINORFAIL, "You aren't a wizard.");
        }
        return 0;

    case PROFCMD_1:
        switch (u.job)
        {
        case Prof_fighter:
            // return uwhirl();
            break;
        default:
            print_msg(0, "Your current profession has no ability on that key.");
            break;
        }
        return 0;

    case PROFCMD_2:
        switch (u.job)
        {
        case Prof_fighter:
            select_dir(&step);
            if (step != libmrl::NOWHERE)
            {
            }
        }

    case PRINT_VERSION:
        print_version();
        return 0;

    case WIZARD_LEVELUP:
        if (wizard_mode)
        {
            gain_experience((lev_threshold(u.level) - u.experience) + 1);
        }
        else
        {
            print_msg(MSGCHAN_MINORFAIL, "You aren't a wizard.");
        }
        return 0;

    case WIZARD_TELEPORT:
        if (wizard_mode)
        {
            teleport_u();
        }
        else
        {
            print_msg(MSGCHAN_MINORFAIL, "You aren't a wizard.");
        }
        return 0;
    case NO_CMD:
        print_msg(MSGCHAN_MINORFAIL, "Received null command.");
        break;
    }
    return 0;
}

int action_speed;

void main_loop(void)
{
    Game_cmd cmd;
    int i;
    std::set<Mon_handle>::iterator miter;
    while (!game_finished)
    {
        display_update();
        switch (game_tick % 5)
        {
        case 0:
            action_speed = SPEED_VERY_SLOW;
            break;
        case 3:
            action_speed = SPEED_SLOW;
            break;
        case 2:
            action_speed = SPEED_NORMAL;
            break;
        case 1:
            action_speed = SPEED_FAST;
            break;
        case 5:
            action_speed = SPEED_VERY_FAST;
            break;
            // ULTRAFAST not supported for now. It takes extra work.
        }
        if (action_speed <= u.speed)
        {
            i = 0;
            while (!i)
            {
                if (u.farmoving)
                {
                    usleep(run_delay * 1000);
                    i = move_player(u.farmove_direction);
                    if (get_interrupt())
                    {
                        disturb_u();
                    }
                }
                else
                {
                    /* Take commands until the player does
                     * something that uses an action. */
                    cmd = get_command();
                    i = do_command(cmd);
                    if (game_finished)
                    {
                        break;
                    }
                }
                Obj const *optr = u.weapon.snapc();
                if ((i == 1) && was_move_command(cmd) && player_next_to_mon() &&
                    ((!optr) || !optr->is_ranged()))
                {
                    // A revolting hack follows, allowing the player to make
                    // a melee attack after moving.
                    do
                    {
                        Game_cmd cmd2 = get_command();
                        i = resolve_dance(cmd2);
                        if (game_finished)
                        {
                            break;
                        }
                    } while (!i);
                }
                if (i < 0)
                {
                    i = 0;
                }
            }
            if (game_finished)
            {
                break;
            }
            /* If you're wearing a ring of doom, zap you. */
            if (u.ring.otyp() == PO_DOOM_RING)
            {
                print_msg(0, "Your ring pulses uncleanly.");
                damage_u(1, DEATH_KILLED, "a ring of doom");
                display_update();
                permobjs[PO_DOOM_RING].known = 1;
            }
            else if (u.ring.otyp() == PO_TELEPORT_RING)
            {
                if (!zero_die(75))
                {
                    print_msg(0, "Your ring flares white!");
                    permobjs[PO_TELEPORT_RING].known = 1;
                    teleport_u();
                }
            }
        }
        // TODO add timed-event queue
        for (miter = currlev->denizens.begin(); miter != currlev->denizens.end();)
        {
            bool monwiped;
            Mon_handle saved_id;
            std::set<Mon_handle>::iterator tmpiter;
            Mon *mptr;
            mptr = miter->snapv();
            saved_id = *miter;
            monwiped = false;
            if (!mptr)
            {
                print_msg(MSGCHAN_INTERROR, "current map's denizen list contains bad handles");
                abort();
            }
            /* Update the monster's status. */
            monwiped = update_mon(*miter);
            if (!monwiped && (action_speed <= permons[mptr->mon_id].speed))
            {
                monwiped = mon_acts(*miter);
            }
            if (game_finished)
            {
                break;
            }
            if (monwiped)
            {
                // miter is no longer valid; find the earliest insertion point
                // for the saved handle value instead.
                tmpiter = currlev->denizens.lower_bound(saved_id);
                miter = tmpiter;
            }
            else
            {
                // advance miter.
                ++miter;
            }
        }
        if (game_finished)
        {
            break;
        }
        update_player();
        game_tick++;
    }
}

int user_permissions(void)
{
#ifdef _POSIX_SAVED_IDS
    return seteuid(user_uid);
#else
    return setreuid(game_uid, user_uid);
#endif
}

int game_permissions(void)
{
#ifdef _POSIX_SAVED_IDS
    return seteuid(game_uid);
#else
    return setreuid(user_uid, game_uid);
#endif
}

int main(void)
{
    int i;
#ifdef MULTIUSER
    user_uid = getuid();
    game_uid = geteuid();
#endif
    get_config();
    display_init();
    ui_init();
    ptac_init();
    monsters_init();
    i = load_game();
    if (i == -1)
    {
        // There was no game to load. If there was a broken game, we'd have
        // exit()ed instead.
        new_game();
    }
    main_loop();
    display_shutdown();
    return 0;
}

const char *numberwords[40] =
{
    "zero", "one", "two", "three", "four",
    "five", "six", "seven", "eight", "nine",
    "ten", "eleven", "twelve", "thirteen", "fourteen",
    "fifteen", "sixteen", "seventeen", "eighteen", "nineteen",
    "twenty", "twenty-one", "twenty-two", "twenty-three", "twenty-four",
    "twenty-five", "twenty-six", "twenty-seven", "twenty-eight", "twenty-nine",
    "thirty", "thirty-one", "thirty-two", "thirty-three", "thirty-four",
    "thirty-five", "thirty-six", "thirty-seven", "thirty-eight", "thirty-nine"
};

int select_and_use_item(Poclass_num pocl, const char *verb, Itemuse_fptr func)
{
    int i;
    int j;
    i = inv_select(pocl, verb, 0);
    if (i >= 0)
    {
        j = func(u.inventory[i]);
        return 1;
    }
    return 0;
}

/* main.c */
