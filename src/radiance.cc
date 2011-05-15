// radiance.cc
// 
// Copyright 2009 Martin Read
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#include "dunbash.hh"
#include "radiance.hh"

/*****************************************************************************
 * XXX AN EXPLANATION OF THE SQUARE RADIANCE TABLE XXX
 *
 * There are (in principle) 65 destination points to cast rays to in the
 * octant (Triangular number 11 is 66, but the origin never gets cast to).
 * This would require bitvectors of size 65 bits, which would need a 128-bit
 * integer on power-of-two architectures.
 *
 * However, we can discard the following vectors from direct consideration:
 * 1,0 1,1 
 * 2,0 2,1 2,2
 * 3,0 3,1 3,2 3,3
 * 4,0 4,1 4,2 4,3 4,4
 * 5,0 5,1 5,2 5,3 5,4 5,5
 * 6,0 6,2 6,3 6,4 6,6
 * 7,0 7,7
 * 8,0 8,4 8,8
 * 9,0 9,9
 * on the following grounds:
 * 4,1 - equivalent to 8,2
 * 4,3 - equivalent to 8,6
 * 3,1 6,2 - equivalent to 9,3
 * 3,2 6,4 - equivalent to 9,6
 * 1,0 2,0 3,0 4,0 5,0 6,0 7,0 8,0 9,0 - equivalent to 10,0
 * 5,1 - equivalent to 10,2
 * 5,2 - equivalent to 10,4
 * 2,1 4,2 6,3 8,4 - equivalent to 10,5
 * 5,3 - equivalent to 10,6
 * 5,4 - equivalent to 10,8
 * 1,1 2,2 3,3 4,4 5,5 6,6 7,7 8,8 9,9 - equivalent to 10,10
 *
 * This leaves us with 33 distinct vectors, whose routes are as follows:
 *
 * 10, 0: 1,0 2,0 3,0 4,0 5,0 6,0 7,0 8,0 9,0 10,0
 * 10, 1: 1,0 2,0 3,0 4,0 5,0 6,0 7,0 8,0 9,0 10,1
 *  9, 1: 1,0 2,0 3,0 4,0 5,0 6,0 7,0 8,0 9,1 10,1
 *  8, 1: 1,0 2,0 3,0 4,0 5,0 6,0 7,0 8,1 9,1 10,1
 *  7, 1: 1,0 2,0 3,0 4,0 5,0 6,0 7,1 8,1 9,1 10,1
 *  6, 1: 1,0 2,0 3,0 4,0 5,0 6,1 7,1 8,1 9,1 10,1
 * 10, 2: 1,0 2,0 3,0 4,0 5,1 6,1 7,1 8,1 9,1 10,2
 *  9, 2: 1,0 2,0 3,0 4,0 5,1 6,1 7,1 8,1 9,2 10,2
 *  8, 2: 1,0 2,0 3,0 4,1 5,1 6,1 7,1 8,2 9,2 10,2
 *  7, 2: 1,0 2,0 3,0 4,1 5,1 6,1 7,2 8,2 9,2 10,2
 * 10, 3: 1,0 2,0 3,0 4,1 5,1 6,1 7,2 8,2 9,2 10,3
 *  9, 3: 1,0 2,0 3,1 4,1 5,1 6,2 7,2 8,2 9,3 10,3
 *  8, 3: 1,0 2,0 3,1 4,1 5,1 6,2 7,2 8,3 9,3 10,3
 * 10, 4: 1,0 2,0 3,1 4,1 5,2 6,2 7,2 8,3 9,3 10,4
 *  7, 3: 1,0 2,0 3,1 4,1 5,2 6,2 7,3 8,3 9,3 10,4
 *  9, 4: 1,0 2,0 3,1 4,1 5,2 6,2 7,3 8,3 9,4 10,4
 * 10, 5: 1,0 2,1 3,1 4,2 5,2 6,3 7,3 8,4 9,4 10,5
 *  9, 5: 1,0 2,1 3,1 4,2 5,2 6,3 7,3 8,4 9,5 10,5
 *  7, 4: 1,0 2,1 3,1 4,2 5,2 6,3 7,4 8,4 9,5 10,6
 * 10, 6: 1,0 2,1 3,1 4,2 5,3 6,3 7,4 8,4 9,5 10,6
 *  8, 5: 1,0 2,1 3,1 4,2 5,3 6,3 7,4 8,5 9,5 10,6
 *  9, 6: 1,0 2,1 3,2 4,2 5,3 6,4 7,4 8,5 9,6 10,6
 * 10, 7: 1,0 2,1 3,2 4,2 5,3 6,4 7,4 8,5 9,6 10,7
 *  7, 5: 1,0 2,1 3,2 4,2 5,3 6,4 7,5 8,5 9,6 10,7
 *  8, 6: 1,0 2,1 3,2 4,3 5,3 6,4 7,5 8,6 9,6 10,7
 *  9, 7: 1,0 2,1 3,2 4,3 5,3 6,4 7,5 8,6 9,7 10,7
 * 10, 8: 1,0 2,1 3,2 4,3 5,4 6,4 7,5 8,6 9,7 10,8
 *  6, 5: 1,0 2,1 3,2 4,3 5,4 6,5 7,5 8,6 9,7 10,8
 *  7, 6: 1,0 2,1 3,2 4,3 5,4 6,5 7,6 8,6 9,7 10,8
 *  8, 7: 1,0 2,1 3,2 4,3 5,4 6,5 7,6 8,7 9,7 10,8
 *  9, 8: 1,0 2,1 3,2 4,3 5,4 6,5 7,6 8,7 9,8 10,8
 * 10, 9: 1,0 2,1 3,2 4,3 5,4 6,5 7,6 8,7 9,8 10,9
 * 10,10: 1,1 2,2 3,3 4,4 5,5 6,6 7,7 8,8 9,9 10,10
 *
 * From this, we can produce an array of unsigned long longs containing
 * bitmasks. The rest of the algorithm should be fairly straightforward
 * to understand, even if the preprocessor construct should be taken
 * out and shot repeatedly in the head.
 */

unsigned long long squaremasks[11][11] =
{
    /* 0 - irrelevant */
    { },
    /* 1 */
    {
	0x0ffffffffull, 0x100000000ull
    },
    /* 2 */
    {
	0x00000ffffull, 0x0ffff0000ull, 0x100000000ull
    },
    /* 3 */
    {
	0x0000007ffull, 0x0001ff800ull, 0x0ffe00000ull, 0x100000000ull
    },
    /* 4 */
    {
	0x0000000ffull, 0x00000ff00ull, 0x000ff0000ull, 0x0ff000000ull,
	0x100000000ull
    },
    /* 5 */
    {
	0x00000003full, 0x000001fc0ull, 0x00007e000ull, 0x003f80000ull,
	0x0fc000000ull, 0x100000000ull,
    },
    /* 6 */
    {
	0x00000001full, 0x0000007e0ull, 0x00001f800ull, 0x0003e0000ull,
	0x007c00000ull, 0x0f8000000ull, 0x100000000ull,
    },
    /* 7 */
    {
	0x00000000full, 0x0000001f0ull, 0x000003e00ull, 0x00003c000ull,
	0x0007c0000ull, 0x00f800000ull, 0x0f0000000ull, 0x100000000ull
    },
    /* 8 */
    {
	0x000000007ull, 0x0000000f8ull, 0x000000f00ull, 0x00000f000ull,
	0x0000f0000ull, 0x000f00000ull, 0x01f000000ull, 0x0e0000000ull,
	0x100000000ull
    },
    /* 9 */
    {
	0x000000003ull, 0x00000007cull, 0x000000780ull, 0x000007800ull,
	0x000018000ull, 0x0001e0000ull, 0x001e00000ull, 0x03e000000ull,
	0x0c0000000ull, 0x100000000ull
    },
    /* 10 */
    { /* 1, 5, 4, 3, 3, 2, 4, 4, 5, 1, 1 */
	0x000000001ull, 0x00000003eull, 0x0000003c0ull, 0x000001c00ull,
	0x00000e000ull, 0x000030000ull, 0x0003c0000ull, 0x003c00000ull,
	0x07c000000ull, 0x080000000ull, 0x100000000ull
    }
};

/* Here follows one of the most staggeringly vile preprocessor constructs
 * I have ever seen.
 */
#define irradiate_octant(step, sweep, ostep, osweep) \
live_mask = 0xffffffffffffffffull; \
for (i = 1; i <= square->radius; i++) \
{ \
    c[1].step = c[0].step ostep i; \
    c[2].step = 10 ostep i; \
    for (j = 0; j <= i; j++) \
    { \
	c[1].sweep = c[0].sweep osweep j; \
	c[2].sweep = 10 osweep j; \
	if (currlev->outofbounds(c[1])) /*(y[1] < 0) || (y[1] >= DUN_HEIGHT) || (x[1] < 0) || (x[1] >= DUN_WIDTH))*/ \
	{ \
	    /* Out of bounds -> invisible. */ \
	    square->array[c[2].y][c[2].x] = 0; \
	    continue; \
	} \
	cell_mask = squaremasks[i][j]; \
	if (cell_mask & live_mask) \
	{ \
	    /* Set radiance-present flag. For now, I'll just have a simple \
	     * boolean flag and if I need more I'll revise the API. */ \
	    square->array[c[2].y][c[2].x] = 1; \
	} \
	else \
	{ \
	    square->array[c[2].y][c[2].x] = 0; \
	} \
	if (square->block(c[1])) \
	{ \
	    live_mask &= ~cell_mask; \
	} \
    } \
}

int irradiate_square(Square_radiance *square, bool hole_in_middle)
{
    libmormegil::Coord c[5];
    int i;
    int j;
    unsigned long long cell_mask;
    unsigned long long live_mask;

    square->array[10][10] = !hole_in_middle;
    c[0] = square->origin;
    irradiate_octant(x, y, +, +);
    irradiate_octant(x, y, +, -);
    irradiate_octant(x, y, -, +);
    irradiate_octant(x, y, -, -);
    irradiate_octant(y, x, +, +);
    irradiate_octant(y, x, +, -);
    irradiate_octant(y, x, -, +);
    irradiate_octant(y, x, -, -);
    return 0;
}

int spiral_square(Square_radiance *square, Square_eff_func fptr, void *data)
{
    int i;
    bool rv = false;
    libmormegil::Coord c;
    libmormegil::Coord o = { 10, 10 };
    for (i = 0; i < 21 * 21; ++i)
    {
        c = o + spiral_path[i];
        if (square->array[c.y][c.x])
        {
            bool tmp = fptr(square->origin + spiral_path[i], data);
            if (tmp)
            {
                rv = true;
            }
        }
    }
    return rv;
}

/*****************************************************************************/

int irradiate_beam(Beam_radiance *beam)
{
    Direction_data dirdata;
    libmormegil::Offset adelta;
    libmormegil::Coord c1 = beam->origin;
    libmormegil::Coord c2;
    int rtcap;
    int yrt = 0;
    int xrt = 0;
    int i;

    /* Extract the magic numbers. */
    compute_directions(beam->dest, beam->origin, &dirdata);
    adelta = libmormegil::abs(dirdata.delta);
    rtcap = std::max(adelta.y, adelta.x);
    c1 = beam->origin;
    beam->range_remaining = beam->range;
    beam->final_distance = 0;
    while ((c1 != beam->dest) && (beam->range_remaining))
    {
	yrt += adelta.y;
	xrt += adelta.x;
        c2 = c1;
	if (yrt >= rtcap)
	{
	    c1.y += dirdata.sign.y;
	    yrt -= rtcap;
	}
	if (xrt >= rtcap)
	{
	    c1.x += dirdata.sign.x;
	    xrt -= rtcap;
	}
	beam->final_distance++;
	beam->range_remaining--;
	i = beam->optics(c1, c2, &(dirdata.delta));
	switch (i)
	{
	case OPTIC_TRANSMITTED:
	    break;
	case OPTIC_ABSORBED_INTERNAL:
	    beam->range_remaining = 0;
	    break;
	case OPTIC_ABSORBED_EXTERNAL:
	    beam->range_remaining = 0;
	    break;
	case OPTIC_REFLECTED:
            adelta = libmormegil::abs(dirdata.delta);
	    xrt = 0;
	    yrt = 0;
	    rtcap = (adelta.y > adelta.x) ? adelta.y : adelta.x;
	    dirdata.sign = sign(dirdata.delta);
	    break;
	}
    }
    return 0;
}

// radiance.cc
