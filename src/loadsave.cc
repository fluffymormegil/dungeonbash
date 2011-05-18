/* loadsave.cc
 * 
 * Copyright 2009,2010 Martin Read
 * Copyright 2009 Stefan O'Rear
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
#include "bossrooms.hh"
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

void serialize(FILE *fp, const Level_tag *ptag);
void serialize(FILE *fp, Levext_rooms const *lerp);
void serialize(FILE *fp, Levext_rooms_boss const *lerp);
Levext_rooms *deserialize_levext_rooms(FILE *fp);
Levext_rooms_boss *deserialize_levext_rooms_boss(FILE *fp);
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

static void deserialize_gamestate(FILE *fp)
{
    uint32_t version_check;
    deserialize(fp, &version_check);
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
    deserialize(fp, &game_tick);
    deserialize(fp, &next_obj_handle);
    deserialize(fp, &next_mon_handle);
    deserialize(fp, rng_state, 5);
}

static void deserialize(FILE *fp, Player *ptmp)
{
    uint32_t tmp;
    checked_fread(ptmp->name, 1, 16, fp);
    ptmp->name[16] = '\0';
    deserialize(fp, &(ptmp->lev));
    deserialize(fp, &(ptmp->pos));
    deserialize(fp, &tmp); ptmp->sex = Critter_sex(tmp);
    deserialize(fp, &tmp); ptmp->job = Player_profession(tmp);
    deserialize(fp, &ptmp->body);
    deserialize(fp, &ptmp->bdam);
    deserialize(fp, &ptmp->agility);
    deserialize(fp, &ptmp->adam);
    deserialize(fp, &ptmp->hpmax);
    deserialize(fp, &ptmp->hpcur);
    deserialize(fp, &ptmp->mpmax);
    deserialize(fp, &ptmp->mpcur);
    deserialize(fp, &ptmp->food);
    deserialize(fp, &ptmp->experience);
    deserialize(fp, &ptmp->speed);
    deserialize(fp, ptmp->resistances, DT_COUNT);
    deserialize(fp, ptmp->cooldowns, 10);
    deserialize(fp, &ptmp->combat_timer);
    deserialize(fp, &ptmp->next_cloud_tick);
    deserialize(fp, &ptmp->level);
    deserialize(fp, &ptmp->gold);
    deserialize_ohandle_array(fp, ptmp->inventory, INVENTORY_SIZE);
    // The following are stored as inventory offsets
    deserialize_ohandle_array(fp, &(ptmp->weapon), 1);
    deserialize_ohandle_array(fp, &(ptmp->armour), 1);
    deserialize_ohandle_array(fp, &(ptmp->ring), 1);
    uint32_t flav;
    deserialize(fp, &flav);
    while (flav != 0xffffffffu)
    {
        Perseff_data peff;
        uint64_t t64;
        uint64_t t32;
        peff.flavour = Persistent_effect(flav);
        deserialize(fp, &peff.power);
        deserialize(fp, &peff.duration);
        deserialize(fp, &t32); peff.by_you = t32;
        deserialize(fp, &t32); peff.on_you = t32;
        deserialize(fp, &t64); peff.caster = Mon_handle(t64);
        deserialize(fp, &t64); peff.victim = Mon_handle(t64);
        u.perseffs.push_back(peff);
        u.status.set_flag(peff.flavour);
        deserialize(fp, &flav);
    }
}

void deserialize(FILE *fp, Level_tag *ptag)
{
    uint32_t tmp;
    deserialize(fp, &tmp); ptag->dungeon = Dungeon_num(tmp);
    deserialize(fp, &ptag->level);
}

void deserialize_objects(FILE *fp)
{
    uint32_t count;
    uint32_t i;
    uint64_t oref;
    deserialize(fp, &count);
    for (i = 0; i < count; ++i)
    {
        deserialize(fp, &oref);
        Obj *optr = new Obj;
        objects[oref] = optr;
        deserialize(fp, optr);
        objects[oref]->self = Obj_handle(oref);
    }
}

void deserialize(FILE *fp, Obj *optr)
{
    deserialize(fp, &optr->obj_id);
    deserialize(fp, &optr->quan);
    deserialize(fp, &optr->with_you);
    deserialize(fp, &optr->lev);
    deserialize(fp, &optr->pos);
    deserialize(fp, &optr->durability);
    deserialize(fp, optr->meta, 2);
#ifdef DEBUG_LOADSAVE
    print_msg(0, "   deserialized object of type %d pos %d %d levtag %d %d withyou %d", optr->obj_id, optr->pos.y, optr->pos.x, optr->lev.dungeon, optr->lev.level, optr->with_you);
#endif
}

void deserialize_ohandle_array(FILE *fp, Obj_handle *array, int count)
{
    int i;
    uint64_t t64;
    for (i = 0; i < count; ++i)
    {
        deserialize(fp, &t64);
        array[i] = Obj_handle(t64);
    }
}

void deserialize_permobj_vars(FILE *fp)
{
    int i;
    for (i = 0; i < PO_COUNT; ++i)
    {
        deserialize(fp, &permobjs[i].known);
        deserialize(fp, &permobjs[i].power);
    }
}

void deserialize_monsters(FILE *fp)
{
    uint32_t count;
    uint32_t i;
    uint64_t mref;
    Mon *mptr;
    deserialize(fp, &count);
    for (i = 0; i < count; ++i)
    {
        mptr = new Mon;
        deserialize(fp, &mref);
        monsters[mref] = mptr;
        deserialize(fp, mptr);
        monsters[mref]->self = Mon_handle(mref);
    }
}

void deserialize(FILE *fp, Mon *mptr)
{
    libmormegil::Coord c;
    uint32_t k;
    uint32_t t32;
    deserialize(fp, &mptr->mon_id);
    deserialize(fp, &mptr->lev);
    deserialize(fp, &mptr->pos);
    deserialize(fp, &mptr->ai_lastpos);
    deserialize(fp, &mptr->hpmax);
    deserialize(fp, &mptr->hpcur);
    deserialize(fp, &mptr->mtohit);
    deserialize(fp, &mptr->rtohit);
    deserialize(fp, &mptr->defence);
    deserialize(fp, &mptr->mdam);
    deserialize(fp, &mptr->rdam);
    deserialize(fp, &mptr->awake);
    deserialize(fp, &mptr->meta);
    deserialize(fp, &mptr->no_exp);
    deserialize(fp, &t32); mptr->sex = Critter_sex(t32);
    deserialize(fp, &k);
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
    deserialize(fp, &c);
    if (c != dunbash::NOWHERE)
    {
        mptr->current_path = new Astar_path();
        do
        {
            mptr->current_path->push_back(c);
            deserialize(fp, &c);
        } while (c != dunbash::NOWHERE);
    }
    else
    {
        mptr->current_path = 0;
    }
    uint32_t flav;
    deserialize(fp, &flav);
    while (flav != 0xffffffffu)
    {
        Perseff_data peff;
        uint64_t t64;
        peff.flavour = Persistent_effect(flav);
        deserialize(fp, &peff.power);
        deserialize(fp, &peff.duration);
        deserialize(fp, &peff.by_you);
        deserialize(fp, &peff.on_you);
        deserialize(fp, &t64);
        peff.caster = Mon_handle(t64);
        deserialize(fp, &t64);
        peff.victim = Mon_handle(t64);
        mptr->perseffs.push_back(peff);
        deserialize(fp, &flav);
    }
}

void deserialize_levels(FILE *fp)
{
    Level_tag lt;
    int i = 0;
    deserialize(fp, &lt);
    while (lt != no_level)
    {
        Level * lptr;
        lptr = deserialize_level(fp, lt);
        deserialize(fp, &lt);
        ++i;
    }
}

Level * deserialize_level(FILE *fp, Level_tag lt)
{
    int i;
    Level *lp;
    uint32_t ht;
    uint32_t wd;
    uint32_t tmp;
    Leventry_mode lem;
    libmormegil::Coord pos;
    Level_tag dest;
    deserialize(fp, &ht);
    deserialize(fp, &wd);
    if ((ht > MAX_DUN_HEIGHT) ||
        (wd > MAX_DUN_WIDTH))
    {
        throw(EINVAL);
    }
    lp = new Level(lt, ht, wd);
    levels[lt] = lp;
    lp->self = lt;
    deserialize(fp, &tmp);
    lp->levtype = tmp;
    deserialize(fp, &pos);
    while (pos != dunbash::NOWHERE)
    {
        deserialize(fp, &tmp); lem = Leventry_mode(tmp);
        lp->exit_modes[pos] = lem;
        deserialize(fp, &pos);
    }
    deserialize(fp, &tmp);
    while (tmp != 0xffffffffu)
    {
        lem = Leventry_mode(tmp);
        deserialize(fp, &dest);
        lp->exit_dests[lem] = dest;
        deserialize(fp, &tmp);
    }
    deserialize(fp, &tmp);
    while (tmp != 0xffffffffu)
    {
        lem = Leventry_mode(tmp);
        deserialize(fp, &pos);
        lp->entries[lem] = pos;
        deserialize(fp, &tmp);
    }
    for (i = 0; i < lp->height; ++i)
    {
        deserialize(fp, lp->mflags[i], lp->width);
    }
    // save terrain
    for (i = 0; i < lp->height; ++i)
    {
        deserialize(fp, (uint32_t *) lp->terrain[i], lp->width);
    }
    // save regionnums
    for (i = 0; i < lp->height; ++i)
    {
        deserialize(fp, (uint32_t *) lp->rnums[i], lp->width);
    }
    switch (lp->levtype)
    {
    case LEVEL_ROOMS:
        lp->levextra = deserialize_levext_rooms(fp);
        lp->levextra->parent = lp;
        break;
    case LEVEL_ROOMS_BOSS:
        lp->levextra = deserialize_levext_rooms_boss(fp);
        lp->levextra->parent = lp;
        break;
    }
    return lp;
}

Levext_rooms_boss *deserialize_levext_rooms_boss(FILE *fp)
{
#ifdef DEBUG_LOADSAVE
    print_msg(0, "deserialising Levext_rooms");
#endif
    Levext_rooms_boss *lerp = new Levext_rooms_boss;
    uint32_t t32;
    deserialize(fp, &lerp->overridden_monsel);
    deserialize(fp, &lerp->actual_rooms);
    deserialize(fp, &lerp->zoo_room);
    deserialize(fp, &t32);
    lerp->zoo_style = Levext_rooms::Zoo_style(t32);
    deserialize(fp, &lerp->dstairs_room);
    deserialize(fp, &lerp->ustairs_room);
    deserialize(fp, &lerp->dstairs_pos);
    deserialize(fp, &lerp->ustairs_pos);
    int i;
    for (i = 0; i < Levext_rooms::MAX_ROOMS; ++i)
    {
        deserialize(fp, &(lerp->bounds[i][0]));
        deserialize(fp, &(lerp->bounds[i][1]));
        deserialize(fp, lerp->segsused + i);
        deserialize(fp, &t32);
        lerp->roomflav[i] = Room_flavour(t32);
    }
    int j;
    for (i = 0; i < Levext_rooms::MAX_ROOMS; ++i)
    {
        for (j = 0; j < Levext_rooms::MAX_ROOMS; ++j)
        {
            deserialize(fp, &(lerp->linkage[i][j]));
        }
    }
    deserialize(fp, &t32); lerp->spec.x_terrain = Terrain_num(t32);
    deserialize(fp, &t32); lerp->spec.hash_terrain = Terrain_num(t32);
    deserialize(fp, &t32); lerp->spec.dot_terrain = Terrain_num(t32);
    deserialize(fp, &t32); lerp->spec.tilde_terrain = Terrain_num(t32);
    deserialize(fp, lerp->spec.num_pmons, 10);
    deserialize(fp, lerp->spec.num_pobjs, 10);
    for (i = 0; i < 10; ++i)
    {
        deserialize(fp, &t32); lerp->spec.num_terrs[i] = Terrain_num(t32);
    }
    return lerp;
}

Levext_rooms *deserialize_levext_rooms(FILE *fp)
{
#ifdef DEBUG_LOADSAVE
    print_msg(0, "deserialising Levext_rooms");
#endif
    Levext_rooms *lerp = new Levext_rooms;
    uint32_t t32;
    deserialize(fp, &lerp->overridden_monsel);
    deserialize(fp, &lerp->actual_rooms);
    deserialize(fp, &lerp->zoo_room);
    deserialize(fp, &t32); lerp->zoo_style = Levext_rooms::Zoo_style(t32);
    deserialize(fp, &lerp->dstairs_room);
    deserialize(fp, &lerp->ustairs_room);
    deserialize(fp, &lerp->dstairs_pos);
    deserialize(fp, &lerp->ustairs_pos);
    int i;
    for (i = 0; i < Levext_rooms::MAX_ROOMS; ++i)
    {
        deserialize(fp, &(lerp->bounds[i][0]));
        deserialize(fp, &(lerp->bounds[i][1]));
        deserialize(fp, lerp->segsused + i);
        deserialize(fp, &t32); lerp->roomflav[i] = Room_flavour(t32);
    }
    int j;
    for (i = 0; i < Levext_rooms::MAX_ROOMS; ++i)
    {
        for (j = 0; j < Levext_rooms::MAX_ROOMS; ++j)
        {
            deserialize(fp, &(lerp->linkage[i][j]));
        }
    }
    return lerp;
}

static void serialize(FILE *fp, Player const *ptmp)
{
#ifdef DEBUG_LOADSAVE
    print_msg(0, "serialising player");
#endif
    fwrite(ptmp->name, 1, 16, fp);
    serialize(fp, ptmp->lev.dungeon);
    serialize(fp, ptmp->lev.level);
    serialize(fp, ptmp->pos);
    serialize(fp, uint32_t(ptmp->sex));
    serialize(fp, uint32_t(ptmp->job));
    serialize(fp, ptmp->body);
    serialize(fp, ptmp->bdam);
    serialize(fp, ptmp->agility);
    serialize(fp, ptmp->adam);
    serialize(fp, ptmp->hpmax);
    serialize(fp, ptmp->hpcur);
    serialize(fp, ptmp->mpmax);
    serialize(fp, ptmp->mpcur);
    serialize(fp, ptmp->food);
    serialize(fp, ptmp->experience);
    serialize(fp, ptmp->speed);
    serialize(fp, ptmp->resistances, DT_COUNT);
    serialize(fp, ptmp->cooldowns, 10);
    serialize(fp, ptmp->combat_timer);
    serialize(fp, ptmp->next_cloud_tick);
    serialize(fp, ptmp->level);
    serialize(fp, ptmp->gold);
    serialize_ohandle_array(fp, ptmp->inventory, INVENTORY_SIZE);
    serialize_ohandle_array(fp, &(ptmp->weapon), 1);
    serialize_ohandle_array(fp, &(ptmp->armour), 1);
    serialize_ohandle_array(fp, &(ptmp->ring), 1);
    std::list<Perseff_data>::const_iterator peffiter;
    for (peffiter = u.perseffs.begin(); peffiter != u.perseffs.end(); ++peffiter)
    {
        serialize(fp, uint32_t(peffiter->flavour));
        serialize(fp, peffiter->power);
        serialize(fp, peffiter->duration);
        serialize(fp, peffiter->by_you);
        serialize(fp, peffiter->on_you);
        serialize(fp, peffiter->caster.value);
        serialize(fp, peffiter->victim.value);
    }
    serialize(fp, 0xffffffffu);
}

void serialize(FILE *fp, const Level_tag *ptag)
{
    serialize(fp, uint32_t(ptag->dungeon));
    serialize(fp, ptag->level);
}

void serialize(FILE *fp, Obj const *optr)
{
#ifdef DEBUG_LOADSAVE
    print_msg(0, "   serialized object of type %d pos %d %d levtag %d %d withyou %d", optr->obj_id, optr->pos.y, optr->pos.x, optr->lev.dungeon, optr->lev.level, optr->with_you);
#endif
    serialize(fp, optr->obj_id);
    serialize(fp, optr->quan);
    serialize(fp, optr->with_you);
    serialize(fp, &(optr->lev));
    serialize(fp, optr->pos);
    serialize(fp, optr->durability);
    serialize(fp, optr->meta, 2);
}

void serialize_objects(FILE *fp)
{
    int32_t count;
    std::map<uint64_t, Obj *>::iterator iter;
    count = objects.size();
    serialize(fp, count);
    for (iter = objects.begin(); iter != objects.end(); ++iter)
    {
#ifdef DEBUG_LOADSAVE
        print_msg(0, "serialising object ID %llx", iter->first);
#endif
        serialize(fp, iter->first);
        serialize(fp, iter->second);
    }
}

void serialize_ohandle_array(FILE *fp, Obj_handle const *array, int count)
{
    int i;
    for (i = 0; i < count; ++i)
    {
        serialize(fp, array[i].value);
    }
}

static void serialize_gamestate(FILE *fp)
{
    serialize(fp, uint32_t((MAJVERS << 24) | (MINVERS << 16) | (PATCHVERS << 8)));
    serialize(fp, game_tick);
    serialize(fp, next_obj_handle);
    serialize(fp, next_mon_handle);
    serialize(fp, rng_state, 5);
}

void serialize_levels(FILE *fp)
{
    std::map<Level_tag, Level *>::iterator iter;
    int i = 0;
    for (iter = levels.begin(); iter != levels.end(); ++iter)
    {
        serialize(fp, &(iter->first));
        ++i;
        serialize(fp, iter->second);
    }
    serialize(fp, &no_level);
}

void serialize(FILE *fp, Level const *lp)
{
    int i;
    // parameters
    serialize(fp, lp->height);
    serialize(fp, lp->width);
    serialize(fp, lp->levtype);
    // connectivity
    Exitmode_citer exciter;
    for (exciter = lp->exit_modes.begin();
         exciter != lp->exit_modes.end();
         ++exciter)
    {
        serialize(fp, exciter->first);
        serialize(fp, uint32_t(exciter->second));
    }
    serialize(fp, dunbash::NOWHERE);
    Exitdest_citer edciter;
    for (edciter = lp->exit_dests.begin();
         edciter != lp->exit_dests.end();
         ++edciter)
    {
        serialize(fp, uint32_t(edciter->first));
        serialize(fp, &(edciter->second));
    }
    serialize(fp, 0xffffffffu);
    Entry_citer enciter;
    for (enciter = lp->entries.begin();
         enciter != lp->entries.end();
         ++enciter)
    {
        serialize(fp, uint32_t(enciter->first));
        serialize(fp, enciter->second);
    }
    serialize(fp, 0xffffffffu);
    // skip mobjs
    // skip mmons
    // skip astar parameters
    // save flags
    for (i = 0; i < lp->height; ++i)
    {
        serialize(fp, lp->mflags[i], lp->width);
    }
    // save terrain
    for (i = 0; i < lp->height; ++i)
    {
        serialize(fp, (uint32_t *) lp->terrain[i], lp->width);
    }
    // save regionnums
    for (i = 0; i < lp->height; ++i)
    {
        serialize(fp, (uint32_t *) lp->rnums[i], lp->width);
    }
    switch (lp->levtype)
    {
    case LEVEL_ROOMS:
        {
            Levext_rooms const *lerp = dynamic_cast<Levext_rooms const *>(lp->levextra);
            /* I could check for errors, or I could have the program segfault its
             * guts out. The latter is probably saner. */
            serialize(fp, lerp);
        }
        break;
    case LEVEL_ROOMS_BOSS:
        {
            Levext_rooms_boss const *lerp = dynamic_cast<Levext_rooms_boss const *>(lp->levextra);
            /* I could check for errors, or I could have the program segfault its
             * guts out. The latter is probably saner. */
            serialize(fp, lerp);
        }
        break;
    }
}

void serialize(FILE *fp, Levext_rooms_boss const *lerp)
{
#ifdef DEBUG_LOADSAVE
    print_msg(0, "serialising Levext_rooms");
#endif
    serialize(fp, (Levext_rooms const *) lerp);
    // then serialize the extra stuff
    serialize(fp, uint32_t(lerp->spec.x_terrain));
    serialize(fp, uint32_t(lerp->spec.hash_terrain));
    serialize(fp, uint32_t(lerp->spec.dot_terrain));
    serialize(fp, uint32_t(lerp->spec.tilde_terrain));
    serialize(fp, lerp->spec.num_pmons, 10);
    serialize(fp, lerp->spec.num_pobjs, 10);
    for (int i = 0; i < 10; ++i)
    {
        serialize(fp, uint32_t(lerp->spec.num_terrs[i]));
    }
}

void serialize(FILE *fp, Levext_rooms const *lerp)
{
#ifdef DEBUG_LOADSAVE
    print_msg(0, "serialising Levext_rooms");
#endif
    serialize(fp, lerp->overridden_monsel);
    serialize(fp, lerp->actual_rooms);
    serialize(fp, lerp->zoo_room);
    serialize(fp, lerp->zoo_style);
    serialize(fp, lerp->dstairs_room);
    serialize(fp, lerp->ustairs_room);
    serialize(fp, lerp->ustairs_pos);
    serialize(fp, lerp->dstairs_pos);
    int i;
    for (i = 0; i < Levext_rooms::MAX_ROOMS; ++i)
    {
        serialize(fp, lerp->bounds[i][0]);
        serialize(fp, lerp->bounds[i][1]);
        serialize(fp, lerp->segsused[i]);
        serialize(fp, uint32_t(lerp->roomflav[i]));
    }
    int j;
    for (i = 0; i < Levext_rooms::MAX_ROOMS; ++i)
    {
        for (j = 0; j < Levext_rooms::MAX_ROOMS; ++j)
        {
            serialize(fp, lerp->linkage[i][j]);
        }
    }
}

void serialize(FILE *fp, Mon const *mptr)
{
    Astar_path::iterator iter;
    serialize(fp, mptr->mon_id);
    serialize(fp, &(mptr->lev));
    serialize(fp, mptr->pos);
    serialize(fp, mptr->ai_lastpos);
    serialize(fp, mptr->hpmax);
    serialize(fp, mptr->hpcur);
    serialize(fp, mptr->mtohit);
    serialize(fp, mptr->rtohit);
    serialize(fp, mptr->defence);
    serialize(fp, mptr->mdam);
    serialize(fp, mptr->rdam);
    serialize(fp, mptr->awake);
    serialize(fp, mptr->meta);
    serialize(fp, mptr->no_exp);
    serialize(fp, mptr->sex);
    if (mptr->name)
    {
        uint32_t k = strlen(mptr->name) + 1;
        serialize(fp, k);
        fwrite(mptr->name, k, 1, fp);
    }
    else
    {
        serialize(fp, uint32_t(0));
    }
    if (mptr->current_path)
    {
        for (iter = mptr->current_path->begin();
             iter != mptr->current_path->end();
             ++iter)
        {
            serialize(fp, *iter);
        }
    }
    serialize(fp, dunbash::NOWHERE);
    std::list<Perseff_data>::const_iterator peffiter;
    for (peffiter = mptr->perseffs.begin(); peffiter != mptr->perseffs.end(); ++peffiter)
    {
        serialize(fp, uint32_t(peffiter->flavour));
        serialize(fp, peffiter->power);
        serialize(fp, peffiter->duration);
        serialize(fp, peffiter->by_you);
        serialize(fp, peffiter->on_you);
        serialize(fp, peffiter->caster.value);
        serialize(fp, peffiter->victim.value);
    }
    serialize(fp, 0xffffffffu);
}

void serialize_monsters(FILE *fp)
{
    uint32_t count = monsters.size();
    std::map<uint64_t, Mon *>::const_iterator iter;
    serialize(fp, count);
    for (iter = monsters.begin(); iter != monsters.end(); ++iter)
    {
        serialize(fp, iter->first);
        serialize(fp, iter->second);
    }
}

void serialize_permobj_vars(FILE *fp)
{
    int i;
    for (i = 0; i < PO_COUNT; ++i)
    {
        serialize(fp, permobjs[i].known);
        serialize(fp, permobjs[i].power);
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
    serialize_gamestate(fp);
    serialize(fp, &u);
    serialize_levels(fp);
    serialize_monsters(fp);
    serialize_objects(fp);
    serialize_permobj_vars(fp);
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
            deserialize_gamestate(fp);
            deserialize(fp, &u);
            deserialize_levels(fp);
            currlev = u.lev.snapv();
            if (!currlev)
            {
                print_msg(0, "fatal error: invalid u.lev");
                sleep(1);
                abort();
            }
            deserialize_monsters(fp);
            deserialize_objects(fp);
            deserialize_permobj_vars(fp);
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
