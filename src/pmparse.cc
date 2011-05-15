/* pmparse.cc - permons DB parser for Martin's Dungeon Bash
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

#define PMPARSE_CC

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <ctype.h>
#include "pmon1.hh"
#include "srcgen.hh"

struct Pmparse_action;
struct Pmparse_state;
struct Flag_table;

// There's only one state object, which we recycle.
typedef int (*Pmparse_func)(const char *);

struct Pmparse_state
{
    bool live;
    std::string name;
    std::string plural;
    std::string description;
    std::string species_string;
    std::string sym_string;
    std::string colour_string;
    unsigned rarity;
    unsigned threat;
    unsigned health;
    Mon_mattk melee;
    std::string maux_string;
    Mon_rattk ranged;
    std::string rdam_string;
    std::string raux_string;
    std::string shootverb;
    int defence;
    int armour;
    int exper;
    std::string speed_string;
    std::string flag_string;
    std::string anatomy_string;
};

struct Flag_table
{
    const char *srcname;
    const char *destname;
    // for now there's only one flag field, but that might change.
    int field_num;
};

namespace
{
    FILE *srcfile;
    FILE *header;
    int line_number;

    Pmparse_state working_monster;

    int extract_mon_names(const char *s);
    int extract_mon_desc(const char *s);
    int extract_mon_species(const char *s);
    int extract_mon_symbol(const char *s);
    int extract_mon_colour(const char *s);
    int extract_mon_rarity(const char *s);
    int extract_mon_threat(const char *s);
    int extract_mon_hp(const char *s);
    int extract_mon_melee(const char *s);
    int extract_mon_ranged(const char *s);
    int extract_mon_defence(const char *s);
    int extract_mon_armour(const char *s);
    int extract_mon_experience(const char *s);
    int extract_mon_speed(const char *s);
    int extract_mon_flags(const char *s);
    int extract_mon_anatomy(const char *s);
    void commit_monster(void);

    void commit_monster(void)
    {
        // to srcfile
        fprintf(srcfile, "{\n    \"%s\", \"%s\",\n", working_monster.name.c_str(), working_monster.plural.c_str());
        fprintf(srcfile, "    \"%s\",\n", working_monster.description.c_str());
        fprintf(srcfile, "    %s, %s, %s,\n", working_monster.species_string.c_str(), working_monster.sym_string.c_str(), working_monster.colour_string.c_str());
        fprintf(srcfile, "    %u, %u, %u,\n", working_monster.rarity, working_monster.threat, working_monster.health);
        fprintf(srcfile, "    { %d, %d, %d, %s, %d }, { %d, %d, %s, \"%s\", %d, %s, %d },\n",
                working_monster.melee.acc,
                working_monster.melee.dam,
                working_monster.melee.auxchance,
                working_monster.maux_string.c_str(),
                working_monster.melee.aux_strength,
                working_monster.ranged.acc,
                working_monster.ranged.dam,
                working_monster.rdam_string.c_str(),
                working_monster.shootverb.c_str(),
                working_monster.ranged.auxchance,
                working_monster.raux_string.c_str(),
                working_monster.ranged.aux_strength);
        fprintf(srcfile, "    %d, %d, %d, %s, %s,\n    %s\n", working_monster.defence, working_monster.armour, working_monster.exper,
                working_monster.speed_string.c_str(), working_monster.flag_string.c_str(), working_monster.anatomy_string.c_str());
        fprintf(srcfile, "}");

        // to header
        char *deftag = create_tag_from_name(working_monster.name.c_str(), "PM_");
        fprintf(header, "    %s", deftag);
        working_monster.flag_string = "0";
        working_monster.ranged.acc = -1;
        working_monster.ranged.dam = -1;
    }

    int extract_mon_names(const char *s)
    {
        if (working_monster.live)
        {
            commit_monster();
            fputs(",\n", srcfile);
            fputs(",\n", header);
        }
        char name[128];
        char plural[128];
        int ssfret;
        ssfret = sscanf(s, " \"%127[^\"]\" \"%127[^\"]\"", name, plural);
        if (ssfret != 2)
        {
            fprintf(stderr, "srcgen: short or ill-formed monster name at line %d\n", line_number);
            return -1;
        }
        working_monster.name = name;
        working_monster.plural = plural;
        working_monster.live = true;
        working_monster.melee.acc = 0;
        working_monster.melee.dam = 0;
        working_monster.ranged.acc = -1;
        working_monster.ranged.dam = -1;
        working_monster.species_string = "Species_own";
        working_monster.maux_string = "Maux_none";
        working_monster.raux_string = "Raux_none";
        working_monster.rdam_string = "DT_PHYS";
        working_monster.speed_string = "SPEED_NORMAL";
        working_monster.shootverb = "";
        return 0;
    }

    int extract_mon_desc(const char *s)
    {
        char tmpdesc[160];
        int ssfret;
        ssfret = sscanf(s, " %159[^\n]", tmpdesc);
        if (ssfret == 1)
        {
            tmpdesc[159] = '\0';
            working_monster.description = tmpdesc;
        }
        else
        {
            working_monster.description = "Something indescribable. (the author of your monster database is a moron)";
        }
        return 0;
    }
    int extract_mon_species(const char *s)
    {
        char spe[128];
        int ssfret;
        ssfret = sscanf(s, " %127s", spe);
        if (ssfret != 1)
        {
            fprintf(stderr, "srcgen: short or ill-formed species data at line %d\n", line_number);
        }
        working_monster.species_string = "Species_";
        working_monster.species_string += spe;
        return 0;
    }

    int extract_mon_symbol(const char *s)
    {
        char sym[128];
        int ssfret;
        ssfret = sscanf(s, " %127s", sym);
        working_monster.sym_string = "PMSYM_";
        working_monster.sym_string += sym;
        return 0;
    }

    int extract_mon_colour(const char *s)
    {
        char clr[128];
        int ssfret;
        ssfret = sscanf(s, " %127s", clr);
        working_monster.colour_string = "DBCLR_";
        working_monster.colour_string += clr;
        return 0;
    }

    int extract_mon_rarity(const char *s)
    {
        unsigned n = 100;
        sscanf(s, " %u", &n);
        working_monster.rarity = n;
        return 0;
    }

    int extract_mon_threat(const char *s)
    {
        unsigned threat = 0;
        sscanf(s, " %u", &threat);
        working_monster.threat = threat;
        return 0;
    }

    int extract_mon_hp(const char *s)
    {
        unsigned hp = 0;
        sscanf(s, " %u", &hp);
        working_monster.health = hp;
        return 0;
    }

    int extract_mon_melee(const char *s)
    {
        int dam = 0;
        int acc = 0;
        char tmpaux[128] = "none";
        int auxchance = 0;
        int auxpow = 0;
        tmpaux[0] = '\0';
        sscanf(s, " %d %d %d %127s %d", &acc, &dam, &auxchance, tmpaux, &auxpow);
        working_monster.melee.acc = acc;
        working_monster.melee.dam = dam;
        working_monster.melee.auxchance = auxchance;
        working_monster.melee.aux_strength = auxpow;
        working_monster.maux_string = "Maux_";
        working_monster.maux_string += tmpaux;
        return 0;
    }

    int extract_mon_ranged(const char *s)
    {
        int dam = 0;
        int acc = 0;
        char tmpverb[128] = "";
        char tmpaux[128] = "none";
        char tmpdtyp[128] = "PHYS";
        int auxchance = 0;
        int auxpow = 0;
        sscanf(s, " %d %d %127s %d \"%127[^\"]\" %127s %d", &acc, &dam, tmpdtyp, &auxchance, tmpverb, tmpaux, &auxpow);
        working_monster.ranged.acc = acc;
        working_monster.ranged.dam = dam;
        working_monster.ranged.auxchance = auxchance;
        working_monster.ranged.aux_strength = auxpow;
        working_monster.rdam_string = "DT_";
        working_monster.rdam_string += tmpdtyp;
        working_monster.raux_string = "Raux_";
        working_monster.raux_string += tmpaux;
        working_monster.shootverb = tmpverb;
        return 0;
    }

    int extract_mon_defence(const char *s)
    {
        unsigned def = 0;
        sscanf(s, " %u", &def);
        working_monster.defence = def;
        return 0;
    }

    int extract_mon_armour(const char *s)
    {
        unsigned armr = 0;
        sscanf(s, " %u", &armr);
        working_monster.armour = armr;
        return 0;
    }

    int extract_mon_experience(const char *s)
    {
        unsigned exper = 0;
        sscanf(s, " %u", &exper);
        working_monster.exper = exper;
        return 0;
    }

    int extract_mon_speed(const char *s)
    {
        char spd[128];
        int ssfret;
        ssfret = sscanf(s, " %127s", spd);
        working_monster.speed_string = "SPEED_";
        working_monster.speed_string += spd;
        return 0;
    }

    int extract_mon_anatomy(const char *s)
    {
        char tmpbuf[128];
        bool done = false;
        int i;
        while ((*s) && isspace(*s))
        {
            ++s;
        }
        /* We're allowed arbitrarily many body parts in principle, but the
         * code is currently built on the assumption that we have at most 32
         * possible body parts. Organs can wait for the hypothetical gorebash
         * / hentaibash forks. */
        working_monster.anatomy_string = "0";
        do
        {
            i = 0;
            while ((*s) && isascii(*s) && !isspace(*s))
            {
                if (i < 127)
                {
                    tmpbuf[i++] = *s;
                }
                ++s;
            }
            if (i)
            {
                tmpbuf[std::min(i, 127)] = '\0';
                working_monster.anatomy_string += " | (1 << Anat_";
                working_monster.anatomy_string += tmpbuf;
                working_monster.anatomy_string += ")";
            }
            else
            {
                done = true;
            }
            while ((*s) && isascii(*s) && isspace(*s))
            {
                ++s;
            }
            if ((*s == '\n') || !*s)
            {
                done = true;
            }
        } while (!done);
        return 0;
    }

    int extract_mon_flags(const char *s)
    {
        char tmpbuf[128];
        bool done = false;
        int i;
        while ((*s) && isspace(*s))
        {
            ++s;
        }
        /* We're allowed arbitrarily many flags in principle. */
        working_monster.flag_string = "0";
        do
        {
            i = 0;
            while ((*s) && isascii(*s) && !isspace(*s))
            {
                if (i < 127)
                {
                    tmpbuf[i++] = *s;
                }
                ++s;
            }
            if (i)
            {
                tmpbuf[std::min(i, 127)] = '\0';
                working_monster.flag_string += " | ";
                char *tmpstr = create_tag_from_name(tmpbuf, "PMF_");
                working_monster.flag_string += tmpstr;
            }
            else
            {
                done = true;
            }
            while ((*s) && isascii(*s) && isspace(*s))
            {
                ++s;
            }
            if ((*s == '\n') || !*s)
            {
                done = true;
            }
        } while (!done);
        return 0;
    }

    struct Pmparse_action
    {
        const char *tag;
        Pmparse_func func;
    };

    Pmparse_action actions[] =
    {
        { "mon", extract_mon_names },
        { "species", extract_mon_species },
        { "desc", extract_mon_desc },
        { "symbol", extract_mon_symbol },
        { "colour", extract_mon_colour },
        { "rarity", extract_mon_rarity },
        { "threat", extract_mon_threat },
        { "health", extract_mon_hp },
        { "melee", extract_mon_melee },
        { "ranged", extract_mon_ranged },
        { "defence", extract_mon_defence },
        { "armour", extract_mon_armour },
        { "experience", extract_mon_experience },
        { "speed", extract_mon_speed },
        { "flags", extract_mon_flags },
        { "anatomy", extract_mon_anatomy },
        { 0, 0 }
    };

}

int parse_permons(const char *fname)
{
    FILE *data = fopen(fname, "r");
    if (!data)
    {
        perror("parse_permons: couldn't open input file for write");
        exit(3);
    }
    char buffer[512];
    bool done = false;
    char *fgret;
    int i;
    srcfile = fopen("permons.cc", "w");
    if (!srcfile)
    {
        perror("parse_permons: couldn't open permons.cc for write");
        exit(4);
    }
    header = fopen("pmonid.hh", "w");
    if (!header)
    {
        perror("parse_permons: couldn't open pmonid.hh for write");
        exit(4);
    }
    fprintf(srcfile, "/* This file is automatically generated and is not the preferred form for making\n * modifications to the work. */\n\n");
    fprintf(srcfile, "#define PERMONS_CC\n#include \"pmon1.hh\"\nPermon permons[] =\n{\n");
    fprintf(header, "/* This file is automatically generated and is not the preferred form for making\n * modifications to the work. */\n\n");
    fprintf(header, "#ifndef PMONID_HH\n#define PMONID_HH\n\nenum Pmon_num {\n");
    do
    {
        fgret = fgets(buffer, 512, data);
        if (fgret)
        {
            if (!strcasecmp(buffer, "ENDMONS"))
            {
                done = true;
            }
            else if (buffer[0] == '#')
            {
                continue;
            }
            else
            {
                for (i = 0; actions[i].tag; ++i)
                {
                    int l = strlen(actions[i].tag);
                    if (!strncasecmp(buffer, actions[i].tag, l))
                    {
                        char const *bufptr = buffer;
                        while (*bufptr && !isspace(*bufptr))
                        {
                            ++bufptr;
                        }
                        if (*bufptr)
                        {
                            actions[i].func(bufptr);
                        }
                        break;
                    }
                }
            }
        }
        else if (feof(data))
        {
            // We ought to have a terminator statement, but we'll let its
            // absence go.
            done = true;
        }
        else
        {
            exit(2);
        }
    } while (!done);
    if (working_monster.live)
    {
        commit_monster();
    }
    else
    {
        fprintf(stderr, "srcgen: empty permons file '%s'.\n", fname);
        exit(3);
    }
    fprintf(srcfile, "\n};\nconst int PM_COUNT = (sizeof permons) / (sizeof permons[0]);\n");
    fprintf(header, "\n};\n#endif\n");
    fclose(srcfile);
    fclose(header);
    return 0;
}

/* pmparse.cc */
