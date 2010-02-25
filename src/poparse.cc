/* poparse.cc - permobj DB parser for Martin's Dungeon Bash
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

#define POPARSE_CC

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <ctype.h>
#include <errno.h>
#include "pobj1.hh"
#include "srcgen.hh"

struct Poparse_action;
struct Poparse_state;
struct Flag_table;

// There's only one state object, which we recycle.
typedef int (*Poparse_func)(const char *);

struct Poparse_state
{
    bool live;
    std::string name;
    std::string plural;
    std::string description;
    std::string pocl_string;
    unsigned rarity;
    std::string qual_string;
    std::string sym_string;
    std::string colour_string;
    unsigned power;
    std::string known_string;
    unsigned depth;
    std::string flag_string;
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
    int extract_obj_names(const char *s);
    int extract_obj_desc(const char *s);
    int extract_obj_symbol(const char *s);
    int extract_obj_quality(const char *s);
    int extract_obj_colour(const char *s);
    int extract_obj_rarity(const char *s);
    int extract_obj_power(const char *s);
    int extract_obj_known(const char *s);
    int extract_obj_depth(const char *s);
    int extract_obj_flags(const char *s);
    void commit_object(void);
    struct Poparse_action
    {
        const char *tag;
        Poparse_func func;
    };

    Poparse_action actions[] =
    {
        { "obj", extract_obj_names },
        { "desc", extract_obj_desc },
        { "rarity", extract_obj_rarity },
        { "symbol", extract_obj_symbol },
        { "quality", extract_obj_quality },
        { "colour", extract_obj_colour },
        { "power", extract_obj_power },
        { "known", extract_obj_known },
        { "depth", extract_obj_depth },
        { "flags", extract_obj_flags },
        { 0, 0 }
    };

    FILE *srcfile;
    FILE *header;
    int line_number;
    int first_foo[NUM_OF_POCLASSES];
    int last_foo[NUM_OF_POCLASSES];
    int obj_idx;

    Poparse_state working_object;

    const char *pocltags[NUM_OF_POCLASSES] = 
    {
        "NONE", "WEAPON", "POTION", "SCROLL", "ARMOUR", "RING",
        "FOOD", "MISC", "WAND", "CARRION", "COIN"
    };


    void commit_object(void)
    {
        // to srcfile
        fprintf(srcfile, "{\n    \"%s\", \"%s\",\n", working_object.name.c_str(), working_object.plural.c_str());
        fprintf(srcfile, "    \"%s\",\n", working_object.description.c_str());
        fprintf(srcfile, "    %s, %s,\n", working_object.pocl_string.c_str(), working_object.qual_string.c_str());
        fprintf(srcfile, "    %u, %s, %s,\n", working_object.rarity, working_object.sym_string.c_str(), working_object.colour_string.c_str());
        fprintf(srcfile, "    %u, %s, %u,\n", working_object.power, working_object.known_string.c_str(), working_object.depth);
        fprintf(srcfile, "    %s,\n", working_object.flag_string.c_str());
        fprintf(srcfile, "}");
        ++obj_idx;

        // to header
        char *deftag = create_tag_from_name(working_object.name.c_str(), "PO_");
        fprintf(header, "    %s", deftag);
    }

    int extract_obj_names(const char *s)
    {
        if (working_object.live)
        {
            commit_object();
            fputs(",\n", srcfile);
            fputs(",\n", header);
            working_object.live = false;
        }
        char name[48];
        char plural[48];
        char pocl[48];
        int ssfret;
        int i;
        name[47] = '\0';
        plural[47] = '\0';
        pocl[47] = '\0';
        ssfret = sscanf(s, " %47[A-Z_] \"%47[^\"]\" \"%47[^\"]\"", pocl, name, plural);
        if (ssfret != 3)
        {
            fprintf(stderr, "srcgen: short or ill-formed object decl at line %d\n", line_number);
            return -1;
        }
        for (i = 0; i < NUM_OF_POCLASSES; ++i)
        {
            if (!strcmp(pocl, pocltags[i]))
            {
                if (first_foo[i] == -1)
                {
                    first_foo[i] = obj_idx;
                }
                last_foo[i] = obj_idx;
                break;
            }
        }
        working_object.name = name;
        working_object.plural = plural;
        working_object.description = "Something indescribable. (the author of your object database is a moron)";
        working_object.pocl_string = "POCLASS_";
        working_object.pocl_string += pocl;
        working_object.rarity = 100;
        working_object.qual_string = "Itemqual_normal";
        working_object.sym_string = "OSYM_NONE";
        working_object.known_string = "false";
        working_object.live = true;
        working_object.flag_string = "0";
        return 0;
    }

    int extract_obj_desc(const char *s)
    {
        char tmpdesc[160];
        int ssfret;
        ssfret = sscanf(s, " %159[^\n]", tmpdesc);
        if (ssfret == 1)
        {
            tmpdesc[159] = '\0';
            working_object.description = tmpdesc;
        }
        else
        {
            working_object.description = "Something indescribable. (the author of your object database is a moron)";
        }
        return 0;
    }

    int extract_obj_symbol(const char *s)
    {
        char sym[128];
        int ssfret;
        ssfret = sscanf(s, " %127s", sym);
        working_object.sym_string = "POSYM_";
        working_object.sym_string += sym;
        return 0;
    }

    int extract_obj_quality(const char *s)
    {
        char qual[128];
        int ssfret;
        ssfret = sscanf(s, " %127s", qual);
        working_object.qual_string = "Itemqual_";
        working_object.qual_string += qual;
        return 0;
    }

    int extract_obj_colour(const char *s)
    {
        char clr[128];
        int ssfret;
        ssfret = sscanf(s, " %127s", clr);
        working_object.colour_string = "DBCLR_";
        working_object.colour_string += clr;
        return 0;
    }

    int extract_obj_rarity(const char *s)
    {
        unsigned n = 100;
        sscanf(s, " %u", &n);
        working_object.rarity = n;
        return 0;
    }

    int extract_obj_power(const char *s)
    {
        unsigned power = 0;
        sscanf(s, " %u", &power);
        working_object.power = power;
        return 0;
    }

    int extract_obj_depth(const char *s)
    {
        unsigned depth = 0;
        sscanf(s, " %u", &depth);
        working_object.depth = depth;
        return 0;
    }

    int extract_obj_flags(const char *s)
    {
        char tmpbuf[128];
        bool done = false;
        int i;
        while ((*s) && isspace(*s))
        {
            ++s;
        }
        /* We're allowed arbitrarily many flags in principle. */
        working_object.flag_string = "0";
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
                tmpbuf[libmrl::min(i, 127)] = '\0';
                working_object.flag_string += " | ";
                char *tmpstr = create_tag_from_name(tmpbuf, "POF_");
                working_object.flag_string += tmpstr;
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

    int extract_obj_known(const char *s)
    {
        char spd[128];
        int ssfret;
        ssfret = sscanf(s, " %127s", spd);
        if (!strcasecmp(spd, "true") ||
            !strcasecmp(spd, "yes"))
        {
            working_object.known_string = "true";
        }
        else
        {
            working_object.known_string = "false";
        }
        return 0;
    }
}

int parse_permobj(const char *fname)
{
    FILE *data = fopen(fname, "r");
    if (!data)
    {
        fprintf(stderr, "parse_permobj: couldn't open '%s' for read: %s\n", fname, strerror(errno));
        exit(3);
    }
    char buffer[512];
    bool done = false;
    char *fgret;
    int i;
    for (i = 0; i < NUM_OF_POCLASSES; i++)
    {
        first_foo[i] = -1;
        last_foo[i] = -1;
    }
    srcfile = fopen("permobj.cc", "w");
    if (!srcfile)
    {
        perror("parse_permobj: couldn't open permobj.cc for write");
        exit(4);
    }
    header = fopen("pobjid.hh", "w");
    if (!header)
    {
        perror("parse_permobj: couldn't open pobjid.hh for write");
        exit(4);
    }
    fprintf(srcfile, "/* This file is automatically generated and is not the preferred form for making\n * modifications to the work. */\n\n");
    fprintf(srcfile, "#define PERMOBJ_CC\n#include \"pobj1.hh\"\nPermobj permobjs[] =\n{\n");
    fprintf(header, "/* This file is automatically generated and is not the preferred form for making\n * modifications to the work. */\n\n");
    fprintf(header, "#ifndef POBJID_HH\n#define POBJID_HH\n\nenum Pobj_num {\n");
    do
    {
        ++line_number;
        fgret = fgets(buffer, 512, data);
        if (fgret)
        {
            if (!strcasecmp(buffer, "ENDOBJS"))
            {
                done = true;
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
    if (working_object.live)
    {
        commit_object();
    }
    else
    {
        fprintf(stderr, "srcgen: empty permobj file '%s'.\n", fname);
        exit(3);
    }
    fprintf(srcfile, "\n};\nconst int PO_COUNT = (sizeof permobjs) / (sizeof permobjs[0]);\n");
    fprintf(header, "\n};\n\n");
    fprintf(header, "#define PO_FIRST_FOOD %d\n#define PO_LAST_FOOD %d\n",
            first_foo[POCLASS_FOOD], last_foo[POCLASS_FOOD]);
    fprintf(header, "#define PO_FIRST_ARMOUR %d\n#define PO_LAST_ARMOUR %d\n",
            first_foo[POCLASS_ARMOUR], last_foo[POCLASS_ARMOUR]);
    fprintf(header, "#define PO_FIRST_SCROLL %d\n#define PO_LAST_SCROLL %d\n",
            first_foo[POCLASS_SCROLL], last_foo[POCLASS_SCROLL]);
    fprintf(header, "#define PO_FIRST_RING %d\n#define PO_LAST_RING %d\n",
            first_foo[POCLASS_RING], last_foo[POCLASS_RING]);
    fprintf(header, "#define PO_FIRST_POTION %d\n#define PO_LAST_POTION %d\n",
            first_foo[POCLASS_POTION], last_foo[POCLASS_POTION]);
    fprintf(header, "#define PO_FIRST_MISC %d\n#define PO_LAST_MISC %d\n",
            first_foo[POCLASS_MISC], last_foo[POCLASS_MISC]);
    fprintf(header, "#define PO_FIRST_FOOD %d\n#define PO_LAST_FOOD %d\n",
            first_foo[POCLASS_FOOD], last_foo[POCLASS_FOOD]);
    fprintf(header, "#define PO_FIRST_WEAPON %d\n#define PO_LAST_WEAPON %d\n",
            first_foo[POCLASS_WEAPON], last_foo[POCLASS_WEAPON]);
    fprintf(header, "#endif\n");
    fclose(srcfile);
    fclose(header);
    return 0;
}

/* poparse.cc */
