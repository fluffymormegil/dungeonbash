/* cfgfile.cc - configuration file processor
 * 
 * Copyright 2005-20099 Martin Read
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
#include "cfgfile.hh"

#include <vector>
#include <string>
#include <map>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#define CFGFILE_DEBUG
struct Bool_config_var
{
    const char *name;
    bool *destptr;
    bool unconf;
};

struct Int_config_var
{
    const char *name;
    int *destptr;
    int unconf;
};

struct String_config_var
{
    const char *name;
    std::string *destptr;
    const char *unconf;
};

#ifdef MULTIUSER
Bool_config_var bool_syscfg_vars[] =
{
    { "always_fsync", &always_fsync, false },
    { 0, 0, 0 }
};

String_config_var string_syscfg_vars[] =
{
    { "system_playground", &configured_system_playground, PLAYGROUND },
    { 0, 0, 0 }
};
#endif

String_config_var string_config_vars[] =
{
    { "name", &configured_name, "Ingold" },
    { "language", &configured_language, "en" },
    { "preferred_display", &preferred_display, COMPILED_DEFAULT_DISPLAY },
    { 0, 0, 0 }
};

Int_config_var int_config_vars[] =
{
    { "run_delay", &run_delay, 40 },
    { "projectile_delay", &projectile_delay, 40 },
    { 0, 0, 0 }
};

Bool_config_var bool_config_vars[] =
{
    { "suppress_boringfail", suppressions + MSGCHAN_BORINGFAIL, false },
    { "suppress_fluff", suppressions + MSGCHAN_FLUFF, false },
    { "suppress_mon_alert", suppressions + MSGCHAN_MON_ALERT, false },
    { "suppress_numeric", suppressions + MSGCHAN_NUMERIC, false },
    { "suppress_taunt", suppressions + MSGCHAN_TAUNT, false },
    { "name_prompt", &name_prompt, true },
    { "levelup_wait", &levelup_wait, true },
    { "save_wait", &save_wait, true },
    { "reload_wait", &reload_wait, false },
    { "inventory_colours", &fruit_salad_inventory, false },
    { 0, 0, false }
};

/* TODO - find someone who groks Windows to tell me the canonical way to
 * find the user's home directory. */

#ifdef MULTIUSER
static char system_cfgfile_name[1024];
static char system_cfgdir_name[1024];
#endif

static char user_cfgfile_name[1024];
static char user_cfgdir_name[1024];

void build_cfg_names()
{
    const char *eptr;
    /* The following should work on any UNIX-like system that isn't
     * unutterably ancient. I don't know how to canonically do the equivalent
     * operation on Windows. */
    eptr = getenv("HOME");
    if (!eptr)
    {
        eptr = ".";
    }
    snprintf(user_cfgdir_name, 1024, "%s", eptr);
    snprintf(user_cfgfile_name, 1024, "%s/%s", user_cfgdir_name, ".dunbashrc");
#ifdef MULTIUSER
    snprintf(system_cfgdir_name, 1024, "%s", GLOBALCFGDIR);
    snprintf(system_cfgfile_name, 1024, "%s/%s", system_cfgdir_name, "config");
#endif
}

bool string_to_bool(std::string& s)
{
    const char *cs = s.c_str();
    if (!strcasecmp(cs, "yes") || !strcasecmp(cs, "true") || !strcasecmp(cs, "on"))
    {
        return true;
    }
    else if (!strcasecmp(cs, "no") || !strcasecmp(cs, "false") || !strcasecmp(cs, "off"))
    {
        return false;
    }
#ifdef CFGFILE_DEBUG
    fprintf(stderr, "string_to_bool: inappropriate option value '%s'\n", cs);
#endif
    return false;
}

int cfgfile_to_stringvec(const char *file, std::map<std::string, std::string> *pmap)
{
    int fd;
    int i;
    char *buf;
    char *leftside;
    char *rightside;
    fd = open(file, O_RDONLY, 0);
    if (fd != -1)
    {
#ifdef CFGFILE_DEBUG
        fprintf(stderr, "cfgfile_to_stringvec: opened %s\n", file);
#endif
        struct stat st;
        i = fstat(fd, &st);
        if (i >= 0)
        {
            if (st.st_size)
            {
#ifdef CFGFILE_DEBUG
                fprintf(stderr, "cfgfile_to_stringvec: parsing\n");
#endif
                FILE *fp = fdopen(fd, "r");
                buf = (char *) malloc(1024);
                leftside = (char *) malloc(512);
                rightside = (char *) malloc(512);
                do
                {
                    char *s;
                    s = fgets(buf, 1024, fp);
                    if (!s)
                    {
                        break;
                    }
                    i = sscanf(buf, " %511[^ \t\n=] = %511[^\n]", leftside, rightside);
                    if (i == 2)
                    {
                        std::string l(leftside);
                        std::string r(rightside);
                        (*pmap)[l] = r;
#ifdef CFGFILE_DEBUG
                        fprintf(stderr, "cfgfile_to_stringvec: %s = %s\n", leftside, rightside);
#endif
                    }
                    else
                    {
#ifdef CFGFILE_DEBUG
                        fprintf(stderr, "cfgfile_to_stringvec: unexpected linefmt %s\n", leftside);
#endif
                    }
                } while (1);
                free(buf);
                free(leftside);
                free(rightside);
            }
            else
            {
#ifdef CFGFILE_DEBUG
                fprintf(stderr, "cfgfile_to_stringvec: empty\n");
#endif
            }
        }
        close(fd);
        return i;
    }
    else
    {
        // The file not being there is not really an error condition.
#ifdef CFGFILE_DEBUG
        fprintf(stderr, "cfgfile_to_stringvec: couldn't open %s\n", file);
#endif
        return (errno == ENOENT) ? 0 : -1;
    }
}

void load_user_config()
{
    std::map<std::string, std::string> configlines;
    int i;
    int j;
#ifdef MULTIUSER
    user_permissions();
#endif
    i = cfgfile_to_stringvec(user_cfgfile_name, &configlines);
#ifdef MULTIUSER
    game_permissions();
#endif
    if (i >= 0)
    {
        std::string tag;
        for (j = 0; string_config_vars[j].name; ++j)
        {
            std::map<std::string, std::string>::iterator iter;
            tag = string_config_vars[j].name;
            iter = configlines.find(tag);
            if (iter != configlines.end())
            {
                *(string_config_vars[j].destptr) = iter->second;
            }
            else
            {
                // use unconfigured value
                *(string_config_vars[j].destptr) = string_config_vars[j].unconf;
            }
        }
        for (j = 0; int_config_vars[j].name; ++j)
        {
            std::map<std::string, std::string>::iterator iter;
            tag = int_config_vars[j].name;
            iter = configlines.find(tag);
            if (iter != configlines.end())
            {
                const char *cc = iter->second.c_str();
                sscanf(cc, "%d", int_config_vars[j].destptr);
            }
            else
            {
                *(int_config_vars[j].destptr) = int_config_vars[j].unconf;
            }
        }
        for (j = 0; bool_config_vars[j].name; ++j)
        {
            std::map<std::string, std::string>::iterator iter;
            tag = bool_config_vars[j].name;
            iter = configlines.find(tag);
            if (iter != configlines.end())
            {
                *(bool_config_vars[j].destptr) = string_to_bool(iter->second);
            }
            else
            {
                *(bool_config_vars[j].destptr) = bool_config_vars[j].unconf;
            }
        }
    }
    else
    {
        perror("duungeonbash: Couldn't load user configuration file");
    }
}

#ifdef MULTIUSER
void load_system_config()
{
    std::map<std::string, std::string> configlines;
    int i;
    int j;
    i = cfgfile_to_stringvec(system_cfgfile_name, &configlines);
    if (i == 0)
    {
        std::string tag;
        for (j = 0; string_syscfg_vars[j].name; ++j)
        {
            std::map<std::string, std::string>::iterator iter;
            tag = string_syscfg_vars[j].name;
            iter = configlines.find(tag);
            if (iter != configlines.end())
            {
                *(string_syscfg_vars[j].destptr) = iter->second;
            }
            else
            {
                // use unconfigured value
                *(string_syscfg_vars[j].destptr) = string_syscfg_vars[j].unconf;
            }
        }
        for (j = 0; bool_syscfg_vars[j].name; ++j)
        {
            std::map<std::string, std::string>::iterator iter;
            tag = bool_syscfg_vars[j].name;
            iter = configlines.find(tag);
            if (iter != configlines.end())
            {
                *(bool_syscfg_vars[j].destptr) = string_to_bool(iter->second);
            }
            else
            {
                *(bool_syscfg_vars[j].destptr) = bool_syscfg_vars[j].unconf;
            }
        }
    }
    else
    {
        perror("dungeonbash: Couldn't load system configuration file");
    }
}
#endif

void get_config()
{
    build_cfg_names();
#ifdef MULTIUSER
    int i;
    load_system_config();
    i = chdir(configured_system_playground.c_str());
    if (i != 0)
    {
        fprintf(stderr, "dungeonbash: couldn't enter playground %s:", configured_system_playground.c_str(), strerror(errno));
        exit(1);
    }
#endif
    load_user_config();
}

/* cfgfile.cc */
