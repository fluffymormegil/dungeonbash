/* s20prng.cc - Salsa20-based PRNG support for libmrl
 * 
 * Copyright 2010 Martin Read
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

#define S20PRNG_CC

#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "s20prng.hh"

void libmrl::S20prng::restore(FILE *fp, S20prng *buf)
{
    /* Read in a saved state */
    depth = deserialise_uint32(fp);
    loop = deserialise_uint32(fp);
    streampos = deserialise_uint64(fp);
    deserialise(fp, nonce, 2);
    deserialise(fp, key, 8);
    refill_state();
}

int libmrl::S20prng::initialize(void const *keybuf, void const *noncebuf)
{
    if ((!keybuf) || (!noncebuf))
    {
        errno = EINVAL;
        return -1;
    }
    memcpy(key, keybuf, 32);
    memcpy(nonce, noncebuf, 8);
    streampos = 0;
    loop = 0;
    refill_state();
    return 0;
}

void libmrl::S20prng::refill_state()
{
    /* ill the state array with the relevant numbers... */

    /* Fixed words go one in each column, one in each row. */
    state[0] = fixed0;
    state[5] = fixed1;
    state[10] = fixed2;
    state[15] = fixed3;
    /* first slice of key spans all four columns, and touches two rows. */
    state[1] = key[0];
    state[2] = key[1];
    state[3] = key[2];
    state[4] = key[3];
    /* second slice of keye spans all four columns, and touches two rows. */
    state[11] = key[4];
    state[12] = key[5];
    state[13] = key[6];
    state[14] = key[7];
    /* Streampos and nonce go in middle of array. We don't have to worry about
     * Bernstein's caution regarding nonce renewal, since we cannot ever
     * feasibly use all 2^68 32-bit outputs of the generator. */
    state[6] = uint32_t(streampos);
    state[7] = uint32_t(streampos >> 32);
    state[8] = nonce[0];
    state[9] = nonce[1];
    /* ... and perform the requested number oof roundds of Salsa20 on them. */
    slash(depth);
}

/* s20prng.cc */
