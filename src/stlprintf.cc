/* stlprintf.cc - printf-for-STL-strings for Martin's Dungeon Bash
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

#define STLPRINTF_CC

#include <stdarg.h>
#include <string>

std::string vstlprintf(const char *fmt, va_list args)
{
    va_list ap;
    int bsize = 80;

    do
    {
        char* buf = new char[bsize];

        va_copy(ap, args);
        int ret = vsnprintf(buf, bsize, fmt, ap);
        va_end(ap);

        // glibc2.0 compatibility;
        if (ret == -1)
        {
            ret = bsize * 2;
        }

        if (ret <= bsize)
        {
            std::string retv(buf, ret);
            delete buf;
            return retv;
        }

        delete buf;
        bsize = ret + 1;
    }
    while(1);
}

std::string stlprintf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    std::string ret = vstlprintf(fmt, ap);
    va_end(ap);
    return ret;
}

/* stlprintf.cc */
