/* s20prng.cc - PRNG using Salsa20 for "libmrl"
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

#ifndef S20PRNG_HH
#define S20PRNG_HH

#include <stdio.h>
#include <stdint.h>
#include "serial.hh"
#include "mathops.hh"

namespace libmrl
{
    /* An S20prng object holds the state of a Salsa20 stream cipher
     * implementation. */
    class S20prng
    {
    private:
        uint32_t state[16];
        uint32_t nonce[2];
        uint32_t key[8];
        uint64_t streampos;
        uint32_t loop;
        uint32_t depth;
    public:
        static const uint32_t fixed0 = 0x4a409382;
        static const uint32_t fixed1 = 0x2299f31d;
        static const uint32_t fixed2 = 0x0082efa9;
        static const uint32_t fixed3 = 0x8ec4e6c8;
        S20prng() : streampos(0), loop(0), depth(12) { }
        /* if performance issues crop up, I'll break out the assembly-language
         * versions. */
        void doubleround()
        {
            state[ 4] ^= rotl(state[ 0] + state[12], 7);
            state[ 8] ^= rotl(state[ 4] + state[ 0], 9);
            state[12] ^= rotl(state[ 8] + state[ 4], 13);
            state[ 0] ^= rotl(state[12] + state[ 8], 18);
            state[ 9] ^= rotl(state[ 5] + state[ 1], 7);
            state[13] ^= rotl(state[ 9] + state[ 5], 9);
            state[ 1] ^= rotl(state[13] + state[ 9], 13);
            state[ 5] ^= rotl(state[ 1] + state[13], 18);
            state[14] ^= rotl(state[10] + state[ 6], 7);
            state[ 2] ^= rotl(state[14] + state[10], 9);
            state[ 6] ^= rotl(state[ 2] + state[14], 13);
            state[10] ^= rotl(state[ 6] + state[ 2], 18);
            state[ 3] ^= rotl(state[15] + state[11], 7);
            state[ 7] ^= rotl(state[ 3] + state[15], 9);
            state[11] ^= rotl(state[ 7] + state[ 3], 13);
            state[15] ^= rotl(state[11] + state[ 7], 18);

            state[ 1] ^= rotl(state[ 0] + state[ 3], 7);
            state[ 2] ^= rotl(state[ 1] + state[ 0], 9);
            state[ 3] ^= rotl(state[ 2] + state[ 1], 13);
            state[ 0] ^= rotl(state[ 3] + state[ 2], 18);
            state[ 6] ^= rotl(state[ 5] + state[ 4], 7);
            state[ 7] ^= rotl(state[ 6] + state[ 5], 9);
            state[ 4] ^= rotl(state[ 7] + state[ 6], 13);
            state[ 5] ^= rotl(state[ 4] + state[ 7], 18);
            state[11] ^= rotl(state[10] + state[ 9], 7);
            state[ 8] ^= rotl(state[11] + state[10], 9);
            state[ 9] ^= rotl(state[ 8] + state[11], 13);
            state[10] ^= rotl(state[ 9] + state[ 8], 18);
            state[12] ^= rotl(state[15] + state[14], 7);
            state[13] ^= rotl(state[12] + state[15], 9);
            state[14] ^= rotl(state[13] + state[12], 13);
            state[15] ^= rotl(state[14] + state[13], 18);
        }
        void slash(int n)
        {
            for (int i = 0; i < (n >> 1); ++i)
            {
                doubleround();
            }
        }
        int initialize(void const *keybuf, void const *noncebuf);
        void refill_state();
        uint32_t get_num()
        {
            uint32_t val = state[loop];
            ++loop;
            loop &= 15;
            if (loop == 0)
            {
                ++streampos;
                refill_state();
            }
            return val;
        }
        void restore(FILE *fp, S20prng *buf);
    };
}
#endif

/* s20prng.hh */
