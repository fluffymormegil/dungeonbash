/* bmagic.cc - monster spellcasting code
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
#include "bmagic.hh"
#include "objects.hh"
#include "monsters.hh"
#include "pmonid.hh"
#include "pobjid.hh"
#include "combat.hh"
#include "radiance.hh"
#include <string.h>

/* BLACK MAGIC
 *
 * Certain of the denizens of the dungeon have the power to use black magic
 * against the player.
 * 
 * The "ordinary" lich may unleash bolts of necromantic force against
 * the player, or smite him at close quarters with their staves of necromancy,
 * or invoke grim curses against him.
 *
 * The dreaded master liches can smite the player from a distance with
 * their necromantic powers without lying on a cardinal direction from
 * him, and can steal the player's vitality with a touch, as well as having
 * the spells of their lesser brethren.  Furthermore, they may attempt to
 * evade the player in the same manner as wizards.
 *
 * Itinerant wizards roaming the dungeon cast bolts of lightning, and strike
 * in hand-to-hand combat with staves wreathed with enchantments of shattering
 * force; if sorely pressed, they may invoke their powers to teleport across
 * the dungeon level, cheating the player of his victory.
 * 
 * Archmages, learned scholars of the Black Arts and veterans of many a
 * confrontation, have the powers of wizards. In addition, an archmage who
 * teleports away from the player to evade death may well leave him with a
 * group of summoned monsters.
 * 
 * The more potent order of demons known as defilers may cast curses against
 * the player, or call down a column of fire to smite him.
 *
 * Some forms of black magic may be defended against by wearing the proper
 * armour or putting on a suitable ring; others bypass all such defences to
 * strike the player directly, although some of these can be evaded by those
 * with high enough agility.
 */

Monspell selector_mundane(Mon const *mptr, bool cansee, const Direction_data& dir_data);
Monspell selector_archmage(Mon const *mptr, bool cansee, const Direction_data& dir_data);
Monspell selector_wizard(Mon const *mptr, bool cansee, const Direction_data& dir_data);
Monspell selector_defiler(Mon const *mptr, bool cansee, const Direction_data& dir_data);
Monspell selector_immolator(Mon const *mptr, bool cansee, const Direction_data& dir_data);
Monspell selector_dominator(Mon const *mptr, bool cansee, const Direction_data& dir_data);
Monspell selector_deathlord(Mon const *mptr, bool cansee, const Direction_data& dir_data);
Monspell selector_darkangel(Mon const *mptr, bool cansee, const Direction_data& dir_data);
Monspell selector_lich(Mon const *mptr, bool cansee, const Direction_data& dir_data);
Monspell selector_master_lich(Mon const *mptr, bool cansee, const Direction_data& dir_data);

/*
 * TODO Cooldowns on powerful spells.
 *
 * TODO Delayed-triggering spells. (For ones directly cast on the player, these
 * can be implemented as persistent effects whose per-turn action is to do
 * nothing.)
 */
int corruption_spell(Mon const *mptr);
int animate_dead(Mon const *mptr);
void chainstrike_spell(Mon const *mptr);
void shackle_spell(Mon const *mptr);

int use_black_magic(Mon_handle mon)
{
    /* Returns zero for no spell selected, -1 for unsupported spell
     * selected, 1 for supported spell selected. */
    Mon *mptr = mon.snapv();
    Monspell to_cast = MS_REJECT;
    int rval = 1;	/* Default to success; failure paths will force this
                         * to an appropriate value. */
    Direction_data dir_data;
    Perseff_data peff;
    int i;
    bool cansee;
    std::string castername;
    compute_directions(u.pos, mptr->pos, &dir_data);
    cansee = mptr->in_fov();
    switch (mptr->mon_id)
    {
    case PM_ARCHMAGE:
        to_cast = selector_archmage(mptr, cansee, dir_data);
        break;

    case PM_WIZARD:
        to_cast = selector_wizard(mptr, cansee, dir_data);
        break;

    case PM_MASTER_LICH:
        to_cast = selector_master_lich(mptr, cansee, dir_data);
        break;

    case PM_LICH:
        to_cast = selector_lich(mptr, cansee, dir_data);
        break;

    case PM_DEFILER:
        to_cast = selector_defiler(mptr, cansee, dir_data);
        break;

    case PM_DOMINATOR:
        to_cast = selector_dominator(mptr, cansee, dir_data);
        break;
    case PM_IMMOLATOR:
        to_cast = selector_immolator(mptr, cansee, dir_data);
        break;
    case PM_DEATHLORD:
        to_cast = selector_deathlord(mptr, cansee, dir_data);
        break;
    case PM_DARK_ANGEL:
        to_cast = selector_darkangel(mptr, cansee, dir_data);
        break;

    default:
        print_msg(MSGCHAN_INTERROR, "WARNING: attempt to have non-caster cast spells.\n");
        to_cast = selector_mundane(mptr, cansee, dir_data);
        break;
    }
    switch (to_cast)
    {
    default:
        /* If this happens, we're trying to cast an unimplemented
         * spell. */
        print_msg(MSGCHAN_INTERROR, "Can't happen: Bogus spell %d!\n", to_cast);
        rval = -1;
        break;

    case MS_REJECT:
        /* No usable spell available. */
        rval = 0;
        break;

    case MS_STRIKE_STAFF:
        mhitu(mon, DT_PHYS);
        break;

    case MS_NECRO_STAFF:
        mhitu(mon, DT_NECRO);
        break;

    case MS_CHILLING_TOUCH:
        mhitu(mon, DT_COLD);
        break;

    case MS_LIGHTNING:
        mshootu(mon, DT_ELEC);
        break;

    case MS_NECRO_BOLT:
        mshootu(mon, DT_NECRO);
        break;

    case MS_TELEPORT_AND_SUMMON:
        /* Do the summoning... */
        mptr->get_name(&castername, 3, true);
        print_msg(0, "%s calls for help...\n", castername.c_str());
        /* (Try to) summon 2-6 monsters. */
        i = summoning(mptr->pos, dice(2, 3));
        if (i == 0)
        {
            print_msg(0, "... luckily for you, help wasn't listening.\n");
        }
        else
        {
            print_msg(0, "... and gets it.\n");
        }
        /* ... and fall through. */
    case MS_TELEPORT_ESCAPE:
        mptr->get_name(&castername, 3, true);
        print_msg(0, "%s vanishes in a puff of smoke.\n", castername.c_str());
        teleport_mon(mon);
        break;

    case MS_TELEPORT_ASSAULT:
        /* It is rare that a monster will cast this spell, but not
         * unheard of. */
        teleport_mon_to_you(mon);
        break;

    case MS_CURSE_ARMOURMELT:
        mptr->curses();
        if (u.status.test_flag(Perseff_protection))
        {
            malignant_aura();
        }
        else
        {
            peff.flavour = Perseff_armourmelt_curse;
            peff.duration = 10 + one_die(10);
            peff.caster = mptr->self;
            peff.by_you = false;
            print_msg(0, "Your armour seems uncannily fragile!\n");
            u.apply_effect(peff);
        }
        break;

    case MS_CURSE_LEADFOOT:
        mptr->curses();
        if (u.status.test_flag(Perseff_protection))
        {
            malignant_aura();
        }
        else
        {
            peff.flavour = Perseff_leadfoot_curse;
            peff.duration = 10 + one_die(10);
            peff.caster = mptr->self;
            peff.by_you = false;
            print_msg(0, "Your feet feel like lead!\n");
            u.apply_effect(peff);
        }
        break;

    case MS_CURSE_WITHERING:
        mptr->curses();
        if (u.status.test_flag(Perseff_protection))
        {
            malignant_aura();
        }
        else
        {
            peff.flavour = Perseff_wither_curse;
            peff.duration = 10 + one_die(10);
            peff.caster = mptr->self;
            peff.by_you = false;
            print_msg(0, "Crippling decrepitude afflicts you!\n");
            u.apply_effect(peff);
        }
        break;

    case MS_NECRO_SMITE:
        mptr->curses();
        if (player_resists_dtype(DT_NECRO))
        {
            print_msg(0, "Darkness reaches towards you, but dissolves.\n");
        }
        else
        {
            print_msg(0, "Soul-chilling darkness engulfs you!\n");
            damage_u(dice(1, 20), DEATH_KILLED_MON, permons[mon.snapc()->mon_id].name);
        }
        break;

    case MS_FIRE_COLUMN:
        mptr->curses();
        if (player_resists_dtype(DT_FIRE))
        {
            print_msg(0, "The fires of Hell lightly singe you.\n");
            damage_u(dice(1, 5), DEATH_KILLED_MON, permons[mon.snapc()->mon_id].name);
        }
        else
        {
            print_msg(0, "The fires of Hell burn you!\n");
            damage_u(dice(1, 20), DEATH_KILLED_MON, permons[mon.snapc()->mon_id].name);
        }
        break;

    case MS_ANIMATE_DEAD:
        animate_dead(mptr);
        break;

    case MS_CORRUPTION:
        corruption_spell(mptr);
        break;

    case MS_CHAINSTRIKE:
        chainstrike_spell(mptr);
        break;

    case MS_SHACKLE:
        shackle_spell(mptr);
        break;
    }
    return rval;
}

void malignant_aura()
{
    print_msg(MSGCHAN_BORINGFAIL, "A malignant aura surrounds you briefly.\n");
}

void Mon::curses() const
{
    std::string castername;
    get_name(&castername, 3, true);
    print_msg(0, "%s points at you and curses horribly.\n", castername.c_str());
}

void Mon::incants() const
{
    std::string castername;
    get_name(&castername, 3, true);
    print_msg(0, "%s utters a fell incantation.\n", castername.c_str());
}

Monspell selector_archmage(Mon const *mptr, bool cansee, const Direction_data& dir_data)
{
    if (cansee)
    {
        /* We have LOS; choose a spell on that basis. */
        if ((mptr->hpcur < (mptr->hpmax * 25 / 100)) && (zero_die(10) < 2))
        {
            return zero_die(3) ? MS_TELEPORT_ESCAPE : MS_TELEPORT_AND_SUMMON;
        }
        else if (dir_data.meleerange && (zero_die(10) > 3))
        {
            return MS_STRIKE_STAFF;
        }
        else if (dir_data.oncardinal)
        {
            return MS_LIGHTNING;
        }
    }
    else if (!zero_die(40))
    {
        /* 
         * We lack LOS, but pass the 1-in-40 chance; use
         * black magic to relocate us to the player's location.
         */
        return MS_TELEPORT_ASSAULT;
    }
    return MS_REJECT;
}

Monspell selector_wizard(Mon const *mptr, bool cansee, const Direction_data& dir_data)
{
    if (cansee)
    {
        if ((mptr->hpcur < (mptr->hpmax * 25 / 100)) && (zero_die(10) < 2))
        {
            return MS_TELEPORT_ESCAPE;
        }
        else if (dir_data.meleerange && (zero_die(10) > 2))
        {
            return MS_STRIKE_STAFF;
        }
        else if (dir_data.oncardinal)
        {
            return MS_LIGHTNING;
        }
        else
        {
            return MS_REJECT;
        }
    }
    else if (!zero_die(80))
    {
        /* we lack LOS, but passed the 1-in-80 chance to
         * close with the player by means of black magic. */
        return MS_TELEPORT_ASSAULT;
    }
    else
    {
        return MS_REJECT;
    }
}

Monspell selector_dominator(Mon const *mptr, bool cansee, const Direction_data& dir_data)
{
    if (cansee)
    {
        if (!dir_data.meleerange)
        {
            return MS_CHAINSTRIKE;
        }
        else if (!u.test_mobility(false))
        {
            return MS_SHACKLE;
        }
        else
        {
            return MS_REJECT;
        }
    }
    else
    {
        return MS_REJECT;
    }
}

Monspell selector_deathlord(Mon const *mptr, bool cansee, const Direction_data& dir_data)
{
    if (cansee)
    {
        if (!dir_data.meleerange)
        {
            switch (zero_die(7))
            {
            case 6:
                if (!u.status.test_flag(Perseff_wither_curse))
                {
                    return MS_CURSE_WITHERING;
                }
            case 4:
                if (!u.status.test_flag(Perseff_leadfoot_curse))
                {
                    return MS_CURSE_LEADFOOT;
                }
                /* fall through */
            case 5:
                if (!u.status.test_flag(Perseff_armourmelt_curse))
                {
                    return MS_CURSE_ARMOURMELT;
                }
                /* fall through */
            default:
                return MS_CORRUPTION;
            }
        }
        else
        {
            return MS_REJECT;
        }
    }
    else
    {
        return MS_REJECT;
    }
}

Monspell selector_immolator(Mon const *mptr, bool cansee, const Direction_data& dir_data)
{
    if (cansee)
    {
        if (!dir_data.meleerange)
        {
            switch (zero_die(7))
            {
            case 6:
                if (!u.status.test_flag(Perseff_wither_curse))
                {
                    return MS_CURSE_WITHERING;
                }
            case 4:
                if (!u.status.test_flag(Perseff_leadfoot_curse))
                {
                    return MS_CURSE_LEADFOOT;
                }
                /* fall through */
            case 5:
                if (!u.status.test_flag(Perseff_armourmelt_curse))
                {
                    return MS_CURSE_ARMOURMELT;
                }
                /* fall through */
            default:
                return MS_CORRUPTION;
            }
        }
        else
        {
            return MS_REJECT;
        }
    }
    else
    {
        return MS_REJECT;
    }
}

Monspell selector_defiler(Mon const *mptr, bool cansee, const Direction_data& dir_data)
{
    if (cansee)
    {
        if (!dir_data.meleerange)
        {
            switch (zero_die(7))
            {
            case 6:
                if (!u.status.test_flag(Perseff_wither_curse))
                {
                    return MS_CURSE_WITHERING;
                }
            case 4:
                if (!u.status.test_flag(Perseff_leadfoot_curse))
                {
                    return MS_CURSE_LEADFOOT;
                }
                /* fall through */
            case 5:
                if (!u.status.test_flag(Perseff_armourmelt_curse))
                {
                    return MS_CURSE_ARMOURMELT;
                }
                /* fall through */
            default:
                return MS_CORRUPTION;
            }
        }
        else
        {
            return MS_REJECT;
        }
    }
    else
    {
        return MS_REJECT;
    }
}

Monspell selector_lich(Mon const *mptr, bool cansee, const Direction_data& dir_data)
{
    int dieroll;
    if (cansee)
    {
        if (dir_data.meleerange)
        {
            dieroll = zero_die(6);
            switch (dieroll)
            {
            case 4:
                if (!u.status.test_flag(Perseff_leadfoot_curse))
                {
                    return MS_CURSE_LEADFOOT;
                }
                /* fall through */
            case 5:
                if (!u.status.test_flag(Perseff_armourmelt_curse))
                {
                    return MS_CURSE_ARMOURMELT;
                }
                /* fall through */
            default:
                return MS_NECRO_STAFF;
            }
        }
        else if (dir_data.oncardinal)
        {
            if (int(dir_data.delta) < 3)
            {
                switch (zero_die(6))
                {
                case 4:
                    if (!u.status.test_flag(Perseff_leadfoot_curse))
                    {
                        return MS_CURSE_LEADFOOT;
                    }
                    /* fall through */
                case 5:
                    if (!u.status.test_flag(Perseff_armourmelt_curse))
                    {
                        return MS_CURSE_ARMOURMELT;
                    }
                    /* fall through */
                default:
                    return MS_NECRO_BOLT;
                }
            }
            else
            {
                return MS_NECRO_BOLT;
            }
        }
    }
    return MS_REJECT;
}

Monspell selector_master_lich(Mon const *mptr, bool cansee, const Direction_data& dir_data)
{
    if (cansee)
    {
        if ((mptr->hpcur < (mptr->hpmax * 25 / 100)) && (zero_die(10) < 4))
        {
            return !zero_die(3) ? MS_TELEPORT_ESCAPE : MS_TELEPORT_AND_SUMMON;
        }
        else if (dir_data.meleerange)
        {
            switch (zero_die(7))
            {
            case 6:
                if (!u.status.test_flag(Perseff_wither_curse))
                {
                    return MS_CURSE_WITHERING;
                }
            case 4:
                if (!u.status.test_flag(Perseff_leadfoot_curse))
                {
                    return MS_CURSE_LEADFOOT;
                }
                /* fall through */
            case 5:
                if (!u.status.test_flag(Perseff_armourmelt_curse))
                {
                    return MS_CURSE_ARMOURMELT;
                }
                /* fall through */
            default:
                return zero_die(2) ? MS_CHILLING_TOUCH : MS_STRIKE_STAFF;
            }
        }
        else if (int(dir_data.delta) < 3)
        {
            switch (zero_die(10))
            {
            case 9:
                if (!u.status.test_flag(Perseff_wither_curse))
                {
                    return MS_CURSE_WITHERING;
                }
            case 8:
                if (!u.status.test_flag(Perseff_leadfoot_curse))
                {
                    return MS_CURSE_LEADFOOT;
                }
                /* fall through */
            case 7:
                if (!u.status.test_flag(Perseff_armourmelt_curse))
                {
                    return MS_CURSE_ARMOURMELT;
                }
                /* fall through */
            default:
                return MS_NECRO_SMITE;
            }
        }
        else if (int(dir_data.delta) < 8)
        {
            switch (zero_die(7))
            {
            case 6:
                if (!u.status.test_flag(Perseff_wither_curse))
                {
                    return MS_CURSE_WITHERING;
                }
            case 4:
                if (!u.status.test_flag(Perseff_leadfoot_curse))
                {
                    return MS_CURSE_LEADFOOT;
                }
                /* fall through */
            case 5:
                if (!u.status.test_flag(Perseff_armourmelt_curse))
                {
                    return MS_CURSE_ARMOURMELT;
                }
                /* fall through */
            default:
                return MS_NECRO_SMITE;
            }
        }
    }
    else if (!zero_die(40))
    {
        /* we lack LOS, but passed the 1-in-40 chance to
         * close with the player by means of black magic. */
        return MS_TELEPORT_ASSAULT;
    }
    return MS_REJECT;
}

Monspell selector_darkangel(Mon const *mptr, bool cansee, const Direction_data& dir_data)
{
    if (cansee)
    {
    }
    else
    {
        // dark angels don't do assault teleports.
    }
    return MS_REJECT;
}

Monspell selector_mundane(Mon const *mptr, bool cansee, const Direction_data& dir_data)
{
    return MS_REJECT;
}

/* New spell for v1.128.0 Experimental: Raise Dead.
 *
 * This spell turns all corpses within a 3sq radius of the caster (and in
 * their FOV) into zombies.
 *
 * When I add boneyard floors, it will raise skeletons from them.
 *
 * Returns: The number of corpses raised.
 */

int animate_dead(Mon const *mptr)
{
    Obj *optr;
    libmrl::Coord indices;
    libmrl::Coord cell;
    int zombies = 0;
    bool saw_zombie;
    Square_radiance animation_map =
    {
        {},
        mptr->pos,
        3,
        block_vision
    };
    mptr->incants();
    memset(animation_map.array, 0, sizeof animation_map.array);
    animation_map.array[10][10] = 1;
    irradiate_square(&animation_map);
    for ((indices.y = 7), (cell.y = mptr->pos.y - 3);
         (indices.y <= 13);
         indices.y++, cell.y++)
    {
        for ((indices.x = 7), (cell.x = mptr->pos.x - 3);
             (indices.x <= 13);
             indices.x++, cell.x++)
        {
            Obj_handle oh = currlev->object_at(cell);
            if (oh.valid())
            {
                optr = oh.snapv();
                if (optr->obj_id == PO_CORPSE)
                {
                    libmrl::Coord pos = get_mon_scatter(cell);
                    if (pos != libmrl::NOWHERE)
                    {
                        ++zombies;
                        if (pos_visible(cell) || pos_visible(pos))
                        {
                            saw_zombie = true;
                        }
                        create_mon(optr->meta, pos);
                    }
                }
                consume_obj(oh);
            }
        }
    }
    if (saw_zombie)
    {
        print_msg(0, "The dead rise up!\n");
    }
    return zombies;
}

/* New spell for 1.128.0 Experimental: Corruption
 *
 * This spell is *nasty*. It either drains a stat point, damages your armour,
 * damages your weapon, or just does some damage.
 */

int corruption_spell(Mon const *mptr)
{
    int dieroll = zero_die(10);
    int rv = 0;
    mptr->curses();
    switch (dieroll)
    {
    case 0:
        print_msg(0, "The air smells of bitter almonds for a moment.\n");
        break;
    case 1:
        print_msg(0, "Your weapon seems more fragile.\n");
        damage_obj(u.weapon);
        rv = 1;
        break;
    case 2:
        print_msg(0, "Your armour seems more fragile.\n");
        damage_obj(u.armour);
        rv = 1;
        break;
    case 3:
        print_msg(0, "Vile fumes make you reel.\n");
        drain_agility(1, "mystical corruption", false);
        rv = 1;
        break;
    case 4:
        print_msg(0, "You feel feverish.\n");
        drain_body(1, "mystical corruption", false);
        rv = 1;
        break;
    default:
        print_msg(0, "Corruption wracks your body.\n");
        damage_u(dice(1, 10), DEATH_KILLED_MON, permons[mptr->mon_id].name);
        rv = 1;
        break;
    }
    return rv;
}

/* New spell for 1.130.0 Experimental: Chainstrike
 */

void chainstrike_spell(Mon const *mptr)
{
    int chains = dice(1, 4);
    int hits = 0;
    int dmg = 0;
    std::string name;
    mptr->curses();
    print_msg(0, "Barbed chains lash at you!\n");
    for (int i = 0; i < chains; ++i)
    {
        int dieroll = zero_die(50);
        if (dieroll > u.defence)
        {
            ++hits;
            dmg += dice(1, 10);
        }
    }
    if (hits)
    {
        if (mptr->name)
        {
            mptr->get_name(&name, 1, false);
        }
        else
        {
            name = permons[mptr->mon_id].name;
        }
        print_msg(0, "You are struck by %s of them.\n", numberwords[hits]);
        damage_u(dmg, DEATH_KILLED_MON, name.c_str());
    }
}

void shackle_spell(Mon const *mptr)
{
    mptr->curses();
    if (u.test_mobility(false))
    {
        int dur = libmrl::max(15 - zero_die((u.body + u.agility) / 5), 0);
        if (dur > 0)
        {
            Perseff_data peff = {
                Perseff_binding_chains,
                20,
            };
            u.apply_effect(peff);
        }
        else
        {
            print_msg(0, "Chains reach up from the floor to seize you, but you elude their grasp.\n");
        }
    }
}

/* bmagic.cc */
