/* loadsave.hh
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

#ifndef LOADSAVE_HH
#define LOADSAVE_HH

#include <stdio.h>
#include <stdint.h>
#include <errno.h>

inline void checked_fwrite(void *buf, int sz, int count, FILE *fp)
{
    int i = fwrite(buf, sz, count, fp);
    if (i != count)
    {
        throw(errno);
    }
}

inline void checked_fread(void *buf, int sz, int count, FILE *fp)
{
    int i = fread(buf, sz, count, fp);
    if (i != count)
    {
        throw(errno);
    }
}

inline void serialise(FILE *fp, uint16_t data)
{
    unsigned char buf[2];
    buf[0] = data >> 8;
    buf[1] = data;
    checked_fwrite(buf, 1, 2, fp);
}

inline void serialise(FILE *fp, uint64_t data)
{
    unsigned char buf[8];
    buf[0] = data >> 56;
    buf[1] = data >> 48;
    buf[2] = data >> 40;
    buf[3] = data >> 32;
    buf[4] = data >> 24;
    buf[5] = data >> 16;
    buf[6] = data >> 8;
    buf[7] = data;
    checked_fwrite(buf, 1, 8, fp);
}

inline void serialise(FILE *fp, uint32_t data)
{
    unsigned char buf[4];
    buf[0] = data >> 24;
    buf[1] = data >> 16;
    buf[2] = data >> 8;
    buf[3] = data;
    checked_fwrite(buf, 1, 4, fp);
}

inline void serialise(FILE *fp, uint32_t const *buf, int len)
{
    int i;
    for (i = 0; i < len; ++i)
    {
        serialise(fp, buf[i]);
    }
}

inline void serialise_ohandle(FILE *fp, Obj_handle foo)
{
    serialise(fp, uint64_t(foo.value));
}

extern void serialise(FILE *fp, Level const *lp);
extern void serialise(FILE *fp, libmrl::Coord c);
extern void serialise(FILE *fp, Obj const *optr);
extern void serialise(FILE *fp, Mon const *mptr);
extern void serialise_ohandle_array(FILE *fp, Obj_handle const *array, int count);
extern void serialise_monsters(FILE *fp);
extern void serialise_objects(FILE *fp);
extern void serialise_permobj_vars(FILE *fp);

inline uint16_t deserialise_uint16(FILE *fp)
{
    uint16_t tmp;
    unsigned char buf[2];
    checked_fread(buf, 1, 2, fp);
    tmp = (buf[0] & 0xff) << 8;
    tmp |= buf[1] & 0xff;
    return tmp;
}

inline uint32_t deserialise_uint32(FILE *fp)
{
    uint32_t tmp;
    char buf[4];
    checked_fread(buf, 1, 4, fp);
    tmp = (buf[0] & 0xff) << 24;
    tmp |= (buf[1] & 0xff) << 16;
    tmp |= (buf[2] & 0xff) << 8;
    tmp |= (buf[3] & 0xff);
    return tmp;
}

inline uint64_t deserialise_uint64(FILE *fp)
{
    uint32_t tmp;
    char buf[8];
    checked_fread(buf, 1, 8, fp);
    tmp = (buf[0] & 0xffull) << 56;
    tmp |= (buf[1] & 0xffull) << 48;
    tmp |= (buf[2] & 0xffull) << 40;
    tmp |= (buf[3] & 0xffull) << 32;
    tmp |= (buf[4] & 0xffull) << 24;
    tmp |= (buf[5] & 0xffull) << 16;
    tmp |= (buf[6] & 0xffull) << 8;
    tmp |= (buf[7] & 0xffull);
    return tmp;
}

inline void deserialise(FILE *fp, uint32_t *buf, int len)
{
    int i;
    for (i = 0; i < len; ++i)
    {
        buf[i] = deserialise_uint32(fp);
    }
}

extern void deserialise(FILE *fp, Obj *optr);
extern void deserialise(FILE *fp, Mon *mptr);
extern void deserialise(FILE *fp, libmrl::Coord *ppos);
extern void deserialise(FILE *fp, Level_tag *ptag);
extern void deserialise_ohandle_array(FILE *fp, Obj_handle *array, int count);
extern Level *deserialise_level(FILE *fp, Level_tag lt);
extern void deserialise_monsters(FILE *fp);
extern void deserialise_objects(FILE *fp);
extern void deserialise_permobj_vars(FILE *fp);

extern int save_game(void);
extern int load_game(void);
extern void kill_game(void);

#endif

/* dunbash.hh */
