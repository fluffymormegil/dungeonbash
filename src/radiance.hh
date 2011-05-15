// radiance.hh
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

#ifndef RADIANCE_HH
#define RADIANCE_HH

/* XXX BEAM RADIANCE XXX
 *
 * Beam radiance can be used for projectiles, energy beams, etc. just by
 * changing the optics.
 *
 * The two kinds of absorption are to support exploding projectiles.
 */

#define OPTIC_TRANSMITTED 0
#define OPTIC_ABSORBED_INTERNAL 1
#define OPTIC_ABSORBED_EXTERNAL 2
#define OPTIC_REFLECTED 3
/* I'd do refraction and diffraction as well, but that just makes my brain
 * hurt :-) */

typedef int (*Beam_optic_func)(libmormegil::Coord c1, libmormegil::Coord c2, libmormegil::Offset *pdelta);

struct Beam_cell {
    libmormegil::Coord pos;
    int optic_state;
};

struct Beam_radiance {
    /* cells to be affected */
    Beam_cell *array;
    libmormegil::Coord origin;
    libmormegil::Coord dest;
    /* other data */
    int range;
    int range_remaining;
    int final_distance;
    /* optics function */
    Beam_optic_func optics;
    Beam_radiance(int r) : range(r)
    {
        array = new Beam_cell[range]();
    }
    ~Beam_radiance()
    {
        delete[] array;
    }
};

extern int irradiate_beam(Beam_radiance *beam);
extern Beam_radiance *allocate_beam(int range);
extern void discard_beam(Beam_radiance *beam);

/* XXX SQUARE RADIANCE XXX */

/* Square, not circular, because roguespace is non-Euclidean, and I have
 * decided to treat distance consistently throughout. */
typedef bool (*Square_block_func)(libmormegil::Coord c);
typedef bool (*Square_eff_func)(libmormegil::Coord c, void *data);
extern bool block_vision(libmormegil::Coord pos);

struct Square_radiance {
    /* flag map denoting squares to be affected */
    int array[21][21];
    /* centre of effect */
    libmormegil::Coord origin;
    /* radius */
    int radius;
    Square_block_func block;
};

extern libmormegil::Offset spiral_path[21 * 21];
extern int irradiate_square(Square_radiance *square, bool hole_in_middle = false);
extern int spiral_square(Square_radiance *square, Square_eff_func fptr, void *data);

#endif

/* radiance.hh */
