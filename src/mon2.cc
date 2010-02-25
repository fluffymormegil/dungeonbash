/* mon2.c
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

/* TODO: Convert missile AI to a new-style AI function. */
#define MON2_C
#include "dunbash.hh"
#include "monsters.hh"
#include "objects.hh"
#include "bmagic.hh"
#include "combat.hh"

/* AI map cell descriptor. */
struct Ai_cell {
    libmrl::Coord pos;
    libmrl::Coord delta;
    int score;
};

/* prototypes for AI preference functions. */
static void get_naive_prefs(libmrl::Coord pos, libmrl::Coord delta, libmrl::Coord *pref_pos);
static void get_drunk_prefs(libmrl::Coord pos, libmrl::Coord delta, libmrl::Coord *pref_pos);
static void build_ai_cells(Ai_cell *cells, libmrl::Coord pos);
static int ai_cell_compare(Ai_cell *cell, libmrl::Coord delta);
static void get_dodger_prefs(libmrl::Coord pos, libmrl::Coord delta, libmrl::Coord *pref_pos);
static void get_chase_prefs(Mon_handle mon, libmrl::Coord *pref_pos);

/* get_drunk_prefs()
 *
 * Fills the three-entry preference arrays with three randomly-selected
 * adjacent squares.
 */

static void get_drunk_prefs(libmrl::Coord pos, libmrl::Coord delta, libmrl::Coord *pref_pos)
{
    libmrl::Coord sgn;
    int tryct;
    int pref_idx;
    int idx2;
    bool retry;
    pref_pos[0] = pos;
    pref_pos[1] = pos;
    pref_pos[2] = pos;
    for (pref_idx = 0; pref_idx < 3; pref_idx++)
    {
	for (tryct = 0; tryct < 40; tryct++)
	{
	    retry = false;
	    sgn.y = zero_die(3) - 1;
	    sgn.x = zero_die(3) - 1;
	    if (!sgn.y && !sgn.x)
	    {
		continue;
	    }
	    for (idx2 = 0; idx2 < pref_idx; idx2++)
	    {
		if (pref_pos[idx2].y == pos + sgn)
		{
		    retry = true;
		    break;
		}
	    }
	    if (retry) 
	    {
		continue;
	    }
	    pref_pos[pref_idx] = pos + sgn;
	    break;
	}
    }
}

/* get_chase_prefs()
 *
 * The naive "chase" AI is used by non-stupid non-smart monsters to chase your
 * last known position. If after moving towards it once they can't see you,
 * they will give up and revert to "drunk" AI. (Contrast stupid monsters,
 * who always use "drunk" AI if they can't see you, and smart monsters, who
 * always use "seeking" AI if they can't see you.)
 *
 * This function takes different parameters to the other AI preference
 * functions because it has to have access to the monster's lasty/lastx
 * details.
 */

static void get_chase_prefs(Mon_handle mon, libmrl::Coord *pref_pos)
{
    Mon *mptr = mon.snapv();
    libmrl::Coord pos = mptr->pos;
    libmrl::Coord delta = mptr->ai_lastpos - pos;
    libmrl::Coord sgn = libmrl::sign(delta);
    libmrl::Coord adelta = libmrl::abs(delta);

    if (mptr->can_pass(pos + sgn))
    {
        *pref_pos = pos + sgn;
    }
    else
    {
        if (!(sgn.y))
        {
            /* We're on the horizontal; check the horizontally adjacent
             * square, then the squares one square north or south in a
             * random order. */
            pref_pos[1].x = pref_pos[2].x = pos.x + sgn.x;
            if (zero_die(2))
            {
                pref_pos[1].y = pos.y - 1;
                pref_pos[2].y = pos.y + 1;
            }
            else
            {
                pref_pos[1].y = pos.y + 1;
                pref_pos[2].y = pos.y - 1;
            }
        }
        else if (!(sgn.x))
        {
            /* We're on the horizontal; check the horizontally adjacent
             * square, then the squares one square north or south in a
             * random order. */
            pref_pos[1].y = pref_pos[2].y = pos.y + sgn.y;
            if (zero_die(2))
            {
                pref_pos[1].x = pos.x - 1;
                pref_pos[2].x = pos.x + 1;
            }
            else
            {
                pref_pos[1].x = pos.x + 1;
                pref_pos[2].x = pos.x - 1;
            }
        }
        else
        {
            if (zero_die(2))
            {
                pref_pos[1].x = pos.x;
                pref_pos[1].y = pos.y + sgn.y;
                pref_pos[2].x = pos.x + sgn.x;
                pref_pos[2].y = pos.y;
            }
            else
            {
                pref_pos[2].x = pos.x;
                pref_pos[2].y = pos.y + sgn.y;
                pref_pos[1].x = pos.x + sgn.x;
                pref_pos[1].y = pos.y;
            }
        }
        if (mptr->can_pass(pref_pos[1]))
        {
            pref_pos[0] = pref_pos[1];
        }
        else if (mptr->can_pass(pref_pos[2]))
        {
            pref_pos[0] = pref_pos[2];
        }
        else
        {
            pref_pos[0] = mptr->pos;
        }
    }
}

/* get_naive_prefs()
 *
 * Fills the three-entry preference arrays with three best choices for closing
 * with the player - optimal first, then secondaries in random order as #2 and
 * #3.
 */

static void get_naive_prefs(libmrl::Coord pos, libmrl::Coord delta, libmrl::Coord *pref_pos)
{
    libmrl::Coord sgn = libmrl::sign(delta);
    libmrl::Coord adelta = libmrl::abs(delta);
    pref_pos[0] = pos + sgn;
    if (!sgn.y)
    {
        /* We're on the horizontal; check the horizontally adjacent
         * square, then the squares one square north or south in a
         * random order. */
        if (zero_die(2))
        {
            pref_pos[1].y = pos.y - 1;
            pref_pos[2].y = pos.y + 1;
        }
        else
        {
            pref_pos[1].y = pos.y + 1;
            pref_pos[2].y = pos.y - 1;
        }
        pref_pos[1].x = pos.x + sgn.x;
        pref_pos[2].x = pos.x + sgn.x;
    }
    else if (!sgn.x)
    {
        if (zero_die(2))
        {
            pref_pos[1].x = pos.x - 1;
            pref_pos[2].x = pos.x + 1;
        }
        else
        {
            pref_pos[1].x = pos.x + 1;
            pref_pos[2].x = pos.x - 1;
        }
        pref_pos[1].y = pos.y + sgn.y;
        pref_pos[2].y = pos.y + sgn.y;
    }
    else
    {
        if (zero_die(2))
        {
            pref_pos[1].x = pos.x;
            pref_pos[1].y = pos.y + sgn.y;
            pref_pos[2].x = pos.x + sgn.x;
            pref_pos[2].y = pos.y;
        }
        else
        {
            pref_pos[2].x = pos.x;
            pref_pos[2].y = pos.y + sgn.y;
            pref_pos[1].x = pos.x + sgn.x;
            pref_pos[1].y = pos.y;
        }
    }
}

/* XXX build_ai_cells()
 *
 * Populate array of eight AI cell descriptors.
 */

static void build_ai_cells(Ai_cell *cells, libmrl::Coord pos)
{
    cells[0].score = 0;
    cells[1].score = 0;
    cells[2].score = 0;
    cells[3].score = 0;
    cells[4].score = 0;
    cells[5].score = 0;
    cells[6].score = 0;
    cells[7].score = 0;
    cells[0].pos = pos + libmrl::NORTHWEST;
    cells[1].pos = pos + libmrl::NORTH;
    cells[2].pos = pos + libmrl::NORTHEAST;
    cells[3].pos = pos + libmrl::WEST;
    cells[4].pos = pos + libmrl::EAST;
    cells[5].pos = pos + libmrl::SOUTHWEST;
    cells[6].pos = pos + libmrl::SOUTH;
    cells[7].pos = pos + libmrl::SOUTHEAST;
}

/* XXX ai_cell_compare()
 *
 * Find relative range of cell compared to monster's current range.
 */
static int ai_cell_compare(Ai_cell *cell, libmrl::Coord delta)
{
    /* returns -1 for closer, 0 for same range, +1 for further. */
    int pointrange = int(delta);
    int cellrange = int(cell->delta);
    if (cellrange < pointrange)
    {
        return -1;
    }
    else if (cellrange > pointrange)
    {
        return 1;
    }
    return 0;
}

/* XXX get_dodger_prefs()
 *
 * Get preferences for "smart" monsters without ranged attacks.
 */
static void get_dodger_prefs(libmrl::Coord pos, libmrl::Coord delta, libmrl::Coord *pref_pos)
{
    /* "Dodgers" are smart melee-only monsters. They will try to avoid
     * the cardinals as they close, and will even flow around other
     * monsters to try to get to the player. 
     *
     * This function does *all* the work of selecting a destination square
     * for a smart melee-only monster; accordingly, only pref_y[0] and
     * pref_x[0] get set.
     */
    Ai_cell ai_cells[8];
    Mon_handle mon = currlev->monster_at(pos);
    Mon *mptr = mon.snapv();
    int i;
    libmrl::Coord adelta;
    int j;
    int highest_score = -10000;
    int tryct;
    *pref_pos = pos;
    adelta = libmrl::abs(delta);
    build_ai_cells(ai_cells, pos);
    /* Build the local delta.x/delta.y arrays. */
    for (i = 0; i < 8; i++)
    {
        ai_cells[i].delta = u.pos - ai_cells[i].pos;
        /* Scoring factors:
         * Square on cardinal: -2.
         * Square closer to player: +1.
         * Square further from player: -3.
         * Square next to player: +10.
         *
         * Yes, this monster prizes not opening the range more than
         * it prizes staying off the cardinal; this is intentional.
         * It also prizes staying off the cardinal more than actually
         * closing. When I add more AI state to the monster structure,
         * this will change.
         */
        if (!mptr->can_pass(ai_cells[i].pos))
        {
            /* Square impassable. Set score WAY out of bounds
             * and continue. */
            ai_cells[i].score = -10000;
            continue;
        }
        /* Cardinality */
        if (ai_cells[i].delta.cardinal())
        {
            /* Score this square down for being on a cardinal. */
            ai_cells[i].score -= 2;
        }
        j = ai_cell_compare(ai_cells + i, delta);
        /* Range */
        if (libmrl::abs(int(ai_cells[i].delta)) < 2)
        {
            /* Score upward a *lot* for being adjacent to player */
            ai_cells[i].score += 10;
        }
        else if (j > 0)
        {
            ai_cells[i].score -= 3;
        }
        else if (j < 0)
        {
            ai_cells[i].score += 1;
        }
        if (ai_cells[i].score > highest_score)
        {
            highest_score = ai_cells[i].score;
        }
    }
    if (highest_score == -10000)
    {
        /* No good targets. */
        return;
    }
    for (tryct = 0; tryct < 32; tryct++)
    {
        i = zero_die(8);
        if (ai_cells[i].score == highest_score)
        {
            *pref_pos = ai_cells[i].pos;
            break;
        }
    }
    return;
}

void select_space(libmrl::Coord *ppos, libmrl::Coord delta, int selection_mode)
{
    libmrl::Coord ai_pos[3];
    libmrl::Coord sgn = libmrl::sign(delta);
    libmrl::Coord adelta = libmrl::abs(delta);
    libmrl::Coord pos2 = libmrl::NOWHERE;
    Mon *mptr = currlev->monster_at(*ppos).snapv();

    switch (selection_mode)
    {
    case AI_BANZAI:
        /* Simple convergence */
        get_naive_prefs(*ppos, delta, ai_pos);
        if (mptr->can_pass(ai_pos[0]))
        {
            pos2 = ai_pos[0];
        }
        else if (mptr->can_pass(ai_pos[1]))
        {
            pos2 = ai_pos[1];
        }
        else if (mptr->can_pass(ai_pos[2]))
        {
            pos2 = ai_pos[2];
        }
        else
        {
            pos2 = *ppos;
        }
        break;
    case AI_ARCHER:
        /* Converge to cardinal */
        if (delta.cardinal())
        {
            /* On cardinal. Stay there if we can. But close anyway. */
            pos2 = *ppos;
            pos2 += sgn;
            if (mptr->can_pass(pos2))
            {
                break;
            }
            pos2.x = ppos->x;
            if (mptr->can_pass(pos2))
            {
                break;
            }
            pos2.y = ppos->y;
            pos2.x = ppos->x + sgn.x;
            if (mptr->can_pass(pos2))
            {
                break;
            }
        }
        else if ((adelta.y == 1) || (adelta.y > adelta.x))
        {
            /* One step in ydir off EW cardinal, or further
             * off cardinal in y than in x */
            pos2 = *ppos;
            pos2.y += sgn.y;
            if (mptr->can_pass(pos2))
            {
                break;
            }
            pos2.x = ppos->x + sgn.x;
            if (mptr->can_pass(pos2))
            {
                break;
            }
            pos2.y = ppos->y;
            if (mptr->can_pass(pos2))
            {
                break;
            }
        }
        else if ((adelta.x == 1) || ((adelta.y > 1) && (adelta.x > adelta.y)))
        {
            /* One step off a diagonal cardinal, with adx > ady */
            pos2 = *ppos;
            pos2.x += sgn.x;
            if (mptr->can_pass(pos2))
            {
                break;
            }
            pos2.y = ppos->y + sgn.y;
            if (mptr->can_pass(pos2))
            {
                break;
            }
            pos2.x = ppos->x;
            if (mptr->can_pass(pos2))
            {
                break;
            }
        }
        pos2 = *ppos;
        break;
    case AI_DODGER:
        get_dodger_prefs(*ppos, delta, ai_pos);
        pos2 = ai_pos[0];
        break;
    case AI_DRUNK:
        /* "Drunk" monster i.e. monster moving while it doesn't know
         * how to find you. */
        get_drunk_prefs(*ppos, delta, ai_pos);
        if (mptr->can_pass(ai_pos[0]))
        {
            pos2 = ai_pos[0];
        }
        else if (mptr->can_pass(ai_pos[1]))
        {
            pos2 = ai_pos[1];
        }
        else if (mptr->can_pass(ai_pos[2]))
        {
            pos2 = ai_pos[2];
        }
        else
        {
            pos2 = *ppos;
        }
        break;
    case AI_STALK:
        /* "Seeking" monster i.e. monster moving while it can't see
         * you, but thinks it knows where you are. This AI isn't
         * great, but it'll do for now. */
        if (mptr->current_path)
        {
            if (!(mptr->current_path->empty()))
            {
                pos2 = astar_advance(mptr);
                if (!currlev->outofbounds(pos2))
                {
                    break;
                }
            }
            mptr->discard_path();
        }
        mptr->find_astar_path(mptr->ai_lastpos);
        if (mptr->current_path)
        {
            // Path is known-good.
            pos2 = astar_advance(mptr);
        }
        if (pos2 == libmrl::NOWHERE)
        {
            pos2 = mptr->pos;
        }
        break;
    case AI_CHASE:
        /* "chase" AI i.e. pursue your last known position. */
        get_chase_prefs(currlev->monster_at(*ppos), ai_pos);
        pos2 = ai_pos[0];
        break;
    }
    *ppos = pos2;
}

bool mon_acts(Mon_handle mon)
{
    Mon *mptr = mon.snapv();
    Direction_data dir_data;
    libmrl::Coord pos;
    int special_used = 0;
    std::string name;

    if (pmon_is_peaceful(mptr->mon_id))
    {
        // peaceful monsters do nothing.
        return false;
    }
    /* delta.y,delta.x == direction monster must go to reach you. */
    pos = mptr->pos;
    if (pos == u.pos)
    {
        print_msg(MSGCHAN_INTERROR, "Program disordered: monster in player's square.");
        print_msg(MSGCHAN_INTERROR, "Discarding misplaced monster.");
        release_monster(mon);
        currlev->set_mon_at(pos, -1);
        return true;
    }
    if (!(currlev->monster_at(mptr->pos) == mon))
    {
        print_msg(MSGCHAN_INTERROR, "Program disordered: monster(s) misplaced.");
        release_monster(mon);
        return true;
    }
    compute_directions(u.pos, mptr->pos, &dir_data);
    if (dir_data.meleerange)
    {
        /* Adjacent! Attack you.  Demons have a 1 in 10 chance of
         * attempting to summon another demon instead of attacking
         * you. */
        mptr->notice_you();
        if (pmon_is_demon(mptr->mon_id) && !zero_die(10))
        {
            // demons may summon weaker demons to assist them.
            int demon_num = weaker_demon(mptr->mon_id);
            if (demon_num != NO_PM)
            {
                summon_mon_near(demon_num, pos);
                special_used = 1;
            }
        }
        else if (pmon_is_magician(mptr->mon_id))
        {
            special_used = use_black_magic(mon);
        }
        if (!special_used)
        {
            mhitu(mon, DT_PHYS);
        }
    }
    else if (mptr->in_fov())
    {
        // TODO implement checking for Assassin Soul
        /* In FOV. */
        mptr->notice_you();
        if (pmon_is_magician(mptr->mon_id))
        {
            /* Two-thirds of the time, try to use black magic. */
            if (zero_die(6) < 4)
            {
                special_used = use_black_magic(mon);
            }
            if (special_used)
            {
                return false;
            }
            /* Didn't, or couldn't, use black magic; converge
             * as if an archer. */
            select_space(&pos, dir_data.delta, AI_ARCHER);
        }
        else if (pmon_is_archer(mptr->mon_id))
        {
            if (dir_data.oncardinal && (zero_die(6) < 3))
            {
                special_used = 1;
                mshootu(mon, permons[mptr->mon_id].ranged.dt);
            }
            if (special_used)
            {
                return false;
            }
            select_space(&pos, dir_data.delta, AI_ARCHER);
        }
        else if (pmon_is_smart(mptr->mon_id))
        {
            select_space(&pos, dir_data.delta, AI_DODGER);
        }
        else
        {
            select_space(&pos, dir_data.delta, AI_BANZAI);
        }
        if (pos != mptr->pos)
        {
            /* We decided to move; move! */
            move_mon(mon, pos);
        }
    }
    else if (!mptr->awake)
    {
        return false;
    }
    else
    {
        /* Out of LOS, but awake. Stupid monsters move "drunkenly"; smart
         * monsters (may) seek you out. */
        if (pmon_is_magician(mptr->mon_id))
        {
            /* Magicians may have spells that are used when
             * you are out of sight.  For example, some magicians
             * may teleport themselves to your vicinity. */
            special_used = use_black_magic(mon);
        }
        if (special_used)
        {
            return false;
        }
        if (pmon_is_smart(mptr->mon_id))
        {
            select_space(&pos, dir_data.delta, AI_STALK);
        }
        else if (pmon_is_stupid(mptr->mon_id) || (mptr->ai_lastpos == libmrl::NOWHERE))
        {
            select_space(&pos, dir_data.delta, AI_DRUNK);
        }
        else
        {
            select_space(&pos, dir_data.delta, AI_CHASE);
        }
        if (pos != mptr->pos)
        {
            /* We decided to move; move! */
            move_mon(mon, pos);
        }
    }
    if (mptr->ai_lastpos == mptr->pos)
    {
        if (pmon_is_smart(mptr->mon_id) &&
            (u.pos.distance(mptr->pos) < AI_GUESS_RANGE))
        {
            mptr->ai_lastpos = u.pos;
        }
        else
        {
            mptr->ai_lastpos = mptr->in_fov() ? u.pos : get_mon_scatter(mptr->ai_lastpos);
        }
    }
    return false;
}

void Mon::notice_you(bool quiet)
{
    if (!awake)
    {
        std::string tmp;
        get_name(&tmp, 2);
        if (!quiet)
        {
            print_msg(MSGCHAN_MON_ALERT, "%s notices you.", tmp.c_str());
        }
        awake = true;
    }
}

/* mon2.c */
