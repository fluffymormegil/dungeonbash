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

namespace libmrl
{
    /* An S20prng object holds the state of a Salsa20 stream cipher
     * implementation. */
    struct S20prng
    {
        uint32_t state[16];
        uint64_t counter;
        void doubleround()
        {
            state[ 4] ^= (state[ 0]+state[12])<<7;
            state[ 9] ^= (state[ 5]+state[ 1])<<7;
            state[14] ^= (state[10]+state[ 6])<<7;
            state[ 3] ^= (state[15]+state[11])<<7;
            state[ 8] ^= (state[ 4]+state[ 0])<<9;
            state[13] ^= (state[ 9]+state[ 5])<<9;
            state[ 2] ^= (state[14]+state[10])<<9;
            state[ 7] ^= (state[ 3]+state[15])<<9;
            state[12] ^= (state[ 8]+state[ 4])<<13;
            state[ 1] ^= (state[13]+state[ 9])<<13;
            state[ 6] ^= (state[ 2]+state[14])<<13;
            state[11] ^= (state[ 7]+state[ 3])<<13;
            state[ 0] ^= (state[12]+state[ 8])<<18;
            state[ 5] ^= (state[ 1]+state[13])<<18;
            state[10] ^= (state[ 6]+state[ 2])<<18;
            state[15] ^= (state[11]+state[ 7])<<18;

            state[ 1] ^= (state[ 0]+state[ 3])<<7;
            state[ 6] ^= (state[ 5]+state[ 3])<<7;
            state[11] ^= (state[10]+state[ 9])<<7;
            state[12] ^= (state[15]+state[14])<<7;
            state[ 2] ^= (state[ 1]+state[ 0])<<9;
            state[ 7] ^= (state[ 6]+state[ 5])<<9;
            state[ 8] ^= (state[11]+state[10])<<9;
            state[13] ^= (state[12]+state[15])<<9;
            state[ 3] ^= (state[ 2]+state[ 1])<<13;
            state[ 4] ^= (state[ 7]+state[ 6])<<13;
            state[ 9] ^= (state[ 8]+state[11])<<13;
            state[14] ^= (state[13]+state[12])<<13;
            state[ 0] ^= (state[ 3]+state[ 2])<<18;
            state[ 5] ^= (state[ 4]+state[ 7])<<18;
            state[10] ^= (state[ 9]+state[ 8])<<18;
            state[15] ^= (state[14]+state[13])<<18;
        }
        void slash(int n)
        {
            for (int i = 0; i < (n >> 1); ++i)
            {
                doubleround();
            }
        }
    };
}
#endif

/* s20prng.hh */
