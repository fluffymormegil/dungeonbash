// astar.cc - A* pathfinding algorithm for Martin's Dungeon Bash
//
// Copyright 2009 Martin Read
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

#define ASTAR_CC
#include "dunbash.hh"
#include "astar.hh"
#include "monsters.hh"
#include <assert.h>

namespace
{
    int astar_invoc = 1;
}

libmormegil::Offset astar_steps[8] =
{
    { -1, 0 },
    { 1, 0 },
    { 0, -1 },
    { 0, 1 },
    { -1, -1 },
    { 1, 1 },
    { 1, -1 },
    { -1, 1 },
};

void Mon::find_astar_path(libmormegil::Coord goal)
{
    Astar_openset openset;
    Astar_openset_entry current;
    Astar_openset_entry newent;
    Astar_openset::iterator iter;
    Astar_path *path = 0;
    int i;
    int next_g;
    bool successful = false;

    current.pos = pos;
    current.f = current.pos.dist_inf(goal);
    currlev->set_as_invoc(current.pos, astar_invoc);
    currlev->set_as_came_from(current.pos, dunbash::NOWHERE);
    currlev->set_as_gscore(current.pos, 0);
    openset.insert(current);
    while (!(openset.empty()))
    {
        /* get first entry */
        iter = openset.begin();
        current = *iter;
        /* and remove it */
        openset.erase(iter);
        currlev->set_as_considered(current.pos, ASTAR_CLOSED);
        if (current.pos == goal)
        {
            successful = true;
            break;
        }
        else
        {
            next_g = currlev->as_gscore(current.pos) + 1;
            /* Consider its successors */
            for (i = 0; i < COMPASS_POINTS; ++i)
            {
                newent.pos = current.pos + astar_steps[i];
                if (currlev->outofbounds(newent.pos))
                {
                    continue;
                }
                if (currlev->as_invoc(newent.pos) == astar_invoc)
                {
                    // Has been opened. Has it been closed/rejected?
                    switch (currlev->as_considered(newent.pos))
                    {
                    case ASTAR_REJECTED:
                    case ASTAR_CLOSED:
                        /* skip */
                        continue;
                    case ASTAR_OPEN:
                        /* If we've found a lower-g route to an open cell,
                         * update the openset accordingly. */
                        if (next_g < currlev->as_gscore(newent.pos))
                        {
                            newent.f = currlev->as_gscore(newent.pos) + newent.pos.dist_inf(goal);
                            iter = openset.find(newent);
                            if (iter != openset.end())
                            {
                                openset.erase(iter);
                            }
                            newent.f = next_g + newent.pos.dist_inf(goal);
                            currlev->set_as_came_from(newent.pos, current.pos);
                            openset.insert(newent);
                        }
                        else
                        {
                            continue;
                        }
                        break;
                    }
                }
                else
                {
                    currlev->set_as_invoc(newent.pos, astar_invoc);
                    currlev->set_as_considered(newent.pos, ASTAR_UNCONSIDERED);
                }
                switch (currlev->as_considered(newent.pos))
                {
                default:
                case ASTAR_UNCONSIDERED:
                    if (will_pass(newent.pos))
                    {
                        newent.f = newent.pos.dist_inf(goal) + next_g;
                        currlev->set_as_gscore(newent.pos, next_g);
                        currlev->set_as_came_from(newent.pos, current.pos);
                        currlev->set_as_considered(newent.pos, ASTAR_OPEN);
                        openset.insert(newent);
                    }
                    else
                    {
                        currlev->set_as_considered(newent.pos, ASTAR_REJECTED);
                    }
                    break;
                case ASTAR_OPEN:
                    continue;
                case ASTAR_CLOSED:
                    continue;
                case ASTAR_REJECTED:
                    continue;
                }
            }
        }
    }
    if (successful)
    {
        libmormegil::Coord nextstep;
        path = new Astar_path;
        path->push_front(goal);
        for (nextstep = currlev->as_came_from(goal);
             (nextstep != pos) && (nextstep != dunbash::NOWHERE);
             nextstep = currlev->as_came_from(nextstep))
        {
            path->push_front(nextstep);
        }
    }
    current_path = path;
    astar_invoc += 2;
}

void Mon::discard_path()
{
    if (current_path)
    {
        delete current_path;
    }
    current_path = 0;
}

libmormegil::Coord astar_advance(Mon *mptr)
{
    Astar_path::iterator iter1;
    libmormegil::Coord result;
    if (mptr->current_path->empty())
    {
        delete mptr->current_path;
        result = dunbash::NOWHERE;
        mptr->current_path = 0;
    }
    else
    {
        iter1 = mptr->current_path->begin();
        if (!mptr->can_pass(*iter1))
        {
            result = dunbash::NOWHERE;
        }
        else
        {
            result = *iter1;
            mptr->current_path->pop_front();
        }
    }
    return result;
}

// astar.cc
