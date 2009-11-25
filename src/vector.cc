/* vector.cc
 * 
 * Copyright 2005 Martin Read
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

void compute_directions(libmrl::Coord c1, libmrl::Coord c2,
                        Direction_data *dir_data) //int *pdy, int *pdx, int *psy, int *psx, int *pmelee, int *pcardinal)
{
    dir_data->delta = c1;
    dir_data->delta -= c2;
    dir_data->sign.y = libmrl::sign(dir_data->delta.y);
    dir_data->sign.x = libmrl::sign(dir_data->delta.x);
    dir_data->meleerange = (c1.distance(c2) < 2);
    dir_data->oncardinal = ((dir_data->delta.y == 0) ||
                            (dir_data->delta.x == 0) ||
                            (libmrl::abs(dir_data->delta.y) ==
                             libmrl::abs(dir_data->delta.x)));
}

/* vector.cc */
