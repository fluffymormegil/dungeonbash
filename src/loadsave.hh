// loadsave.hh
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

#ifndef LOADSAVE_HH
#define LOADSAVE_HH

#include <stdio.h>
#include <stdint.h>
#include <errno.h>

#include <libmormegil/serial.hh>

using libmormegil::serialize;
using libmormegil::deserialize;
using libmormegil::checked_fwrite;
using libmormegil::checked_fread;

inline void serialize(FILE *fp, uint32_t const *buf, int len)
{
    int i;
    for (i = 0; i < len; ++i)
    {
        serialize(fp, buf[i]);
    }
}

inline void serialize_ohandle(FILE *fp, Obj_handle foo)
{
    serialize(fp, uint64_t(foo.value));
}

extern void serialize(FILE *fp, Level const *lp);
extern void serialize(FILE *fp, Obj const *optr);
extern void serialize(FILE *fp, Mon const *mptr);
extern void serialize_ohandle_array(FILE *fp, Obj_handle const *array, int count);
extern void serialize_monsters(FILE *fp);
extern void serialize_objects(FILE *fp);
extern void serialize_permobj_vars(FILE *fp);

inline void deserialize(FILE *fp, uint32_t *buf, int len)
{
    int i;
    for (i = 0; i < len; ++i)
    {
        deserialize(fp, buf + i);
    }
}

extern void deserialize(FILE *fp, Obj *optr);
extern void deserialize(FILE *fp, Mon *mptr);
extern void deserialize(FILE *fp, Level_tag *ptag);
extern void deserialize_ohandle_array(FILE *fp, Obj_handle *array, int count);
extern Level *deserialize_level(FILE *fp, Level_tag lt);
extern void deserialize_monsters(FILE *fp);
extern void deserialize_objects(FILE *fp);
extern void deserialize_permobj_vars(FILE *fp);

extern int save_game(void);
extern int load_game(void);
extern void kill_game(void);

#endif

// loadsave.hh
