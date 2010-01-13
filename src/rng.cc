/* rng.cc
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
#include "hexpi.hh"
#include <time.h>
#include <unistd.h>

uint32_t rng_state[5];
uint32_t saved_state[5];

// The xorshift PRNG is not cryptography-grade; its merits are "it's fast",
// "George Marsaglia has done the analysis to confirm its basic soundness" and
// "the code is comprehensible". If you can point to a cryptography-grade PRNG
// that is ubiquitous on Linux, BSD Unix, MinGW or Cygwin, and Solaris 10, let
// me know and I'll ditch this thing.
//
// TODO reread George Marsaglia's paper on the subject to see if he comments
// on _precisely_ how cryptoweak it is.
uint32_t rng(void)
{
    uint32_t tmp;

    tmp = rng_state[0] ^ (rng_state[0] >> 7);
    rng_state[0] = rng_state[1];
    rng_state[1] = rng_state[2];
    rng_state[2] = rng_state[3];
    rng_state[3] = rng_state[4];
    rng_state[4] = (rng_state[4] ^ (rng_state[4] << 6)) ^ (tmp ^ (tmp << 13));
    return (rng_state[2] + rng_state[2] + 1) * rng_state[4];
}

void rng_init(void)
{
    // We don't use a full 160 bits of initial entropy, unfortunately.
    int i;
    uint32_t t1;
    uint32_t t2;
    uint32_t t3;
#ifdef GETTIMEOFDAY_EXISTS
    struct timeval tv;
    gettimeofday(&tv);
    t1 = tv.sec;
    t3 = tv.usec;
#else
    t1 = uint32_t(time(NULL));
    t3 = 0;
#endif
#ifdef GETPID_AND_GETUID_EXIST
    t2 = (getpid() & 0xffff) | (getuid() << 16);
#else
    t2 = 0x1ed2c3b4;
#endif
    rng_state[0] = hex_pi[0];
    rng_state[1] = hex_pi[1];
    rng_state[2] = t3 ^ hex_pi[2];
    rng_state[3] = t1 ^ hex_pi[3];
    rng_state[4] = t2 ^ hex_pi[4];
    /* Flush through the first 100 numbers */
    for (i = 0; i < 100; i++)
    {
        rng();
    }
}

/* rng.cc */
