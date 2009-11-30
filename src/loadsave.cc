/* loadsave.cc
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

#include "dunbash.hh"
#include "objects.hh"
#include "monsters.hh"
#include "combat.hh"
#include "rooms.hh"
#include "vision.hh"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <limits.h>
#include <string>
#include "cfgfile.hh"
#include "loadsave.hh"

//#define DEBUG_LOADSAVE

// A bit of an invariant on the file handling: since dunbash.sav.gz is only
// accessed by gzip, which we assume to be stable, it is considered
// uncorrupted and used in preference to dunbash.sav, which we could have
// crashed in the middle of writing.

bool save_wait;
bool reload_wait;
bool always_fsync;

void serialise(FILE *fp, const Level_tag *ptag);
void serialise(FILE *fp, Levext_rooms const *lerp);
Levext_rooms *deserialise_levext_rooms(FILE *fp);
static void rebuild_mapmons(void);
static void rebuild_mapobjs(void);

static void rebuild_mapmons(void)
{
    std::map<uint64_t, Mon *>::const_iterator iter;
    for (iter = monsters.begin(); iter != monsters.end(); ++iter)
    {
        Level *lptr = iter->second->lev.snapv();
        if (lptr)
        {
            lptr->set_mon_at(iter->second->pos, Mon_handle(iter->first));
            lptr->denizens.insert(Mon_handle(iter->first));
        }
    }
}

static void rebuild_mapobjs(void)
{
    std::map<uint64_t, Obj *>::const_iterator iter;
    for (iter = objects.begin(); iter != objects.end(); ++iter)
    {
        if (iter->second->with_you)
        {
            continue;
        }
        Level *lptr = iter->second->lev.snapv();
        if (lptr)
        {
            lptr->set_obj_at(iter->second->pos, Obj_handle(iter->first));
            lptr->booty.insert(Obj_handle(iter->first));
        }
    }
}

static void deserialise_gamestate(FILE *fp)
{
    uint32_t version_check = deserialise_uint32(fp);
    if ((version_check & 0xffff0000u) != ((MAJVERS << 24) | (MINVERS << 16)))
    {
        print_msg(0, "version mismatch in saved game");
        print_msg(0, "current version is %d.%d, saved game is version %d.%d",
                  MAJVERS, MINVERS, version_check >> 24, (version_check >> 16) & 0xffu);
#ifdef MULTIUSER
        print_msg(0, "Please contact your system's administrator to remove your obsolete saved game file.");
#endif
        press_enter();
        display_shutdown();
        exit(1);
    }
    game_tick = deserialise_uint32(fp);
    next_obj_handle = deserialise_uint64(fp);
    next_mon_handle = deserialise_uint64(fp);
    deserialise(fp, rng_state, 5);
}

static void deserialise(FILE *fp, Player *ptmp)
{
    checked_fread(ptmp->name, 1, 16, fp);
    ptmp->name[16] = '\0';
    deserialise(fp, &(ptmp->lev));
    deserialise(fp, &(ptmp->pos));
    ptmp->sex = Critter_sex(deserialise_uint32(fp));
    ptmp->body = deserialise_uint32(fp);
    ptmp->bdam = deserialise_uint32(fp);
    ptmp->agility = deserialise_uint32(fp);
    ptmp->adam = deserialise_uint32(fp);
    ptmp->hpmax = deserialise_uint32(fp);
    ptmp->hpcur = deserialise_uint32(fp);
    ptmp->food = deserialise_uint32(fp);
    ptmp->experience = deserialise_uint32(fp);
    ptmp->speed = deserialise_uint32(fp);
    deserialise(fp, ptmp->resistances, DT_COUNT);
    ptmp->level = deserialise_uint32(fp);
    ptmp->gold = deserialise_uint32(fp);
    deserialise_ohandle_array(fp, ptmp->inventory, INVENTORY_SIZE);
    // The following are stored as inventory offsets
    ptmp->weapon = deserialise_uint32(fp);
    ptmp->armour = deserialise_uint32(fp);
    ptmp->ring = deserialise_uint32(fp);
    uint32_t flav = deserialise_uint32(fp);
    while (flav != 0xffffffffu)
    {
        Perseff_data peff;
        uint64_t t64;
        peff.flavour = Persistent_effect(flav);
        peff.power = deserialise_uint32(fp);
        peff.duration = deserialise_uint32(fp);
        peff.by_you = deserialise_uint32(fp);
        peff.on_you = deserialise_uint32(fp);
        t64 = deserialise_uint64(fp);
        peff.caster = Mon_handle(t64);
        t64 = deserialise_uint64(fp);
        peff.victim = Mon_handle(t64);
        u.perseffs.push_back(peff);
        u.status.set_flag(peff.flavour);
        flav = deserialise_uint32(fp);
    }
}

void deserialise(FILE *fp, libmrl::Coord *c)
{
    c->y = int(deserialise_uint32(fp));
    c->x = int(deserialise_uint32(fp));
}

void deserialise(FILE *fp, Level_tag *ptag)
{
    ptag->dungeon = Dungeon_num(deserialise_uint32(fp));
    ptag->level = deserialise_uint32(fp);
}

void deserialise_objects(FILE *fp)
{
    uint32_t count;
    uint32_t i;
    uint64_t oref;
    count = deserialise_uint32(fp);
    for (i = 0; i < count; ++i)
    {
        oref = deserialise_uint64(fp);
        Obj *optr = new Obj;
        objects[oref] = optr;
        deserialise(fp, optr);
        objects[oref]->self = Obj_handle(oref);
    }
}

void deserialise(FILE *fp, Obj *optr)
{
    optr->obj_id = int(deserialise_uint32(fp));
    optr->quan = int(deserialise_uint32(fp));
    optr->with_you = bool(deserialise_uint32(fp));
    deserialise(fp, &(optr->lev));
    deserialise(fp, &(optr->pos));
    optr->durability = int(deserialise_uint32(fp));
    optr->meta = int(deserialise_uint32(fp));
#ifdef DEBUG_LOADSAVE
    print_msg(0, "   deserialised object of type %d pos %d %d levtag %d %d withyou %d", optr->obj_id, optr->pos.y, optr->pos.x, optr->lev.dungeon, optr->lev.level, optr->with_you);
#endif
}

void deserialise_ohandle_array(FILE *fp, Obj_handle *array, int count)
{
    int i;
    for (i = 0; i < count; ++i)
    {
        array[i] = Obj_handle(deserialise_uint64(fp));
    }
}

void deserialise_permobj_vars(FILE *fp)
{
    int i;
    for (i = 0; i < PO_COUNT; ++i)
    {
        permobjs[i].known = bool(deserialise_uint32(fp));
        permobjs[i].power = int(deserialise_uint32(fp));
    }
}

void deserialise_monsters(FILE *fp)
{
    uint32_t count = deserialise_uint32(fp);
    uint32_t i;
    uint64_t mref;
    Mon *mptr;
    for (i = 0; i < count; ++i)
    {
        mptr = new Mon;
        mref = deserialise_uint64(fp);
        monsters[mref] = mptr;
        deserialise(fp, mptr);
        monsters[mref]->self = Mon_handle(mref);
    }
}

void deserialise(FILE *fp, Mon *mptr)
{
    libmrl::Coord c;
    int k;
    mptr->mon_id = deserialise_uint32(fp);
    deserialise(fp, &(mptr->lev));
    deserialise(fp, &(mptr->pos));
    deserialise(fp, &(mptr->ai_lastpos));
    mptr->hpmax = deserialise_uint32(fp);
    mptr->hpcur = deserialise_uint32(fp);
    mptr->mtohit = deserialise_uint32(fp);
    mptr->rtohit = deserialise_uint32(fp);
    mptr->defence = deserialise_uint32(fp);
    mptr->mdam = deserialise_uint32(fp);
    mptr->rdam = deserialise_uint32(fp);
    mptr->awake = deserialise_uint32(fp);
    mptr->meta = deserialise_uint32(fp);
    mptr->sex = Critter_sex(deserialise_uint32(fp));
    k = deserialise_uint32(fp);
    // check for reasonable length of name
    if ((k > 0) && (k < 256))
    {
        mptr->name = (char *)malloc(k);
        checked_fread(mptr->name, 1, k, fp);
    }
    else
    {
        mptr->name = 0;
    }
    deserialise(fp, &c);
    if (c != libmrl::NOWHERE)
    {
        mptr->current_path = new Astar_path();
        do
        {
            mptr->current_path->push_back(c);
            deserialise(fp, &c);
        } while (c != libmrl::NOWHERE);
    }
    else
    {
        mptr->current_path = 0;
    }
    uint32_t flav = deserialise_uint32(fp);
    while (flav != 0xffffffffu)
    {
        Perseff_data peff;
        uint64_t t64;
        peff.flavour = Persistent_effect(flav);
        peff.power = deserialise_uint32(fp);
        peff.duration = deserialise_uint32(fp);
        peff.by_you = deserialise_uint32(fp);
        peff.on_you = deserialise_uint32(fp);
        t64 = deserialise_uint64(fp);
        peff.caster = Mon_handle(t64);
        t64 = deserialise_uint64(fp);
        peff.victim = Mon_handle(t64);
        mptr->perseffs.push_back(peff);
        flav = deserialise_uint32(fp);
    }
}

void deserialise_levels(FILE *fp)
{
    Level_tag lt;
    int i = 0;
    deserialise(fp, &lt);
    while (lt != no_level)
    {
        Level * lptr;
        lptr = deserialise_level(fp, lt);
        deserialise(fp, &lt);
        ++i;
    }
}

Level * deserialise_level(FILE *fp, Level_tag lt)
{
    int i;
    Level *lp;
    uint32_t ht;
    uint32_t wd;
    uint32_t tmp;
    Leventry_mode lem;
    libmrl::Coord pos;
    Level_tag dest;
    ht = deserialise_uint32(fp);
    wd = deserialise_uint32(fp);
    if ((ht > MAX_DUN_HEIGHT) ||
        (wd > MAX_DUN_WIDTH))
    {
        throw(EINVAL);
    }
    lp = new Level(ht, wd);
    levels[lt] = lp;
    lp->self = lt;
    lp->levtype = deserialise_uint32(fp);
    deserialise(fp, &pos);
    while (pos != libmrl::NOWHERE)
    {
        lem = Leventry_mode(deserialise_uint32(fp));
        lp->exit_modes[pos] = lem;
        deserialise(fp, &pos);
    }
    tmp = deserialise_uint32(fp);
    while (tmp != 0xffffffffu)
    {
        lem = Leventry_mode(tmp);
        deserialise(fp, &dest);
        lp->exit_dests[lem] = dest;
        tmp = deserialise_uint32(fp);
    }
    tmp = deserialise_uint32(fp);
    while (tmp != 0xffffffffu)
    {
        lem = Leventry_mode(tmp);
        deserialise(fp, &pos);
        lp->entries[lem] = pos;
        tmp = deserialise_uint32(fp);
    }
    for (i = 0; i < lp->height; ++i)
    {
        deserialise(fp, lp->mflags[i], lp->width);
    }
    // save terrain
    for (i = 0; i < lp->height; ++i)
    {
        deserialise(fp, (uint32_t *) lp->terrain[i], lp->width);
    }
    // save regionnums
    for (i = 0; i < lp->height; ++i)
    {
        deserialise(fp, (uint32_t *) lp->rnums[i], lp->width);
    }
    switch (lp->levtype)
    {
    case LEVEL_ROOMS:
        lp->levextra = deserialise_levext_rooms(fp);
        lp->levextra->parent = lp;
        break;
    }
    return lp;
}

Levext_rooms *deserialise_levext_rooms(FILE *fp)
{
#ifdef DEBUG_LOADSAVE
    print_msg(0, "deserialising Levext_rooms");
#endif
    Levext_rooms *lerp = new Levext_rooms;
    lerp->overridden_monsel = deserialise_uint32(fp);
    lerp->actual_rooms = deserialise_uint32(fp);
    lerp->zoo_room = deserialise_uint32(fp);
    lerp->zoo_style = Levext_rooms::Zoo_style(deserialise_uint32(fp));
    lerp->dstairs_room = deserialise_uint32(fp);
    lerp->ustairs_room = deserialise_uint32(fp);
    deserialise(fp, &(lerp->dstairs_pos));
    deserialise(fp, &(lerp->ustairs_pos));
    int i;
    for (i = 0; i < Levext_rooms::MAX_ROOMS; ++i)
    {
        deserialise(fp, &(lerp->bounds[i][0]));
        deserialise(fp, &(lerp->bounds[i][1]));
        lerp->segsused[i] = deserialise_uint32(fp);
        lerp->roomflav[i] = Room_flavour(deserialise_uint32(fp));
    }
    int j;
    for (i = 0; i < Levext_rooms::MAX_ROOMS; ++i)
    {
        for (j = 0; j < Levext_rooms::MAX_ROOMS; ++j)
        {
            lerp->linkage[i][j] = deserialise_uint32(fp);
        }
    }
    return lerp;
}

static void serialise(FILE *fp, Player const *ptmp)
{
#ifdef DEBUG_LOADSAVE
    print_msg(0, "serialising player");
#endif
    fwrite(ptmp->name, 1, 16, fp);
    serialise(fp, uint32_t(ptmp->lev.dungeon));
    serialise(fp, uint32_t(ptmp->lev.level));
    serialise(fp, uint32_t(ptmp->pos.y));
    serialise(fp, uint32_t(ptmp->pos.x));
    serialise(fp, uint32_t(ptmp->sex));
    serialise(fp, uint32_t(ptmp->body));
    serialise(fp, uint32_t(ptmp->bdam));
    serialise(fp, uint32_t(ptmp->agility));
    serialise(fp, uint32_t(ptmp->adam));
    serialise(fp, uint32_t(ptmp->hpmax));
    serialise(fp, uint32_t(ptmp->hpcur));
    serialise(fp, uint32_t(ptmp->food));
    serialise(fp, ptmp->experience);
    serialise(fp, uint32_t(ptmp->speed));
    serialise(fp, ptmp->resistances, DT_COUNT);
    serialise(fp, uint32_t(ptmp->level));
    serialise(fp, uint32_t(ptmp->gold));
    serialise_ohandle_array(fp, ptmp->inventory, INVENTORY_SIZE);
    serialise(fp, uint32_t(ptmp->weapon.value));
    serialise(fp, uint32_t(ptmp->armour.value));
    serialise(fp, uint32_t(ptmp->ring.value));
    std::list<Perseff_data>::const_iterator peffiter;
    for (peffiter = u.perseffs.begin(); peffiter != u.perseffs.end(); ++peffiter)
    {
        serialise(fp, uint32_t(peffiter->flavour));
        serialise(fp, uint32_t(peffiter->power));
        serialise(fp, uint32_t(peffiter->duration));
        serialise(fp, uint32_t(peffiter->by_you));
        serialise(fp, uint32_t(peffiter->on_you));
        serialise(fp, peffiter->caster.value);
        serialise(fp, peffiter->victim.value);
    }
    serialise(fp, 0xffffffffu);
}

void serialise(FILE *fp, const Level_tag *ptag)
{
    serialise(fp, uint32_t(ptag->dungeon));
    serialise(fp, uint32_t(ptag->level));
}

void serialise(FILE *fp, Obj const *optr)
{
#ifdef DEBUG_LOADSAVE
    print_msg(0, "   serialised object of type %d pos %d %d levtag %d %d withyou %d", optr->obj_id, optr->pos.y, optr->pos.x, optr->lev.dungeon, optr->lev.level, optr->with_you);
#endif
    serialise(fp, uint32_t(optr->obj_id));
    serialise(fp, uint32_t(optr->quan));
    serialise(fp, uint32_t(optr->with_you));
    serialise(fp, &(optr->lev));
    serialise(fp, optr->pos);
    serialise(fp, uint32_t(optr->durability));
    serialise(fp, uint32_t(optr->meta));
}

void serialise_objects(FILE *fp)
{
    int count;
    std::map<uint64_t, Obj *>::iterator iter;
    count = objects.size();
    serialise(fp, uint32_t(count));
    for (iter = objects.begin(); iter != objects.end(); ++iter)
    {
#ifdef DEBUG_LOADSAVE
        print_msg(0, "serialising object ID %llx", iter->first);
#endif
        serialise(fp, iter->first);
        serialise(fp, iter->second);
    }
}

void serialise_ohandle_array(FILE *fp, Obj_handle const *array, int count)
{
    int i;
    for (i = 0; i < count; ++i)
    {
        serialise(fp, array[i].value);
    }
}

static void serialise_gamestate(FILE *fp)
{
    serialise(fp, uint32_t((MAJVERS << 24) | (MINVERS << 16) | (PATCHVERS << 8)));
    serialise(fp, game_tick);
    serialise(fp, next_obj_handle);
    serialise(fp, next_mon_handle);
    serialise(fp, rng_state, 5);
}

void serialise(FILE *fp, libmrl::Coord c)
{
    serialise(fp, uint32_t(c.y));
    serialise(fp, uint32_t(c.x));
}

void serialise_levels(FILE *fp)
{
    std::map<Level_tag, Level *>::iterator iter;
    int i = 0;
    for (iter = levels.begin(); iter != levels.end(); ++iter)
    {
        serialise(fp, &(iter->first));
        ++i;
        serialise(fp, iter->second);
    }
    serialise(fp, &no_level);
}

void serialise(FILE *fp, Level const *lp)
{
    int i;
    // parameters
    serialise(fp, uint32_t(lp->height));
    serialise(fp, uint32_t(lp->width));
    serialise(fp, uint32_t(lp->levtype));
    // connectivity
    Exitmode_citer exciter;
    for (exciter = lp->exit_modes.begin();
         exciter != lp->exit_modes.end();
         ++exciter)
    {
        serialise(fp, exciter->first);
        serialise(fp, uint32_t(exciter->second));
    }
    serialise(fp, libmrl::NOWHERE);
    Exitdest_citer edciter;
    for (edciter = lp->exit_dests.begin();
         edciter != lp->exit_dests.end();
         ++edciter)
    {
        serialise(fp, uint32_t(edciter->first));
        serialise(fp, &(edciter->second));
    }
    serialise(fp, 0xffffffffu);
    Entry_citer enciter;
    for (enciter = lp->entries.begin();
         enciter != lp->entries.end();
         ++enciter)
    {
        serialise(fp, uint32_t(enciter->first));
        serialise(fp, enciter->second);
    }
    serialise(fp, 0xffffffffu);
    // skip mobjs
    // skip mmons
    // skip astar parameters
    // save flags
    for (i = 0; i < lp->height; ++i)
    {
        serialise(fp, lp->mflags[i], lp->width);
    }
    // save terrain
    for (i = 0; i < lp->height; ++i)
    {
        serialise(fp, (uint32_t *) lp->terrain[i], lp->width);
    }
    // save regionnums
    for (i = 0; i < lp->height; ++i)
    {
        serialise(fp, (uint32_t *) lp->rnums[i], lp->width);
    }
    switch (lp->levtype)
    {
    case LEVEL_ROOMS:
        Levext_rooms const *lerp = dynamic_cast<Levext_rooms const *>(lp->levextra);
        /* I could check for errors, or I could have the program segfault its
         * guts out. The latter is probably saner. */
        serialise(fp, lerp);
        break;
    }
}

void serialise(FILE *fp, Levext_rooms const *lerp)
{
#ifdef DEBUG_LOADSAVE
    print_msg(0, "serialising Levext_rooms");
#endif
    serialise(fp, uint32_t(lerp->overridden_monsel));
    serialise(fp, uint32_t(lerp->actual_rooms));
    serialise(fp, uint32_t(lerp->zoo_room));
    serialise(fp, uint32_t(lerp->zoo_style));
    serialise(fp, uint32_t(lerp->dstairs_room));
    serialise(fp, uint32_t(lerp->ustairs_room));
    serialise(fp, lerp->ustairs_pos);
    serialise(fp, lerp->dstairs_pos);
    int i;
    for (i = 0; i < Levext_rooms::MAX_ROOMS; ++i)
    {
        serialise(fp, lerp->bounds[i][0]);
        serialise(fp, lerp->bounds[i][1]);
        serialise(fp, uint32_t(lerp->segsused[i]));
        serialise(fp, uint32_t(lerp->roomflav[i]));
    }
    int j;
    for (i = 0; i < Levext_rooms::MAX_ROOMS; ++i)
    {
        for (j = 0; j < Levext_rooms::MAX_ROOMS; ++j)
        {
            serialise(fp, uint32_t(lerp->linkage[i][j]));
        }
    }
}

void serialise(FILE *fp, Mon const *mptr)
{
    Astar_path::iterator iter;
    serialise(fp, uint32_t(mptr->mon_id));
    serialise(fp, &(mptr->lev));
    serialise(fp, mptr->pos);
    serialise(fp, mptr->ai_lastpos);
    serialise(fp, uint32_t(mptr->hpmax));
    serialise(fp, uint32_t(mptr->hpcur));
    serialise(fp, uint32_t(mptr->mtohit));
    serialise(fp, uint32_t(mptr->rtohit));
    serialise(fp, uint32_t(mptr->defence));
    serialise(fp, uint32_t(mptr->mdam));
    serialise(fp, uint32_t(mptr->rdam));
    serialise(fp, uint32_t(mptr->awake));
    serialise(fp, uint32_t(mptr->meta));
    serialise(fp, uint32_t(mptr->sex));
    if (mptr->name)
    {
        int k = strlen(mptr->name) + 1;
        serialise(fp, uint32_t(k));
        fwrite(mptr->name, k, 1, fp);
    }
    else
    {
        serialise(fp, uint32_t(0));
    }
    if (mptr->current_path)
    {
        for (iter = mptr->current_path->begin();
             iter != mptr->current_path->end();
             ++iter)
        {
            serialise(fp, *iter);
        }
    }
    serialise(fp, libmrl::NOWHERE);
    std::list<Perseff_data>::const_iterator peffiter;
    for (peffiter = mptr->perseffs.begin(); peffiter != mptr->perseffs.end(); ++peffiter)
    {
        serialise(fp, uint32_t(peffiter->flavour));
        serialise(fp, uint32_t(peffiter->power));
        serialise(fp, uint32_t(peffiter->duration));
        serialise(fp, uint32_t(peffiter->by_you));
        serialise(fp, uint32_t(peffiter->on_you));
        serialise(fp, peffiter->caster.value);
        serialise(fp, peffiter->victim.value);
    }
    serialise(fp, 0xffffffffu);
}

void serialise_monsters(FILE *fp)
{
    int count = monsters.size();
    std::map<uint64_t, Mon *>::const_iterator iter;
    serialise(fp, uint32_t(count));
    for (iter = monsters.begin(); iter != monsters.end(); ++iter)
    {
        serialise(fp, iter->first);
        serialise(fp, iter->second);
    }
}

void serialise_permobj_vars(FILE *fp)
{
    int i;
    for (i = 0; i < PO_COUNT; ++i)
    {
        serialise(fp, uint32_t(permobjs[i].known));
        serialise(fp, uint32_t(permobjs[i].power));
    }
}

int save_game(void)
{
    FILE *fp;
    int fd;
    int i;
    std::string filename;
#ifdef MULTIUSER
    char uidbuf[16];
    filename = configured_system_playground;
    filename += "/save/dunbash-";
    sprintf(uidbuf, "%x", user_uid);
    filename += uidbuf;
    filename += ".sav";
    game_permissions();
#else
    filename = "dunbash.sav";
#endif
    fd = open(filename.c_str(), O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1)
    {
        print_msg(MSGCHAN_INTERROR, "could not save to %s: %s", filename.c_str(),
                  strerror(errno));
        press_enter();
        return -1;
    }
    fp = fdopen(fd, "wb");
    serialise_gamestate(fp);
    serialise(fp, &u);
    serialise_levels(fp);
    serialise_monsters(fp);
    serialise_objects(fp);
    serialise_permobj_vars(fp);
    fflush(fp);
    if (always_fsync)
    {
        fsync(fileno(fp));
    }
    fclose(fp);
    /* Compress! */
    std::string command;
    command = COMPRESSOR;
    command += " ";
    command += filename;
    i = system(command.c_str());
    if (i != 0)
    {
        print_msg(MSGCHAN_INTERROR, "could not compress save file");
        press_enter();
        return -1;
    }
    return 0;
}

int load_game(void)
{
    FILE *fp;
    struct stat st;
    std::string command;
    std::string filename;
    std::string compressed_filename;
    int i;
#ifdef MULTIUSER
    char uidbuf[16];
    filename = configured_system_playground;
    filename += "/save/dunbash-";
    sprintf(uidbuf, "%x", getuid());
    filename += uidbuf;
    game_permissions();
#else
    filename = "dunbash";
#endif
    filename += ".sav";
    compressed_filename = filename;
    compressed_filename += COMPRESSED_SUFFIX;
    i = stat(compressed_filename.c_str(), &st);
    if (i != -1)
    {
        command = DECOMPRESSOR;
        command += " ";
        command += compressed_filename;
        i = system(command.c_str());
        if (i != 0)
        {
            print_msg(MSGCHAN_INTERROR, "compressed save file found but unable to decompress. Giving up...");
            press_enter();
            display_shutdown();
            exit(1);
        }
    }
    i = stat(filename.c_str(), &st);
    if (i != -1)
    {
        fp = fopen(filename.c_str(), "rb");
        if (!fp)
        {
            print_msg(MSGCHAN_INTERROR, "could not open uncompressed save file. Giving up...");
            press_enter();
            display_shutdown();
            exit(1);
        }
        try
        {
            print_msg(0, "Loading saved game...");
            deserialise_gamestate(fp);
            deserialise(fp, &u);
            deserialise_levels(fp);
            currlev = u.lev.snapv();
            if (!currlev)
            {
                print_msg(0, "fatal error: invalid u.lev");
                sleep(1);
                abort();
            }
            deserialise_monsters(fp);
            deserialise_objects(fp);
            deserialise_permobj_vars(fp);
        }
        catch (...)
        {
            print_msg(MSGCHAN_INTERROR, "read error parsing save file.");
            press_enter();
            display_shutdown();
            exit(1);
        }
        fclose(fp);
        rebuild_mapmons();
        rebuild_mapobjs();
        touch_back_buffer();
        do_vision();
        status_updated = 1;
        map_updated = 1;
        hard_redraw = 1;
        recalc_defence();
        print_msg(0, "Game loaded.");
        if (reload_wait)
        {
            press_enter();
        }
        look_at_floor();
        return 0;
    }
    return -1;
}

void kill_game(void)
{
    std::string filename;
#ifdef MULTIUSER
    char uidbuf[16];
    filename = configured_system_playground;
    filename += "/save/dunbash-";
    sprintf(uidbuf, "%x", getuid());
    filename += uidbuf;
    game_permissions();
#else
    filename = "dunbash";
#endif
    filename += ".sav";

    if (!wizard_mode)
    {
        unlink(filename.c_str());
        filename += COMPRESSED_SUFFIX;
        unlink(filename.c_str());
    }
}

/* loadsave.c */
