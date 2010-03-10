/* objects.hh
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

#ifndef OBJECTS_HH
#define OBJECTS_HH

#ifndef INDIE_HH
#include "indie.hh"
#endif

#ifndef POBJ1_HH
#include "pobj1.hh"
#endif

#ifndef POBJ2_H
#include "pobj2.hh"
#endif

#ifndef DUNBASH_H
#include "dunbash.hh"
#endif

#include <string>
#include <map>

struct Obj;

struct Obj_handle
{
    uint64_t value;
    Obj *snapv() const;
    Obj const *snapc() const;
    Obj_handle(uint64_t v = 0ull) : value(v) { }
    bool operator <(const Obj_handle& m) const { return value < m.value; }
    bool operator !=(const Obj_handle& m) const { return value != m.value; }
    bool operator ==(const Obj_handle& m) const { return value == m.value; }
    bool valid() const { return value; }
    int otyp() const;
    void wipe() const;
};
/* XXX struct Obj */
struct Obj {
    int obj_id;
    Obj_handle self;
    Level_tag lev;
    int quan;
    bool with_you;      /* Preserved when item DB is reaped on level change. */
    libmrl::Coord pos;
    int used;   /* Entry is occupied. */
    int durability;     /* Weapons and armour degrade with use. */
    uint32_t meta[2]; /* some metadata */
    void get_name(std::string *strptr) const;
    bool is_ranged() const;
    Item_quality quality() const;
};

#define MAX_OBJECTS 100 /* SHould be enough. */
extern std::map<uint64_t, Obj *> objects;

inline Obj *Obj_handle::snapv() const
{
    std::map<uint64_t, Obj *>::iterator iter = objects.find(value);
    return (iter == objects.end()) ? 0 : iter->second;
}

inline Obj const *Obj_handle::snapc() const
{
    std::map<uint64_t, Obj *>::iterator iter = objects.find(value);
    return (iter == objects.end()) ? 0 : iter->second;
}

inline int Obj_handle::otyp() const { return value ? snapc()->obj_id : NO_POBJ; }

inline void Obj_handle::wipe() const
{
    std::map<uint64_t, Obj *>::iterator iter = objects.find(value);
    if (iter != objects.end())
    {
        objects.erase(iter);
    }
}

extern const Obj_handle NO_OBJECT;
extern uint64_t next_obj_handle;

#define OBJ_MAX_DUR 100

extern int get_random_pobj(void);
extern int consume_obj(Obj_handle obj);
extern int release_obj(Obj_handle obj);
extern void fprint_obj_name(FILE *fp, Obj_handle obj);
extern void print_obj_name(Obj_handle obj);
extern void describe_object(Obj_handle obj);
extern Obj_handle create_obj(int po_idx, int quantity, bool with_you, libmrl::Coord pos, Level *lptr = 0);
extern Obj_handle create_corpse(int pm_idx, libmrl::Coord pos, Level *lptr = 0);
extern int drop_obj(int inv_idx);
extern Obj_handle create_obj_class(Poclass_num pocl, int quantity, bool with_you, libmrl::Coord pos, Level *lptr = 0);
extern libmrl::Coord get_obj_scatter(libmrl::Coord pos, Level *lptr = 0);

typedef int (*Itemuse_fptr)(Obj_handle);
extern int zap_wand(Obj_handle obj);
extern int activate_misc(Obj_handle obj);
extern int read_scroll(Obj_handle obj);
extern int quaff_potion(Obj_handle obj);
extern int eat_food(Obj_handle obj);
extern void attempt_pickup(void);
extern int damage_obj(Obj_handle obj);
extern int evasion_penalty(Obj_handle obj);

typedef Obj_handle (Inventory)[INVENTORY_SIZE];

#endif

/* objects.hh */
