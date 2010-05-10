/* serial.hh - libmrl serialization primitives
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

#ifndef LIBMRL_SERIAL_HH
#define LIBMRL_SERIAL_HH

#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include "coord.hh"

namespace libmrl
{
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

inline void serialise(FILE *fp, libmrl::Coord c)
{
    serialise(fp, uint32_t(c.y));
    serialise(fp, uint32_t(c.x));
}

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

inline void deserialise(FILE *fp, libmrl::Coord *c)
{
    c->y = int(deserialise_uint32(fp));
    c->x = int(deserialise_uint32(fp));
}
}

#endif

/* serial.hh */
