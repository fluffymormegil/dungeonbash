/* rooms.hh
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

#ifndef ROOMS_HH
#define ROOMS_HH

#ifndef DUNBASH_HH
#include "dunbash.hh"
#endif

#ifndef MONSTERS_HH
#include "monsters.hh"
#endif

#ifndef OBJECTS_HH
#include "objects.hh"
#endif

#include <vector>

enum Room_flavour
{
    Rflav_dull,
    Rflav_treasure_zoo,
    Rflav_smithy,
    Rflav_crypt,
    Rflav_shrine,
    Rflav_barracks,
    Rflav_laboratory,
    Rflav_library,
    Rflav_arcane
};

struct Room : public Region
{
    Room_flavour flav;
    int linkage_group_;
    std::vector<int> linkage_;

    virtual bool linked_to(int r) const
    {
        return (r >= 0) && (((unsigned) r) < linkage_.size()) && linkage_[r];
    }
    virtual ~Room() {}
};

struct Rect_room : public Room
{
    // A rectangular room which contains every cell within its bounds.
    libmormegil::Coord bounds[2];
    bool contains(libmormegil::Coord c) const
    {
        return ((c.y > bounds[0].y) && (c.x > bounds[0].x) &&
                (c.y < bounds[1].y) && (c.x < bounds[1].x));
    }
    virtual libmormegil::Coord random_point() const 
    {
        libmormegil::Coord tmp =
        {
            exclusive_flat(bounds[0].y, bounds[1].y),
            exclusive_flat(bounds[0].x, bounds[1].x)
        };
        return tmp;
    }
};

struct Levext_rooms : public Levextra
{
    enum
    {
        NO_ROOM = -1,
        MAX_ROOMS = 9
    };
    enum Zoo_style
    {
        NO_ZOO = -1,
        ZOO_TREASURE,
        ZOO_MORGUE,
        ZOO_SHRINE_FIRE,
        ZOO_SHRINE_IRON,
        ZOO_SHRINE_BONE,
        ZOO_SHRINE_DECAY, /* "acid" next to altar */
        ZOO_SHRINE_FLESH, /* bathing pools */
        ZOO_SMITHY,
        TOTAL_ZOOS
    };
    int16_t actual_rooms;
    int16_t zoo_room;
    Zoo_style zoo_style;
    int16_t dstairs_room;
    int16_t ustairs_room;
    libmormegil::Coord ustairs_pos;
    libmormegil::Coord dstairs_pos;
    libmormegil::Coord bounds[MAX_ROOMS][2];
    int16_t linkage[MAX_ROOMS][MAX_ROOMS];
    bool segsused[MAX_ROOMS];
    Room_flavour roomflav[MAX_ROOMS];

    Levext_rooms();
    virtual void excavate(void);
    virtual void populate(void);
    virtual libmormegil::Coord get_injection_point(Leventry_mode mode) const;
    virtual void add_random_room(int yseg, int xseg);
    virtual void excavate_zoo_room(void);
    void excavate_room(int roomidx);
    void link_rooms(int r1, int r2);
    void put_stairs(void);
    virtual void populate_zoo_room(void);
    void populate_treasure_zoo(void);
    void populate_smithy(void);
    void populate_shrine(void);
    void populate_morgue(void);
    libmormegil::Coord get_room_cell(int room) const;
    int get_levgen_mon_spot(libmormegil::Coord *ppos) const;
    void excavate_corridor_segment(libmormegil::Coord c1, libmormegil::Coord c2, bool door1, bool door2);
    void excavate_normal_room(int rnum);
    void excavate_shrine(int rnum);
    void excavate_morgue(int rnum);
    void excavate_smithy(int rnum);
    virtual int leave_region(libmormegil::Coord c);
    virtual int enter_region(libmormegil::Coord c);
};

#endif

/* rooms.hh */
