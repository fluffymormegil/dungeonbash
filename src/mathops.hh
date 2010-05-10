/* mathops.hh
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

#ifndef LIBMRL_MATHOPS_HH
#define LIBMRL_MATHOPS_HH

#include <stdint.h>

namespace libmrl
{
    inline uint32_t rotl(uint32_t a, uint32_t dist)
    {
        return (a << dist) | (a >> (32 - dist));
    }

    template <typename T> inline T max(const T& left, const T& right)
    {
        return (left < right) ? right : left;
    }
    template <typename T> inline T min(const T& left, const T& right)
    {
        return (left < right) ? left : right;
    }
    template <typename T> inline T abs(const T& orig)
    {
        return (orig < 0) ? -orig : orig;
    }
    template <typename T> inline T sign(const T& orig)
    {
        return (orig != 0) ? ((orig < 0) ? T(-1) : T(1)) : T(0);
    }
    /* div_up - integer divide with upward rounding and no FP conversion.
     *
     * If you want to know why I don't just invoke FP arithmetic then call
     * ceil(): It's _messy_ on RISC architectures, since they tend not to have
     * a convert-integer-and-place-in-FPR instruction. (x86 *does* have such
     * an instruction).
     */
    template <typename T> inline T div_up(T orig, T divisor)
    {
        return (orig + (divisor - 1)) / divisor;
    }
}

#endif

// mathops.hh
