/* objects.cc
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

#define OBJECTS_CC

#include "dunbash.hh"
#include "objects.hh"
#include "pobjid.hh"
#include "monsters.hh"

std::map<uint64_t, Obj *> objects;
const Obj_handle NO_OBJECT(0ull);
uint64_t next_obj_handle = 1ull;

const char ring_colours[20][16] = {
    "gold", "ruby", "sapphire", "ivory", "coral",
    "amethyst", "silver", "iron", "copper", "jade",
    "haematite", "bone", "crystal", "platinum", "lead",
    "diamond", "topaz", "emerald", "electrum", "smoky quartz"
};

const char scroll_titles[20][16] = {
    "grem pho", "terra terrax", "phong", "ateh malkuth", "xixaxa",
    "aku ryo tai san", "qoph shin tau", "ythek shri", "ia ia", "cthulhu fhtagn",
    "arifech malex", "DOOM", "leme athem", "hail smkznrf", "rorrim foo",
    "ad aerarium", "ligemrom", "asher ehiyeh", "YELLOW SIGN", "ELDER SIGN"
};

const char potion_colours[20][16] = {
    "purple", "red", "blue", "green", "yellow",
    "orange", "white", "black", "brown", "fizzy",
    "grey", "silver", "gold", "shimmering", "glowing",
    "navy blue", "bottle green", "amber", "lilac", "ivory"
};

void identify_pobj(int num)
{
    permobjs[num].known = 1;
}

int read_scroll(Obj_handle obj)
{
    Obj *optr = obj.snapv();
    int i;
    Perseff_data peff;
    switch (optr->obj_id)
    {
    case PO_IDENTIFY_SCROLL:
        print_msg(0, "This is an identify scroll!");
        for (i = 0; i < 19; i++)
        {
            if (u.inventory[i].valid())
            {
                identify_pobj(u.inventory[i].otyp());
            }
        }
        break;
    case PO_TELEPORT_SCROLL:
        teleport_u();
        break;
    case PO_FIRE_SCROLL:
        print_msg(0, "The scroll explodes in flames!");
        if (u.ring.otyp() == PO_FIRE_RING)
        {
            print_msg(0, "Your ring glows, and the flames seem cool.");
            identify_pobj(PO_FIRE_RING);
            break;
        }
        i = damage_u(dice(4, 10), DEATH_KILLED, "searing flames");
	if (!i)
	{
	    print_msg(0, "That hurt!");
	}
	break;
    case PO_SUMMONING_SCROLL:
	i = summoning(u.pos, one_die(3) + 1);
	if (i > 0)
	{
	    print_msg(0, "Monsters appear!");
	}
	else
	{
	    print_msg(0, "You hear a snarl of frustration.");
	}
	break;
    case PO_AGGRAVATING_SCROLL:
	print_msg(0, "You hear a high-pitched humming noise.");
        aggravate_monsters(currlev);
	break;
    case PO_PROTECTION_SCROLL:
	print_msg(0, "You feel like something is helping you.");
        peff.flavour = Perseff_protection;
        peff.power = 100;
        peff.duration = 100;
        peff.by_you = true;
        peff.on_you = true;
        u.apply_effect(peff);
	break;
    default:
	print_msg(MSGCHAN_INTERROR, "Impossible: reading non-scroll");
	return 0;
    }
    identify_pobj(optr->obj_id);
    return consume_obj(obj);
}

int release_obj(Obj_handle obj)
{
    int i;
    Obj *optr = obj.snapv();
    if (optr)
    {
        if (!optr->with_you)
        {
            Level *lptr = optr->lev.snapv();
            std::set<Obj_handle>::iterator iter = lptr->booty.find(obj);
            lptr->booty.erase(iter);
            lptr->set_obj_at(optr->pos, NO_OBJECT);
        }
        else
	{
	    if (obj == u.armour)
	    {
		u.armour = NO_OBJECT;
		recalc_defence();
	    }
	    else if (obj == u.weapon)
	    {
		u.weapon = NO_OBJECT;
		recalc_defence();
	    }
	    else if (obj == u.ring)
	    {
		u.ring = NO_OBJECT;
		recalc_defence();
	    }
	    for (i = 0; i < 19; i++)
	    {
		if (u.inventory[i] == obj)
		{
		    u.inventory[i] = NO_OBJECT;
		    break;
		}
	    }
	}
        objects.erase(obj.value);
        delete optr;
        return 0;
    }
    else
    {
        return -1;
    }
}

int consume_obj(Obj_handle obj)
{
    Obj *optr = obj.snapv();
    optr->quan--;
    if (optr->quan == 0)
    {
        release_obj(obj);
	return 1;
    }
    return 0;
}

int eat_food(Obj_handle obj)
{
    Obj *optr = obj.snapv();
    if (permobjs[optr->obj_id].poclass != POCLASS_FOOD)
    {
	print_msg(MSGCHAN_INTERROR, "Error: attempt to eat non-food (%d)!", optr->obj_id);
	return -1;
    }
    if (u.food < 0)
    {
	print_msg(0, "You ravenously devour your food!");
    }
    else
    {
	print_msg(0, "You eat some food.");
    }
    u.food += 1500;
    status_updated = 1;
    display_update();
    return consume_obj(obj);
}

int quaff_potion(Obj_handle obj)
{
    Obj *optr = obj.snapv();
    switch (optr->obj_id)
    {
    case PO_BODY_POTION:
	gain_body(1, 1);
	break;
    case PO_AGILITY_POTION:
	gain_agility(1, 1);
	break;
    case PO_WEAKNESS_POTION:
	print_msg(0, "You feel that was a bad idea!");
	drain_body(one_die(4), "a potion of weakness", 1);
	drain_agility(one_die(4), "a potion of weakness", 1);
	break;
    case PO_POISON_POTION:
	print_msg(0, "This is poison!");
	damage_u(dice(3, 12), DEATH_KILLED, "drinking poison");
	display_update();
	break;
    case PO_HEALING_POTION:
	/* Heal player; if hit points brought to max, gain one
	 * hit point. */
	heal_u(dice(3, 12), 1, 1);
	break;
    case PO_RESTORATION_POTION:
	print_msg(0, "This potion makes you feel warm all over.");
	status_updated = 1;
	if (!zero_die(2))
	{
	    if (u.adam)
	    {
		u.adam = 0;
		print_msg(0, "You feel less clumsy.");
	    }
	    else if (u.bdam)
	    {
		u.bdam = 0;
		print_msg(0, "You feel less feeble.");
	    }
	}
	else
	{
	    if (u.bdam)
	    {
		u.bdam = 0;
		print_msg(0, "You feel less feeble.");
	    }
	    else if (u.adam)
	    {
		u.adam = 0;
		print_msg(0, "You feel less clumsy.");
	    }
	}
	break;
    default:
	print_msg(MSGCHAN_INTERROR, "Impossible: quaffing non-potion");
	return 0;
    }
    identify_pobj(optr->obj_id);
    return consume_obj(obj);
}

void flavours_init(void)
{
    int colour_choices[10];
    int i;
    int j;
    int done;
    /* Flavoured items use "power" to track their flavour.  This is a
     * gross and unforgiveable hack. */
    /* Rings */
    for (i = 0; i < 10;)
    {
	colour_choices[i] = zero_die(20);
	done = 1;
	for (j = 0; j < i; j++)
	{
	    if (colour_choices[i] == colour_choices[j])
	    {
		done = 0;
	    }
	}
	if (done)
	{
	    i++;
	}
    }
    permobjs[PO_REGENERATION_RING].power = colour_choices[0];
    permobjs[PO_FIRE_RING].power = colour_choices[1];
    permobjs[PO_WEDDING_RING].power = colour_choices[2];
    permobjs[PO_VAMPIRE_RING].power = colour_choices[3];
    permobjs[PO_FROST_RING].power = colour_choices[4];
    permobjs[PO_DOOM_RING].power = colour_choices[5];
    permobjs[PO_TELEPORT_RING].power = colour_choices[6];
    /* Scrolls */
    for (i = 0; i < 10;)
    {
	colour_choices[i] = zero_die(20);
	done = 1;
	for (j = 0; j < i; j++)
	{
	    if (colour_choices[i] == colour_choices[j])
	    {
		done = 0;
	    }
	}
	if (done)
	{
	    i++;
	}
    }
    permobjs[PO_FIRE_SCROLL].power = colour_choices[0];
    permobjs[PO_TELEPORT_SCROLL].power = colour_choices[1];
    permobjs[PO_SUMMONING_SCROLL].power = colour_choices[2];
    permobjs[PO_IDENTIFY_SCROLL].power = colour_choices[3];
    permobjs[PO_PROTECTION_SCROLL].power = colour_choices[4];
    permobjs[PO_AGGRAVATING_SCROLL].power = colour_choices[5];
    /* Potions */
    for (i = 0; i < 10;)
    {
	colour_choices[i] = zero_die(20);
	done = 1;
	for (j = 0; j < i; j++)
	{
	    if (colour_choices[i] == colour_choices[j])
	    {
		done = 0;
	    }
	}
	if (done)
	{
	    i++;
	}
    }
    permobjs[PO_HEALING_POTION].power = colour_choices[0];
    permobjs[PO_BODY_POTION].power = colour_choices[1];
    permobjs[PO_POISON_POTION].power = colour_choices[2];
    permobjs[PO_AGILITY_POTION].power = colour_choices[3];
    permobjs[PO_WEAKNESS_POTION].power = colour_choices[4];
    permobjs[PO_RESTORATION_POTION].power = colour_choices[5];
}

Obj_handle get_free_object_handle(void)
{
    Obj_handle otmp(next_obj_handle);
    ++next_obj_handle;
    return otmp;
}

Obj_handle create_obj_class(Poclass_num po_class, int quantity, bool with_you, libmrl::Coord pos, Level *lptr)
{
    int po_idx;
    int tryct;
    for (tryct = 0; tryct < 200; tryct++)
    {
	switch (po_class)
	{
	case POCLASS_POTION:
	    po_idx = inclusive_flat(PO_FIRST_POTION, PO_LAST_POTION);
	    break;
	case POCLASS_SCROLL:
	    po_idx = inclusive_flat(PO_FIRST_SCROLL, PO_LAST_SCROLL);
	    break;
	case POCLASS_RING:
	    po_idx = inclusive_flat(PO_FIRST_RING, PO_LAST_RING);
	    break;
	default:
	    /* No getting armour/weapons by class... yet. */
	    return NO_OBJECT;
	}
	if (zero_die(100) < permobjs[po_idx].rarity)
	{
	    continue;
	}
	break;
    }
    return create_obj(po_idx, quantity, with_you, pos, lptr);
}

int get_random_pobj(int depth)
{
    int tryct;
    int po_idx;
    for (tryct = 0; tryct < 200; tryct++)
    {
	po_idx = zero_die(PO_COUNT);
	if (zero_die(100) < permobjs[po_idx].rarity)
	{
	    po_idx = NO_POBJ;
	    continue;
	}
	/* v1.3: Do not permit generation of particularly powerful
	 * items (runeswords, mage armour, etc.) at shallow depths.
	 * (game balance fix) */
	if (depth < permobjs[po_idx].depth)
	{
	    po_idx = NO_POBJ;
	    continue;
	}
	break;
    }
    return po_idx;
}

Obj_handle create_obj(int po_idx, int quantity, bool with_you, libmrl::Coord pos, Level *lptr)
{
    Obj_handle oh = get_free_object_handle();
    Obj *optr = new Obj();
    if (!lptr)
    {
        lptr = currlev;
    }
    objects[oh.value] = optr;
    if (po_idx == NO_POBJ)
    {
	po_idx = get_random_pobj(lptr->self.level);
	if (po_idx == NO_POBJ)
	{
	    return NO_OBJECT;
	}
    }
    optr->obj_id = po_idx;
    optr->with_you = with_you;
    optr->pos = pos;
    if (po_idx == PO_GOLD_PIECE)
    {
	optr->quan = dice(lptr->self.level + 1, 20);
    }
    else
    {
	optr->quan = quantity;
    }
    switch (permobjs[po_idx].poclass)
    {
    case POCLASS_WEAPON:
    case POCLASS_ARMOUR:
	/* Set durability of weapons and armour to a suitable value.
	 * 100 has been chosen for the moment, but this may need
	 * tuning. */
	optr->durability = OBJ_MAX_DUR;
	break;
    case POCLASS_WAND:
        /* Use durability field for charges. */
        optr->durability = 10;
        break;
    default:
	break;
    }
    if (!optr->with_you)
    {
        lptr->booty.insert(oh);
        lptr->set_obj_at(pos, oh);
        optr->lev = lptr->self;
    }
    return oh;
}

void Obj::get_name(std::string *s) const
{
    char tmpbuf[32];
    char const *an;
    Permobj *poptr = permobjs + obj_id;
    /* TODO tweak this later to handle carrion that shouldn't be recognizable
     * as coming from any given species. */
    if (poptr->poclass == POCLASS_CARRION)
    {
        Permon *pmptr = permons + meta[0];
        *s = is_vowel(pmptr->name[0]) ? "an " : "a ";
        *s += pmptr->name;
        *s += " ";
        *s += poptr->name;
    }
    else if (poptr->known)
    {
        if (quan > 1)
        {
            sprintf(tmpbuf, "%d ", quan);
            *s = tmpbuf;
            *s += poptr->plural;
        }
        else if (po_is_stackable(obj_id))
        {
            *s = "1 ";
            *s += poptr->name;
        }
        else
        {
            an = is_vowel(poptr->name[0]) ? "an " : "a ";
            if ((poptr->poclass == POCLASS_WEAPON) ||
                (poptr->poclass == POCLASS_ARMOUR))
            {
                *s = an;
                *s += poptr->name;
                sprintf(tmpbuf, " (%d/%d)", durability, OBJ_MAX_DUR);
                *s += tmpbuf;
            }
            else if (poptr->poclass == POCLASS_WAND)
            {
                *s = an;
                *s += poptr->name;
                sprintf(tmpbuf, " (%d charges)", durability);
                *s += tmpbuf;
            }
            else
            {
                *s = an;
                *s += poptr->name;
            }
        }
    }
    else
    {
        sprintf(tmpbuf, " (%d)", obj_id);
        switch (poptr->poclass)
        {
	case POCLASS_NONE:
            *s = "a non-thing";
            *s += tmpbuf;
	    break;
	case POCLASS_FOOD:
            *s = "a mysterious food";
            *s += tmpbuf;
	    break;
	case POCLASS_WEAPON:
            *s = "a mysterious weapon";
            *s += tmpbuf;
	    break;
	case POCLASS_ARMOUR:
	    if ((obj_id == PO_MUNDANE_ROBE) ||
		(obj_id == PO_ROBE_OF_SHADOWS) ||
		(obj_id == PO_ROBE_OF_SWIFTNESS))
	    {
                *s = "a robe";
	    }
	    else
	    {
                *s = "some mysterious armour";
                *s += tmpbuf;
	    }
	    break;
	case POCLASS_SCROLL:
            sprintf(tmpbuf, "%d ", quan);
	    if (quan > 1)
	    {
                *s = tmpbuf;
                *s += "scrolls '";
	    }
	    else
	    {
		*s = "1 scroll '";
	    }
            *s += scroll_titles[poptr->power];
            *s += "'";
	    break;
        case POCLASS_WAND:
            *s = "a magical wand";
            break;
	case POCLASS_POTION:
            sprintf(tmpbuf, "%d ", quan);
            *s = tmpbuf;
            *s += potion_colours[poptr->power];
            *s += std::string((quan > 1) ? " potions" : " potion");
	    break;
	case POCLASS_RING:
            *s = is_vowel(ring_colours[poptr->power][0]) ? "an " : "a ";
            *s += ring_colours[poptr->power];
            *s += " ring";
	    break;
        default:
            *s = "a mysterious object";
            *s += tmpbuf;
            break;
	}
    }
}

int drop_obj(int inv_idx)
{
    Obj *optr;
    libmrl::Coord pos;
    std::string namestr;
    optr = u.inventory[inv_idx].snapv();
    pos = get_obj_scatter(u.pos);
    if (pos != libmrl::NOWHERE)
    {
        optr->pos = pos;
        currlev->set_obj_at(pos, u.inventory[inv_idx]);
        currlev->booty.insert(u.inventory[inv_idx]);
	if (u.weapon == u.inventory[inv_idx])
	{
	    u.weapon = NO_OBJECT;
	}
	u.inventory[inv_idx] = NO_OBJECT;
	optr->with_you = 0;
        optr->lev = u.lev;
        newsym(pos);
        display_update();
        optr->get_name(&namestr);
	print_msg(0, "You drop %s.", namestr.c_str());
	return 0;
    }
    else
    {
	print_msg(MSGCHAN_MINORFAIL, "There is no room to drop anything.");
    }
    return -1;
}

bool po_is_stackable(int po)
{
    switch (permobjs[po].poclass)
    {
    default:
	return false;
    case POCLASS_POTION:
    case POCLASS_SCROLL:
    case POCLASS_FOOD:
	return true;
    }
}

void attempt_pickup(void)
{
    int i;
    int stackable;
    Obj_handle floor_oh = currlev->object_at(u.pos);
    Obj *floorobj = floor_oh.snapv();
    Obj_handle inv_oh;
    Obj *invobj;
    std::string namebuf;
    std::set<Obj_handle>::iterator iter = currlev->booty.find(floor_oh);
    if (floorobj->obj_id == PO_GOLD_PIECE)
    {
	print_msg(0, "You get %d gold.", floorobj->quan);
	u.gold += floorobj->quan;
        release_obj(floor_oh);
	return;
    }
    stackable = po_is_stackable(floorobj->obj_id);
    floorobj->get_name(&namebuf);
    if (stackable)
    {
	for (i = 0; i < 19; i++)
	{
            invobj = u.inventory[i].snapv();
	    if (invobj && (invobj->obj_id == floorobj->obj_id))
	    {
		print_msg(0, "You get %s.", namebuf.c_str());
		invobj->quan += floorobj->quan;
                release_obj(floor_oh);
                goto common;
	    }
	}
    }
    for (i = 0; i < 19; i++)
    {
	if (!u.inventory[i].valid())
	{
	    break;
	}
    }
    if (i == 19)
    {
	print_msg(0, "Your pack is full.");
	return;
    }
    u.inventory[i] = currlev->object_at(u.pos);
    invobj = u.inventory[i].snapv();
    currlev->booty.erase(iter);
common:
    currlev->set_obj_at(u.pos, NO_OBJECT);
    invobj->with_you = 1;
    invobj->pos = libmrl::NOWHERE;
    invobj->get_name(&namebuf);
    print_msg(0, "You now have");
    print_msg(0, "%c) %s", 'a' + i, namebuf.c_str());
}

void damage_obj(Obj_handle obj)
{
    Obj *optr = obj.snapv();
    if (!optr)
    {
        print_msg(0, "internal error: bad handle passed to damage_obj()");
        return;
    }
    optr->durability--;
    if (optr->durability <= 0)
    {
	/* Break the object. Weapons and armour don't stack. */
	if (obj == u.weapon)
	{
	    print_msg(0, "Your weapon breaks!");
	}
	else if (obj == u.armour)
	{
	    print_msg(0, "Your armour is ruined!");
	}
        else if (permobjs[optr->obj_id].poclass == POCLASS_WAND)
        {
            print_msg(0, "Your wand is consumed in a puff of wild magic.");
        }
	consume_obj(obj);
	recalc_defence();
    }
} 

void describe_object(Obj_handle obj)
{
    Obj *optr;
    Permobj *poptr;
    std::string desc;
    optr = obj.snapv();
    poptr = permobjs + optr->obj_id;
    optr->get_name(&desc);
    print_msg(0, "%s", desc.c_str());
    print_msg(0, "");
    if (poptr->known)
    {
	print_msg(0, "%s", poptr->description);
    }
    else
    {
	switch (poptr->poclass)
	{
	case POCLASS_NONE:
	default:
	    print_msg(0, "This unidentified permobj (%d) is indescribable.", optr->obj_id);
	    break;
	case POCLASS_ARMOUR:
	    if ((optr->obj_id == PO_MUNDANE_ROBE) ||
		(optr->obj_id == PO_ROBE_OF_SHADOWS) ||
		(optr->obj_id == PO_ROBE_OF_SWIFTNESS))
	    {
		print_msg(0, "A simple woolen robe.");
	    }
	    else
	    {
		print_msg(0, "An unidentified and indescribable piece of armour (%d)", optr->obj_id);
	    }
	    break;
	case POCLASS_SCROLL:
	    print_msg(0, "A mysterious scroll.");
            print_msg(0, "Reading it will unleash its enchantment.");
	    break;
	case POCLASS_POTION:
	    print_msg(0, "A rather dubious-looking liquid.");
            print_msg(0, "Quaffing it may be baleful or beneficial.");
	    break;
	case POCLASS_RING:
	    print_msg(0, "Some rings are baneful, some are beneficial, and");
	    print_msg(0, "some are junk.");
	    break;
        case POCLASS_WAND:
            print_msg(0, "Wands can unleash powerful magic, but have strictly limited charges and vanish when depleted.");
	}
    }
}

int evasion_penalty(Obj_handle obj)
{
    switch (obj.otyp())
    {
    case PO_MUNDANE_ROBE:
	return 5;

    case PO_LEATHER_ARMOUR:
    case PO_DRAGONHIDE_ARMOUR:
	return 10;

    case PO_CHAINMAIL:
    case PO_SACRED_CHAINMAIL:
	return 25;

    case PO_PLATE_ARMOUR:
    case PO_MAGE_ARMOUR:
    case PO_METEORIC_PLATE_ARMOUR:
	return 40;

    case PO_ROBE_OF_SWIFTNESS:
	return 0;

    case PO_ROBE_OF_SHADOWS:
	return -15;	/* This is a bonus. */

    default:
	/* If you've somehow managed to wear a non-armour, you're abusing
	 * a bug; get a 100% penalty to evasion. */
	print_msg(0, "Trying to find evasion penalty of non-armour!");
	return 100;
    }
}

libmrl::Coord get_obj_scatter(libmrl::Coord pos, Level *lptr)
{
    libmrl::Coord delta;
    int tryct = 0;
    if (!lptr)
    {
        lptr = currlev;
    }
    while (((lptr->object_at(pos).valid()) ||
            terrain_data[lptr->terrain_at(pos)].impassable ||
            terrain_data[lptr->terrain_at(pos)].feature) &&
           tryct < 100)
    {
	delta.y = zero_die(3) - 1;
	delta.x = zero_die(3) - 1;
        // Don't try to scatter through a wall.
        if (int(delta) && !(terrain_data[lptr->terrain_at(pos + delta)].impassable))
        {
            ++tryct;
            pos += delta;
        }
    }
    if (tryct >= 100)
    {
	return libmrl::NOWHERE;
    }
    return pos;
}

Obj_handle create_corpse(int pm_idx, libmrl::Coord pos, Level *lptr)
{
    Obj_handle obj = create_obj(PO_CORPSE, 1, false, pos, lptr);
    if (obj.valid())
    {
        obj.snapv()->meta[0] = pm_idx;
    }
    return obj;
}

bool Obj::is_ranged() const
{
    return (obj_id == PO_CROSSBOW) || (obj_id == PO_BOW);
}

Item_quality Obj::quality() const
{
    if (permobjs[obj_id].known)
    {
        return permobjs[obj_id].qual;
    }
    else
    {
        return Itemqual_normal;
    }
}

/* objects.c */
