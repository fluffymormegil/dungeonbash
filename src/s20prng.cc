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

namespace libmrl
{
    void restore_s20prng(FILE *fp, S20prng *buf)
    {
        /* Read in a saved Salsa20 PRNG state */
    }

    void init_new_s20prng(S20prng *buf, FILE *keyfile)
    {
        /* words 0-7: The keyfile is a plaintext file holding a 256-bit key.
         * How the keyfile is generated is yet to be decided. */
        /* words 8-9: The stream position is stored in a 64-bit counter in the
         * object. */
        /* words 10-11: We generate the nonce from time(), getpid() */
        struct timeval tv;
        gettimeofday(&tv);
        uint32_t nonce[2];
        nonce[0] = tv.tv_sec;
        nonce[1] = tv.tv_usec & 0xffff;
        nonce[1] |= (getpid() & 0xffff) << 16;
        /* words 12-15: We load these with a slice of pi. */
    }
}

/* s20prng.cc */
